#pragma once

#include "generated/ifccBaseVisitor.h"
#include "generated/ifccParser.h"
#include "SymbolInfo.h"
#include <iostream>
#include <unordered_map>
#include <string>

class SymbolTableVisitor : public ifccBaseVisitor {
public:
    // Table des symboles : nom de variable -> informations associées
    std::unordered_map<std::string, SymbolInfo> symbolTable;
    int nextIndex = 0;
    bool errorOccurred = false;

    // Destructeur virtuel
    virtual ~SymbolTableVisitor();

    // Visiteur pour une déclaration de variable (règle decl)
    virtual antlrcpp::Any visitDecl(ifccParser::DeclContext* ctx) override;

    // Visiteur pour une affectation (règle assignment)
    virtual antlrcpp::Any visitAssignment(ifccParser::AssignmentContext* ctx) override;

    // Visiteur pour une expression variable (alternative étiquetée #ExprVariable)
    virtual antlrcpp::Any visitExprVariable(ifccParser::ExprVariableContext* ctx) override;

    // Vérifie que chaque variable déclarée est utilisée
    void checkUnusedVariables();
};
