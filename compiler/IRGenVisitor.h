#pragma once

#include "generated/ifccBaseVisitor.h"  // Chemin vers vos fichiers générés par ANTLR
#include "generated/ifccParser.h"
#include <unordered_map>
#include <string>
#include "IR.h"
#include "SymbolTableVisitor.h"

class  IRGenVisitor : public ifccBaseVisitor {
	public:
        CFG* cfg;  // Pointeur vers le CFG en cours 
        CodeGenBackend *backend; // Backend pour la génération de code
        SymbolTableVisitor stv;

        virtual antlrcpp::Any visitProg(ifccParser::ProgContext *ctx) override ;
        virtual antlrcpp::Any visitReturn_stmt(ifccParser::Return_stmtContext *ctx) override;
        virtual antlrcpp::Any visitMoinsExpr(ifccParser::MoinsExprContext *ctx) override;
        virtual antlrcpp::Any visitNotExpr(ifccParser::NotExprContext *ctx) override;
        virtual antlrcpp::Any visitAssignment(ifccParser::AssignmentContext *ctx) override;
        virtual antlrcpp::Any visitParExpr(ifccParser::ParExprContext *ctx) override;
        virtual antlrcpp::Any visitAddSubExpr(ifccParser::AddSubExprContext *ctx);
        virtual antlrcpp::Any visitMulDivExpr(ifccParser::MulDivExprContext *ctx);
        virtual antlrcpp::Any visitCompExpr(ifccParser::CompExprContext *ctx);
        virtual antlrcpp::Any visitEgalExpr(ifccParser::EgalExprContext *ctx);
        virtual antlrcpp::Any visitOuExcExpr(ifccParser::OuExcExprContext *ctx);
        virtual antlrcpp::Any visitOuIncExpr(ifccParser::OuIncExprContext *ctx);
        virtual antlrcpp::Any visitIdExpr(ifccParser::IdExprContext *ctx);
        virtual antlrcpp::Any visitDecl(ifccParser::DeclContext *ctx) override;
        virtual antlrcpp::Any visitConstExpr(ifccParser::ConstExprContext *ctx) override;
        virtual antlrcpp::Any visitEtLogExpr(ifccParser::EtLogExprContext* ctx) override;
        virtual antlrcpp::Any visitIf_stmt(ifccParser::If_stmtContext* ctx) override;
        virtual antlrcpp::Any visitWhile_stmt(ifccParser::While_stmtContext* ctx) override;

        private:
        int tempCpt = 1;
        std::string newTemp();
};

