/**
 * \file negamax.h
 * \brief Ce fichier décrit l'algorithme de recherche negamax avec élagage
 * alpha-bêta.
 * \details negamax explore l'arbre des coups jusqu'à une profondeur donnée,
 * évalue les feuilles avec evaluate, et remonte le meilleur score du point de
 * vue du joueur au trait. L'élagage alpha-bêta coupe les branches qui ne
 * peuvent plus améliorer le résultat.
 * - SCORE_MATE le score d'un mat immédiat.
 * - SEARCH_MAX_DEPTH le plafond de profondeur de l'approfondissement itératif.
 * - negamax() renvoie le score de \b cb à la profondeur \b depth.
 * - search_best_move() renvoie le meilleur coup pour le joueur au trait de \b
 * cb.
 * - search_best_move_timed() idem, borné par un budget de temps.
 *
 * \note Le moteur joue \b toujours à pleine force. Il n'existe pas de réglage
 * de difficulté : le \b temps est le seul paramètre qui borne la recherche
 * (voir search_best_move_timed()). Une échelle de niveaux (profondeur réduite +
 * bruit ajouté aux scores racine) a existé puis a été retirée :
 * l'approfondissement itératif borné en temps couvre le besoin, sans dégrader
 * la qualité de jeu ni compliquer la recherche.
 *
 * \author LECOMPAGNON Mathéo
 * \date 6th July 2026
 */

#ifndef __NEGAMAX_H__
#define __NEGAMAX_H__

#include "../../types/headers/movelist.h"
#include "nnue.h"

/**
 * \brief SCORE_MATE est le score d'un mat subi immédiatement (-SCORE_MATE pour
 * le joueur maté).
 * \details Un mat atteint après \b ply demi-coups vaut -SCORE_MATE + ply : un
 * mat lointain est donc moins grave qu'un mat proche, ce qui fait résister le
 * moteur le plus longtemps possible dans une position perdue, et conclure au
 * plus court dans une position gagnée.
 *
 * Il est pris nettement au-dessus de toute évaluation plausible, mais
 * strictement sous SCORE_INF, pour qu'un score de mat ne puisse être confondu
 * ni avec une évaluation ordinaire, ni avec l'infini de la fenêtre alpha-bêta.
 */
#define SCORE_MATE ((score)29000)

/**
 * \brief SEARCH_MAX_DEPTH est la profondeur maximale visée quand aucune n'est
 * imposée.
 * \details Ce n'est pas une limite de force mais un garde-fou : c'est le \b
 * budget \b de \b temps qui arrête réellement l'approfondissement itératif,
 * bien avant cette valeur (la profondeur 8 demande déjà quelques secondes). La
 * valeur reste modeste parce que le compteur de profondeur est un uint8_t : un
 * plafond à 255 le ferait déborder si le budget était énorme.
 */
#define SEARCH_MAX_DEPTH ((uint8_t)64)

/**
 * \brief negamax(cb,depth,alpha,beta) renvoie le score du chessboard \b cb
 * passé dans l'algorithme negamax.
 * \details evaluate() rendant le score du point de vue du joueur au trait, le
 * changement de camp est géré par la négation du negamax (pas de paramètre de
 * couleur).
 *
 * Positions terminales : une position sans coup légal vaut -SCORE_MATE si le
 * roi du joueur au trait est en échec (mat), et 0 sinon (pat, donc nulle). Ce
 * test n'a lieu qu'aux noeuds internes : un mat situé exactement à l'horizon
 * (depth==0) est rendu comme une évaluation ordinaire.
 * \param cb un \b chessboard bien initialisé
 * \param depth un entier
 * \param alpha un \b score
 * \param beta un \b score
 * \return Le score suite à l'algorithme negamax
 */
score negamax(chessboard cb, uint8_t depth, score alpha, score beta);

/**
 * \brief search_best_move(cb, depth) renvoie le meilleur coup pour le joueur
 * sur \b cb.
 * \details Point d'entrée de la recherche : (re)construit les accumulateurs
 * NNUE de \b cb via nnue_refresh(), puis évalue chaque coup racine par negamax
 * et renvoie celui de score maximal.
 *
 * Un coup légal est toujours renvoyé s'il en existe un, y compris lorsque tous
 * perdent par mat forcé : dans ce cas le score du mat le plus lointain
 * l'emporte, et le moteur résiste au lieu de rester muet.
 * \pre depth >=1
 * \param cb un \b chessboard bien initialisé
 * \param depth la profondeur de recherche
 * \return le meilleur \b move OU un coup nul {0}, (from==to==0) si et seulement
 * si aucun coup légal n'existe (mat ou pat sur \b cb).
 * \post \b cb est restauré dans son état initial (make/unmake) à la fin.
 */
move search_best_move(chessboard cb, uint8_t depth);

/**
 * \brief search_best_move_timed(cb, max_depth, budget_ms) cherche par
 * approfondissement itératif borné par un budget de temps.
 * \details Explore les profondeurs 1, 2, 3... jusqu'à \b max_depth, en
 * s'arrêtant avant toute itération que l'on estime ne pas pouvoir terminer dans
 * \b budget_ms millisecondes. Renvoie toujours le meilleur coup de la dernière
 * profondeur \b entièrement explorée : le moteur répond donc dans un délai
 * borné même si \b max_depth est hors de portée (une interface graphique comme
 * Knights demande parfois une profondeur inatteignable, ex. "go depth 16"). La
 * profondeur 1 étant quasi instantanée, un coup légal est toujours renvoyé s'il
 * en existe un.
 * \pre max_depth >= 1 (une valeur nulle est ramenée à 1)
 * \param cb un \b chessboard bien initialisé
 * \param max_depth la profondeur maximale visée (plafond ; SEARCH_MAX_DEPTH à
 * défaut d'exigence)
 * \param budget_ms le temps maximal alloué au coup, en millisecondes
 * \return le meilleur \b move trouvé OU un coup nul {0} si aucun coup légal
 * n'existe (mat/pat).
 * \post \b cb est restauré dans son état initial à la fin.
 */
move search_best_move_timed(chessboard cb, uint8_t max_depth,
                            uint32_t budget_ms);

#endif
