#ifndef __RULE_ITER_H__
#define __RULE_ITER_H__

/**
 * \file rule_iter.h
 * \brief Génération des coups légaux.
 * \details Fonctions du fichier :
 * - generate_move(cb) donne la liste des coups légaux du joueur au trait ;
 * - is_king_in_check(cb, c) teste si le roi de couleur c est en échec ;
 * - is_available_index(index, cb) teste si une case est dans l'échiquier et
 * n'est pas occupée par une pièce du joueur ;
 * - generate_<pièce>_move() ajoutent les coups pseudo-légaux d'une pièce, y
 * compris le roque pour le roi et la prise en passant pour le pion (exposées
 * pour les tests).
 *
 * \author LECOMPAGNON Mathéo
 * \date 8th June 2026
 * \copyright MIT, voir le fichier LICENSE à la racine du projet.
 */

#include "movelist.h"
#include "type.h"

/**
 * \brief generate_move(cb) retourne les coups légaux du joueur au trait de \b
 * cb.
 * \details Les coups pseudo-légaux sont générés puis filtrés : ceux qui
 * laissent le roi du joueur en échec sont écartés.
 * \param cb un \b chessboard
 * \return la liste des coups légaux
 */
movelist generate_move(chessboard cb);

/**
 * \brief is_king_in_check(cb, c) indique si le roi de couleur \b c est en échec
 * sur \b cb.
 * \param cb un \b chessboard
 * \param c la \b color du roi testé
 * \return 1 si le roi est attaqué, 0 sinon (ou s'il n'y a pas de roi)
 */
uint8_t is_king_in_check(chessboard cb, color c);

// À des fins de test
uint8_t is_available_index(int8_t index, chessboard cb);
void generate_knight_move(chessboard, movelist);
void generate_rook_move(chessboard, movelist);
void generate_bishop_move(chessboard, movelist);
void generate_queen_move(chessboard, movelist);
void generate_king_move_aux(chessboard, piecetype, movelist);
void generate_pawn_move_aux(chessboard, movelist);
void generate_king_move(chessboard, movelist);
void generate_pawn_move(chessboard, movelist);

#endif
