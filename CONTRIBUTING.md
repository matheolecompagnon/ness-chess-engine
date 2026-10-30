# Contribuer au projet NESS

Ce guide rassemble les règles techniques et collaboratives pour développer sur le moteur d'échecs NESS.

---

## Règles du dépôt et workflow

### Gestion des branches
* Branche main protégée : aucun push direct autorisé.
* Pull Requests (PR) : passage obligatoire par une PR GitHub.
* Périmètre d'une PR : un seul sujet, correctif ou fonctionnalité, par PR.

### Sécurité des données
* Fichiers exclus : interdiction de commiter des poids réels (.nnue).
* Comportement par défaut : liaison automatique sur algorithm/src/nnue_stub.c.
* Évaluation par défaut : le moteur stubbé renvoie un score de 0.

---

## Convention des commits et linéarité

### Syntaxe obligatoire
Chaque commit utilise le format "type: description" en minuscules. Un espace est strictement obligatoire après les deux-points.

* Exemple valide : `feat: ajout du null-move pruning`
* Exemple invalide : `feat:ajout du null-move pruning`

### Types de commits et impact SemVer

| Type | Rôle pour NESS | Impact Version |
| :--- | :--- | :--- |
| feat | Nouvelle fonctionnalité algorithmique (ex: PVS, Polyglot) | MINOR |
| fix | Correction de bug (ex: fuite de génération de coup) | PATCH |
| perf | Optimisation (ex: vectorisation SIMD, bitboards) | PATCH |
| docs | Modification de la documentation | Aucun |
| test | Ajout ou modification de tests (CUnit, auto-test) | Aucun |
| refactor | Réécriture de code sans changement de comportement | Aucun |
| style | Mise en forme (espaces, indentation, renommage) | Aucun |
| chore | Maintenance (Makefile, scripts de bench) | Aucun |

### Linéarité Git stricte
* Rebase local : exécutez `git rebase origin/main` avant de pousser.
* Zéro merge commit : les noeuds de fusion locaux sont interdits.
* Stratégie GitHub : activation exclusive du Squash merge et Rebase merge.

---

## Guide de compilation

### Commandes de base
```sh
make            # Construit le binaire exécutable UCI ./chess
make re         # Force la reconstruction complète du projet
```

### Variables de configuration du Makefile
Chaque modification de variable impose l'exécution de `make re`.

| Variable | Rôle | Configuration par défaut |
| :--- | :--- | :--- |
| OPT | Options d'optimisation du compilateur | -O3 -march=native -flto |
| MOVELIST | Implémentation de la pile de coups | dynamic ou linked |
| NET | Nom du fichier d'évaluation (sans le .c) dans algorithm/src/ | nnue_stub |
| BOOK=1 | Active le livre d'ouverture Polyglot | Désactivé par défaut |
| BOOK_PATH | Chemin du livre binaire | books/komodo.bin |


### Règles du compilateur
* Drapeaux de build : compilation en -Wall -Wextra -Werror -std=c11.
* Avertissements : tout warning stoppe la compilation.
* Fichiers d'en-tête : le Makefile ne traquant pas les .h, lancez `make re`.
* Objets à plat : stockage dans obj/. Doublons de noms de base interdits.

### Macros de diagnostic
À passer via `CFLAGS+="-D[MACRO]"` ou dans les options du compilateur pour isoler l'impact des algorithmes :
* `TT_BITS` : taille de la table de transposition (défaut : 20, soit 32 Mo).
* `NESS_BENCH` : active le compteur de nœuds global pour les outils de bench.
* `NESS_DISABLE_TT` : désactive la table de transposition.
* `NESS_DISABLE_NULLMOVE` : désactive l'élagage par coup nul.
* `NESS_DISABLE_PVS` : désactive la recherche par variation principale.
* `NESS_DISABLE_LMR` : désactive les réductions de coups tardifs.


---

## Validation et tests

### Commandes de test
```sh
make test_re                       # Tests unitaires CUnit (pile dynamique)
make MOVELIST=linked test_re       # Tests unitaires CUnit (pile chaînée)
make re && ./chess --selftest      # Auto-test interne (doit renvoyer 0)
```

### Processus d'intégration d'un test unitaire
1. Écrire : créer la fonction test_xxx avec CU_ASSERT_* dans tests/src/test_<module>.c.
2. Déclarer : ajouter le prototype dans tests/headers/test_<module>.h.
3. Enregistrer : ajouter CU_add_test(suite, "Test xxx", test_xxx) dans tests/src/test_main.c.

---

## Architecture du code

Séparation entre interfaces (headers/) et implémentations (src/) :
* types/ : bitboards, échiquier, structures de données.
* rules/ : génération et application des coups.
* algorithm/ : moteur de recherche (Negamax), table TT, NNUE.
* uci/ : protocole de communication texte et livre Polyglot.

### Choix techniques
* Make/Unmake : modifications de l'échiquier en place (apply_move / undo_move). Aucune copie d'état.
* Pile opaque : ne pas empiler dans une liste de coups en cours de parcours.
* LTO (-flto) : obligatoire pour l'inlining cross-modules de evaluate et get_bit.

---

## Check-list avant PR

Cochez ces éléments localement avant de pousser sur GitHub :
- [ ] `make test_re` et `make MOVELIST=linked test_re` passent à 100%.
- [ ] L'auto-test `./chess --selftest` retourne le code 0.
- [ ] Le test de flux UCI répond uciok, readyok et émet un bestmove.
- [ ] Les gains de recherche sont chiffrés via le compteur de noeuds de `make bench`.
- [ ] Les blocs de commentaires au format Doxygen (\brief, \param) sont présents dans les .h.
- [ ] L'historique Git is rebasé et linéaire (`git rebase origin/main`).

---

## Code de conduite du projet NESS

### Engagement
* Environnement professionnel : expérience exempte de harcèlement pour tous les développeurs.
* Rigueur technique : interdiction de falsifier les résultats d'Elo ou de benchmarks.
* Partage : explications des choix algorithmiques lors des revues de code.
* Objectivité : acceptation des critiques axées sur la performance (cache-misses, mémoire).

---

## 

### TODO

- [ ] **Profilage** : perf et/ou gprof pour voir les possibilités d'optimisations.
- [ ] **Quiescence** : Prolongation de la recherche sur les captures et échecs (anti-effet d'horizon).
- [ ] **Tri des coups** : Intégration des Killer moves et de la table d'historique.
- [ ] **Règle des 50 coups** : Prise en compte de la nulle sans capture ni poussée de pion.
- [ ] **Clé Zobrist** : Mise à jour incrémentale à chaque coup plutôt qu'un recalcul global.
- [ ] **Contrôle UCI** : Recherche non bloquante avec support de stop et retours info depth.
- [ ] **Multithreading** : Suppression de l'état global pour permettre la recherche parallèle.
- [ ] **Poids NNUE :** Généraliser le plus possible l'importation des fichiers de coefficients.
- [ ] **Poids embarqués :** Proposer un nnue "non nul" embarqué par défaut pour le moteur.
- [ ] **Recherche :** Permettre le remplacement de l'algorithme Negamax par d'autres modules.
- [ ] **Calcul HPC :** Intégrer des optimisations pour le calcul haute performance.
- [ ] **Tests UCI :** Créer une suite de validation pour les commandes textes.
- [ ] **Tests unitaires :** Étendre la couverture de test aux modules de recherche et d'évaluation.

### Attention

- Le clamp de l'évaluation borne à SCORE_INF 30000, alors que le SCORE_MATE vaut 29000
