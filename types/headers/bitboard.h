#ifndef __BITBOARD_H__
#define __BITBOARD_H__

/**
 * \file bitboard.h
 * \brief Ce fichier décrit la structure d'un bitboard et les fonctions pour le
 * manipuler.
 * \details Un \b bitboard est un uint64_t où chaque bit représente une case de
 * l'échiquier (bit k = case d'index k, par exemple bit 0 = case A1). Il
 * représente un ensemble de cases sur lequel on fait des opérations bit à bit
 * en O(1).
 * - \b bitboard un ensemble de cases codé sur 64 bits.
 * - \b cell l'index d'une case (0 à 63).
 * - set_bit() pour mettre à 1 le bit de la case \b c.
 * - clear_bit() pour mettre à 0 le bit de la case \b c.
 * - get_bit() pour lire la valeur (0 ou 1) du bit de la case \b c.
 * - first_cell() pour trouver la case du premier bit à 1 (le plus faible).
 * - first_cell_scan() même résultat, par parcours bit à bit (implémentation de
 * référence).
 *
 * \author LECOMPAGNON Mathéo
 * \date 5th June 2026
 */

#include <stdint.h>

/**
 * \brief Type \b bitboard représentant un bitboard.
 * \details Un bitboard \b bitboard est un uint64_t où chaque bit
 * représente une case de l'échiquier.
 * bit k = case d'index k dans le bitboard
 * par exemple bit 0 = case A1.
 */
typedef uint64_t bitboard;

/**
 * \brief Type \b cell représentant l'index d'une case dans un bitboard.
 * \details Une case \b cell est un uint8_t et prend au maximum pour
 * valeur 63 (0, 1, ..., 63).
 *
 */
typedef uint8_t cell;

/**
 * 	\brief Remplace le bit de position \b c dans le bitboard \b *b par 1.
 *	\param b un pointeur vers un \b bitboard.
 *	\param c l'index d'une case.
 */
void set_bit(bitboard *b, cell c);

/**
 * 	\brief Remplace le bit de position \b c dans le bitboard \b *b par 0.
 *	\param b un pointeur vers un \b bitboard.
 *	\param c l'index d'une case.
 */
void clear_bit(bitboard *b, cell c);

/**
 * 	\brief Récupère le bit de position \b c dans le bitboard \b b.
 *	\param b un \b bitboard.
 *	\param c l'index d'une case.
 * 	\return la valeur (0 ou 1) du bit de position \b c dans \b b.
 */
uint8_t get_bit(bitboard b, cell c);

/**
 * 	\brief Récupère la case du bit à 1 de plus faible poids dans le bitboard
 *\b b.
 *	\details C'est la version utilisée par le moteur : elle s'appuie sur
 *__builtin_ctzll, compilé en une instruction machine (TZCNT/BSF sur x86). Sert
 *à parcourir un ensemble de cases sans tester les 64 bits un par un. Demande
 *donc GCC ou Clang, comme le reste du Makefile.
 *	\param b un \b bitboard.
 * 	\return l'index de la case du premier bit à 1, ou 64 si \b b est vide.
 */
cell first_cell(bitboard b);

/**
 * 	\brief Récupère la case du bit à 1 de plus faible poids en parcourant
 *les bits un par un.
 *	\details Implémentation de référence, conservée : elle ne dépend
 *d'aucune extension du compilateur et sert de témoin de comparaison face à
 *first_cell().
 *	\param b un \b bitboard.
 * 	\return l'index de la case du premier bit à 1, ou 64 si \b b est vide.
 */
cell first_cell_scan(bitboard b);

/**
 * 	\brief Retire du bitboard \b *b la case de son bit à 1 de plus faible
 *poids et la retourne.
 *	\details Combine first_cell() et clear_bit(). C'est la façon normale de
 *parcourir un bitboard : tant qu'il est non nul, chaque appel rend une case
 *occupée et l'enlève de l'ensemble, ce qui fait autant de tours de boucle que
 *de pièces au lieu de 64.
 *	\code
 *	while (b) {
 *		cell sq = pop_first_cell(&b);
 *		// ... traitement de la case sq
 *	}
 *	\endcode
 *	\param b un pointeur vers un \b bitboard, modifié par l'appel.
 * 	\return l'index de la case retirée, ou 64 si \b *b était déjà vide (dans
 *ce cas \b *b est inchangé).
 */
cell pop_first_cell(bitboard *b);

#endif
