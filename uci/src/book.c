/**
 * \file book.c
 * \brief Livre d'ouverture Polyglot (voir book.h). Compile uniquement avec make
 * BOOK=1.
 * \details Trois etapes : (1) calculer le hachage Polyglot de la position, (2)
 * chercher ce hachage dans le fichier .bin (entrees triees, recherche
 * dichotomique), (3) traduire le coup Polyglot en \b move legal du moteur. Tout
 * est cote UCI/PC : aucune influence sur le NNUE.
 *
 * Limites :
 * - Pas de variété, le coup qui est choisi est celui le plus "fort"
 *
 * \author LECOMPAGNON Mathéo
 */

#include <stdio.h>
#include <stdlib.h>

#include "bitboard.h"
#include "book.h"
#include "rule_iter.h"

/* Chemin par defaut du livre, fixe a la compilation (make BOOK=1 le renseigne).
 */
#ifndef NESS_BOOK_PATH
#define NESS_BOOK_PATH "books/komodo.bin"
#endif

/*
 * Table de correspondance entre notre piecetype (PAWN=0, ROOK=1, KNIGHT=2,
 * BISHOP=3, QUEEN=4, KING=5) et l'ordre Polyglot des pieces (pawn=1, knight=2,
 * bishop=3, rook=4, queen=5, king=6).
 */
static const int POLY_PT[6] = {
    [PAWN] = 1, [KNIGHT] = 2, [BISHOP] = 3, [ROOK] = 4, [QUEEN] = 5, [KING] = 6,
};

/* Definition :
 * Gros-boutiste - stocke les plus gros octets du nombre en premier.
 */

/* Le livre charge en memoire : un tableau d'entrees de 16 octets
 */
static unsigned char *book_data = NULL;
static size_t book_entries = 0;
static int book_tried = 0; /* pour n'essayer d'ouvrir qu'une fois */

/** \internal \brief Lit un entier 64 bits gros-boutiste (format Polyglot). */
static uint64_t rd64(const unsigned char *p) {
  uint64_t v = 0;
  for (int i = 0; i < 8; i++) {
    v = (v << 8) | p[i];
  }
  return v;
}

/** \internal \brief Lit un entier 16 bits gros-boutiste. */
static uint16_t rd16(const unsigned char *p) {
  return (uint16_t)((p[0] << 8) | p[1]);
}

/**
 * \internal
 * \brief polyglot_key(cb) calcule le hachage Polyglot de la position \b cb.
 * \details Reproduit exactement l'algorithme du format Polyglot (cf.
 * python-chess) : pieces, droits de roque, colonne en passant (SEULEMENT si une
 * prise est reellement possible), trait.
 *
 * Voir https://hgm.nubati.net/book_format.html pour l'explication du hashage
 * Zobrist (clé polyglot)
 * key = piece ^ castle ^ enpassant ^ trait
 */
static uint64_t polyglot_key(chessboard cb) {
  uint64_t key = 0;

  /* Pieces : index = 64 * (2*(type_polyglot-1) + pivot) + case ; pivot = 1 pour
   * les blancs. */
  for (color c = WHITE; c <= BLACK; c++) {
    int pivot = (c == WHITE) ? 1 : 0;
    for (piecetype t = PAWN; t <= KING; t++) {
      int piece_index = (POLY_PT[t] - 1) * 2 + pivot;
      bitboard b = get_piece_bitboard(cb, c, t);
      while (b) {
        cell sq = pop_first_cell(&b);
        key ^= POLYGLOT_KEYS[64 * piece_index + sq];
      }
    }
  }

  /* Droits de roque : (768 à 771). */
  if (get_castling_right(cb, WHITE_KING_SIDE)) {
    key ^= POLYGLOT_KEYS[768];
  }
  if (get_castling_right(cb, WHITE_QUEEN_SIDE)) {
    key ^= POLYGLOT_KEYS[769];
  }
  if (get_castling_right(cb, BLACK_KING_SIDE)) {
    key ^= POLYGLOT_KEYS[770];
  }
  if (get_castling_right(cb, BLACK_QUEEN_SIDE)) {
    key ^= POLYGLOT_KEYS[771];
  }

  /* En passant : uniquement si un pion du joueur au trait peut prendre. */
  int ep = get_en_passant_target(cb);
  if (ep >= 0) {
    color stm = get_chessboard_color(cb);
    int file = ep % 8;
    // Ligne de capture : ep est la case de la pièce prise en passant
    // caprank est la ligne où la pièce doit être posée après le coup
    int caprank = (stm == WHITE) ? (ep / 8) - 1 : (ep / 8) + 1;
    if (caprank >= 0 && caprank < 8) {
      bitboard pawns = get_piece_bitboard(cb, stm, PAWN);
      int can = 0;
      if (file > 0 && get_bit(pawns, caprank * 8 + file - 1)) {
        can = 1;
      }
      if (file < 7 && get_bit(pawns, caprank * 8 + file + 1)) {
        can = 1;
      }
      if (can) {
        key ^= POLYGLOT_KEYS[772 + file];
      }
    }
  }

  /* Trait : seul les blancs au trait.  */
  if (get_chessboard_color(cb) == WHITE) {
    key ^= POLYGLOT_KEYS[780];
  }

  return key;
}

/**
 * \internal
 * \brief book_open(path) charge le livre \b path en memoire (une seule fois).
 */
static void book_open(const char *path) {
  FILE *f = fopen(path, "rb");
  if (!f) {
    return;
  }
  // Calcul de la taille du fichier
  fseek(f, 0, SEEK_END);
  long sz = ftell(f);
  fseek(f, 0, SEEK_SET);
  // Le fichier contient des entiers 16 bits
  if (sz > 0 && sz % 16 == 0) {
    book_data = malloc((size_t)sz);
    // On récupère les données de f
    if (book_data && fread(book_data, 1, (size_t)sz, f) == (size_t)sz) {
      book_entries = (size_t)sz / 16; // Nombre d'entiers stockés
    } else {
      free(book_data);
      book_data = NULL;
    }
  }
  fclose(f);
}

/**
 * \internal
 * \brief decode_and_match(cb, pm, out) traduit un coup Polyglot 16 bits en \b
 * move legal.
 * \details Le format Polyglot encode le roque comme "le roi prend sa propre
 * tour" (e1 -> h1) : on le retraduit vers le coup de roi a deux cases du
 * moteur. Le coup est ensuite valide en le cherchant dans la liste des coups
 * legaux (ce qui fixe aussi son \b kind : prise, en passant...).
 */
static uint8_t decode_and_match(chessboard cb, uint16_t pm, move *out) {
  cell to = (cell)(((pm >> 3) & 7) * 8 + (pm & 7));
  cell from = (cell)(((pm >> 9) & 7) * 8 + ((pm >> 6) & 7));
  int promo = (pm >> 12) & 7; /* 0 aucun, 1 cavalier, 2 fou, 3 tour, 4 dame */

  /* Roque : le roi prend sa tour dans le format Polyglot. On remappe vers e1g1
   * / e1c1 / etc., mais seulement si c'est bien un ROI qui part de sa case
   * (sinon c'est un vrai coup de tour). */
  color stm = get_chessboard_color(cb);
  if (get_bit(get_piece_bitboard(cb, stm, KING), from)) {
    if (from == 4 && to == 7) {
      to = 6;
    } else if (from == 4 && to == 0) {
      to = 2;
    } else if (from == 60 && to == 63) {
      to = 62;
    } else if (from == 60 && to == 56) {
      to = 58;
    }
  }

  piecetype promo_pt = EMPTY;
  if (promo == 1) {
    promo_pt = KNIGHT;
  } else if (promo == 2) {
    promo_pt = BISHOP;
  } else if (promo == 3) {
    promo_pt = ROOK;
  } else if (promo == 4) {
    promo_pt = QUEEN;
  }

  /* Validation : on ne renvoie le coup que s'il figure dans les coups legaux de
   * la position. */
  movelist m = generate_move(cb);
  uint8_t ok = 0;
  for (movelist p = get_next(m); p != NULL; p = get_next(p)) {
    move c = get_current_move(p);
    if (c.from == from && c.to == to && c.promotion == promo_pt) {
      *out = c;
      ok = 1;
      break;
    }
  }
  free_movelist(m);
  return ok;
}

uint8_t book_probe(chessboard cb, move *out) {
  if (!book_tried) {
    book_tried = 1;
    const char *path = getenv("NESS_BOOK");
    if (!path) {
      path = NESS_BOOK_PATH;
    }
    book_open(path);
  }
  if (!book_data || book_entries == 0) {
    return 0;
  }

  uint64_t key = polyglot_key(cb);

  /* Recherche dichotomique de la premiere entree de cette cle (le fichier est
   * trie par cle). */
  size_t lo = 0, hi = book_entries;
  while (lo < hi) {
    size_t mid = lo + (hi - lo) / 2;
    if (rd64(book_data + 16 * mid) < key) {
      lo = mid + 1;
    } else {
      hi = mid;
    }
  }

  /* Parmi toutes les entrees de cette cle, on retient le coup de plus fort
   * poids. */
  uint16_t best_weight = 0, best_move = 0;
  int found = 0;
  for (size_t i = lo; i < book_entries && rd64(book_data + 16 * i) == key;
       i++) {
    uint16_t w = rd16(book_data + 16 * i + 10);
    if (!found || w > best_weight) {
      best_weight = w;
      best_move = rd16(book_data + 16 * i + 8);
      found = 1;
    }
  }
  if (!found) {
    return 0;
  }

  return decode_and_match(cb, best_move, out);
}
