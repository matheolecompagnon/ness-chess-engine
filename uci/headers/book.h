#ifndef __BOOK_H__
#define __BOOK_H__

/**
 * \file book.h
 * \brief Livre d'ouverture Polyglot, OPTIONNEL et cote UCI (jamais dans le
 * NNUE).
 * \details Ce module n'est compile qu'avec \c make \c BOOK=1. Il permet au
 * binaire d'etre AUTONOME : il joue son livre d'ouverture tout seul, sans
 * qu'une interface (Knights) ou un pont (lichess-bot) ait a le fournir. Il vit
 * dans la couche UCI, cote PC ; il ne touche NI a l'evaluation NNUE, NI aux
 * poids, NI a la recherche - le portage FPGA du reseau reste intact.
 *
 * Le hachage suit le standard Polyglot (constantes dans polyglot_keys.c,
 * extraites de python-chess pour correspondre au bit pres aux livres .bin
 * usuels).
 *
 * \author LECOMPAGNON Mathéo
 */

#include "movelist.h"
#include "type.h"
#include <stdint.h>

/** \brief Les 781 constantes de hachage Polyglot (definies dans
 * polyglot_keys.c). */
extern const uint64_t POLYGLOT_KEYS[781];

/**
 * \brief book_probe(cb, out) cherche un coup de livre pour la position \b cb.
 * \details Au premier appel, ouvre le livre (chemin \c NESS_BOOK_PATH, ou la
 * variable d'environnement \c NESS_BOOK si elle est definie). Choisit le coup
 * de plus fort poids et le traduit en \b move legal du moteur (roque et en
 * passant compris). Sans livre ou hors livre, renvoie 0 et \b out n'est pas
 * modifie.
 * \param cb un \b chessboard bien initialise.
 * \param out ou ecrire le coup trouve.
 * \return 1 si un coup de livre legal a ete trouve, 0 sinon.
 */
uint8_t book_probe(chessboard cb, move *out);

#endif
