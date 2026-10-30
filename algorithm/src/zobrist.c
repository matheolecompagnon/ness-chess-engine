/**
 * \file zobrist.c
 * Ce fichier implémente le hachage de Zobrist : le calcul de la clé 64 bits qui
 * identifie une position, et sur laquelle s'appuie la table de transposition.
 *
 * \author LECOMPAGNON Mathéo
 * \date 21th July 2026
 */

#include "zobrist.h"
#include "bitboard.h"

/**
 * \internal
 * \brief Graine du générateur pseudo-aléatoire.
 * \details Valeur arbitraire mais FIXE : les clés sont ainsi identiques d'une
 * exécution à l'autre, ce qui rend le nombre de noeuds du benchmark
 * reproductible.
 */
#define ZOBRIST_SEED 0x9E3779B97F4A7C15ULL

/** \internal Un nombre par (couleur, type de pièce, case). */
static uint64_t piece_keys[2][6][64];
/** \internal Un nombre pour "c'est aux noirs de jouer". */
static uint64_t side_key;
/** \internal Un nombre par combinaison des 4 droits de roque (bits 0 à 3). */
static uint64_t castling_keys[16];
/** \internal Un nombre par case cible de prise en passant. */
static uint64_t en_passant_keys[64];
/** \internal Vaut 1 dès que la table a été tirée : rend zobrist_init()
 * idempotent. */
static uint8_t zobrist_ready = 0;

/**
 * \internal
 * \brief splitmix64(state) rend le nombre pseudo-aléatoire suivant et avance
 * l'état.
 * \details Générateur volontairement minuscule : il ne sert qu'à remplir la
 * table une fois au démarrage, pas à produire de l'aléa de qualité
 * cryptographique. Aucune dépendance externe, arithmétique entière uniquement,
 * résultat identique sur toute machine.
 * \param state un pointeur vers l'état du générateur, modifié par l'appel.
 * \return un entier 64 bits pseudo-aléatoire.
 */
static uint64_t splitmix64(uint64_t *state) {
  uint64_t z = (*state += 0x9E3779B97F4A7C15ULL);
  z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
  z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
  return z ^ (z >> 31);
}

void zobrist_init(void) {
  if (zobrist_ready) {
    return;
  } // Déjà tirée : on ne rejoue pas les nombres
  uint64_t state = ZOBRIST_SEED;

  for (color c = WHITE; c <= BLACK; c++) {
    for (piecetype t = PAWN; t <= KING; t++) {
      for (cell sq = 0; sq < 64; sq++) {
        piece_keys[c][t][sq] = splitmix64(&state);
      }
    }
  }
  side_key = splitmix64(&state);
  for (int i = 0; i < 16; i++) {
    castling_keys[i] = splitmix64(&state);
  }
  for (int i = 0; i < 64; i++) {
    en_passant_keys[i] = splitmix64(&state);
  }

  zobrist_ready = 1;
}

uint64_t zobrist_key(chessboard cb) {
  zobrist_init(); // Idempotent : évite d'imposer un appel d'initialisation aux
                  // appelants

  uint64_t key = 0;

  // 1. Les pièces : on ne visite que les cases occupées
  for (color c = WHITE; c <= BLACK; c++) {
    for (piecetype t = PAWN; t <= KING; t++) {
      bitboard b = get_piece_bitboard(cb, c, t);
      while (b) {
        cell sq = pop_first_cell(&b);
        key ^= piece_keys[c][t][sq];
      }
    }
  }

  // 2. Le trait : on n'ajoute un nombre que pour un seul des deux camps, ce qui
  // suffit à distinguer la même disposition de pièces selon celui qui doit
  // jouer.
  if (get_chessboard_color(cb) == BLACK) {
    key ^= side_key;
  }

  // 3. Les droits de roque : les 4 bits forment directement l'index de la
  // combinaison
  uint8_t rights = 0;
  for (castlingtype t = WHITE_QUEEN_SIDE; t <= BLACK_KING_SIDE; t++) {
    if (get_castling_right(cb, t)) {
      rights = (uint8_t)(rights | (1u << t));
    }
  }
  key ^= castling_keys[rights];

  // 4. La cible de prise en passant, quand la règle s'applique
  int8_t ep = get_en_passant_target(cb);
  if (ep >= 0) {
    key ^= en_passant_keys[ep];
  }

  return key;
}
