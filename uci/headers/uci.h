#ifndef __UCI_H__
#define __UCI_H__

/**
 * \file uci.h
 * \brief Ce fichier décrit l'interface UCI (Universal Chess Interface) du
 * moteur.
 * \details Le protocole UCI est un dialogue en texte sur l'entrée/sortie
 * standard : une interface graphique (Knights, Cute Chess, Arena, XBoard, ...)
 * envoie des commandes ligne par ligne, le moteur répond de même. Cela permet
 * de faire jouer le moteur contre un humain ou contre un autre moteur.
 *
 * Commandes reconnues :
 * - \b uci : le moteur s'identifie et répond \b uciok.
 * - \b isready : le moteur répond \b readyok.
 * - \b ucinewgame : réinitialise l'échiquier à la position de départ.
 * - \b setoption : le moteur n'annonce \b aucune option ; toute option reçue
 * est signalée puis ignorée (Knights envoie par exemple un "Hash" jamais
 * annoncé).
 * - \b position \b startpos|fen \b \<FEN\> [moves \b e2e4 \b e7e5 ...] : place
 * la position à analyser.
 * - \b go [depth \b n] [movetime \b t] : lance la recherche par
 * approfondissement itératif et répond \b bestmove \b \<coup\>. \b movetime
 * fixe le budget de temps en ms (sinon un défaut),
 *   \b depth un plafond de profondeur (sinon SEARCH_MAX_DEPTH, hors de portée
 * en pratique). La recherche s'arrête avant toute itération non terminable dans
 * le temps imparti : le moteur répond toujours dans un délai borné, même si
 * l'interface demande une profondeur inatteignable (ex. Knights envoie \b go \b
 * depth \b 16).
 *
 * \note Le moteur joue \b toujours à pleine force : le \b temps est le seul
 * réglage. L'échelle de difficulté "Skill Level" (profondeur réduite + bruit
 * sur les scores racine) a existé puis a été retirée, l'approfondissement
 * itératif borné en temps couvrant le besoin.
 * - \b help : affiche la liste des commandes (extension hors protocole, pour
 * l'usage à la main).
 * - \b quit : termine la boucle.
 *
 * Lorsque l'entrée standard est un terminal, le moteur affiche un accueil au
 * lancement pour ne pas paraître bloqué. Lancé par une interface graphique, il
 * reste muet tant qu'on ne lui a rien demandé, comme l'exige le protocole.
 *
 * - uci_loop() pour lancer la boucle de dialogue.
 * - uci_set_startpos() pour placer la position de départ standard sur un
 * échiquier vide.
 *
 * \author LECOMPAGNON Mathéo
 * \date 16th July 2026
 */

#include "type.h"

/**
 * \brief uci_set_startpos(cb) place la position de départ standard sur
 * l'échiquier \b cb.
 * \details Pose les pièces, met les blancs au trait, accorde les quatre droits
 * de roque et annule la cible du en passant. Les accumulateurs NNUE ne sont \b
 * pas recalculés : appeler nnue_refresh() ensuite si l'on veut évaluer ou
 * appliquer un coup.
 * \pre \b cb est un échiquier vide (sortant de create_chessboard()).
 * \param cb un \b chessboard
 */
void uci_set_startpos(chessboard cb);

/**
 * \brief uci_loop() lit les commandes UCI sur l'entrée standard jusqu'à \b quit
 * (ou EOF).
 * \details Maintient un échiquier interne, initialisé à la position de départ.
 * Une commande inconnue est ignorée, comme le veut le protocole.
 */
void uci_loop(void);

#endif
