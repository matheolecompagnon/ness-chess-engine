/**
 * \file test_bitboard.c
 *
 * \date 9 juil. 2026
 * \author Mathéo LECOMPAGNON
 */

#include "test_bitboard.h"
#include "bitboard.h"
#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>

// --- Tests pour set_bit ---
void test_set_bit(void) {
  bitboard b = 0ULL;

  // Case 0 (bit de poids faible)
  set_bit(&b, 0);
  CU_ASSERT_EQUAL(b, 1ULL);

  // Case 63 : vérifie le décalage au-delà de 32 bits (d'où le 1ULL dans
  // set_bit)
  set_bit(&b, 63);
  CU_ASSERT_TRUE(get_bit(b, 63));
  CU_ASSERT_EQUAL(b, (1ULL << 63) | 1ULL);

  // set_bit est idempotent : rallumer un bit déjà à 1 ne change rien
  set_bit(&b, 0);
  CU_ASSERT_EQUAL(get_bit(b, 0), 1);
}

// --- Tests pour clear_bit ---
void test_clear_bit(void) {
  bitboard b = 0ULL;

  set_bit(&b, 27);
  CU_ASSERT_TRUE(get_bit(b, 27));

  // On éteint la case : elle repasse à 0, le bitboard redevient vide
  clear_bit(&b, 27);
  CU_ASSERT_FALSE(get_bit(b, 27));
  CU_ASSERT_EQUAL(b, 0ULL);

  // clear_bit sur un bit déjà à 0 : sans effet
  clear_bit(&b, 10);
  CU_ASSERT_EQUAL(b, 0ULL);
}

// --- Tests pour get_bit ---
void test_get_bit(void) {
  bitboard b = 0ULL;

  // Un bitboard vide n'a aucun bit allumé
  for (int c = 0; c < 64; c++) {
    CU_ASSERT_FALSE(get_bit(b, c));
  }

  // On allume deux cases et on vérifie leur isolement
  set_bit(&b, 5);
  set_bit(&b, 40);
  for (int c = 0; c < 64; c++) {
    if (c == 5 || c == 40) {
      CU_ASSERT_TRUE(get_bit(b, c));
    } else {
      CU_ASSERT_FALSE(get_bit(b, c));
    }
  }
}
