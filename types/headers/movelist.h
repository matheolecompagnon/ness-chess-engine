#ifndef __MOVELIST_H__
#define __MOVELIST_H__

/**
 * \file movelist.h
 * \brief Interface abstraite d'une pile de coups (\b movelist).
 * \details Ce header décrit **uniquement** l'interface d'une pile de coups
 * (LIFO), indépendamment de sa représentation interne. Le type \b movelist est
 * \b opaque : la structure concrète n'est pas exposée ici mais définie dans
 * chaque implémentation. Cela permet à plusieurs implémentations
 * interchangeables de fournir la même API :
 * - \b linked_movelist.c : liste chaînée simple avec noeud sentinelle (une
 * allocation par coup) ;
 * - \b dynamic_movelist.c : tableau dynamique, une seule allocation par pile
 * (implémentation par défaut, ~ 8 a 10 % plus rapide aux profondeurs 7-8 ; voir
 * le rapport, D7).
 *
 * On choisit l'implémentation à la compilation (un seul des .c est lié à
 * l'exécutable) :
 * \b make \b MOVELIST=linked ou \b make \b MOVELIST=dynamic (défaut).
 *
 * \warning Le parcours par get_next() rend des pointeurs de noeuds dont la
 * validité dépend de l'implémentation : avec \b dynamic_movelist.c, un
 * push_movelist() peut réallouer le tableau et donc invalider tout pointeur de
 * noeud obtenu avant lui sur la \b même pile. Ne pas empiler dans une pile
 * qu'on est en train de parcourir.
 *
 * API commune :
 * - create_movelist() pour initialiser une pile de coups vide.
 * - free_movelist() pour désallouer la mémoire d'une pile de coups.
 * - is_empty_movelist() pour tester si la pile est vide.
 * - push_movelist() pour empiler un coup au sommet de la pile.
 * - pop_movelist() pour dépiler et retourner le coup au sommet de la pile.
 * - get_current_move() pour récupérer le coup porté par le noeud \b m.
 * - get_next() pour récupérer le noeud suivant lors d'un parcours.
 * - length() pour connaître le nombre de coups empilés.
 *
 * \author LECOMPAGNON Mathéo
 * \date 10th June 2026
 */

#include <stdint.h>
#include <stdlib.h>

#include "type.h"

/**
 * \brief Enumération \b movetype décrivant la nature d'un coup.
 * \details Permet à apply_move de savoir comment répercuter le coup sans avoir
 * à redéduire sa nature. La promotion reste portée par
 * le champ \b promotion et se combine avec MOVE_NORMAL ou MOVE_CAPTURE.
 */
typedef enum movetype {
  MOVE_NORMAL = 0, /**< Déplacement simple, case d'arrivée vide */
  MOVE_CAPTURE =
      1, /**< Prise classique : pièce adverse sur la case d'arrivée */
  MOVE_DOUBLE_PUSH = 2, /**< Avance de deux cases d'un pion */
  MOVE_EN_PASSANT =
      3, /**< Prise en passant : le pion pris n'est pas sur la case d'arrivée */
  MOVE_CASTLE_KING = 4, /**< Petit roque */
  MOVE_CASTLE_QUEEN = 5 /**< Grand roque */
} movetype;

/**
 * \brief Structure permettant de modéliser un coup.
 */
typedef struct move {
  cell from;           /**<Pièce de départ */
  cell to;             /**<Pièce d'arrivée */
  piecetype type;      /**<Type de la pièce jouée */
  piecetype promotion; /**<Type de la promotion choisie si un pion arrive à la
                          dernière ligne */
  movetype kind;       /**<Nature du coup */
} move;

/**
 * \brief Type opaque représentant la pile des coups.
 * \details La structure concrète \b struct \b movelist n'est volontairement pas
 * définie dans ce header : chaque implémentation ( \b linked_movelist.c, \b
 * dynamic_movelist.c, ...) en donne sa propre définition dans son fichier
 * source. Le code client manipule donc uniquement le pointeur opaque et passe
 * par les fonctions de l'API ci-dessous (jamais par un accès direct aux
 * champs).
 */
typedef struct movelist *movelist;

/**
 * \brief create_movelist() initialise une \b movelist vide (une sentinelle
 * seule).
 * \return la \b movelist initialisée
 */
movelist create_movelist();

/**
 * \brief free_movelist(m) désalloue la mémoire utilisée pour \b m.
 * \details Parcours itératif pour ne pas consommer la pile d'appels sur une
 * longue liste.
 * \param m un \b movelist bien initialisé.
 */
void free_movelist(movelist m);

/**
 * \brief is_empty_movelist(m) teste si la pile \b m ne contient aucun coup.
 * \pre \b m bien initialisée.
 * \param m une \b movelist
 * \return 1 si la pile est vide, 0 sinon.
 */
uint8_t is_empty_movelist(movelist m);

/**
 * \brief push_movelist(c, m) empile un coup \b c au sommet de la pile \b m.
 * \pre \b m bien initialisée.
 * \param c un coup
 * \param m une \b movelist
 */
void push_movelist(move c, movelist m);

/**
 * \brief pop_movelist(m) dépile et retourne le coup au sommet de la pile \b m.
 * \pre \b m bien initialisée et non vide.
 * \param m une \b movelist
 * \return le coup qui était au sommet de la pile.
 */
move pop_movelist(movelist m);

/**
 * \brief get_current_move(m) retourne le coup porté par le noeud \b m.
 * \pre \b m est un noeud réel rendu par get_next() (pas l'entête de pile).
 * \param m une \b movelist
 * \return le coup porté par le noeud \b m.
 */
move get_current_move(movelist m);

/**
 * \brief get_next(m) retourne le noeud suivant de la liste \b m.
 * \pre \b m bien initialisée.
 * \param m une \b movelist
 * \return le noeud suivant, NULL en fin de liste.
 */
movelist get_next(movelist m);

/**
 * \brief length(m) retourne le nombre de coups atteignables depuis \b m par
 * get_next().
 * \details Usage normal : appelée sur l'entête de pile, elle rend le nombre de
 * coups empilés. Appelée sur un noeud rendu par get_next(), elle rend le nombre
 * de coups qui le \b suivent, lui \b non \b compris (donc 0 sur le dernier
 * noeud). Cette précision n'est pas un détail : les deux implémentations
 * doivent être indiscernables sur ce point, et l'une d'elles a déjà divergé de
 * 1 ici.
 * \pre \b m bien initialisée.
 * \param m une \b movelist
 * \return le nombre de coups accessibles après \b m.
 */
uint32_t length(movelist m);

#endif
