/**
 * \file linked_movelist.c
 * \brief Implémentation de l'interface \b movelist.h par une liste chaînée
 * simple.
 * \details Pile (LIFO) avec un noeud sentinelle en tête ; les coups réels sont
 * portés par les noeuds suivants. L'empilage et le dépilage se font en tête
 * (après la sentinelle) en O(1). Une allocation \b malloc est faite par coup
 * empilé.
 *
 * \author LECOMPAGNON Mathéo
 * \date 10th June 2026
 */

#include <stdlib.h>

#include "../headers/movelist.h"

/**
 * \brief Définition concrète (privée) de la pile de coups pour cette
 * implémentation.
 * \details Liste chaînée simple : chaque noeud porte un coup et pointe vers le
 * suivant.
 */
struct movelist {
  move
      c; /**< Coup porté par le noeud courant (indéterminé sur la sentinelle) */
  struct movelist *next; /**< Noeud suivant, NULL en fin de liste */
};

movelist create_movelist() {
  movelist m = malloc(sizeof(struct movelist));
  m->next = NULL;
  return m;
}

void free_movelist(movelist m) {
  while (m != NULL) {
    movelist next = m->next;
    free(m);
    m = next;
  }
  return;
}

uint8_t is_empty_movelist(movelist m) { return m->next == NULL; }

void push_movelist(move c, movelist m) {
  movelist new_node = malloc(sizeof(struct movelist));
  new_node->c = c;
  new_node->next = m->next;
  m->next = new_node;
}

move pop_movelist(movelist m) {
  // la pile n'est pas vide (m->next != NULL).
  movelist top = m->next;
  move c = top->c;
  m->next = top->next;
  free(top);
  return c;
}

move get_current_move(movelist m) { return m->c; }

movelist get_next(movelist m) { return m->next; }

uint32_t length(movelist m) {
  uint16_t compteur = 0;
  // Parcours de la liste m
  while (!is_empty_movelist(m)) {
    m = get_next(m);
    compteur++;
  }
  return compteur;
}
