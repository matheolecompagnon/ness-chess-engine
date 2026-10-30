/**
 * \file test_rule.h
 *
 * \date 2 juil. 2026
 * \author Mathéo LECOMPAGNON
 */

#include "test_utils.h"
#include <stdio.h>

chessboard create_empty_chessboard() {
  chessboard cb = malloc(sizeof(struct chessboard));
  for (int c = 0; c < 2; c++) {
    for (int p = 0; p < 6; p++) {
      cb->pieceboard[c][p] = 0ULL;
    }
  }
  cb->chessboard_color = WHITE;
  cb->castling_rights = 0;
  cb->en_passant_target = -1;
  return cb;
}

void place_piece(chessboard cb, piecetype piece_type, color color, cell index) {
  set_bit(&cb->pieceboard[color][piece_type], index);
}

chessboard create_chessboard_from_pieces(piece pieces[], cell indices[],
                                         int count) {
  chessboard cb = create_empty_chessboard();
  for (int i = 0; i < count; i++) {
    place_piece(cb, pieces[i].type, pieces[i].piece_color, indices[i]);
  }
  return cb;
}

uint8_t move_exists(movelist m, move move_to_find) {
  if (m == NULL) {
    return 0;
  } // La liste est vide
  // Parcours des vrais noeuds (on saute la sentinelle en tête, dont le champ .c
  // n'est pas initialisé) jusqu'à trouver le coup cherché.
  for (movelist n = get_next(m); n != NULL; n = get_next(n)) {
    move current = get_current_move(n);
    if (current.from == move_to_find.from && current.to == move_to_find.to &&
        current.type == move_to_find.type &&
        current.promotion == move_to_find.promotion &&
        current.kind == move_to_find.kind) {
      return 1; // On a trouvé le coup
    }
  }
  return 0; // Le coup n'est pas dans la liste
}

int count_moves(movelist m) {
  int count = 0;
  if (m == NULL) {
    return 0;
  } // La liste des coups est nulle.
  // On parcours la liste en comptant chaque élément
  while (get_next(m) != NULL) {
    m = get_next(m);
    count++;
  }
  return count;
}
