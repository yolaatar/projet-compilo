#include "SymboleTableVisitor.h"

SymbolTableVisitor::~SymbolTableVisitor() {
    // Rien de particulier à faire ici
}

// Traitement de la déclaration : "int x;" ou "int x = expr;"
antlrcpp::Any SymbolTableVisitor::visitDecl(ifccParser::DeclContext* ctx) {
    std::string varName = ctx->ID()->getText();
    if (symbolTable.find(varName) != symbolTable.end()) {
        std::cerr << "Erreur : la variable " << varName << " est déclarée plusieurs fois.\n";
        errorOccurred = true;
    } else {
        symbolTable[varName] = { nextIndex, false };
        std::clog << "Déclaration de la variable " << varName
                  << " avec index " << nextIndex << "\n";
        nextIndex += 4;
    }
    // Si une initialisation est présente, visiter l'expression pour vérifier son contenu.
    if (ctx->expr() != nullptr) {
        this->visit(ctx->expr());
    }
    return 0;
}

// Traitement de l'affectation : "x = expr;"
antlrcpp::Any SymbolTableVisitor::visitAssignment(ifccParser::AssignmentContext* ctx) {
    std::string varName = ctx->ID()->getText();
    if (symbolTable.find(varName) == symbolTable.end()) {
        std::cerr << "Erreur : la variable " << varName << " n'est pas déclarée.\n";
        errorOccurred = true;
    } else {
        symbolTable[varName].used = true;
    }
    // Vérifier l'expression à droite
    this->visit(ctx->expr());
    return 0;
}

// Traitement d'une expression variable
antlrcpp::Any SymbolTableVisitor::visitExprVariable(ifccParser::ExprVariableContext* ctx) {
    std::string varName = ctx->ID()->getText();
    if (symbolTable.find(varName) == symbolTable.end()) {
        std::cerr << "Erreur : la variable " << varName << " utilisée n'a pas été déclarée.\n";
        errorOccurred = true;
    } else {
        symbolTable[varName].used = true;
    }
    return 0;
}

// Vérification des variables non utilisées
void SymbolTableVisitor::checkUnusedVariables() {
    for (const auto& kv : symbolTable) {
        if (!kv.second.used) {
            std::cerr << "Avertissement : la variable " << kv.first << " est déclarée mais jamais utilisée.\n";
        }
    }
}
