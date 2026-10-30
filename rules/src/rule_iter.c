/**
 * \file rule_iter.c
 * Ce fichier implémente les fonctions permettant de gérer les règles de
 * l'échiquier
 * - generate_knight_move(cb) donne la liste des coups possibles du cavalier du
 * joueur actuel.
 * - generate_rook_move(cb) donne la liste des coups possibles de la tour du
 * joueur actuel.
 * - generate_bishop_move(cb) donne la liste des coups possibles du fou du
 * joueur actuel.
 * - generate_queen_move(cb) donne la liste des coups possibles de la reine du
 * joueur actuel.
 * - generate_king_move_aux(cb) donne la liste des coups possibles du roi du
 * joueur actuel sans roque.
 * - generate_pawn_move_aux(cb) donne la liste des coups possibles du pion du
 * joueur actuel sans en passant.
 * - generate_king_move(cb) donne la liste des coups possibles du roi du joueur
 * actuel.
 * - generate_pawn_move(cb) donne la liste des coups possibles du pion du joueur
 * actuel.
 * - generate_move(cb) donne la liste des coups possible du joueur actuel.
 * \details ces fonctions n'implémentent que les coups du joueur. Ils
 * n'implémentent pas les répercussions de chaque coup. Par exemple, il ne passe
 * pas le pion en reine lors de la promotion. Ou encore, il ne prend également
 * aucune pièce lors des prises. Le but est d'implémenter les coups possibles de
 * chaque pièce et de les proposer dans une liste de coups.
 *
 * \author LECOMPAGNON Mathéo
 * \date 8th June 2026
 */

#include <stdint.h>
#include <stdlib.h>

#include "../headers/rule_iter.h"

/**
 * \internal
 * \brief is_available_index(index,self_index,cb) vérifie la condition "l'index
 * est disponible dans l'échiquier et n'atteint pas les pièces actives"
 * \pre index n'est pas l'index de la pièce jouée.
 * \param index un entier.
 * \param cb un chessboard.
 * \return Le test "L'index \b index est disponible dans l'échiquier : la case
 * n'est pas prise par le joueur et l'index correspond à une case de
 * l'échiquier".
 *
 */
uint8_t is_available_index(int8_t index, chessboard cb) {
  // On vérifie si l'index n'est pas hors de l'échiquier
  if (index < 0 || index > 63) {
    return 0;
  }
  /*
   * On récupère le bitboard correspondant à toutes les pièces de l'équipe qui
   * joue Si l'index est déjà occupé par l'équipe qui joue alors on ne peut y
   * bouger une pièce
   */
  uint8_t res =
      1 - get_bit(get_color_bitboard(cb, get_chessboard_color(cb)), index);
  return res;
}

/**
 * \internal
 * \brief generate_knight_move(cb, m) ajoute la liste des coups possibles du
 * cavalier dans \b m.
 * \pre moves bien initialisé.
 * \param cb un \b chessboard.
 * \param m un \b movelist.
 *
 */
void generate_knight_move(chessboard cb, movelist m) {
  int8_t knight_moves[8] = {-17, -15, -10, -6, 6,
                            10,  15,  17}; // Mouvements possibles du cavalier
  bitboard knight_bitboard = get_piece_bitboard(
      cb, get_chessboard_color(cb),
      KNIGHT); // On récupère le bitboard des cavaliers actifs
  /*
   * On parcourt les cavaliers jouables un par un et on ajoute leurs coups à
   * moves. pop_first_cell rend la case du cavalier de plus petit index et le
   * retire du bitboard : on ne visite que les cases occupées.
   */
  // Parcours du bitboard
  while (knight_bitboard) {
    cell i = pop_first_cell(&knight_bitboard);
    // Parcours de tous les coups possibles
    for (uint8_t i_move = 0; i_move < 8; i_move++) {
      // Si le coup est possible : dans l'échiquier, dehors des cases prises par
      // le joueur, et coup légal
      if (is_available_index(i + knight_moves[i_move], cb) &&
          (abs((i + knight_moves[i_move]) % 8 - i % 8) <=
           2) // La différence avec la colonne suivante est réaliste.
      ) {
        // Ajout dans la liste de coups du coup possible.
        move c = {.from = i,                      // Position de départ
                  .to = i + knight_moves[i_move], // Coup possible
                  .type = KNIGHT,
                  .promotion = EMPTY,
                  // Prise si la case d'arrivée est occupée par l'adversaire
                  .kind = get_bit(get_color_bitboard(
                                      cb, 1 - get_chessboard_color(cb)),
                                  i + knight_moves[i_move])
                              ? MOVE_CAPTURE
                              : MOVE_NORMAL};
        push_movelist(c, m);
      }
    }
  }
  return;
}

/**
 * \internal
 * \brief generate_rook_move_aux(cb, p, m) ajoute la liste des coups possibles
 * de la pièce choisie suivant les coups de la tour dans \b m.
 * \details Cette fonction est utilisée dans generate_rook_move et dans
 * generate_queen_move
 * \pre moves bien initialisé.
 * \param cb un \b chessboard.
 * \param p le \b piecetype de la pièce dont on génère les coups (ROOK ou
 * QUEEN).
 * \param m un \b movelist.
 *
 */
void generate_rook_move_aux(chessboard cb, piecetype p, movelist m) {
  bitboard active_bitboard = get_piece_bitboard(
      cb, get_chessboard_color(cb),
      p); // On récupère le bitboard des pièces actives souhaitées
  int8_t rook_moves[4] = {
      -8, -1, 1, 8}; // Coefficient multiplicateur : haut, gauche, droite, bas
  // Parcours des seules cases occupées par les tours du joueur actif
  while (active_bitboard) {
    cell rook_index = pop_first_cell(
        &active_bitboard); // Case de la piece traitée, retirée du bitboard
    // On parcours tous les côtés possibles : haut gauche droite bas
    for (uint8_t k = 0; k < 4; k++) {
      int8_t to_index =
          rook_index + rook_moves[k]; // int8 pour éviter les underflow lorsque
                                      // to_index doit être négatif
      /*
       * Tant que l'index ne sort pas de l'échiquier, qu'il n'atteint pas une
       * case déjà prise par le joueur actif : is_available_index et qu'il n'a
       * pas déjà atteint (pendant l'étape k) une case prise par le joueur
       * adverse : not opponent_reached
       */
      uint8_t opponent_reached = 0;
      while (is_available_index(to_index, cb) && !opponent_reached &&
             !((rook_moves[k] == 1 || rook_moves[k] == -1) &&
               (to_index / 8 != rook_index / 8))) {
        // opponent_reached : vaut 1 si l'index est sur une case adverse (donc
        // une prise)
        opponent_reached = get_bit(
            get_color_bitboard(
                cb, 1 - get_chessboard_color(cb)), // Toutes les cases adverses
            to_index);

        // Ajout dans la liste de coups du coup possible.
        move c = {.from = rook_index, // Position de départ
                  .to = to_index,     // Coup possible
                  .type = p,          // Pièce jouée
                  .promotion = EMPTY,
                  .kind = opponent_reached ? MOVE_CAPTURE : MOVE_NORMAL};
        push_movelist(c, m);

        to_index += rook_moves[k];
      }
    }
  }
}

/**
 * \internal
 * \brief generate_rook_move(cb, m) ajoute la liste des coups possibles de la
 * tour dans m.
 * \pre moves bien initialisé.
 * \param cb un \b chessboard.
 * \param m un \b movelist.
 *
 */
void generate_rook_move(chessboard cb, movelist m) {
  // On génère tous les coups
  generate_rook_move_aux(cb, ROOK, m);
}

/**
 * \internal
 * \brief generate_bishop_move_aux(cb, p, m) ajoute dans \b m les coups de la
 * pièce \b p suivant les diagonales du fou.
 * \pre moves bien initialisé.
 * \param cb un \b chessboard.
 * \param p le \b piecetype de la pièce dont on génère les coups (BISHOP ou
 * QUEEN).
 * \param m un \b movelist.
 *
 */
void generate_bishop_move_aux(chessboard cb, piecetype p, movelist m) {
  bitboard active_bitboard = get_piece_bitboard(
      cb, get_chessboard_color(cb),
      p); // On récupère le bitboard des pièces actives de type voulu.
  int8_t bishop_moves[4] = {
      -9, -7, 7, 9}; // Coefficient multiplicateur : haut, gauche, droite, bas
  // Parcours des seules cases occupées par les pièces de type voulu du joueur
  // actif
  while (active_bitboard) {
    cell bishop_index = pop_first_cell(
        &active_bitboard); // Case de la piece traitée, retirée du bitboard
    // On parcours tous les côtés possibles : haut gauche droite bas
    for (uint8_t k = 0; k < 4; k++) {
      int8_t to_index =
          bishop_index + bishop_moves[k]; // int8 pour éviter les underflow
                                          // lorsque to_index doit être négatif
      /*
       * Tant que l'index ne sort pas de l'échiquier, qu'il n'atteint pas une
       * case déjà prise par le joueur actif : is_available_index et qu'il n'a
       * pas déjà atteint (pendant l'étape k) une case prise par le joueur
       * adverse : not opponent_reached
       */
      uint8_t opponent_reached = 0; // Variable décrivant si la pièce a atteint
                                    // un adversaire dans cette direction
      while (is_available_index(to_index, cb) && !opponent_reached &&
             abs(to_index % 8 - (to_index - bishop_moves[k]) % 8) ==
                 1 // différence de colonne avec la case précédente est 1.
      ) {
        // opponent_reached : vaut 1 si l'index est sur une case adverse (donc
        // une prise)
        opponent_reached = get_bit(
            get_color_bitboard(
                cb, 1 - get_chessboard_color(cb)), // Toutes les cases adverses
            to_index);
        // Ajout dans la liste de coups du coup possible.
        move c = {.from = bishop_index, // Position de départ
                  .to = to_index,       // Coup possible
                  .type = p,            // Type de la pièce jouée.
                  .promotion = EMPTY,
                  .kind = opponent_reached ? MOVE_CAPTURE : MOVE_NORMAL};
        push_movelist(c, m);
        to_index += bishop_moves[k];
      }
    }
  }
}
/**
 * \internal
 * \brief generate_bishop_move(cb, m) ajoute la liste des coups possibles du fou
 * dans \b m.
 * \pre moves bien initialisé.
 * \param cb un \b chessboard.
 * \param m un \b movelist.
 *
 */
void generate_bishop_move(chessboard cb, movelist m) {
  // On génère tous les coups
  generate_bishop_move_aux(cb, BISHOP, m);
}
/**
 * \internal
 * \brief generate_king_move_aux(cb, p, m) ajoute la liste des coups possibles
 * du roi dans \b m sans la règle du roque.
 * \pre moves bien initialisé.
 * \param cb un \b chessboard.
 * \param p le \b piecetype de la pièce dont on génère les coups (KING).
 * \param m un \b movelist.
 *
 */
void generate_king_move_aux(chessboard cb, piecetype p, movelist m) {
  // On récupère le bitboard des pièces actives de type souhaité
  bitboard active_bitboard =
      get_piece_bitboard(cb, get_chessboard_color(cb), p);
  // Mouvements possible du roi à prendre en compte pour les mouvements
  // verticaux et horizontaux
  int8_t king_move[3] = {-1, 0, 1};

  /*
   * On parcourt les pièces jouables de type souhaité une par une (il peut y
   * avoir deux reines) et on ajoute tous leurs coups possibles à moves.
   */
  // Parcours du bitboard
  while (active_bitboard) {
    cell k = pop_first_cell(
        &active_bitboard); // Case de la piece traitée, retirée du bitboard
    // Déplacement vertical
    for (int i = 0; i < 3; i++) {
      // Déplacement horizontal
      for (int j = 0; j < 3; j++) {
        int8_t to_index =
            k + 8 * king_move[i] + king_move[j]; // On récupère le coup à tester
        // Si le coup est possible : dans l'échiquier, dehors des cases prises
        // par le joueur, et coup légal
        if (
            // Pas un coup sur place
            (to_index != k)
            // Test de l'index et n'est pas une case déjà occupée par le joueur
            && is_available_index(to_index, cb)
            // Test du mouvement horizontal : jamais plus d'une colonne de
            // différence
            && (king_move[j] == 0 || abs(to_index % 8 - (uint8_t)k % 8) == 1)
            // Pas besoin de tester le mouvement vertical : position >= 0 et
            // position < 64 déjà vérifié
        ) {
          // Ajout dans la liste de coups du coup possible.
          move c = {.from = k,      // Position de départ
                    .to = to_index, // Coup possible
                    .type = p,
                    .promotion = EMPTY,
                    // Prise si la case d'arrivée est occupée par l'adversaire
                    .kind = get_bit(get_color_bitboard(
                                        cb, 1 - get_chessboard_color(cb)),
                                    to_index)
                                ? MOVE_CAPTURE
                                : MOVE_NORMAL};
          push_movelist(c, m);
        }
      }
    }
  }
  return;
}
/**
 * \internal
 * \brief generate_queen_move(cb, m) ajoute la liste des coups possibles de la
 * reine dans \b m.
 * \pre moves bien initialisé.
 * \param cb un \b chessboard.
 * \param m un \b movelist.
 *
 */
void generate_queen_move(chessboard cb, movelist m) {
  // On génère tous les coups droits (inclut les cases adjacentes orthogonales)
  generate_rook_move_aux(cb, QUEEN, m);
  // On génère tous les coups diagonaux (inclut les cases adjacentes diagonales)
  generate_bishop_move_aux(cb, QUEEN, m);
}
/**
 * \internal
 * \brief generate_promotion_pawn(cb, m, from, to) ajoute à la liste des coups
 * possibles toutes les promotions dans \b m.
 * \pre moves est bien initialisé.
 * \pre to est sur la ligne du tableau
 * \param cb un chessboard
 * \param m un \b movelist.
 * \param from un \b cell.
 * \param to un \b cell.
 * \return la condition "les promotions ont bien été générées."
 */
uint8_t generate_promotion_pawn(chessboard cb, movelist m, cell from, cell to) {
  if ((get_chessboard_color(cb) == WHITE &&
       to / 8 == 7) // Le pion blanc est arrivé à la ligne 8
      || (get_chessboard_color(cb) == BLACK &&
          to / 8 == 0) // Le pion noir est arrivé à la ligne 1
  ) {
    // Une promotion peut se faire par simple poussée ou en prenant une pièce
    // adverse
    uint8_t cap =
        get_bit(get_color_bitboard(cb, 1 - get_chessboard_color(cb)), to);
    for (piecetype type = ROOK; type <= QUEEN;
         type++) { // Toutes les pièces sauf le roi et le pion
      move c = {.from = from,
                .to = to,
                .type = PAWN,
                .promotion = type,
                .kind = cap ? MOVE_CAPTURE : MOVE_NORMAL};
      push_movelist(c, m);
    }
    return 1;
  } else {
    return 0;
  }
}
/**
 * \internal
 * \brief generate_pawn_move_aux(cb, m) ajoute la liste des coups possibles du
 * pion dans \b m sans la règle en passant.
 * \pre moves bien initialisé.
 * \param cb un chessboard.
 * \param m un \b movelist.
 */
void generate_pawn_move_aux(chessboard cb, movelist m) {
  bitboard pawn_bitboard =
      get_piece_bitboard(cb, get_chessboard_color(cb),
                         PAWN); // On récupère le bitboard des pions actifs
  // Parcours des seules cases occupées par un pion
  while (pawn_bitboard) {
    cell k = pop_first_cell(
        &pawn_bitboard); // Case du pion traité, retirée du bitboard
    /*
     * Il y a une distinction entre le pion blanc et noir:
     * - Un pion blanc se déplace vers le haut (-8) et sa ligne initiale est
     * d'indice 1
     * - Un pion noir se déplace vers le bas (+8) et sa ligne initiale est
     * d'indice 6
     */

    // ============
    // Avance de 1
    // ============

    int8_t pawn_move = (get_chessboard_color(cb) == WHITE)
                           ? 8
                           : -8; // Direction du pion qui dépend de la couleur
    bitboard opponent = get_color_bitboard(cb, 1 - get_chessboard_color(cb));
    // Si le coup est valide, on l'ajoute
    if (is_available_index(k + pawn_move,
                           cb) // L'index est dans l'échiquier et la case non
                               // occupée par le joueur
        && (get_bit(opponent, k + pawn_move) ==
            0) // La place n'est pas prise par une pièce opposée
    ) {

      if (!generate_promotion_pawn(
              cb, m, k,
              k + pawn_move)) { // Si le pion n'arrive pas sur la ligne finale
        move c = {.from = k,
                  .to = k + pawn_move,
                  .type = PAWN,
                  .promotion = EMPTY,
                  .kind = MOVE_NORMAL}; // On ajoute le coup "avance de 1"
        push_movelist(c, m);
      }

      // Etat actuel : la case devant le pion est libre
      // Si on est en ligne de départ : Peut on avancer de 2 ?
      // ============
      // Avance de 2
      // ============

      if (k / 8 == 6 || k / 8 == 1) { // En ligne 7 ou en ligne 2
        if (is_available_index(k + 2 * pawn_move,
                               cb) // L'index est dans l'échiquier et la case
                                   // non occupée par le joueur
            && get_bit(opponent, k + pawn_move * 2) ==
                   0) // Pas prise par une pièce opposée
        {
          if (!generate_promotion_pawn(
                  cb, m, k,
                  k + pawn_move *
                          2)) { // Si le pion n'arrive pas sur la ligne finale
            move c = {.from = k,
                      .to = k + pawn_move * 2,
                      .type = PAWN,
                      .promotion = EMPTY,
                      .kind =
                          MOVE_DOUBLE_PUSH}; // On ajoute le coup "avance de 2"
            push_movelist(c, m);
          }
        }
      }
    }

    // ==================
    // Prise en diagonale
    // ==================
    /* Prendre une pièce en diagonale :
     * - Le pion peut prendre une pièce ennemie en diagonale devant lui
     * Attention : Débordements lorsque le pion est placé sur les colonnes
     * extrêmes de l'échiquier : A et H
     */
    int8_t capture_moves[2] = {pawn_move - 1,
                               pawn_move + 1}; // Différents mouvements
    for (uint8_t i = 0; i < 2; i++) {
      if (is_available_index(k + capture_moves[i], cb) // La case est disponible
          && get_bit(opponent,
                     k + capture_moves[i]) // La case est prise par l'adversaire
          && abs((k + capture_moves[i]) % 8 - (int8_t)(k % 8)) ==
                 1) // La différence avec la case attaquée est 1.
      {

        if (!generate_promotion_pawn(
                cb, m, k, k + capture_moves[i])) { // Si le pion n'arrive pas
                                                   // sur la ligne finale
          move c = {.from = k,
                    .to = k + capture_moves[i],
                    .type = PAWN,
                    .promotion = EMPTY,
                    .kind = MOVE_CAPTURE};
          push_movelist(c, m);
        }
      }
    }
  }
}

/**
 * \internal
 * \brief is_cell_attacked(cb, c, by) vérifie si la case \b square est attaquée
 * par la couleur \b by.
 * \param cb     un \b chessboard.
 * \param c la case à tester (0-63).
 * \param by     la couleur de l'attaquant.
 * \return 1 si la case est attaquée, 0 sinon.
 * \warning ne prend pas en compte la règle du en passant, car is_cell_attacked
 * n'est utilisée que pour le roque
 */
uint8_t is_cell_attacked(chessboard cb, cell c, color by) {

  // Cavalier

  int8_t knight_moves[8] = {-17, -15, -10, -6, 6, 10, 15, 17};
  bitboard knights =
      get_piece_bitboard(cb, by, KNIGHT); // Tableau des cavaliers attaquants
  for (uint8_t i = 0; i < 8; i++) {
    int8_t from =
        (int8_t)c +
        knight_moves[i];       // D'où le cavalier pourrait attaquer la case c.
    if (from >= 0 && from < 64 // La case fait parti du bitboard
        && get_bit(knights, from) // Il y a un cavalier sur la case
        && abs((int8_t)(c % 8) - (int8_t)(from % 8)) <=
               2) // Evite les débordements sur les bords
    {
      return 1;
    }
  }

  // Roi

  int8_t king_move[3] = {-1, 0, 1};
  bitboard king = get_piece_bitboard(cb, by, KING); // Tableau du roi attaquant
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      if (i != 1 || j != 1) { // si j==1 et i==1, c'est la case du roi du
                              // joueur, ce qu'on ne veut pas tester.
        int8_t from = (int8_t)c + 8 * king_move[i] +
                      king_move[j]; // Case du roi potentiel
        if (from >= 0 && from < 64  // La case est sur l'échiquier
            && get_bit(king, from)  // Il y a bien un roi dans la case
            &&
            (king_move[j] == 0 || abs((int8_t)(c % 8) - (int8_t)(from % 8)) ==
                                      1)) // Evite les débordements de colonne
        {
          return 1;
        }
      }
    }
  }

  // Pion
  /*
   * On regarde d'où un pion adverse pourrait attaquer la case.
   * Un pion blanc attaque vers le haut (+8), donc il attaque depuis square-8 +1
   * ou -1. Un pion noir attaque vers le bas (-8), donc il attaque depuis
   * square+8 +1 ou -1.
   */
  bitboard pawns = get_piece_bitboard(cb, by, PAWN);
  int8_t pawn_dir = (by == WHITE) ? -8 : 8; // direction inverse
  int8_t pawn_sources[2] = {(int8_t)c + pawn_dir - 1,
                            (int8_t)c + pawn_dir + 1}; // D'où vient l'attaque
  for (uint8_t i = 0; i < 2; i++) {
    int8_t from =
        pawn_sources[i]; // Position de la provenance de l'attaque à tester
    if (from >= 0 && from < 64  // Dans l'échiquier
        && get_bit(pawns, from) // est bien un pion
        && abs((int8_t)(c % 8) - (int8_t)(from % 8)) ==
               1) { // Evite les débordements aux colonnes extrêmes
      return 1;
    }
  }

  // Tour / Dame
  int8_t rook_moves[4] = {-8, -1, 1, 8};
  bitboard rook_queen =
      get_piece_bitboard(cb, by, ROOK) |
      get_piece_bitboard(cb, by, QUEEN); // Cases des tours ou reines attaquants
  bitboard occupied = get_color_bitboard(cb, WHITE) |
                      get_color_bitboard(cb, BLACK); // Cases actives
  for (uint8_t k = 0; k < 4; k++) {
    int8_t from = (int8_t)c + rook_moves[k];
    uint8_t blocked = 0;
    while (from >= 0 && from < 64 // est dans l'échiquier
           && !((rook_moves[k] == 1 || rook_moves[k] == -1) &&
                (from / 8 != c / 8)) // Evite des débordements
           && !blocked) {
      if (get_bit(rook_queen, from))
        return 1;
      blocked = get_bit(occupied, from); /* S'il y a une pièce bloquante,
      elle n'est pas une tour ou une reine donc on arrête la boucle */
      from += rook_moves[k];
    }
  }

  // Fou / Dame
  int8_t bishop_moves[4] = {-9, -7, 7, 9};
  bitboard bishop_queen =
      get_piece_bitboard(cb, by, BISHOP) |
      get_piece_bitboard(cb, by, QUEEN); // Cases des fous ou reines attaquants
  for (uint8_t k = 0; k < 4; k++) {
    int8_t from = (int8_t)c + bishop_moves[k]; // Case attaquant
    uint8_t blocked = 0;
    while (from >= 0 && from < 64 // Est dans l'échiquier
           && abs(from % 8 - (from - bishop_moves[k]) % 8) ==
                  1 // Evite les débordements
           && !blocked) {
      if (get_bit(bishop_queen, from))
        return 1;
      blocked = get_bit(occupied, from); /* S'il y a une pièce qui bloque, elle
      n'est pas un fou ou une reine Donc on arrête la boucle */
      from += bishop_moves[k];
    }
  }

  return 0;
}

/**
 * \internal
 * \brief generate_king_move(cb, m) ajoute la liste des coups possibles du roi
 * dans \b m.
 * \pre moves bien initialisé.
 * \param cb un chessboard.
 * \param m un \b movelist.
 *
 */
void generate_king_move(chessboard cb, movelist m) {
  generate_king_move_aux(cb, KING, m);
  // règle du roque

  bitboard occupied =
      get_color_bitboard(cb, WHITE) | get_color_bitboard(cb, BLACK);

  if (get_chessboard_color(cb) == WHITE) { // Blancs

    if (get_castling_right(cb, WHITE_KING_SIDE) &&
        get_bit(get_piece_bitboard(cb, WHITE, KING),
                4) // le roi est bien sur sa case initiale e1
        && get_bit(get_piece_bitboard(cb, WHITE, ROOK),
                   7) // la tour est bien présente en h1
        && !get_bit(occupied, 5) && !get_bit(occupied, 6) &&
        !is_cell_attacked(cb, 4, BLACK) && !is_cell_attacked(cb, 5, BLACK) &&
        !is_cell_attacked(cb, 6, BLACK)) {
      move c = {
          .from = 4,
          .to = 6,
          .type = KING,
          .promotion = EMPTY,
          .kind =
              MOVE_CASTLE_KING}; // On ajoute le coup "avance de 2" pour le roi
      push_movelist(c, m);
      // Attention : ne pas oublier de bouger la tour dans apply_move
    }
    if (get_castling_right(cb, WHITE_QUEEN_SIDE) &&
        get_bit(get_piece_bitboard(cb, WHITE, KING),
                4) // le roi est bien sur sa case initiale e1
        && get_bit(get_piece_bitboard(cb, WHITE, ROOK),
                   0) // la tour est bien présente en a1
        && !get_bit(occupied, 1) && !get_bit(occupied, 2) &&
        !get_bit(occupied, 3) && !is_cell_attacked(cb, 2, BLACK) &&
        !is_cell_attacked(cb, 3, BLACK) && !is_cell_attacked(cb, 4, BLACK)) {
      move c = {
          .from = 4,
          .to = 2,
          .type = KING,
          .promotion = EMPTY,
          .kind =
              MOVE_CASTLE_QUEEN}; // On ajoute le coup "avance de 2" pour le roi
      push_movelist(c, m);
      // Attention : ne pas oublier de bouger la tour dans apply_move
    }

  } else { // Noirs

    if (get_castling_right(cb, BLACK_KING_SIDE) &&
        get_bit(get_piece_bitboard(cb, BLACK, KING),
                60) // le roi est bien sur sa case initiale e8
        && get_bit(get_piece_bitboard(cb, BLACK, ROOK),
                   63) // la tour est bien présente en h8
        && !get_bit(occupied, 61) && !get_bit(occupied, 62) &&
        !is_cell_attacked(cb, 60, WHITE) && !is_cell_attacked(cb, 61, WHITE) &&
        !is_cell_attacked(cb, 62, WHITE)) {
      move c = {
          .from = 60,
          .to = 62,
          .type = KING,
          .promotion = EMPTY,
          .kind =
              MOVE_CASTLE_KING}; // On ajoute le coup "avance de 2" pour le roi
      push_movelist(c, m);
      // Attention : ne pas oublier de bouger la tour dans apply_move
    }
    if (get_castling_right(cb, BLACK_QUEEN_SIDE) &&
        get_bit(get_piece_bitboard(cb, BLACK, KING),
                60) // le roi est bien sur sa case initiale e8
        && get_bit(get_piece_bitboard(cb, BLACK, ROOK),
                   56) // la tour est bien présente en a8
        && !get_bit(occupied, 58) && !get_bit(occupied, 59) &&
        !get_bit(occupied, 57) && !is_cell_attacked(cb, 58, WHITE) &&
        !is_cell_attacked(cb, 59, WHITE) && !is_cell_attacked(cb, 60, WHITE)) {
      move c = {
          .from = 60,
          .to = 58,
          .type = KING,
          .promotion = EMPTY,
          .kind =
              MOVE_CASTLE_QUEEN}; // On ajoute le coup "avance de 2" pour le roi
      push_movelist(c, m);
      // Attention : ne pas oublier de bouger la tour dans apply_move
    }
  }
}
/**
 * \internal
 * \brief generate_pawn_move(cb, m) ajoute la liste des coups possibles du pion
 * dans \b m.
 * \pre moves bien initialisé.
 * \param cb un chessboard.
 * \param m un \b movelist.
 *
 */
void generate_pawn_move(chessboard cb, movelist m) {
  generate_pawn_move_aux(cb, m);
  // règle du en passant

  /**
   * Attention : la variable de la règle du en passant n'est jamais modifiée
   * directement dans la génération de la liste des coups :
   * TODO modification de la variable en passant à implémenter plus tard (futur
   * apply_move)
   */

  if (get_en_passant_target(cb) != -1) {
    // La règle du en passant est exploitable
    bitboard pawns = get_piece_bitboard(cb, get_chessboard_color(cb), PAWN);
    int8_t target = get_en_passant_target(cb);
    int8_t opponent_move = (1 - get_chessboard_color(cb) == WHITE)
                               ? 8
                               : -8; /* Si l'adversaire est en blanc, il va vers
                                      * le bas de l'échiquier
                                      */
    /* Les deux cases d'où un pion pourrait capturer en passant */
    int8_t sources[2] = {target + opponent_move - 1,
                         target + opponent_move + 1};
    for (uint8_t i = 0; i < 2; i++) {
      int8_t from = sources[i];
      if (from >= 0 && from < 64  // L'emplacement dans l'échiquier du pion
                                  // appliquant la règle est correct
          && get_bit(pawns, from) // C'est bien un pion
          &&
          is_available_index(
              target, cb) // La case cible n'est pas occupée par le joueur actif
          && abs(target % 8 - from % 8) ==
                 1) // Evite les problèmes de débordement de colonne
      {
        move c = {.from = from,
                  .to = target,
                  .type = PAWN,
                  .promotion = EMPTY,
                  .kind = MOVE_EN_PASSANT};
        push_movelist(c, m);
      }
    }
  }
}

/**
 * \internal
 * \brief find_king_cell(cb, c) retourne la case du roi de couleur \b c, ou 64
 * si absent.
 * \param cb un \b chessboard.
 * \param c une \b color.
 * \return la case du roi, ou 64 s'il n'y a pas de roi.
 */
static cell find_king_cell(chessboard cb, color c) {
  // first_cell rend 64 si le bitboard est vide : meme convention qu'auparavant.
  return first_cell(get_piece_bitboard(cb, c, KING));
}

uint8_t is_king_in_check(chessboard cb, color c) {
  cell k = find_king_cell(cb, c);
  if (k == 64)
    return 0; // pas de roi : rien à mettre en échec
  // is_cell_attacked ignore la prise en passant, sans conséquence ici : un roi
  // ne se prend pas ainsi.
  return is_cell_attacked(cb, k, (color)(1 - c));
}

/**
 * \internal
 * \brief simulate_move_into(n, cb, c) écrit dans \b n la position \b cb où \b c
 * a été joué.
 * \details Application minimale destinée au seul test de légalité : déplacement
 * de la pièce, prise normale, prise en passant, déplacement de la tour du roque
 * et promotion. Ne met à jour ni les droits de roque, ni la cible du en
 * passant, ni les accumulateurs NNUE - rien de tout cela n'étant lu par
 * is_cell_attacked(). Ce n'est donc pas un apply_move() complet.
 *
 * \b n est fourni par l'appelant, en pratique sur la \b pile : cette fonction
 * n'alloue rien. C'est un choix mesuré (D11) : la version
 * d'origine faisait un malloc par coup pseudo-légal, et la variante par
 * apply_move()/undo_move() s'est révélée plus lente encore, car défaire un coup
 * revient à refaire tout le travail scalaire de l'aller.
 * \param n la destination, non initialisée en entrée.
 * \param cb un \b chessboard.
 * \param c un \b move.
 */
static void simulate_move_into(struct chessboard *n, chessboard cb, move c) {
  *n = *cb; // copie complète : bitboards et scalaires
  color me = get_chessboard_color(cb);
  color opp = 1 - me;

  // 1. On retire la pièce jouée de sa case de départ
  clear_bit(&n->pieceboard[me][c.type], c.from);
  // 2. On retire une éventuelle pièce adverse capturée sur la case d'arrivée
  for (piecetype t = PAWN; t <= KING; t++) {
    clear_bit(&n->pieceboard[opp][t], c.to);
  }
  // 3. Prise en passant : le pion capturé n'est pas sur la case d'arrivée
  if (c.kind == MOVE_EN_PASSANT) {
    cell captured = (me == WHITE) ? c.to - 8 : c.to + 8;
    clear_bit(&n->pieceboard[opp][PAWN], captured);
  }
  // 4. Roque : on déplace aussi la tour
  if (c.kind == MOVE_CASTLE_KING || c.kind == MOVE_CASTLE_QUEEN) {
    cell rook_from, rook_to;
    if (c.to == 6) {
      rook_from = 7;
      rook_to = 5;
    } // petit roque blanc
    else if (c.to == 2) {
      rook_from = 0;
      rook_to = 3;
    } // grand roque blanc
    else if (c.to == 62) {
      rook_from = 63;
      rook_to = 61;
    } // petit roque noir
    else {
      rook_from = 56;
      rook_to = 59;
    } // grand roque noir
    clear_bit(&n->pieceboard[me][ROOK], rook_from);
    set_bit(&n->pieceboard[me][ROOK], rook_to);
  }
  // 5. On place la pièce jouée sur la case d'arrivée (promotion prise en
  // compte)
  piecetype placed = (c.promotion != EMPTY) ? c.promotion : c.type;
  set_bit(&n->pieceboard[me][placed], c.to);
}

/**
 * \internal
 * \brief is_move_legal(cb, c) vérifie que le coup \b c ne laisse pas le roi du
 * joueur au trait en échec.
 * \details Couvre le roi qui se déplacerait sur une case attaquée ainsi que les
 * pièces clouées. La position d'essai vit sur la pile : ce test est le chemin
 * le plus chaud du moteur (un appel par coup pseudo-légal, à chaque noeud de
 * l'arbre), il ne doit donc rien allouer.
 * \param cb un \b chessboard.
 * \param c un \b move.
 * \return 1 si le coup est légal, 0 sinon.
 */
static uint8_t is_move_legal(chessboard cb, move c) {
  color me = get_chessboard_color(cb);
  struct chessboard n; // position d'essai, sur la pile : aucune allocation
  simulate_move_into(&n, cb, c);
  cell king = find_king_cell(&n, me);
  // Sans roi identifiable, on ne peut pas conclure : on garde le coup
  return (king == 64) ? 1 : !is_cell_attacked(&n, king, 1 - me);
}

movelist generate_move(chessboard cb) {
  // On génère d'abord tous les coups pseudo-légaux
  movelist pseudo = create_movelist();
  generate_knight_move(cb, pseudo);
  generate_rook_move(cb, pseudo);
  generate_bishop_move(cb, pseudo);
  generate_queen_move(cb, pseudo);
  generate_king_move(cb, pseudo);
  generate_pawn_move(cb, pseudo);

  // Puis on ne conserve que ceux qui laissent le roi hors d'échec
  movelist m = create_movelist();
  for (movelist p = get_next(pseudo); p != NULL; p = get_next(p)) {
    move c = get_current_move(p);
    if (is_move_legal(cb, c)) {
      push_movelist(c, m);
    }
  }
  free_movelist(pseudo);
  return m;
}
