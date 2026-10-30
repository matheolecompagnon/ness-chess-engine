Ce dossier contient les outils en Python pour évaluer, comparer et mesurer la
force du moteur d'échecs **Ness** face à des étalons comme Stockfish.

## Contenu du dossier

* **`openings.epd`** : Un livre d'ouvertures équilibré (généré au préalable)
contenant des positions FEN variées pour éviter que le moteur (qui est
déterministe) ne joue toujours la même partie.
* **`elo_match.py`** : Script de mesure de l'Elo de Ness par matchs contre un
 Stockfish bridé (`UCI_LimitStrength`). Chaque ouverture est jouée deux fois
 (couleurs inversées) pour annuler l'avantage des blancs, avec calcul d'une
 marge d'erreur statistique à 95 %.
* **`compare_engines.py`** : Script de combat libre entre deux
moteurs configurables (profondeur, nœuds, temps, options UCI), idéal pour des
duels reproductibles et indépendants de la charge de la machine.
* **`gen_openings.py`** : Le script ayant servi à générer le livre d'ouvertures
via Stockfish (fourni à titre informatif, le fichier `.epd` étant déjà inclus
dans le dépôt).

---

## Prérequis

Le moteur **Ness** en C est entièrement autonome et ne nécessite aucune
dépendance.

Cependant, pour faire tourner les scripts de test et les matchs d'évaluation
(`elo_match.py` / `compare_engines.py`), vous aurez besoin de :

1. **Un système Linux (Debian/Ubuntu recommandé)** avec **Python 3.8+** minimum.
2. **Stockfish** (nécessaire comme adversaire de référence pour les matchs Elo):
```bash
sudo apt update
sudo apt install stockfish
```
3. La bibliothèque `python3-chess` :
```bash
sudo apt install python3-chess # sous Debian/Ubuntu
pip install chess # Si vous préférez via pip
```
