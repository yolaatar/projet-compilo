#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>

#include "antlr4-runtime.h"
#include "generated/ifccLexer.h"
#include "generated/ifccParser.h"
#include "generated/ifccBaseVisitor.h"

#include "CodeGenVisitor.h"
#include "SymboleTableVisitor.h"

using namespace antlr4;
using namespace std;

int main(int argn, const char **argv)
{
    stringstream in;
    if (argn == 2)
    {
        ifstream lecture(argv[1]);
        if (!lecture.good())
        {
            cerr << "error: cannot read file: " << argv[1] << endl;
            exit(1);
        }
        in << lecture.rdbuf();
    }
    else
    {
        cerr << "usage: ifcc path/to/file.c" << endl;
        exit(1);
    }

    ANTLRInputStream input(in.str());

    ifccLexer lexer(&input);
    CommonTokenStream tokens(&lexer);

    tokens.fill();

    ifccParser parser(&tokens);
    tree::ParseTree *tree = parser.axiom();

    if (parser.getNumberOfSyntaxErrors() != 0)
    {
        cerr << "error: syntax error during parsing" << endl;
        exit(1);
    }

    // Après l'analyse statique :
    SymbolTableVisitor symVisitor;
    symVisitor.visit(tree);
    symVisitor.checkUnusedVariables();
    if (symVisitor.errorOccurred)
    {
        std::cerr << "Erreurs détectées dans la table des symboles. Abandon de la compilation.\n";
        return 1;
    }

    // Transfert de la table des symboles au générateur de code
    CodeGenVisitor codeGenVisitor;
    codeGenVisitor.symbolTable = symVisitor.symbolTable; // ASSUREZ-VOUS DE FAIRE CE TRANSFERT

    codeGenVisitor.visit(tree);

    return 0;
}
