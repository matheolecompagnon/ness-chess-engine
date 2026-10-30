/**
 * \file uci.c
 * Ce fichier implémente le dialogue UCI décrit dans uci.h.
 *
 * \author LECOMPAGNON Mathéo
 * \date 16th July 2026
 */

// isatty() est POSIX : en -std=c11 strict il faut le demander explicitement.
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../../rules/headers/rule_iter.h"
#include "../../types/headers/movelist.h"
#include "apply_move.h"
#include "bitboard.h"
#include "negamax.h"
#include "nnue.h"
#include "repetition.h"
#include "tt.h"
#include "uci.h"
#include "zobrist.h"
#ifdef NESS_BOOK
#include "book.h" /* livre d'ouverture optionnel, compile avec make BOOK=1 */
#endif

/** Taille du tampon de lecture d'une ligne : une partie longue tient dans
 * "position ... moves ...". */
#define UCI_LINE_MAX 8192

/** Nombre maximal de mots dans une ligne de commande. */
#define UCI_TOKEN_MAX 1024

/**
 * \brief Budget de temps par défaut d'un coup, en millisecondes, quand
 * l'interface n'impose pas de "movetime".
 * \details Plafonne la recherche par approfondissement itératif : le moteur
 * répond dans ce délai même si la profondeur demandée est hors de portée. À
 * ajuster selon le confort de jeu voulu.
 */
#define UCI_DEFAULT_MOVETIME_MS 5000u

/**
 * \brief Nombre de coups supposé restant quand l'interface ne l'indique pas.
 * \details En cadence Fischer (celle des serveurs de parties), "movestogo" est
 * absent : la partie n'a pas de contrôle intermédiaire. Il faut donc estimer
 * soi-même sur combien de coups étaler la pendule. Une valeur trop petite
 * épuise la réserve en ouverture, trop grande fait jouer trop vite et laisse du
 * temps inutilisé à la fin. 30 est un compromis usuel.
 */
#define UCI_ASSUMED_MOVES_LEFT 30

/**
 * \brief Fraction maximale de la pendule engageable sur un seul coup (1/N).
 * \details Garde-fou contre une position difficile qui engloutirait la réserve
 * : quoi qu'il arrive, on ne consacre jamais plus du quart du temps restant à
 * un coup.
 */
#define UCI_MAX_CLOCK_FRACTION 4

/**
 * \brief Marge de sécurité retranchée au budget, en millisecondes.
 * \details Couvre le temps de transmission du coup à l'interface et
 * l'imprécision du butoir (la recherche ne consulte l'horloge qu'un noeud sur
 * quelques milliers). Sans cette marge, on perd des parties au drapeau alors
 * que le coup était trouvé.
 */
#define UCI_TIME_SAFETY_MS 50

/**
 * \brief Budget plancher, en millisecondes.
 * \details Quand la pendule est presque vide, on cherche quand même un minimum
 * : mieux vaut un coup faible qu'un coup nul. La profondeur 1 est de toute
 * façon quasi instantanée.
 */
#define UCI_MIN_MOVETIME_MS 10

/**
 * \brief Plafonds absolus de temps par coup, en millisecondes, selon la
 * cadence.
 * \details Aucun coup ne réfléchit plus longtemps que cela. Au-delà, le gain de
 * force est négligeable (facteur de branchement ~5 : le demi-coup suivant
 * coûterait 5x plus, les secondes intermédiaires n'affinent que le même
 * résultat), et une longue cadence ferait inutilement patienter l'adversaire.
 * Le plafond dépend de la vitesse de la partie :
 *  - parties rapides (bullet, blitz) : 3 s suffisent, et un coup plus long y
 * "gèle" l'adversaire ;
 *  - parties lentes (rapide, classique) : jusqu'à 10 s, on peut se permettre
 * d'aller plus profond.
 */
#define UCI_MAX_MOVE_MS_BLITZ 3000 /**< Plafond pour bullet et blitz. */
#define UCI_MAX_MOVE_MS_SLOW 10000 /**< Plafond pour rapide et classique. */

/**
 * \brief Frontière blitz / rapide, en millisecondes de durée estimée de partie.
 * \details Lichess classe une partie par sa durée estimée = base + 40 x
 * incrément. La limite haute du blitz est de 8 minutes. En deçà, on applique le
 * plafond blitz ; au-delà, le plafond lent. On reconstitue cette durée à partir
 * de ce que l'interface envoie (temps restant et incrément) : c'est exact en
 * début de partie, et en fin de partie un temps restant plus faible abaisse le
 * plafond, ce qui est le bon réflexe (moins de temps -> jouer plus vite).
 */
#define UCI_BLITZ_RAPID_BOUNDARY_MS 480000

/**
 * \brief square_to_index(s) convertit un nom de case UCI ("e4") en index de
 * bitboard.
 * \param s une chaîne d'au moins 2 caractères
 * \return l'index de la case (0 à 63), ou -1 si \b s n'est pas une case valide.
 */
static int square_to_index(const char *s) {
  // Cas impossible : la case n'est pas dans l'échiquier
  if (s[0] < 'a' || s[0] > 'h' || s[1] < '1' || s[1] > '8')
    return -1;
  // On calcule l'index de la case : (rangée * 8 + colonne)
  return (s[1] - '1') * 8 + (s[0] - 'a');
}

/**
 * \brief index_to_square(sq, buf) écrit dans \b buf le nom UCI de la case \b
 * sq.
 * \pre buf fait au moins 3 octets.
 */
static void index_to_square(cell sq, char *buf) {
  buf[0] = 'a' + (sq % 8);
  buf[1] = '1' + (sq / 8);
  buf[2] = '\0';
}

/**
 * \brief promotion_from_char(c) convertit la lettre de promotion UCI en \b
 * piecetype.
 * \return le type promu, ou EMPTY si \b c n'est pas une promotion connue.
 */
static piecetype promotion_from_char(char c) {
  switch (c) {
  case 'q':
    return QUEEN;
  case 'r':
    return ROOK;
  case 'b':
    return BISHOP;
  case 'n':
    return KNIGHT;
  default:
    return EMPTY;
  }
}

/**
 * \brief promotion_to_char(t) convertit un type promu en lettre UCI.
 * \return la lettre, ou 0 s'il n'y a pas de promotion.
 */
static char promotion_to_char(piecetype t) {
  switch (t) {
  case QUEEN:
    return 'q';
  case ROOK:
    return 'r';
  case BISHOP:
    return 'b';
  case KNIGHT:
    return 'n';
  default:
    return 0;
  }
}

/**
 * \brief move_to_uci(c, buf) écrit le coup \b c en notation UCI ("e2e4",
 * "e7e8q") dans \b buf.
 * \pre buf fait au moins 6 octets.
 */
static void move_to_uci(move c, char *buf) {
  index_to_square(c.from, buf);
  index_to_square(c.to, buf + 2);
  char p = promotion_to_char(c.promotion);
  if (p) {
    buf[4] = p;
    buf[5] = '\0';
  }
}

void uci_set_startpos(chessboard cb) {
  for (cell f = 0; f < 8; f++) {
    set_bit(get_piece_bitboard_ref(cb, WHITE, PAWN), 8 + f);
    set_bit(get_piece_bitboard_ref(cb, BLACK, PAWN), 48 + f);
  }
  piecetype back[8] = {ROOK, KNIGHT, BISHOP, QUEEN, KING, BISHOP, KNIGHT, ROOK};
  for (cell f = 0; f < 8; f++) {
    set_bit(get_piece_bitboard_ref(cb, WHITE, back[f]), f);
    set_bit(get_piece_bitboard_ref(cb, BLACK, back[f]), 56 + f);
  }
  set_chessboard_color(cb, WHITE);
  set_castling_right(cb, WHITE_QUEEN_SIDE);
  set_castling_right(cb, WHITE_KING_SIDE);
  set_castling_right(cb, BLACK_QUEEN_SIDE);
  set_castling_right(cb, BLACK_KING_SIDE);
  set_en_passant_target(cb, -1);
}

/**
 * \brief clear_board(cb) vide tous les bitboards de \b cb et remet son état à
 * zéro.
 */
static void clear_board(chessboard cb) {
  for (int c = 0; c < 2; c++)
    for (int t = 0; t < 6; t++)
      *get_piece_bitboard_ref(cb, (color)c, (piecetype)t) = 0;
  set_chessboard_color(cb, WHITE);
  for (int t = 0; t < 4; t++)
    remove_castling_right(cb, (castlingtype)t);
  set_en_passant_target(cb, -1);
}

/**
 * \brief piece_from_fen(c, out_color, out_type) décode une lettre de pièce FEN.
 * \details Majuscule = blanc, minuscule = noir.
 * \return 1 si \b c est une pièce connue, 0 sinon.
 */
static int piece_from_fen(char c, color *out_color, piecetype *out_type) {
  *out_color = (c >= 'a' && c <= 'z') ? BLACK : WHITE;
  switch (c) {
  case 'p':
  case 'P':
    *out_type = PAWN;
    return 1;
  case 'r':
  case 'R':
    *out_type = ROOK;
    return 1;
  case 'n':
  case 'N':
    *out_type = KNIGHT;
    return 1;
  case 'b':
  case 'B':
    *out_type = BISHOP;
    return 1;
  case 'q':
  case 'Q':
    *out_type = QUEEN;
    return 1;
  case 'k':
  case 'K':
    *out_type = KING;
    return 1;
  default:
    return 0;
  }
}

/**
 * \brief set_fen(cb, placement, stm, castling, ep) place sur \b cb la position
 * décrite par un FEN.
 * \details Les champs "demi-coups" et "numéro de coup" du FEN sont ignorés : le
 * \b chessboard ne les stocke pas. La chaîne de placement se lit de la rangée 8
 * vers la rangée 1.
 * \param cb un \b chessboard (vidé puis rempli)
 * \param placement le champ des pièces ("rnbqkbnr/pppppppp/...")
 * \param stm le champ du joueur au trait ("w" ou "b"), NULL accepté (blancs par
 * défaut)
 * \param castling le champ des roques ("KQkq" ou "-"), NULL accepté
 * \param ep le champ du en passant ("e3" ou "-"), NULL accepté
 * \return 1 si le FEN a été accepté, 0 sinon (dans ce cas \b cb est laissé
 * vide ou qu'il n'y a pas un et un seul roi de chaque côté).
 */
static int set_fen(chessboard cb, const char *placement, const char *stm,
                   const char *castling, const char *ep) {
  clear_board(cb);

  int rank = 7, file = 0;
  int white_kings = 0, black_kings = 0;
  for (const char *p = placement; *p; p++) {
    if (*p == '/') {
      if (file != 8 || rank == 0)
        return 0;
      rank--;
      file = 0;
    } else if (*p >= '1' && *p <= '8') {
      file += *p - '0';
      if (file > 8)
        return 0;
    } else {
      color pc;
      piecetype pt;
      if (!piece_from_fen(*p, &pc, &pt) || file > 7)
        return 0;

      if (pt == KING) {
        if (pc == WHITE) {
          white_kings++;
        } else {
          black_kings++;
        }
      }

      set_bit(get_piece_bitboard_ref(cb, pc, pt), (cell)(rank * 8 + file));
      file++;
    }
  }
  if (rank != 0 || file != 8 || white_kings != 1 || black_kings != 1)
    return 0;

  if (stm && stm[0] == 'b')
    set_chessboard_color(cb, BLACK);

  if (castling) {
    for (const char *p = castling; *p; p++) {
      switch (*p) {
      case 'K':
        set_castling_right(cb, WHITE_KING_SIDE);
        break;
      case 'Q':
        set_castling_right(cb, WHITE_QUEEN_SIDE);
        break;
      case 'k':
        set_castling_right(cb, BLACK_KING_SIDE);
        break;
      case 'q':
        set_castling_right(cb, BLACK_QUEEN_SIDE);
        break;
      default:
        break; // '-' et le reste : aucun droit à poser
      }
    }
  }

  if (ep && ep[0] != '-') {
    int sq = square_to_index(ep);
    if (sq >= 0)
      set_en_passant_target(cb, (int16_t)sq);
  }
  return 1;
}

/**
 * \brief find_legal_move(cb, s, out) cherche parmi les coups légaux de \b cb
 * celui noté \b s.
 * \details La notation UCI ne donne que la case de départ, la case d'arrivée et
 * l'éventuelle promotion : on identifie le coup en le retrouvant dans
 * generate_move(), ce qui renseigne du même coup sa nature (roque, en passant,
 * prise...) attendue par apply_move().
 * \param cb un \b chessboard bien initialisé
 * \param s un coup en notation UCI ("e2e4", "e7e8q")
 * \param out reçoit le coup trouvé
 * \return 1 si un coup légal correspond, 0 sinon.
 */
static int find_legal_move(chessboard cb, const char *s, move *out) {
  size_t len = strlen(s);
  if (len < 4)
    return 0;
  int from = square_to_index(s);
  int to = square_to_index(s + 2);
  if (from < 0 || to < 0)
    return 0;
  piecetype promo = (len >= 5) ? promotion_from_char(s[4]) : EMPTY;

  movelist moves = generate_move(cb);
  int found = 0;
  while (!is_empty_movelist(moves)) {
    move c = pop_movelist(moves);
    if (!found && c.from == from && c.to == to && c.promotion == promo) {
      *out = c;
      found = 1;
    }
  }
  free_movelist(moves);
  return found;
}

/**
 * \brief handle_position(cb, tok, ntok) exécute la commande "position".
 * \details Reconnaît "position startpos [moves ...]" et "position fen <6
 * champs> [moves ...]". Les coups qui suivent "moves" sont joués dans l'ordre ;
 * un coup illégal interrompt la série.
 * \param cb un \b chessboard (réécrit)
 * \param tok les mots de la ligne, tok[0] == "position"
 * \param ntok le nombre de mots
 */
static void handle_position(chessboard cb, char **tok, int ntok) {
  int i = 1;
  if (i >= ntok)
    return;

  if (strcmp(tok[i], "startpos") == 0) {
    clear_board(cb);
    uci_set_startpos(cb);
    i++;
  } else if (strcmp(tok[i], "fen") == 0) {
    // Les champs "demi-coups" et "numéro de coup" (tok[i+5], tok[i+6]) ne sont
    // pas stockés.
    if (i + 4 >= ntok)
      return;
    if (!set_fen(cb, tok[i + 1], tok[i + 2], tok[i + 3], tok[i + 4])) {
      printf("info string FEN invalide\n");
      fflush(stdout);
      return;
    }
    i += 5;
    if (i < ntok && strcmp(tok[i], "-") != 0 && strcmp(tok[i], "moves") != 0)
      i++; // demi-coups
    if (i < ntok && strcmp(tok[i], "moves") != 0)
      i++; // numéro de coup
  } else {
    return;
  }

  nnue_refresh(cb); // apply_move exige des accumulateurs à jour

  /*
   * Historique des positions pour la nulle par répétition. L'interface renvoie
   * la partie ENTIÈRE à chaque coup ("position startpos moves e2e4 e7e5 ..."),
   * on reconstitue donc l'historique du début : on repart d'un historique vide
   * et on empile chaque position traversée, la position initiale comprise. Sans
   * cela le moteur ne verrait que les répétitions internes à son arbre de
   * recherche, jamais celles de la partie réelle.
   */
  repetition_clear();
  repetition_push(zobrist_key(cb));

  if (i < ntok && strcmp(tok[i], "moves") == 0) {
    /*
     * Reconstruction ATOMIQUE de la position. On mémorise l'état de départ
     * (après startpos ou fen) et le nombre de positions empilées, pour pouvoir
     * tout défaire si un coup n'est pas reconnu. Sans cela, un seul coup non
     * appliqué laisserait cb à MOITIÉ joué, et la recherche qui suit
     * (handle_go) explorerait cette position corrompue au lieu de la vraie :
     * accumulateurs NNUE et historique désaccordés, coup rendu illégal côté
     * serveur, partie perdue. Un serveur de parties renvoie la partie entière à
     * chaque coup, donc un échec ici trahit un défaut de génération ; on
     * préfère alors conserver un état cohérent.
     */
    struct chessboard base =
        *cb; // copie de valeur : bitboards, scalaires, accumulateurs NNUE
    int pushed = 0;
    uint8_t ok = 1;
    for (i++; i < ntok; i++) {
      move c;
      if (!find_legal_move(cb, tok[i], &c)) {
        printf("info string coup non reconnu, position ignoree : %s\n", tok[i]);
        fflush(stdout);
        ok = 0;
        break;
      }
      apply_move(c, cb); // position permanente : pas d'undo_move ici
      repetition_push(
          zobrist_key(cb)); // La position atteinte entre dans l'historique
      pushed++;
    }
    if (!ok) {
      // On revient à l'état d'avant les coups : historique tronqué, échiquier
      // restauré.
      while (pushed-- > 0) {
        repetition_pop();
      }
      *cb = base;
    }
  }
}

/**
 * \brief handle_setoption(tok, ntok) exécute la commande "setoption".
 * \details Le moteur n'annonce et ne reconnaît \b aucune option : il joue
 * toujours à pleine force, bornée par le seul budget de temps. Toute option
 * reçue est donc signalée puis ignorée, le protocole n'ayant pas de réponse
 * d'erreur. Knights, par exemple, envoie un "setoption name Hash value 32"
 * jamais annoncé : l'ignorer proprement est le comportement attendu.
 * \param tok les mots de la ligne, tok[0] == "setoption"
 * \param ntok le nombre de mots
 */
static void handle_setoption(char **tok, int ntok) {
  if (ntok < 2 || strcmp(tok[1], "name") != 0)
    return;
  printf("info string option inconnue ignoree\n");
  fflush(stdout);
}

/**
 * \brief handle_go(cb, tok, ntok) exécute la commande "go" et répond
 * "bestmove".
 * \details "depth n" fixe la profondeur maximale visée (sans lui, celle du
 * niveau courant) ; "movetime t" fixe le budget de temps en millisecondes (sans
 * lui, UCI_DEFAULT_MOVETIME_MS). La recherche est un approfondissement itératif
 * borné par ce budget (search_best_move_timed) : le moteur répond toujours dans
 * un délai maîtrisé, même quand l'interface demande une profondeur hors de
 * portée - Knights, par exemple, encode sa difficulté en "go depth 16", que le
 * temps plafonne au lieu de laisser le moteur geler. Le niveau continue de
 * régler l'affaiblissement.
 * \param cb un \b chessboard bien initialisé
 * \param tok les mots de la ligne, tok[0] == "go"
 * \param ntok le nombre de mots
 */
/**
 * \internal
 * \brief compute_budget_ms(...) décide du temps à consacrer au coup courant.
 * \details Trois cas, par ordre de priorité :
 *  1. "movetime t" : l'interface impose un budget, on le respecte tel quel.
 *  2. "wtime/btime (+winc/binc, movestogo)" : c'est une VRAIE pendule, il faut
 * la répartir.
 *  3. ni l'un ni l'autre : budget par défaut (cas de "go depth n" seul, ou d'un
 * "go" nu).
 *
 * Répartition de la pendule. On alloue le temps restant divisé par le nombre de
 * coups qu'il reste à jouer, plus l'essentiel de l'incrément (qui, lui, est
 * reconduit à chaque coup et peut donc être dépensé sans entamer la réserve).
 * Quand l'interface ne dit pas combien de coups restent - c'est le cas courant
 * en cadence Fischer - on prend UCI_ASSUMED_MOVES_LEFT comme estimation de la
 * longueur restante de la partie.
 *
 * Deux garde-fous, sans lesquels on perd des parties au temps :
 *  - jamais plus de UCI_MAX_CLOCK_FRACTION de la pendule sur un seul coup,
 * sinon une position difficile en début de partie engloutirait la réserve ;
 *  - une marge de sécurité est retranchée pour couvrir la latence de
 * transmission du coup ; en dessous, on rend un budget plancher plutôt que
 * zéro, pour toujours jouer un coup légal.
 *
 * \param cb l'échiquier (pour savoir quelle pendule lire : celle du joueur au
 * trait)
 * \param movetime budget imposé en ms, ou -1
 * \param wtime,btime temps restant en ms pour chaque camp, ou -1 si absent
 * \param winc,binc incrément en ms pour chaque camp
 * \param movestogo coups restants avant le contrôle, ou -1 si absent
 * \return le budget en millisecondes.
 */
static uint32_t compute_budget_ms(chessboard cb, int movetime, int wtime,
                                  int btime, int winc, int binc,
                                  int movestogo) {
  // 1. Budget imposé par l'interface : rien à décider.
  if (movetime > 0) {
    return (uint32_t)movetime;
  }

  // 2. Pendule réelle ? On lit celle du joueur au trait.
  const int white = (get_chessboard_color(cb) == WHITE);
  const int time_left = white ? wtime : btime;
  const int inc = white ? winc : binc;
  if (time_left < 0) {
    // 3. Aucune information de temps : "go depth n" seul, ou "go" nu.
    return UCI_DEFAULT_MOVETIME_MS;
  }

  const int moves_left = (movestogo > 0) ? movestogo : UCI_ASSUMED_MOVES_LEFT;
  // L'incrément est reconduit à chaque coup : on peut le dépenser presque en
  // entier, mais on en garde une fraction pour ne pas dériver si l'estimation
  // est légèrement optimiste.
  long budget = (long)time_left / moves_left + (long)inc * 4 / 5;

  // Garde-fou : ne jamais engager plus d'une fraction de la pendule sur un seul
  // coup.
  const long cap = (long)time_left / UCI_MAX_CLOCK_FRACTION;
  if (budget > cap) {
    budget = cap;
  }

  // Plafond ABSOLU par coup, selon la vitesse de la partie. On reconstitue la
  // duree estimee facon Lichess (temps restant + 40 x increment) pour
  // distinguer blitz de rapide/classique. Au-dela du plafond, le temps
  // supplementaire ne rapporte quasiment rien (facteur de branchement ~5 : le
  // demi-coup suivant couterait 5x plus), et un coup trop long "gele"
  // inutilement l'adversaire.
  const long estimated = (long)time_left + 40L * (long)inc;
  const long move_cap = (estimated < UCI_BLITZ_RAPID_BOUNDARY_MS)
                            ? UCI_MAX_MOVE_MS_BLITZ
                            : UCI_MAX_MOVE_MS_SLOW;
  if (budget > move_cap) {
    budget = move_cap;
  }

  // Marge de sécurité : le coup doit encore être transmis avant que le drapeau
  // tombe.
  budget -= UCI_TIME_SAFETY_MS;
  if (budget < UCI_MIN_MOVETIME_MS) {
    budget = UCI_MIN_MOVETIME_MS;
  }

  return (uint32_t)budget;
}

static void handle_go(chessboard cb, char **tok, int ntok) {
  // Aucun plafond de force : on vise loin, c'est le budget de temps qui arrête
  // l'approfondissement itératif. "go depth n" reste prioritaire s'il est
  // fourni.
  int depth = SEARCH_MAX_DEPTH;
  int movetime = -1;          // budget explicite éventuel ; -1 = aucun
  int wtime = -1, btime = -1; // temps restant à la pendule, par couleur
  int winc = 0, binc = 0;     // incrément par coup, par couleur
  int movestogo = -1; // coups restants avant le prochain contrôle de temps
  for (int i = 1; i < ntok - 1; i++) {
    if (strcmp(tok[i], "depth") == 0) {
      int d = atoi(tok[i + 1]);
      if (d >= 1 && d <= 255)
        depth = d;
    } else if (strcmp(tok[i], "movetime") == 0) {
      int t = atoi(tok[i + 1]);
      if (t > 0)
        movetime = t;
    } else if (strcmp(tok[i], "wtime") == 0) {
      wtime = atoi(tok[i + 1]);
    } else if (strcmp(tok[i], "btime") == 0) {
      btime = atoi(tok[i + 1]);
    } else if (strcmp(tok[i], "winc") == 0) {
      winc = atoi(tok[i + 1]);
    } else if (strcmp(tok[i], "binc") == 0) {
      binc = atoi(tok[i + 1]);
    } else if (strcmp(tok[i], "movestogo") == 0) {
      movestogo = atoi(tok[i + 1]);
    }
  }

#ifdef NESS_BOOK
  // Livre d'ouverture (binaire autonome, make BOOK=1) : si la position est au
  // livre, on joue le coup du livre immediatement, sans recherche. Utile sous
  // Knights, qui ne fournit pas de livre lui-meme. Hors livre, on retombe sur
  // la recherche normale ci-dessous.
  {
    move book = {0};
    if (book_probe(cb, &book)) {
      char bbuf[6] = {0};
      move_to_uci(book, bbuf);
      printf("bestmove %s\n", bbuf);
      fflush(stdout);
      return;
    }
  }
#endif

  uint32_t budget_ms =
      compute_budget_ms(cb, movetime, wtime, btime, winc, binc, movestogo);
  move best = search_best_move_timed(cb, (uint8_t)depth, budget_ms);

  // search_best_move renvoie un coup nul (from == to == 0) s'il n'existe aucun
  // coup légal.
  if (best.from == best.to) {
    printf("bestmove 0000\n");
  } else {
    char buf[6] = {0};
    move_to_uci(best, buf);
    printf("bestmove %s\n", buf);
  }
  fflush(stdout);
}

/**
 * \brief print_help() affiche le mode d'emploi des commandes reconnues.
 */
static void print_help(void) {
  printf("Commandes reconnues :\n");
  printf("  uci                                  identification du moteur\n");
  printf(
      "  isready                              verifie que le moteur repond\n");
  printf(
      "  ucinewgame                           remet la position de depart\n");
  printf("  position startpos [moves e2e4 ...]   place une position\n");
  printf("  position fen <FEN> [moves ...]       place une position depuis un "
         "FEN\n");
  printf("  go [depth n]                         cherche le meilleur coup\n");
  printf("  help                                 affiche cette aide\n");
  printf("  quit                                 quitte\n");
}

/**
 * \brief print_banner() affiche l'accueil interactif.
 * \details Appelé seulement quand l'entrée standard est un terminal : sans cela
 * le moteur reste muet au lancement, ce qui donne l'impression d'un programme
 * bloque. Une interface graphique lance le binaire avec un tube en entree et ne
 * voit donc jamais ce texte.
 */
static void print_banner(void) {
  printf("Ness -- moteur d'echecs UCI. Tapez une commande puis Entree.\n");
  print_help();
  printf("Exemple : position startpos moves e2e4 / go depth 4\n\n");
}

/**
 * \brief tokenize(line, tok, max) découpe \b line en mots séparés par des
 * espaces.
 * \details \b line est modifiée en place (des '\\0' y sont insérés).
 * \return le nombre de mots trouvés.
 */
static int tokenize(char *line, char **tok, int max) {
  int n = 0;
  for (char *p = strtok(line, " \t\r\n"); p && n < max;
       p = strtok(NULL, " \t\r\n"))
    tok[n++] = p;
  return n;
}

void uci_loop(void) {
  setvbuf(stdout, NULL, _IOLBF,
          0); // l'interface attend nos réponses ligne par ligne

  int interactive = isatty(
      STDIN_FILENO); // une GUI nous parle par un tube : on reste muet pour elle
  if (interactive)
    print_banner();

  chessboard cb = create_chessboard();
  uci_set_startpos(cb);
  nnue_refresh(cb);

  char line[UCI_LINE_MAX];
  char *tok[UCI_TOKEN_MAX];

  while (fgets(line, sizeof(line), stdin)) {
    int ntok = tokenize(line, tok, UCI_TOKEN_MAX);
    if (ntok == 0)
      continue;

    if (strcmp(tok[0], "uci") == 0) {
      printf("id name Ness\n");
      printf("id author LECOMPAGNON Matheo\n");
      printf("uciok\n");
    } else if (strcmp(tok[0], "isready") == 0) {
      printf("readyok\n");
    } else if (strcmp(tok[0], "ucinewgame") == 0) {
      clear_board(cb);
      uci_set_startpos(cb);
      nnue_refresh(cb);
      // Nouvelle partie : les positions memorisees ne servent plus a rien, on
      // repart d'une table vide plutot que de garder des entrees d'une partie
      // precedente.
      tt_clear();
      // Idem pour l'historique des positions : celles de la partie precedente
      // ne doivent pas faire croire a une repetition dans la nouvelle.
      repetition_clear();
    } else if (strcmp(tok[0], "setoption") == 0) {
      handle_setoption(tok, ntok);
    } else if (strcmp(tok[0], "position") == 0) {
      handle_position(cb, tok, ntok);
    } else if (strcmp(tok[0], "go") == 0) {
      handle_go(cb, tok, ntok);
    } else if (strcmp(tok[0], "help") == 0) {
      print_help();
    } else if (strcmp(tok[0], "quit") == 0) {
      break;
    }
    // Toute autre commande est ignorée, comme le prévoit le protocole.
    fflush(stdout);
  }

  free_chessboard(cb);
}
