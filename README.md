<p align="center">
  <img src="logo_ness_engine.svg" alt="Ness engine" width="150" />
</p>

<h1 align="center">NESS chess engine</h1>

<p align="center">
  <a href="https://github.com/matheolecompagnon/ness-chess-engine/actions">
    <img src="https://img.shields.io/github/actions/workflow/status/matheolecompagnon/ness-chess-engine/ci.yml" alt="CI" />
  </a>
  <a href="https://github.com/matheolecompagnon/ness-chess-engine/releases">
    <img src="https://img.shields.io/github/v/release/matheolecompagnon/ness-chess-engine?include_prereleases" alt="Release" />
  </a>
  <a href="LICENSE">
    <img src="https://img.shields.io/github/license/matheolecompagnon/ness-chess-engine" alt="License" />
  </a>
  <a href="https://github.com/matheolecompagnon/ness-chess-engine/commits/main">
    <img src="https://img.shields.io/github/last-commit/matheolecompagnon/ness-chess-engine" alt="Last commit" />
  </a>
  <img src="https://img.shields.io/badge/language-C11-blue" alt="C11" />
  <img src="https://img.shields.io/badge/protocol-UCI-green" alt="UCI" />
  <img src="https://img.shields.io/badge/platform-Linux-lightgrey" alt="Linux" />
</p>

---

Moteur d'échecs en C11 : bitboards, recherche negamax alpha-bêta, évaluation NNUE, protocole UCI. Aucune dépendance hors bibliothèque standard (CUnit pour les tests uniquement).

## Démarrage rapide

```bash
make
./chess
```

Puis, dans le terminal pour tester :
```text
position startpos moves e2e4 e7e5
go movetime 1000
```
Le moteur répond `bestmove <coup>`. Pour l'utiliser dans une interface graphique (Knights, Arena, Cute Chess), indiquez le chemin du binaire `./chess`.

**Prérequis :** `gcc`, `make`. 
**Tests :** `libcunit1-dev`, puis `make test`.

## Commandes UCI reconnues

`uci`, `isready`, `ucinewgame`, `position`, `go`, `help`, `quit`.

*   `position startpos [moves e2e4 ...]` ou `position fen <FENE_CHAMPS> [moves ...]`
*   `go [depth n] [movetime ms] [wtime ms] [btime ms] [winc ms] [binc ms] [movestogo n]`

_Note : Les fonctions `stop`, `ponder` et `infinite` ne sont pas gérées. La recherche est bloquante. Les promotions s'écrivent en minuscules (`e7e8q`). Les options `setoption` sont ignorées._

## Architecture & Fonctionnement

*   **Représentation :** Bitboard de 64 bits par couleur et par type de pièce. Coups appliqués et annulés en place (*make/unmake*), sans copie de l'échiquier.
*   **Recherche :** Negamax avec élagage alpha-bêta en approfondissement itératif. Intègre une table de transposition (clés Zobrist, 2^20 entrées), le *null-move pruning*, la recherche PVS, les réductions de coups tardifs (LMR) et la détection de répétition (pénalité de 25 centipions).
*   **Gestion du temps :** Calcul automatique du budget par coup en fonction du temps restant et de l'incrément envoyé par l'interface UCI. Pensez à fournir les variables de temps lors du protocole `go`.
*   **Livre d'ouverture :** Format Polyglot optionnel via `make BOOK=1`. Utilise la variable d'environnement `NESS_BOOK` à l'exécution ou le chemin par défaut `BOOK_PATH`.

## Évaluation & Réseau NNUE attendu

Le moteur intègre une évaluation neuronale de type **Input-768 -> Hidden -> Output-1** avec une couche d'activation Clipped ReLU calculée en nombres entiers (`int16_t`).

⚠️ **ATTENTION :** Les réseaux officiels de Stockfish (architectures Half-KP à 41 024 entrées) **ne sont pas compatibles**.

*   **Comportement par défaut :** Sans fichier de poids, le Makefile lie `nnue_stub.c`. Tous les poids valent zéro, l'évaluation renvoie 0 et le moteur joue dénué d'évaluation intelligente.
*   **Spécifications techniques :** L'entrée doit faire strictement **768 index** (2 couleurs × 6 pièces × 64 cases), sans intégrer la position du Roi. Paramètres d'entraînement requis : `Scale = 400`, `QA = 127`, `QB = 64`.

### Procédure d'importation d'un réseau personnalisé :
Si vous entraînez un réseau compatible via un framework externe (ex: Marlinflow), convertissez le binaire `.nnue` avant de compiler.

Le script `tools/nnue_to_c.py` se charge déjà de definir `NNUE_HL=256` et `NNUE_INPUT=768` dans le fichier `mon_reseau.c`. Ainsi, pas besoin de surcharger ces macros à la compilation.

```bash
# 1. Conversion du binaire brut en tableau C (exemple pour 256 neurones cachés et 768 entrées)
python3 tools/nnue_to_c.py algorithm/nets/mon_reseau.nnue -o algorithm/src/mon_reseau.c --input 768 --hl 256

# 2. Recompilation du moteur avec le nouveau réseau
make NET=mon_reseau re
```

## Organisation du code

    types/        bitboard, échiquier, pile de coups (interchangeable dynamic/linked)
    rules/        génération et application des coups légaux
    algorithm/    recherche, table de transposition, répétition, Zobrist, NNUE
    uci/          protocole UCI, livre d'ouverture Polyglot
    app/          point d'entrée principal (main.c)
    tests/        suites de tests unitaires CUnit (42 tests, 714 assertions)

## Documentation technique

*   **Développement / PR :** Voir les instructions et la charte Git dans [CONTRIBUTING.md](CONTRIBUTING.md).
*   **Outils annexes :** Voir les documentations dédiées pour les benchmarks [[bench/README.md](bench/README.md)], les livres d'ouverture [[books/README.md](books/README.md)] et les tests [[tests/README.md](tests/README.md)].
*   **Doxygen :** Génération locale de l'API web via `cd doc && doxygen Doxyfile` (Ouvrir ensuite `doc/html/index.html`).

## Limites du moteur

* Pas de quiescence, ni de killers/historique.
* Pas de règle des 50 coups.
* Zobrist recalculé à chaque nœud.
* Recherche bloquante, sans stop ni lignes info.
* État global : pas de multithreading possible en l’état.

## Licence

Ce projet est distribué sous licence MIT. Voir le fichier `LICENSE` pour plus de détails.
