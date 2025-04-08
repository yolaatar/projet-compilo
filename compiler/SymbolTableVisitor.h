#pragma once

#include "antlr4-runtime.h"
#include "generated/ifccBaseVisitor.h"
using namespace std;

struct SymbolTableStruct
{
    bool initialised = false;
    int offset;
    bool used = false;
    string asmOperand; 
};

struct FunctionSignature
{
    std::string returnType;
    std::vector<std::string> paramsTypes;
class  SymbolTableVisitor : public ifccBaseVisitor {
	public:
        static const int INTSIZE = 4;      
        std::vector<std::unordered_map<std::string, SymbolTableStruct>> symbolStack;
;
        int offset = INTSIZE;
        int error = 0;
        int warning = 0;
        virtual antlrcpp::Any visitDecl(ifccParser::DeclContext *ctx) override;
        virtual antlrcpp::Any visitAssignment(ifccParser::AssignmentContext *ctx) override;
        virtual antlrcpp::Any visitIdExpr(ifccParser::IdExprContext *context) override;
        virtual antlrcpp::Any visitReturn_stmt(ifccParser::Return_stmtContext *ctx) override;
        virtual antlrcpp::Any visitProg(ifccParser::ProgContext *ctx) override;
        virtual antlrcpp::Any visitBlock(ifccParser::BlockContext *ctx) override;
        void addToSymbolTable(const std::string &s);
        std::string createNewTemp();
        void checkSymbolTable();
        void writeWarning(std::string message);
        void writeError(std::string message);
        void print_symbol_table(std::ostream& os = std::cerr) const;
        void exitScope();
        void enterScope();
};

class SymbolTableVisitor : public ifccBaseVisitor
{
public :

    SymbolTableVisitor();

    static const int INTSIZE = 4;
    std::map<std::string, SymbolTableStruct> symbolTable;
    std::map<std::string, FunctionSignature>* functionTable;
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
    void print_symbol_table(std::ostream &os = std::cerr) const;
};
