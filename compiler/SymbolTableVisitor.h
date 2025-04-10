#pragma once

#include "antlr4-runtime.h"
#include "generated/ifccBaseVisitor.h"  // Adaptez le chemin selon votre projet
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
    // Le nom unique à utiliser dans l’IR ou pour la génération d’assembleur.
    std::string uniqueName;
};

// Structure représentant la signature d'une fonction
struct FunctionSignature {
    std::string returnType;
    std::vector<std::string> paramsTypes;
};

// Structure représentant un scope
struct Scope {
    // Nous utilisons ici la clé originale pour insérer les symboles.
    // Cela permettra à la recherche (visitIdExpr) de retrouver le symbole par son nom d'origine.
    std::unordered_map<std::string, SymbolTableStruct> symbols;
    int offset;
    // Pour générer un préfixe indiquant le niveau, on peut conserver le niveau ici.
    int level;
    Scope* parent;
};

class SymbolTableVisitor : public ifccBaseVisitor {
public:
    static const int INTSIZE = 4;
    
    // Le scope courant.
    Scope* currentScope;
    // Table des fonctions (peut être assignée par l'appelant)
    std::map<std::string, FunctionSignature>* functionTable = nullptr;
    
    int error = 0;
    int warning = 0;
    // Si vous utilisez des suffixes pour les temporaires, vous pouvez ici initialiser à 1
    int tempSuffixCounter = 1;
    
    SymbolTableVisitor();

    // Méthode utilitaire pour accéder au scope global.
    Scope* getGlobalScope() const;
    // Renvoie le niveau d'un scope (global = 1, interne = 2, etc.)
    int getScopeLevel(Scope* scope) const;

    // Méthodes de visite (overrides)
    virtual antlrcpp::Any visitDecl(ifccParser::DeclContext *ctx) override;
    virtual antlrcpp::Any visitAssignment(ifccParser::AssignmentContext *ctx) override;
    virtual antlrcpp::Any visitIdExpr(ifccParser::IdExprContext *ctx) override;
    virtual antlrcpp::Any visitReturn_stmt(ifccParser::Return_stmtContext *ctx) override;
    virtual antlrcpp::Any visitProg(ifccParser::ProgContext *ctx) override;
    virtual antlrcpp::Any visitBlock(ifccParser::BlockContext *ctx) override;

    // Ajoute une variable dans le scope courant et renvoie son nom unique
    // Pour les variables utilisateur, on utilise la clé originale et on stocke dans uniqueName le nom préfixé (ex. "s1_a").
    // Pour les temporaires (commençant par '!'), on conserve leur nom.
    std::string addToSymbolTable(const std::string &s);

    // Crée un temporaire (pour IR) basé sur le compteur d'offset du scope courant.
    std::string createNewTemp();

    // Vérification de la table (par exemple pour le scope global)
    void checkSymbolTable();

    // Fonctions d'alerte
    void writeWarning(const std::string &message);
    void writeError(const std::string &message);

    // Gestion des scopes
    void enterScope();
    void exitScope();

    // Affichage pour débogage
    void printCurrentScope(std::ostream &os = std::cerr) const;
    void printGlobalSymbolTable(std::ostream &os = std::cerr) const;
};
