/**
 * \file test_apply_move.c
 *
 * \date 9 juil. 2026
 * \author Mathéo LECOMPAGNON
 */

#include "test_apply_move.h"
#include "../../types/headers/movelist.h"
#include "apply_move.h"
#include "bitboard.h"
#include "test_utils.h"
#include "type.h"
#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>

// Vrai si la case sq porte une pièce de type t et de couleur col sur cb
static int has_piece(chessboard cb, color col, piecetype t, cell sq) {
  return get_bit(get_piece_bitboard(cb, col, t), sq);
}

// Vrai si la case sq est vide pour toutes les couleurs et tous les types
static int is_empty_cell(chessboard cb, cell sq) {
  for (int col = WHITE; col <= BLACK; col++) {
    for (int t = PAWN; t <= KING; t++) {
      if (get_bit(get_piece_bitboard(cb, col, t), sq))
        return 0;
    }
  }
  return 1;
}

// --- Coup simple (MOVE_NORMAL) ---
void test_apply_normal(void) {
  chessboard cb = create_empty_chessboard();
  cb->chessboard_color = WHITE;
  place_piece(cb, KNIGHT, WHITE, 1); // cavalier b1

  move c = {.from = 1,
            .to = 18,
            .type = KNIGHT,
            .promotion = EMPTY,
            .kind = MOVE_NORMAL};
  apply_move(c, cb);

  CU_ASSERT_TRUE(is_empty_cell(cb, 1)); // la case de départ est vidée
  CU_ASSERT_TRUE(
      has_piece(cb, WHITE, KNIGHT, 18)); // le cavalier est à l'arrivée

  free_chessboard(cb);
}

// --- Capture (MOVE_CAPTURE) ---
void test_apply_capture(void) {
  chessboard cb = create_empty_chessboard();
  cb->chessboard_color = WHITE;
  place_piece(cb, ROOK, WHITE, 0);   // tour blanche a1
  place_piece(cb, KNIGHT, BLACK, 8); // cavalier noir a2 (pièce prise)

  move c = {.from = 0,
            .to = 8,
            .type = ROOK,
            .promotion = EMPTY,
            .kind = MOVE_CAPTURE};
  apply_move(c, cb);

  CU_ASSERT_TRUE(is_empty_cell(cb, 0));             // départ vidé
  CU_ASSERT_TRUE(has_piece(cb, WHITE, ROOK, 8));    // tour à l'arrivée
  CU_ASSERT_FALSE(has_piece(cb, BLACK, KNIGHT, 8)); // pièce adverse retirée

  free_chessboard(cb);
}

// --- Double poussée blanche + pose de la cible en passant ---
void test_apply_double_push_white(void) {
  chessboard cb = create_empty_chessboard();
  cb->chessboard_color = WHITE;
  place_piece(cb, PAWN, WHITE, 12); // pion e2

  move c = {.from = 12,
            .to = 28,
            .type = PAWN,
            .promotion = EMPTY,
            .kind = MOVE_DOUBLE_PUSH};
  apply_move(c, cb);

  CU_ASSERT_TRUE(is_empty_cell(cb, 12));
  CU_ASSERT_TRUE(has_piece(cb, WHITE, PAWN, 28)); // pion en e4
  CU_ASSERT_EQUAL(get_en_passant_target(cb), 20); // cible en passant = e3

  free_chessboard(cb);
}

// --- Double poussée noire + pose de la cible en passant ---
void test_apply_double_push_black(void) {
  chessboard cb = create_empty_chessboard();
  cb->chessboard_color = BLACK;
  place_piece(cb, PAWN, BLACK, 52); // pion e7

  move c = {.from = 52,
            .to = 36,
            .type = PAWN,
            .promotion = EMPTY,
            .kind = MOVE_DOUBLE_PUSH};
  apply_move(c, cb);

  CU_ASSERT_TRUE(is_empty_cell(cb, 52));
  CU_ASSERT_TRUE(has_piece(cb, BLACK, PAWN, 36)); // pion en e5
  CU_ASSERT_EQUAL(get_en_passant_target(cb), 44); // cible en passant = e6

  free_chessboard(cb);
}

// --- Prise en passant blanche ---
void test_apply_en_passant_white(void) {
  chessboard cb = create_empty_chessboard();
  cb->chessboard_color = WHITE;
  place_piece(cb, PAWN, WHITE, 36); // pion blanc e5
  place_piece(cb, PAWN, BLACK, 35); // pion noir d5 (vient de la double poussée)
  set_en_passant_target(cb, 43);    // cible d6

  move c = {.from = 36,
            .to = 43,
            .type = PAWN,
            .promotion = EMPTY,
            .kind = MOVE_EN_PASSANT};
  apply_move(c, cb);

  CU_ASSERT_TRUE(is_empty_cell(cb, 36));          // départ vidé
  CU_ASSERT_TRUE(has_piece(cb, WHITE, PAWN, 43)); // pion blanc en d6
  CU_ASSERT_FALSE(has_piece(
      cb, BLACK, PAWN, 35)); // pion noir capturé (pas sur la case d'arrivée)

  free_chessboard(cb);
}

// --- Prise en passant noire ---
void test_apply_en_passant_black(void) {
  chessboard cb = create_empty_chessboard();
  cb->chessboard_color = BLACK;
  place_piece(cb, PAWN, BLACK, 27); // pion noir d4
  place_piece(cb, PAWN, WHITE,
              28);               // pion blanc e4 (vient de la double poussée)
  set_en_passant_target(cb, 20); // cible e3

  move c = {.from = 27,
            .to = 20,
            .type = PAWN,
            .promotion = EMPTY,
            .kind = MOVE_EN_PASSANT};
  apply_move(c, cb);

  CU_ASSERT_TRUE(is_empty_cell(cb, 27));
  CU_ASSERT_TRUE(has_piece(cb, BLACK, PAWN, 20));  // pion noir en e3
  CU_ASSERT_FALSE(has_piece(cb, WHITE, PAWN, 28)); // pion blanc capturé

  free_chessboard(cb);
}

// --- Petit roque blanc ---
void test_apply_castle_king_white(void) {
  chessboard cb = create_empty_chessboard();
  cb->chessboard_color = WHITE;
  place_piece(cb, KING, WHITE, 4);
  place_piece(cb, ROOK, WHITE, 7);
  set_castling_right(cb, WHITE_KING_SIDE);

  move c = {.from = 4,
            .to = 6,
            .type = KING,
            .promotion = EMPTY,
            .kind = MOVE_CASTLE_KING};
  apply_move(c, cb);

  CU_ASSERT_TRUE(has_piece(cb, WHITE, KING, 6)); // roi en g1
  CU_ASSERT_TRUE(has_piece(cb, WHITE, ROOK, 5)); // tour en f1
  CU_ASSERT_TRUE(is_empty_cell(cb, 4));
  CU_ASSERT_TRUE(is_empty_cell(cb, 7));
  CU_ASSERT_FALSE(
      get_castling_right(cb, WHITE_KING_SIDE)); // droit perdu après le roque

  free_chessboard(cb);
}

// --- Grand roque blanc ---
void test_apply_castle_queen_white(void) {
  chessboard cb = create_empty_chessboard();
  cb->chessboard_color = WHITE;
  place_piece(cb, KING, WHITE, 4);
  place_piece(cb, ROOK, WHITE, 0);
  set_castling_right(cb, WHITE_QUEEN_SIDE);

  move c = {.from = 4,
            .to = 2,
            .type = KING,
            .promotion = EMPTY,
            .kind = MOVE_CASTLE_QUEEN};
  apply_move(c, cb);

  CU_ASSERT_TRUE(has_piece(cb, WHITE, KING, 2)); // roi en c1
  CU_ASSERT_TRUE(has_piece(cb, WHITE, ROOK, 3)); // tour en d1
  CU_ASSERT_TRUE(is_empty_cell(cb, 4));
  CU_ASSERT_TRUE(is_empty_cell(cb, 0));

  free_chessboard(cb);
}

// --- Petit roque noir ---
void test_apply_castle_king_black(void) {
  chessboard cb = create_empty_chessboard();
  cb->chessboard_color = BLACK;
  place_piece(cb, KING, BLACK, 60);
  place_piece(cb, ROOK, BLACK, 63);
  set_castling_right(cb, BLACK_KING_SIDE);

  move c = {.from = 60,
            .to = 62,
            .type = KING,
            .promotion = EMPTY,
            .kind = MOVE_CASTLE_KING};
  apply_move(c, cb);

  CU_ASSERT_TRUE(has_piece(cb, BLACK, KING, 62)); // roi en g8
  CU_ASSERT_TRUE(has_piece(cb, BLACK, ROOK, 61)); // tour en f8
  CU_ASSERT_TRUE(is_empty_cell(cb, 60));
  CU_ASSERT_TRUE(is_empty_cell(cb, 63));

  free_chessboard(cb);
}

// --- Grand roque noir ---
void test_apply_castle_queen_black(void) {
  chessboard cb = create_empty_chessboard();
  cb->chessboard_color = BLACK;
  place_piece(cb, KING, BLACK, 60);
  place_piece(cb, ROOK, BLACK, 56);
  set_castling_right(cb, BLACK_QUEEN_SIDE);

  move c = {.from = 60,
            .to = 58,
            .type = KING,
            .promotion = EMPTY,
            .kind = MOVE_CASTLE_QUEEN};
  apply_move(c, cb);

  CU_ASSERT_TRUE(has_piece(cb, BLACK, KING, 58)); // roi en c8
  CU_ASSERT_TRUE(has_piece(cb, BLACK, ROOK, 59)); // tour en d8
  CU_ASSERT_TRUE(is_empty_cell(cb, 60));
  CU_ASSERT_TRUE(is_empty_cell(cb, 56));

  free_chessboard(cb);
}

// --- Promotion par poussée ---
void test_apply_promotion_push(void) {
  chessboard cb = create_empty_chessboard();
  cb->chessboard_color = WHITE;
  place_piece(cb, PAWN, WHITE, 48); // pion a7

  move c = {.from = 48,
            .to = 56,
            .type = PAWN,
            .promotion = QUEEN,
            .kind = MOVE_NORMAL};
  apply_move(c, cb);

  CU_ASSERT_FALSE(has_piece(cb, WHITE, PAWN, 56)); // pas un pion sur a8
  CU_ASSERT_TRUE(has_piece(cb, WHITE, QUEEN, 56)); // mais une dame
  CU_ASSERT_TRUE(is_empty_cell(cb, 48));

  free_chessboard(cb);
}

// --- Promotion par prise ---
void test_apply_promotion_capture(void) {
  chessboard cb = create_empty_chessboard();
  cb->chessboard_color = WHITE;
  place_piece(cb, PAWN, WHITE, 49); // pion b7
  place_piece(cb, ROOK, BLACK, 56); // tour noire a8 (prise)

  move c = {.from = 49,
            .to = 56,
            .type = PAWN,
            .promotion = QUEEN,
            .kind = MOVE_CAPTURE};
  apply_move(c, cb);

  CU_ASSERT_TRUE(is_empty_cell(cb, 49));
  CU_ASSERT_FALSE(has_piece(cb, BLACK, ROOK, 56)); // tour capturée
  CU_ASSERT_TRUE(has_piece(cb, WHITE, QUEEN, 56)); // dame promue à l'arrivée

  free_chessboard(cb);
}

// --- Un coup qui n'est pas une double poussée efface la cible en passant ---
void test_apply_en_passant_reset(void) {
  chessboard cb = create_empty_chessboard();
  cb->chessboard_color = WHITE;
  place_piece(cb, KNIGHT, WHITE, 1);
  set_en_passant_target(cb, 20); // une cible existe avant le coup

  move c = {.from = 1,
            .to = 18,
            .type = KNIGHT,
            .promotion = EMPTY,
            .kind = MOVE_NORMAL};
  apply_move(c, cb);

  CU_ASSERT_EQUAL(get_en_passant_target(cb), -1); // remise à -1

  free_chessboard(cb);
}

// --- Un déplacement de roi retire les deux droits de roque de sa couleur ---
void test_apply_castling_rights_king(void) {
  chessboard cb = create_empty_chessboard();
  cb->chessboard_color = WHITE;
  place_piece(cb, KING, WHITE, 4);
  set_castling_right(cb, WHITE_QUEEN_SIDE);
  set_castling_right(cb, WHITE_KING_SIDE);
  set_castling_right(cb, BLACK_QUEEN_SIDE);
  set_castling_right(cb, BLACK_KING_SIDE);

  move c = {.from = 4,
            .to = 5,
            .type = KING,
            .promotion = EMPTY,
            .kind = MOVE_NORMAL};
  apply_move(c, cb);

  CU_ASSERT_FALSE(
      get_castling_right(cb, WHITE_QUEEN_SIDE)); // droits blancs perdus
  CU_ASSERT_FALSE(get_castling_right(cb, WHITE_KING_SIDE));
  CU_ASSERT_TRUE(
      get_castling_right(cb, BLACK_QUEEN_SIDE)); // droits noirs intacts
  CU_ASSERT_TRUE(get_castling_right(cb, BLACK_KING_SIDE));

  free_chessboard(cb);
}

// --- Un déplacement de tour retire le seul droit correspondant ---
void test_apply_castling_rights_rook(void) {
  chessboard cb = create_empty_chessboard();
  cb->chessboard_color = WHITE;
  place_piece(cb, ROOK, WHITE, 0); // tour a1
  set_castling_right(cb, WHITE_QUEEN_SIDE);
  set_castling_right(cb, WHITE_KING_SIDE);

  move c = {.from = 0,
            .to = 1,
            .type = ROOK,
            .promotion = EMPTY,
            .kind = MOVE_NORMAL};
  apply_move(c, cb);

  CU_ASSERT_FALSE(
      get_castling_right(cb, WHITE_QUEEN_SIDE)); // côté dame perdu (tour a1)
  CU_ASSERT_TRUE(get_castling_right(cb, WHITE_KING_SIDE)); // côté roi intact

  free_chessboard(cb);
}

// --- Aller-retour apply_move / undo_move : coup simple ---
// undo_move doit restaurer EXACTEMENT l'état d'avant apply_move (pièces, trait,
// droits, en passant).
void test_apply_undo_roundtrip(void) {
  chessboard cb = create_empty_chessboard();
  cb->chessboard_color = WHITE;
  place_piece(cb, KNIGHT, WHITE, 1);
  set_en_passant_target(cb, 20);           // état à restaurer par undo
  set_castling_right(cb, WHITE_KING_SIDE); // état à restaurer par undo

  move c = {.from = 1,
            .to = 18,
            .type = KNIGHT,
            .promotion = EMPTY,
            .kind = MOVE_NORMAL};
  undo_info u = apply_move(c, cb);

  // Après apply : le cavalier a bougé et le trait a changé
  CU_ASSERT_TRUE(has_piece(cb, WHITE, KNIGHT, 18));
  CU_ASSERT_TRUE(is_empty_cell(cb, 1));
  CU_ASSERT_EQUAL(get_chessboard_color(cb), BLACK);
  CU_ASSERT_EQUAL(get_en_passant_target(cb), -1); // effacée par le coup

  undo_move(c, cb, u);

  // Après undo : retour à l'état initial exact
  CU_ASSERT_TRUE(has_piece(cb, WHITE, KNIGHT, 1));
  CU_ASSERT_TRUE(is_empty_cell(cb, 18));
  CU_ASSERT_EQUAL(get_chessboard_color(cb), WHITE);
  CU_ASSERT_EQUAL(get_en_passant_target(cb), 20);
  CU_ASSERT_TRUE(get_castling_right(cb, WHITE_KING_SIDE));

  free_chessboard(cb);
}

// --- Aller-retour apply_move / undo_move : capture (la pièce prise doit
// revenir) ---
void test_undo_capture_roundtrip(void) {
  chessboard cb = create_empty_chessboard();
  cb->chessboard_color = WHITE;
  place_piece(cb, ROOK, WHITE, 0);   // tour blanche a1
  place_piece(cb, KNIGHT, BLACK, 8); // cavalier noir a2 (pièce prise)

  move c = {.from = 0,
            .to = 8,
            .type = ROOK,
            .promotion = EMPTY,
            .kind = MOVE_CAPTURE};
  undo_info u = apply_move(c, cb);
  undo_move(c, cb, u);

  // La tour est revenue en a1, le cavalier noir capturé est ressuscité en a2
  CU_ASSERT_TRUE(has_piece(cb, WHITE, ROOK, 0));
  CU_ASSERT_TRUE(has_piece(cb, BLACK, KNIGHT, 8));
  CU_ASSERT_FALSE(has_piece(cb, WHITE, ROOK, 8));
  CU_ASSERT_EQUAL(get_chessboard_color(cb), WHITE);

  free_chessboard(cb);
}
