/**
 * \file bench.c
 * \brief Micro-benchmark reproductible de la recherche (negamax).
 * \details Mesure, pour la position de départ et à profondeur fixe, le nombre
 * de noeuds explorés et le temps de recherche. Le nombre de noeuds est la
 * métrique de référence : déterministe et indépendante de la machine, elle
 * mesure directement l'efficacité de l'élagage (tri des coups, table de
 * transposition, PVS, null-move...). Le temps, lui, dépend du CPU et des
 * options de compilation (-O3, -march=native).
 *
 * À compiler avec -DNESS_BENCH pour activer le compteur de noeuds (voir la
 * cible "bench" du Makefile). On lance simplement :
 *     make bench            # profondeurs 4 à 8 par défaut
 *     ./bench_ness 9       # jusqu'à la profondeur 9
 *
 * Historique des mesures (position de départ, à garder à jour quand on
 * optimise) :
 *   - Bug de tri inversé (avant 2026-07-20, -O0)       :
 *           prof. 6 = 42 945 460 noeuds / ~105 s.
 *   - Tri corrigé + fusion depth==1 + -O3 (2026-07-20) :
 *           prof. 6 =    18 910 noeuds / ~0,15 s,
 *           prof. 8 =    641 167 noeuds / ~5,5 s.
 *   - movelist en tableau dynamique (2026-07-20)      : noeuds inchangés (la
 * structure de données ne change pas l'arbre), temps :
 *           prof. 8 =    7 094 ms -> 6 548 ms (-7,7 %, cf. D7).
 *   - parcours par __builtin_ctzll (2026-07-21)       : noeuds inchangés, temps
 *           prof. 8 =    -29,6 % en alternance stricte (cf. D9 ; machine
 * chargée, gain sous-estimé).
 *   - table de transposition (2026-07-21)             :
 *           prof. 8 = 358 612 noeuds (-44,1 %) / 1 851 ms,
 *           prof. 9 = 1 562 680 noeuds (-35,1 %) / 10 123 ms (cf. D10). Même
 * meilleur coup à toutes les profondeurs.
 *   - test de légalité sans malloc (2026-07-21)      : noeuds inchangés,
 * recherche :
 *           prof. 8 = 1 672 ms -> 1 487 ms (-11 %, cf. D11 ; la copie passe sur
 * la pile, apply/undo s'est révélé plus lent).
 *   - -flto (2026-07-21)                             : noeuds inchangés,
 facteur 2 (cf. D12).
 *   - Temps refaits apres D12, mediane de 5 executions :
 *       prof. 4 = 3,4 ms | 5 = 13,0 ms | 6 = 58,4 ms | 7 = 237,8 ms
 *          | 8 = 1 059 ms | 9 = 6 978 ms.
 *       Facteur de branchement effectif 4,6/demi-coup (identique en temps et en
 * noeuds).
 * Prochaine étape attendue (PVS/null-move/LMR, quiescence) : encore moins de
 * noeuds à profondeur égale, et surtout des profondeurs 10-12 atteignables.
 * Ré-exécuter ce bench après chaque ajout et reporter les chiffres ci-dessus.
 *
 * \author Mathéo Lecompagnon
 * Benchmark ajouté le 2026-07-20
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "negamax.h"
#include "type.h"
#include "uci.h"

/* Défini dans negamax.c sous -DNESS_BENCH : nombre de noeuds (appels à
 * negamax_ply). */
extern unsigned long negamax_nodes;

/* Convertit une case 0..63 en notation algébrique ("e2"). */
static void square_name(cell sq, char *buf) {
  buf[0] = (char)('a' + (sq % 8));
  buf[1] = (char)('1' + (sq / 8));
  buf[2] = '\0';
}

/* Mesure une recherche à profondeur fixe depuis la position de départ. */
static void bench_startpos(int depth) {
  chessboard cb = create_chessboard();
  uci_set_startpos(cb);

  negamax_nodes = 0;
  clock_t t0 = clock();
  move best = search_best_move(cb, (uint8_t)depth);
  clock_t t1 = clock();

  double ms = (double)(t1 - t0) * 1000.0 / (double)CLOCKS_PER_SEC;
  double knps = (ms > 0.0) ? ((double)negamax_nodes / ms) : 0.0; // knoeuds/s
  char from[3], to[3];
  square_name(best.from, from);
  square_name(best.to, to);

  printf("  %2d   | %14lu | %10.1f | %10.0f | %s%s\n", depth, negamax_nodes, ms,
         knps, from, to);

  free_chessboard(cb);
}

int main(int argc, char **argv) {
  int max_depth = (argc > 1) ? atoi(argv[1]) : 8;
  if (max_depth < 1)
    max_depth = 1;

  printf("== Benchmark Ness (position de depart, pleine force) ==\n");
  printf(" prof. |         noeuds |  temps(ms) |   knoeuds/s | coup\n");
  printf(" ------+----------------+------------+-------------+------\n");
  for (int d = 4; d <= max_depth; d++) {
    bench_startpos(d);
    fflush(stdout);
  }
  return 0;
}
