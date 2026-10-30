/**
 * \file repetition.c
 * Ce fichier implémente la détection de la nulle par répétition triple (voir
 * repetition.h). L'historique des positions est un simple tableau de clés de
 * Zobrist, empilé le long du chemin exploré et dépilé au retour, exactement
 * comme l'accumulateur NNUE est avancé par apply_move() et restauré par
 * undo_move().
 *
 * \author LECOMPAGNON Mathéo
 * \date 21st July 2026
 */

#include "repetition.h"

/**
 * \internal
 * \brief Nombre maximal de positions mémorisées.
 * \details Une partie d'échecs dépasse rarement 200 coups (400 demi-coups) et
 * la recherche n'ajoute que sa profondeur courante. 1024 laisse donc une marge
 * confortable ; au-delà, on cesse simplement d'empiler (voir
 * repetition_push()), ce qui dégrade la détection sans jamais corrompre la
 * mémoire.
 */
#define REPETITION_MAX 1024

/**
 * \internal
 * \brief Portée du parcours arrière, en demi-coups.
 * \details Ce n'est pas une approximation : deux positions identiques ne
 * peuvent être séparées que par des coups \b réversibles. Dès qu'un pion avance
 * ou qu'une pièce est prise, la position d'avant est définitivement hors
 * d'atteinte. Or 100 demi-coups sans poussée de pion ni prise font intervenir
 * la règle des 50 coups, qui clôt la partie par la nulle. Remonter au-delà de
 * 100 demi-coups est donc inutile, et cela borne le coût du test à chaque
 * noeud.
 */
#define REPETITION_LOOKBACK 100

/**
 * \internal
 * \brief Les clés des positions rencontrées, de la plus ancienne à la plus
 * récente.
 */
static uint64_t rep_keys[REPETITION_MAX];

/**
 * \internal
 * \brief Nombre de positions actuellement mémorisées.
 */
static uint16_t rep_len = 0;

/**
 * \internal
 * \brief Indice de la première position appartenant à l'arbre de recherche.
 * \details Tout ce qui est d'indice strictement inférieur relève de la partie
 * réellement jouée.
 */
static uint16_t rep_root = 0;

void repetition_clear(void) {
  rep_len = 0;
  rep_root = 0;
}

void repetition_push(uint64_t key) {
  // Débordement : on cesse d'empiler plutôt que d'écrire hors du tableau. La
  // détection devient incomplète, mais le moteur reste correct.
  if (rep_len >= REPETITION_MAX) {
    return;
  }
  rep_keys[rep_len] = key;
  rep_len++;
}

void repetition_pop(void) {
  if (rep_len > 0) {
    rep_len--;
  }
}

void repetition_begin_search(uint64_t root_key) {
  // Benchmark et --selftest ne passent pas par l'UCI : sans cet ajout, la
  // position racine ne figurerait dans aucun historique et un retour vers elle
  // passerait inaperçu.
  if (rep_len == 0) {
    repetition_push(root_key);
  }

  /*
   * La racine est le DERNIER élément de l'historique, et c'est elle qui ouvre
   * le chemin de recherche : rep_len-1, donc, et non rep_len. Y revenir en
   * cours de recherche doit compter comme une répétition d'arbre (une
   * occurrence suffit), car le joueur au trait peut alors ramener la partie à
   * cette position autant de fois qu'il le veut. Placer la frontière un cran
   * plus loin ferait passer ce retour pour une simple occurrence de
   * l'historique de partie, et il faudrait alors une répétition de plus pour
   * conclure.
   */
  rep_root = (uint16_t)(rep_len - 1);
}

uint8_t repetition_is_draw(uint64_t key) {
#ifdef NESS_REP_STRICT_3FOLD
  unsigned int seen =
      0; // Nombre d'occurrences antérieures trouvées dans la partie jouée
#endif

  /*
   * La position courante n'est pas encore empilée : les positions ayant le même
   * joueur au trait sont donc celles d'indice rep_len-2, rep_len-4, etc. On
   * remonte de deux en deux, ce qui divise par deux le nombre de comparaisons
   * et évite de confondre deux positions de disposition identique mais de trait
   * opposé.
   */
  int i = (int)rep_len - 2;
  int stop = (int)rep_len -
             REPETITION_LOOKBACK; // Borne arrière (cf. REPETITION_LOOKBACK)
  if (stop < 0) {
    stop = 0;
  }

  for (; i >= stop; i -= 2) {
    if (rep_keys[i] != key) {
      continue;
    }

    // Occurrence trouvée dans le chemin de la recherche : une seule suffit.
    if (i >= (int)rep_root) {
      return 1;
    }

#ifdef NESS_REP_STRICT_3FOLD
    // Ancien comportement (drapeau) : dans la partie jouée, il faut la vraie
    // triple occurrence (deux antérieures), car la position courante en serait
    // la troisième.
    seen++;
    if (seen >= 2) {
      return 1;
    }
#else
    /*
     * Défaut : une SEULE occurrence antérieure dans la partie jouée suffit à
     * traiter la position comme nulle DANS LA RECHERCHE. Combinée au contempt
     * (negamax.c), cette règle interdit au camp GAGNANT de revenir sur une
     * position déjà vue : il doit visiter des positions neuves, donc progresser
     * vers le mat au lieu de "tourner en rond" et concéder la nulle par
     * répétition dans une finale gagnée. C'est la convention des moteurs forts
     * ("2-fold = nulle en recherche") : si le joueur au trait peut ramener la
     * partie à une position déjà rencontrée, il peut recommencer, donc la
     * recherche traite la nulle comme acquise. L'arbitre, lui, exige toujours
     * la vraie triple occurrence - que ce refus systématique de répéter, côté
     * gagnant, finit précisément par éviter.
     */
    return 1;
#endif
  }
  return 0;
}
