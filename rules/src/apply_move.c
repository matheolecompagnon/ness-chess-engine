/**
 * \file apply_move.c
 * \author LECOMPAGNON Mathéo
 * \date 7th July 2026
 */

#include "apply_move.h"
#include "type.h"

#include "../../types/headers/movelist.h"
#include "nnue.h"

undo_info apply_move(move c, chessboard cb) {
  color chessboard_color = get_chessboard_color(cb);

  /*
   * Enregistrement de l'état irréversible AVANT toute modification : c'est ce
   * que undo_move ne pourrait pas reconstituer à partir du seul coup c.
   */
  undo_info u;
  u.captured = EMPTY;
  u.castling_rights = cb->castling_rights;
  u.en_passant_target = get_en_passant_target(cb);

  /*
   * La particularité de notre architecture :
   * - generate_move vérifie toutes les conditions pour tous ces types de coups
   *   (MOVE_NORMAL, MOVE_CAPTURE, MOVE_DOUBLE_PUSH, MOVE_EN_PASSANT,
   * MOVE_CASTLE_KING, MOVE_CASTLE_QUEEN) ; apply_move les applique en supposant
   * tout vérifié.
   * - À chaque clear_bit correspond un nnue_remove_piece, à chaque set_bit un
   *   nnue_add_piece (mêmes couleur/type/case), pour maintenir les
   * accumulateurs.
   */
  /*
   * ============
   * Coup basique
   * ============
   * MOVE_NORMAL
   */
  if (c.kind == MOVE_NORMAL) {
    // La pièce est retirée de sa case de départ
    clear_bit(get_piece_bitboard_ref(cb, chessboard_color, c.type), c.from);
    nnue_remove_piece(cb, chessboard_color, c.type, c.from);

    // Gestion de la promotion (hypothèse que ce soit le pion)
    if (c.promotion != EMPTY) {
      set_bit(get_piece_bitboard_ref(cb, chessboard_color, c.promotion), c.to);
      nnue_add_piece(cb, chessboard_color, c.promotion, c.to);
    } else {
      // La pièce est mise à sa case d'arrivée
      set_bit(get_piece_bitboard_ref(cb, chessboard_color, c.type), c.to);
      nnue_add_piece(cb, chessboard_color, c.type, c.to);
    }
  }
  /*
   * ========
   * Capture
   * ========
   * MOVE_CAPTURE
   */
  else if (c.kind == MOVE_CAPTURE) {
    // La pièce est retirée de sa case de départ
    clear_bit(get_piece_bitboard_ref(cb, chessboard_color, c.type), c.from);
    nnue_remove_piece(cb, chessboard_color, c.type, c.from);

    // Recherche de la pièce capturée : hypothèse qu'il y en a une
    piecetype t = EMPTY;
    for (piecetype i = PAWN; i <= KING; i++) {
      if (get_bit(get_piece_bitboard(cb, 1 - chessboard_color, i), c.to)) {
        t = i;
        break;
      }
    }
    u.captured = t; // Mémorisé pour undo_move
    // La pièce capturée (de type t) est retirée de l'échiquier.
    // Garde t != EMPTY : un MOVE_CAPTURE mal étiqueté (case d'arrivée vide)
    // laisserait t à EMPTY (= 6). get_piece_bitboard_ref(cb, couleur, 6)
    // écrirait alors hors de pieceboard[couleur][0..5], et NNUE_PIECE_ORDER[6]
    // serait lu hors tableau : comportement indéfini. La génération de coups ne
    // produit pas un tel coup, mais on ne fait pas confiance à l'étiquette pour
    // une écriture mémoire.
    if (t != EMPTY) {
      clear_bit(get_piece_bitboard_ref(cb, 1 - chessboard_color, t), c.to);
      nnue_remove_piece(cb, 1 - chessboard_color, t, c.to);
    }

    // Gestion de la promotion (hypothèse que ce soit le pion)
    if (c.promotion != EMPTY) {
      set_bit(get_piece_bitboard_ref(cb, chessboard_color, c.promotion), c.to);
      nnue_add_piece(cb, chessboard_color, c.promotion, c.to);
    } else {
      // La pièce est mise à sa case d'arrivée
      set_bit(get_piece_bitboard_ref(cb, chessboard_color, c.type), c.to);
      nnue_add_piece(cb, chessboard_color, c.type, c.to);
    }
  }
  /*
   * ===============================
   * Coup de deux cases pour le pion
   * ===============================
   * MOVE_DOUBLE_PUSH
   */
  else if (c.kind == MOVE_DOUBLE_PUSH) {
    // Le double coup ne peut être une capture.
    clear_bit(get_piece_bitboard_ref(cb, chessboard_color, c.type), c.from);
    nnue_remove_piece(cb, chessboard_color, c.type, c.from);
    set_bit(get_piece_bitboard_ref(cb, chessboard_color, c.type), c.to);
    nnue_add_piece(cb, chessboard_color, c.type, c.to);

    // Le pion vient de faire un double coup, il peut être pris avec la règle du
    // en_passant
    int8_t pawn_direction =
        (chessboard_color == WHITE) ? 8 : -8; // Direction du pion
    set_en_passant_target(
        cb, c.to - pawn_direction); // Case entre départ et arrivée
  }
  /*
   * =======================
   * Coup en passant du pion
   * =======================
   * MOVE_EN_PASSANT
   */
  else if (c.kind == MOVE_EN_PASSANT) {
    clear_bit(get_piece_bitboard_ref(cb, chessboard_color, c.type), c.from);
    nnue_remove_piece(cb, chessboard_color, c.type, c.from);
    set_bit(get_piece_bitboard_ref(cb, chessboard_color, c.type), c.to);
    nnue_add_piece(cb, chessboard_color, c.type, c.to);

    // La pièce capturée par la méthode du en passant est prise.
    // Le pion pris est dans la case c.to + opponent_direction.
    int8_t opponent_direction = (chessboard_color == WHITE) ? -8 : 8;
    cell captured_sq = c.to + opponent_direction;
    u.captured = PAWN; // Un en passant capture toujours un pion
    clear_bit(get_piece_bitboard_ref(cb, 1 - chessboard_color, PAWN),
              captured_sq);
    nnue_remove_piece(cb, 1 - chessboard_color, PAWN, captured_sq);
  }
  /*
   * ==========================
   * Le roque du roi côté droit
   * ==========================
   * MOVE_CASTLE_KING
   */
  else if (c.kind == MOVE_CASTLE_KING) {
    // Le roi fait +2, la tour fait -2
    clear_bit(get_piece_bitboard_ref(cb, chessboard_color, KING), c.from);
    nnue_remove_piece(cb, chessboard_color, KING, c.from);
    set_bit(get_piece_bitboard_ref(cb, chessboard_color, KING), c.to);
    nnue_add_piece(cb, chessboard_color, KING, c.to);

    clear_bit(get_piece_bitboard_ref(cb, chessboard_color, ROOK),
              c.from + 3); // Tour à droite
    nnue_remove_piece(cb, chessboard_color, ROOK, c.from + 3);
    set_bit(get_piece_bitboard_ref(cb, chessboard_color, ROOK),
            c.from + 1); // Position finale
    nnue_add_piece(cb, chessboard_color, ROOK, c.from + 1);
  }
  /*
   * ===========================
   * Le roque du roi côté gauche
   * ===========================
   * MOVE_CASTLE_QUEEN
   */
  else if (c.kind == MOVE_CASTLE_QUEEN) {
    // Le roi fait -2, la tour fait +3
    clear_bit(get_piece_bitboard_ref(cb, chessboard_color, KING), c.from);
    nnue_remove_piece(cb, chessboard_color, KING, c.from);
    set_bit(get_piece_bitboard_ref(cb, chessboard_color, KING), c.to);
    nnue_add_piece(cb, chessboard_color, KING, c.to);

    clear_bit(get_piece_bitboard_ref(cb, chessboard_color, ROOK),
              c.from - 4); // Tour à gauche
    nnue_remove_piece(cb, chessboard_color, ROOK, c.from - 4);
    set_bit(get_piece_bitboard_ref(cb, chessboard_color, ROOK),
            c.from - 1); // Position finale
    nnue_add_piece(cb, chessboard_color, ROOK, c.from - 1);
  }

  /*
   * =================================
   * Modification des variables du jeu
   * =================================
   */

  // Si le coup joué n'est pas un double coup, la règle du en passant ne
  // s'applique plus.
  if (c.kind != MOVE_DOUBLE_PUSH) {
    set_en_passant_target(cb, -1);
  }

  // Modification des droits de roque : si la pièce jouée est un roi
  if (c.type == KING && ((get_castling_right(cb, WHITE_QUEEN_SIDE)) ||
                         (get_castling_right(cb, WHITE_KING_SIDE)) ||
                         (get_castling_right(cb, BLACK_QUEEN_SIDE)) ||
                         (get_castling_right(cb, BLACK_KING_SIDE)))) {
    if (chessboard_color == WHITE) {
      remove_castling_right(cb, WHITE_QUEEN_SIDE);
      remove_castling_right(cb, WHITE_KING_SIDE);
    } else {
      remove_castling_right(cb, BLACK_QUEEN_SIDE);
      remove_castling_right(cb, BLACK_KING_SIDE);
    }
  }
  // Si la pièce est une tour
  if (c.type == ROOK && ((get_castling_right(cb, WHITE_QUEEN_SIDE)) ||
                         (get_castling_right(cb, WHITE_KING_SIDE)) ||
                         (get_castling_right(cb, BLACK_QUEEN_SIDE)) ||
                         (get_castling_right(cb, BLACK_KING_SIDE)))) {
    if (get_castling_right(cb, WHITE_QUEEN_SIDE) && chessboard_color == WHITE &&
        c.from == 0) {
      remove_castling_right(cb, WHITE_QUEEN_SIDE);
    }
    if (get_castling_right(cb, WHITE_KING_SIDE) && chessboard_color == WHITE &&
        c.from == 7) {
      remove_castling_right(cb, WHITE_KING_SIDE);
    }
    if (get_castling_right(cb, BLACK_QUEEN_SIDE) && chessboard_color == BLACK &&
        c.from == 56) {
      remove_castling_right(cb, BLACK_QUEEN_SIDE);
    }
    if (get_castling_right(cb, BLACK_KING_SIDE) && chessboard_color == BLACK &&
        c.from == 63) {
      remove_castling_right(cb, BLACK_KING_SIDE);
    }
  }

  set_chessboard_color(cb, 1 - chessboard_color);
  return u;
}

void undo_move(move c, chessboard cb, undo_info u) {
  // apply_move a basculé la couleur : le joueur qui avait bougé est l'opposé du
  // trait courant.
  color mover = 1 - get_chessboard_color(cb);

  // 1. Restauration de l'état irréversible (couleur, droits de roque, en
  // passant).
  set_chessboard_color(cb, mover);
  cb->castling_rights = u.castling_rights;
  set_en_passant_target(cb, u.en_passant_target);

  // Type effectivement arrivé sur la case d'arrivée (dame si promotion, sinon
  // la pièce jouée).
  piecetype arrival = (c.promotion != EMPTY) ? c.promotion : c.type;

  // 2. Annulation des mouvements de pièces + accumulateurs (opérations inverses
  // de apply_move).
  if (c.kind == MOVE_NORMAL) {
    // On retire la pièce de la case d'arrivée...
    clear_bit(get_piece_bitboard_ref(cb, mover, arrival), c.to);
    nnue_remove_piece(cb, mover, arrival, c.to);
    // ...et on la remet sur sa case de départ (avec son type d'origine).
    set_bit(get_piece_bitboard_ref(cb, mover, c.type), c.from);
    nnue_add_piece(cb, mover, c.type, c.from);
  } else if (c.kind == MOVE_CAPTURE) {
    clear_bit(get_piece_bitboard_ref(cb, mover, arrival), c.to);
    nnue_remove_piece(cb, mover, arrival, c.to);
    // On ressuscite la pièce capturée (type mémorisé dans u.captured). Même
    // garde symétrique que dans apply_move : si aucune pièce n'avait été
    // capturée, u.captured vaut EMPTY et il n'y a rien à remettre (sans la
    // garde, écriture hors bornes).
    if (u.captured != EMPTY) {
      set_bit(get_piece_bitboard_ref(cb, 1 - mover, u.captured), c.to);
      nnue_add_piece(cb, 1 - mover, u.captured, c.to);
    }
    set_bit(get_piece_bitboard_ref(cb, mover, c.type), c.from);
    nnue_add_piece(cb, mover, c.type, c.from);
  } else if (c.kind == MOVE_DOUBLE_PUSH) {
    clear_bit(get_piece_bitboard_ref(cb, mover, c.type), c.to);
    nnue_remove_piece(cb, mover, c.type, c.to);
    set_bit(get_piece_bitboard_ref(cb, mover, c.type), c.from);
    nnue_add_piece(cb, mover, c.type, c.from);
  } else if (c.kind == MOVE_EN_PASSANT) {
    clear_bit(get_piece_bitboard_ref(cb, mover, c.type), c.to);
    nnue_remove_piece(cb, mover, c.type, c.to);
    set_bit(get_piece_bitboard_ref(cb, mover, c.type), c.from);
    nnue_add_piece(cb, mover, c.type, c.from);
    // On remet le pion capturé en passant sur sa case.
    int8_t opponent_direction = (mover == WHITE) ? -8 : 8;
    cell captured_sq = c.to + opponent_direction;
    set_bit(get_piece_bitboard_ref(cb, 1 - mover, PAWN), captured_sq);
    nnue_add_piece(cb, 1 - mover, PAWN, captured_sq);
  } else if (c.kind == MOVE_CASTLE_KING) {
    // Roi : de c.to vers c.from
    clear_bit(get_piece_bitboard_ref(cb, mover, KING), c.to);
    nnue_remove_piece(cb, mover, KING, c.to);
    set_bit(get_piece_bitboard_ref(cb, mover, KING), c.from);
    nnue_add_piece(cb, mover, KING, c.from);
    // Tour : de c.from+1 vers c.from+3
    clear_bit(get_piece_bitboard_ref(cb, mover, ROOK), c.from + 1);
    nnue_remove_piece(cb, mover, ROOK, c.from + 1);
    set_bit(get_piece_bitboard_ref(cb, mover, ROOK), c.from + 3);
    nnue_add_piece(cb, mover, ROOK, c.from + 3);
  } else if (c.kind == MOVE_CASTLE_QUEEN) {
    // Roi : de c.to vers c.from
    clear_bit(get_piece_bitboard_ref(cb, mover, KING), c.to);
    nnue_remove_piece(cb, mover, KING, c.to);
    set_bit(get_piece_bitboard_ref(cb, mover, KING), c.from);
    nnue_add_piece(cb, mover, KING, c.from);
    // Tour : de c.from-1 vers c.from-4
    clear_bit(get_piece_bitboard_ref(cb, mover, ROOK), c.from - 1);
    nnue_remove_piece(cb, mover, ROOK, c.from - 1);
    set_bit(get_piece_bitboard_ref(cb, mover, ROOK), c.from - 4);
    nnue_add_piece(cb, mover, ROOK, c.from - 4);
  }
}
