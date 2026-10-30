/**
 * \file test_movelist.c
 *
 * \date 9 juil. 2026
 * \author Mathéo LECOMPAGNON
 */

#include "test_movelist.h"
#include "../../types/headers/movelist.h"
#include "type.h"
#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>

// Compare deux coups champ à champ
static int move_equal(move a, move b) {
  return a.from == b.from && a.to == b.to && a.type == b.type &&
         a.promotion == b.promotion && a.kind == b.kind;
}

// --- Tests pour create_movelist ---
void test_create_movelist(void) {
  movelist m = create_movelist();

  CU_ASSERT_PTR_NOT_NULL(m);
  CU_ASSERT_TRUE(is_empty_movelist(m)); // une pile fraîche est vide
  CU_ASSERT_EQUAL(length(m), 0);

  free_movelist(m);
}

// --- Tests pour push_movelist / pop_movelist ---
void test_push_pop_movelist(void) {
  movelist m = create_movelist();
  move a = {.from = 1,
            .to = 2,
            .type = PAWN,
            .promotion = EMPTY,
            .kind = MOVE_NORMAL};

  push_movelist(a, m);
  CU_ASSERT_FALSE(is_empty_movelist(m));
  CU_ASSERT_EQUAL(length(m), 1);

  move out = pop_movelist(m);
  CU_ASSERT_TRUE(move_equal(out, a)); // on récupère bien le coup empilé
  CU_ASSERT_TRUE(is_empty_movelist(m));
  CU_ASSERT_EQUAL(length(m), 0);

  free_movelist(m);
}

// --- Tests de l'ordre LIFO ---
void test_lifo_order(void) {
  movelist m = create_movelist();
  move a = {.from = 1,
            .to = 2,
            .type = PAWN,
            .promotion = EMPTY,
            .kind = MOVE_NORMAL};
  move b = {.from = 3,
            .to = 4,
            .type = KNIGHT,
            .promotion = EMPTY,
            .kind = MOVE_NORMAL};
  move c = {.from = 5,
            .to = 6,
            .type = BISHOP,
            .promotion = EMPTY,
            .kind = MOVE_CAPTURE};

  push_movelist(a, m);
  push_movelist(b, m);
  push_movelist(c, m);
  CU_ASSERT_EQUAL(length(m), 3);

  // Pile : dernier empilé, premier dépilé -> c, b, a
  CU_ASSERT_TRUE(move_equal(pop_movelist(m), c));
  CU_ASSERT_TRUE(move_equal(pop_movelist(m), b));
  CU_ASSERT_TRUE(move_equal(pop_movelist(m), a));
  CU_ASSERT_TRUE(is_empty_movelist(m));

  free_movelist(m);
}

// --- Tests pour get_next / get_current_move ---
void test_get_next_current(void) {
  movelist m = create_movelist();
  move a = {.from = 1,
            .to = 2,
            .type = PAWN,
            .promotion = EMPTY,
            .kind = MOVE_NORMAL};

  push_movelist(a, m);

  // get_next(sentinelle) donne le premier vrai noeud, dont get_current_move
  // rend le coup
  movelist top = get_next(m);
  CU_ASSERT_PTR_NOT_NULL(top);
  CU_ASSERT_TRUE(move_equal(get_current_move(top), a));

  // Un seul coup empilé : rien après le sommet
  CU_ASSERT_PTR_NULL(get_next(top));

  free_movelist(m);
}

// --- Test du parcours complet et de length() le long du parcours ---
// Ce test verrouille la sémantique de length() sur un NOEUD (nombre de coups
// qui le SUIVENT, lui non compris). Les deux implémentations de l'interface
// opaque doivent être indiscernables ici : dynamic_movelist.c a déjà divergé de
// 1 sur ce point. À relancer avec les deux : make test puis make
// MOVELIST=linked test_re.
void test_traversal_length(void) {
  movelist m = create_movelist();
  move a = {.from = 1,
            .to = 2,
            .type = PAWN,
            .promotion = EMPTY,
            .kind = MOVE_NORMAL};
  move b = {.from = 3,
            .to = 4,
            .type = KNIGHT,
            .promotion = EMPTY,
            .kind = MOVE_NORMAL};
  move c = {.from = 5,
            .to = 6,
            .type = BISHOP,
            .promotion = EMPTY,
            .kind = MOVE_CAPTURE};

  push_movelist(a, m);
  push_movelist(b, m);
  push_movelist(c, m);

  // Sur l'entête : le nombre de coups empilés.
  CU_ASSERT_EQUAL(length(m), 3);

  // Parcours du sommet vers le fond : c, puis b, puis a (ordre LIFO).
  movelist n0 = get_next(m);
  CU_ASSERT_PTR_NOT_NULL_FATAL(n0);
  CU_ASSERT_TRUE(move_equal(get_current_move(n0), c));
  CU_ASSERT_EQUAL(length(n0), 2); // b et a le suivent
  CU_ASSERT_FALSE(is_empty_movelist(n0));

  movelist n1 = get_next(n0);
  CU_ASSERT_PTR_NOT_NULL_FATAL(n1);
  CU_ASSERT_TRUE(move_equal(get_current_move(n1), b));
  CU_ASSERT_EQUAL(length(n1), 1); // a le suit
  CU_ASSERT_FALSE(is_empty_movelist(n1));

  movelist n2 = get_next(n1);
  CU_ASSERT_PTR_NOT_NULL_FATAL(n2);
  CU_ASSERT_TRUE(move_equal(get_current_move(n2), a));
  CU_ASSERT_EQUAL(length(n2), 0); // dernier noeud : plus rien après
  CU_ASSERT_TRUE(is_empty_movelist(n2));

  CU_ASSERT_PTR_NULL(get_next(n2));

  // Le parcours ne consomme pas la pile.
  CU_ASSERT_EQUAL(length(m), 3);

  free_movelist(m);
}
