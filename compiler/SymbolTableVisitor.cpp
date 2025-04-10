#include "SymbolTableVisitor.h"
#include "IR.h"
#include <iostream>
#include <cstdlib>
#include <string>

// ------------------------------------------------------
// Constructeur : création du scope global
// ------------------------------------------------------
SymbolTableVisitor::SymbolTableVisitor() {
    currentScope = new Scope;
    currentScope->offset = INTSIZE;  // Par exemple, 4
    currentScope->parent = nullptr;
    // functionTable doit être initialisé par l'appelant
}

// ------------------------------------------------------
// Méthode utilitaire pour accéder au scope global
// ------------------------------------------------------
Scope* SymbolTableVisitor::getGlobalScope() const {
    Scope* scope = currentScope;
    while (scope && scope->parent != nullptr) {
        scope = scope->parent;
    }
    return scope;
}

// ------------------------------------------------------
// VISITE DES AFFECTATIONS
// ------------------------------------------------------
antlrcpp::Any SymbolTableVisitor::visitAssignment(ifccParser::AssignmentContext *ctx) {
    std::string varName = ctx->ID()->getText();
    bool found = false;
    // Recherche dans la chaîne des scopes
    Scope* scope = currentScope;
    while (scope != nullptr) {
        if (scope->symbols.find(varName) != scope->symbols.end()) {
            scope->symbols[varName].initialised = true;
            found = true;
            break;
        }
        scope = scope->parent;
    }
    if (!found) {
        writeError(varName + " is not defined");
        exit(EXIT_FAILURE);
    }
    visit(ctx->expr());
    return 0;
}

// ------------------------------------------------------
// VISITE DU PROGRAMME (fonction principale)
// ------------------------------------------------------
antlrcpp::Any SymbolTableVisitor::visitProg(ifccParser::ProgContext *ctx) {
    // Le scope global est déjà créé dans le constructeur.
    currentScope->offset = INTSIZE;  // Réinitialisation si nécessaire
    
    std::string funcName = ctx->ID()->getText();
    std::string returnType = ctx->type()->getText();
    std::vector<std::string> params;

    // Ajout des paramètres dans le scope global
    if (ctx->decl_params()) {
        for (auto param : ctx->decl_params()->param()) {
            std::string paramName = param->ID()->getText();
            addToSymbolTable(paramName);
            currentScope->symbols[paramName].initialised = true; // Les paramètres sont initialisés.
            params.push_back("int");
        }
    }
    
    if (functionTable) {
        (*functionTable)[funcName] = FunctionSignature{returnType, params};
    }
    
    for (auto inst : ctx->inst()) {
        visit(inst);
    }
    
    // Affichage global pour debug
    printGlobalSymbolTable(std::cerr);
    return 0;
}

// ------------------------------------------------------
// VISITE DU RETURN
// ------------------------------------------------------
antlrcpp::Any SymbolTableVisitor::visitReturn_stmt(ifccParser::Return_stmtContext *ctx) {
    if (ctx->expr()) {
        visit(ctx->expr());
    }
    checkSymbolTable();
    return 0;
}

// ------------------------------------------------------
// AJOUT DANS LA TABLE DES SYMBOLES (scope courant)
// ------------------------------------------------------

std::string SymbolTableVisitor::addToSymbolTable(const std::string &s) {
    // Vérifier dans le scope courant si le symbole existe déjà
    if (currentScope->symbols.find(s) != currentScope->symbols.end()) {
        writeWarning(s + " is already defined in this scope");
        return currentScope->symbols[s].uniqueName; // On renvoie ce qui a été stocké (probablement "a")
    }
    // Ici, le uniqueName est simplement le nom original
    std::string uniqueName = s; // On conserve le nom
    
    // Allouer l'offset pour ce symbole dans le scope courant
    int varOffset = currentScope->offset;
    currentScope->offset += INTSIZE;
    SymbolTableStruct symbol;
    symbol.offset = -varOffset;  // Convention négative pour la pile
    symbol.uniqueName = uniqueName; // On stocke juste "a"
    symbol.initialised = false;
    symbol.used = false;
    // Insérer dans le scope courant sous la clé "s"
    currentScope->symbols[s] = symbol;
    return uniqueName;
}




// ------------------------------------------------------
// GÉNÉRATION DE TEMPORAIRE
// ------------------------------------------------------
std::string SymbolTableVisitor::createNewTemp() {
    std::string prefix = codegenBackend->getTempPrefix();
    // Utilisation du compteur d'offset du scope courant
    std::string temp = prefix + std::to_string(currentScope->offset);
    addToSymbolTable(temp);
    return temp;
}

// ------------------------------------------------------
// VÉRIFICATION DE LA TABLE DES SYMBOLES (global)
// ------------------------------------------------------
void SymbolTableVisitor::checkSymbolTable() {
    // On vérifie le scope global
    Scope* global = getGlobalScope();
    for (auto const& item : global->symbols) {
        if (!item.second.initialised || !item.second.used) {
            writeWarning(item.first + " is defined but not used");
        }
    }
}

// ------------------------------------------------------
// FONCTIONS D'ALERTES
// ------------------------------------------------------
void SymbolTableVisitor::writeWarning(const std::string &message) {
    warning++;
    std::cerr << "[WARNING] " << message << "\n";
}

void SymbolTableVisitor::writeError(const std::string &message) {
    error++;
    std::cerr << "[ERROR] " << message << "\n";
}

// ------------------------------------------------------
// GESTION DES SCOPES : Entrée et Sortie
// ------------------------------------------------------
void SymbolTableVisitor::enterScope() {
    Scope* newScope = new Scope;
    newScope->parent = currentScope;
    // On reprend l'offset du parent pour l'allocation locale.
    newScope->offset = currentScope->offset;
    currentScope = newScope;
    std::cerr << "[DEBUG] Enter new scope" << "\n";
}

void SymbolTableVisitor::exitScope() {
    Scope* oldScope = currentScope;
    if (currentScope != nullptr) {
        currentScope = currentScope->parent;
        delete oldScope;
        std::cerr << "[DEBUG] Exit scope" << "\n";
    }
}

// ------------------------------------------------------
// VISITE D'UN BLOCK (Block)
// ------------------------------------------------------
antlrcpp::Any SymbolTableVisitor::visitBlock(ifccParser::BlockContext *ctx) {
    enterScope();
    printCurrentScope(std::cerr);
    for (auto child : ctx->children) {
        this->visit(child);
    }
    exitScope();
    return 0;
}

// ------------------------------------------------------
// VISITE DES DÉCLARATIONS
// ------------------------------------------------------
antlrcpp::Any SymbolTableVisitor::visitDecl(ifccParser::DeclContext *ctx) {
    std::string varName = ctx->ID()->getText();
    // Récupérer le nom issu de addToSymbolTable, qui est tout simplement "a"
    std::string unique = addToSymbolTable(varName);
    if (ctx->expr() != nullptr) {
        currentScope->symbols[varName].initialised = true;
        visit(ctx->expr());
    }
    return unique;
}




// ------------------------------------------------------
// VISITE DES IDEXPR (Accès à une variable)
// ------------------------------------------------------
antlrcpp::Any SymbolTableVisitor::visitIdExpr(ifccParser::IdExprContext *ctx) {
    std::string varName = ctx->ID()->getText();
    Scope* scope = currentScope;
    while (scope != nullptr) {
        if (scope->symbols.find(varName) != scope->symbols.end()) {
            SymbolTableStruct &info = scope->symbols[varName];
            if (!info.initialised) {
                writeError(varName + " is not initialised");
                exit(EXIT_FAILURE);
            }
            info.used = true;
            return info.uniqueName;  // Retourne "a"
        }
        scope = scope->parent;
    }
    writeError(varName + " is not defined");
    exit(EXIT_FAILURE);
    return "";
}



// ------------------------------------------------------
// AFFICHAGE POUR DEBUG
// ------------------------------------------------------
void SymbolTableVisitor::printCurrentScope(std::ostream &os) const {
    os << "==== Current Scope Symbol Table ====\n";
    for (const auto &entry : currentScope->symbols) {
        os << "  " << entry.first 
           << " -> offset: " << entry.second.offset 
           << ", unique name: " << entry.second.uniqueName << "\n";
    }
    os << "====================================\n";
}

void SymbolTableVisitor::printGlobalSymbolTable(std::ostream &os) const {
    Scope* global = getGlobalScope();
    os << "==== Global Symbol Table ====\n";
    for (const auto &entry : global->symbols) {
        os << "  " << entry.first 
           << " -> offset: " << entry.second.offset 
           << ", unique name: " << entry.second.uniqueName << "\n";
    }
    os << "=============================\n";
}  
