# Livres d'ouverture

Ness peut utiliser un livre d'ouverture de deux manières :
1. **Intégré au moteur (Recommandé pour l'autonomie)** : Activé directement à la compilation (`make BOOK=1`), le moteur interroge lui-même le fichier `.bin` avant de lancer sa recherche.
2. **Via l'interface externe** : Géré par `lichess-bot` ou `cutechess` (le moteur joue sans livre interne).

## Téléchargement

```sh
cd books
curl -sL -o komodo.bin https://raw.githubusercontent.com/michaeldv/donna_opening_books/master/komodo.bin
```

D'autres livres (`gm2001`, `rodent`...) sont disponibles dans le même dépôt :
https://github.com/michaeldv/donna_opening_books

## Utilisation

### 1. Utilisation native par le moteur (Intégré)
Pour compiler le moteur avec le support du livre d'ouverture :

```sh
make clean && make BOOK=1
```

Par défaut, le moteur cherchera le livre dans `books/komodo.bin`. Tu peux modifier ce chemin par défaut à la compilation ou spécifier un autre fichier au moment de lancer ton exécutable via une variable d'environnement :

```sh
NESS_BOOK=books/gm2001.bin ./chess
```

### 2. Utilisation via une interface externe (Lichess ou Cutechess)
Si tu n'as pas compilé avec `BOOK=1`, tu peux déléguer la gestion du livre à l'interface :

**lichess-bot**, dans `config.yml` :
```yaml
engine:
  polyglot:
    enabled: true
    book:
      standard:
        - /chemin/vers/books/komodo.bin
```

**cutechess-cli** :
```sh
cutechess-cli -engine cmd=./chess -book books/komodo.bin -bookdepth 8 ...
```

`komodo.bin` peut être remplacé par n'importe quel autre livre Polyglot. Pour mesurer la force brute de ton moteur sans aucune aide théorique, compile sans l'option `BOOK=1` et joue sans livre dans l'interface.
