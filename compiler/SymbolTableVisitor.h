
#pragma once

#include "generated/ifccBaseVisitor.h"  // Chemin vers vos fichiers générés par ANTLR
#include "generated/ifccParser.h"
#include <unordered_map>
#include <string>
#include <map>
#include <iostream>

// Structure représentant une entrée de la table des symboles
struct SymbolTableStruct {
    bool initialised = false;
    int offset;
    bool used = false;
    std::string uniqueName; 
};


// Structure représentant un scope
struct Scope {
    std::unordered_map<std::string, SymbolTableStruct> symbols; // clé = nom d'origine
    int offset;
    int level;       // Niveau du scope (1 pour global, 2 pour interne, etc.)
    Scope* parent;
};

struct FunctionSignature {
    std::string returnType;
    std::vector<std::string> paramsTypes;
};

class SymbolTableVisitor : public ifccBaseVisitor {
public:
    static const int INTSIZE = 4;

    int tempSuffixCounter; 

    Scope* currentScope;
    std::map<std::string, FunctionSignature>* functionTable;

    int error = 0;
    int warning = 0;
    

    SymbolTableVisitor();

    virtual antlrcpp::Any visitDecl(ifccParser::DeclContext *ctx) override;

    virtual antlrcpp::Any visitAssign(ifccParser::AssignContext *ctx) override;
    virtual antlrcpp::Any visitPlusAssign(ifccParser::PlusAssignContext *ctx) override;
    virtual antlrcpp::Any visitMinusAssign(ifccParser::MinusAssignContext *ctx) override;
    virtual antlrcpp::Any visitMulAssign(ifccParser::MulAssignContext *ctx) override;
    virtual antlrcpp::Any visitDivAssign(ifccParser::DivAssignContext *ctx) override;

    
    virtual antlrcpp::Any visitIdExpr(ifccParser::IdExprContext *ctx) override;
    virtual antlrcpp::Any visitReturn_stmt(ifccParser::Return_stmtContext *ctx) override;
    virtual antlrcpp::Any visitProg(ifccParser::ProgContext *ctx) override;
    virtual antlrcpp::Any visitBlock(ifccParser::BlockContext *ctx) override;

    std::string addToSymbolTable(const std::string &s);
    std::string getUniqueName(const std::string &s) const;
    int getScopeLevel(Scope* scope) const;
    std::string createNewTemp();
    void checkSymbolTable();

    void writeWarning(const std::string &message);
    void writeError(const std::string &message);

    void enterScope();
    void exitScope();

    void printCurrentScope(std::ostream &os = std::cerr) const;
    void printGlobalSymbolTable(std::ostream &os = std::cerr) const;
    
    Scope *getGlobalScope() const;

private:
    // Table pour conserver les symboles des scopes quittés
    std::unordered_map<std::string, SymbolTableStruct> aggregatedSymbols;

    antlrcpp::Any checkAssign(const std::string &varName, ifccParser::ExprContext *exprCtx);
};