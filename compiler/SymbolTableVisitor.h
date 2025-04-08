#pragma once


#include "antlr4-runtime.h"
#include "generated/ifccBaseVisitor.h"

struct SymbolTableStruct {
    bool initialised = false;
    int offset;
    bool used = false;
};

class  SymbolTableVisitor : public ifccBaseVisitor {
	public:
        static const int INTSIZE = 4;      
        std::map<std::string, SymbolTableStruct> symbolTable;
        int offset = INTSIZE;
        int error = 0;
        int warning = 0;
        virtual antlrcpp::Any visitDecl(ifccParser::DeclContext *ctx) override;
        virtual antlrcpp::Any visitAssignment(ifccParser::AssignmentContext *ctx) override;
        virtual antlrcpp::Any visitIdExpr(ifccParser::IdExprContext *context) override;
        virtual antlrcpp::Any visitReturn_stmt(ifccParser::Return_stmtContext *ctx) override;
        virtual antlrcpp::Any visitProg(ifccParser::ProgContext *ctx) override;
        void addToSymbolTable(std::string s);
        std::string createNewTemp();
        void checkSymbolTable();
        void writeWarning(std::string message);
        void writeError(std::string message);
        void print_symbol_table(std::ostream& os = std::cerr) const;
};
