#include "SymbolTableVisitor.h"
#include "IR.h"  // Pour INTSIZE et éventuelles dépendances
#include <iostream>
#include <cstdlib>

SymbolTableVisitor::SymbolTableVisitor() {
    currentScope = new Scope;
    currentScope->offset = INTSIZE;  // Par exemple, 4 octets
    currentScope->parent = nullptr;
    currentScope->level = 1;         // Scope global = niveau 1
    functionTable = nullptr;
    tempSuffixCounter = 1;
}

int SymbolTableVisitor::getScopeLevel(Scope* scope) const {
    return scope ? scope->level : 0;
}


// Renvoie le scope global (le plus haut niveau)
Scope* SymbolTableVisitor::getGlobalScope() const {
    Scope* scope = currentScope;
    while (scope && scope->parent != nullptr) {
        scope = scope->parent;
    }
    return scope;
}

std::string SymbolTableVisitor::addToSymbolTable(const std::string &s) {
    // Pour les temporaires dont le nom commence par '!'
    if (!s.empty() && s[0] == '!') {
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
        std::cerr << "[DEBUG] Added temporary: " << s << "\n";
        return s;
    }
    // Pour une variable utilisateur, la clé est le nom d'origine
    if (currentScope->symbols.find(s) != currentScope->symbols.end()) {
        writeWarning(s + " is already defined in this scope");
        return currentScope->symbols[s].uniqueName;
    }
    int level = getScopeLevel(currentScope);  // Par exemple, 1 pour global, 2 pour un bloc interne
    std::string uniqueName = "s" + std::to_string(level) + "_" + s;
    
    int varOffset = currentScope->offset;
    currentScope->offset += INTSIZE;
    
    SymbolTableStruct symbol;
    symbol.offset = -varOffset;
    symbol.uniqueName = uniqueName;
    symbol.initialised = false;
    symbol.used = false;
    
    currentScope->symbols[s] = symbol;
    std::cerr << "[DEBUG] Added variable: " << s << " -> " << uniqueName << " in scope level " << level << "\n";
    return uniqueName;
}

std::string SymbolTableVisitor::getUniqueName(const std::string &s) const {
    Scope* scope = currentScope;
    while (scope != nullptr) {
        auto it = scope->symbols.find(s);
        if (it != scope->symbols.end()) {
            std::cerr << "[DEBUG] getUniqueName: found \"" << s << "\" -> " 
                      << it->second.uniqueName << " in scope level " << getScopeLevel(scope) << "\n";
            return it->second.uniqueName;
        }
        scope = scope->parent;
    }
    // Si non trouvé dans les scopes actifs, consulter aggregatedSymbols.
    auto itGlobal = aggregatedSymbols.find(s);
    if (itGlobal != aggregatedSymbols.end()) {
        std::cerr << "[DEBUG] getUniqueName (from aggregated): found \"" << s << "\" -> " 
                  << itGlobal->second.uniqueName << "\n";
        return itGlobal->second.uniqueName;
    }
    std::cerr << "[ERROR] getUniqueName: \"" << s << "\" is not defined\n";
    exit(EXIT_FAILURE);
    return "";
}
std::string SymbolTableVisitor::createNewTemp() {
    Scope* global = getGlobalScope();
    std::string temp = "!tmp" + std::to_string(tempSuffixCounter++);
    
    // Ajouter au scope GLOBAL
    int varOffset = global->offset;
    global->offset += INTSIZE;
    
    SymbolTableStruct symbol;
    symbol.offset = -varOffset;
    symbol.uniqueName = temp;
    symbol.initialised = true;
    symbol.used = true;
    
    global->symbols[temp] = symbol; // Clé = uniqueName
    return temp;
}

void SymbolTableVisitor::checkSymbolTable() {
    Scope* global = getGlobalScope();
    for (auto const& item : global->symbols) {
        if (!item.second.initialised || !item.second.used) {
            writeWarning(item.first + " is defined but not used");
        }
    }
}

void SymbolTableVisitor::writeWarning(const std::string &message) {
    warning++;
    std::cerr << "[WARNING] " << message << "\n";
}

void SymbolTableVisitor::writeError(const std::string &message) {
    error++;
    std::cerr << "[ERROR] " << message << "\n";
}

void SymbolTableVisitor::enterScope() {
    Scope* newScope = new Scope;
    newScope->parent = currentScope;
    newScope->offset = currentScope->offset;
    newScope->level = currentScope->level + 1;
    currentScope = newScope;
    std::cerr << "[DEBUG] Enter new scope (level " << newScope->level << ")" << "\n";
}

void SymbolTableVisitor::exitScope() {
    Scope* oldScope = currentScope;
    currentScope = currentScope->parent;
    delete oldScope; // Les symboles locaux sont supprimés
}


antlrcpp::Any SymbolTableVisitor::visitAssignment(ifccParser::AssignmentContext *ctx) {
    std::string varName = ctx->ID()->getText();
    bool found = false;
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

antlrcpp::Any SymbolTableVisitor::visitProg(ifccParser::ProgContext *ctx) {
    currentScope->offset = INTSIZE;  // Réinitialisation éventuelle du global
    
    std::string funcName = ctx->ID()->getText();
    std::string returnType = ctx->type()->getText();
    std::vector<std::string> params;

    if (ctx->decl_params()) {
        for (auto param : ctx->decl_params()->param()) {
            std::string paramName = param->ID()->getText();
            addToSymbolTable(paramName);
            currentScope->symbols[paramName].initialised = true;
            params.push_back("int");
        }
    }
    
    if (functionTable) {
        (*functionTable)[funcName] = FunctionSignature{returnType, params};
    }
    
    for (auto inst : ctx->inst()) {
        visit(inst);
    }
    
    printGlobalSymbolTable(std::cerr);
    return 0;
}

antlrcpp::Any SymbolTableVisitor::visitReturn_stmt(ifccParser::Return_stmtContext *ctx) {
    if (ctx->expr())
        visit(ctx->expr());
    checkSymbolTable();
    return 0;
}

antlrcpp::Any SymbolTableVisitor::visitBlock(ifccParser::BlockContext *ctx) {
    enterScope(); // Nouveau scope interne (niveau 2)
    printCurrentScope(std::cerr);
    for (auto child : ctx->children) {
        this->visit(child);  // Génère l'IR dans le scope interne
    }
    // Ici, avant exitScope(), le code du bloc interne utilise les symboles du niveau 2 (par ex. s2_a)
    exitScope();
    return 0;
}


antlrcpp::Any SymbolTableVisitor::visitDecl(ifccParser::DeclContext *ctx) {
    std::string varName = ctx->ID()->getText();
    std::string unique = addToSymbolTable(varName); // Par exemple, "s1_a" ou "s2_a"
    if (ctx->expr() != nullptr) {
        currentScope->symbols[varName].initialised = true;  // Inscrit dans le scope courant (clé = nom original)
        visit(ctx->expr());
    }
    return unique;
}

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
            return info.uniqueName;  // Par exemple "s1_a" ou "s2_a"
        }
        scope = scope->parent;
    }
    writeError(varName + " is not defined");
    exit(EXIT_FAILURE);
    return "";
}

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
