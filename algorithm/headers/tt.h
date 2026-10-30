#ifndef __TT_H__
#define __TT_H__

/**
 * \file tt.h
 * \brief Table de transposition : mémorisation des positions déjà cherchées.
 * \details Une même position est atteinte par de nombreux chemins de coups
 * différents
 * ("transpositions") : 1.e4 e5 2.Cf3 et 1.Cf3 e5 2.e4 donnent le même
 * échiquier. Sans mémoire, la recherche explore chaque fois le sous-arbre
 * entier. La table de transposition retient, pour chaque position rencontrée
 * (identifiée par sa clé Zobrist, voir zobrist.h) :
 * - le \b score trouvé et la \b profondeur à laquelle il a été obtenu ;
 * - la \b nature de ce score (exact, ou simple borne due à une coupure
 * alpha-bêta) ;
 * - le \b meilleur \b coup constaté, qui sert à ordonner les coups même quand
 * le score est inutilisable.
 *
 * Deux gains distincts en découlent : on évite de refouiller un sous-arbre déjà
 * résolu assez profondément, et on joue en premier le meilleur coup de la fois
 * précédente, ce qui multiplie les coupures alpha-bêta. Le second gain est le
 * plus important avec l'approfondissement itératif : chaque itération lègue son
 * meilleur coup à la suivante.
 *
 * - TT_BITS fixe la taille de la table (2^TT_BITS entrées).
 * - tt_clear() vide la table.
 * - tt_probe() interroge la table à une position donnée.
 * - tt_probe_move() ne demande que le meilleur coup mémorisé (sans coupure
 * possible).
 * - tt_store() enregistre le résultat d'une recherche.
 *
 * \author LECOMPAGNON Mathéo
 * \date 21th July 2026
 */

#include <stdint.h>

#include "movelist.h"
#include "nnue.h"

/**
 * \brief TT_BITS fixe la taille de la table : 2^TT_BITS entrées.
 * \details 20 donne 1 048 576 entrées de 32 octets, soit 32 Mo. C'est un
 * compromis : plus la table est grande, moins deux positions différentes se
 * disputent la même case, mais moins elle tient dans les caches du processeur.
 * Cette valeur suppose la cible PC actuelle ; un portage vers une cible
 * embarquée la réduirait fortement (elle est ici le seul paramètre à changer).
 *
 * La valeur peut être imposée à la compilation (\b -DTT_BITS=22) pour mesurer
 * l'effet de la taille sans toucher au source.
 */
#ifndef TT_BITS
#define TT_BITS 20
#endif

/**
 * \brief Nature du score mémorisé dans une entrée.
 * \details Une recherche alpha-bêta ne rend pas toujours la valeur exacte d'une
 * position : quand elle coupe, elle rend seulement une borne. Il faut donc
 * retenir de quel cas il s'agit, sous peine d'utiliser une borne comme une
 * valeur.
 */
typedef enum tt_flag {
  TT_EXACT =
      0, /**< Score exact : tous les coups ont été examinés sans coupure. */
  TT_LOWER = 1, /**< Borne inférieure : la recherche a coupé sur beta, le vrai
                   score est >= value. */
  TT_UPPER = 2  /**< Borne supérieure : aucun coup n'a atteint alpha, le vrai
                   score est <= value. */
} tt_flag;

/**
 * \brief Résultat d'une interrogation de la table.
 */
typedef struct tt_result {
  uint8_t hit;    /**< 1 si la table contient bien cette position. */
  uint8_t cutoff; /**< 1 si \b value peut être renvoyée telle quelle par la
                     recherche. */
  score value; /**< Le score mémorisé, ramené à la distance courante à la
                  racine. */
  uint8_t has_move; /**< 1 si \b best contient un coup exploitable pour
                       l'ordonnancement. */
  move best;        /**< Le meilleur coup mémorisé pour cette position. */
} tt_result;

/**
 * \brief tt_clear() vide entièrement la table.
 * \details À appeler entre deux parties (commande UCI \b ucinewgame). Ce n'est
 * pas nécessaire entre deux coups d'une même partie : les entrées restent
 * valides puisqu'elles sont indexées par la position elle-même.
 */
void tt_clear(void);

/**
 * \brief tt_probe(key, depth, alpha, beta, ply) interroge la table pour la
 * position \b key.
 * \details Le score mémorisé n'est réutilisable que s'il provient d'une
 * recherche \b au \b moins aussi profonde que celle demandée, et si sa nature
 * est compatible avec la fenêtre courante : une borne inférieure ne conclut que
 * si elle dépasse déjà beta, une borne supérieure que si elle reste sous alpha.
 * Sinon l'entrée n'est pas perdue pour autant : son meilleur coup sert quand
 * même à l'ordonnancement.
 * \param key la clé Zobrist de la position.
 * \param depth la profondeur restante de la recherche en cours.
 * \param alpha,beta la fenêtre alpha-bêta courante.
 * \param ply la distance en demi-coups à la racine (pour rétablir les scores de
 * mat).
 * \return un \b tt_result décrivant ce que la table sait de cette position.
 */
tt_result tt_probe(uint64_t key, uint8_t depth, score alpha, score beta,
                   uint16_t ply);

/**
 * \brief tt_probe_move(key, out) récupère le seul meilleur coup mémorisé pour
 * \b key.
 * \details Sert à la racine, où il ne faut jamais couper sur un score mémorisé
 * (on doit renvoyer un coup, pas une valeur) mais où le meilleur coup de
 * l'itération précédente est précisément ce qui rend l'approfondissement
 * itératif efficace.
 * \param key la clé Zobrist de la position.
 * \param out un pointeur où écrire le coup trouvé (non modifié si la fonction
 * rend 0).
 * \return 1 si un coup a été trouvé, 0 sinon.
 */
uint8_t tt_probe_move(uint64_t key, move *out);

/**
 * \brief tt_store(...) enregistre le résultat d'une recherche pour la position
 * \b key.
 * \details En cas de collision d'index entre deux positions, l'entrée existante
 * n'est remplacée que si elle concerne une autre position ou si elle vient
 * d'une recherche moins profonde : une information chère (grande profondeur)
 * n'est pas écrasée par une information bon marché.
 * \param key la clé Zobrist de la position.
 * \param depth la profondeur de la recherche qui a produit \b value.
 * \param flag la nature de \b value (exact, borne inférieure ou supérieure).
 * \param value le score obtenu.
 * \param best le meilleur coup trouvé.
 * \param has_move 1 si \b best est significatif, 0 sinon (position terminale).
 * \param ply la distance en demi-coups à la racine (pour normaliser les scores
 * de mat).
 */
void tt_store(uint64_t key, uint8_t depth, tt_flag flag, score value, move best,
              uint8_t has_move, uint16_t ply);

#endif
