/**
 * \file bitboard.c
 * Ce fichier implémente les fonctions permettant de gérer un bitboard
 * et faire des opérations.
 *
 * \author LECOMPAGNON Mathéo
 * \date 5th June 2026
 */

#include "bitboard.h"

void set_bit(bitboard *b, cell c) {
  /*
   * a la liste de bits
   * n la position
   * a | (1<<n) met le neme bit de a à 1
   */
  // ULL unsigned long long pour éviter les erreurs de décalage > 32
  *b = *b | (1ULL << c);
}

void clear_bit(bitboard *b, cell c) {
  /*
   * a la liste de bits
   * n la position
   * a & ~(1<<n) met le neme bit de a à 0
   */
  // ULL unsigned long long pour éviter les erreurs de décalage > 32
  *b = *b & ~(1ULL << c);
}

uint8_t get_bit(bitboard b, cell c) {
  /*
   * a la liste de bits
   * n la position
   * a>>n & 1 donne la valeur du neme bit
   */
  return (b >> c) & 1;
}

cell first_cell_scan(bitboard b) {
  /*
   * Parcours naif : on teste les bits du plus faible au plus fort et on rend
   * l'index du premier qui vaut 1. O(64) tests dans le pire cas.
   */
  for (cell i = 0; i < 64; i++) {
    if (get_bit(b, i))
      return i;
  }
  return 64;
}

cell first_cell(bitboard b) {
  // __builtin_ctzll(0) est indéfini : le cas du bitboard vide se traite à part.
  if (b == 0)
    return 64;
  /*
   * ctz = count trailing zeros : nombre de bits à 0 avant le premier bit à 1,
   * ce qui est exactement l'index de ce bit. Une seule instruction machine.
   */
  return (cell)__builtin_ctzll(b);
}

cell pop_first_cell(bitboard *b) {
  /*
   * On lit la case du premier bit à 1 avec first_cell, puis on retire cette
   * case de l'ensemble avec clear_bit : le prochain appel rendra donc la case
   * suivante. C'est ce qui permet de parcourir un bitboard en ne visitant que
   * les cases occupées.
   */
  cell c = first_cell(*b);
  if (c == 64)
    return 64; // Bitboard vide : rien à retirer
  clear_bit(b, c);
  return c;
}
