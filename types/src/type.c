/**
 * \file type.c
 * Ce fichier implémente les fonctions permettant de gérer un échiquier complet.
 *
 * \author LECOMPAGNON Mathéo
 * \date 5th June 2026
 */

#include "type.h"
#include <stdlib.h>

piecetype get_piece_type(piece p) { return p.type; }

color get_piece_color(piece p) { return p.piece_color; }

chessboard create_chessboard() {
  chessboard cb = calloc(1, sizeof(struct chessboard));
  cb->castling_rights =
      0; // A faire attention, lors du début d'une partie tout est à 1.
  cb->chessboard_color = WHITE;
  cb->en_passant_target = -1;
  return cb;
}

void free_chessboard(chessboard cb) { free(cb); }

bitboard get_piece_bitboard(chessboard cb, color c, piecetype type) {
  return cb->pieceboard[c][type];
}

bitboard *get_piece_bitboard_ref(chessboard cb, color c, piecetype type) {
  return &cb->pieceboard[c][type];
}

color get_chessboard_color(chessboard cb) { return cb->chessboard_color; }

bitboard get_color_bitboard(chessboard cb, color c) {
  bitboard b = 0ULL;
  for (int piece_type = PAWN; piece_type <= KING; piece_type++) {
    b |= cb->pieceboard[c][piece_type];
  }
  return b;
}

uint8_t get_castling_right(chessboard cb, castlingtype t) {
  /*
   * a la liste de bits (castling_rights)
   * t la position
   * a>>t & 1 donne la valeur du teme bit
   */
  return (cb->castling_rights >> t) & 1;
}

void set_castling_right(chessboard cb, castlingtype t) {
  // 1ULL pour éviter les erreurs de décalage
  cb->castling_rights |= (1ULL << t);
}

void remove_castling_right(chessboard cb, castlingtype t) {
  // 1ULL pour éviter les erreurs de décalage
  cb->castling_rights &= ~(1ULL << t);
}

int8_t get_en_passant_target(chessboard cb) { return cb->en_passant_target; }

void set_en_passant_target(chessboard cb, int16_t t) {
  cb->en_passant_target = t; /* t=-1 si la règle du en passant ne peut être
  appliquée, sinon est un \b cell représentant où le pion peut prendre */
}

void set_chessboard_color(chessboard cb, color c) { cb->chessboard_color = c; }
