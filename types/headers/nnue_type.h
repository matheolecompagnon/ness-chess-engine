#ifndef __NNUE_TYPE_H__
#define __NNUE_TYPE_H__

/**
 * \file nnue_type.h
 * \brief Ce fichier décrit les types de données du réseau NNUE.
 * \details Le NNUE (Efficiently Updatable Neural Network) évalue une position
 * sans tout recalculer à chaque fois : la sortie de sa 1ère couche (l'\b
 * accumulator) est mise à jour petit à petit à chaque coup, en ne changeant que
 * les features qui bougent. On garde \b deux accumulateurs : un vu par les
 * Blancs, un vu par les Noirs. Ce fichier ne contient que les
 * \b types ; la logique est dans nnue.c.
 * - \b NNUE_HL_SIZE le nombre de neurones de la couche cachée.
 * - \b NNUE_INPUT_SIZE le nombre de features en entrée (2 couleurs * 6 types *
 * 64 cases).
 * - \b accumulator la sortie de la 1ère couche pour une perspective.
 * - \b accumulator_pair les deux accumulateurs (un par perspective).
 * - \b network les poids et biais du réseau (à remplir depuis l'entraînement).
 *
 * \see Structures conçues d'après la page NNUE du Chess Programming Wiki :
 *      https://www.chessprogramming.org/NNUE
 * \author LECOMPAGNON Mathéo
 * \date 13th July 2026
 */

#include <stdint.h>

/*
 * Inclusion de la configuration unifiée du réseau actif (générée ou gérée par
 * le Makefile). Cela garantit que NNUE_INPUT_SIZE et NNUE_HL_SIZE sont
 * rigoureusement identiques dans toute l'unité de compilation et évitent les
 * conflits LTO.
 */
#include "net_config.h"

/*
 * Toutes les dimensions et tous les parametres de quantification sont proteges
 * par #ifndef : ce sont les valeurs du reseau ACTUEL (nnue_stub.c), mais
 * elles peuvent etre SURCHARGEES a la compilation par -D (voir la variable NET
 * du Makefile). Cela permet de garder plusieurs reseaux de tailles differentes
 * dans le depot et de choisir lequel construire sans editer ce fichier. Toute
 * unite de compilation voit la meme valeur : la surcharge doit donc etre passee
 * globalement (dans CFLAGS), et changer de reseau impose un rebuild complet
 * ('make NET=... re').
 */

/**
 * \brief Nombre de features en entrée du réseau.
 * \details Une feature = (couleur, type de pièce, case) : 2 * 6 * 64 = 768.
 */
#ifndef NNUE_INPUT_SIZE
#define NNUE_INPUT_SIZE 768
#endif

/**
 * \brief Nombre de neurones de la couche cachée.
 * \details Doit correspondre EXACTEMENT à la valeur utilisée à l'entraînement
 * du réseau.
 */
#ifndef NNUE_HL_SIZE
#define NNUE_HL_SIZE 32
#endif

/*
 * Paramètres de quantification / d'échelle (fixés à l'entraînement).
 * NNUE_SCALE : facteur pour repasser en unité de score (centipions).
 * NNUE_QA : borne de la CReLU (quantification des activations).
 * NNUE_QB : quantification des poids de sortie.
 */
#ifndef NNUE_SCALE
#define NNUE_SCALE 400
#endif
#ifndef NNUE_QA
#define NNUE_QA 127
#endif
#ifndef NNUE_QB
#define NNUE_QB 64
#endif

/**
 * \brief Structure \b accumulator : la sortie de la 1ère couche du NNUE pour
 * une perspective.
 * \details C'est la partie coûteuse du réseau ; on la maintient
 * incrémentalement (add/sub) plutôt que de la recalculer à chaque évaluation.
 */
typedef struct accumulator {
  int16_t values[NNUE_HL_SIZE]; /**< Valeurs des neurones de la couche cachée */
} accumulator;

/**
 * \brief Structure \b accumulator_pair : les deux accumulateurs, un par
 * perspective.
 * \details perspective[WHITE] voit l'échiquier du point de vue des Blancs,
 * perspective[BLACK] du point de vue des Noirs (couleur des features inversée,
 * cases retournées verticalement).
 */
typedef struct accumulator_pair {
  accumulator perspective[2]; /**< Indexé par \b color : [WHITE] et [BLACK] */
} accumulator_pair;

/**
 * \brief Structure \b network : les poids et biais du réseau NNUE.
 * \details Destinée à résider en flash (const) : à remplir avec les valeurs
 * issues de l'entraînement. accumulator_weights[f] est le vecteur ajouté à
 * l'accumulateur quand la feature f est active.
 */
typedef struct network {
  int16_t accumulator_weights[NNUE_INPUT_SIZE]
                             [NNUE_HL_SIZE]; /**< Poids de la 1ère couche */
  int16_t accumulator_biases[NNUE_HL_SIZE];  /**< Biais de la 1ère couche */
  int16_t output_weights[2 * NNUE_HL_SIZE]; /**< Poids de sortie : [0,HL[ = stm,
                                               [HL,2HL[ = nstm */
  int16_t output_bias;                      /**< Biais de sortie */
} network;

#endif
