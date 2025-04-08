#include "SymbolTableVisitor.h"
#include "IR.h"
#include <iostream>
#include <cstdlib>
#include <string>
#include <any>
#include <unordered_map>
#include <vector>

///////////////////////////
// VISITE DES DÉCLARATIONS
////////////////////////////

antlrcpp::Any SymbolTableVisitor::visitDecl(ifccParser::DeclContext *ctx)
{
    std::string varName = ctx->ID()->getText();

    // Vérifier uniquement dans le contexte courant (le haut de la pile)
    if (symbolStack.back().find(varName) != symbolStack.back().end()) {
        writeWarning(varName + " is already defined in this scope");
    } else {
        addToSymbolTable(varName);
    }

    if (ctx->expr() != nullptr) {
        // Marquer la variable dans le contexte courant comme initialisée
        symbolStack.back()[varName].initialised = true;
        visit(ctx->expr());
    } 

    return 0;
}

///////////////////////////
// VISITE DES AFFECTATIONS
////////////////////////////

antlrcpp::Any SymbolTableVisitor::visitAssignment(ifccParser::AssignmentContext *ctx)
{
    std::string varName = ctx->ID()->getText();
    bool found = false;
    // Chercher dans tous les contextes, du plus récent vers le plus ancien.
    for (auto it = symbolStack.rbegin(); it != symbolStack.rend(); ++it) {
        if (it->find(varName) != it->end()) {
            (*it)[varName].initialised = true;
            found = true;
            break;
        }
    }
    if (!found) {
        writeError(varName + " is not defined");
        exit(EXIT_FAILURE);
    }
    visit(ctx->expr());
    return 0;
}

///////////////////////////
// VISITE DES IDEXPR
////////////////////////////

antlrcpp::Any SymbolTableVisitor::visitIdExpr(ifccParser::IdExprContext *ctx)
{
    std::string varName = ctx->ID()->getText();
    // Rechercher la variable dans la pile, du contexte le plus récent au plus ancien.
    for (auto it = symbolStack.rbegin(); it != symbolStack.rend(); ++it) {
        if (it->find(varName) != it->end()) {
            SymbolTableStruct &info = (*it)[varName];
            if (!info.initialised) {
                writeError(varName + " is not initialised");
                exit(EXIT_FAILURE);
            }
            info.used = true;
            return info.asmOperand;  // Retourner par exemple l'opérande à utiliser en assembleur.
        }
    }
    writeError(varName + " is not defined");
    exit(EXIT_FAILURE);
    return 0;
}

//------------------------------------------------------------------------------
// VISITE DU PROGRAMME (fonction principale)
//------------------------------------------------------------------------------

antlrcpp::Any SymbolTableVisitor::visitProg(ifccParser::ProgContext *ctx) {
    // Réinitialiser la pile avec un contexte global vide
    symbolStack.clear();
    symbolStack.push_back(std::unordered_map<std::string, SymbolTableStruct>());
    offset = INTSIZE;

    // Ajout des paramètres dans le contexte global s'il y en a
    if (ctx->decl_params()) {
        for (auto param : ctx->decl_params()->param()) {
            std::string paramName = param->ID()->getText();
            addToSymbolTable(paramName);
            symbolStack.back()[paramName].initialised = true; // Les paramètres sont initialisés.
        }
    }

    // Visiter les instructions du corps du programme
    for (auto inst : ctx->inst()) {
        visit(inst);
    }

    return 0;
}

//------------------------------------------------------------------------------
// VISITE DU RETURN
//------------------------------------------------------------------------------

antlrcpp::Any SymbolTableVisitor::visitReturn_stmt(ifccParser::Return_stmtContext *ctx){
    if (ctx->expr()) {
        visit(ctx->expr());
    }

    checkSymbolTable();
    return 0;
}

//------------------------------------------------------------------------------
// AJOUT DANS LA TABLE DES SYMBOLES (contexte courant)
//------------------------------------------------------------------------------

void SymbolTableVisitor::addToSymbolTable(const std::string &s) {
    if (symbolStack.back().find(s) != symbolStack.back().end()) {
        writeWarning(s + " is already defined in this scope");
    } else {
        symbolStack.back()[s] = SymbolTableStruct{};
        symbolStack.back()[s].offset = -offset;
        offset += INTSIZE;
    }
}

//------------------------------------------------------------------------------
// GÉNÉRATION DE TEMPORAIRE
//------------------------------------------------------------------------------

std::string SymbolTableVisitor::createNewTemp() {
    std::string prefix = codegenBackend->getTempPrefix();
    std::string temp = prefix + std::to_string(offset);
    addToSymbolTable(temp);
    return temp;
}

//------------------------------------------------------------------------------
// VÉRIFICATION DE LA TABLE DES SYMBOLES
//------------------------------------------------------------------------------
// Ici, nous vérifions par exemple uniquement le contexte global (premier de la pile)
// Vous pouvez l'adapter selon vos besoins.
void SymbolTableVisitor::checkSymbolTable(){
    if (!symbolStack.empty()) {
        for (auto const& item : symbolStack.front()) {
            if (!item.second.initialised || !item.second.used) {
                writeWarning(item.first + " is defined but not used");
            }
        }
    }
}

//------------------------------------------------------------------------------
// FONCTIONS D'ALERTES
//------------------------------------------------------------------------------

void SymbolTableVisitor::writeWarning(std::string message){
    warning++;
    std::cerr << "[WARNING] " << message << "\n";
}

void SymbolTableVisitor::writeError(std::string message){
    error++;
    std::cerr << "[ERROR] " << message << "\n";
}

//------------------------------------------------------------------------------
// AFFICHAGE DE LA TABLE DES SYMBOLES (contexte global)
//------------------------------------------------------------------------------

void SymbolTableVisitor::print_symbol_table(std::ostream& os) const {
    os << "==== Symbol Table ====\n";
    if (!symbolStack.empty()) {
        for (const auto& [name, info] : symbolStack.front()) {
            os << "  " << name << " -> offset: " << info.offset << "\n";
        }
    }
    os << "======================\n";
}

//------------------------------------------------------------------------------
// GESTION DES PORTÉES : Entrée et Sortie de Scope
//------------------------------------------------------------------------------

void SymbolTableVisitor::enterScope() {
    // Créez un nouveau contexte vide et l'empilez
    symbolStack.push_back(std::unordered_map<std::string, SymbolTableStruct>());
}

void SymbolTableVisitor::exitScope() {
    // Supprimez le contexte courant (s'il existe)
    if (!symbolStack.empty()) {
        symbolStack.pop_back();
    }
}
