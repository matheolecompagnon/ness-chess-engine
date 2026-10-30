/**
 * \file dynamic_movelist.c
 * \brief Implémentation de l'interface \b movelist.h par un tableau dynamique.
 * \details Seconde implémentation interchangeable de la pile de coups (voir \b
 * movelist.h et
 * \b linked_movelist.c). Ici les coups sont rangés dans un \b tableau contigu
 * réalloué par doublement : on ne paie plus un \b malloc par coup empilé mais
 * une allocation par pile (et éventuellement quelques \b realloc), et le
 * parcours devient séquentiel en mémoire.
 *
 * Représentation. La structure \b struct \b movelist joue deux rôles distincts
 * :
 * - l'\b entête (le pointeur rendu par create_movelist(), celui que détient le
 * code client) porte le tableau \b buf, sa capacité \b cap et le nombre de
 * coups empilés (champ \b idx) ;
 * - un \b noeud (élément de \b buf, rendu par get_next()) porte un coup \b c et
 * son index \b idx dans le tableau. On distingue les deux par \b buf : non NULL
 * sur l'entête, NULL sur un noeud.
 *
 * Ordre de parcours. La pile est un LIFO : le sommet est la \b fin du tableau.
 * get_next(entête) rend donc le dernier coup empilé, puis on descend vers
 * l'index 0 dont le suivant est NULL. Le code client voit exactement le même
 * ordre qu'avec la liste chaînée.
 *
 * \warning Un \b realloc déplace le tableau : tout pointeur de noeud obtenu par
 * get_next() est invalidé par un push_movelist() ultérieur sur la \b même pile.
 * C'est la seule contrainte ajoutée par rapport à la liste chaînée ; aucun
 * appelant du projet ne parcourt une pile en l'alimentant simultanément
 * (rule_iter.c parcourt \b pseudo tout en remplissant une \b autre pile).
 *
 * \author LECOMPAGNON Mathéo
 * \date 20th July 2026
 */

#include <stdlib.h>

#include "../headers/movelist.h"

/**
 * \internal
 * \brief Capacité initiale du tableau de coups.
 * \details Une position d'échecs offre typiquement 30 à 40 coups légaux (218
 * dans le pire cas théorique). Partir de 64 évite la quasi-totalité des
 * réallocations sans gaspiller de mémoire.
 */
#define MOVELIST_INITIAL_CAPACITY 64

/**
 * \brief Définition concrète (privée) de la pile de coups pour cette
 * implémentation.
 * \details Sert à la fois d'entête de pile et d'élément du tableau (voir
 * l'entête du fichier).
 */
struct movelist {
  move c;       /**< Noeud : coup porté. Entête : inutilisé */
  uint32_t idx; /**< Noeud : index dans le tableau. Entête : nombre de coups
                   empilés */
  uint32_t cap; /**< Noeud : inutilisé. Entête : capacité du tableau*/
  struct movelist *buf; /**< Noeud : NULL (sert de discriminant). Entête :
                           tableau des coups [bloc des données] */
};

movelist create_movelist() {
  /* création de l'entête */
  movelist m = malloc(sizeof(struct movelist));
  m->idx = 0; // pile vide
  m->cap = MOVELIST_INITIAL_CAPACITY;

  /* création du tableau des coups */
  m->buf = malloc(MOVELIST_INITIAL_CAPACITY * sizeof(struct movelist));
  return m;
}

void free_movelist(movelist m) {
  // Une seule libération pour tous les coups
  free(m->buf);
  // On libère l'entête de pile
  free(m);
  return;
}

uint8_t is_empty_movelist(movelist m) {
  // Entête : la pile est vide si aucun coup n'est empilé. Noeud : "vide"
  // signifie qu'il n'y a plus de suivant, donc qu'on est au fond du tableau.
  // Dans les deux cas, idx == 0.
  return m->idx == 0;
}

void push_movelist(move c, movelist m) {
  if (m->idx == m->cap) { // tableau plein : on double la capacité
    m->cap *= 2;
    m->buf = realloc(m->buf, m->cap * sizeof(struct movelist));
  }
  movelist n = &m->buf[m->idx];
  n->c = c;
  n->idx = m->idx;
  n->buf = NULL; // marque l'élément comme un noeud et non comme une entête
  m->idx++;
}

move pop_movelist(movelist m) {
  // la pile n'est pas vide (m->idx > 0) : le sommet est le dernier élément du
  // tableau.
  m->idx--;
  return m->buf[m->idx].c;
}

move get_current_move(movelist m) { return m->c; }

movelist get_next(movelist m) {
  if (m->buf != NULL) { // entête : le suivant est le sommet de la pile,
                        // c'est-à-dire le dernier coup
    return (m->idx == 0) ? NULL : &m->buf[m->idx - 1];
  }
  // noeud : on descend vers le fond de la pile, l'index 0 n'a pas de suivant
  return (m->idx == 0) ? NULL : (m - 1);
}

uint32_t length(movelist m) {
  /*
   * Aucune branche n'est nécessaire ici : les deux rôles se rejoignent.
   * - Entête : idx EST le nombre de coups empilés, tenu à jour par push/pop.
   * - Noeud d'index i : les coups qui le SUIVENT dans le parcours sont ceux
   *   d'index i-1, ..., 0, soit exactement i coups.
   * C'est aussi la sémantique de linked_movelist.c, dont la boucle compte les
   * noeuds situés APRES m (elle s'arrête sur le dernier sans le compter).
   * Rendre idx+1 sur un noeud, comme le faisait la version précédente,
   * décalait cette implémentation de 1 par rapport à l'autre : deux
   * implémentations d'une interface opaque doivent être indiscernables.
   */
  return m->idx;
}
