#ifndef __APPLY_MOVE__
#define __APPLY_MOVE__

/**
 * \file apply_move.h
 * \brief Applique et annule un coup sur un \b chessboard.
 * \details Fonctions du fichier :
 * - apply_move(c, cb) applique le coup c sur cb et renvoie un undo_info ;
 * - undo_move(c, cb, u) annule le coup c et remet cb dans son état d'avant.
 *
 * apply_move() suppose le coup déjà validé (issu de generate_move). Elle
 * modifie l'échiquier en place : bitboards, droits de roque, en passant, trait,
 * accumulateurs NNUE.
 *
 * \author LECOMPAGNON Mathéo
 * \date 7th July 2026
 */

#include "movelist.h"
#include "type.h"

/**
 * \brief Structure \b undo_info : ce qu'il faut pour annuler un coup.
 * \details Le coup seul ne suffit pas : on ne sait pas ce qui a été capturé, ni
 * quels droits de roque et quel en passant existaient avant.
 */
typedef struct undo_info {
  piecetype captured;        /**< EMPTY si ce n'est pas une prise */
  uint8_t castling_rights;   /**< Droits de roque avant le coup */
  int16_t en_passant_target; /**< Cible du en passant avant le coup, -1 s'il n'y
                                en avait pas */
} undo_info;

/**
 * \brief apply_move(c, cb) applique le coup \b c sur l'échiquier \b cb.
 * \pre Le coup c est valide (généré par generate_move).
 * \pre Les accumulateurs cb->acc sont à jour (voir nnue_refresh()).
 * \param c le coup
 * \param cb l'échiquier, modifié en place
 * \return l'undo_info à passer à undo_move()
 */
undo_info apply_move(move c, chessboard cb);

/**
 * \brief undo_move(c, cb, u) annule le coup \b c et remet \b cb dans son état
 * d'avant.
 * \details L'accumulateur NNUE est remis en arrière par les opérations
 * inverses, il n'est pas sauvegardé dans undo_info.
 * \pre c et u viennent du dernier apply_move(c, cb) appliqué à cb.
 * \param c le coup appliqué
 * \param cb l'échiquier, modifié en place
 * \param u l'undo_info renvoyé par apply_move()
 */
void undo_move(move c, chessboard cb, undo_info u);

#endif
