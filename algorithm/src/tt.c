/**
 * \file tt.c
 * Ce fichier implémente la table de transposition décrite dans tt.h : un
 * tableau statique indexé par les bits de poids faible de la clé Zobrist, avec
 * une entrée par case.
 *
 * \author LECOMPAGNON Mathéo
 * \date 21th July 2026
 */

#include "tt.h"
#include "negamax.h"

/** \internal Nombre d'entrées de la table. */
#define TT_SIZE (1UL << TT_BITS)
/** \internal Masque des bits de la clé servant d'index (la taille est une
 * puissance de deux). */
#define TT_MASK (TT_SIZE - 1UL)

/**
 * \internal
 * \brief Seuil au-dessus duquel un score est considéré comme un score de mat.
 * \details Un mat vaut +/-SCORE_MATE corrigé de la distance à la racine, qui
 * reste petite (quelques dizaines de demi-coups). Tout score au-delà de ce
 * seuil est donc un mat et doit être normalisé avant mémorisation (voir
 * to_tt_score()).
 */
#define TT_MATE_LIMIT ((score)(SCORE_MATE - 512))

/**
 * \internal
 * \brief Une entrée de la table.
 * \details \b depth vaut 0 tant que l'entrée n'a jamais servi : comme on
 * n'enregistre jamais de recherche de profondeur 0, cette valeur marque sans
 * ambiguïté une case vide, et évite de confondre une case neuve avec une
 * position dont la clé vaudrait 0.
 */
typedef struct tt_entry {
  uint64_t key;  /**< Clé Zobrist complète : signature vérifiée à la lecture. */
  move best;     /**< Meilleur coup trouvé pour cette position. */
  score value;   /**< Score mémorisé, normalisé pour les mats. */
  uint8_t depth; /**< Profondeur de la recherche ayant produit \b value ; 0 =
                    entrée vide. */
  uint8_t flag;  /**< Nature de \b value (\b tt_flag). */
  uint8_t has_move; /**< 1 si \b best est significatif. */
} tt_entry;

/** \internal La table. Statique : aucune allocation dynamique, mémoire fixe et
 * connue. */
static tt_entry table[TT_SIZE];

void tt_clear(void) {
  for (unsigned long i = 0; i < TT_SIZE; i++) {
    table[i].key = 0;
    table[i].depth = 0;
    table[i].has_move = 0;
  }
}

/**
 * \internal
 * \brief to_tt_score(v, ply) normalise un score avant mémorisation.
 * \details Un score de mat est relatif à la racine de la recherche ("mat en 3
 * coups depuis la position de départ"). Or la même position peut être atteinte
 * à une autre distance de la racine lors d'une recherche ultérieure. On
 * mémorise donc la distance au mat \b depuis \b la \b position elle-même,
 * indépendante du chemin, et from_tt_score() refait la conversion inverse à la
 * lecture.
 * \param v le score à mémoriser.
 * \param ply la distance en demi-coups entre la racine et la position.
 * \return le score normalisé.
 */
static score to_tt_score(score v, uint16_t ply) {
  if (v > TT_MATE_LIMIT) {
    return (score)(v + (score)ply);
  }
  if (v < -TT_MATE_LIMIT) {
    return (score)(v - (score)ply);
  }
  return v;
}

/**
 * \internal
 * \brief from_tt_score(v, ply) refait la conversion inverse de to_tt_score().
 * \param v le score lu dans la table.
 * \param ply la distance en demi-coups entre la racine et la position.
 * \return le score exprimé depuis la racine de la recherche en cours.
 */
static score from_tt_score(score v, uint16_t ply) {
  if (v > TT_MATE_LIMIT) {
    return (score)(v - (score)ply);
  }
  if (v < -TT_MATE_LIMIT) {
    return (score)(v + (score)ply);
  }
  return v;
}

tt_result tt_probe(uint64_t key, uint8_t depth, score alpha, score beta,
                   uint16_t ply) {
  tt_result r = {0, 0, 0, 0, {0, 0, PAWN, EMPTY, MOVE_NORMAL}};
  tt_entry *e = &table[key & TT_MASK];

  // depth == 0 marque une case jamais écrite ; la clé complète départage les
  // collisions.
  if (e->depth == 0 || e->key != key) {
    return r;
  }

  r.hit = 1;
  r.has_move = e->has_move;
  r.best = e->best;

  // Un score obtenu moins profondément que la recherche en cours ne dit rien
  // d'assez sûr : on garde le coup pour l'ordonnancement, mais pas la valeur.
  if (e->depth < depth) {
    return r;
  }

  score v = from_tt_score(e->value, ply);
  if (e->flag == TT_EXACT) {
    r.cutoff = 1;
    r.value = v;
  } else if (e->flag == TT_LOWER && v >= beta) {
    // Le vrai score est au moins v, donc au moins beta : la coupure aurait lieu
    // de toute façon.
    r.cutoff = 1;
    r.value = v;
  } else if (e->flag == TT_UPPER && v <= alpha) {
    // Le vrai score est au plus v, donc au plus alpha : cette branche ne peut
    // rien améliorer.
    r.cutoff = 1;
    r.value = v;
  }
  return r;
}

uint8_t tt_probe_move(uint64_t key, move *out) {
  tt_entry *e = &table[key & TT_MASK];
  if (e->depth == 0 || e->key != key || !e->has_move) {
    return 0;
  }
  *out = e->best;
  return 1;
}

void tt_store(uint64_t key, uint8_t depth, tt_flag flag, score value, move best,
              uint8_t has_move, uint16_t ply) {
  // depth == 0 est la marque d'une case vide : on ne mémorise donc jamais une
  // telle recherche (elle n'aurait de toute façon aucune valeur, une feuille se
  // réévalue plus vite qu'elle ne se retrouve dans la table).
  if (depth == 0) {
    return;
  }

  tt_entry *e = &table[key & TT_MASK];

  /*
   * Politique de remplacement : on écrit si la case est vide, si elle
   * appartient à une autre position, ou si la nouvelle recherche est au moins
   * aussi profonde. Autrement dit, on ne remplace jamais un résultat profond
   * (coûteux à obtenir) par un résultat superficiel de la même position.
   */
  if (e->depth != 0 && e->key == key && e->depth > depth) {
    return;
  }

  e->key = key;
  e->value = to_tt_score(value, ply);
  e->depth = depth;
  e->flag = (uint8_t)flag;
  e->has_move = has_move;
  if (has_move) {
    e->best = best;
  }
}
