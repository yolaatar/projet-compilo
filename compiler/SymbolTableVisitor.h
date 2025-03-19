#pragma once


#include "antlr4-runtime.h"
#include "generated/ifccBaseVisitor.h"


class  SymbolTableVisitor : public ifccBaseVisitor {
	public:
        struct SymbolTableStruct {
            bool initialised = false;
            int offset;
            bool used = false;
        };

        static const int INTSIZE = 4;      
        std::map<std::string, SymbolTableStruct> symbolTable;
        int offset = INTSIZE;
        int error = 0;
        int warning = 0;
        virtual antlrcpp::Any visitDecl(ifccParser::DeclContext *ctx) override;
        virtual antlrcpp::Any visitAssignment(ifccParser::AssignmentContext *ctx) override;
        virtual antlrcpp::Any visitIdExpr(ifccParser::IdExprContext *context) override;
        virtual antlrcpp::Any visitReturn_stmt(ifccParser::Return_stmtContext *ctx) override;
        void addToSymbolTable(std::string s);
        void checkSymbolTable();
        void writeWarning(std::string message);
        void writeError(std::string message);
};
