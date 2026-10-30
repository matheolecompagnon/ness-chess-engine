#include <stdio.h>
#include <string.h>

#include "../rules/headers/rule_iter.h"
#include "../types/headers/movelist.h"
#include "apply_move.h"
#include "bitboard.h"
#include "negamax.h"
#include "nnue.h"
#include "type.h"
#include "uci.h"

static void square_name(cell sq, char *buf) {
  buf[0] = 'a' + (sq % 8);
  buf[1] = '1' + (sq / 8);
  buf[2] = '\0';
}

// Vrai si l'accumulateur incrémental cb->acc est égal à un recalcul complet
// depuis les pièces.
static int acc_is_consistent(chessboard cb) {
  accumulator_pair saved = cb->acc; // sauvegarde de la version incrémentale
  nnue_refresh(cb);                 // recalcule from scratch dans cb->acc
  int ok = (memcmp(&saved, &cb->acc, sizeof(accumulator_pair)) == 0);
  cb->acc = saved; // on remet la version incrémentale
  return ok;
}

// Compte les noeuds feuilles de l'arbre légal à la profondeur donnée
// (make/unmake).
static uint64_t perft(chessboard cb, int depth) {
  if (depth == 0)
    return 1;
  movelist moves = generate_move(cb);
  uint64_t nodes = 0;
  while (!is_empty_movelist(moves)) {
    move c = pop_movelist(moves);
    undo_info u = apply_move(c, cb);
    nodes += perft(cb, depth - 1);
    undo_move(c, cb, u);
  }
  free_movelist(moves);
  return nodes;
}

// Perft qui vérifie en plus la cohérence de l'accumulateur à chaque noeud.
static uint64_t perft_acc(chessboard cb, int depth, int *acc_ok) {
  if (!acc_is_consistent(cb))
    *acc_ok = 0;
  if (depth == 0)
    return 1;
  movelist moves = generate_move(cb);
  uint64_t nodes = 0;
  while (!is_empty_movelist(moves)) {
    move c = pop_movelist(moves);
    undo_info u = apply_move(c, cb);
    nodes += perft_acc(cb, depth - 1, acc_ok);
    undo_move(c, cb, u);
  }
  free_movelist(moves);
  return nodes;
}

// Batterie de tests de non-regression (perft, accumulateur NNUE, recherche).
static int selftest(void) {
  chessboard cb = create_chessboard();
  uci_set_startpos(cb);
  nnue_refresh(cb); // base correcte pour l'accumulateur

  // --- 1. Perft : correction de generate_move + apply_move + undo_move ---
  const uint64_t expected[] = {1, 20, 400, 8902, 197281, 4865609};
  int perft_ok = 1;
  printf("== Perft (position de depart) ==\n");
  for (int d = 1; d <= 5; d++) {
    uint64_t n = perft(cb, d);
    int ok = (n == expected[d]);
    perft_ok &= ok;
    printf("  profondeur %d : %10llu attendu %10llu  [%s]\n", d,
           (unsigned long long)n, (unsigned long long)expected[d],
           ok ? "OK" : "ECHEC");
  }

  // --- 2. Cohérence de l'accumulateur NNUE le long de l'arbre (profondeur 3)
  int acc_ok = 1;
  perft_acc(cb, 3, &acc_ok);
  printf("== Accumulateur NNUE incremental == [%s]\n", acc_ok ? "OK" : "ECHEC");

  // --- 3. Le board est-il intact apres tout ce make/unmake ? ---
  int board_ok = acc_is_consistent(cb) && get_chessboard_color(cb) == WHITE &&
                 get_en_passant_target(cb) == -1 &&
                 get_bit(get_piece_bitboard(cb, WHITE, KING), 4) &&
                 get_bit(get_piece_bitboard(cb, BLACK, KING), 60);
  printf("== Board restaure a l'identique == [%s]\n",
         board_ok ? "OK" : "ECHEC");

  // --- 4. Un coup de recherche pour la route ---
  move best = search_best_move(cb, 4);
  char from[3], to[3];
  square_name(best.from, from);
  square_name(best.to, to);
  printf("== Recherche == meilleur coup (prof. 4) : %s -> %s\n", from, to);

  int all_ok = perft_ok && acc_ok && board_ok;
  printf("\nRESULTAT GLOBAL : %s\n",
         all_ok ? "TOUT EST BON" : "PROBLEME DETECTE");

  free_chessboard(cb);
  return all_ok ? 0 : 1;
}

int main(int argc, char **argv) {
  // Sans argument : on parle UCI, ce qu'attend une interface graphique lancant
  // le binaire tel quel.
  if (argc > 1 && strcmp(argv[1], "--selftest") == 0)
    return selftest();
  uci_loop();
  return 0;
}
