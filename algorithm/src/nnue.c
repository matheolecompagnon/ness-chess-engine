/**
 * \file nnue.c
 * Ce fichier implémente le réseau NNUE : l'évaluation d'une position et la mise
 * à jour des accumulateurs quand une pièce apparaît ou disparaît.
 * \details Le réseau regarde l'échiquier de deux façons (vue des Blancs, vue
 * des Noirs). Les accumulateurs sont rangés dans le chessboard (cb->acc) :
 * apply_move les met à jour à chaque coup (add/sub), undo_move fait l'inverse
 * pour revenir en arrière. On ne recalcule tout que dans nnue_refresh(), une
 * seule fois au début.
 *
 * \see Implémentation fondée sur la page NNUE du Chess Programming Wiki :
 *      https://www.chessprogramming.org/NNUE
 * \author LECOMPAGNON Mathéo (adapté du tutoriel NNUE minimal de Stockfish)
 * \date 13th July 2026
 */

#include "nnue.h"
#include "bitboard.h"
#include "type.h"

/**
 * \brief Les poids et biais du réseau, en flash.
 * \details Définis dans nnue_stub.c ou le nnue chargé.
 */
extern const network nnue_net;

/**
 * \brief feature_index(perspective, c, t, sq) donne l'index de la feature (c,
 * t, sq) vue depuis \b perspective.
 * \details Une feature = (couleur, type de pièce, case). Vue par les Noirs, on
 * inverse la couleur de la pièce et on retourne la case verticalement (sq ^
 * 0b111000), pour que le réseau voie toujours "sa" moitié d'échiquier en bas.
 * \pre t dans [PAWN, KING] (pas EMPTY).
 */
/*
 * Ordre des pièces utilisé À L'ENTRAÎNEMENT (Ness_project_old) :
 *   PAWN=0, KNIGHT=1, BISHOP=2, ROOK=3, QUEEN=4, KING=5.
 * Notre enum piecetype diffère (PAWN=0, ROOK=1, KNIGHT=2, BISHOP=3, QUEEN=4,
 * KING=5). On remappe donc notre type vers l'index d'entraînement pour que les
 * features tombent sur les bons poids de tiny.nnue.
 */
static const uint8_t NNUE_PIECE_ORDER[6] = {
    [PAWN] = 0, [KNIGHT] = 1, [BISHOP] = 2, [ROOK] = 3, [QUEEN] = 4, [KING] = 5,
};

static uint16_t feature_index(color perspective, color c, piecetype t,
                              cell sq) {
  if (perspective == BLACK) {
    c = 1 - c;          // On inverse la couleur
    sq = sq ^ 0b111000; // On retourne la case verticalement
  }
  return (uint16_t)c * 64 * 6 + (uint16_t)NNUE_PIECE_ORDER[t] * 64 + sq;
}

/**
 * \brief accumulator_add(a, index) ajoute la feature \b index à l'accumulateur
 * \b a.
 */
static void accumulator_add(accumulator *a, uint16_t index) {
  for (int i = 0; i < NNUE_HL_SIZE; i++) {
    a->values[i] += nnue_net.accumulator_weights[index][i];
  }
}

/**
 * \brief accumulator_sub(a, index) retire la feature \b index de l'accumulateur
 * \b a.
 */
static void accumulator_sub(accumulator *a, uint16_t index) {
  for (int i = 0; i < NNUE_HL_SIZE; i++) {
    a->values[i] -= nnue_net.accumulator_weights[index][i];
  }
}

void nnue_add_piece(chessboard cb, color c, piecetype t, cell sq) {
  // Une pièce touche les DEUX perspectives (avec des index différents)
  accumulator_add(&cb->acc.perspective[WHITE], feature_index(WHITE, c, t, sq));
  accumulator_add(&cb->acc.perspective[BLACK], feature_index(BLACK, c, t, sq));
}

void nnue_remove_piece(chessboard cb, color c, piecetype t, cell sq) {
  accumulator_sub(&cb->acc.perspective[WHITE], feature_index(WHITE, c, t, sq));
  accumulator_sub(&cb->acc.perspective[BLACK], feature_index(BLACK, c, t, sq));
}

void nnue_refresh(chessboard cb) {
  // On repart des biais
  for (int i = 0; i < NNUE_HL_SIZE; i++) {
    cb->acc.perspective[WHITE].values[i] = nnue_net.accumulator_biases[i];
    cb->acc.perspective[BLACK].values[i] = nnue_net.accumulator_biases[i];
  }
  // On ajoute toutes les pièces présentes
  for (color c = WHITE; c <= BLACK; c++) {
    for (piecetype t = PAWN; t <= KING; t++) {
      bitboard b = get_piece_bitboard(cb, c, t);
      // On ne visite que les cases occupées : pop_first_cell rend une case
      // occupée et la retire du bitboard à chaque tour.
      while (b) {
        cell sq = pop_first_cell(&b);
        nnue_add_piece(cb, c, t, sq);
      }
    }
  }
}

/**
 * \brief crelu(v) applique la Clipped ReLU : borne \b v dans [0, NNUE_QA].
 */
static int16_t crelu(int16_t v) {
  if (v < 0) {
    return 0;
  }
  if (v > NNUE_QA) {
    return NNUE_QA;
  }
  return v;
}

score evaluate(chessboard cb) {
  // Le réseau ne raisonne pas en "Blancs/Noirs" mais en "joueur au trait /
  // adversaire".
  color stm = get_chessboard_color(cb);
  const accumulator *stm_acc =
      &cb->acc.perspective[stm]; // Perspective du joueur au trait
  const accumulator *nstm_acc =
      &cb->acc.perspective[1 - stm]; // Perspective de l'adversaire

  int32_t eval = 0;
  for (int i = 0; i < NNUE_HL_SIZE; i++) {
    // Attention aux dépassements : on accumule en int32_t.
    eval += (int32_t)crelu(stm_acc->values[i]) * nnue_net.output_weights[i];
    eval += (int32_t)crelu(nstm_acc->values[i]) *
            nnue_net.output_weights[i + NNUE_HL_SIZE];
  }
  eval += nnue_net.output_bias;

  // On repasse à l'échelle d'un score (en centipions)
  eval *= NNUE_SCALE;
  eval /= (NNUE_QA * NNUE_QB);

  // Clamp de sécurité
  if (eval >= SCORE_INF) {
    eval = SCORE_INF - 1;
  } else if (eval <= -SCORE_INF) {
    eval = -SCORE_INF + 1;
  }

  return (score)eval; // Score du point de vue du joueur au trait (positif = à
                      // son avantage)
}
