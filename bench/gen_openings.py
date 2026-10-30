#!/usr/bin/env python3
"""Genere un livre d'ouvertures equilibre pour les matchs de mesure d'Elo.

Ce fichier a été écrit par IA puis validé par Mathéo LECOMPAGNON

Dépendances : Python >= 3.8, stockfish, python3-chess

Pourquoi ce fichier existe : Ness est un moteur *deterministe*. A position
egale, profondeur egale et budget de temps egal, il rejoue exactement le meme
coup. Si l'on faisait disputer 40 parties depuis la position de depart, on
obtiendrait 40 fois la meme partie : la mesure n'aurait aucune valeur
statistique (n = 1, pas n = 40).

La parade standard est un livre d'ouvertures : chaque partie demarre d'une
position differente, et la meme position est jouee deux fois avec les couleurs
inversees pour annuler l'avantage des blancs.

Methode : on developpe un arbre depuis la position de depart en demandant a
Stockfish ses MULTIPV meilleurs coups a chaque demi-coup, et on ne conserve
que les lignes dont l'evaluation reste dans +/- MAX_CP centipions. On obtient
ainsi des ouvertures a la fois variees et equilibrees - une ouverture deja
gagnante fausserait la mesure autant qu'une ouverture unique.

Sortie : un fichier EPD (une FEN par ligne) consomme par elo_match.py.
"""

import argparse
import chess
import chess.engine

PLIES = 6      # profondeur du livre en demi-coups
MULTIPV = 3    # nombre de coups candidats retenus par position
MAX_CP = 60    # ecart d'evaluation tolere, en centipions
DEPTH = 12     # profondeur d'analyse de Stockfish pour trier les candidats


def expand(engine, board, plies_left, out, seen):
    """Developpe recursivement l'arbre d'ouvertures depuis board."""
    if plies_left == 0:
        fen = board.epd()
        if fen not in seen:
            seen.add(fen)
            out.append(fen)
        return

    infos = engine.analyse(
        board, chess.engine.Limit(depth=DEPTH), multipv=MULTIPV
    )
    for info in infos:
        score = info["score"].white().score(mate_score=100000)
        if score is None or abs(score) > MAX_CP:
            continue
        pv = info.get("pv")
        if not pv:
            continue
        board.push(pv[0])
        expand(engine, board, plies_left - 1, out, seen)
        board.pop()


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--stockfish", default="stockfish",
                    help="chemin du binaire Stockfish (arbitre de qualite)")
    ap.add_argument("--out", default="bench/openings.epd",
                    help="fichier EPD produit")
    ap.add_argument("--plies", type=int, default=PLIES,
                    help="profondeur du livre en demi-coups")
    args = ap.parse_args()

    engine = chess.engine.SimpleEngine.popen_uci(args.stockfish)
    try:
        out, seen = [], set()
        expand(engine, chess.Board(), args.plies, out, seen)
    finally:
        engine.quit()

    with open(args.out, "w") as f:
        for fen in out:
            f.write(fen + "\n")
    print("%d ouvertures ecrites dans %s (%d demi-coups, |eval| <= %d cp)"
          % (len(out), args.out, args.plies, MAX_CP))


if __name__ == "__main__":
    main()
