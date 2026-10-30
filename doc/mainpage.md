# Moteur d'échecs NNUE - NESS

## Introduction

Ceci est la documentation technique du moteur d'échecs **NESS**, développé en C11. Le projet évalue la meilleure position possible sur un échiquier grâce à l'inférence et à la mise à jour incrémentale d'un réseau de neurones efficacement mis à jour (NNUE) embarqué. Il implémente le protocole standard UCI afin de s'interfacer de manière transparente avec des interfaces graphiques ou des outils d'arbitrage externes.

Ce projet englobe la génération complète de coups légaux, des algorithmes de recherche avancés sous contrainte de temps, une gestion de table de transposition, ainsi qu'un harnais complet de tests et de benchmarks pour valider la non-régression et mesurer la force du moteur.

## Choix d'implémentations

*   **Représentation (Bitboards) :** Modélisation de l'échiquier via 2 × 6 bitboards (entiers 64 bits non signés) représentant chaque type de pièce par couleur. L'application et l'annulation des coups s'effectuent "en place" (*make/unmake*) sur l'échiquier afin d'éviter les copies lourdes en mémoire lors de l'exploration de l'arbre de recherche.
*   **Algorithme de Recherche :** Architecture Negamax avec approfondissement itératif borné par le budget de temps. Elle intègre un élagage alpha-bêta, une table de transposition (clés de Zobrist), le *null-move pruning*, la recherche de mouvements principaux (PVS), les réductions de coups tardifs (LMR), et la détection de répétition (pénalité de nulle de 25 centipions).
*   **Indépendance logicielle :** Le moteur est autonome et n'embarque aucune dépendance hors de la bibliothèque standard C (CUnit est requis uniquement pour l'exécution des tests unitaires), maximisant ainsi sa portabilité et facilitant un futur portage vers du calcul haute performance (HPC) ou FPGA.

## Organisation des Modules et Rôle des Fichiers

L'arborescence sépare rigoureusement les interfaces (`headers/`) et les implémentations (`src/`).

### 1. Types et Structures de données (types/)

*   `bitboard.h` : Définition des bitboards de 64 bits, manipulation de bits et détection de la première case occupée.
*   `type.h` : Structure principale `chessboard` regroupant les bitboards par couleur/pièce, le trait, les droits de roque, la cible en passant, et l'historique des accumulateurs NNUE.
*   `movelist.h` : Interface opaque d'une pile de coups. Deux implémentations interchangeables cohabitent : `dynamic_movelist.c` (par tableau dynamique, comportement par défaut) et `linked_movelist.c` (par liste chaînée).
    *   *⚠️ Attention technique :* Avec l'implémentation dynamique, un appel à `push_movelist()` peut réallouer le tableau et invalider les pointeurs de noeuds obtenus avant lui par `get_next()`. Il ne faut jamais empiler dans une pile en cours de parcours.
*   `nnue_type.h` : Définition des types et des dimensions du réseau (`network`, `accumulator`). Les dimensions sont figées par des macros de compilation pour des raisons de performance.

### 2. Règles du jeu (rules/)

*   `rule_iter.h` : `generate_move()` produit l'ensemble des coups légaux du jeu d'échecs en filtrant les coups pseudo-légaux qui laisseraient le roi en échec. Les fonctions de génération par pièce sont exposées pour les tests.
*   `apply_move.h` : Fonctions `apply_move()` et `undo_move()` appliquant et annulant un coup en place, tout en déclenchant la mise à jour incrémentale différentielle des accumulateurs NNUE.

### 3. Algorithmes de recherche et Évaluation (algorithm/)

*   `negamax.h` : Recherche negamax alpha-bêta, approfondissement itératif et gestion fine du budget de temps.
*   `nnue.h` : Inférence du réseau via `evaluate()` et gestion de la mise à jour complète (`nnue_refresh_()`).
*   `zobrist.h` : Génération des clés de hachage uniques pour l'identification d'une position.
*   `repetition.h` : Gestionnaire de l'historique des positions pour appliquer la règle de nulle par répétition.
*   `tt.h` : Table de transposition mémorisant les résultats des noeuds visités (2^`TT_BITS` entrées de 32 octets, 32 Mo par défaut) pour éviter les recalculs. Elle est vidée à l'initialisation par `ucinewgame`.

### 4. Couches applicatives, Tests et Outils

*   `uci/` (`uci.h`, `book.c`) : Boucle de communication UCI (`uci_loop()`), décodage des commandes textuelles (`position`, `go`) et lecteur optionnel de livre d'ouverture Polyglot.
*   `app/` (`main.c`) : Point d'entrée de l'exécutable.
*   `tools/` (`nnue_to_c.py`) : Script utilitaire pour formater et exporter un réseau de neurones entraîné vers un fichier source C.
*   `tests/` & `bench/` : Suites de tests CUnit et fichiers d'instrumentation.

## Guide de Compilation et Commandes Utiles

Les avertissements du compilateur sont traités comme des erreurs (`-Wall -Wextra -Werror -std=c11`). Toute modification d'un fichier d'en-tête `.h` ou changement de variable de build impose une reconstruction complète (`make re`).

```bash
# --- Compilation standard ---
make                                            # Produit le binaire ./chess (Moteur UCI)
make re                                         # Force une reconstruction complète

# --- Validation et Tests ---
make test_re                                    # Lance les tests unitaires CUnit (Pile dynamique)
make MOVELIST=linked test_re                    # Lance les tests unitaires avec la pile chaînée
./chess --selftest                              # Auto-test complet (Perft 1-5, cohérence NNUE, recherche depth 4)

# --- Métriques et Profiling ---
make bench                                      # Construit bench_ness avec le compteur de noeuds activé
python3 bench/compare_engines.py --a ./moteur1 --b ./moteur2 --games 100  # Arbitrage et comparaison de force
```

### Optimisations et Macros de Diagnostic (NNUE_DEFS / OPT)

Par défaut, le moteur applique `-O3 -march=native -flto`. L'optimisation à l'édition de liens (`-flto`) est obligatoire à la compilation et au lien pour permettre l'inlinage des fonctions critiques (`get_bit`, `evaluate`) à travers les frontières des modules.

Pour mesurer l'apport d'un algorithme ou reproduire des états antérieurs, vous pouvez injecter des macros de désactivation lors du build via la variable `NNUE_DEFS` ou `OPT` :
*   `-DNESS_DISABLE_TT` : Neutralise la sonde et le stockage dans la table de transposition.
*   `-DNESS_DISABLE_NULLMOVE` : Désactive le *null-move pruning*.
*   `-DNESS_DISABLE_PVS` : Désactive la recherche *Principal Variation Search*.
*   `-DNESS_DISABLE_LMR` : Désactive les réductions de coups tardifs (*Late Move Reductions*).
*   `-DNESS_DISABLE_FOLD` : Désactive la fusion du niveau `depth == 1`.
*   `-DNESS_ORDER_REVERSED` : Inverse le tri des coups pour stresser l'alpha-bêta.

*Note : Ces macros ne doivent pas être utilisées sur un binaire de production.*

## Directives pour les Développeurs (CI / Pull Requests)

Avant de soumettre une contribution ou d'ouvrir une demande de fusion, assurez-vous de respecter les critères de validation suivants :
1.  **Tests unitaires :** Les cibles `make test_re` et `make MOVELIST=linked test_re` doivent passer sans erreur.
2.  **Stabilité :** L'auto-test global `./chess --selftest` doit retourner 0.
3.  **Conformité UCI :** Le flux de commandes de base `uci -> isready -> position -> go depth 4 -> quit` doit répondre correctement (`uciok`, `readyok`, `bestmove`).
4.  **Règles de recherche :** Tout changement dans les algorithmes de recherche doit être accompagné de mesures comparatives du nombre de noeuds via `make bench`.
5.  **Documentation :** Chaque nouvelle fonction doit être documentée au format Doxygen (`\brief`, `\param`, `\return`).
6.  **Sécurité des données :** Ne jamais pousser de fichiers de poids réels (`.nnue` ou `embedded_nnue_*.c`) sur le dépôt distant. Ils sont protégés par le fichier `.gitignore`. Par défaut, le moteur se lie à `nnue_stub.c` (poids à zéro, évaluation neutre).

## Références

*   **Architecture du réseau NNUE :** La conception et les équations de mise à jour s'appuient sur la documentation du *Chess Programming Wiki* : <https://www.chessprogramming.org>
*   **Validation Perft :** Les valeurs de noeuds de référence pour le débogage du générateur de coups proviennent de : <https://www.chessprogramming.org>
