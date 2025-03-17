#pragma once

#include "generated/ifccBaseVisitor.h"  // Chemin vers vos fichiers générés par ANTLR
#include "generated/ifccParser.h"
#include "SymbolInfo.h"                // Contient la définition de SymbolInfo
#include <unordered_map>
#include <string>

class CodeGenVisitor : public ifccBaseVisitor {
public:
    // Table des symboles obtenue par le SymbolTableVisitor
    std::unordered_map<std::string, SymbolInfo> symbolTable;

    // Méthodes pour générer du code assembleur
    virtual antlrcpp::Any visitProg(ifccParser::ProgContext* ctx) override;
    virtual antlrcpp::Any visitDecl(ifccParser::DeclContext* ctx) override;
    virtual antlrcpp::Any visitAssignment(ifccParser::AssignmentContext* ctx) override;
    virtual antlrcpp::Any visitReturn_stmt(ifccParser::Return_stmtContext* ctx) override;

    // Méthodes pour gérer les expressions
    virtual antlrcpp::Any visitAdditiveExpr(ifccParser::AdditiveExprContext* ctx) override;
    virtual antlrcpp::Any visitMultiplicativeExpr(ifccParser::MultiplicativeExprContext* ctx) override;
    virtual antlrcpp::Any visitConstExpr(ifccParser::ConstExprContext* ctx) override;
    virtual antlrcpp::Any visitExprVariable(ifccParser::ExprVariableContext* ctx) override;
    virtual antlrcpp::Any visitParensExpr(ifccParser::ParensExprContext* ctx) override;
};
