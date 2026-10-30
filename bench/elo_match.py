#!/usr/bin/env python3
"""Mesure la force de Ness en Elo par matchs contre Stockfish bride.

Ce fichier a été rédigé par IA puis validé par Mathéo LECOMPAGNON.

Dépendances : Python >= 3.8, stockfish, binaire du moteur, un binaire du moteur
(en argument --engine <moteur_exe> si le nom du moteur n'est pas chess)

Principe : on ne peut pas mesurer un Elo dans l'absolu, seulement un *ecart*
d'Elo entre deux joueurs. On fait donc jouer Ness contre un adversaire dont
la force est connue - Stockfish bride a un Elo cible via son option
UCI_LimitStrength / UCI_Elo, qui sert d'etalon - puis on convertit le score
obtenu en ecart d'Elo par la formule logistique du systeme :

    ecart = -400 * log10(1 / score - 1)      avec score = (V + N/2) / parties

L'Elo estime de Ness est alors Elo(etalon) + ecart. Cet etalon est approximatif
(la calibration UCI_Elo de Stockfish n'est pas la meme echelle que le classement
FIDE ou Lichess), ce qui est signale dans le resultat : ce qui est solide, c'est
l'ecart mesure et sa marge d'erreur, pas la valeur absolue.

Protocole applique, dans l'esprit du reste du banc (bench/compare.sh) :
  - livre d'ouvertures equilibre (bench/gen_openings.py), car Ness est
    deterministe et rejouerait sinon toujours la meme partie ;
  - chaque ouverture jouee DEUX fois, couleurs inversees, pour annuler
    l'avantage des blancs ;
  - meme budget de temps par coup pour les deux moteurs ;
  - nulles par repetition, regle des 50 coups et materiel insuffisant
    reclamees automatiquement (Ness ne connait pas ces regles et pourrait
    sinon errer indefiniment dans une position nulle).

Exemple :
    bench/.venv/bin/python bench/elo_match.py --games 40 --movetime 1000 \
        --sf-elo 1500
"""

import argparse
import math
import os
import sys
import threading
import chess
import chess.engine
import chess.pgn

RESULT_LOCK = threading.Lock()


def load_openings(path, needed):
    """Charge le livre d'ouvertures et en preleve `needed` positions.

    Le prelevement est regulierement espace plutot que pris en tete de fichier :
    les premieres lignes de l'EPD partagent leurs premiers demi-coups et
    donneraient un echantillon peu varie.
    """
    with open(path) as f:
        fens = [ln.strip() for ln in f if ln.strip()]
    if not fens:
        sys.exit("livre d'ouvertures vide : %s" % path)
    step = max(1, len(fens) // needed)
    picked = fens[::step][:needed]
    while len(picked) < needed:  # livre plus court que demande : on boucle
        picked += fens[:needed - len(picked)]
    return picked


def play_game(ness_cmd, sf_cmd, sf_opts, fen, ness_white, movetime_ms,
              max_moves):
    """Joue une partie et renvoie (score_ness, board, raison).

    score_ness vaut 1.0 (gain), 0.5 (nulle) ou 0.0 (perte).
    """
    limit = chess.engine.Limit(time=movetime_ms / 1000.0)
    ness = chess.engine.SimpleEngine.popen_uci(ness_cmd)
    sf = chess.engine.SimpleEngine.popen_uci(sf_cmd)
    try:
        sf.configure(sf_opts)
        board = chess.Board(fen)
        while not board.is_game_over(claim_draw=True):
            if board.fullmove_number > max_moves:
                return 0.5, board, "limite de %d coups" % max_moves
            engine = ness if (board.turn == chess.WHITE) == ness_white else sf
            who = "Ness" if engine is ness else "Stockfish"
            try:
                result = engine.play(board, limit)
            except chess.engine.EngineError as exc:
                loser_is_ness = engine is ness
                return (0.0 if loser_is_ness else 1.0), board, \
                    "erreur moteur (%s) : %s" % (who, exc)
            if result.move is None or result.move not in board.legal_moves:
                loser_is_ness = engine is ness
                return (0.0 if loser_is_ness else 1.0), board, \
                    "coup illegal de %s : %s" % (who, result.move)
            board.push(result.move)

        outcome = board.outcome(claim_draw=True)
        reason = outcome.termination.name.lower().replace("_", " ")
        if outcome.winner is None:
            return 0.5, board, reason
        return (1.0 if outcome.winner == (chess.WHITE if ness_white
                                          else chess.BLACK) else 0.0), \
            board, reason
    finally:
        ness.quit()
        sf.quit()


def elo_from_score(score):
    """Convertit un score (0..1) en ecart d'Elo. Renvoie None si 0 % ou 100 %."""
    if score <= 0.0 or score >= 1.0:
        return None
    return -400.0 * math.log10(1.0 / score - 1.0)


def margin(wins, draws, losses):
    """Marge d'erreur a 95 % sur l'ecart d'Elo, en points d'Elo.

    On calcule l'ecart-type du score moyen a partir de la distribution reelle
    des resultats (et non d'une approximation binomiale a deux issues, qui
    ignorerait les nulles), puis on le propage sur la courbe logistique.
    """
    n = wins + draws + losses
    if n < 2:
        return None
    score = (wins + 0.5 * draws) / n
    var = (wins * (1 - score) ** 2 + draws * (0.5 - score) ** 2
           + losses * (0 - score) ** 2) / n
    sigma = math.sqrt(var / n)
    lo, hi = score - 1.96 * sigma, score + 1.96 * sigma
    lo, hi = min(max(lo, 1e-6), 1 - 1e-6), min(max(hi, 1e-6), 1 - 1e-6)
    return (elo_from_score(hi) - elo_from_score(lo)) / 2.0


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--engine", default="./chess", help="binaire Ness")
    ap.add_argument("--stockfish", default="stockfish", help="binaire Stockfish")
    ap.add_argument("--openings", default="bench/openings.epd",
                    help="livre d'ouvertures (bench/gen_openings.py)")
    ap.add_argument("--games", type=int, default=40,
                    help="nombre de parties (arrondi au pair : couleurs inversees)")
    ap.add_argument("--movetime", type=int, default=1000,
                    help="budget de temps par coup, en ms, pour les DEUX moteurs")
    ap.add_argument("--sf-elo", type=int, default=1500,
                    help="Elo cible de Stockfish (UCI_LimitStrength), min 1320")
    ap.add_argument("--sf-skill", type=int, default=None,
                    help="alternative : Skill Level 0..20 au lieu de --sf-elo")
    ap.add_argument("--concurrency", type=int, default=4,
                    help="parties menees en parallele")
    ap.add_argument("--max-moves", type=int, default=200,
                    help="nulle technique au-dela de ce nombre de coups")
    ap.add_argument("--pgn", default="bench/match.pgn",
                    help="fichier PGN des parties jouees")
    args = ap.parse_args()

    if args.sf_skill is not None:
        sf_opts = {"Skill Level": args.sf_skill, "Threads": 1, "Hash": 16}
        anchor_label = "Stockfish Skill Level %d" % args.sf_skill
        anchor_elo = None
    else:
        sf_opts = {"UCI_LimitStrength": True, "UCI_Elo": args.sf_elo,
                   "Threads": 1, "Hash": 16}
        anchor_label = "Stockfish UCI_Elo %d" % args.sf_elo
        anchor_elo = args.sf_elo

    npairs = max(1, args.games // 2)
    fens = load_openings(args.openings, npairs)

    # Chaque ouverture donne deux parties, Ness blanc puis Ness noir.
    jobs = [(i, fen, white)
            for i, fen in enumerate(fens) for white in (True, False)]

    results = []
    games_pgn = []
    done = [0]

    def worker(job):
        idx, fen, ness_white = job
        score, board, reason = play_game(
            args.engine, args.stockfish, sf_opts, fen, ness_white,
            args.movetime, args.max_moves)
        game = chess.pgn.Game.from_board(board)
        game.headers["Event"] = "Ness vs %s" % anchor_label
        game.headers["White"] = "Ness" if ness_white else "Stockfish"
        game.headers["Black"] = "Stockfish" if ness_white else "Ness"
        game.headers["FEN"] = fen
        game.headers["SetUp"] = "1"
        game.headers["Termination"] = reason
        with RESULT_LOCK:
            results.append(score)
            games_pgn.append(str(game))
            done[0] += 1
            w = results.count(1.0)
            d = results.count(0.5)
            ls = results.count(0.0)
            print("[%3d/%3d] ouverture %3d, Ness %s -> %.1f  (%s)  "
                  "cumul %d-%d-%d"
                  % (done[0], len(jobs), idx,
                     "blancs" if ness_white else "noirs", score, reason,
                     w, d, ls), flush=True)

    # Ordonnanceur simple : un pool de threads qui pioche dans la liste de
    # parties. Chaque partie lance ses deux processus moteur ; le parallelisme
    # est donc borne pour ne pas fausser les budgets de temps par coup.
    lock = threading.Lock()
    it = iter(jobs)

    def run():
        while True:
            with lock:
                job = next(it, None)
            if job is None:
                return
            worker(job)

    threads = [threading.Thread(target=run)
               for _ in range(min(args.concurrency, len(jobs)))]
    for t in threads:
        t.start()
    for t in threads:
        t.join()

    wins = results.count(1.0)
    draws = results.count(0.5)
    losses = results.count(0.0)
    n = len(results)
    score = (wins + 0.5 * draws) / n
    diff = elo_from_score(score)
    err = margin(wins, draws, losses)

    with open(args.pgn, "w") as f:
        f.write("\n\n".join(games_pgn) + "\n")

    print()
    print("=" * 62)
    print("Adversaire etalon : %s" % anchor_label)
    print("Budget par coup   : %d ms (identique pour les deux moteurs)" % args.movetime)
    print("Parties           : %d  (%d gains, %d nulles, %d defaites)"
          % (n, wins, draws, losses))
    print("Score de Ness    : %.1f %%" % (100 * score))
    if diff is None:
        print("Ecart d'Elo       : non calculable (score de 0 % ou 100 %),")
        print("                    rejouer contre un etalon plus adapte")
    else:
        print("Ecart d'Elo       : %+.0f%s"
              % (diff, "" if err is None else "  +/- %.0f (95 %%)" % err))
        if anchor_elo is not None:
            print("Elo estime Ness  : %.0f%s"
                  % (anchor_elo + diff,
                     "" if err is None else "  +/- %.0f" % err))
            print()
            print("Reserve : l'echelle UCI_Elo de Stockfish est une calibration")
            print("interne, pas un classement FIDE ni Lichess. L'ecart mesure et")
            print("sa marge sont fiables ; la valeur absolue est indicative.")
    print("PGN des parties   : %s" % args.pgn)
    print("=" * 62)


if __name__ == "__main__":
    main()
