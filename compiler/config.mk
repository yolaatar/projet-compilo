# the ANTLRJAR part below is copied from /usr/bin/antlr4
ANTLRJAR=/Users/yolaatar/Developer/4IF/projet-compilo/antlr/jar/antlr-4.9.2-complete.jar
ANTLRINC=/Users/yolaatar/Developer/4IF/projet-compilo/antlr/include
ANTLRLIB=/usr/lib/x86_64-linux-gnu/libantlr4-runtime.a


# Chemin vers votre projet (ou utilisez un chemin relatif si vous préférez)
PROJECT_ROOT := /Users/yolaatar/Developer/4IF/projet-compilo

ANTLR = java -jar $(PROJECT_ROOT)/antlr/jar/antlr-4.9.2-complete.jar
ANTLRINC = $(PROJECT_ROOT)/antlr/include
ANTLRLIB = $(PROJECT_ROOT)/antlr/lib/libantlr4-runtime.a

