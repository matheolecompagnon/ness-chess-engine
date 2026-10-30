#ifndef __TYPE_H__
#define __TYPE_H__

/**
 * \file type.h
 * \brief Ce fichier décrit les types représentant un échiquier complet et leurs
 * fonctions.
 * \details L'état du jeu est réparti par bitboards : un \b chessboard réunit,
 * pour chaque couleur et chaque type de pièce, le bitboard des positions, ainsi
 * que le joueur au trait, les droits de roque et la cible du en passant.
 * - \b piecetype le type d'une pièce (PAWN, ROOK, KNIGHT, BISHOP, QUEEN, KING)
 * ; EMPTY = pas de promotion.
 * - \b color la couleur d'un joueur (WHITE, BLACK).
 * - \b castlingtype un des quatre droits de roque (WHITE/BLACK, côté
 * reine/roi).
 * - \b piece une pièce décrite par son type et sa couleur.
 * - \b chessboard l'état complet de l'échiquier (pointeur sur struct
 * chessboard).
 * - get_piece_type() / get_piece_color() pour lire le type et la couleur d'une
 * \b piece.
 * - get_piece_bitboard() pour lire (copie) le bitboard d'un type et d'une
 * couleur.
 * - get_piece_bitboard_ref() pour obtenir un pointeur modifiable sur ce
 * bitboard.
 * - get_color_bitboard() pour l'union des pièces d'une couleur.
 * - get_chessboard_color() pour le joueur au trait.
 * - get_castling_right / set_castling_right / remove_castling_right pour les
 * droits de roque.
 * - get_en_passant_target / set_en_passant_target pour la cible du en passant
 * (-1 à défaut).
 * - create_chessboard() pour allouer un échiquier vide, free_chessboard(cb)
 * pour le désallouer.
 *
 * \author LECOMPAGNON Mathéo
 * \date 8th June 2026
 */

#include "bitboard.h"
#include "nnue_type.h"

/**
 * \brief Enumération \b piecetype représentant chaque type de pièces sur
 * l'échiquier.
 * \details Ces valeurs sont prises pour correspondre à l'index dans un tableau.
 */
typedef enum piecetype {
  PAWN = 0,   /**< Pion */
  ROOK = 1,   /**< Tour */
  KNIGHT = 2, /**< Cavalier */
  BISHOP = 3, /**< Fou */
  QUEEN = 4,  /**< Reine */
  KING = 5,   /**< Roi */
  EMPTY = 6   /**< Pas de promotion appliquée */
} piecetype;

/**
 * \brief Enumération \b color représentant les couleurs de l'échiquier
 * \details Ces valeurs sont prises pour correspondre à l'index dans un tableau.
 */
typedef enum color {
  WHITE = 0, /**< Blancs */
  BLACK = 1  /**< Noirs */
} color;

typedef enum castlingtype {
  WHITE_QUEEN_SIDE = 0, /**< blancs, côté reine */
  WHITE_KING_SIDE = 1,  /**< blancs, côté roi */
  BLACK_QUEEN_SIDE = 2, /**< noirs, côté reine */
  BLACK_KING_SIDE = 3,  /**< noirs, côté roi */
} castlingtype;

/**
 * \brief Structure \b piece modélisant une pièce d'échecs par son type \b type,
 * et sa couleur \b color.
 */
typedef struct piece {
  piecetype type;    /**< Rôle */
  color piece_color; /**< Couleur */
} piece;

/**
 * \brief Structure représentant l'état complet de l'échiquier.
 * \details Chaque type de pièce d'une couleur admet un bitboard.
 */
struct chessboard {
  bitboard pieceboard[2][6]; /**< [color][piecetype] = positions des pièces,
                                piecetype de 0 (PAWN) à 5 (KING) */
  color chessboard_color;    /**< Joueur dont c'est le tour */
  uint8_t castling_rights; /**< Droits de faire les roques de chaque couleur de
  chaque côté : bit 0 blanc gauche, bit 1 blanc droit, bit 2 noir gauche, bit 3
  noir droit */
  int16_t en_passant_target; /**< Case où un pion pourrait se déplacer dans la
                                règle du en passant. -1 à défaut. */
  accumulator_pair acc;      /**< Accumulateurs NNUE : avancés par apply_move,
                                restaurés par undo_move (make/unmake) */
};

typedef struct chessboard *chessboard;

/**
 * \brief create_chessboard() initialise un chessboard vide
 * \return un \b chessboard vide.
 */
chessboard create_chessboard();

/**
 * \brief free_chesboard(cb) désalloue la mémoire utilisée pour creer le
 * chesboard \b cb.
 * \param cb un \b chessboard bien initialisé
 */
void free_chessboard(chessboard cb);

/**
 * \brief get_piece_type(p) retourne le type de la pièce \b p.
 * \param p une \b piece
 * \return Le type de la pièce \b p.
 */
piecetype get_piece_type(piece p);

/**
 * \brief get_piece_color(p) retourne la couleur de la pièce \b p.
 * \param p une \b piece
 * \return La couleur de la pièce \b p.
 */
color get_piece_color(piece p);

/**
 * \brief get_piece_bitboard(cb, c, type) retourne le bitboard des pièces de
 * type \b type et de couleur \b c de l'échiquier \b cb.
 * \param cb un \b chessboard
 * \param c une \b color
 * \param type un \b piecetype
 * \return Le bitboard des pièces de type \b type et de couleur \b c de
 * l'échiquier \b cb.
 */
bitboard get_piece_bitboard(chessboard cb, color c, piecetype type);

/**
 * \brief get_piece_bitboard_ref(cb, c, type) retourne un pointeur vers le
 * bitboard des pièces de type \b type et de couleur \b c de l'échiquier \b cb.
 * \details Contrairement à get_piece_bitboard() qui renvoie une copie (lecture
 * seule), cette fonction renvoie l'adresse du champ réel : à utiliser pour \b
 * modifier le bitboard via set_bit()/clear_bit().
 * \param cb un \b chessboard
 * \param c une \b color
 * \param type un \b piecetype
 * \return Un pointeur vers le bitboard des pièces de type \b type et de couleur
 * \b c.
 */
bitboard *get_piece_bitboard_ref(chessboard cb, color c, piecetype type);

/**
 * \brief get_color_bitboard(cb, c) retourne le bitboard de toutes les pièces de
 * couleur \b c.
 * \param cb un \b chessboard
 * \param c une \b color
 * \return Le bitboard réunissant toutes les cases occupées par les pièces de
 * couleur \b c.
 */
bitboard get_color_bitboard(chessboard cb, color c);

/**
 * \brief get_chessboard_color(cb) retourne la couleur du joueur au trait de
 * l'échiquier \b cb.
 * \param cb un \b chessboard
 * \return La couleur du joueur au trait de l'échiquier \b cb.
 */
color get_chessboard_color(chessboard cb);

/** \brief get_castling_right(cb, t) retourne la condition "le droit de type t
 *est vérifié"
 *  \param cb un \b chessboard
 *  \param t un \b castlingtype
 *	\return la condition "le droit de type t est vérifié".
 */
uint8_t get_castling_right(chessboard cb, castlingtype t);

/** \brief set_castling_right(cb, t) met à 1 la condition "le droit de type t
 * est vérifié"
 *  \param cb un \b chessboard
 *  \param t un \b castlingtype
 */
void set_castling_right(chessboard cb, castlingtype t);

/** \brief remove_castling_right(cb, t) met à 0 la condition "le droit de type t
 * est vérifié"
 *  \param cb un \b chessboard
 *  \param t un \b castlingtype
 */
void remove_castling_right(chessboard cb, castlingtype t);

/**
 * \brief get_en_passant_target(cb) retourne l'index de la case cible du en
 * passant, ou -1 si la règle ne s'applique pas.
 * \param cb un \b chessboard.
 * \return -1 si la règle du en passant ne peut s'appliquer, sinon l'index de la
 * case pouvant être atteinte par un pion en passant.
 */
int8_t get_en_passant_target(chessboard cb);

/**
 * \brief set_en_passant_target(cb, t) met la valeur du en passant à \b t.
 * Sinon -1.
 * \pre t=-1 ou t est un \b cell.
 * \param cb un \b chessboard.
 * \param t la case cible du en passant, ou -1 si la règle ne s'applique pas.
 */
void set_en_passant_target(chessboard cb, int16_t t);

/**
 * \brief set_chessboard_color(cb, c) met la couleur de l'échiquier à \b c.
 * \param cb un \b chessboard.
 * \param c un \b color
 */
void set_chessboard_color(chessboard cb, color c);

#endif
