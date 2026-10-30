/**
 * \file test_type.c
 *
 * \date 9 juil. 2026
 * \author Mathéo LECOMPAGNON
 */

#include "test_type.h"
#include "bitboard.h"
#include "type.h"
#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>

// --- Tests pour create_chessboard ---
void test_create_chessboard(void) {
  chessboard cb = create_chessboard();

  CU_ASSERT_PTR_NOT_NULL(cb);
  CU_ASSERT_EQUAL(get_chessboard_color(cb), WHITE);
  CU_ASSERT_EQUAL(get_en_passant_target(cb), -1); // pas de cible en passant

  // Tous les bitboards des pièces sont vides
  for (int c = WHITE; c <= BLACK; c++) {
    for (int t = PAWN; t <= KING; t++) {
      CU_ASSERT_EQUAL(get_piece_bitboard(cb, c, t), 0ULL);
    }
  }

  // Aucun droit de roque au départ
  CU_ASSERT_FALSE(get_castling_right(cb, WHITE_QUEEN_SIDE));
  CU_ASSERT_FALSE(get_castling_right(cb, WHITE_KING_SIDE));
  CU_ASSERT_FALSE(get_castling_right(cb, BLACK_QUEEN_SIDE));
  CU_ASSERT_FALSE(get_castling_right(cb, BLACK_KING_SIDE));

  free_chessboard(cb);
}

// --- Tests pour get_piece_bitboard / get_piece_bitboard_ref /
// get_color_bitboard ---
void test_piece_bitboard(void) {
  chessboard cb = create_chessboard();

  // On écrit via la référence, on relit via la lecture
  set_bit(get_piece_bitboard_ref(cb, WHITE, PAWN), 8);
  set_bit(get_piece_bitboard_ref(cb, WHITE, PAWN), 9);
  CU_ASSERT_TRUE(get_bit(get_piece_bitboard(cb, WHITE, PAWN), 8));
  CU_ASSERT_TRUE(get_bit(get_piece_bitboard(cb, WHITE, PAWN), 9));

  // get_color_bitboard = union de tous les types d'une couleur
  set_bit(get_piece_bitboard_ref(cb, WHITE, KING), 4);
  bitboard white = get_color_bitboard(cb, WHITE);
  CU_ASSERT_TRUE(get_bit(white, 8));
  CU_ASSERT_TRUE(get_bit(white, 9));
  CU_ASSERT_TRUE(get_bit(white, 4));

  // Les noirs n'ont aucune pièce
  CU_ASSERT_EQUAL(get_color_bitboard(cb, BLACK), 0ULL);

  free_chessboard(cb);
}

// --- Tests pour les droits de roque ---
void test_castling_rights(void) {
  chessboard cb = create_chessboard();

  CU_ASSERT_FALSE(get_castling_right(cb, WHITE_KING_SIDE));

  set_castling_right(cb, WHITE_KING_SIDE);
  CU_ASSERT_TRUE(get_castling_right(cb, WHITE_KING_SIDE));
  // Poser un droit n'affecte pas les autres
  CU_ASSERT_FALSE(get_castling_right(cb, WHITE_QUEEN_SIDE));
  CU_ASSERT_FALSE(get_castling_right(cb, BLACK_KING_SIDE));

  remove_castling_right(cb, WHITE_KING_SIDE);
  CU_ASSERT_FALSE(get_castling_right(cb, WHITE_KING_SIDE));

  free_chessboard(cb);
}

// --- Tests pour la cible en passant ---
void test_en_passant_target(void) {
  chessboard cb = create_chessboard();

  CU_ASSERT_EQUAL(get_en_passant_target(cb), -1);

  set_en_passant_target(cb, 20);
  CU_ASSERT_EQUAL(get_en_passant_target(cb), 20);

  set_en_passant_target(cb, -1);
  CU_ASSERT_EQUAL(get_en_passant_target(cb), -1);

  free_chessboard(cb);
}

// --- Tests pour get_piece_type / get_piece_color ---
void test_piece_accessors(void) {
  piece p = {.type = QUEEN, .piece_color = BLACK};
  CU_ASSERT_EQUAL(get_piece_type(p), QUEEN);
  CU_ASSERT_EQUAL(get_piece_color(p), BLACK);
}
