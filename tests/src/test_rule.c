/**
 * \file test_rule.h
 *
 * \date 2 juil. 2026
 * \author Mathéo LECOMPAGNON
 */

#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>

#include "../../rules/headers/rule_iter.h"
#include "../../types/headers/movelist.h"
#include "test_utils.h"
#include "type.h"

// --- Tests pour is_available_index ---
void test_is_available_index(void) {
  chessboard cb = create_empty_chessboard();
  cb->chessboard_color = WHITE;

  // Test des index invalides (hors échiquier)
  CU_ASSERT_FALSE(is_available_index(-1, cb));
  CU_ASSERT_FALSE(is_available_index(64, cb));

  // Test des index valides (cases vides)
  CU_ASSERT_TRUE(is_available_index(0, cb));
  CU_ASSERT_TRUE(is_available_index(63, cb));

  // Test des index occupés par le joueur actif
  for (int t = PAWN; t <= KING; t++) {
    place_piece(cb, t, WHITE, 10);
    CU_ASSERT_FALSE(
        is_available_index(10, cb)); // Case occupée par le joueur actif
  }

  // Test des index occupés par l'adversaire
  for (int t = PAWN; t <= KING; t++) {
    place_piece(cb, t, BLACK, 20);
    CU_ASSERT_TRUE(is_available_index(20, cb)); // Case occupée par l'adversaire
  }

  free_chessboard(cb);
}

// --- Tests pour generate_knight_move ---
void test_generate_knight_move(void) {

  /*
   * ==========================
   * TEST MILIEU : D4 (index 27)
   * ==========================
   */
  chessboard cb = create_empty_chessboard();
  cb->chessboard_color = WHITE;

  // On place le cavalier
  place_piece(cb, KNIGHT, WHITE, 27);

  movelist m = create_movelist();
  generate_knight_move(cb, m);

  // Un cavalier en d4 (index 27) a 8 coups légaux sur un échiquier vide aux
  // index 10, 12, 17, 21, 33, 37, 42 et 44
  CU_ASSERT_EQUAL(count_moves(m), 8);

  move expected_moves1[] = {
      {.from = 27, .to = 10, .type = KNIGHT, .promotion = EMPTY},
      {.from = 27, .to = 12, .type = KNIGHT, .promotion = EMPTY},
      {.from = 27, .to = 17, .type = KNIGHT, .promotion = EMPTY},
      {.from = 27, .to = 21, .type = KNIGHT, .promotion = EMPTY},
      {.from = 27, .to = 33, .type = KNIGHT, .promotion = EMPTY},
      {.from = 27, .to = 37, .type = KNIGHT, .promotion = EMPTY},
      {.from = 27, .to = 42, .type = KNIGHT, .promotion = EMPTY},
      {.from = 27, .to = 44, .type = KNIGHT, .promotion = EMPTY}};

  // TESTS
  for (int i = 0; i < 8; i++) {
    CU_ASSERT_TRUE(move_exists(m, expected_moves1[i]));
  }

  // Free
  free_movelist(m);
  free_chessboard(cb);

  /*
   * ========================================
   * TEST COINS STRICTS SEUL (A1, A8, H1, H8)
   * ========================================
   */
  cb = create_empty_chessboard();
  cb->chessboard_color = WHITE;

  // On place les cavaliers
  place_piece(cb, KNIGHT, WHITE, 0);
  place_piece(cb, KNIGHT, WHITE, 7);
  place_piece(cb, KNIGHT, WHITE, 56);
  place_piece(cb, KNIGHT, WHITE, 63);

  // Génération de la liste de coups
  m = create_movelist();
  generate_knight_move(cb, m);

  // Un cavalier en d4 (index 27) a 8 coups légaux sur un échiquier vide aux
  // index 10, 12, 17, 21, 33, 37, 42 et 44
  CU_ASSERT_EQUAL(count_moves(m), 8);

  move expected_moves2[] = {
      {.from = 0, .to = 10, .type = KNIGHT, .promotion = EMPTY},
      {.from = 0, .to = 17, .type = KNIGHT, .promotion = EMPTY},
      {.from = 7, .to = 13, .type = KNIGHT, .promotion = EMPTY},
      {.from = 7, .to = 22, .type = KNIGHT, .promotion = EMPTY},
      {.from = 56, .to = 41, .type = KNIGHT, .promotion = EMPTY},
      {.from = 56, .to = 50, .type = KNIGHT, .promotion = EMPTY},
      {.from = 63, .to = 46, .type = KNIGHT, .promotion = EMPTY},
      {.from = 63, .to = 53, .type = KNIGHT, .promotion = EMPTY}};

  // TESTS
  for (int i = 0; i < 8; i++) {
    CU_ASSERT_TRUE(move_exists(m, expected_moves2[i]));
  }

  // Free
  free_movelist(m);
  free_chessboard(cb);

  /*
   * =========================================
   * TESTS COTES STRICTS SEUL (A4, D1, D8, H4)
   * =========================================
   */

  cb = create_empty_chessboard();
  cb->chessboard_color = WHITE;

  // On place les cavaliers
  place_piece(cb, KNIGHT, WHITE, 3);
  place_piece(cb, KNIGHT, WHITE, 24);
  place_piece(cb, KNIGHT, WHITE, 31);
  place_piece(cb, KNIGHT, WHITE, 59);

  // Génération de la liste de coups
  m = create_movelist();
  generate_knight_move(cb, m);

  // Il y a 16 coups légaux dans l'ensemble des 4 cavaliers.
  CU_ASSERT_EQUAL(count_moves(m), 16);

  move expected_moves3[] = {
      {.from = 3, .to = 9, .type = KNIGHT, .promotion = EMPTY},
      {.from = 3, .to = 18, .type = KNIGHT, .promotion = EMPTY},
      {.from = 3, .to = 13, .type = KNIGHT, .promotion = EMPTY},
      {.from = 3, .to = 20, .type = KNIGHT, .promotion = EMPTY},
      {.from = 24, .to = 9, .type = KNIGHT, .promotion = EMPTY},
      {.from = 24, .to = 18, .type = KNIGHT, .promotion = EMPTY},
      {.from = 24, .to = 34, .type = KNIGHT, .promotion = EMPTY},
      {.from = 24, .to = 41, .type = KNIGHT, .promotion = EMPTY},
      {.from = 31, .to = 14, .type = KNIGHT, .promotion = EMPTY},
      {.from = 31, .to = 21, .type = KNIGHT, .promotion = EMPTY},
      {.from = 31, .to = 37, .type = KNIGHT, .promotion = EMPTY},
      {.from = 31, .to = 46, .type = KNIGHT, .promotion = EMPTY},
      {.from = 59, .to = 42, .type = KNIGHT, .promotion = EMPTY},
      {.from = 59, .to = 49, .type = KNIGHT, .promotion = EMPTY},
      {.from = 59, .to = 44, .type = KNIGHT, .promotion = EMPTY},
      {.from = 59, .to = 53, .type = KNIGHT, .promotion = EMPTY}};

  // TESTS
  for (int i = 0; i < 16; i++) {
    CU_ASSERT_TRUE(move_exists(m, expected_moves3[i]));
  }

  // Free
  free_movelist(m);
  free_chessboard(cb);

  /*
   * ======================================
   * TESTS COINS SEUL (B2, G2, B7, G7)
   * ======================================
   */

  cb = create_empty_chessboard();
  cb->chessboard_color = WHITE;

  // On place les cavaliers
  place_piece(cb, KNIGHT, WHITE, 9);
  place_piece(cb, KNIGHT, WHITE, 14);
  place_piece(cb, KNIGHT, WHITE, 49);
  place_piece(cb, KNIGHT, WHITE, 54);

  // Génération de la liste de coups
  m = create_movelist();
  generate_knight_move(cb, m);

  // Chaque cavalier proche d'un coin a 4 coups légaux : 4 cavaliers = 16 coups.
  CU_ASSERT_EQUAL(count_moves(m), 16);

  move expected_moves4[] = {
      {.from = 9, .to = 3, .type = KNIGHT, .promotion = EMPTY},
      {.from = 9, .to = 19, .type = KNIGHT, .promotion = EMPTY},
      {.from = 9, .to = 24, .type = KNIGHT, .promotion = EMPTY},
      {.from = 9, .to = 26, .type = KNIGHT, .promotion = EMPTY},
      {.from = 14, .to = 4, .type = KNIGHT, .promotion = EMPTY},
      {.from = 14, .to = 20, .type = KNIGHT, .promotion = EMPTY},
      {.from = 14, .to = 29, .type = KNIGHT, .promotion = EMPTY},
      {.from = 14, .to = 31, .type = KNIGHT, .promotion = EMPTY},
      {.from = 49, .to = 32, .type = KNIGHT, .promotion = EMPTY},
      {.from = 49, .to = 34, .type = KNIGHT, .promotion = EMPTY},
      {.from = 49, .to = 43, .type = KNIGHT, .promotion = EMPTY},
      {.from = 49, .to = 59, .type = KNIGHT, .promotion = EMPTY},
      {.from = 54, .to = 37, .type = KNIGHT, .promotion = EMPTY},
      {.from = 54, .to = 39, .type = KNIGHT, .promotion = EMPTY},
      {.from = 54, .to = 44, .type = KNIGHT, .promotion = EMPTY},
      {.from = 54, .to = 60, .type = KNIGHT, .promotion = EMPTY}};

  // TESTS
  for (int i = 0; i < 16; i++) {
    CU_ASSERT_TRUE(move_exists(m, expected_moves4[i]));
  }

  // Free
  free_movelist(m);
  free_chessboard(cb);

  /*
   * ======================================
   * TESTS COTES SEULS (D2, B4, G4, D7)
   * ======================================
   */

  cb = create_empty_chessboard();
  cb->chessboard_color = WHITE;

  // On place les cavaliers
  place_piece(cb, KNIGHT, WHITE, 11);
  place_piece(cb, KNIGHT, WHITE, 25);
  place_piece(cb, KNIGHT, WHITE, 30);
  place_piece(cb, KNIGHT, WHITE, 51);

  // Génération de la liste de coups
  m = create_movelist();
  generate_knight_move(cb, m);

  // Chaque cavalier sur un côté a 6 coups légaux : 4 cavaliers = 24 coups.
  CU_ASSERT_EQUAL(count_moves(m), 24);

  move expected_moves5[] = {
      {.from = 11, .to = 1, .type = KNIGHT, .promotion = EMPTY},
      {.from = 11, .to = 5, .type = KNIGHT, .promotion = EMPTY},
      {.from = 11, .to = 17, .type = KNIGHT, .promotion = EMPTY},
      {.from = 11, .to = 21, .type = KNIGHT, .promotion = EMPTY},
      {.from = 11, .to = 26, .type = KNIGHT, .promotion = EMPTY},
      {.from = 11, .to = 28, .type = KNIGHT, .promotion = EMPTY},
      {.from = 25, .to = 8, .type = KNIGHT, .promotion = EMPTY},
      {.from = 25, .to = 10, .type = KNIGHT, .promotion = EMPTY},
      {.from = 25, .to = 19, .type = KNIGHT, .promotion = EMPTY},
      {.from = 25, .to = 35, .type = KNIGHT, .promotion = EMPTY},
      {.from = 25, .to = 40, .type = KNIGHT, .promotion = EMPTY},
      {.from = 25, .to = 42, .type = KNIGHT, .promotion = EMPTY},
      {.from = 30, .to = 13, .type = KNIGHT, .promotion = EMPTY},
      {.from = 30, .to = 15, .type = KNIGHT, .promotion = EMPTY},
      {.from = 30, .to = 20, .type = KNIGHT, .promotion = EMPTY},
      {.from = 30, .to = 36, .type = KNIGHT, .promotion = EMPTY},
      {.from = 30, .to = 45, .type = KNIGHT, .promotion = EMPTY},
      {.from = 30, .to = 47, .type = KNIGHT, .promotion = EMPTY},
      {.from = 51, .to = 34, .type = KNIGHT, .promotion = EMPTY},
      {.from = 51, .to = 36, .type = KNIGHT, .promotion = EMPTY},
      {.from = 51, .to = 41, .type = KNIGHT, .promotion = EMPTY},
      {.from = 51, .to = 45, .type = KNIGHT, .promotion = EMPTY},
      {.from = 51, .to = 57, .type = KNIGHT, .promotion = EMPTY},
      {.from = 51, .to = 61, .type = KNIGHT, .promotion = EMPTY}};

  // TESTS
  for (int i = 0; i < 24; i++) {
    CU_ASSERT_TRUE(move_exists(m, expected_moves5[i]));
  }

  // Free
  free_movelist(m);
  free_chessboard(cb);

  /*
   * ===================================================
   * TESTS COINS SANS CHEVAUCHEMENT SEUL (C3, F3, C6, F6)
   * ===================================================
   */

  cb = create_empty_chessboard();
  cb->chessboard_color = WHITE;

  // On place les cavaliers
  place_piece(cb, KNIGHT, WHITE, 18);
  place_piece(cb, KNIGHT, WHITE, 21);
  place_piece(cb, KNIGHT, WHITE, 42);
  place_piece(cb, KNIGHT, WHITE, 45);

  // Génération de la liste de coups
  m = create_movelist();
  generate_knight_move(cb, m);

  // Chaque cavalier a ici sa pleine mobilité de 8 coups : 4 cavaliers = 32
  // coups.
  CU_ASSERT_EQUAL(count_moves(m), 32);

  move expected_moves6[] = {
      {.from = 18, .to = 1, .type = KNIGHT, .promotion = EMPTY},
      {.from = 18, .to = 3, .type = KNIGHT, .promotion = EMPTY},
      {.from = 18, .to = 8, .type = KNIGHT, .promotion = EMPTY},
      {.from = 18, .to = 12, .type = KNIGHT, .promotion = EMPTY},
      {.from = 18, .to = 24, .type = KNIGHT, .promotion = EMPTY},
      {.from = 18, .to = 28, .type = KNIGHT, .promotion = EMPTY},
      {.from = 18, .to = 33, .type = KNIGHT, .promotion = EMPTY},
      {.from = 18, .to = 35, .type = KNIGHT, .promotion = EMPTY},
      {.from = 21, .to = 4, .type = KNIGHT, .promotion = EMPTY},
      {.from = 21, .to = 6, .type = KNIGHT, .promotion = EMPTY},
      {.from = 21, .to = 11, .type = KNIGHT, .promotion = EMPTY},
      {.from = 21, .to = 15, .type = KNIGHT, .promotion = EMPTY},
      {.from = 21, .to = 27, .type = KNIGHT, .promotion = EMPTY},
      {.from = 21, .to = 31, .type = KNIGHT, .promotion = EMPTY},
      {.from = 21, .to = 36, .type = KNIGHT, .promotion = EMPTY},
      {.from = 21, .to = 38, .type = KNIGHT, .promotion = EMPTY},
      {.from = 42, .to = 25, .type = KNIGHT, .promotion = EMPTY},
      {.from = 42, .to = 27, .type = KNIGHT, .promotion = EMPTY},
      {.from = 42, .to = 32, .type = KNIGHT, .promotion = EMPTY},
      {.from = 42, .to = 36, .type = KNIGHT, .promotion = EMPTY},
      {.from = 42, .to = 48, .type = KNIGHT, .promotion = EMPTY},
      {.from = 42, .to = 52, .type = KNIGHT, .promotion = EMPTY},
      {.from = 42, .to = 57, .type = KNIGHT, .promotion = EMPTY},
      {.from = 42, .to = 59, .type = KNIGHT, .promotion = EMPTY},
      {.from = 45, .to = 28, .type = KNIGHT, .promotion = EMPTY},
      {.from = 45, .to = 30, .type = KNIGHT, .promotion = EMPTY},
      {.from = 45, .to = 35, .type = KNIGHT, .promotion = EMPTY},
      {.from = 45, .to = 39, .type = KNIGHT, .promotion = EMPTY},
      {.from = 45, .to = 51, .type = KNIGHT, .promotion = EMPTY},
      {.from = 45, .to = 55, .type = KNIGHT, .promotion = EMPTY},
      {.from = 45, .to = 60, .type = KNIGHT, .promotion = EMPTY},
      {.from = 45, .to = 62, .type = KNIGHT, .promotion = EMPTY}};

  // TESTS
  for (int i = 0; i < 32; i++) {
    CU_ASSERT_TRUE(move_exists(m, expected_moves6[i]));
  }

  // Free
  free_movelist(m);
  free_chessboard(cb);

  /*
   * ============================================
   * TEST BLOCAGE PAR DES PIECES DU JOUEUR : D4
   * ============================================
   */

  cb = create_empty_chessboard();
  cb->chessboard_color = WHITE;

  // On place le cavalier en d4 (index 27)
  place_piece(cb, KNIGHT, WHITE, 27);

  /*
   * On bloque 3 des 8 cases d'arrivée du cavalier avec des pièces du joueur
   * actif (WHITE). Ces cases (10, 12, 17) ne doivent plus être proposées.
   */
  place_piece(cb, PAWN, WHITE, 10);
  place_piece(cb, PAWN, WHITE, 12);
  place_piece(cb, PAWN, WHITE, 17);

  // Génération de la liste de coups
  m = create_movelist();
  generate_knight_move(cb, m);

  // 8 coups possibles - 3 cases bloquées par le joueur = 5 coups.
  CU_ASSERT_EQUAL(count_moves(m), 5);

  // Les coups vers les cases bloquées ne doivent PAS exister
  move blocked_moves[] = {
      {.from = 27, .to = 10, .type = KNIGHT, .promotion = EMPTY},
      {.from = 27, .to = 12, .type = KNIGHT, .promotion = EMPTY},
      {.from = 27, .to = 17, .type = KNIGHT, .promotion = EMPTY}};
  for (int i = 0; i < 3; i++) {
    CU_ASSERT_FALSE(move_exists(m, blocked_moves[i]));
  }

  // Les coups vers les cases libres doivent toujours exister
  move expected_moves7[] = {
      {.from = 27, .to = 21, .type = KNIGHT, .promotion = EMPTY},
      {.from = 27, .to = 33, .type = KNIGHT, .promotion = EMPTY},
      {.from = 27, .to = 37, .type = KNIGHT, .promotion = EMPTY},
      {.from = 27, .to = 42, .type = KNIGHT, .promotion = EMPTY},
      {.from = 27, .to = 44, .type = KNIGHT, .promotion = EMPTY}};
  for (int i = 0; i < 5; i++) {
    CU_ASSERT_TRUE(move_exists(m, expected_moves7[i]));
  }

  // Free
  free_movelist(m);
  free_chessboard(cb);

  /*
   * ============================================
   * TEST PRISE D'UNE PIECE ADVERSE : D4
   * ============================================
   */

  cb = create_empty_chessboard();
  cb->chessboard_color = WHITE;

  // On place le cavalier en d4 (index 27)
  place_piece(cb, KNIGHT, WHITE, 27);

  /*
   * On place des pièces adverses (BLACK) sur 2 des cases d'arrivée du cavalier.
   * Ces cases (10, 12) restent atteignables : le cavalier peut y prendre la
   * pièce adverse.
   */
  place_piece(cb, PAWN, BLACK, 10);
  place_piece(cb, PAWN, BLACK, 12);

  // Génération de la liste de coups
  m = create_movelist();
  generate_knight_move(cb, m);

  // Les prises sont autorisées : les 8 coups sont toujours présents.
  CU_ASSERT_EQUAL(count_moves(m), 8);

  move expected_moves8[] = {
      {.from = 27,
       .to = 10,
       .type = KNIGHT,
       .promotion = EMPTY,
       .kind = MOVE_CAPTURE}, // prise
      {.from = 27,
       .to = 12,
       .type = KNIGHT,
       .promotion = EMPTY,
       .kind = MOVE_CAPTURE}, // prise
      {.from = 27, .to = 17, .type = KNIGHT, .promotion = EMPTY},
      {.from = 27, .to = 21, .type = KNIGHT, .promotion = EMPTY},
      {.from = 27, .to = 33, .type = KNIGHT, .promotion = EMPTY},
      {.from = 27, .to = 37, .type = KNIGHT, .promotion = EMPTY},
      {.from = 27, .to = 42, .type = KNIGHT, .promotion = EMPTY},
      {.from = 27, .to = 44, .type = KNIGHT, .promotion = EMPTY}};
  for (int i = 0; i < 8; i++) {
    CU_ASSERT_TRUE(move_exists(m, expected_moves8[i]));
  }

  // Free
  free_movelist(m);
  free_chessboard(cb);
}

// --- Tests pour generate_pawn_move ---
void test_generate_pawn_move(void) {

  /*
   * ===================================================
   * TEST POSITION DE DEPART SANS ADVERSAIRE : E2 (index 12)
   * ===================================================
   */
  chessboard cb = create_empty_chessboard();
  cb->chessboard_color = WHITE;

  // On place le pion en e2 (index 12), sur sa ligne de départ
  place_piece(cb, PAWN, WHITE, 12);

  movelist m = create_movelist();
  generate_pawn_move(cb, m);

  // Sur sa ligne de départ et sans adversaire : avance de 1 (e3 = 20) et de 2
  // (e4 = 28)
  CU_ASSERT_EQUAL(count_moves(m), 2);

  move expected_moves1[] = {
      {.from = 12, .to = 20, .type = PAWN, .promotion = EMPTY},
      {.from = 12,
       .to = 28,
       .type = PAWN,
       .promotion = EMPTY,
       .kind = MOVE_DOUBLE_PUSH}};

  // TESTS
  for (int i = 0; i < 2; i++) {
    CU_ASSERT_TRUE(move_exists(m, expected_moves1[i]));
  }

  // Free
  free_movelist(m);
  free_chessboard(cb);

  /*
   * =========================================================
   * TEST PION DEJA DEPLACE SANS ADVERSAIRE : E3 (index 20)
   * =========================================================
   */
  cb = create_empty_chessboard();
  cb->chessboard_color = WHITE;

  // On place le pion en e3 (index 20) : il n'est plus sur sa ligne de départ
  place_piece(cb, PAWN, WHITE, 20);

  m = create_movelist();
  generate_pawn_move(cb, m);

  // Hors de sa ligne de départ : il ne peut avancer que de 1 (e4 = 28)
  CU_ASSERT_EQUAL(count_moves(m), 1);

  move expected_moves2[] = {
      {.from = 20, .to = 28, .type = PAWN, .promotion = EMPTY}};

  // TESTS
  for (int i = 0; i < 1; i++) {
    CU_ASSERT_TRUE(move_exists(m, expected_moves2[i]));
  }

  // Free
  free_movelist(m);
  free_chessboard(cb);

  /*
   * =========================================================
   * TEST PROMOTION : E7 (index 52) -> E8 (index 60)
   * =========================================================
   */
  cb = create_empty_chessboard();
  cb->chessboard_color = WHITE;

  // On place le pion en e7 (index 52) : il va promouvoir en avançant en e8
  // (index 60)
  place_piece(cb, PAWN, WHITE, 52);

  m = create_movelist();
  generate_pawn_move(cb, m);

  // La promotion doit générer 4 coups (tour, cavalier, fou, dame) vers e8
  CU_ASSERT_EQUAL(count_moves(m), 4);

  move expected_moves3[] = {
      {.from = 52, .to = 60, .type = PAWN, .promotion = ROOK},
      {.from = 52, .to = 60, .type = PAWN, .promotion = KNIGHT},
      {.from = 52, .to = 60, .type = PAWN, .promotion = BISHOP},
      {.from = 52, .to = 60, .type = PAWN, .promotion = QUEEN}};

  // TESTS
  for (int i = 0; i < 4; i++) {
    CU_ASSERT_TRUE(move_exists(m, expected_moves3[i]));
  }

  // Free
  free_movelist(m);
  free_chessboard(cb);

  /*
   * =========================================================
   * TEST PRISE EN DIAGONALE : E4 (index 28)
   * =========================================================
   */
  cb = create_empty_chessboard();
  cb->chessboard_color = WHITE;

  // On place le pion en e4 (index 28)
  place_piece(cb, PAWN, WHITE, 28);

  // On place deux pièces adverses sur ses diagonales : d5 (index 35) et f5
  // (index 37)
  place_piece(cb, PAWN, BLACK, 35);
  place_piece(cb, PAWN, BLACK, 37);

  m = create_movelist();
  generate_pawn_move(cb, m);

  // Avance de 1 (e5 = 36) + prise à gauche (d5 = 35) + prise à droite (f5 = 37)
  // = 3 coups
  CU_ASSERT_EQUAL(count_moves(m), 3);

  move expected_moves4[] = {
      {.from = 28, .to = 36, .type = PAWN, .promotion = EMPTY}, // avance
      {.from = 28,
       .to = 35,
       .type = PAWN,
       .promotion = EMPTY,
       .kind = MOVE_CAPTURE}, // prise
      {.from = 28,
       .to = 37,
       .type = PAWN,
       .promotion = EMPTY,
       .kind = MOVE_CAPTURE} // prise
  };

  // TESTS
  for (int i = 0; i < 3; i++) {
    CU_ASSERT_TRUE(move_exists(m, expected_moves4[i]));
  }

  // Free
  free_movelist(m);
  free_chessboard(cb);

  /*
   * ===============================================================
   * TEST ANTI-DEBORDEMENT DE COLONNE : H4 (index 31)
   * ===============================================================
   * Un pion en h4 (colonne H) calcule ses prises en 31+7 (g5 = 38, légale)
   * et 31+9 (index 40). L'index 40 correspond à a5 : il est "de l'autre côté"
   * de l'échiquier (colonne A). Sans la garde sur la colonne, le pion prendrait
   * cette pièce à tort après un débordement d'index. On vérifie qu'il ne la
   * prend PAS.
   */
  cb = create_empty_chessboard();
  cb->chessboard_color = WHITE;

  // On place le pion en h4 (index 31)
  place_piece(cb, PAWN, WHITE, 31);

  // Pièce adverse sur la vraie diagonale g5 (index 38) : prise légale
  place_piece(cb, PAWN, BLACK, 38);
  // Pièce adverse sur a5 (index 40) : de l'autre côté de l'échiquier, ne doit
  // PAS être prise
  place_piece(cb, PAWN, BLACK, 40);

  m = create_movelist();
  generate_pawn_move(cb, m);

  // Avance de 1 (h5 = 39) + prise légale (g5 = 38) = 2 coups. La fausse prise
  // vers a5 est exclue.
  CU_ASSERT_EQUAL(count_moves(m), 2);

  move expected_moves5[] = {
      {.from = 31, .to = 39, .type = PAWN, .promotion = EMPTY}, // avance
      {.from = 31,
       .to = 38,
       .type = PAWN,
       .promotion = EMPTY,
       .kind = MOVE_CAPTURE} // prise légale
  };

  // TESTS
  for (int i = 0; i < 2; i++) {
    CU_ASSERT_TRUE(move_exists(m, expected_moves5[i]));
  }

  // La prise débordant sur l'autre côté de l'échiquier (a5 = 40) ne doit PAS
  // exister
  move overflow_move = {.from = 31, .to = 40, .type = PAWN, .promotion = EMPTY};
  CU_ASSERT_FALSE(move_exists(m, overflow_move));

  // Free
  free_movelist(m);
  free_chessboard(cb);

  /*
   * =========================================================
   * TEST EN PASSANT : e5 (36) prend en passant en d6 (43)
   * =========================================================
   * Le pion noir vient de jouer d7-d5 (il est en d5 = 35). La case
   * survolée d6 (43) est la cible du en passant. Le pion blanc e5 (36)
   * peut le prendre en passant en se déplaçant en d6 (43).
   */
  cb = create_empty_chessboard();
  cb->chessboard_color = WHITE;

  // Pion blanc en e5 (index 36)
  place_piece(cb, PAWN, WHITE, 36);
  // Pion noir en d5 (index 35) : il vient d'avancer de deux cases
  place_piece(cb, PAWN, BLACK, 35);
  // Cible du en passant : d6 (index 43)
  set_en_passant_target(cb, 43);

  m = create_movelist();
  generate_pawn_move(cb, m);

  // Avance de 1 (e6 = 44) + prise en passant (d6 = 43) = 2 coups
  CU_ASSERT_EQUAL(count_moves(m), 2);

  move expected_moves6[] = {
      {.from = 36, .to = 44, .type = PAWN, .promotion = EMPTY}, // avance
      {.from = 36,
       .to = 43,
       .type = PAWN,
       .promotion = EMPTY,
       .kind = MOVE_EN_PASSANT} // prise en passant
  };

  // TESTS
  for (int i = 0; i < 2; i++) {
    CU_ASSERT_TRUE(move_exists(m, expected_moves6[i]));
  }

  // Free
  free_movelist(m);
  free_chessboard(cb);

  /*
   * ==================================================================
   * TEST EN PASSANT EMPECHE PAR UNE PIECE DU JOUEUR SUR LA CASE CIBLE
   * ==================================================================
   * Même configuration, mais une pièce du joueur actif occupe déjà la
   * case d'arrivée d6 (43). Le pion ne peut pas s'y déplacer : le en
   * passant ne doit PAS être proposé.
   */
  cb = create_empty_chessboard();
  cb->chessboard_color = WHITE;

  // Pion blanc en e5 (index 36)
  place_piece(cb, PAWN, WHITE, 36);
  // Pion noir en d5 (index 35)
  place_piece(cb, PAWN, BLACK, 35);
  // Pièce du joueur actif sur la case cible d6 (index 43)
  place_piece(cb, KNIGHT, WHITE, 43);
  // Cible du en passant : d6 (index 43)
  set_en_passant_target(cb, 43);

  m = create_movelist();
  generate_pawn_move(cb, m);

  // La case d6 étant occupée par le joueur, seule l'avance e6 (44) reste
  // possible
  CU_ASSERT_EQUAL(count_moves(m), 1);

  move expected_moves7[] = {
      {.from = 36, .to = 44, .type = PAWN, .promotion = EMPTY} // avance
  };

  // TESTS
  for (int i = 0; i < 1; i++) {
    CU_ASSERT_TRUE(move_exists(m, expected_moves7[i]));
  }

  // La prise en passant vers d6 (43) ne doit PAS exister
  move blocked_en_passant = {.from = 36,
                             .to = 43,
                             .type = PAWN,
                             .promotion = EMPTY,
                             .kind = MOVE_EN_PASSANT};
  CU_ASSERT_FALSE(move_exists(m, blocked_en_passant));

  // Free
  free_movelist(m);
  free_chessboard(cb);

  /*
   * =========================================================
   * TEST BLOCAGE DEVANT PAR UNE PIECE DU MEME JOUEUR : E2 (12)
   * =========================================================
   * Une pièce du joueur actif occupe la case juste devant le pion (e3 = 20).
   * Le pion ne peut ni avancer de 1 ni de 2.
   */
  cb = create_empty_chessboard();
  cb->chessboard_color = WHITE;

  // Pion blanc en e2 (index 12), sur sa ligne de départ
  place_piece(cb, PAWN, WHITE, 12);
  // Pièce du joueur actif juste devant, en e3 (index 20)
  place_piece(cb, KNIGHT, WHITE, 20);

  m = create_movelist();
  generate_pawn_move(cb, m);

  // La case devant est bloquée par le joueur : aucun coup pour le pion
  CU_ASSERT_EQUAL(count_moves(m), 0);

  // Ni l'avance de 1 (e3 = 20) ni l'avance de 2 (e4 = 28) ne doivent exister
  move blocked_single = {
      .from = 12, .to = 20, .type = PAWN, .promotion = EMPTY};
  move blocked_double = {.from = 12,
                         .to = 28,
                         .type = PAWN,
                         .promotion = EMPTY,
                         .kind = MOVE_DOUBLE_PUSH};
  CU_ASSERT_FALSE(move_exists(m, blocked_single));
  CU_ASSERT_FALSE(move_exists(m, blocked_double));

  // Free
  free_movelist(m);
  free_chessboard(cb);

  /*
   * ==================================================================
   * TEST PRISE AVEC PROMOTION : b7 (49) pousse ou prend en promouvant
   * ==================================================================
   * Le pion en b7 peut promouvoir en avançant en b8 (57) ou en prenant
   * en a8 (56) ou c8 (58). Chaque arrivée génère 4 promotions.
   */
  cb = create_empty_chessboard();
  cb->chessboard_color = WHITE;

  // Pion blanc en b7 (index 49)
  place_piece(cb, PAWN, WHITE, 49);
  // Pièces adverses à prendre en promouvant : a8 (56) et c8 (58)
  place_piece(cb, ROOK, BLACK, 56);
  place_piece(cb, KNIGHT, BLACK, 58);

  m = create_movelist();
  generate_pawn_move(cb, m);

  // 3 cases d'arrivée (b8, a8, c8) x 4 promotions = 12 coups
  CU_ASSERT_EQUAL(count_moves(m), 12);

  // Promotion par poussée (b8) et par prise (a8, c8) : on vérifie la dame et le
  // cavalier
  move promo_push_q = {.from = 49, .to = 57, .type = PAWN, .promotion = QUEEN};
  move promo_push_n = {.from = 49, .to = 57, .type = PAWN, .promotion = KNIGHT};
  move promo_take_a8 = {.from = 49,
                        .to = 56,
                        .type = PAWN,
                        .promotion = QUEEN,
                        .kind = MOVE_CAPTURE};
  move promo_take_c8 = {.from = 49,
                        .to = 58,
                        .type = PAWN,
                        .promotion = QUEEN,
                        .kind = MOVE_CAPTURE};
  CU_ASSERT_TRUE(move_exists(m, promo_push_q));
  CU_ASSERT_TRUE(move_exists(m, promo_push_n));
  CU_ASSERT_TRUE(move_exists(m, promo_take_a8));
  CU_ASSERT_TRUE(move_exists(m, promo_take_c8));

  // Free
  free_movelist(m);
  free_chessboard(cb);

  /*
   * ==================================================================
   * TEST EN PASSANT DEPUIS UNE COLONNE DE BORD (garde de colonne)
   * ==================================================================
   * Le pion noir vient de jouer a7-a5 (a5 = 32), cible du en passant a6 (40).
   * Le pion blanc b5 (33) peut prendre en passant en a6. Le pion blanc h4 (31)
   * apparaît dans le calcul des sources (40-8+1 = 33 ; 40-8-1 = 31) mais 31 est
   * sur la colonne H : il ne doit PAS prendre en a6 par débordement.
   */
  cb = create_empty_chessboard();
  cb->chessboard_color = WHITE;

  place_piece(cb, PAWN, BLACK, 32); // a5
  place_piece(cb, PAWN, WHITE, 33); // b5 : prise en passant légale
  place_piece(cb, PAWN, WHITE, 31); // h4 : ne doit pas déborder vers a6
  set_en_passant_target(cb, 40);    // a6

  m = create_movelist();
  generate_pawn_move(cb, m);

  // h4 avance (h5), b5 avance (b6) + prise en passant (a6) = 3 coups
  CU_ASSERT_EQUAL(count_moves(m), 3);

  // La prise en passant légale (b5 -> a6) doit exister
  move ep_legal = {.from = 33,
                   .to = 40,
                   .type = PAWN,
                   .promotion = EMPTY,
                   .kind = MOVE_EN_PASSANT};
  CU_ASSERT_TRUE(move_exists(m, ep_legal));

  // La prise en passant par débordement (h4 -> a6) ne doit PAS exister
  move ep_overflow = {.from = 31, .to = 40, .type = PAWN, .promotion = EMPTY};
  CU_ASSERT_FALSE(move_exists(m, ep_overflow));

  // Free
  free_movelist(m);
  free_chessboard(cb);
}

// --- Tests pour generate_pawn_move (pion noir) ---
void test_generate_pawn_move_black(void) {

  /*
   * ===================================================
   * TEST POSITION DE DEPART SANS ADVERSAIRE : E7 (index 52)
   * ===================================================
   * Le pion noir avance vers le bas (-8) et sa ligne de départ est le rang 7.
   */
  chessboard cb = create_empty_chessboard();
  cb->chessboard_color = BLACK;

  // On place le pion noir en e7 (index 52), sur sa ligne de départ
  place_piece(cb, PAWN, BLACK, 52);

  movelist m = create_movelist();
  generate_pawn_move(cb, m);

  // Sur sa ligne de départ : avance de 1 (e6 = 44) et de 2 (e5 = 36)
  CU_ASSERT_EQUAL(count_moves(m), 2);

  move expected_moves1[] = {
      {.from = 52, .to = 44, .type = PAWN, .promotion = EMPTY},
      {.from = 52,
       .to = 36,
       .type = PAWN,
       .promotion = EMPTY,
       .kind = MOVE_DOUBLE_PUSH}};
  for (int i = 0; i < 2; i++) {
    CU_ASSERT_TRUE(move_exists(m, expected_moves1[i]));
  }

  free_movelist(m);
  free_chessboard(cb);

  /*
   * =========================================================
   * TEST PION DEJA DEPLACE SANS ADVERSAIRE : E6 (index 44)
   * =========================================================
   */
  cb = create_empty_chessboard();
  cb->chessboard_color = BLACK;

  // Pion noir en e6 (index 44) : hors de sa ligne de départ
  place_piece(cb, PAWN, BLACK, 44);

  m = create_movelist();
  generate_pawn_move(cb, m);

  // Il ne peut avancer que de 1 (e5 = 36)
  CU_ASSERT_EQUAL(count_moves(m), 1);

  move expected_moves2[] = {
      {.from = 44, .to = 36, .type = PAWN, .promotion = EMPTY}};
  for (int i = 0; i < 1; i++) {
    CU_ASSERT_TRUE(move_exists(m, expected_moves2[i]));
  }

  free_movelist(m);
  free_chessboard(cb);

  /*
   * =========================================================
   * TEST PROMOTION : E2 (index 12) -> E1 (index 4)
   * =========================================================
   * Le pion noir promeut en atteignant le rang 1.
   */
  cb = create_empty_chessboard();
  cb->chessboard_color = BLACK;

  // Pion noir en e2 (index 12) : il va promouvoir en avançant en e1 (index 4)
  place_piece(cb, PAWN, BLACK, 12);

  m = create_movelist();
  generate_pawn_move(cb, m);

  // La promotion doit générer 4 coups (tour, cavalier, fou, dame) vers e1
  CU_ASSERT_EQUAL(count_moves(m), 4);

  move expected_moves3[] = {
      {.from = 12, .to = 4, .type = PAWN, .promotion = ROOK},
      {.from = 12, .to = 4, .type = PAWN, .promotion = KNIGHT},
      {.from = 12, .to = 4, .type = PAWN, .promotion = BISHOP},
      {.from = 12, .to = 4, .type = PAWN, .promotion = QUEEN}};
  for (int i = 0; i < 4; i++) {
    CU_ASSERT_TRUE(move_exists(m, expected_moves3[i]));
  }

  free_movelist(m);
  free_chessboard(cb);

  /*
   * =========================================================
   * TEST PRISE EN DIAGONALE : E5 (index 36)
   * =========================================================
   */
  cb = create_empty_chessboard();
  cb->chessboard_color = BLACK;

  // Pion noir en e5 (index 36)
  place_piece(cb, PAWN, BLACK, 36);
  // Pièces adverses (blanches) sur ses diagonales : d4 (27) et f4 (29)
  place_piece(cb, PAWN, WHITE, 27);
  place_piece(cb, PAWN, WHITE, 29);

  m = create_movelist();
  generate_pawn_move(cb, m);

  // Avance de 1 (e4 = 28) + prise à gauche (d4 = 27) + prise à droite (f4 = 29)
  // = 3 coups
  CU_ASSERT_EQUAL(count_moves(m), 3);

  move expected_moves4[] = {
      {.from = 36, .to = 28, .type = PAWN, .promotion = EMPTY}, // avance
      {.from = 36,
       .to = 27,
       .type = PAWN,
       .promotion = EMPTY,
       .kind = MOVE_CAPTURE}, // prise
      {.from = 36,
       .to = 29,
       .type = PAWN,
       .promotion = EMPTY,
       .kind = MOVE_CAPTURE} // prise
  };
  for (int i = 0; i < 3; i++) {
    CU_ASSERT_TRUE(move_exists(m, expected_moves4[i]));
  }

  free_movelist(m);
  free_chessboard(cb);

  /*
   * ===============================================================
   * TEST ANTI-DEBORDEMENT DE COLONNE : A5 (index 32)
   * ===============================================================
   * Un pion noir en a5 (colonne A) calcule ses prises en 32-7 (b4 = 25, légale)
   * et 32-9 (index 23). L'index 23 correspond à h3 : de l'autre côté de
   * l'échiquier. On vérifie qu'il ne prend PAS cette pièce par débordement.
   */
  cb = create_empty_chessboard();
  cb->chessboard_color = BLACK;

  // Pion noir en a5 (index 32)
  place_piece(cb, PAWN, BLACK, 32);
  // Pièce adverse sur la vraie diagonale b4 (index 25) : prise légale
  place_piece(cb, PAWN, WHITE, 25);
  // Pièce adverse sur h3 (index 23) : de l'autre côté, ne doit PAS être prise
  place_piece(cb, PAWN, WHITE, 23);

  m = create_movelist();
  generate_pawn_move(cb, m);

  // Avance de 1 (a4 = 24) + prise légale (b4 = 25) = 2 coups
  CU_ASSERT_EQUAL(count_moves(m), 2);

  move expected_moves5[] = {
      {.from = 32, .to = 24, .type = PAWN, .promotion = EMPTY}, // avance
      {.from = 32,
       .to = 25,
       .type = PAWN,
       .promotion = EMPTY,
       .kind = MOVE_CAPTURE} // prise légale
  };
  for (int i = 0; i < 2; i++) {
    CU_ASSERT_TRUE(move_exists(m, expected_moves5[i]));
  }

  // La prise débordant sur l'autre côté de l'échiquier (h3 = 23) ne doit PAS
  // exister
  move overflow_move = {.from = 32, .to = 23, .type = PAWN, .promotion = EMPTY};
  CU_ASSERT_FALSE(move_exists(m, overflow_move));

  free_movelist(m);
  free_chessboard(cb);

  /*
   * =========================================================
   * TEST EN PASSANT : d4 (27) prend en passant en e3 (20)
   * =========================================================
   * Le pion blanc vient de jouer e2-e4 (il est en e4 = 28). La case survolée
   * e3 (20) est la cible du en passant. Le pion noir d4 (27) peut le prendre.
   */
  cb = create_empty_chessboard();
  cb->chessboard_color = BLACK;

  // Pion noir en d4 (index 27)
  place_piece(cb, PAWN, BLACK, 27);
  // Pion blanc en e4 (index 28) : il vient d'avancer de deux cases
  place_piece(cb, PAWN, WHITE, 28);
  // Cible du en passant : e3 (index 20)
  set_en_passant_target(cb, 20);

  m = create_movelist();
  generate_pawn_move(cb, m);

  // Avance de 1 (d3 = 19) + prise en passant (e3 = 20) = 2 coups
  CU_ASSERT_EQUAL(count_moves(m), 2);

  move expected_moves6[] = {
      {.from = 27, .to = 19, .type = PAWN, .promotion = EMPTY}, // avance
      {.from = 27,
       .to = 20,
       .type = PAWN,
       .promotion = EMPTY,
       .kind = MOVE_EN_PASSANT} // prise en passant
  };
  for (int i = 0; i < 2; i++) {
    CU_ASSERT_TRUE(move_exists(m, expected_moves6[i]));
  }

  free_movelist(m);
  free_chessboard(cb);

  /*
   * ==================================================================
   * TEST EN PASSANT EMPECHE PAR UNE PIECE DU JOUEUR SUR LA CASE CIBLE
   * ==================================================================
   */
  cb = create_empty_chessboard();
  cb->chessboard_color = BLACK;

  // Pion noir en d4 (index 27), pion blanc en e4 (index 28)
  place_piece(cb, PAWN, BLACK, 27);
  place_piece(cb, PAWN, WHITE, 28);
  // Pièce du joueur actif (noir) sur la case cible e3 (index 20)
  place_piece(cb, KNIGHT, BLACK, 20);
  set_en_passant_target(cb, 20);

  m = create_movelist();
  generate_pawn_move(cb, m);

  // La case e3 étant occupée par le joueur, seule l'avance d3 (19) reste
  // possible
  CU_ASSERT_EQUAL(count_moves(m), 1);

  move expected_moves7[] = {
      {.from = 27, .to = 19, .type = PAWN, .promotion = EMPTY}};
  for (int i = 0; i < 1; i++) {
    CU_ASSERT_TRUE(move_exists(m, expected_moves7[i]));
  }

  // La prise en passant vers e3 (20) ne doit PAS exister
  move blocked_en_passant = {.from = 27,
                             .to = 20,
                             .type = PAWN,
                             .promotion = EMPTY,
                             .kind = MOVE_EN_PASSANT};
  CU_ASSERT_FALSE(move_exists(m, blocked_en_passant));

  free_movelist(m);
  free_chessboard(cb);

  /*
   * =========================================================
   * TEST BLOCAGE DEVANT PAR UNE PIECE DU MEME JOUEUR : E7 (52)
   * =========================================================
   */
  cb = create_empty_chessboard();
  cb->chessboard_color = BLACK;

  // Pion noir en e7 (index 52), sur sa ligne de départ
  place_piece(cb, PAWN, BLACK, 52);
  // Pièce du joueur actif juste devant, en e6 (index 44)
  place_piece(cb, KNIGHT, BLACK, 44);

  m = create_movelist();
  generate_pawn_move(cb, m);

  // La case devant est bloquée par le joueur : aucun coup pour le pion
  CU_ASSERT_EQUAL(count_moves(m), 0);

  // Ni l'avance de 1 (e6 = 44) ni l'avance de 2 (e5 = 36) ne doivent exister
  move blocked_single = {
      .from = 52, .to = 44, .type = PAWN, .promotion = EMPTY};
  move blocked_double = {.from = 52,
                         .to = 36,
                         .type = PAWN,
                         .promotion = EMPTY,
                         .kind = MOVE_DOUBLE_PUSH};
  CU_ASSERT_FALSE(move_exists(m, blocked_single));
  CU_ASSERT_FALSE(move_exists(m, blocked_double));

  free_movelist(m);
  free_chessboard(cb);
}

// --- Tests pour generate_bishop_move ---
void test_generate_bishop_move(void) {

  /*
   * =========================================================
   * TEST FOU AU MILIEU SANS AUTRE PIECE : D4 (index 27)
   * =========================================================
   */
  chessboard cb = create_empty_chessboard();
  cb->chessboard_color = WHITE;

  // On place le fou en d4 (index 27)
  place_piece(cb, BISHOP, WHITE, 27);

  movelist m = create_movelist();
  generate_bishop_move(cb, m);

  // Sur un échiquier vide, le fou en d4 a 13 coups (4 diagonales)
  CU_ASSERT_EQUAL(count_moves(m), 13);

  move expected_moves1[] = {
      // Diagonale bas-gauche : c3, b2, a1
      {.from = 27, .to = 18, .type = BISHOP, .promotion = EMPTY},
      {.from = 27, .to = 9, .type = BISHOP, .promotion = EMPTY},
      {.from = 27, .to = 0, .type = BISHOP, .promotion = EMPTY},
      // Diagonale bas-droite : e3, f2, g1
      {.from = 27, .to = 20, .type = BISHOP, .promotion = EMPTY},
      {.from = 27, .to = 13, .type = BISHOP, .promotion = EMPTY},
      {.from = 27, .to = 6, .type = BISHOP, .promotion = EMPTY},
      // Diagonale haut-gauche : c5, b6, a7
      {.from = 27, .to = 34, .type = BISHOP, .promotion = EMPTY},
      {.from = 27, .to = 41, .type = BISHOP, .promotion = EMPTY},
      {.from = 27, .to = 48, .type = BISHOP, .promotion = EMPTY},
      // Diagonale haut-droite : e5, f6, g7, h8
      {.from = 27, .to = 36, .type = BISHOP, .promotion = EMPTY},
      {.from = 27, .to = 45, .type = BISHOP, .promotion = EMPTY},
      {.from = 27, .to = 54, .type = BISHOP, .promotion = EMPTY},
      {.from = 27, .to = 63, .type = BISHOP, .promotion = EMPTY}};

  // TESTS
  for (int i = 0; i < 13; i++) {
    CU_ASSERT_TRUE(move_exists(m, expected_moves1[i]));
  }

  // Free
  free_movelist(m);
  free_chessboard(cb);

  /*
   * =========================================================
   * TEST DEBORDEMENT DE COLONNE : A4 (index 24)
   * =========================================================
   * Un fou en a4 (colonne A) ne peut pas glisser vers la gauche : les
   * diagonales -9 (index 15 = h2) et +7 (index 31 = h4) débordent sur
   * la colonne H, de l'autre côté de l'échiquier. On y place des pièces
   * adverses pour vérifier que le fou ne les prend pas après débordement.
   */
  cb = create_empty_chessboard();
  cb->chessboard_color = WHITE;

  // On place le fou en a4 (index 24)
  place_piece(cb, BISHOP, WHITE, 24);
  // Pièces adverses sur les cases de débordement : h2 (15) et h4 (31)
  place_piece(cb, PAWN, BLACK, 15);
  place_piece(cb, PAWN, BLACK, 31);

  m = create_movelist();
  generate_bishop_move(cb, m);

  // Seules les deux diagonales vers la droite existent : 7 coups
  CU_ASSERT_EQUAL(count_moves(m), 7);

  move expected_moves2[] = {
      // Diagonale bas-droite : b3, c2, d1
      {.from = 24, .to = 17, .type = BISHOP, .promotion = EMPTY},
      {.from = 24, .to = 10, .type = BISHOP, .promotion = EMPTY},
      {.from = 24, .to = 3, .type = BISHOP, .promotion = EMPTY},
      // Diagonale haut-droite : b5, c6, d7, e8
      {.from = 24, .to = 33, .type = BISHOP, .promotion = EMPTY},
      {.from = 24, .to = 42, .type = BISHOP, .promotion = EMPTY},
      {.from = 24, .to = 51, .type = BISHOP, .promotion = EMPTY},
      {.from = 24, .to = 60, .type = BISHOP, .promotion = EMPTY}};

  // TESTS
  for (int i = 0; i < 7; i++) {
    CU_ASSERT_TRUE(move_exists(m, expected_moves2[i]));
  }

  // Les prises par débordement de colonne (h2 = 15, h4 = 31) ne doivent PAS
  // exister
  move overflow_move1 = {
      .from = 24, .to = 15, .type = BISHOP, .promotion = EMPTY};
  move overflow_move2 = {
      .from = 24, .to = 31, .type = BISHOP, .promotion = EMPTY};
  CU_ASSERT_FALSE(move_exists(m, overflow_move1));
  CU_ASSERT_FALSE(move_exists(m, overflow_move2));

  // Free
  free_movelist(m);
  free_chessboard(cb);

  /*
   * =========================================================
   * TEST BLOCAGE PAR UNE PIECE DU MEME JOUEUR : D4 (index 27)
   * =========================================================
   * Une pièce du joueur en f6 (45) bloque la diagonale haut-droite : le
   * fou peut aller en e5 (36) mais pas sur f6 ni au-delà (g7, h8).
   */
  cb = create_empty_chessboard();
  cb->chessboard_color = WHITE;

  // On place le fou en d4 (index 27)
  place_piece(cb, BISHOP, WHITE, 27);
  // Pièce du joueur qui bloque la diagonale haut-droite, en f6 (index 45)
  place_piece(cb, KNIGHT, WHITE, 45);

  m = create_movelist();
  generate_bishop_move(cb, m);

  // 13 coups - la case f6 - les 2 cases derrière (g7, h8) = 10 coups
  CU_ASSERT_EQUAL(count_moves(m), 10);

  // Le fou peut aller jusqu'en e5 (36), juste avant la pièce alliée
  move before_block = {
      .from = 27, .to = 36, .type = BISHOP, .promotion = EMPTY};
  CU_ASSERT_TRUE(move_exists(m, before_block));

  // Il ne peut aller ni sur la case alliée (f6 = 45) ni derrière (g7 = 54, h8 =
  // 63)
  move on_ally = {.from = 27, .to = 45, .type = BISHOP, .promotion = EMPTY};
  move behind_ally1 = {
      .from = 27, .to = 54, .type = BISHOP, .promotion = EMPTY};
  move behind_ally2 = {
      .from = 27, .to = 63, .type = BISHOP, .promotion = EMPTY};
  CU_ASSERT_FALSE(move_exists(m, on_ally));
  CU_ASSERT_FALSE(move_exists(m, behind_ally1));
  CU_ASSERT_FALSE(move_exists(m, behind_ally2));

  // Free
  free_movelist(m);
  free_chessboard(cb);

  /*
   * ==================================================================
   * TEST PRISE ET NON-DEPASSEMENT DE LA PIECE PRISE : D4 (index 27)
   * ==================================================================
   * Une pièce adverse en f6 (45) peut être prise. Le fou doit pouvoir
   * s'y déplacer (prise) mais ne peut PAS jouer les cases situées
   * derrière elle (g7 = 54, h8 = 63).
   */
  cb = create_empty_chessboard();
  cb->chessboard_color = WHITE;

  // On place le fou en d4 (index 27)
  place_piece(cb, BISHOP, WHITE, 27);
  // Pièce adverse à prendre sur la diagonale haut-droite, en f6 (index 45)
  place_piece(cb, PAWN, BLACK, 45);

  m = create_movelist();
  generate_bishop_move(cb, m);

  // 13 coups - les 2 cases derrière la pièce prise (g7, h8) = 11 coups
  CU_ASSERT_EQUAL(count_moves(m), 11);

  // Le fou peut aller en e5 (36) puis prendre en f6 (45)
  move step_before = {.from = 27, .to = 36, .type = BISHOP, .promotion = EMPTY};
  move capture = {.from = 27,
                  .to = 45,
                  .type = BISHOP,
                  .promotion = EMPTY,
                  .kind = MOVE_CAPTURE};
  CU_ASSERT_TRUE(move_exists(m, step_before));
  CU_ASSERT_TRUE(move_exists(m, capture));

  // Il ne peut PAS jouer derrière la pièce prise (g7 = 54, h8 = 63)
  move behind_capture1 = {
      .from = 27, .to = 54, .type = BISHOP, .promotion = EMPTY};
  move behind_capture2 = {
      .from = 27, .to = 63, .type = BISHOP, .promotion = EMPTY};
  CU_ASSERT_FALSE(move_exists(m, behind_capture1));
  CU_ASSERT_FALSE(move_exists(m, behind_capture2));

  // Free
  free_movelist(m);
  free_chessboard(cb);
}

// --- Tests pour generate_rook_move ---
void test_generate_rook_move(void) {

  /*
   * =========================================================
   * TEST TOUR AU MILIEU SANS AUTRE PIECE : D4 (index 27)
   * =========================================================
   */
  chessboard cb = create_empty_chessboard();
  cb->chessboard_color = WHITE;

  // On place la tour en d4 (index 27)
  place_piece(cb, ROOK, WHITE, 27);

  movelist m = create_movelist();
  generate_rook_move(cb, m);

  // Sur un échiquier vide, la tour en d4 a 14 coups (4 directions)
  CU_ASSERT_EQUAL(count_moves(m), 14);

  move expected_moves1[] = {
      // Vers le bas : d3, d2, d1
      {.from = 27, .to = 19, .type = ROOK, .promotion = EMPTY},
      {.from = 27, .to = 11, .type = ROOK, .promotion = EMPTY},
      {.from = 27, .to = 3, .type = ROOK, .promotion = EMPTY},
      // Vers la gauche : c4, b4, a4
      {.from = 27, .to = 26, .type = ROOK, .promotion = EMPTY},
      {.from = 27, .to = 25, .type = ROOK, .promotion = EMPTY},
      {.from = 27, .to = 24, .type = ROOK, .promotion = EMPTY},
      // Vers la droite : e4, f4, g4, h4
      {.from = 27, .to = 28, .type = ROOK, .promotion = EMPTY},
      {.from = 27, .to = 29, .type = ROOK, .promotion = EMPTY},
      {.from = 27, .to = 30, .type = ROOK, .promotion = EMPTY},
      {.from = 27, .to = 31, .type = ROOK, .promotion = EMPTY},
      // Vers le haut : d5, d6, d7, d8
      {.from = 27, .to = 35, .type = ROOK, .promotion = EMPTY},
      {.from = 27, .to = 43, .type = ROOK, .promotion = EMPTY},
      {.from = 27, .to = 51, .type = ROOK, .promotion = EMPTY},
      {.from = 27, .to = 59, .type = ROOK, .promotion = EMPTY}};

  // TESTS
  for (int i = 0; i < 14; i++) {
    CU_ASSERT_TRUE(move_exists(m, expected_moves1[i]));
  }

  // Free
  free_movelist(m);
  free_chessboard(cb);

  /*
   * ==================================================================
   * TEST PRISE ET NON-DEPASSEMENT DE LA PIECE PRISE : D4 (index 27)
   * ==================================================================
   * Une pièce adverse en f4 (29) peut être prise vers la droite. La tour
   * doit pouvoir s'y déplacer (prise) mais ne peut PAS jouer les cases
   * situées derrière elle (g4 = 30, h4 = 31).
   */
  cb = create_empty_chessboard();
  cb->chessboard_color = WHITE;

  // On place la tour en d4 (index 27)
  place_piece(cb, ROOK, WHITE, 27);
  // Pièce adverse à prendre vers la droite, en f4 (index 29)
  place_piece(cb, PAWN, BLACK, 29);

  m = create_movelist();
  generate_rook_move(cb, m);

  // 14 coups - les 2 cases derrière la pièce prise (g4, h4) = 12 coups
  CU_ASSERT_EQUAL(count_moves(m), 12);

  // La tour peut aller en e4 (28) puis prendre en f4 (29)
  move step_before = {.from = 27, .to = 28, .type = ROOK, .promotion = EMPTY};
  move capture = {.from = 27,
                  .to = 29,
                  .type = ROOK,
                  .promotion = EMPTY,
                  .kind = MOVE_CAPTURE};
  CU_ASSERT_TRUE(move_exists(m, step_before));
  CU_ASSERT_TRUE(move_exists(m, capture));

  // Elle ne peut PAS jouer derrière la pièce prise (g4 = 30, h4 = 31)
  move behind_capture1 = {
      .from = 27, .to = 30, .type = ROOK, .promotion = EMPTY};
  move behind_capture2 = {
      .from = 27, .to = 31, .type = ROOK, .promotion = EMPTY};
  CU_ASSERT_FALSE(move_exists(m, behind_capture1));
  CU_ASSERT_FALSE(move_exists(m, behind_capture2));

  // Free
  free_movelist(m);
  free_chessboard(cb);
}

// --- Tests pour generate_king_move (déplacements simples) ---
void test_generate_king_move(void) {

  /*
   * =========================================================
   * TEST ROI AU MILIEU SANS AUTRE PIECE : D4 (index 27)
   * =========================================================
   */
  chessboard cb = create_empty_chessboard();
  cb->chessboard_color = WHITE;

  // On place le roi en d4 (index 27)
  place_piece(cb, KING, WHITE, 27);

  movelist m = create_movelist();
  generate_king_move(cb, m);

  // Sur un échiquier vide, le roi en d4 a 8 coups
  CU_ASSERT_EQUAL(count_moves(m), 8);

  move expected_moves1[] = {
      {.from = 27, .to = 18, .type = KING, .promotion = EMPTY},
      {.from = 27, .to = 19, .type = KING, .promotion = EMPTY},
      {.from = 27, .to = 20, .type = KING, .promotion = EMPTY},
      {.from = 27, .to = 26, .type = KING, .promotion = EMPTY},
      {.from = 27, .to = 28, .type = KING, .promotion = EMPTY},
      {.from = 27, .to = 34, .type = KING, .promotion = EMPTY},
      {.from = 27, .to = 35, .type = KING, .promotion = EMPTY},
      {.from = 27, .to = 36, .type = KING, .promotion = EMPTY}};

  // TESTS
  for (int i = 0; i < 8; i++) {
    CU_ASSERT_TRUE(move_exists(m, expected_moves1[i]));
  }

  // Free
  free_movelist(m);
  free_chessboard(cb);

  /*
   * =========================================================
   * TESTS COTES STRICTS SEUL (D1, A4, H4, D8)
   * =========================================================
   */
  cb = create_empty_chessboard();
  cb->chessboard_color = WHITE;

  // On place les rois sur les bords
  place_piece(cb, KING, WHITE, 3);
  place_piece(cb, KING, WHITE, 24);
  place_piece(cb, KING, WHITE, 31);
  place_piece(cb, KING, WHITE, 59);

  m = create_movelist();
  generate_king_move(cb, m);

  // Chaque roi sur un bord (hors coin) a 5 coups : 4 rois = 20 coups
  CU_ASSERT_EQUAL(count_moves(m), 20);

  move expected_moves2[] = {
      // D1 (index 3)
      {.from = 3, .to = 2, .type = KING, .promotion = EMPTY},
      {.from = 3, .to = 4, .type = KING, .promotion = EMPTY},
      {.from = 3, .to = 10, .type = KING, .promotion = EMPTY},
      {.from = 3, .to = 11, .type = KING, .promotion = EMPTY},
      {.from = 3, .to = 12, .type = KING, .promotion = EMPTY},
      // A4 (index 24)
      {.from = 24, .to = 16, .type = KING, .promotion = EMPTY},
      {.from = 24, .to = 17, .type = KING, .promotion = EMPTY},
      {.from = 24, .to = 25, .type = KING, .promotion = EMPTY},
      {.from = 24, .to = 32, .type = KING, .promotion = EMPTY},
      {.from = 24, .to = 33, .type = KING, .promotion = EMPTY},
      // H4 (index 31)
      {.from = 31, .to = 22, .type = KING, .promotion = EMPTY},
      {.from = 31, .to = 23, .type = KING, .promotion = EMPTY},
      {.from = 31, .to = 30, .type = KING, .promotion = EMPTY},
      {.from = 31, .to = 38, .type = KING, .promotion = EMPTY},
      {.from = 31, .to = 39, .type = KING, .promotion = EMPTY},
      // D8 (index 59)
      {.from = 59, .to = 50, .type = KING, .promotion = EMPTY},
      {.from = 59, .to = 51, .type = KING, .promotion = EMPTY},
      {.from = 59, .to = 52, .type = KING, .promotion = EMPTY},
      {.from = 59, .to = 58, .type = KING, .promotion = EMPTY},
      {.from = 59, .to = 60, .type = KING, .promotion = EMPTY}};

  // TESTS
  for (int i = 0; i < 20; i++) {
    CU_ASSERT_TRUE(move_exists(m, expected_moves2[i]));
  }

  // Free
  free_movelist(m);
  free_chessboard(cb);

  /*
   * =========================================================
   * TESTS COINS STRICTS SEUL (A1, H1, A8, H8)
   * =========================================================
   */
  cb = create_empty_chessboard();
  cb->chessboard_color = WHITE;

  // On place les rois dans les coins
  place_piece(cb, KING, WHITE, 0);
  place_piece(cb, KING, WHITE, 7);
  place_piece(cb, KING, WHITE, 56);
  place_piece(cb, KING, WHITE, 63);

  m = create_movelist();
  generate_king_move(cb, m);

  // Chaque roi dans un coin a 3 coups : 4 rois = 12 coups
  CU_ASSERT_EQUAL(count_moves(m), 12);

  move expected_moves3[] = {
      // A1 (index 0)
      {.from = 0, .to = 1, .type = KING, .promotion = EMPTY},
      {.from = 0, .to = 8, .type = KING, .promotion = EMPTY},
      {.from = 0, .to = 9, .type = KING, .promotion = EMPTY},
      // H1 (index 7)
      {.from = 7, .to = 6, .type = KING, .promotion = EMPTY},
      {.from = 7, .to = 14, .type = KING, .promotion = EMPTY},
      {.from = 7, .to = 15, .type = KING, .promotion = EMPTY},
      // A8 (index 56)
      {.from = 56, .to = 48, .type = KING, .promotion = EMPTY},
      {.from = 56, .to = 49, .type = KING, .promotion = EMPTY},
      {.from = 56, .to = 57, .type = KING, .promotion = EMPTY},
      // H8 (index 63)
      {.from = 63, .to = 54, .type = KING, .promotion = EMPTY},
      {.from = 63, .to = 55, .type = KING, .promotion = EMPTY},
      {.from = 63, .to = 62, .type = KING, .promotion = EMPTY}};

  // TESTS
  for (int i = 0; i < 12; i++) {
    CU_ASSERT_TRUE(move_exists(m, expected_moves3[i]));
  }

  // Free
  free_movelist(m);
  free_chessboard(cb);

  /*
   * =========================================================
   * TEST BLOCAGE PAR DES PIECES DU JOUEUR (TOUTES DIRECTIONS) : D4 (27)
   * =========================================================
   * Les 8 cases autour du roi sont occupées par des pièces du joueur actif :
   * le roi n'a aucun coup possible.
   */
  cb = create_empty_chessboard();
  cb->chessboard_color = WHITE;

  // On place le roi en d4 (index 27)
  place_piece(cb, KING, WHITE, 27);

  // On entoure le roi de pièces du joueur actif
  cell around[8] = {18, 19, 20, 26, 28, 34, 35, 36};
  for (int i = 0; i < 8; i++) {
    place_piece(cb, KNIGHT, WHITE, around[i]);
  }

  m = create_movelist();
  generate_king_move(cb, m);

  // Toutes les cases sont bloquées par le joueur : aucun coup
  CU_ASSERT_EQUAL(count_moves(m), 0);

  // Aucun coup vers les cases occupées par le joueur ne doit exister
  for (int i = 0; i < 8; i++) {
    move blocked = {
        .from = 27, .to = around[i], .type = KING, .promotion = EMPTY};
    CU_ASSERT_FALSE(move_exists(m, blocked));
  }

  // Free
  free_movelist(m);
  free_chessboard(cb);

  /*
   * =========================================================
   * TEST PRISE : D4 (27) prend des pièces adverses adjacentes
   * =========================================================
   */
  cb = create_empty_chessboard();
  cb->chessboard_color = WHITE;

  // On place le roi en d4 (index 27)
  place_piece(cb, KING, WHITE, 27);

  // Pièces adverses sur deux cases adjacentes : e4 (28) et d5 (35)
  place_piece(cb, PAWN, BLACK, 28);
  place_piece(cb, PAWN, BLACK, 35);

  m = create_movelist();
  generate_king_move(cb, m);

  // Les cases adverses sont accessibles (prises) : le roi garde ses 8 coups
  CU_ASSERT_EQUAL(count_moves(m), 8);

  // Les prises doivent exister
  move capture1 = {.from = 27,
                   .to = 28,
                   .type = KING,
                   .promotion = EMPTY,
                   .kind = MOVE_CAPTURE};
  move capture2 = {.from = 27,
                   .to = 35,
                   .type = KING,
                   .promotion = EMPTY,
                   .kind = MOVE_CAPTURE};
  CU_ASSERT_TRUE(move_exists(m, capture1));
  CU_ASSERT_TRUE(move_exists(m, capture2));

  // Free
  free_movelist(m);
  free_chessboard(cb);

  /*
   * =========================================================
   * TEST DEBORDEMENT DE COLONNE : H4 (index 31)
   * =========================================================
   * Un roi en h4 (colonne H) ne doit pas se déplacer vers la droite : les
   * cases a5 (32), a6 (40) et a4 (24) sont de l'autre côté de l'échiquier.
   * Même occupées par l'adversaire, elles ne doivent pas être atteignables.
   */
  cb = create_empty_chessboard();
  cb->chessboard_color = WHITE;

  // On place le roi en h4 (index 31)
  place_piece(cb, KING, WHITE, 31);

  // Pièces adverses sur les cases de débordement (colonne A)
  place_piece(cb, PAWN, BLACK, 32); // a5
  place_piece(cb, PAWN, BLACK, 40); // a6
  place_piece(cb, PAWN, BLACK, 24); // a4

  m = create_movelist();
  generate_king_move(cb, m);

  // Le roi en h4 n'a que 5 coups légaux (pas de débordement)
  CU_ASSERT_EQUAL(count_moves(m), 5);

  move expected_moves6[] = {
      {.from = 31, .to = 22, .type = KING, .promotion = EMPTY},
      {.from = 31, .to = 23, .type = KING, .promotion = EMPTY},
      {.from = 31, .to = 30, .type = KING, .promotion = EMPTY},
      {.from = 31, .to = 38, .type = KING, .promotion = EMPTY},
      {.from = 31, .to = 39, .type = KING, .promotion = EMPTY}};
  for (int i = 0; i < 5; i++) {
    CU_ASSERT_TRUE(move_exists(m, expected_moves6[i]));
  }

  // Les déplacements débordant sur la colonne A ne doivent PAS exister
  move overflow1 = {.from = 31, .to = 32, .type = KING, .promotion = EMPTY};
  move overflow2 = {.from = 31, .to = 40, .type = KING, .promotion = EMPTY};
  move overflow3 = {.from = 31, .to = 24, .type = KING, .promotion = EMPTY};
  CU_ASSERT_FALSE(move_exists(m, overflow1));
  CU_ASSERT_FALSE(move_exists(m, overflow2));
  CU_ASSERT_FALSE(move_exists(m, overflow3));

  // Free
  free_movelist(m);
  free_chessboard(cb);
}

// --- Tests pour generate_king_move (roque) ---
void test_generate_king_castling(void) {

  /*
   * =========================================================
   * TEST ROQUE VALIDE (PETIT ET GRAND) : roi blanc en e1 (4)
   * =========================================================
   */
  chessboard cb = create_empty_chessboard();
  cb->chessboard_color = WHITE;

  // Roi en e1 (4), tours en a1 (0) et h1 (7)
  place_piece(cb, KING, WHITE, 4);
  place_piece(cb, ROOK, WHITE, 0);
  place_piece(cb, ROOK, WHITE, 7);
  // Droits de roque des deux côtés pour les blancs
  set_castling_right(cb, WHITE_KING_SIDE);
  set_castling_right(cb, WHITE_QUEEN_SIDE);

  movelist m = create_movelist();
  generate_king_move(cb, m);

  // 5 coups simples (d1, f1, d2, e2, f2) + petit roque + grand roque = 7 coups
  CU_ASSERT_EQUAL(count_moves(m), 7);

  // Le petit roque (e1->g1 = 4->6) et le grand roque (e1->c1 = 4->2) existent
  move short_castle = {.from = 4,
                       .to = 6,
                       .type = KING,
                       .promotion = EMPTY,
                       .kind = MOVE_CASTLE_KING};
  move long_castle = {.from = 4,
                      .to = 2,
                      .type = KING,
                      .promotion = EMPTY,
                      .kind = MOVE_CASTLE_QUEEN};
  CU_ASSERT_TRUE(move_exists(m, short_castle));
  CU_ASSERT_TRUE(move_exists(m, long_castle));

  // Free
  free_movelist(m);
  free_chessboard(cb);

  /*
   * =========================================================
   * TEST ROQUE BLOQUE PAR DES PIECES ENTRE LE ROI ET LA TOUR
   * =========================================================
   */
  cb = create_empty_chessboard();
  cb->chessboard_color = WHITE;

  place_piece(cb, KING, WHITE, 4);
  place_piece(cb, ROOK, WHITE, 0);
  place_piece(cb, ROOK, WHITE, 7);
  set_castling_right(cb, WHITE_KING_SIDE);
  set_castling_right(cb, WHITE_QUEEN_SIDE);
  // Pièces du joueur qui bloquent : f1 (5) côté roi, b1 (1) côté dame
  place_piece(cb, BISHOP, WHITE, 5);
  place_piece(cb, KNIGHT, WHITE, 1);

  m = create_movelist();
  generate_king_move(cb, m);

  // Aucun roque n'est possible (cases intermédiaires occupées)
  move short_castle_b = {.from = 4,
                         .to = 6,
                         .type = KING,
                         .promotion = EMPTY,
                         .kind = MOVE_CASTLE_KING};
  move long_castle_b = {.from = 4,
                        .to = 2,
                        .type = KING,
                        .promotion = EMPTY,
                        .kind = MOVE_CASTLE_QUEEN};
  CU_ASSERT_FALSE(move_exists(m, short_castle_b));
  CU_ASSERT_FALSE(move_exists(m, long_castle_b));

  // Free
  free_movelist(m);
  free_chessboard(cb);

  /*
   * =========================================================
   * TEST ROQUE INTERDIT : UNE CASE DE TRANSIT EST VISEE
   * =========================================================
   * Une tour adverse en g8 (62) vise g1 (6), case traversée par le petit
   * roque : le roque côté roi est interdit.
   */
  cb = create_empty_chessboard();
  cb->chessboard_color = WHITE;

  place_piece(cb, KING, WHITE, 4);
  place_piece(cb, ROOK, WHITE, 7);
  set_castling_right(cb, WHITE_KING_SIDE);
  // Tour adverse visant g1 (6)
  place_piece(cb, ROOK, BLACK, 62);

  m = create_movelist();
  generate_king_move(cb, m);

  // La case g1 étant visée, le petit roque est interdit
  move short_castle_c = {.from = 4,
                         .to = 6,
                         .type = KING,
                         .promotion = EMPTY,
                         .kind = MOVE_CASTLE_KING};
  CU_ASSERT_FALSE(move_exists(m, short_castle_c));

  // Free
  free_movelist(m);
  free_chessboard(cb);

  /*
   * =========================================================
   * TEST ROQUE INTERDIT : LE ROI OU LA TOUR A DEJA BOUGE
   * =========================================================
   * Les pièces sont en place et le chemin est libre, mais le droit de roque
   * a été perdu (le roi ou la tour a bougé auparavant).
   */
  cb = create_empty_chessboard();
  cb->chessboard_color = WHITE;

  place_piece(cb, KING, WHITE, 4);
  place_piece(cb, ROOK, WHITE, 0);
  place_piece(cb, ROOK, WHITE, 7);
  // On accorde puis on retire les droits : simule un roi/tour ayant déjà bougé
  set_castling_right(cb, WHITE_KING_SIDE);
  set_castling_right(cb, WHITE_QUEEN_SIDE);
  remove_castling_right(cb, WHITE_KING_SIDE);
  remove_castling_right(cb, WHITE_QUEEN_SIDE);

  m = create_movelist();
  generate_king_move(cb, m);

  // Sans le droit de roque, aucun roque n'est proposé
  move short_castle_d = {.from = 4,
                         .to = 6,
                         .type = KING,
                         .promotion = EMPTY,
                         .kind = MOVE_CASTLE_KING};
  move long_castle_d = {.from = 4,
                        .to = 2,
                        .type = KING,
                        .promotion = EMPTY,
                        .kind = MOVE_CASTLE_QUEEN};
  CU_ASSERT_FALSE(move_exists(m, short_castle_d));
  CU_ASSERT_FALSE(move_exists(m, long_castle_d));

  // Free
  free_movelist(m);
  free_chessboard(cb);

  /*
   * =========================================================
   * TEST ROQUE VALIDE POUR LES NOIRS (PETIT ET GRAND)
   * =========================================================
   */
  cb = create_empty_chessboard();
  cb->chessboard_color = BLACK;

  // Roi noir en e8 (60), tours en a8 (56) et h8 (63)
  place_piece(cb, KING, BLACK, 60);
  place_piece(cb, ROOK, BLACK, 56);
  place_piece(cb, ROOK, BLACK, 63);
  set_castling_right(cb, BLACK_KING_SIDE);
  set_castling_right(cb, BLACK_QUEEN_SIDE);

  m = create_movelist();
  generate_king_move(cb, m);

  // 5 coups simples + petit roque (60->62) + grand roque (60->58) = 7 coups
  CU_ASSERT_EQUAL(count_moves(m), 7);

  move short_castle_black = {.from = 60,
                             .to = 62,
                             .type = KING,
                             .promotion = EMPTY,
                             .kind = MOVE_CASTLE_KING};
  move long_castle_black = {.from = 60,
                            .to = 58,
                            .type = KING,
                            .promotion = EMPTY,
                            .kind = MOVE_CASTLE_QUEEN};
  CU_ASSERT_TRUE(move_exists(m, short_castle_black));
  CU_ASSERT_TRUE(move_exists(m, long_castle_black));

  // Free
  free_movelist(m);
  free_chessboard(cb);

  /*
   * ==================================================================
   * TEST ROQUE AUTORISE : UNE PIECE ALLIEE PROTEGE LA CASE DE TRANSIT
   * ==================================================================
   * Une tour adverse en g8 (62) viserait g1 (6), mais un pion allié en g4
   * (30) coupe la ligne d'attaque. La case g1 n'est donc plus visée et le
   * petit roque redevient possible.
   */
  cb = create_empty_chessboard();
  cb->chessboard_color = WHITE;

  place_piece(cb, KING, WHITE, 4);
  place_piece(cb, ROOK, WHITE, 7);
  set_castling_right(cb, WHITE_KING_SIDE);
  // Tour adverse qui viserait g1 (6)
  place_piece(cb, ROOK, BLACK, 62);
  // Pion allié qui coupe la ligne de la tour adverse en g4 (30)
  place_piece(cb, PAWN, WHITE, 30);

  m = create_movelist();
  generate_king_move(cb, m);

  // La ligne d'attaque est coupée : g1 n'est plus visée, le petit roque est
  // possible
  move short_castle_f = {.from = 4,
                         .to = 6,
                         .type = KING,
                         .promotion = EMPTY,
                         .kind = MOVE_CASTLE_KING};
  CU_ASSERT_TRUE(move_exists(m, short_castle_f));

  // Free
  free_movelist(m);
  free_chessboard(cb);

  /*
   * ==================================================================
   * TEST ROQUE INTERDIT : LA TOUR N'EST PAS PRESENTE
   * ==================================================================
   * Le droit de roque est positionné et le chemin est libre, mais aucune
   * tour n'est en h1 : le roque ne doit PAS être généré.
   */
  cb = create_empty_chessboard();
  cb->chessboard_color = WHITE;

  // Roi en e1 (4), droit de petit roque, mais AUCUNE tour en h1
  place_piece(cb, KING, WHITE, 4);
  set_castling_right(cb, WHITE_KING_SIDE);

  m = create_movelist();
  generate_king_move(cb, m);

  // Sans tour en h1, le petit roque ne doit pas exister
  move castle_no_rook = {.from = 4,
                         .to = 6,
                         .type = KING,
                         .promotion = EMPTY,
                         .kind = MOVE_CASTLE_KING};
  CU_ASSERT_FALSE(move_exists(m, castle_no_rook));

  // Free
  free_movelist(m);
  free_chessboard(cb);
}

// --- Tests pour generate_queen_move ---
void test_generate_queen_move(void) {

  /*
   * =========================================================
   * TEST REINE AU MILIEU SANS AUTRE PIECE : D4 (index 27)
   * =========================================================
   * La reine combine les coups de la tour (14) et du fou (13) = 27 coups.
   */
  chessboard cb = create_empty_chessboard();
  cb->chessboard_color = WHITE;

  // On place la reine en d4 (index 27)
  place_piece(cb, QUEEN, WHITE, 27);

  movelist m = create_movelist();
  generate_queen_move(cb, m);

  // Sur un échiquier vide, la reine en d4 a 27 coups
  CU_ASSERT_EQUAL(count_moves(m), 27);

  move expected_moves1[] = {
      // Coups droits (tour) : bas, gauche, droite, haut
      {.from = 27, .to = 19, .type = QUEEN, .promotion = EMPTY},
      {.from = 27, .to = 11, .type = QUEEN, .promotion = EMPTY},
      {.from = 27, .to = 3, .type = QUEEN, .promotion = EMPTY},
      {.from = 27, .to = 26, .type = QUEEN, .promotion = EMPTY},
      {.from = 27, .to = 25, .type = QUEEN, .promotion = EMPTY},
      {.from = 27, .to = 24, .type = QUEEN, .promotion = EMPTY},
      {.from = 27, .to = 28, .type = QUEEN, .promotion = EMPTY},
      {.from = 27, .to = 29, .type = QUEEN, .promotion = EMPTY},
      {.from = 27, .to = 30, .type = QUEEN, .promotion = EMPTY},
      {.from = 27, .to = 31, .type = QUEEN, .promotion = EMPTY},
      {.from = 27, .to = 35, .type = QUEEN, .promotion = EMPTY},
      {.from = 27, .to = 43, .type = QUEEN, .promotion = EMPTY},
      {.from = 27, .to = 51, .type = QUEEN, .promotion = EMPTY},
      {.from = 27, .to = 59, .type = QUEEN, .promotion = EMPTY},
      // Coups diagonaux (fou) : bas-gauche, bas-droite, haut-gauche,
      // haut-droite
      {.from = 27, .to = 18, .type = QUEEN, .promotion = EMPTY},
      {.from = 27, .to = 9, .type = QUEEN, .promotion = EMPTY},
      {.from = 27, .to = 0, .type = QUEEN, .promotion = EMPTY},
      {.from = 27, .to = 20, .type = QUEEN, .promotion = EMPTY},
      {.from = 27, .to = 13, .type = QUEEN, .promotion = EMPTY},
      {.from = 27, .to = 6, .type = QUEEN, .promotion = EMPTY},
      {.from = 27, .to = 34, .type = QUEEN, .promotion = EMPTY},
      {.from = 27, .to = 41, .type = QUEEN, .promotion = EMPTY},
      {.from = 27, .to = 48, .type = QUEEN, .promotion = EMPTY},
      {.from = 27, .to = 36, .type = QUEEN, .promotion = EMPTY},
      {.from = 27, .to = 45, .type = QUEEN, .promotion = EMPTY},
      {.from = 27, .to = 54, .type = QUEEN, .promotion = EMPTY},
      {.from = 27, .to = 63, .type = QUEEN, .promotion = EMPTY}};

  // TESTS
  for (int i = 0; i < 27; i++) {
    CU_ASSERT_TRUE(move_exists(m, expected_moves1[i]));
  }

  // Free
  free_movelist(m);
  free_chessboard(cb);

  /*
   * =========================================================
   * TEST REINE DANS UN COIN : A1 (index 0)
   * =========================================================
   * Depuis un coin, la reine a 21 coups (7 en colonne, 7 en ligne, 7 en
   * diagonale).
   */
  cb = create_empty_chessboard();
  cb->chessboard_color = WHITE;

  // On place la reine en a1 (index 0)
  place_piece(cb, QUEEN, WHITE, 0);

  m = create_movelist();
  generate_queen_move(cb, m);

  // Depuis a1, la reine a 21 coups
  CU_ASSERT_EQUAL(count_moves(m), 21);

  move expected_moves2[] = {
      // Colonne a : a2..a8
      {.from = 0, .to = 8, .type = QUEEN, .promotion = EMPTY},
      {.from = 0, .to = 16, .type = QUEEN, .promotion = EMPTY},
      {.from = 0, .to = 24, .type = QUEEN, .promotion = EMPTY},
      {.from = 0, .to = 32, .type = QUEEN, .promotion = EMPTY},
      {.from = 0, .to = 40, .type = QUEEN, .promotion = EMPTY},
      {.from = 0, .to = 48, .type = QUEEN, .promotion = EMPTY},
      {.from = 0, .to = 56, .type = QUEEN, .promotion = EMPTY},
      // Ligne 1 : b1..h1
      {.from = 0, .to = 1, .type = QUEEN, .promotion = EMPTY},
      {.from = 0, .to = 2, .type = QUEEN, .promotion = EMPTY},
      {.from = 0, .to = 3, .type = QUEEN, .promotion = EMPTY},
      {.from = 0, .to = 4, .type = QUEEN, .promotion = EMPTY},
      {.from = 0, .to = 5, .type = QUEEN, .promotion = EMPTY},
      {.from = 0, .to = 6, .type = QUEEN, .promotion = EMPTY},
      {.from = 0, .to = 7, .type = QUEEN, .promotion = EMPTY},
      // Diagonale a1-h8 : b2..h8
      {.from = 0, .to = 9, .type = QUEEN, .promotion = EMPTY},
      {.from = 0, .to = 18, .type = QUEEN, .promotion = EMPTY},
      {.from = 0, .to = 27, .type = QUEEN, .promotion = EMPTY},
      {.from = 0, .to = 36, .type = QUEEN, .promotion = EMPTY},
      {.from = 0, .to = 45, .type = QUEEN, .promotion = EMPTY},
      {.from = 0, .to = 54, .type = QUEEN, .promotion = EMPTY},
      {.from = 0, .to = 63, .type = QUEEN, .promotion = EMPTY}};

  // TESTS
  for (int i = 0; i < 21; i++) {
    CU_ASSERT_TRUE(move_exists(m, expected_moves2[i]));
  }

  // Free
  free_movelist(m);
  free_chessboard(cb);

  /*
   * =========================================================
   * TEST DEBORDEMENT DE COLONNE : A4 (index 24)
   * =========================================================
   * En a4 (colonne A), la diagonale bas-gauche déborderait sur h2 (15),
   * de l'autre côté de l'échiquier. Une pièce adverse y est placée pour
   * vérifier que la reine ne la prend pas après débordement.
   */
  cb = create_empty_chessboard();
  cb->chessboard_color = WHITE;

  // On place la reine en a4 (index 24)
  place_piece(cb, QUEEN, WHITE, 24);
  // Pièce adverse sur la case de débordement h2 (15), inatteignable
  place_piece(cb, PAWN, BLACK, 15);

  m = create_movelist();
  generate_queen_move(cb, m);

  // La reine en a4 a 21 coups (aucun débordement)
  CU_ASSERT_EQUAL(count_moves(m), 21);

  // La prise par débordement de colonne (h2 = 15) ne doit PAS exister
  move overflow_move = {
      .from = 24, .to = 15, .type = QUEEN, .promotion = EMPTY};
  CU_ASSERT_FALSE(move_exists(m, overflow_move));

  // Free
  free_movelist(m);
  free_chessboard(cb);

  /*
   * ==================================================================
   * TEST BLOCAGE PAR DES PIECES DU JOUEUR (DIAGONALE ET LIGNE) : D4 (27)
   * ==================================================================
   * Une pièce alliée en f6 (45) bloque la diagonale haut-droite et une
   * autre en f4 (29) bloque la ligne vers la droite. La reine ne peut ni
   * s'y déplacer ni passer derrière.
   */
  cb = create_empty_chessboard();
  cb->chessboard_color = WHITE;

  // On place la reine en d4 (index 27)
  place_piece(cb, QUEEN, WHITE, 27);
  // Pièces alliées qui bloquent : f6 (45) en diagonale, f4 (29) en ligne
  place_piece(cb, KNIGHT, WHITE, 45);
  place_piece(cb, KNIGHT, WHITE, 29);

  m = create_movelist();
  generate_queen_move(cb, m);

  // 27 coups - 3 (diagonale : 45, 54, 63) - 3 (ligne : 29, 30, 31) = 21 coups
  CU_ASSERT_EQUAL(count_moves(m), 21);

  // La reine s'arrête juste avant les pièces alliées (e5 = 36 et e4 = 28)
  move before_diag = {.from = 27, .to = 36, .type = QUEEN, .promotion = EMPTY};
  move before_line = {.from = 27, .to = 28, .type = QUEEN, .promotion = EMPTY};
  CU_ASSERT_TRUE(move_exists(m, before_diag));
  CU_ASSERT_TRUE(move_exists(m, before_line));

  // Elle ne peut ni aller sur les cases alliées, ni derrière
  move on_ally_diag = {.from = 27, .to = 45, .type = QUEEN, .promotion = EMPTY};
  move behind_diag = {.from = 27, .to = 54, .type = QUEEN, .promotion = EMPTY};
  move on_ally_line = {.from = 27, .to = 29, .type = QUEEN, .promotion = EMPTY};
  move behind_line = {.from = 27, .to = 30, .type = QUEEN, .promotion = EMPTY};
  CU_ASSERT_FALSE(move_exists(m, on_ally_diag));
  CU_ASSERT_FALSE(move_exists(m, behind_diag));
  CU_ASSERT_FALSE(move_exists(m, on_ally_line));
  CU_ASSERT_FALSE(move_exists(m, behind_line));

  // Free
  free_movelist(m);
  free_chessboard(cb);

  /*
   * =========================================================
   * TEST PRISE : D4 (27) prend des pièces adverses adjacentes
   * =========================================================
   */
  cb = create_empty_chessboard();
  cb->chessboard_color = WHITE;

  // On place la reine en d4 (index 27)
  place_piece(cb, QUEEN, WHITE, 27);
  // Pièces adverses adjacentes : e4 (28) en ligne, d5 (35) en colonne
  place_piece(cb, PAWN, BLACK, 28);
  place_piece(cb, PAWN, BLACK, 35);

  m = create_movelist();
  generate_queen_move(cb, m);

  // 27 coups - 3 (ligne droite derrière 28) - 3 (colonne derrière 35) = 21
  // coups
  CU_ASSERT_EQUAL(count_moves(m), 21);

  // Les prises doivent exister
  move capture_line = {.from = 27,
                       .to = 28,
                       .type = QUEEN,
                       .promotion = EMPTY,
                       .kind = MOVE_CAPTURE};
  move capture_col = {.from = 27,
                      .to = 35,
                      .type = QUEEN,
                      .promotion = EMPTY,
                      .kind = MOVE_CAPTURE};
  CU_ASSERT_TRUE(move_exists(m, capture_line));
  CU_ASSERT_TRUE(move_exists(m, capture_col));

  // Free
  free_movelist(m);
  free_chessboard(cb);

  /*
   * ==================================================================
   * TEST PRISE ET NON-DEPASSEMENT (DIAGONALE ET LIGNE) : D4 (27)
   * ==================================================================
   * Une pièce adverse en f6 (45, diagonale) et une en f4 (29, ligne)
   * peuvent être prises, mais la reine ne peut pas jouer les cases situées
   * derrière elles.
   */
  cb = create_empty_chessboard();
  cb->chessboard_color = WHITE;

  // On place la reine en d4 (index 27)
  place_piece(cb, QUEEN, WHITE, 27);
  // Pièces adverses à prendre : f6 (45) en diagonale, f4 (29) en ligne
  place_piece(cb, PAWN, BLACK, 45);
  place_piece(cb, PAWN, BLACK, 29);

  m = create_movelist();
  generate_queen_move(cb, m);

  // 27 coups - 2 (derrière 45 : 54, 63) - 2 (derrière 29 : 30, 31) = 23 coups
  CU_ASSERT_EQUAL(count_moves(m), 23);

  // La reine peut avancer puis prendre, en diagonale comme en ligne
  move step_diag = {.from = 27, .to = 36, .type = QUEEN, .promotion = EMPTY};
  move capture_diag = {.from = 27,
                       .to = 45,
                       .type = QUEEN,
                       .promotion = EMPTY,
                       .kind = MOVE_CAPTURE};
  move step_line = {.from = 27, .to = 28, .type = QUEEN, .promotion = EMPTY};
  move capture_line2 = {.from = 27,
                        .to = 29,
                        .type = QUEEN,
                        .promotion = EMPTY,
                        .kind = MOVE_CAPTURE};
  CU_ASSERT_TRUE(move_exists(m, step_diag));
  CU_ASSERT_TRUE(move_exists(m, capture_diag));
  CU_ASSERT_TRUE(move_exists(m, step_line));
  CU_ASSERT_TRUE(move_exists(m, capture_line2));

  // Elle ne peut PAS jouer derrière les pièces prises
  move behind_diag2 = {.from = 27, .to = 54, .type = QUEEN, .promotion = EMPTY};
  move behind_diag3 = {.from = 27, .to = 63, .type = QUEEN, .promotion = EMPTY};
  move behind_line2 = {.from = 27, .to = 30, .type = QUEEN, .promotion = EMPTY};
  move behind_line3 = {.from = 27, .to = 31, .type = QUEEN, .promotion = EMPTY};
  CU_ASSERT_FALSE(move_exists(m, behind_diag2));
  CU_ASSERT_FALSE(move_exists(m, behind_diag3));
  CU_ASSERT_FALSE(move_exists(m, behind_line2));
  CU_ASSERT_FALSE(move_exists(m, behind_line3));

  // Free
  free_movelist(m);
  free_chessboard(cb);
}

// --- Tests de la légalité des coups (generate_move) ---
void test_legal_moves(void) {
  /*
   * generate_move ne renvoie que des coups LEGAUX : le roi du joueur au trait
   * ne doit jamais rester (ou se mettre) en échec après le coup. Cela couvre
   * le roi qui irait sur une case attaquée, la parade forcée sous échec, et
   * les pièces clouées.
   */

  /*
   * =========================================================
   * LE ROI NE PEUT PAS SE DEPLACER SUR UNE CASE ATTAQUEE
   * =========================================================
   * Tour noire en d8 (59) : les cases d1 (3) et d2 (11) sont visées.
   * Le roi blanc e1 (4) ne peut y aller.
   */
  chessboard cb = create_empty_chessboard();
  cb->chessboard_color = WHITE;
  place_piece(cb, KING, WHITE, 4);
  place_piece(cb, ROOK, BLACK, 59); // d8 vise la colonne d
  place_piece(cb, KING, BLACK, 56); // roi noir en a8 (position valide)

  movelist m = generate_move(cb);

  // Seules f1 (5), e2 (12) et f2 (13) sont légales : 3 coups
  CU_ASSERT_EQUAL(count_moves(m), 3);

  // Les cases visées sont exclues
  move into_d1 = {.from = 4, .to = 3, .type = KING, .promotion = EMPTY};
  move into_d2 = {.from = 4, .to = 11, .type = KING, .promotion = EMPTY};
  CU_ASSERT_FALSE(move_exists(m, into_d1));
  CU_ASSERT_FALSE(move_exists(m, into_d2));

  // Les cases sûres restent jouables
  move safe_f1 = {.from = 4, .to = 5, .type = KING, .promotion = EMPTY};
  move safe_e2 = {.from = 4, .to = 12, .type = KING, .promotion = EMPTY};
  CU_ASSERT_TRUE(move_exists(m, safe_f1));
  CU_ASSERT_TRUE(move_exists(m, safe_e2));

  free_movelist(m);
  free_chessboard(cb);

  /*
   * =========================================================
   * SOUS ECHEC : SEULES LES PARADES SONT LEGALES
   * =========================================================
   * Le roi blanc e1 (4) est en échec par la tour noire e8 (60). Les coups
   * qui ne parent pas l'échec (a2-a3) sont exclus ; l'interposition du
   * cavalier g1 en e2 et la fuite du roi restent légales.
   */
  cb = create_empty_chessboard();
  cb->chessboard_color = WHITE;
  place_piece(cb, KING, WHITE, 4);
  place_piece(cb, ROOK, BLACK, 60);  // e8 met le roi en échec
  place_piece(cb, PAWN, WHITE, 8);   // a2
  place_piece(cb, KNIGHT, WHITE, 6); // g1
  place_piece(cb, KING, BLACK, 56);  // roi noir en a8

  m = generate_move(cb);

  // 4 fuites du roi (d1, f1, d2, f2) + 1 interposition (Cg1-e2) = 5 coups
  CU_ASSERT_EQUAL(count_moves(m), 5);

  // Un coup sans rapport avec l'échec est exclu
  move a2a3 = {.from = 8, .to = 16, .type = PAWN, .promotion = EMPTY};
  CU_ASSERT_FALSE(move_exists(m, a2a3));

  // Le roi ne peut pas rester sur la colonne e (e2 = 12 toujours attaquée)
  move stay_e2 = {.from = 4, .to = 12, .type = KING, .promotion = EMPTY};
  CU_ASSERT_FALSE(move_exists(m, stay_e2));

  // L'interposition du cavalier (Cg1-e2 : 6 -> 12) et la fuite du roi (Kd1)
  // sont légales
  move block_e2 = {.from = 6, .to = 12, .type = KNIGHT, .promotion = EMPTY};
  move flee_d1 = {.from = 4, .to = 3, .type = KING, .promotion = EMPTY};
  CU_ASSERT_TRUE(move_exists(m, block_e2));
  CU_ASSERT_TRUE(move_exists(m, flee_d1));

  free_movelist(m);
  free_chessboard(cb);

  /*
   * =========================================================
   * PIECE CLOUEE : ELLE NE PEUT PAS BOUGER (CLOUAGE)
   * =========================================================
   * Le cavalier blanc e2 (12) est cloué sur son roi e1 (4) par la tour
   * noire e8 (60) : bouger le cavalier exposerait le roi, c'est interdit.
   */
  cb = create_empty_chessboard();
  cb->chessboard_color = WHITE;
  place_piece(cb, KING, WHITE, 4);
  place_piece(cb, KNIGHT, WHITE, 12); // e2, cloué
  place_piece(cb, ROOK, BLACK, 60);   // e8
  place_piece(cb, KING, BLACK, 56);   // roi noir en a8

  m = generate_move(cb);

  // Le cavalier cloué est immobile : seuls les 4 coups du roi (d1, f1, d2, f2)
  // restent
  CU_ASSERT_EQUAL(count_moves(m), 4);

  // Un coup du cavalier cloué (Ce2-c3 : 12 -> 18) est exclu
  move pinned_knight = {
      .from = 12, .to = 18, .type = KNIGHT, .promotion = EMPTY};
  CU_ASSERT_FALSE(move_exists(m, pinned_knight));

  // Le roi peut toujours se déplacer sur une case sûre
  move king_d1 = {.from = 4, .to = 3, .type = KING, .promotion = EMPTY};
  CU_ASSERT_TRUE(move_exists(m, king_d1));

  free_movelist(m);
  free_chessboard(cb);
}

/**
 * \brief move_kind_of(m, from, to, promotion) retourne le champ kind du coup
 * correspondant dans \b m, ou -1 s'il est absent.
 */
static int move_kind_of(movelist m, cell from, cell to, piecetype promotion) {
  for (movelist p = get_next(m); p != NULL; p = get_next(p)) {
    move c = get_current_move(p);
    if (c.from == from && c.to == to && c.promotion == promotion) {
      return (int)c.kind;
    }
  }
  return -1;
}

// --- Tests des métadonnées de coup (champ kind) ---
void test_move_metadata(void) {

  /*
   * =========================================================
   * CAVALIER : MOVE_NORMAL vs MOVE_CAPTURE
   * =========================================================
   */
  chessboard cb = create_empty_chessboard();
  cb->chessboard_color = WHITE;
  place_piece(cb, KNIGHT, WHITE, 27); // d4
  place_piece(cb, PAWN, BLACK, 10);   // c2 : cible d'une prise

  movelist m = create_movelist();
  generate_knight_move(cb, m);

  CU_ASSERT_EQUAL(move_kind_of(m, 27, 10, EMPTY), MOVE_CAPTURE);
  CU_ASSERT_EQUAL(move_kind_of(m, 27, 12, EMPTY), MOVE_NORMAL);

  free_movelist(m);
  free_chessboard(cb);

  /*
   * =========================================================
   * PION : MOVE_NORMAL, MOVE_DOUBLE_PUSH, MOVE_CAPTURE
   * =========================================================
   */
  cb = create_empty_chessboard();
  cb->chessboard_color = WHITE;
  place_piece(cb, PAWN, WHITE,
              8); // a2 : pour l'avance simple et la double poussée
  place_piece(cb, PAWN, WHITE, 28); // e4 : pour la prise
  place_piece(cb, PAWN, BLACK, 35); // d5 : cible de la prise

  m = create_movelist();
  generate_pawn_move(cb, m);

  CU_ASSERT_EQUAL(move_kind_of(m, 8, 16, EMPTY), MOVE_NORMAL);      // a2-a3
  CU_ASSERT_EQUAL(move_kind_of(m, 8, 24, EMPTY), MOVE_DOUBLE_PUSH); // a2-a4
  CU_ASSERT_EQUAL(move_kind_of(m, 28, 35, EMPTY), MOVE_CAPTURE);    // e4xd5

  free_movelist(m);
  free_chessboard(cb);

  /*
   * =========================================================
   * PION : MOVE_EN_PASSANT
   * =========================================================
   */
  cb = create_empty_chessboard();
  cb->chessboard_color = WHITE;
  place_piece(cb, PAWN, WHITE, 36); // e5
  place_piece(cb, PAWN, BLACK, 35); // d5
  set_en_passant_target(cb, 43);    // d6

  m = create_movelist();
  generate_pawn_move(cb, m);

  CU_ASSERT_EQUAL(move_kind_of(m, 36, 43, EMPTY),
                  MOVE_EN_PASSANT); // prise en passant
  CU_ASSERT_EQUAL(move_kind_of(m, 36, 44, EMPTY), MOVE_NORMAL); // avance simple

  free_movelist(m);
  free_chessboard(cb);

  /*
   * =========================================================
   * ROI : MOVE_CASTLE_KING et MOVE_CASTLE_QUEEN
   * =========================================================
   */
  cb = create_empty_chessboard();
  cb->chessboard_color = WHITE;
  place_piece(cb, KING, WHITE, 4);
  place_piece(cb, ROOK, WHITE, 0);
  place_piece(cb, ROOK, WHITE, 7);
  set_castling_right(cb, WHITE_KING_SIDE);
  set_castling_right(cb, WHITE_QUEEN_SIDE);

  m = create_movelist();
  generate_king_move(cb, m);

  CU_ASSERT_EQUAL(move_kind_of(m, 4, 6, EMPTY),
                  MOVE_CASTLE_KING); // petit roque
  CU_ASSERT_EQUAL(move_kind_of(m, 4, 2, EMPTY),
                  MOVE_CASTLE_QUEEN);                         // grand roque
  CU_ASSERT_EQUAL(move_kind_of(m, 4, 5, EMPTY), MOVE_NORMAL); // coup simple

  free_movelist(m);
  free_chessboard(cb);

  /*
   * =========================================================
   * PROMOTION : par poussée (MOVE_NORMAL) et par prise (MOVE_CAPTURE)
   * =========================================================
   */
  cb = create_empty_chessboard();
  cb->chessboard_color = WHITE;
  place_piece(cb, PAWN, WHITE, 49);   // b7
  place_piece(cb, ROOK, BLACK, 56);   // a8 : prise en promotion
  place_piece(cb, KNIGHT, BLACK, 58); // c8 : prise en promotion

  m = create_movelist();
  generate_pawn_move(cb, m);

  CU_ASSERT_EQUAL(move_kind_of(m, 49, 57, QUEEN),
                  MOVE_NORMAL); // b8=D par poussée
  CU_ASSERT_EQUAL(move_kind_of(m, 49, 56, QUEEN), MOVE_CAPTURE);  // bxa8=D
  CU_ASSERT_EQUAL(move_kind_of(m, 49, 58, KNIGHT), MOVE_CAPTURE); // bxc8=C

  free_movelist(m);
  free_chessboard(cb);

  /*
   * =========================================================
   * PIECE SLIDING (TOUR) : MOVE_NORMAL vs MOVE_CAPTURE
   * =========================================================
   */
  cb = create_empty_chessboard();
  cb->chessboard_color = WHITE;
  place_piece(cb, ROOK, WHITE, 27); // d4
  place_piece(cb, PAWN, BLACK, 29); // f4 : prise (derrière = inaccessible)

  m = create_movelist();
  generate_rook_move(cb, m);

  CU_ASSERT_EQUAL(move_kind_of(m, 27, 28, EMPTY), MOVE_NORMAL);  // e4
  CU_ASSERT_EQUAL(move_kind_of(m, 27, 29, EMPTY), MOVE_CAPTURE); // f4 (prise)

  free_movelist(m);
  free_chessboard(cb);
}

/**
 * \brief Place la position de départ standard des échecs sur l'échiquier \b cb.
 */
static void place_startpos(chessboard cb) {
  // Pièces blanches (rangées 1 et 2)
  place_piece(cb, ROOK, WHITE, 0);
  place_piece(cb, KNIGHT, WHITE, 1);
  place_piece(cb, BISHOP, WHITE, 2);
  place_piece(cb, QUEEN, WHITE, 3);
  place_piece(cb, KING, WHITE, 4);
  place_piece(cb, BISHOP, WHITE, 5);
  place_piece(cb, KNIGHT, WHITE, 6);
  place_piece(cb, ROOK, WHITE, 7);
  for (cell i = 8; i < 16; i++)
    place_piece(cb, PAWN, WHITE, i);
  // Pièces noires (rangées 7 et 8)
  for (cell i = 48; i < 56; i++)
    place_piece(cb, PAWN, BLACK, i);
  place_piece(cb, ROOK, BLACK, 56);
  place_piece(cb, KNIGHT, BLACK, 57);
  place_piece(cb, BISHOP, BLACK, 58);
  place_piece(cb, QUEEN, BLACK, 59);
  place_piece(cb, KING, BLACK, 60);
  place_piece(cb, BISHOP, BLACK, 61);
  place_piece(cb, KNIGHT, BLACK, 62);
  place_piece(cb, ROOK, BLACK, 63);
}

// --- Tests pour generate_move (toutes les pièces) ---
void test_generate_move(void) {

  /*
   * =========================================================
   * POSITION DE DEPART : les blancs ont 20 coups
   * =========================================================
   */
  chessboard cb = create_empty_chessboard();
  cb->chessboard_color = WHITE;
  place_startpos(cb);
  cb->castling_rights = 0x0F; // tous les droits de roque (sans effet : cases
                              // entre pièces occupées)

  movelist m = generate_move(cb);

  // 16 coups de pions (8 x avance simple/double) + 4 coups de cavaliers = 20
  CU_ASSERT_EQUAL(count_moves(m), 20);

  // Quelques coups d'ouverture classiques
  move e2e4 = {.from = 12,
               .to = 28,
               .type = PAWN,
               .promotion = EMPTY,
               .kind = MOVE_DOUBLE_PUSH};
  move a2a4 = {.from = 8,
               .to = 24,
               .type = PAWN,
               .promotion = EMPTY,
               .kind = MOVE_DOUBLE_PUSH};
  move nb1c3 = {.from = 1, .to = 18, .type = KNIGHT, .promotion = EMPTY};
  move ng1f3 = {.from = 6, .to = 21, .type = KNIGHT, .promotion = EMPTY};
  CU_ASSERT_TRUE(move_exists(m, e2e4));
  CU_ASSERT_TRUE(move_exists(m, a2a4));
  CU_ASSERT_TRUE(move_exists(m, nb1c3));
  CU_ASSERT_TRUE(move_exists(m, ng1f3));

  free_movelist(m);
  free_chessboard(cb);

  /*
   * =========================================================
   * POSITION AVEC EN PASSANT : roi e1, roi e8, pions e5/d5, cible d6
   * =========================================================
   */
  cb = create_empty_chessboard();
  cb->chessboard_color = WHITE;
  place_piece(cb, KING, WHITE, 4);
  place_piece(cb, KING, BLACK, 60);
  place_piece(cb, PAWN, WHITE, 36); // e5
  place_piece(cb, PAWN, BLACK, 35); // d5 (vient de jouer d7-d5)
  set_en_passant_target(cb, 43);    // d6

  m = generate_move(cb);

  // 5 coups de roi + avance e6 + prise en passant d6 = 7
  CU_ASSERT_EQUAL(count_moves(m), 7);

  // La prise en passant (e5 -> d6) doit être présente
  move en_passant = {.from = 36,
                     .to = 43,
                     .type = PAWN,
                     .promotion = EMPTY,
                     .kind = MOVE_EN_PASSANT};
  CU_ASSERT_TRUE(move_exists(m, en_passant));

  free_movelist(m);
  free_chessboard(cb);

  /*
   * =========================================================
   * POSITION AVEC ROQUE DISPONIBLE (PETIT ET GRAND)
   * =========================================================
   */
  cb = create_empty_chessboard();
  cb->chessboard_color = WHITE;
  place_piece(cb, KING, WHITE, 4);
  place_piece(cb, ROOK, WHITE, 0);
  place_piece(cb, ROOK, WHITE, 7);
  place_piece(cb, KING, BLACK, 60);
  set_castling_right(cb, WHITE_KING_SIDE);
  set_castling_right(cb, WHITE_QUEEN_SIDE);

  m = generate_move(cb);

  // 7 coups de roi (5 simples + 2 roques) + 10 (tour a1) + 9 (tour h1) = 26
  CU_ASSERT_EQUAL(count_moves(m), 26);

  // Les deux roques doivent être présents
  move short_castle = {.from = 4,
                       .to = 6,
                       .type = KING,
                       .promotion = EMPTY,
                       .kind = MOVE_CASTLE_KING};
  move long_castle = {.from = 4,
                      .to = 2,
                      .type = KING,
                      .promotion = EMPTY,
                      .kind = MOVE_CASTLE_QUEEN};
  CU_ASSERT_TRUE(move_exists(m, short_castle));
  CU_ASSERT_TRUE(move_exists(m, long_castle));

  free_movelist(m);
  free_chessboard(cb);

  /*
   * =========================================================
   * POSITION AVEC ROQUE BLOQUE : g1 est visée par une tour adverse
   * =========================================================
   */
  cb = create_empty_chessboard();
  cb->chessboard_color = WHITE;
  place_piece(cb, KING, WHITE, 4);
  place_piece(cb, ROOK, WHITE, 7);
  place_piece(cb, KING, BLACK, 60);
  place_piece(cb, ROOK, BLACK, 62); // g8 vise g1
  set_castling_right(cb, WHITE_KING_SIDE);

  m = generate_move(cb);

  // 5 coups de roi + 9 (tour h1) = 14, sans le petit roque
  CU_ASSERT_EQUAL(count_moves(m), 14);

  // Le petit roque ne doit PAS exister (g1 visée)
  move blocked_castle = {.from = 4,
                         .to = 6,
                         .type = KING,
                         .promotion = EMPTY,
                         .kind = MOVE_CASTLE_KING};
  CU_ASSERT_FALSE(move_exists(m, blocked_castle));

  free_movelist(m);
  free_chessboard(cb);

  /*
   * =========================================================
   * POSITION DE PROMOTION : pion b7 pousse ou prend en promouvant
   * =========================================================
   */
  cb = create_empty_chessboard();
  cb->chessboard_color = WHITE;
  place_piece(cb, KING, WHITE, 4);
  place_piece(cb, KING, BLACK, 60);
  place_piece(cb, PAWN, WHITE, 49);   // b7
  place_piece(cb, ROOK, BLACK, 56);   // a8 (prise en promotion possible)
  place_piece(cb, KNIGHT, BLACK, 58); // c8 (prise en promotion possible)

  m = generate_move(cb);

  // 5 coups de roi + 12 coups de pion (3 arrivées x 4 promotions) = 17
  CU_ASSERT_EQUAL(count_moves(m), 17);

  // Promotion par poussée (b8) et par prise (a8, c8) en dame
  move promo_push = {.from = 49, .to = 57, .type = PAWN, .promotion = QUEEN};
  move promo_capture_a8 = {.from = 49,
                           .to = 56,
                           .type = PAWN,
                           .promotion = QUEEN,
                           .kind = MOVE_CAPTURE};
  move promo_capture_c8 = {.from = 49,
                           .to = 58,
                           .type = PAWN,
                           .promotion = QUEEN,
                           .kind = MOVE_CAPTURE};
  CU_ASSERT_TRUE(move_exists(m, promo_push));
  CU_ASSERT_TRUE(move_exists(m, promo_capture_a8));
  CU_ASSERT_TRUE(move_exists(m, promo_capture_c8));

  free_movelist(m);
  free_chessboard(cb);
}
