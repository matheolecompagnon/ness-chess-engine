#ifndef __ZOBRIST_H__
#define __ZOBRIST_H__

/**
 * \file zobrist.h
 * \brief Clé de hachage Zobrist : identifiant 64 bits d'une position.
 * \details Le hachage de Zobrist associe à chaque position un entier de 64
 * bits, obtenu en combinant par OU exclusif un nombre pseudo-aléatoire tiré une
 * fois pour toutes par caractéristique de la position :
 * - un nombre par triplet (couleur, type de pièce, case) : 2 x 6 x 64 nombres ;
 * - un nombre pour "c'est aux noirs de jouer" ;
 * - un nombre par combinaison de droits de roque (16 combinaisons) ;
 * - un nombre par case cible de prise en passant.
 *
 * Deux positions identiques donnent donc la même clé, et deux positions
 * différentes ont une probabilité négligeable de partager la même (environ 1
 * sur 2^64). C'est cette clé qui sert d'index et de signature à la table de
 * transposition (voir tt.h).
 *
 * - zobrist_init() tire les nombres de la table (idempotent, appelé
 * automatiquement).
 * - zobrist_key() calcule la clé d'un échiquier.
 *
 * \author LECOMPAGNON Mathéo
 * \date 21th July 2026
 */

#include <stdint.h>

#include "type.h"

/**
 * \brief zobrist_init() initialise la table des nombres pseudo-aléatoires du
 * hachage.
 * \details Les nombres sont produits par un générateur déterministe à graine
 * fixe : deux exécutions du moteur donnent donc exactement les mêmes clés, ce
 * qui rend les mesures reproductibles. L'appel est idempotent et fait
 * automatiquement par zobrist_key() : il n'est pas nécessaire de l'appeler
 * soi-même.
 */
void zobrist_init(void);

/**
 * \brief zobrist_key(cb) calcule la clé de hachage de la position \b cb.
 * \details Le coût est proportionnel au nombre de pièces présentes (parcours
 * des bitboards par pop_first_cell), pas aux 64 cases.
 * \param cb un \b chessboard bien initialisé.
 * \return la clé 64 bits de la position.
 */
uint64_t zobrist_key(chessboard cb);

#endif
