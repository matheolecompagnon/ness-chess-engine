#!/bin/sh
# ============================================================================
# bench/compare.sh - Benchmark comparatif reproductible des optimisations Ness.
#
# Utiliser un autre nnue que nnue_stub.c, qui ne donne aucune évaluation de coup
#
# Reconstruit le moteur dans chaque état successif via des interrupteurs de
# compilation et lance le benchmark de recherche sur la position de départ.
# Chaque configuration isole la contribution d'une optimisation précise.
#
#   Config 1 : tri inversé + sans fusion depth==1 + -O0 (état initial)
#   Config 2 : tri corrigé + sans fusion depth==1 + -O0 (isole : tri)
#   Config 3 : tri corrigé + sans fusion depth==1 + -O3 (isole : compilateur)
#   Config 4 : tri corrigé + fusion depth==1      + -O3 (isole : fusion)
#   Config 5 : Config 4    + movelist dynamique         (isole : allocations)
#   Config 6 : Config 5    + table de transposition     (isole : hachage TT)
#
# Métrique de référence : nombre de noeuds (déterministe).
# Usage : sh bench/compare.sh [nom_du_nnue]
# ============================================================================

set -e
cd "$(dirname "$0")/.."

rm algorithm/headers/net_config.h

# Récupération du nom du réseau NNUE passé en argument, ou 'nnue_stub' par défaut
NNUE_NAME="${1:-nnue_stub}"

# prépare les fichiers de configuration du nnue
make prepare NET=$NNUE_NAME 

INC="-Itypes/headers -Irules/headers -Ialgorithm/headers -Iuci/headers"

# Fichiers sources communs de tous les modules du projet
SRC_TYPES="types/src/bitboard.c types/src/type.c"
SRC_RULES="rules/src/rule_iter.c rules/src/apply_move.c"
SRC_ALGO="algorithm/src/negamax.c algorithm/src/repetition.c algorithm/src/tt.c algorithm/src/zobrist.c"
SRC_UCI="uci/src/book.c uci/src/polyglot_keys.c uci/src/uci.c"

# Sélection conditionnelle du module NNUE pour éviter les conflits de symboles
if [ "$NNUE_NAME" = "nnue_stub" ]; then
    SRC_NNUE="algorithm/src/nnue_stub.c"
else
    SRC_NNUE="algorithm/src/nnue.c algorithm/src/${NNUE_NAME}.c"
fi

LIB="$SRC_TYPES $SRC_RULES $SRC_ALGO $SRC_UCI $SRC_NNUE"
BENCH="bench/bench.c"
OUT=/tmp/ness_cmp_bin

# run <titre> <flags de compilation> <profondeur max> [implementation de movelist]
run() {
    title="$1"; flags="$2"; maxd="$3"; impl="${4:-linked}"
    gcc $flags -std=c11 -DNESS_BENCH $INC "types/src/${impl}_movelist.c" $LIB $BENCH -o "$OUT"
    echo "############################################################"
    echo "# $title"
    echo "#   flags: $flags"
    echo "############################################################"
    "$OUT" "$maxd"
    echo ""
}

echo "Machine : $(uname -m) / $(uname -s)   -   $(date 2>/dev/null || echo 'date indispo')"
echo "NNUE utilisé : $NNUE_NAME"
echo ""

# Config 1 limitée à la profondeur 6 pour éviter les temps d'exécution excessifs.
run "Config 1 - tri INVERSE, sans fusion, -O0 (état initial)" \
    "-O0 -DNESS_DISABLE_TT -DNESS_ORDER_REVERSED -DNESS_DISABLE_FOLD" 6

run "Config 2 - tri corrigé, sans fusion, -O0" \
    "-O0 -DNESS_DISABLE_TT -DNESS_DISABLE_FOLD" 8

run "Config 3 - tri corrigé, sans fusion, -O3 -march=native" \
    "-O3 -march=native -DNESS_DISABLE_TT -DNESS_DISABLE_FOLD" 8

run "Config 4 - tri corrigé, fusion depth==1, -O3 -march=native, movelist chaînée" \
    "-O3 -march=native -DNESS_DISABLE_TT" 8

run "Config 5 - Config 4 + movelist tableau dynamique" \
    "-O3 -march=native -DNESS_DISABLE_TT" 8 dynamic

run "Config 6 - Config 5 + table de transposition (ACTUEL)" \
    "-O3 -march=native" 9 dynamic

rm -f "$OUT"
echo "Terminé. Les chiffres peuvent être consignés pour le suivi des performances."
