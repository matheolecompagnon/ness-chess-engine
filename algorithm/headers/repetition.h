#ifndef __REPETITION_H__
#define __REPETITION_H__

/**
 * \file repetition.h
 * \brief Ce fichier décrit la détection de la nulle par répétition triple.
 * \details Une position répétée trois fois avec le même joueur au trait, les
 * mêmes droits de roque et la même cible en passant est une partie nulle. Sans
 * cette règle, le moteur ne voit aucune différence entre une position déjà
 * visitée et une position neuve : il peut donc "naviguer" indéfiniment dans une
 * position gagnante et concéder la nulle, et surtout il ne sait pas se servir
 * de la répétition comme d'une planche de salut quand il est moins bien.
 *
 * L'identité d'une position est celle que donne déjà la clé de Zobrist
 * (zobrist.h), qui mêle exactement les mêmes ingrédients que la règle. On
 * réutilise donc la clé calculée pour la table de transposition : la détection
 * ne coûte aucun hachage supplémentaire.
 *
 * L'historique est tenu \b hors du \b chessboard, dans ce module. C'est
 * délibéré : la structure chessboard est recopiée intégralement à chaque test
 * de légalité (simulate_move_into, un appel par coup pseudo-légal), et y loger
 * un tableau de plusieurs centaines de clés rendrait cette copie ruineuse. Ici,
 * l'historique est un état global de la recherche, empilé et dépilé le long du
 * chemin exploré.
 *
 * L'historique se compose de deux parties consécutives :
 * - les positions \b réellement \b jouées dans la partie, poussées par uci.c à
 * chaque commande "position ... moves ..." ; la dernière d'entre elles est la
 * position racine ;
 * - le \b chemin de la recherche en cours, empilé et dépilé par negamax_ply().
 *
 * - repetition_clear() vide l'historique (nouvelle partie).
 * - repetition_push() ajoute une position.
 * - repetition_pop() retire la dernière position ajoutée.
 * - repetition_begin_search() marque la frontière entre partie jouée et arbre
 * de recherche.
 * - repetition_is_draw() teste si une position est une nulle par répétition.
 *
 * \author LECOMPAGNON Mathéo
 * \date 21st July 2026
 */

#include <stdint.h>

/**
 * \brief repetition_clear() vide entièrement l'historique des positions.
 * \details À appeler au début d'une nouvelle partie (commande UCI "ucinewgame")
 * et avant de reconstituer une partie avec "position ... moves ...".
 */
void repetition_clear(void);

/**
 * \brief repetition_push(key) ajoute la position de clé \b key à l'historique.
 * \details Appelée par uci.c pour chaque position de la partie (position
 * initiale comprise), et par la recherche pour chaque position du chemin
 * exploré.
 * \param key la clé de Zobrist de la position.
 */
void repetition_push(uint64_t key);

/**
 * \brief repetition_pop() retire la dernière position ajoutée.
 * \details Contrepartie de repetition_push() le long du chemin de recherche : à
 * chaque undo_move() de la récursion doit correspondre un dépilement, faute de
 * quoi l'historique enflerait et signalerait des répétitions inexistantes.
 */
void repetition_pop(void);

/**
 * \brief repetition_begin_search(root_key) marque le début de l'arbre de
 * recherche.
 * \details Tout ce qui a été empilé avant cet appel est de l'historique de \b
 * partie (où la règle exige trois occurrences) ; tout ce qui le sera après est
 * le \b chemin de la recherche (où une seule répétition suffit, voir
 * repetition_is_draw()).
 *
 * Si l'historique est vide - cas du benchmark et de --selftest, qui ne passent
 * pas par l'UCI - la position racine y est ajoutée, afin qu'un retour à la
 * racine soit tout de même détecté.
 * \param root_key la clé de Zobrist de la position racine.
 */
void repetition_begin_search(uint64_t root_key);

/**
 * \brief repetition_is_draw(key) teste si la position de clé \b key est nulle
 * par répétition.
 * \details Convention des moteurs forts, dite "2-fold = nulle en recherche" :
 * \b une seule occurrence antérieure de la position - qu'elle soit dans le \b
 * chemin de la recherche ou dans l'historique de la \b partie \b réellement \b
 * jouée - suffit à la traiter comme nulle. Motif : si le joueur au trait peut
 * ramener la partie à une position déjà rencontrée, rien ne l'empêche de
 * recommencer, donc la recherche tient la nulle pour acquise. Cette règle,
 * combinée au contempt (negamax.c), pousse le camp GAGNANT à ne jamais revenir
 * sur une position déjà vue : il progresse vers le mat au lieu de tourner en
 * rond et de concéder la nulle dans une finale gagnée. L'arbitre, lui, exige
 * toujours la vraie triple occurrence.
 *
 * Compiler avec \c -DNESS_REP_STRICT_3FOLD restaure l'ancien comportement (dans
 * la partie jouée, il faut deux occurrences antérieures, soit la vraie triple
 * occurrence) - conservé comme témoin de mesure.
 * \param key la clé de Zobrist de la position à tester.
 * \return 1 si la position est une nulle par répétition (au sens ci-dessus), 0
 * sinon.
 */
uint8_t repetition_is_draw(uint64_t key);

#endif
