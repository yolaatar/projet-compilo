#pragma once

#include "antlr4-runtime.h"
#include "generated/ifccBaseVisitor.h"
#include <string>
#include <unordered_map>
#include <vector>
#include <map>
#include <iostream>

// Structure représentant une entrée de la table des symboles
struct SymbolTableStruct {
    bool initialised = false;
    int offset;
    bool used = false;
    std::string uniqueName; 
};

// Structure représentant la signature d'une fonction
struct FunctionSignature {
    std::string returnType;
    std::vector<std::string> paramsTypes;
};

// Structure représentant un scope
struct Scope {
    std::unordered_map<std::string, SymbolTableStruct> symbols;
    int offset;
    int level;       // Niveau du scope (1 pour global, 2 pour un bloc interne, etc.)
    Scope* parent;
};


class SymbolTableVisitor : public ifccBaseVisitor {
public:
    static const int INTSIZE = 4;
    
    // Pointeur vers le scope courant
    Scope* currentScope;
    // Table des fonctions (à initialiser ou passer en paramètre)
    std::map<std::string, FunctionSignature>* functionTable;
    
    int error = 0;
    int warning = 0;
    int tempSuffixCounter = 1;
    
    SymbolTableVisitor();
    
    // Méthodes de visite
    virtual antlrcpp::Any visitDecl(ifccParser::DeclContext *ctx) override;
    virtual antlrcpp::Any visitAssignment(ifccParser::AssignmentContext *ctx) override;
    virtual antlrcpp::Any visitIdExpr(ifccParser::IdExprContext *ctx) override;
    virtual antlrcpp::Any visitReturn_stmt(ifccParser::Return_stmtContext *ctx) override;
    virtual antlrcpp::Any visitProg(ifccParser::ProgContext *ctx) override;
    virtual antlrcpp::Any visitBlock(ifccParser::BlockContext *ctx) override;
    
    // Ajoute une variable dans le scope courant et renvoie son nom unique
    std::string addToSymbolTable(const std::string &s);
    
    // Méthode pour créer un temporaire (basé sur le compteur d'offset du scope courant)
    std::string createNewTemp();
    
    // Vérification de la table (par exemple pour le scope global)
    void checkSymbolTable();
    
    // Fonctions d'alertes
    void writeWarning(const std::string &message);
    void writeError(const std::string &message);
    
    // Gestion des scopes
    void enterScope();
    void exitScope();
    
    // Affichage pour debug
    void printCurrentScope(std::ostream &os = std::cerr) const;
    void printGlobalSymbolTable(std::ostream &os = std::cerr) const;
    
    // Méthode utilitaire pour accéder au scope global
    Scope* getGlobalScope() const;
    int getScopeLevel(Scope* scope) const;
};
