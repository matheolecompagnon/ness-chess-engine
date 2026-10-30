/**
 * \file nnue.h
 * \brief Ce fichier décrit le score d'une position et les fonctions du réseau
 * NNUE.
 * \details Un \b score est un entier 16 bits (positif = avantage au joueur au
 * trait). SCORE_INF sert de valeur "infinie" pour la fenêtre alpha-bêta ; il
 * est pris assez petit pour que sa négation ne déborde pas.
 * - \b score le score d'une position (int16_t).
 * - SCORE_INF "l'infini" servant de fenêtre alpha-bêta de départ
 * (+/-SCORE_INF).
 * - evaluate() donne le score de l'échiquier \b cb.
 * - nnue_refresh() recalcule les accumulateurs de \b cb depuis zéro.
 * - nnue_add_piece() met à jour les accumulateurs quand une pièce apparaît.
 * - nnue_remove_piece() met à jour les accumulateurs quand une pièce disparaît.
 *
 * \author LECOMPAGNON Mathéo
 * \date 6th July 2026
 */

#ifndef __NNUE_H__
#define __NNUE_H__

#include "type.h"

typedef int16_t score;

/**
 * \brief SCORE_INF est l'"infini" utilisé par negamax pour la fenêtre
 * alpha-bêta de départ.
 * \details On le prend plus petit que le maximum d'un int16_t pour que
 * -SCORE_INF ne déborde pas. evaluate() doit toujours renvoyer une valeur entre
 * -SCORE_INF et +SCORE_INF (exclus).
 */
#define SCORE_INF ((score)30000)

/**
 * \brief evaluate(cb) donne le score de l'échiquier \b cb.
 * \details Utilise les accumulateurs cb->acc déjà à jour, sans les recalculer.
 * Le score est rendu du point de vue du joueur au trait (positif = à son
 * avantage).
 * \pre cb->acc est à jour (grâce à nnue_refresh, puis apply_move et undo_move).
 * \param cb un \b chessboard.
 * \return Le score de l'échiquier \b cb.
 */
score evaluate(chessboard cb);

/**
 * \brief nnue_refresh(cb) calcule entièrement les accumulateurs de \b cb depuis
 * ses pièces.
 * \details Coûteux : à n'appeler qu'une fois, sur la position racine, avant de
 * lancer la recherche.
 * \param cb un \b chessboard
 */
void nnue_refresh(chessboard cb);

/**
 * \brief nnue_add_piece(cb, c, t, sq) met à jour les accumulateurs pour
 * l'apparition d'une pièce.
 * \details A appeler dans apply_move à chaque set_bit (arrivée d'une pièce), et
 * dans undo_move pour remettre une pièce retirée.
 * \pre t != EMPTY
 * \param cb un \b chessboard.
 * \param c couleur de la pièce
 * \param t son type
 * \param sq sa case.
 */
void nnue_add_piece(chessboard cb, color c, piecetype t, cell sq);

/**
 * \brief nnue_remove_piece(cb, c, t, sq) met à jour les accumulateurs pour le
 * retrait d'une pièce.
 * \details A appeler dans apply_move à chaque clear_bit (pièce qui quitte une
 * case ou capturée), et dans undo_move pour retirer une pièce qu'on avait
 * ajoutée.
 * \pre t != EMPTY
 * \param cb un \b chessboard.
 * \param c couleur de la pièce
 * \param t son type
 * \param sq sa case.
 */
void nnue_remove_piece(chessboard cb, color c, piecetype t, cell sq);

#endif
