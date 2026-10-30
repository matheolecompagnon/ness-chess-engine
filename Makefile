NAME = chess
TEST_NAME = test_chess
CC = gcc
INC = \
    -Itypes/headers \
    -Irules/headers \
    -Ialgorithm/headers \
    -Iuci/headers \
    -Itests/headers

# Options d'optimisation : gain majeur sur la recherche (NNUE + parcours d'arbre).
# -march=native taille le binaire pour le CPU qui compile ; à retirer pour un binaire
# portable ou une cross-compilation vers une autre cible.
# -flto (Link Time Optimization) : le compilateur optimise à travers les fichiers au moment de
# l'édition de liens. Décisif ici (facteur 2, cf. D12), toutes les petites fonctions sont inlinés.
# IMPORTANT : -flto doit figurer à la compilation ET à l'édition de liens (voir les règles plus bas).
# -flto=auto pour éviter le warning de lto et pour s'adapter à la capacité de la machine
OPT = -O3 -march=native -flto=auto

# Options de compilation.
CFLAGS = $(OPT) -Wall -Wextra -Werror -std=c11 $(INC)

OBJ_DIR = obj

# Implémentation de la pile de coups à lier (interface opaque types/headers/movelist.h) :
#   dynamic -> types/src/dynamic_movelist.c (tableau dynamique) = défaut
#   linked  -> types/src/linked_movelist.c  (liste chaînée)
# Choix à la compilation : make MOVELIST=linked
MOVELIST ?= dynamic
MOVELIST_SRC = types/src/$(MOVELIST)_movelist.c

# Réseau NNUE embarqué. Un fichier C définissant `nnue_net` (même format que ceux générés
# par tools/nnue_to_c.py), déposé dans algorithm/src/<NET>.c, est lié à la compilation :
#   make NET=mon_reseau                                     # algorithm/src/mon_reseau.c
#   make NET=mon_reseau NNUE_HL=256 NNUE_INPUT=768 re       # réseau de taille différente
#   make NET_SRC=/chemin/autre.c re                         # fichier n'importe où
# Défaut : NET=nnue_stub. Si le fichier n'existe pas (dépôt public, sans poids), on lie
# algorithm/src/nnue_stub.c : réseau à zéro, le moteur compile et tourne mais n'évalue rien.
# CHANGER DE RÉSEAU OU DE DIMENSION IMPOSE UN 'make ... re' (la taille change l'ABI).
NET ?= nnue_stub
ifndef NET_SRC
  ifneq ($(wildcard algorithm/src/$(NET).c),)
    NET_SRC = algorithm/src/$(NET).c
  else
    ifneq ($(filter-out clean fclean readme rapport elo-setup,$(or $(MAKECMDGOALS),all)),)
      $(warning Fichier de poids algorithm/src/$(NET).c introuvable : lien du stub, evaluation neutre)
    endif
    NET_SRC = algorithm/src/nnue_stub.c
  endif
endif

# Gestion automatique de la configuration du réseau actif (générée par nnue_to_c.py)
# Copie automatiquement le fichier mon_reseau_config.h vers net_config.h pour unifier la compilation.
NET_CONFIG_SRC = algorithm/headers/$(NET)_config.h
NET_CONFIG_TARGET = algorithm/headers/net_config.h

# Sources de la bibliothèque (modules, sans main ni tests)
LIB_SRC = \
    types/src/bitboard.c \
    types/src/type.c \
    $(MOVELIST_SRC) \
    rules/src/rule_iter.c \
    rules/src/apply_move.c \
    algorithm/src/nnue.c \
    algorithm/src/zobrist.c \
    algorithm/src/repetition.c \
    algorithm/src/tt.c \
    algorithm/src/negamax.c \
    $(NET_SRC) \
    uci/src/uci.c

# Livre d'ouverture EMBARQUE, optionnel : make BOOK=1 (defaut : pas de livre).
# Rend le binaire AUTONOME - il joue son livre tout seul, meme sous une interface qui n'en
# fournit pas (Knights). Le module vit dans la couche UCI (cote PC) : il ne touche ni au NNUE,
# ni aux poids, ni a la recherche, donc le portage FPGA du reseau reste intact.
# Chemin du livre fige a la compilation (surchargeable par la variable d'environnement NESS_BOOK) :
#   make BOOK=1                                # utilise books/komodo.bin (chemin absolu)
#   make BOOK=1 BOOK_PATH=/chemin/autre.bin  # un autre livre
BOOK_PATH ?= $(abspath books/komodo.bin)
ifdef BOOK
CFLAGS  += -DNESS_BOOK -DNESS_BOOK_PATH='"$(BOOK_PATH)"'
LIB_SRC += uci/src/book.c uci/src/polyglot_keys.c
endif

# Source principale
MAIN_SRC = app/main.c

# Sources des tests
TEST_SRC = \
    tests/src/test_utils.c \
    tests/src/test_rule.c \
    tests/src/test_bitboard.c \
    tests/src/test_movelist.c \
    tests/src/test_type.c \
    tests/src/test_apply_move.c \
    tests/src/test_main.c

# Objets (aplatis dans obj/ : les basenames sont uniques)
LIB_OBJ  = $(addprefix $(OBJ_DIR)/,$(notdir $(LIB_SRC:.c=.o)))
MAIN_OBJ = $(addprefix $(OBJ_DIR)/,$(notdir $(MAIN_SRC:.c=.o)))
TEST_OBJ = $(addprefix $(OBJ_DIR)/,$(notdir $(TEST_SRC:.c=.o)))

# Où trouver les .c à partir de leur basename
vpath %.c types/src rules/src algorithm/src uci/src app tests/src $(dir $(NET_SRC))

# Règle principale (dépend de la préparation du header de configuration NNUE)
all: prepare $(NAME)

# Étape de préparation : s'assure que le header de configuration du réseau actif est en place
prepare: $(NET_CONFIG_TARGET) | $(OBJ_DIR)

$(NET_CONFIG_TARGET):
	@if [ -f "$(NET_CONFIG_SRC)" ]; then \
		cp -f $(NET_CONFIG_SRC) $@; \
		echo "-> Configuration NNUE chargée : $(NET)"; \
	else \
		echo "/* Default stub config */" > $@; \
		echo "#define NNUE_HL_SIZE 32" >> $@; \
		echo "#define NNUE_INPUT_SIZE 768" >> $@; \
		echo "-> Configuration NNUE par défaut (stub) chargée"; \
	fi

# Exécutable principal
$(NAME): $(LIB_OBJ) $(MAIN_OBJ)
	$(CC) $(OPT) $^ -o $@

# Exécutable des tests
$(TEST_NAME): $(LIB_OBJ) $(TEST_OBJ) | prepare
	$(CC) $(OPT) $^ -o $@ -lcunit

# Compilation générique d'un .c (trouvé via vpath) vers obj/
$(OBJ_DIR)/%.o: %.c $(NET_CONFIG_TARGET) | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Création du dossier obj/
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Lancer les tests
test: $(TEST_NAME)
	./$(TEST_NAME)

# Benchmark de la recherche (compteur de noeuds activé via -DNESS_BENCH).
# Compilé à part pour ne pas mêler des objets instrumentés au binaire normal.
BENCH_NAME = bench_ness
BENCH_SRC = bench/bench.c

$(BENCH_NAME): $(LIB_SRC) $(BENCH_SRC) | prepare
	$(CC) $(CFLAGS) -DNESS_BENCH $^ -o $@

bench: $(BENCH_NAME)
	./$(BENCH_NAME)

# Mesure de force en Elo : matchs de Ness contre Stockfish bridé (bench/elo_match.py).
# Nécessite Stockfish installé et un environnement python local avec python-chess ;
# "make elo-setup" le crée dans bench/.venv (non versionné). ELO_ARGS permet de
# surcharger le protocole, ex. : make elo ELO_ARGS="--games 200 --sf-elo 1800"
VENV = bench/.venv
ELO_PY = $(VENV)/bin/python
ELO_ARGS ?= --games 100 --movetime 1000 --sf-elo 1650 --concurrency 4

elo-setup:
	python3 -m venv $(VENV)
	$(VENV)/bin/pip install -q chess

bench/openings.epd: bench/gen_openings.py
	$(ELO_PY) bench/gen_openings.py

elo: $(NAME) bench/openings.epd
	$(ELO_PY) bench/elo_match.py $(ELO_ARGS)

clean:
	rm -rf $(OBJ_DIR)
	rm -rf bench/__pycache__ __pycache__
	rm -f *~ */*~ */*/*~
	rm -f bench/chess_now bench/chess_pendule bench/chess_snapshot
	rm -f algorithm/headers/net_config.h

fclean: clean
	rm -f $(NAME) $(TEST_NAME) $(BENCH_NAME)

re: fclean all

# Recompilation + tests
test_re: fclean test

.PHONY: all prepare clean fclean re test test_re bench elo elo-setup