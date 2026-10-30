/**
 * \file test_apply_move.h
 *
 * \date 9 juil. 2026
 * \author Mathéo LECOMPAGNON
 */

#ifndef __TEST_APPLY_MOVE_H__
#define __TEST_APPLY_MOVE_H__

void test_apply_normal(void);
void test_apply_capture(void);
void test_apply_double_push_white(void);
void test_apply_double_push_black(void);
void test_apply_en_passant_white(void);
void test_apply_en_passant_black(void);
void test_apply_castle_king_white(void);
void test_apply_castle_queen_white(void);
void test_apply_castle_king_black(void);
void test_apply_castle_queen_black(void);
void test_apply_promotion_push(void);
void test_apply_promotion_capture(void);
void test_apply_en_passant_reset(void);
void test_apply_castling_rights_king(void);
void test_apply_castling_rights_rook(void);
void test_apply_undo_roundtrip(void);
void test_undo_capture_roundtrip(void);

#endif
