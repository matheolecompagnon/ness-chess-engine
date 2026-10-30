/**
 * \file test_rule.h
 *
 * \date 2 juil. 2026
 * \author Mathéo LECOMPAGNON
 */

#include "test_main.h"
#include "test_apply_move.h"
#include "test_bitboard.h"
#include "test_movelist.h"
#include "test_type.h"
#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>
#include <stdio.h>

// Déclarations des fonctions de test
void test_is_available_index(void);
void test_generate_knight_move(void);
void test_generate_pawn_move(void);
void test_generate_pawn_move_black(void);
void test_generate_bishop_move(void);
void test_generate_rook_move(void);
void test_generate_king_move(void);
void test_generate_king_castling(void);
void test_generate_queen_move(void);
void test_legal_moves(void);
void test_move_metadata(void);
void test_generate_move(void);

int test_main() {
  CU_pSuite suite = NULL;

  // Initialisation de CUnit
  if (CUE_SUCCESS != CU_initialize_registry()) {
    return CU_get_error();
  }

  // Ajout de la suite de tests pour les règles
  suite = CU_add_suite("Test_Regles_Echecs", NULL, NULL);
  if (NULL == suite) {
    CU_cleanup_registry();
    return CU_get_error();
  }

  // Ajout des tests
  if ((NULL == CU_add_test(suite, "Test is_available_index",
                           test_is_available_index)) ||
      (NULL == CU_add_test(suite, "Test generate_knight_move",
                           test_generate_knight_move)) ||
      (NULL == CU_add_test(suite, "Test generate_pawn_move",
                           test_generate_pawn_move)) ||
      (NULL == CU_add_test(suite, "Test generate_pawn_move_black",
                           test_generate_pawn_move_black)) ||
      (NULL == CU_add_test(suite, "Test generate_bishop_move",
                           test_generate_bishop_move)) ||
      (NULL == CU_add_test(suite, "Test generate_rook_move",
                           test_generate_rook_move)) ||
      (NULL == CU_add_test(suite, "Test generate_king_move",
                           test_generate_king_move)) ||
      (NULL == CU_add_test(suite, "Test generate_king_castling",
                           test_generate_king_castling)) ||
      (NULL == CU_add_test(suite, "Test generate_queen_move",
                           test_generate_queen_move)) ||
      (NULL == CU_add_test(suite, "Test legal_moves", test_legal_moves)) ||
      (NULL == CU_add_test(suite, "Test move_metadata", test_move_metadata)) ||
      (NULL == CU_add_test(suite, "Test generate_move", test_generate_move))) {
    CU_cleanup_registry();
    return CU_get_error();
  }

  // Ajout de la suite de tests pour le bitboard
  CU_pSuite suite_bitboard = CU_add_suite("Test_Bitboard", NULL, NULL);
  if (NULL == suite_bitboard) {
    CU_cleanup_registry();
    return CU_get_error();
  }
  if ((NULL == CU_add_test(suite_bitboard, "Test set_bit", test_set_bit)) ||
      (NULL == CU_add_test(suite_bitboard, "Test clear_bit", test_clear_bit)) ||
      (NULL == CU_add_test(suite_bitboard, "Test get_bit", test_get_bit))) {
    CU_cleanup_registry();
    return CU_get_error();
  }

  // Ajout de la suite de tests pour la movelist
  CU_pSuite suite_movelist = CU_add_suite("Test_Movelist", NULL, NULL);
  if (NULL == suite_movelist) {
    CU_cleanup_registry();
    return CU_get_error();
  }
  if ((NULL == CU_add_test(suite_movelist, "Test create_movelist",
                           test_create_movelist)) ||
      (NULL == CU_add_test(suite_movelist, "Test push_pop_movelist",
                           test_push_pop_movelist)) ||
      (NULL ==
       CU_add_test(suite_movelist, "Test lifo_order", test_lifo_order)) ||
      (NULL == CU_add_test(suite_movelist, "Test get_next_current",
                           test_get_next_current)) ||
      (NULL == CU_add_test(suite_movelist, "Test traversal_length",
                           test_traversal_length))) {
    CU_cleanup_registry();
    return CU_get_error();
  }

  // Ajout de la suite de tests pour le type (chessboard)
  CU_pSuite suite_type = CU_add_suite("Test_Type", NULL, NULL);
  if (NULL == suite_type) {
    CU_cleanup_registry();
    return CU_get_error();
  }
  if ((NULL == CU_add_test(suite_type, "Test create_chessboard",
                           test_create_chessboard)) ||
      (NULL ==
       CU_add_test(suite_type, "Test piece_bitboard", test_piece_bitboard)) ||
      (NULL ==
       CU_add_test(suite_type, "Test castling_rights", test_castling_rights)) ||
      (NULL == CU_add_test(suite_type, "Test en_passant_target",
                           test_en_passant_target)) ||
      (NULL ==
       CU_add_test(suite_type, "Test piece_accessors", test_piece_accessors))) {
    CU_cleanup_registry();
    return CU_get_error();
  }

  // Ajout de la suite de tests pour apply_move
  CU_pSuite suite_apply = CU_add_suite("Test_Apply_Move", NULL, NULL);
  if (NULL == suite_apply) {
    CU_cleanup_registry();
    return CU_get_error();
  }
  if ((NULL ==
       CU_add_test(suite_apply, "Test apply_normal", test_apply_normal)) ||
      (NULL ==
       CU_add_test(suite_apply, "Test apply_capture", test_apply_capture)) ||
      (NULL == CU_add_test(suite_apply, "Test apply_double_push_white",
                           test_apply_double_push_white)) ||
      (NULL == CU_add_test(suite_apply, "Test apply_double_push_black",
                           test_apply_double_push_black)) ||
      (NULL == CU_add_test(suite_apply, "Test apply_en_passant_white",
                           test_apply_en_passant_white)) ||
      (NULL == CU_add_test(suite_apply, "Test apply_en_passant_black",
                           test_apply_en_passant_black)) ||
      (NULL == CU_add_test(suite_apply, "Test apply_castle_king_white",
                           test_apply_castle_king_white)) ||
      (NULL == CU_add_test(suite_apply, "Test apply_castle_queen_white",
                           test_apply_castle_queen_white)) ||
      (NULL == CU_add_test(suite_apply, "Test apply_castle_king_black",
                           test_apply_castle_king_black)) ||
      (NULL == CU_add_test(suite_apply, "Test apply_castle_queen_black",
                           test_apply_castle_queen_black)) ||
      (NULL == CU_add_test(suite_apply, "Test apply_promotion_push",
                           test_apply_promotion_push)) ||
      (NULL == CU_add_test(suite_apply, "Test apply_promotion_capture",
                           test_apply_promotion_capture)) ||
      (NULL == CU_add_test(suite_apply, "Test apply_en_passant_reset",
                           test_apply_en_passant_reset)) ||
      (NULL == CU_add_test(suite_apply, "Test apply_castling_rights_king",
                           test_apply_castling_rights_king)) ||
      (NULL == CU_add_test(suite_apply, "Test apply_castling_rights_rook",
                           test_apply_castling_rights_rook)) ||
      (NULL == CU_add_test(suite_apply, "Test apply_undo_roundtrip",
                           test_apply_undo_roundtrip)) ||
      (NULL == CU_add_test(suite_apply, "Test undo_capture_roundtrip",
                           test_undo_capture_roundtrip))) {
    CU_cleanup_registry();
    return CU_get_error();
  }

  // Exécution des tests
  CU_basic_set_mode(CU_BRM_VERBOSE);
  CU_basic_run_tests();
  int errors = CU_get_number_of_failures();
  CU_cleanup_registry(); // Free toutes les suites de tests

  return errors;
}

int main() { return test_main(); }
