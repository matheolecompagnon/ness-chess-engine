/**
 * \file test_rule.h
 *
 * \date 2 juil. 2026
 * \author Mathéo LECOMPAGNON
 */

#ifndef __TEST_UTILS_H__
#define __TEST_UTILS_H__

#include "../../types/headers/movelist.h"
#include "bitboard.h"
#include "type.h"
#include <stdlib.h>

/**
 * \brief Crée un échiquier vide (toutes les cases vides).
 * \return Un pointeur vers un chessboard vide.
 */
chessboard create_empty_chessboard();

/**
 * \brief Place une pièce sur l'échiquier à un index donné.
 * \param cb Un pointeur vers un chessboard.
 * \param piece_type Le type de la pièce (PAWN, ROOK, etc.).
 * \param color La couleur de la pièce (WHITE ou BLACK).
 * \param index L'index de la case (0-63).
 */
void place_piece(chessboard cb, piecetype piece_type, color color, cell index);

/**
 * \brief Crée un échiquier à partir d'une liste de pièces et de leurs
 * positions.
 * \param pieces Un tableau de pièces (type + couleur).
 * \param indices Un tableau d'index (0-63).
 * \param count Le nombre de pièces à placer.
 * \return Un pointeur vers le chessboard créé.
 */
chessboard create_chessboard_from_pieces(piece pieces[], cell indices[],
                                         int count);

/**
 * \brief Vérifie si un coup existe dans une movelist.
 * \param m Une movelist.
 * \param move_to_find Le coup à chercher.
 * \return 1 si le coup est trouvé, 0 sinon.
 */
uint8_t move_exists(movelist m, move move_to_find);

/**
 * \brief Compte le nombre de coups dans une movelist.
 * \param m Une movelist.
 * \return Le nombre de coups.
 */
int count_moves(movelist m);

#endif
