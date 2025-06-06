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
        std::map<std::string, FunctionSignature>* functionTable = nullptr;
        bool hasReturned = false; // Indique si une instruction de retour a été rencontrée
        SymbolTableVisitor stv;
        IRGenVisitor();

        virtual antlrcpp::Any visitProg(ifccParser::ProgContext *ctx) override ;
        virtual antlrcpp::Any visitAxiom(ifccParser::AxiomContext *ctx) override ;
        virtual antlrcpp::Any visitReturn_stmt(ifccParser::Return_stmtContext *ctx) override;
        virtual antlrcpp::Any visitMoinsExpr(ifccParser::MoinsExprContext *ctx) override;
        virtual antlrcpp::Any visitNotExpr(ifccParser::NotExprContext *ctx) override;

        virtual antlrcpp::Any visitAssign(ifccParser::AssignContext *ctx) override;
        virtual antlrcpp::Any visitPlusAssign(ifccParser::PlusAssignContext *ctx) override;
        virtual antlrcpp::Any visitMinusAssign(ifccParser::MinusAssignContext *ctx) override;
        virtual antlrcpp::Any visitMulAssign(ifccParser::MulAssignContext *ctx) override;
        virtual antlrcpp::Any visitDivAssign(ifccParser::DivAssignContext *ctx) override;

        virtual antlrcpp::Any visitParExpr(ifccParser::ParExprContext *ctx) override;
        virtual antlrcpp::Any visitAddSubExpr(ifccParser::AddSubExprContext *ctx);
        virtual antlrcpp::Any visitMulDivExpr(ifccParser::MulDivExprContext *ctx);
        virtual antlrcpp::Any visitCompExpr(ifccParser::CompExprContext *ctx);
        virtual antlrcpp::Any visitEgalExpr(ifccParser::EgalExprContext *ctx);
        virtual antlrcpp::Any visitOuExcExpr(ifccParser::OuExcExprContext *ctx);
        virtual antlrcpp::Any visitFunction_call(ifccParser::Function_callContext *ctx);
        virtual antlrcpp::Any visitOuIncExpr(ifccParser::OuIncExprContext *ctx);
        virtual antlrcpp::Any visitIdExpr(ifccParser::IdExprContext *ctx);
        virtual antlrcpp::Any visitDecl(ifccParser::DeclContext *ctx) override;
        virtual antlrcpp::Any visitConstExpr(ifccParser::ConstExprContext *ctx) override;
        virtual antlrcpp::Any visitEtLogExpr(ifccParser::EtLogExprContext* ctx) override;
        virtual antlrcpp::Any visitIf_stmt(ifccParser::If_stmtContext* ctx) override;
   
        virtual antlrcpp::Any visitEtParExpr(ifccParser::EtParExprContext* ctx) override;
        virtual antlrcpp::Any visitOuParExpr(ifccParser::OuParExprContext* ctx) override;
        virtual antlrcpp::Any visitWhile_stmt(ifccParser::While_stmtContext *ctx) override;
        virtual antlrcpp::Any visitBlock(ifccParser::BlockContext *ctx) override;
        virtual antlrcpp::Any visitCharExpr(ifccParser::CharExprContext *ctx) override;

        private:
        int tempCpt = 1;
        std::string newTemp();

        antlrcpp::Any generateCompoundAssign(const std::string& varName, ifccParser::ExprContext* expr, const std::string& op);
};

