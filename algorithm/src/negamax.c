/**
 * \file negamax.c
 * Ce fichier implémente la recherche du meilleur coup : l'algorithme negamax
 * avec élagage alpha-bêta, et le tri des coups qui l'accompagne.
 * - order_moves(m, cb) trie les coups du meilleur au moins bon avec
 * l'heuristique evaluate.
 * - max(a, b) renvoie le plus grand des deux scores.
 * - negamax_ply(cb, depth, alpha, beta, ply) le negamax interne, qui connaît sa
 * distance à la racine.
 * - negamax(cb, depth, alpha, beta) renvoie le score de l'échiquier à la
 * profondeur depth.
 * - search_best_move(cb, depth) renvoie le meilleur coup pour le joueur au
 * trait.
 * - search_best_move_timed(cb, max_depth, budget_ms) fait de même sous un
 * budget de temps.
 *
 * Le moteur joue toujours à pleine force : il n'y a pas de réglage de
 * difficulté, le temps est le seul paramètre qui borne la recherche.
 *
 * \author LECOMPAGNON Mathéo
 * \date 6th July 2026
 */

/* clock_gettime(CLOCK_MONOTONIC) relève de POSIX.1-2001 : il faut le demander
 * explicitement, sans quoi -std=c11 (strictement conforme) masque la
 * déclaration. Doit précéder les includes. */
#define _POSIX_C_SOURCE 200809L

#include "negamax.h"
#include "../../rules/headers/rule_iter.h"
#include "../../types/headers/movelist.h"
#include "apply_move.h"
#include "bitboard.h"
#include "nnue.h"
#include "repetition.h"
#include "tt.h"
#include "type.h"
#include "zobrist.h"
#include <math.h>
#include <time.h>

/**
 * \internal
 * \brief Période de consultation de l'horloge, en noeuds.
 * \details La recherche doit pouvoir être interrompue en cours d'itération,
 * mais lire l'horloge à chaque noeud coûterait plus cher que le noeud lui-même
 * (un appel système, même via vDSO, pèse des dizaines de nanosecondes quand un
 * noeud en coûte quelques centaines). On ne la consulte donc qu'un noeud sur
 * SEARCH_CLOCK_PERIOD. La valeur est une puissance de deux pour que le test se
 * réduise à un ET binaire, et elle reste assez petite pour que le dépassement
 * du délai soit imperceptible (quelques millisecondes au pire).
 */
#define SEARCH_CLOCK_PERIOD 2048u

/**
 * \internal
 * \brief Contempt : pénalité (en centipions) appliquée à une nulle, du point de
 * vue du joueur à la racine.
 * \details Sans lui, une nulle vaut 0, exactement comme une position équilibrée
 * : le moteur n'a alors aucune raison d'éviter une répétition dans une position
 * pourtant favorable, et il "fait la nulle" au lieu de jouer pour gagner. En
 * rendant une nulle légèrement NÉGATIVE pour la racine, on force le moteur à
 * continuer de jouer tant qu'il n'est pas réellement moins bien que cette marge
 * - c'est-à-dire à ne prendre la nulle que quand elle le sauve vraiment.
 * Réglage prudent : 25 centipions (un quart de pion). Trop élevé, le moteur
 * refuserait des nulles salvatrices face à plus fort que lui et perdrait à la
 * place.
 */
#define NEGAMAX_CONTEMPT ((score)25)

/**
 * \internal
 * \brief same_move(a, b) teste si deux coups sont identiques.
 * \details Sert à reconnaître, parmi les coups générés, celui que la table de
 * transposition recommande. La comparaison porte sur tous les champs : deux
 * coups qui partent de la même case vers la même case peuvent différer par leur
 * promotion (dame ou cavalier).
 * \param a,b deux \b move.
 * \return 1 s'ils décrivent le même coup, 0 sinon.
 */
static uint8_t same_move(move a, move b) {
  return (uint8_t)(a.from == b.from && a.to == b.to && a.type == b.type &&
                   a.promotion == b.promotion && a.kind == b.kind);
}

/**
 * \internal
 * \brief order_moves_hint(m, cb, hint, has_hint) trie les coups, en plaçant \b
 * hint en tête.
 * \details Coeur de order_moves(). Le tri reste celui du projet - décroissant
 * selon -evaluate(), c'est-à-dire notre avantage après le coup - à une
 * exception près : si la table de transposition a déjà désigné un meilleur coup
 * pour cette position, ce coup passe devant tous les autres. Il n'est même pas
 * évalué, ce qui économise une évaluation NNUE par noeud.
 *
 * C'est de loin l'apport le plus rentable de la table : un meilleur coup joué
 * en premier fait couper alpha-bêta immédiatement sur les autres branches.
 * \param m un \b movelist non vide.
 * \param cb un \b chessboard bien initialisé.
 * \param hint le coup à placer en tête.
 * \param has_hint 0 pour ignorer \b hint (tri par évaluation seule).
 * \return la \b movelist triée.
 */
static movelist order_moves_hint(movelist m, chessboard cb, move hint,
                                 uint8_t has_hint) {
  uint16_t n = length(m);
  score scores[n]; // Tableau des scores
  move moves[n];   // Tableau des coups

  for (int i = 0; i < n; i++) {
    move c = pop_movelist(m);
    if (has_hint && same_move(c, hint)) {
      // Coup recommandé par la table de transposition : il passe en tête sans
      // être évalué.
      scores[i] = SCORE_INF;
      moves[i] = c;
      continue;
    }
    undo_info u = apply_move(c, cb); // Applique en place...
    // evaluate() rend le score du point de vue du joueur au trait. Après
    // apply_move, le trait est à l'adversaire : evaluate(cb) est donc SON
    // score, d'autant plus élevé que la position lui est favorable,
    // c'est-à-dire que le coup est mauvais POUR NOUS. On stocke l'opposé pour
    // que scores[] mesure notre avantage : le tri décroissant qui suit place
    // alors bien notre meilleur coup en tête, ce qui maximise les coupures
    // alpha-bêta (sans quoi on explorerait notre pire coup d'abord).
#ifdef NESS_ORDER_REVERSED
    // Variante "avant correction" du benchmark comparatif (D3) :
    // reproduit le tri à l'envers d'origine. À n'activer QUE pour la
    // mesure, jamais en production.
    scores[i] = evaluate(cb);
#else
    scores[i] = (score)-evaluate(cb);
#endif
    undo_move(c, cb, u); // ...puis restaure cb
    moves[i] = c;
  }
  // Tri par insertion
  for (int i = 0; i < n; i++) {
    // Le score d'index i est celui qui n'a pas encore été inséré
    int j = i;
    while (
        j > 0 &&
        scores[j] >
            scores[j - 1] // Le maximum (notre meilleur coup) remonte en place 0
    ) {
      score s_aux = scores[j - 1];
      scores[j - 1] = scores[j];
      scores[j] = s_aux;

      move c_aux = moves[j - 1];
      moves[j - 1] = moves[j];
      moves[j] = c_aux;

      j--;
    }
  }

  // On parcours le tableau des coups pour reformer la liste des coups triée
  for (int i = n - 1; i >= 0; i--) {
    push_movelist(moves[i], m);
  }

  return m;
}

movelist order_moves(movelist m, chessboard cb) {
  move none = {0, 0, PAWN, EMPTY, MOVE_NORMAL};
  return order_moves_hint(
      m, cb, none, 0); // Aucun coup privilégié : tri par évaluation seule
}

#ifdef NESS_DISABLE_TT
/*
 * Interrupteur du benchmark comparatif (D10) : neutralise
 * entièrement la table de transposition pour reproduire l'état du moteur avant
 * son ajout. La clé Zobrist n'est même pas calculée, de sorte que la mesure
 * isole bien le coût ET le gain de la table. À n'activer QUE pour la mesure,
 * jamais en production.
 */
static tt_result ness_tt_probe(uint64_t key, uint8_t depth, score alpha,
                               score beta, uint16_t ply) {
  (void)key;
  (void)depth;
  (void)alpha;
  (void)beta;
  (void)ply;
  tt_result r = {0, 0, 0, 0, {0, 0, PAWN, EMPTY, MOVE_NORMAL}};
  return r;
}
static uint8_t ness_tt_probe_move(uint64_t key, move *out) {
  (void)key;
  (void)out;
  return 0;
}
static void ness_tt_store(uint64_t key, uint8_t depth, tt_flag flag,
                          score value, move best, uint8_t has_move,
                          uint16_t ply) {
  (void)key;
  (void)depth;
  (void)flag;
  (void)value;
  (void)best;
  (void)has_move;
  (void)ply;
}
#else
#define ness_tt_probe tt_probe
#define ness_tt_probe_move tt_probe_move
#define ness_tt_store tt_store
#endif

/*
 * Clé de la position. Elle sert à DEUX choses désormais : interroger la table
 * de transposition et reconnaître une répétition. Elle est donc calculée même
 * sous NESS_DISABLE_TT, où seuls la sonde et la mémorisation sont neutralisées.
 * Conséquence pour le benchmark comparatif : la config sans table paie tout de
 * même le hachage, ce qui la rend légèrement plus lente qu'avant l'ajout de la
 * détection de répétition. Les mesures de D10 antérieures ne sont donc plus
 * directement comparables aux nouvelles ; c'est le prix d'une règle du jeu qui
 * manquait.
 */
#define NESS_REP_KEY(cb) zobrist_key(cb)

/**
 * \internal
 * \brief tt_flag_of(value, alpha_orig, beta) qualifie la nature du score \b
 * value.
 * \details Une recherche alpha-bêta ne rend une valeur exacte que si elle a
 * examiné tous les coups sans couper. Si elle a coupé sur beta, elle s'est
 * arrêtée dès qu'un coup était "assez bon" : le vrai score est au moins
 * celui-là (borne inférieure). Si aucun coup n'a atteint alpha, elle sait
 * seulement que le vrai score ne dépasse pas cette valeur (borne supérieure).
 * Mémoriser cette nature est indispensable : sans elle, une borne serait relue
 * comme une valeur.
 * \param value le score rendu par le noeud.
 * \param alpha_orig la valeur de alpha à l'entrée du noeud (avant qu'il ne la
 * relève).
 * \param beta la borne haute de la fenêtre.
 * \return le \b tt_flag correspondant.
 */
static tt_flag tt_flag_of(score value, score alpha_orig, score beta) {
  if (value >= beta) {
    return TT_LOWER;
  }
  if (value <= alpha_orig) {
    return TT_UPPER;
  }
  return TT_EXACT;
}

score max(score a, score b) {
  if (a <= b) {
    return b;
  } else {
    return a;
  }
}

/*
 * ============================================================================
 * Butoir de temps : interrompre une itération en cours
 * ============================================================================
 * Sans ce mécanisme, l'approfondissement itératif ne pouvait que refuser de
 * lancer une itération qu'il estimait trop coûteuse - et il laissait alors
 * dormir tout le temps restant (mesuré : profondeur 7 atteinte en 316 ms sur un
 * budget de 1000 ms, soit deux tiers du temps inutilisés). Désormais on lance
 * l'itération suivante dès qu'il reste du temps, quitte à l'abandonner en route
 * : le pire cas est qu'on n'en tire rien, le meilleur est un demi-coup de
 * profondeur gagné.
 *
 * Horloge MURALE (CLOCK_MONOTONIC) et non clock(), qui mesure le temps CPU :
 * une interface graphique ou un serveur de parties décompte le temps réel, et
 * clock() sous-estime dès que le processus est préempté. C'était un défaut de
 * l'implémentation précédente.
 */

/** \internal \brief Instant limite de la recherche, en millisecondes monotones.
 */
static double search_deadline_ms = 0.0;
/** \internal \brief Vrai si un butoir est armé (sinon la recherche n'est jamais
 * interrompue). */
static uint8_t search_has_deadline = 0;
/** \internal \brief Vrai dès que le butoir a été franchi : l'itération en cours
 * est caduque. */
static uint8_t search_aborted = 0;
/** \internal \brief Compteur de noeuds servant à espacer les consultations de
 * l'horloge. */
static uint32_t search_clock_countdown = 0;
/** \internal \brief Vrai si la dernière itération a pu chercher au moins un
 * coup racine en entier. */
static uint8_t search_root_has_result = 0;

/**
 * \internal
 * \brief now_ms() rend l'instant courant en millisecondes, sur une horloge
 * monotone.
 * \details Monotone : insensible aux changements d'heure système, contrairement
 * à time().
 */
static double now_ms(void) {
  struct timespec t;
  clock_gettime(CLOCK_MONOTONIC, &t);
  return (double)t.tv_sec * 1000.0 + (double)t.tv_nsec / 1000000.0;
}

/**
 * \internal
 * \brief search_arm_deadline(budget_ms) arme le butoir pour la recherche qui va
 * suivre.
 * \param budget_ms le temps alloué à partir de maintenant, en millisecondes.
 */
static void search_arm_deadline(uint32_t budget_ms) {
  search_deadline_ms = now_ms() + (double)budget_ms;
  search_has_deadline = 1;
  search_aborted = 0;
  search_clock_countdown = SEARCH_CLOCK_PERIOD;
}

/**
 * \internal
 * \brief search_disarm_deadline() désarme le butoir : la recherche redevient
 * illimitée.
 * \details Indispensable après search_best_move_timed(), sinon un appel direct
 * ultérieur à search_best_move() (le benchmark, les tests, --selftest)
 * hériterait d'un délai déjà expiré et s'interromprait aussitôt.
 */
static void search_disarm_deadline(void) {
  search_has_deadline = 0;
  search_aborted = 0;
}

/**
 * \internal
 * \brief search_time_is_up() dit si la recherche doit s'arrêter, sans lire
 * l'horloge à chaque appel.
 * \details Une fois search_aborted levé, il le reste jusqu'au prochain armement
 * : toute la récursion en cours remonte alors sans travailler davantage.
 */
static uint8_t search_time_is_up(void) {
  if (!search_has_deadline) {
    return 0;
  }
  if (search_aborted) {
    return 1;
  }
  if (--search_clock_countdown != 0) {
    return 0;
  } // pas encore l'heure de regarder
  search_clock_countdown = SEARCH_CLOCK_PERIOD;
  if (now_ms() >= search_deadline_ms) {
    search_aborted = 1;
  }
  return search_aborted;
}

/**
 * \internal
 * \brief negamax_ply(cb, depth, alpha, beta, ply) est le negamax interne.
 * \details Identique à negamax(), mais reçoit \b ply, sa distance en demi-coups
 * à la position racine. C'est ce qui permet de scorer un mat par son
 * éloignement (-SCORE_MATE + ply) : sans cette information, tous les mats se
 * vaudraient et le moteur ne saurait ni résister ni conclure.
 * \param cb un \b chessboard bien initialisé
 * \param depth la profondeur restante
 * \param alpha,beta la fenêtre alpha-bêta
 * \param ply la distance à la racine, en demi-coups
 * \return le score de \b cb du point de vue du joueur au trait.
 */
#ifdef NESS_BENCH
/* Compteur de noeuds pour le benchmark (bench/bench.c). Compilé seulement avec
 * -DNESS_BENCH : en build normal, ni la variable ni l'incrément ci-dessous
 * n'existent (aucun surcoût). */
unsigned long negamax_nodes = 0;
#endif

static score negamax_ply(chessboard cb, uint8_t depth, score alpha, score beta,
                         uint16_t ply) {
#ifdef NESS_BENCH
  negamax_nodes++;
#endif
  /*
   * Butoir de temps. On remonte immédiatement, SANS rien mémoriser : le score
   * rendu ici est arbitraire et ne vaut rien. C'est l'appelant racine qui,
   * voyant search_aborted, jettera l'itération entière. Écrire un tel score
   * dans la table de transposition l'empoisonnerait durablement, puisqu'elle
   * survit d'une itération à l'autre.
   */
  if (search_time_is_up()) {
    return 0;
  }

  /*
   * Nulle par répétition. Le test précède tout le reste, y compris la table de
   * transposition : une position répétée vaut 0 quel que soit son contenu, et
   * il serait faux de lui substituer un score mémorisé pour une autre
   * occurrence de la même disposition.
   *
   * ply>0 : à la racine on ne teste pas, sans quoi la recherche renoncerait à
   * jouer. La clé est celle de la table de transposition, déjà nécessaire ici :
   * la détection ne coûte donc aucun hachage supplémentaire (cf. NESS_REP_KEY
   * plus haut).
   */
  const uint64_t rep_key = NESS_REP_KEY(cb);
  if (ply > 0 && repetition_is_draw(rep_key)) {
    /*
     * Nulle par répétition, corrigée par le contempt. La pénalité s'applique
     * TOUJOURS du point de vue du joueur à la racine (celui pour qui on cherche
     * un coup) : c'est lui, et lui seul, qu'on veut dissuader de répéter une
     * position favorable. La parité de ply indique de qui c'est le trait à ce
     * noeud : ply pair = même camp que la racine (on rend -contempt), ply
     * impair = l'adversaire (on rend +contempt, car après la négation du
     * negamax cela redevient -contempt pour la racine). Ainsi, quel que soit
     * l'endroit de l'arbre où la répétition est détectée, la racine voit
     * toujours la nulle comme légèrement défavorable, et préfère tout coup qui
     * garde un avantage réel.
     */
    return (ply % 2 == 0) ? (score)(-NEGAMAX_CONTEMPT)
                          : (score)(NEGAMAX_CONTEMPT);
  }

  if (depth == 0) {
    // evaluate() rend déjà le score du point de vue du joueur au trait :
    // la négation du negamax gère le changement de camp, pas besoin de
    // multiplier. On évalue avant de générer les coups : les feuilles sont le
    // gros de l'arbre, et leur faire générer une movelist pour rien coûterait
    // bien plus que ça ne rapporte.
    return evaluate(cb);
  }

  /*
   * Table de transposition. La position a peut-être déjà été cherchée par un
   * autre chemin de coups, ou par l'itération précédente de l'approfondissement
   * itératif. On interroge la table avant tout travail : une entrée assez
   * profonde évite d'explorer le sous-arbre, et même lorsqu'elle ne suffit pas
   * à conclure, son meilleur coup servira à ordonner les coups.
   */
  const score alpha_orig =
      alpha; // Conservé pour savoir, à la fin, si la valeur est exacte
  const uint64_t key =
      rep_key; // Même clé que la détection de répétition : calculée une fois
  tt_result entry = ness_tt_probe(key, depth, alpha, beta, ply);
  if (entry.cutoff) {
    return entry.value;
  }

  movelist child_nodes = generate_move(cb);

  // Position terminale : sans coup légal, c'est un mat si le roi est en échec,
  // un pat sinon. Ce test doit précéder order_moves, qui suppose une liste non
  // vide.
  if (is_empty_movelist(child_nodes)) {
    free_movelist(child_nodes);
    move none = {0, 0, PAWN, EMPTY, MOVE_NORMAL};
    if (is_king_in_check(cb, get_chessboard_color(cb))) {
      score mate =
          (score)(-SCORE_MATE +
                  (score)ply); // mat : d'autant moins grave qu'il est loin
      // Un mat ou un pat est un score exact et définitif : il vaut pour toute
      // profondeur.
      ness_tt_store(key, depth, TT_EXACT, mate, none, 0, ply);
      return mate;
    }
    ness_tt_store(key, depth, TT_EXACT, 0, none, 0, ply);
    return 0; // pat : la partie est nulle, pas perdue
  }

#ifndef NESS_DISABLE_NULLMOVE
  /*
   * Null-move pruning. Si, en s'offrant le luxe de PASSER son tour (coup nul),
   * on reste au-dessus de beta a profondeur reduite, la position est trop
   * favorable pour que l'adversaire s'en sorte : on coupe sans explorer les
   * vrais coups. Un coup nul ne bouge aucune piece -> les accumulateurs NNUE
   * sont inchanges ; seul le trait bascule (evaluate() prend alors la
   * perspective adverse) et l'en passant est efface ; la cle Zobrist,
   * recalculee depuis cb, reflete le basculement. Conditions : profondeur >= 3
   * ; beta n'est pas un score de mat (sinon la fenetre pleine rend la coupure
   * impossible et le test serait du travail perdu) ; pas en echec (passer
   * laisserait le roi pris) ; GARDE-FOU ZUGZWANG : du materiel autre que des
   * pions du cote au trait - en finale de pions, passer son tour peut etre le
   * seul mauvais coup et le null-move conclurait faussement, ce qui degraderait
   * justement les finales qu'on veut bien jouer (cf. D20/D21).
   */
  if (depth >= 3 && beta < SCORE_MATE) {
    color stm_nm = get_chessboard_color(cb);
    bitboard non_pawn = get_piece_bitboard(cb, stm_nm, ROOK) |
                        get_piece_bitboard(cb, stm_nm, KNIGHT) |
                        get_piece_bitboard(cb, stm_nm, BISHOP) |
                        get_piece_bitboard(cb, stm_nm, QUEEN);
    if (non_pawn != 0 && !is_king_in_check(cb, stm_nm)) {
      const uint8_t R = 2; // reduction de profondeur appliquee au coup nul
      int16_t saved_ep = get_en_passant_target(cb);
      set_en_passant_target(cb, -1);
      set_chessboard_color(cb, (stm_nm == WHITE) ? BLACK : WHITE);
      // Fenetre nulle autour de beta : simple test de coupure (le sous-arbre
      // atteint-il beta ?).
      score null_score =
          (score)-negamax_ply(cb, (uint8_t)(depth - 1 - R), (score)-beta,
                              (score)(-beta + 1), (uint16_t)(ply + 1));
      set_chessboard_color(cb, stm_nm);
      set_en_passant_target(cb, saved_ep);
      // On ne coupe pas sur une recherche tronquee par le butoir (null_score
      // serait arbitraire).
      if (!search_aborted && null_score >= beta) {
        free_movelist(child_nodes);
        return beta; // trop bon meme en concedant un tour : coupure
      }
    }
  }
#endif

  /*
   * Cas depth==1 : tous les fils sont des feuilles. Passer par order_moves puis
   * récurser vers depth==0 évaluerait chaque fils DEUX fois (une fois pour le
   * tri, une fois pour la feuille). Or la valeur d'un noeud depth==1 est
   * simplement le meilleur des -evaluate(fils). On l'obtient en une seule
   * passe, sans tri ni récursion : sur la couche la plus peuplée de l'arbre
   * juste au-dessus des feuilles, cela divise par deux le nombre d'évaluations.
   * (NESS_DISABLE_FOLD désactive cette fusion pour le benchmark comparatif,
   * D5.)
   */
#ifndef NESS_DISABLE_FOLD
  if (depth == 1) {
    score value = -SCORE_INF;
    move best_local = {0, 0, PAWN, EMPTY, MOVE_NORMAL};
    uint8_t has_best = 0;
    while (!is_empty_movelist(child_nodes)) {
      move c = pop_movelist(child_nodes);
      undo_info u = apply_move(c, cb);
      // evaluate() est du point de vue de l'adversaire (au trait après le coup)
      // : on nie pour obtenir la valeur du coup de NOTRE point de vue, comme le
      // fait la récursion.
      score s = (score)-evaluate(cb);
      undo_move(c, cb, u);
      if (s > value) { // Nouveau meilleur coup : on le retient pour la table
        value = s;
        best_local = c;
        has_best = 1;
      }
      if (value >= beta) {
        break;
      } // coupure : on dépasse déjà beta, inutile de continuer
      if (search_time_is_up()) {
        break;
      } // butoir : la valeur ci-dessous est incomplète
    }
    free_movelist(child_nodes);
    // Une valeur obtenue sur une liste de coups partiellement parcourue est
    // fausse : on ne la mémorise pas. (Une coupure beta, elle, est une borne
    // valide et reste mémorisée.)
    if (!search_aborted) {
      ness_tt_store(key, depth, tt_flag_of(value, alpha_orig, beta), value,
                    best_local, has_best, ply);
    }
    return value;
  }
#endif

  // Le coup mémorisé par la table, s'il existe, est joué en premier : c'est lui
  // qui provoque les coupures alpha-bêta les plus précoces.
  child_nodes = order_moves_hint(child_nodes, cb, entry.best, entry.has_move);
  score value = -SCORE_INF;
  move best_local = {0, 0, PAWN, EMPTY, MOVE_NORMAL};
  uint8_t has_best = 0;
#ifndef NESS_DISABLE_PVS
  uint8_t searched_one =
      0; // PVS : 1er coup en fenetre pleine, suivants en sonde nulle
#endif
#if !defined(NESS_DISABLE_PVS) && !defined(NESS_DISABLE_LMR)
  // LMR : compteur de coups + savoir si le noeud est en echec (on ne reduit pas
  // alors).
  unsigned int move_count = 0;
  const uint8_t node_in_check = is_king_in_check(cb, get_chessboard_color(cb));
#endif

  /*
   * La position courante entre dans le chemin de recherche : les noeuds
   * explorés en dessous doivent pouvoir la reconnaître s'ils y reviennent. Un
   * seul empilement pour tout le noeud, dépilé juste après la boucle - y
   * compris lorsqu'elle s'interrompt par une coupure. Les chemins de sortie
   * situés plus haut (coupure de la table, position terminale, fusion depth==1)
   * n'ont rien empilé : ils ne récursent pas.
   */
  repetition_push(rep_key);

  /*
   * Parcours de chaque noeud fils jusqu'à la fin de la liste des fils
   * ou bien jusqu'à ce que alpha >= beta
   */
  while (!is_empty_movelist(child_nodes)) {
    move c = pop_movelist(child_nodes);
#if !defined(NESS_DISABLE_PVS) && !defined(NESS_DISABLE_LMR)
    move_count++;
#endif
    undo_info u = apply_move(c, cb); // Applique le coup en place
#ifdef NESS_DISABLE_PVS
    score s = (score)-negamax_ply(cb, depth - 1, (score)-beta, (score)-alpha,
                                  (uint16_t)(ply + 1));
#else
    /*
     * PVS / NegaScout. Le premier coup (le mieux classe : coup de la table puis
     * tri NNUE) est cherche en fenetre pleine. Les suivants sont d'abord sondes
     * en fenetre NULLE [alpha, alpha+1] : grace au bon ordre des coups, la
     * plupart echouent bas (ils ne valent pas mieux qu'alpha) pour une fraction
     * du cout. Un coup qui depasse alpha - donc potentiellement meilleur - est
     * alors re-cherche en fenetre pleine. Resultat identique, moins de noeuds.
     * (NESS_DISABLE_PVS restaure la fenetre pleine partout)
     */
    score s;
    if (!searched_one) {
      s = (score)-negamax_ply(cb, depth - 1, (score)-beta, (score)-alpha,
                              (uint16_t)(ply + 1));
    } else {
      int r = 0; // reduction LMR (0 = aucune)
#ifndef NESS_DISABLE_LMR
      /*
       * LMR (Late Move Reductions). Passe le 1er coup et les tout premiers
       * (move_count >= 4), on cherche les coups TARDIFS et TRANQUILLES a
       * profondeur reduite : le bon ordre des coups (table + tri NNUE) rend
       * improbable qu'un coup classe loin soit le meilleur. Exclus : captures,
       * en passant, promotions, coups donnant echec (tactiques), et tout coup
       * si l'on est soi-meme en echec. Si la recherche reduite depasse alpha,
       * on ne s'y fie pas : re-recherche a profondeur pleine.
       */
      if (depth >= 3 && move_count >= 4 && !node_in_check &&
          c.kind != MOVE_CAPTURE && c.kind != MOVE_EN_PASSANT &&
          c.promotion == EMPTY &&
          !is_king_in_check(cb, get_chessboard_color(cb))) {
        r = (depth >= 5 && move_count >= 6) ? 2 : 1;
      }
#endif
      // Sonde a fenetre nulle, eventuellement a profondeur reduite (LMR).
      s = (score)-negamax_ply(cb, (uint8_t)(depth - 1 - r), (score)(-alpha - 1),
                              (score)-alpha, (uint16_t)(ply + 1));
      // La reduction a echoue haut : on re-cherche a profondeur PLEINE (fenetre
      // nulle).
      if (!search_aborted && r > 0 && s > alpha) {
        s = (score)-negamax_ply(cb, depth - 1, (score)(-alpha - 1),
                                (score)-alpha, (uint16_t)(ply + 1));
      }
      // Fenetre nulle depassee : noeud PV, re-recherche en fenetre pleine.
      if (!search_aborted && s > alpha && s < beta) {
        s = (score)-negamax_ply(cb, depth - 1, (score)-beta, (score)-alpha,
                                (uint16_t)(ply + 1));
      }
    }
    searched_one = 1;
#endif
    undo_move(c, cb, u); // Restaure cb pour le coup suivant
    if (s > value) {     // Nouveau meilleur coup : on le retient pour la table
      value = s;
      best_local = c;
      has_best = 1;
    }
    alpha = max(alpha, value);
    if (alpha >= beta) {
      break;
    }
    if (search_aborted) {
      break;
    } // butoir franchi dans le sous-arbre : on remonte
  }
  repetition_pop(); // Contrepartie du repetition_push() ci-dessus : le noeud
                    // quitte le chemin
  free_movelist(child_nodes);
  // Même précaution que dans la fusion depth==1 : une valeur issue d'un
  // parcours interrompu n'est pas mémorisable, sous peine d'empoisonner la
  // table pour les itérations suivantes.
  if (!search_aborted) {
    ness_tt_store(key, depth, tt_flag_of(value, alpha_orig, beta), value,
                  best_local, has_best, ply);
  }
  return value;
}

score negamax(chessboard cb, uint8_t depth, score alpha, score beta) {
  // cb est la racine : elle est à zéro demi-coup d'elle-même.
  return negamax_ply(cb, depth, alpha, beta, 0);
}

move search_best_move_timed(chessboard cb, uint8_t max_depth,
                            uint32_t budget_ms) {
  if (max_depth < 1) {
    max_depth = 1;
  }

  /*
   * Le butoir est armé une fois pour TOUTE la réflexion, pas par itération :
   * c'est lui qui garantit qu'on rend un coup dans le délai, quelle que soit la
   * profondeur demandée.
   */
  const double start = now_ms();
  search_arm_deadline(budget_ms);

  // Profondeur 1 : quasi instantanée, elle garantit qu'on renvoie toujours un
  // coup légal (ou le coup nul si mat/pat), même si le budget est minuscule.
  move best = search_best_move(cb, 1);

  /*
   * On lance l'itération suivante dès qu'il reste une part utile du budget,
   * SANS chercher à prédire si elle tiendra : le butoir l'interrompra au
   * besoin. C'est le changement de principe par rapport à la version
   * précédente, qui refusait de commencer une itération estimée trop coûteuse
   * et laissait alors dormir le temps restant (mesuré : deux tiers du budget
   * inutilisés à la cadence de 1000 ms).
   *
   * Une itération interrompue n'est pas perdue pour autant : ses coups racine
   * sont examinés dans l'ordre de l'itération précédente, meilleur d'abord,
   * donc le premier entièrement cherché est déjà au moins aussi bon que le
   * choix précédent.
   */
  for (uint8_t d = 2; d <= max_depth; d++) {
    const double remaining = (double)budget_ms - (now_ms() - start);
    // En deçà de ce seuil, commencer une itération ne mène nulle part : on
    // n'aurait pas même le temps de chercher son premier coup racine, et on
    // rendrait la main sans rien.
    if (remaining <= (double)budget_ms * 0.05) {
      break;
    }
    move candidate = search_best_move(cb, d);
    /*
     * On n'adopte le résultat que si l'itération a réellement produit quelque
     * chose. Si le butoir est tombé avant même le premier coup racine,
     * search_best_move() n'a rien à dire et l'on conserve le coup de la
     * profondeur précédente.
     */
    if (search_root_has_result) {
      best = candidate;
    }
    if (search_aborted) {
      break; // le temps est écoulé : inutile de tenter la profondeur suivante
    }
  }

  // La recherche redevient illimitée pour les appelants qui n'imposent pas de
  // budget.
  search_disarm_deadline();
  return best;
}

move search_best_move(chessboard cb, uint8_t depth) {
  // Construction des accumulateurs NNUE pour la position racine (une seule
  // fois). Ensuite apply_move/undo_move les maintiennent tout au long de la
  // recherche.
  nnue_refresh(cb);

  /*
   * Frontière entre la partie réellement jouée (empilée par uci.c) et l'arbre
   * de recherche qui commence ici. Appelé à chaque itération de
   * l'approfondissement itératif : comme la recherche dépile tout ce qu'elle
   * empile, l'historique de partie est intact à chaque fois.
   */
  repetition_begin_search(NESS_REP_KEY(cb));

  movelist child_nodes = generate_move(cb);

  move best = {0};                      // Coup neutre si aucun coup légal
  if (is_empty_movelist(child_nodes)) { // mat ou pat : il n'y a rien à jouer
    free_movelist(child_nodes);
    return best;
  }

  /*
   * À la racine on ne coupe jamais sur un score mémorisé : il faut rendre un
   * COUP, pas une valeur. En revanche on récupère le meilleur coup connu pour
   * cette position, typiquement celui trouvé par l'itération précédente de
   * l'approfondissement itératif. Le jouer en premier donne d'emblée un alpha
   * élevé, qui réfute les autres coups bien plus vite.
   */
  const uint64_t key = NESS_REP_KEY(cb);
  move hint = {0, 0, PAWN, EMPTY, MOVE_NORMAL};
  uint8_t has_hint = ness_tt_probe_move(key, &hint);
  child_nodes = order_moves_hint(child_nodes, cb, hint, has_hint);

  score best_value = -SCORE_INF;
  /*
   * alpha porte le score du meilleur coup racine déjà trouvé : le transmettre
   * aux coups suivants leur permet d'abandonner dès qu'ils sont réfutés, au
   * lieu d'être cherchés chacun en fenêtre pleine. Le meilleur coup reste le
   * même, seul le nombre de noeuds visités baisse.
   */
  score alpha = -SCORE_INF;
  uint8_t first = 1;
  while (!is_empty_movelist(child_nodes)) {
    move c = pop_movelist(child_nodes);
    undo_info u = apply_move(c, cb);
    // negamax rend le score du point de vue de l'adversaire : on le nie. La
    // fenêtre est niée et échangée de la même façon : notre alpha devient le
    // beta de l'adversaire. ply vaut 1 : après ce coup, on est à un demi-coup
    // de la racine.
#ifdef NESS_DISABLE_PVS
    score value =
        (score)-negamax_ply(cb, depth - 1, -SCORE_INF, (score)-alpha, 1);
#else
    // PVS a la racine : le premier coup (celui legue par l'iteration
    // precedente, joue en tete) en fenetre pleine ; les suivants sondes en
    // fenetre nulle, re-cherches seulement s'ils depassent alpha (le beta de la
    // racine est +infini, donc pas de borne haute a verifier).
    score value;
    if (first) {
      value = (score)-negamax_ply(cb, depth - 1, -SCORE_INF, (score)-alpha, 1);
    } else {
      value = (score)-negamax_ply(cb, depth - 1, (score)(-alpha - 1),
                                  (score)-alpha, 1);
      if (!search_aborted && value > alpha) {
        value =
            (score)-negamax_ply(cb, depth - 1, -SCORE_INF, (score)-alpha, 1);
      }
    }
#endif
    undo_move(c, cb, u);
    /*
     * Butoir franchi PENDANT la recherche de ce coup : sa valeur est celle d'un
     * sous-arbre tronqué, donc arbitraire. On l'écarte sans même la comparer -
     * c'est le point délicat de tout le mécanisme, car une valeur tronquée est
     * souvent très haute et deviendrait sinon le "meilleur" coup. On sort de la
     * boucle : les coups déjà entièrement cherchés avant lui, eux, restent
     * valables.
     */
    if (search_aborted) {
      break;
    }
    /*
     * "first ||" est indispensable : la comparaison est stricte, donc si tous
     * les coups ont le même score (typiquement une position perdue où chacun
     * mène au même mat forcé), best ne serait jamais affecté et l'on renverrait
     * un coup nul alors que des coups légaux existent. Le moteur cesserait tout
     * simplement de jouer.
     */
    if (first || value > best_value) {
      best_value = value;
      best = c;
      first = 0;
      if (best_value > alpha) {
        alpha = best_value;
      }
    }
  }
  free_movelist(child_nodes);

  /*
   * "first" vaut encore 1 si aucun coup racine n'a pu être cherché entièrement
   * (butoir franchi dès le premier). Il n'y a alors RIEN à léguer, et "best"
   * est le coup nul : on le signale à search_best_move_timed(), qui conservera
   * le résultat de la profondeur précédente.
   */
  search_root_has_result = (uint8_t)(!first);
  if (first) {
    return best;
  }

  // On lègue le meilleur coup de cette profondeur à la table : la profondeur
  // suivante de l'approfondissement itératif le jouera en premier. C'est ce
  // chaînage qui fait l'essentiel du gain de l'ensemble.
  // Le score, lui, n'est exact que si la profondeur a été explorée en entier :
  // interrompue, la recherche n'a vu qu'une partie des coups racine, donc
  // best_value n'est qu'une borne INFÉRIEURE (on a peut-être manqué mieux). On
  // la mémorise comme telle. Le COUP, lui, reste bon à jouer en premier : c'est
  // le meilleur parmi ceux réellement cherchés.
  ness_tt_store(key, depth, search_aborted ? TT_LOWER : TT_EXACT, best_value,
                best, 1, 0);
  return best;
}
