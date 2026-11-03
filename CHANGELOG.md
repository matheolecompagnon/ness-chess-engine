# Journal des modifications (CHANGELOG)

## [Non publié]

## [0.2.0] - 2026-11-02

### Ajouté

- **Réseaux NNUE embarqués :** `embedded_nnue_768x32` et `embedded_nnue_768x256`.

### Modifié

- Le réseau par défaut est `embedded_nnue_768x256`

### Corrigé

- Correction du clip de l'évaluation en corrélation avec le mate


## [0.1.0-alpha] - 2026-10-29

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

[Non publié]: https://github.com/matheolecompagnon/ness-chess-engine
[0.1.0-alpha]: https://github.com/matheolecompagnon/ness-chess-engine/releases/tag/v0.1.0-alpha
