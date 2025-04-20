Hexanôme 4222 composé de Nabil DAKKOUNE, Eléonore DUGAST, Youssef LAATAR, Morgane NAIBO, Ghita BAKHAT, Evan CAPGRAS, Martin LAVALLÉE, Ksenija BESER

# IFCC - Mini Compilateur C

Ce projet implémente un compilateur simplifié pour un sous-ensemble du langage C. Il prend en entrée un fichier `.c`, génère une représentation intermédiaire (IR), puis du code assembleur pour les architectures **x86** et **ARM64**.

## 📁 Structure du projet

- `main.cpp` : point d'entrée du compilateur
- `IR.h / IR.cpp` : représentation intermédiaire (IR) et gestion des blocs de base (CFG)
- `IRGenVisitor.cpp` : génération de l'IR depuis l'AST
- `SymbolTableVisitor.cpp` : analyse sémantique, gestion des symboles et des portées
- `X86Backend.cpp`, `ARM64Backend.cpp` : génération de code assembleur pour les architectures cibles
- `ifcc.g4` : grammaire ANTLR pour le langage source
- `generated/` : fichiers générés par ANTLR (parser, lexer, visitors)
- `build/` : fichiers objets (.o) et dépendances (.d)
- `config.mk` : configuration système locale (chemins vers ANTLR, etc.)

## ⚙️ Compilation

1. Installe ANTLR 4 (voir [site officiel](https://www.antlr.org/)) et ajoute le jar au `config.mk`.
2. Crée un fichier `config.mk` à la racine du projet (non versionné) contenant :

```makefile
ANTLR=java -jar /path/to/antlr-4.x-complete.jar
ANTLRJAR=/path/to/antlr-4.x-complete.jar
ANTLRLIB=/usr/local/lib/libantlr4-runtime.a
ANTLRINC=/usr/local/include/antlr4-runtime/
