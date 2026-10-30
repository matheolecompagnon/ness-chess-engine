# Journal des modifications (CHANGELOG)

## [Non publié]

### À venir
- **Quiescence** : Prolongation de la recherche sur les captures et échecs (anti-effet d'horizon).
- **Tri des coups** : Intégration des Killer moves et de la table d'historique.
- **Règle des 50 coups** : Prise en compte de la nulle sans capture ni poussée de pion.
- **Clé Zobrist** : Mise à jour incrémentale à chaque coup plutôt qu'un recalcul global.
- **Contrôle UCI** : Recherche non bloquante avec support de stop et retours info depth.
- **Multithreading** : Suppression de l'état global pour permettre la recherche parallèle.
- **Poids NNUE :** Généraliser le plus possible l'importation des fichiers de coefficients.
- **Poids embarqués :** Proposer un nnue "non nul" embarqué par défaut pour le moteur.
- **Recherche :** Permettre le remplacement de l'algorithme Negamax par d'autres modules.
- **Calcul HPC :** Intégrer des optimisations pour le calcul haute performance.
- **Tests UCI :** Créer une suite de validation pour les commandes textes.
- **Tests unitaires :** Étendre la couverture de test aux modules de recherche et d'évaluation.

## [0.0.1] - 2026-10-29

### Ajouté
- **Bitboards :** Modélisation de l'échiquier sur des masques de 64 bits.
- **Générateur :** Calcul des coups pseudo-légaux et filtrage des échecs.
- **Pile de coups :** Interface opaque déclinée en tableau dynamique ou liste chaînée.
- **Recherche :** Architecture Negamax avec élagage alpha-bêta.
- **Variations PVS :** Tri et exploration par variation principale.
- **Réductions LMR :** Diminution de la profondeur sur les coups tardifs.
- **Élagage coup nul :** Mécanisme de Null-Move Pruning pour couper les branches stables.
- **Transposition :** Table de hachage basée sur les clés de Zobrist.
- **Répétitions :** Module de détection de positions identiques pour le score de nulle.
- **Gestion du temps :** Allocation du budget de réflexion selon la pendule UCI.
- **Réseau NNUE :** Évaluation par réseau de neurones à mise à jour différentielle.
- **Stub NNUE :** Liaison automatique vers un réseau neutre à coefficients zéro.
- **Couche UCI :** Interface texte pour la communication avec les plateformes de jeu.
- **Ouvertures :** Intégration d'un lecteur de fichiers binaires Polyglot.
- **Auto-test :** Option de diagnostic interne avec validation des noeuds Perft.
- **Suites CUnit :** Intégration de 42 tests unitaires validant 714 assertions.
- **Harnais Python :** Scripts de calcul d'Elo et de matchs automatisés.

[Non publié]: https://github.com
[0.0.1] : https://github.com
