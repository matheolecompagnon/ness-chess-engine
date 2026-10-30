"""arena.py - banc de matchs moteur contre moteur, limites arbitraires.

Ce fichier a été généré par IA et validé par Mathéo LECOMPAGNON.

Dépendances : python3-chess (chess), Python >= 3.8

Difference avec bench/elo_match.py : ici les DEUX camps sont configurables
librement, et la limite de recherche peut etre exprimee en profondeur ou en
noeuds - donc SANS dependre du temps, donc reproductible et insensible a la
charge de la machine. C'est ce qu'il faut pour un etalonnage serieux.

Exemples :
  # Etalonnage de l'echelle UCI_Elo de Stockfish contre elle-meme
  python3 compare_engines.py --a "stockfish" --a-opt UCI_LimitStrength=true --a-opt UCI_Elo=1900 \
                  --b "stockfish" --b-opt UCI_LimitStrength=true --b-opt UCI_Elo=1400 \
                  --a-limit time=1.0 --b-limit time=1.0 --games 30

  # Ness a profondeur fixe contre Stockfish a nombre de noeuds fixe
  python3 compare_engines.py --a ./chess --a-limit depth=7 \
                  --b stockfish --b-limit nodes=20000 --games 40
"""
import argparse, math, random, sys, time, concurrent.futures as cf
import chess, chess.engine, chess.pgn, chess.polyglot, chess.syzygy


def parse_limit(spec):
    """'depth=7' / 'nodes=20000' / 'time=1.0' -> chess.engine.Limit"""
    k, _, v = spec.partition("=")
    k = k.strip().lower()
    if k == "depth":  return chess.engine.Limit(depth=int(v))
    if k == "nodes":  return chess.engine.Limit(nodes=int(v))
    if k == "time":   return chess.engine.Limit(time=float(v))
    sys.exit("limite inconnue : %s (attendu depth=, nodes= ou time=)" % spec)


def parse_opts(pairs):
    """['UCI_Elo=1400', 'Threads=1'] -> {'UCI_Elo': 1400, 'Threads': 1}
    L'ordre est preserve : UCI_LimitStrength doit etre pose avant UCI_Elo."""
    out = {}
    for p in pairs or []:
        k, _, v = p.partition("=")
        v = v.strip()
        if v.lower() in ("true", "false"):
            out[k.strip()] = (v.lower() == "true")
        else:
            try:
                out[k.strip()] = int(v)
            except ValueError:
                out[k.strip()] = v
    return out


def load_openings(path, needed):
    with open(path) as f:
        fens = [ln.strip() for ln in f if ln.strip()]
    step = max(1, len(fens) // max(needed, 1))
    return (fens[::step] * (needed // max(len(fens[::step]), 1) + 1))[:needed]


def book_move(reader, board, book_depth):
    """Renvoie un coup tire du livre Polyglot pour la position, ou None hors livre.

    Le livre appartient au HARNAIS, pas au moteur : c'est ici, cote arbitre, qu'on joue les coups
    d'ouverture, exactement comme le ferait lichess-bot ou cutechess. Le moteur reste pur.
    Un coup est tire au hasard, pondere par les poids du livre (varie les parties), tant qu'on est
    dans les 'book_depth' premiers demi-coups et que la position figure au livre.
    """
    if reader is None or board.ply() >= book_depth:
        return None
    try:
        return reader.weighted_choice(board).move
    except IndexError:
        return None  # position absente du livre : on sort du livre


def syzygy_result(tb, board, max_pieces):
    """Adjudication par tables de finales Syzygy (cote arbitre, pas dans le moteur).

    Des qu'il reste <= max_pieces pieces, le resultat theorique est CONNU : on l'adjuge au lieu
    de laisser jouer une finale que le moteur pourrait mal conclure. C'est la methode standard
    des outils de test (cutechess fait pareil), et c'est representatif du bot en ligne, ou
    lichess-bot joue les coups Syzygy et convertit reellement ces positions.

    Renvoie le resultat DU POINT DE VUE DU TRAIT : +1 gagne, -1 perd, 0 nulle. None si trop de
    pieces, table absente, ou resultat "maudit/benit" (+/-1 WDL, qui depend de la regle des 50
    coups) : dans ces cas on laisse la partie se jouer.
    """
    if tb is None or chess.popcount(board.occupied) > max_pieces:
        return None
    try:
        wdl = tb.probe_wdl(board)
    except (KeyError, ValueError, chess.syzygy.MissingTableError):
        return None
    if wdl >= 2:
        return 1
    if wdl <= -2:
        return -1
    if wdl == 0:
        return 0
    return None  # +/-1 : gain maudit / perte benie, on laisse jouer


def tb_best_move(tb, board, max_pieces):
    """Coup de finale optimal (technique parfaite) selon les tables Syzygy, ou None.

    Ne "convertit" QUE les gains francs (WDL = +2) : on choisit le coup qui progresse le plus vite
    vers le mat au sens DTZ (distance au prochain coup remettant a zero le compteur des 50 coups),
    ce qui garantit la conversion sous la regle des 50 coups. Renvoie None si trop de pieces, tables
    incompletes, ou position nulle / perdue / gain maudit : dans ces cas le MOTEUR joue (la table
    convertit les gains francs de finale, elle ne remplace pas la recherche partout). C'est ce que
    fait lichess-bot en ligne : coups Syzygy en finale, moteur sinon.
    """
    if tb is None or chess.popcount(board.occupied) > max_pieces:
        return None
    try:
        if tb.probe_wdl(board) < 2:            # on ne convertit que les gains francs
            return None
        cur_dtz = abs(tb.probe_dtz(board))     # distance actuelle au coup remettant le compteur a 0
    except (KeyError, ValueError, chess.syzygy.MissingTableError):
        return None
    best_move, best_key = None, None
    for move in board.legal_moves:
        zeroing = board.is_zeroing(move)       # capture / poussee de pion -> remet le compteur a 0
        board.push(move)
        try:
            wdl_after = tb.probe_wdl(board)    # du POV adverse (au trait apres notre coup)
            dtz_after = tb.probe_dtz(board)
        except (KeyError, ValueError, chess.syzygy.MissingTableError):
            board.pop()
            return None
        repeats = board.is_repetition(3)       # ce coup donnerait-il la nulle par repetition ?
        board.pop()
        if wdl_after > -2:                      # ce coup ne maintient pas un gain franc -> rejete
            continue
        # PROGRES obligatoire : soit un coup a zero (capture / pion, qui rapproche du mat en gagnant
        # du materiel), soit un coup qui fait STRICTEMENT decroitre le DTZ. Sans cette contrainte, la
        # selection DTZ-gloutonne peut stagner sur des egalites et boucler jusqu'a la repetition
        # (cf. KQvKR, KQvKB). Cle de tri (minimisee) : progres, puis non-repetition, puis DTZ, puis
        # preference aux coups a zero.
        progresses = zeroing or (abs(dtz_after) < cur_dtz)
        key = (0 if progresses else 1, 1 if repeats else 0, abs(dtz_after), 0 if zeroing else 1)
        if best_key is None or key < best_key:
            best_key, best_move = key, move
    return best_move


def play_game(a_cmd, a_opts, a_limit, b_cmd, b_opts, b_limit,
              fen, a_white, max_moves, a_book=None, b_book=None, book_depth=16,
              syzygy_path=None, syzygy_pieces=5, clock=None, checkpoint_ply=30,
              a_syzygy=None, b_syzygy=None):
    """Renvoie (score_de_A, raison, nb_demi_coups, temps_de_A).

    Si \b clock = (base_ms, inc_ms) est fourni, la partie se joue a la PENDULE (temps partage sur
    toute la partie, comme sur Lichess) : chaque camp part de base_ms, on decompte le temps
    reellement passe a chaque coup et on ajoute inc_ms ; un camp a court de temps perd au drapeau.
    Un coup de LIVRE (joue par l'arbitre) ne coute presque rien -> le camp avec livre garde sa
    pendule pendant l'ouverture. C'est ce qui rend le "temps gagne" mesurable en jouant.

    \b temps_de_A vaut (pendule de A, pendule de B) en ms au demi-coup \b checkpoint_ply (sortie
    d'ouverture), pour comparer directement le temps restant des deux camps ; None hors pendule.
    """
    ea = chess.engine.SimpleEngine.popen_uci(a_cmd)
    eb = chess.engine.SimpleEngine.popen_uci(b_cmd)
    ra = chess.polyglot.open_reader(a_book) if a_book else None
    rb = chess.polyglot.open_reader(b_book) if b_book else None
    tb = chess.syzygy.open_tablebase(syzygy_path) if syzygy_path else None
    # Tables Syzygy JOUEES par camp (technique de finale parfaite) : chaque camp qui en dispose
    # joue le coup DTZ-optimal des qu'on entre dans le domaine des tables (<= syzygy_pieces).
    tba = chess.syzygy.open_tablebase(a_syzygy) if a_syzygy else None
    tbb = chess.syzygy.open_tablebase(b_syzygy) if b_syzygy else None
    # Pendules (ms) et instantane du temps restant a la sortie d'ouverture.
    wclock = bclock = (clock[0] if clock else 0)
    inc = clock[1] if clock else 0
    snapshot = None  # (pendule blanc, pendule noir) au demi-coup checkpoint_ply
    try:
        if a_opts: ea.configure(a_opts)
        if b_opts: eb.configure(b_opts)
        board = chess.Board(fen)
        while not board.is_game_over(claim_draw=True):
            if board.fullmove_number > max_moves:
                return 0.5, "limite de coups", board.ply(), snapshot
            a_turn = (board.turn == chess.WHITE) == a_white
            eng, lim = (ea, a_limit) if a_turn else (eb, b_limit)
            reader = ra if a_turn else rb
            mover_white = (board.turn == chess.WHITE)
            # 1. Tenter le livre (cote arbitre, quasi gratuit). 2. Sinon, demander au moteur en
            # lui passant la pendule s'il y en a une, pour qu'il gere son temps lui-meme.
            t0 = time.monotonic()
            move = book_move(reader, board, book_depth)
            if move is None:
                side_tb = tba if a_turn else tbb
                if side_tb is not None:
                    move = tb_best_move(side_tb, board, syzygy_pieces)  # coup de finale parfait (DTZ)
            if move is None:
                if clock is not None:
                    lim = chess.engine.Limit(white_clock=wclock / 1000.0, black_clock=bclock / 1000.0,
                                             white_inc=inc / 1000.0, black_inc=inc / 1000.0)
                try:
                    res = eng.play(board, lim)
                except Exception as exc:
                    return (0.0 if a_turn else 1.0), "erreur moteur (%s)" % type(exc).__name__, board.ply(), snapshot
                move = res.move
            elapsed_ms = (time.monotonic() - t0) * 1000.0
            # Decompte de la pendule du joueur au trait.
            if clock is not None:
                if mover_white:
                    wclock -= elapsed_ms
                    if wclock <= 0:
                        return (0.0 if a_white else 1.0), "drapeau", board.ply(), snapshot
                    wclock += inc
                else:
                    bclock -= elapsed_ms
                    if bclock <= 0:
                        return (1.0 if a_white else 0.0), "drapeau", board.ply(), snapshot
                    bclock += inc
            if move is None or move not in board.legal_moves:
                return (0.0 if a_turn else 1.0), "coup illegal", board.ply(), snapshot
            board.push(move)
            if clock is not None and snapshot is None and board.ply() >= checkpoint_ply:
                snapshot = (wclock, bclock)  # temps restant a la sortie d'ouverture
            # Adjudication Syzygy apres le coup.
            r = syzygy_result(tb, board, syzygy_pieces)
            if r is not None:
                if r == 0:
                    return 0.5, "adjugee TB (nulle)", board.ply(), snapshot
                stm_is_a = (board.turn == chess.WHITE) == a_white
                a_wins = (r == 1) == stm_is_a
                return (1.0 if a_wins else 0.0), "adjugee TB", board.ply(), snapshot
        out = board.outcome(claim_draw=True)
        if out.winner is None:
            sc = 0.5
        else:
            sc = 1.0 if (out.winner == chess.WHITE) == a_white else 0.0
        return sc, out.termination.name.lower().replace("_", " "), board.ply(), snapshot
    finally:
        for e in (ea, eb):
            try: e.quit()
            except Exception: pass
        for r in (ra, rb):
            if r is not None:
                try: r.close()
                except Exception: pass
        for t in (tb, tba, tbb):
            if t is not None:
                try: t.close()
                except Exception: pass


def elo(score):
    score = min(max(score, 1e-9), 1 - 1e-9)
    return -400.0 * math.log10(1.0 / score - 1.0)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--a", required=True);  ap.add_argument("--b", required=True)
    ap.add_argument("--a-opt", action="append"); ap.add_argument("--b-opt", action="append")
    ap.add_argument("--a-limit", default="time=1.0"); ap.add_argument("--b-limit", default="time=1.0")
    ap.add_argument("--a-name", default="A"); ap.add_argument("--b-name", default="B")
    ap.add_argument("--games", type=int, default=40)
    ap.add_argument("--openings", default="bench/openings.epd")
    ap.add_argument("--concurrency", type=int, default=4)
    ap.add_argument("--max-moves", type=int, default=200)
    # Livre d'ouverture Polyglot par camp (facultatif). Le livre est joue par le HARNAIS, jamais
    # par le moteur : c'est ce qui donne les combinaisons avec-livre / sans-livre sans toucher
    # aux binaires. Donne les 4 cases du 2x2 v1/v2 x sans-livre/avec-livre.
    ap.add_argument("--a-book", default=None, help="livre .bin pour le camp A (defaut : aucun)")
    ap.add_argument("--b-book", default=None, help="livre .bin pour le camp B (defaut : aucun)")
    ap.add_argument("--book-depth", type=int, default=16,
                    help="profondeur max d'utilisation du livre, en demi-coups (defaut 16)")
    # Tables de finales Syzygy, cote ARBITRE (comme le livre) : les positions <= N pieces sont
    # adjugees au resultat theorique. S'appliquent aux DEUX camps a la fois (c'est l'arbitre).
    ap.add_argument("--syzygy", default=None,
                    help="dossier des tables Syzygy pour adjuger les finales (defaut : aucun)")
    ap.add_argument("--syzygy-pieces", type=int, default=5,
                    help="nombre de pieces max pour adjuger via Syzygy (defaut 5)")
    # Tables Syzygy JOUEES par un camp (technique de finale parfaite, coup DTZ-optimal), comme le
    # fait lichess-bot en ligne. A la difference de --syzygy (arbitre, symetrique), celles-ci ne
    # profitent qu'au camp qui les recoit -> c'est la comparaison "avec / sans fermetures".
    ap.add_argument("--a-syzygy", default=None,
                    help="tables Syzygy JOUEES par le camp A en finale (finale parfaite)")
    ap.add_argument("--b-syzygy", default=None,
                    help="tables Syzygy JOUEES par le camp B en finale (finale parfaite)")
    # Pendule de partie "base+increment" en secondes (ex. "60+0"). Active le jeu au temps PARTAGE
    # sur toute la partie : le camp qui joue son ouverture au livre garde sa pendule pour la suite.
    # C'est ce qui rend le "temps gagne" par le livre mesurable en jouant (--a-limit/--b-limit
    # sont alors ignores : le moteur gere son temps a partir de la pendule).
    ap.add_argument("--clock", default=None, help="pendule 'base+inc' en secondes, ex. 60+0")
    ap.add_argument("--checkpoint", type=int, default=30,
                    help="demi-coup ou relever le temps restant de chaque camp (defaut 30)")
    args = ap.parse_args()

    clock = None
    if args.clock:
        base, _, incs = args.clock.partition("+")
        clock = (int(float(base) * 1000), int(float(incs or 0) * 1000))  # (base_ms, inc_ms)

    a_opts, b_opts = parse_opts(args.a_opt), parse_opts(args.b_opt)
    a_lim, b_lim = parse_limit(args.a_limit), parse_limit(args.b_limit)

    npairs = max(1, args.games // 2)
    fens = load_openings(args.openings, npairs)
    # Chaque ouverture jouee deux fois, couleurs inversees : annule l'avantage des blancs.
    jobs = [(i, fen, aw) for i, fen in enumerate(fens) for aw in (True, False)]

    print("== %s  vs  %s ==" % (args.a_name, args.b_name))
    print("   %s : %s  %s  livre=%s" % (args.a_name, args.a_limit, a_opts or "", args.a_book or "aucun"))
    print("   %s : %s  %s  livre=%s" % (args.b_name, args.b_limit, b_opts or "", args.b_book or "aucun"))
    if args.syzygy:
        print("   Syzygy (arbitre) : %s, adjuge <= %d pieces" % (args.syzygy, args.syzygy_pieces))
    if args.a_syzygy or args.b_syzygy:
        print("   Syzygy (jeu par camp, finale parfaite <= %d pieces) : %s=%s  %s=%s"
              % (args.syzygy_pieces, args.a_name, "oui" if args.a_syzygy else "non",
                 args.b_name, "oui" if args.b_syzygy else "non"))
    if clock:
        print("   Pendule : %.0f s + %.1f s/coup (temps partage sur la partie)" % (clock[0] / 1000, clock[1] / 1000))
    print("   %d parties, %d en parallele\n" % (len(jobs), args.concurrency), flush=True)

    w = d = l = 0
    reasons = {}
    a_times, b_times = [], []  # pendule restante (ms) de chaque camp au demi-coup checkpoint
    with cf.ThreadPoolExecutor(max_workers=args.concurrency) as ex:
        futs = {ex.submit(play_game, args.a, a_opts, a_lim, args.b, b_opts, b_lim,
                          fen, aw, args.max_moves, args.a_book, args.b_book, args.book_depth,
                          args.syzygy, args.syzygy_pieces, clock, args.checkpoint,
                          args.a_syzygy, args.b_syzygy):
                (i, aw) for i, fen, aw in jobs}
        for n, fut in enumerate(cf.as_completed(futs), 1):
            try:
                sc, why, plies, snap = fut.result()
            except Exception as exc:
                print("   partie en echec : %r" % exc, flush=True); continue
            if   sc == 1.0: w += 1
            elif sc == 0.5: d += 1
            else:           l += 1
            reasons[why] = reasons.get(why, 0) + 1
            if snap is not None:
                aw = futs[fut][1]  # A jouait-il les blancs ?
                a_times.append(snap[0] if aw else snap[1])
                b_times.append(snap[1] if aw else snap[0])
            print("[%3d/%3d] %s -> %.1f  (%s, %d demi-coups)  cumul %d-%d-%d"
                  % (n, len(jobs), args.a_name, sc, why, plies, w, d, l), flush=True)

    n = w + d + l
    if n == 0: sys.exit("aucune partie jouee")
    s = (w + d / 2) / n
    se = math.sqrt(max(s * (1 - s), 1e-12) / n)
    lo, hi = max(s - 1.96 * se, 1e-9), min(s + 1.96 * se, 1 - 1e-9)

    print("\n" + "=" * 62)
    print("%s : %d gains, %d nulles, %d defaites sur %d" % (args.a_name, w, d, l, n))
    print("Score %s      : %.1f %%" % (args.a_name, 100 * s))
    print("Ecart d'Elo      : %+.0f   IC 95%% [%+.0f ; %+.0f]" % (elo(s), elo(lo), elo(hi)))
    print("Fins de partie   : %s" % ", ".join("%s x%d" % kv for kv in sorted(reasons.items(), key=lambda t: -t[1])))
    if a_times and b_times:
        ma = sum(a_times) / len(a_times) / 1000.0
        mb = sum(b_times) / len(b_times) / 1000.0
        print("-" * 62)
        print("Temps restant au demi-coup %d (moyenne sur %d parties) :" % (args.checkpoint, len(a_times)))
        print("  %-14s : %.1f s" % (args.a_name, ma))
        print("  %-14s : %.1f s" % (args.b_name, mb))
        print("  Gain de temps de %s : %+.1f s en jouant" % (args.a_name, ma - mb))
    print("=" * 62)


if __name__ == "__main__":
    main()
