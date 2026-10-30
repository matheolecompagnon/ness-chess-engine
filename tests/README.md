# Tests

Tests unitaires du moteur, écrits avec [CUnit](http://cunit.sourceforge.net/). Un seul exécutable, `test_chess`, regroupe 5 suites (`Test_Regles_Echecs`, `Test_Bitboard`, `Test_Movelist`, `Test_Type`, `Test_Apply_Move`) et renvoie le nombre d'échecs en code de sortie.

## Arborescence

    tests
    ├── headers
    │   ├── test_apply_move.h
    │   ├── test_bitboard.h
    │   ├── test_main.h
    │   ├── test_movelist.h
    │   ├── test_rule.h
    │   ├── test_type.h
    │   └── test_utils.h
    └── src
        ├── test_apply_move.c
        ├── test_bitboard.c
        ├── test_main.c
        ├── test_movelist.c
        ├── test_rule.c
        ├── test_type.c
        └── test_utils.c

## Contenu des fichiers

| Fichier | Tests |
|---|---|
| `test_main.c` | Point d'entrée : enregistre les 5 suites CUnit, lance les tests en mode verbeux et retourne le nombre d'échecs. |
| `test_utils.c` | Outils partagés (pas de test) : échiquier vide, pose de pièces, recherche d'un coup dans une `movelist`, comptage des coups. |
| `test_bitboard.c` | `set_bit`, `clear_bit` et `get_bit` : bits 0 et 63, idempotence, isolement des cases. |
| `test_movelist.c` | Pile de coups : création, push/pop, ordre LIFO, parcours (`get_next`, `get_current_move`) et sémantique de `length()` sur un noeud. |
| `test_type.c` | `chessboard` : état initial, bitboards par pièce et par couleur, droits de roque, cible en passant, accesseurs de `piece`. |
| `test_rule.c` | Génération des coups pseudo-légaux de chaque pièce (cavalier, pion blanc et noir, fou, tour, roi, roque, dame), bords de l'échiquier, blocages, prises, en passant, promotions, légalité sous échec et clouages, champ `kind` des coups, et `generate_move` sur positions complètes (20 coups au départ, etc.). |
| `test_apply_move.c` | `apply_move` : coup simple, prise, double poussée, en passant, roques, promotions, mise à jour des droits de roque et de la cible en passant, aller-retour `apply_move` / `undo_move`. |

## Ce que ces tests ne couvrent pas

Aucun test unitaire ici pour la recherche (`negamax`), la table de transposition,
la répétition, le Zobrist, le NNUE ni le parsing UCI. L'auto-test du moteur
(`./chess --selftest`) couvre le perft, la cohérence de l'accumulateur NNUE,
la restauration du board et une recherche de profondeur 4.

## Dépendances

- `gcc` (C11) et `make`
- CUnit, lié avec `-lcunit`
- Les tests lient aussi les sources du moteur (`LIB_SRC`), y compris le réseau NNUE. Sans fichier de poids, le Makefile lie `algorithm/src/nnue_stub.c`.

### Installation de CUnit

Debian / Ubuntu :

    sudo apt install build-essential libcunit1-dev

macOS (Homebrew) :

    brew install cunit

## Lancer les tests

Depuis la racine du projet :

    make test        # compile test_chess et l'exécute
    make test_re     # reconstruction complète puis tests

Les deux implémentations de la pile de coups doivent donner les mêmes résultats (`test_traversal_length` a été écrit pour cela) :

    make test
    make MOVELIST=linked test_re

## Résultat des tests

```bash
Suite: Test_Regles_Echecs
  Test: Test is_available_index ...passed
  Test: Test generate_knight_move ...passed
  Test: Test generate_pawn_move ...passed
  Test: Test generate_pawn_move_black ...passed
  Test: Test generate_bishop_move ...passed
  Test: Test generate_rook_move ...passed
  Test: Test generate_king_move ...passed
  Test: Test generate_king_castling ...passed
  Test: Test generate_queen_move ...passed
  Test: Test legal_moves ...passed
  Test: Test move_metadata ...passed
  Test: Test generate_move ...passed
Suite: Test_Bitboard
  Test: Test set_bit ...passed
  Test: Test clear_bit ...passed
  Test: Test get_bit ...passed
Suite: Test_Movelist
  Test: Test create_movelist ...passed
  Test: Test push_pop_movelist ...passed
  Test: Test lifo_order ...passed
  Test: Test get_next_current ...passed
  Test: Test traversal_length ...passed
Suite: Test_Type
  Test: Test create_chessboard ...passed
  Test: Test piece_bitboard ...passed
  Test: Test castling_rights ...passed
  Test: Test en_passant_target ...passed
  Test: Test piece_accessors ...passed
Suite: Test_Apply_Move
  Test: Test apply_normal ...passed
  Test: Test apply_capture ...passed
  Test: Test apply_double_push_white ...passed
  Test: Test apply_double_push_black ...passed
  Test: Test apply_en_passant_white ...passed
  Test: Test apply_en_passant_black ...passed
  Test: Test apply_castle_king_white ...passed
  Test: Test apply_castle_queen_white ...passed
  Test: Test apply_castle_king_black ...passed
  Test: Test apply_castle_queen_black ...passed
  Test: Test apply_promotion_push ...passed
  Test: Test apply_promotion_capture ...passed
  Test: Test apply_en_passant_reset ...passed
  Test: Test apply_castling_rights_king ...passed
  Test: Test apply_castling_rights_rook ...passed
  Test: Test apply_undo_roundtrip ...passed
  Test: Test undo_capture_roundtrip ...passed

Run Summary:    Type  Total    Ran Passed Failed Inactive
              suites      5      5    n/a      0        0
               tests     42     42     42      0        0
             asserts    714    714    714      0      n/a
```
