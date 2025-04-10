#pragma once

#include "generated/ifccBaseVisitor.h"
#include "generated/ifccParser.h"
#include <map>
#include "IR.h"
#include "SymbolTableVisitor.h"

class IRGenVisitor : public ifccBaseVisitor {
public:
    CFG* cfg;  // Pointeur vers le CFG en cours
    CodeGenBackend* backend; // Backend pour la génération de code
    std::map<std::string, FunctionSignature>* functionTable = nullptr;
    bool hasReturned = false; // Indique si un return a été rencontré

        IRGenVisitor();
    

    virtual antlrcpp::Any visitProg(ifccParser::ProgContext *ctx) override;
    virtual antlrcpp::Any visitAxiom(ifccParser::AxiomContext *ctx) override;
    virtual antlrcpp::Any visitReturn_stmt(ifccParser::Return_stmtContext *ctx) override;
    virtual antlrcpp::Any visitMoinsExpr(ifccParser::MoinsExprContext *ctx) override;
    virtual antlrcpp::Any visitNotExpr(ifccParser::NotExprContext *ctx) override;
    virtual antlrcpp::Any visitAssignment(ifccParser::AssignmentContext *ctx) override;
    virtual antlrcpp::Any visitParExpr(ifccParser::ParExprContext *ctx) override;
    virtual antlrcpp::Any visitAddSubExpr(ifccParser::AddSubExprContext *ctx) override;
    virtual antlrcpp::Any visitMulDivExpr(ifccParser::MulDivExprContext *ctx) override;
    virtual antlrcpp::Any visitCompExpr(ifccParser::CompExprContext *ctx) override;
    virtual antlrcpp::Any visitEgalExpr(ifccParser::EgalExprContext *ctx) override;
    virtual antlrcpp::Any visitOuExcExpr(ifccParser::OuExcExprContext *ctx) override;
    virtual antlrcpp::Any visitFunction_call(ifccParser::Function_callContext *ctx) override;
    virtual antlrcpp::Any visitOuIncExpr(ifccParser::OuIncExprContext *ctx) override;
    virtual antlrcpp::Any visitIdExpr(ifccParser::IdExprContext *ctx) override;
    virtual antlrcpp::Any visitDecl(ifccParser::DeclContext *ctx) override;
    virtual antlrcpp::Any visitConstExpr(ifccParser::ConstExprContext *ctx) override;
    virtual antlrcpp::Any visitEtLogExpr(ifccParser::EtLogExprContext* ctx) override;
    virtual antlrcpp::Any visitIf_stmt(ifccParser::If_stmtContext* ctx) override;
    virtual antlrcpp::Any visitEtParExpr(ifccParser::EtParExprContext* ctx) override;
    virtual antlrcpp::Any visitOuParExpr(ifccParser::OuParExprContext* ctx) override;
    virtual antlrcpp::Any visitWhile_stmt(ifccParser::While_stmtContext *ctx) override;
    virtual antlrcpp::Any visitBlock(ifccParser::BlockContext *ctx) override;

private:
    int tempCpt = 1;
    std::string newTemp();
};
