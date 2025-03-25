#pragma once


#include "antlr4-runtime.h"
#include "generated/ifccBaseVisitor.h"
#include "SymbolTableVisitor.h"


class  CodeGenVisitor : public ifccBaseVisitor {
	public:
        CodeGenVisitor(SymbolTableVisitor stv);

        SymbolTableVisitor stv;

        virtual antlrcpp::Any visitProg(ifccParser::ProgContext *ctx) override ;
        virtual antlrcpp::Any visitReturn_stmt(ifccParser::Return_stmtContext *ctx) override;
        virtual antlrcpp::Any visitAssignment(ifccParser::AssignmentContext *ctx) override;
        virtual antlrcpp::Any visitAddSubExpr(ifccParser::AddSubExprContext *ctx);
        virtual antlrcpp::Any visitMulDivExpr(ifccParser::MulDivExprContext *ctx);
        virtual antlrcpp::Any visitEtLogExpr(ifccParser::EtLogExprContext *ctx);
        virtual antlrcpp::Any visitCompExpr(ifccParser::CompExprContext *ctx);
        virtual antlrcpp::Any visitEgalExpr(ifccParser::EgalExprContext *ctx);
        virtual antlrcpp::Any visitOuExcExpr(ifccParser::OuExcExprContext *ctx);
        virtual antlrcpp::Any visitOuIncExpr(ifccParser::OuIncExprContext *ctx);
        virtual antlrcpp::Any visitIdExpr(ifccParser::IdExprContext *ctx);
        virtual antlrcpp::Any visitDecl(ifccParser::DeclContext *ctx) override;
        virtual antlrcpp::Any visitConstExpr(ifccParser::ConstExprContext *ctx) override;
        virtual antlrcpp::Any visitFunction_call(ifccParser::Function_callContext *ctx) override;

        private:
        int tempCpt = 1;
        std::string newTemp();
};

