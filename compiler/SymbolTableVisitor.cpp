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
    currentScope->level = 1; 
    // functionTable doit être initialisé par l'appelant
}

// ------------------------------------------------------
// Méthode utilitaire pour accéder au scope global
// ------------------------------------------------------
int SymbolTableVisitor::getScopeLevel(Scope* scope) const {
    int level = 0;
    while (scope != nullptr) {
        level++;
        scope = scope->parent;
    }
    return level;
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
    // Si le nom commence par '!', c'est un temporaire : on le conserve tel quel.
    if (!s.empty() && s[0] == '!') {
        // Vérifier s'il existe déjà dans le scope courant
        if (currentScope->symbols.find(s) != currentScope->symbols.end()) {
            writeWarning(s + " is already defined in this scope");
            return currentScope->symbols[s].uniqueName;
        }
        int varOffset = currentScope->offset;
        currentScope->offset += INTSIZE;
        SymbolTableStruct symbol;
        symbol.offset = -varOffset;
        symbol.uniqueName = s;
        symbol.initialised = false;
        symbol.used = false;
        currentScope->symbols[s] = symbol;
        return s;
    }
    
    // Pour les autres noms (variables déclarées par l'utilisateur), on ajoute le préfixe indiquant le niveau de scope.
    int level = getScopeLevel(currentScope); // Par exemple, global = 1, interne = 2, etc.
    std::string uniqueName = "s" + std::to_string(level) + "_" + s;
    
    // Vérifier si le symbole existe déjà dans le scope courant (en utilisant uniqueName comme clé)
    if (currentScope->symbols.find(uniqueName) != currentScope->symbols.end()) {
        writeWarning(s + " is already defined in this scope");
        return currentScope->symbols[uniqueName].uniqueName;
    }
    
    int varOffset = currentScope->offset;
    currentScope->offset += INTSIZE;
    
    SymbolTableStruct symbol;
    symbol.offset = -varOffset;  // Convention négative pour la pile
    symbol.uniqueName = uniqueName;
    symbol.initialised = false;
    symbol.used = false;
    
    // Stocker dans le scope courant sous la clé unique (par exemple "s1_a")
    currentScope->symbols[uniqueName] = symbol;
    return uniqueName;
}





// ------------------------------------------------------
// GÉNÉRATION DE TEMPORAIRE
// ------------------------------------------------------
std::string SymbolTableVisitor::createNewTemp() {
    std::string prefix = codegenBackend->getTempPrefix();
    std::string temp = prefix + std::to_string(currentScope->offset);
    return addToSymbolTable(temp);
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
    std::string unique = addToSymbolTable(varName); // unique sera par ex. "s1_a" ou "s2_a"
    if (ctx->expr() != nullptr) {
        // Marquer comme initialisée dans le scope courant (avec clé unique)
        currentScope->symbols[unique].initialised = true;
        visit(ctx->expr());
    }
    return unique;
}




// ------------------------------------------------------
// VISITE DES IDEXPR (Accès à une variable)
// ------------------------------------------------------
antlrcpp::Any SymbolTableVisitor::visitIdExpr(ifccParser::IdExprContext *ctx) {
    std::string varName = ctx->ID()->getText();
    // Recherche par clé. Comme on stocke avec uniqueName, la clé doit être "sX_a".
    // Il faut parcourir tous les scopes et comparer le nom de base dans le uniqueName.
    Scope* scope = currentScope;
    while (scope != nullptr) {
        for (const auto &entry : scope->symbols) {
            // Si le uniqueName se termine par "_" + varName, on considère que c'est la bonne variable.
            // (Supposons que le format soit "sX_a" et on cherche "a")
            if (entry.second.uniqueName.size() >= varName.size() &&
                entry.second.uniqueName.substr(entry.second.uniqueName.size() - varName.size()) == varName) {
                SymbolTableStruct &info = scope->symbols[entry.first];
                if (!info.initialised) {
                    writeError(varName + " is not initialised");
                    exit(EXIT_FAILURE);
                }
                info.used = true;
                return info.uniqueName;
            }
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
// Renvoie le scope global (le plus haut niveau de la hiérarchie)
Scope* SymbolTableVisitor::getGlobalScope() const {
    Scope* scope = currentScope;
    while (scope && scope->parent != nullptr) {
        scope = scope->parent;
    }
    return scope;
}

