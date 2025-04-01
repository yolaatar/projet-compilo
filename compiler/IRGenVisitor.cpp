#include "IRGenVisitor.h"
#include <iostream>
#include <cstdlib>
#include <string>

using namespace std;

IRGenVisitor::IRGenVisitor(SymbolTableVisitor stv){
    this->stv = stv;
}
// Supposons que IRGenVisitor possède un pointeur vers le CFG dans lequel
// on va ajouter les instructions IR, ainsi qu'une référence à la table des symboles.
  
// Méthode pour créer un nouveau temporary via le CFG (on peut redéfinir newTemp si nécessaire)
std::string IRGenVisitor::newTemp() {
    // Ici, on se repose sur CFG pour créer un temporary compatible (ex: "w0", "w1", etc.)
    return cfg->create_new_tempvar(32);
}



///////////////////////////////////////////////////////////////////////////////
// Traitement de l'instruction de retour : "return expr ;"
///////////////////////////////////////////////////////////////////////////////
antlrcpp::Any IRGenVisitor::visitReturn_stmt(ifccParser::Return_stmtContext* ctx)
{
    // Évaluer l'expression à retourner et récupérer le temporary résultant
    std::string temp = this->visit(ctx->expr()).as<std::string>();
    
    // Créer une instruction IRReturn pour indiquer la valeur de retour
    auto instr = std::make_unique<IRInstr>(
         std::make_unique<IRReturn>(temp, 32)
    );
    cfg->current_bb->add_IRInstr(std::move(instr));
    
    return temp;
}

///////////////////////////////////////////////////////////////////////////////
// Traitement d'une constante (étiquette ConstExpr)
// Dans la nouvelle grammaire : primaryExpr : CONST # ConstExpr
///////////////////////////////////////////////////////////////////////////////
antlrcpp::Any IRGenVisitor::visitConstExpr(ifccParser::ConstExprContext* ctx)
{
    int value = std::stoi(ctx->CONST()->getText());
    
    // Créer un temporary pour contenir la constante (ex : "w0")
    std::string temp = cfg->create_new_tempvar(32);
    
    // Créer une instruction IRLdConst pour charger la constante dans ce temporary
    auto instr = std::make_unique<IRInstr>(
         std::make_unique<IRLdConst>(temp, std::to_string(value), 32)
    );
    cfg->current_bb->add_IRInstr(std::move(instr));
    
    return temp;
}

///////////////////////////////////////////////////////////////////////////////
// Traitement d'une variable (étiquette IdExpr)
// Dans la nouvelle grammaire : primaryExpr : ID # IdExpr
///////////////////////////////////////////////////////////////////////////////
antlrcpp::Any IRGenVisitor::visitIdExpr(ifccParser::IdExprContext* ctx)
{
    std::string varName = ctx->ID()->getText();
    // On retourne simplement le nom de la variable (supposé être déjà dans la table des symboles)
    return varName;
}

// Implémentation minimale pour visitMoinsExpr (pour l'opérateur unaire "-")
antlrcpp::Any IRGenVisitor::visitMoinsExpr(ifccParser::MoinsExprContext* ctx)
{
    std::cerr << "visitMoinsExpr not implemented yet." << std::endl;
    return 0;
}

// Implémentation minimale pour visitMoinsExpr (pour l'opérateur unaire "-")
antlrcpp::Any IRGenVisitor::visitCompExpr(ifccParser::CompExprContext* ctx)
{
    std::cerr << "visitCompExpr not implemented yet." << std::endl;
    return 0;
}

antlrcpp::Any IRGenVisitor::visitEtLogExpr(ifccParser::EtLogExprContext* ctx)
{
    std::cerr << "visitEtLogExpr not implemented yet." << std::endl;
    return 0;
}


///////////////////////////////////////////////////////////////////////////////
// Traitement de l'expression multiplicative (MulDivExpr)
// Dans la nouvelle grammaire, expr op=('*'|'/'|'%') expr # MulDivExpr
///////////////////////////////////////////////////////////////////////////////
antlrcpp::Any IRGenVisitor::visitMulDivExpr(ifccParser::MulDivExprContext* ctx)
{
    std::string left = this->visit(ctx->expr(0)).as<std::string>();
    std::string right = this->visit(ctx->expr(1)).as<std::string>();
    std::string result = cfg->create_new_tempvar(32);
    
    if (ctx->op->getText() == "*") {
        auto instr = std::make_unique<IRInstr>(
            std::make_unique<IRMul>(result, left, right, 32)
        );
        cfg->current_bb->add_IRInstr(std::move(instr));
    } else if (ctx->op->getText() == "/") {
        auto instr = std::make_unique<IRInstr>(
            std::make_unique<IRDiv>(result, left, right, 32)
        );
        cfg->current_bb->add_IRInstr(std::move(instr));
    } else if (ctx->op->getText() == "%") {
        auto instr = std::make_unique<IRInstr>(
            std::make_unique<IRMod>(result, left, right, 32)
        );
        cfg->current_bb->add_IRInstr(std::move(instr));
    } else {
        std::cerr << "Opérateur MulDivExpr inconnu: " << ctx->op->getText() << "\n";
        exit(1);
    }
    
    return result;
}

///////////////////////////////////////////////////////////////////////////////
// visitProg : Traitement de la fonction main.
// Nouvelle grammaire : prog : 'int' 'main' '(' ')' '{' inst* return_stmt '}' ;
///////////////////////////////////////////////////////////////////////////////
antlrcpp::Any IRGenVisitor::visitProg(ifccParser::ProgContext* ctx)
{
    // Création du BasicBlock d'entrée dans le CFG
    BasicBlock* entryBB = new BasicBlock(cfg, cfg->new_BB_name());
    cfg->add_bb(entryBB);
    cfg->current_bb = entryBB;
    
    // Traiter toutes les instructions (inst) dans le corps de la fonction
    for (auto instCtx : ctx->inst()) {
        this->visit(instCtx);
    }
    
    return 0;
}


///////////////////////////////////////////////////////////////////////////////
// visitNotExpr : Traitement de l'opérateur logique "!".
// Exemple d'IR : result = !expr
///////////////////////////////////////////////////////////////////////////////
antlrcpp::Any IRGenVisitor::visitNotExpr(ifccParser::NotExprContext* ctx) {
    std::string exprTemp = this->visit(ctx->expr()).as<std::string>();
    std::string result = cfg->create_new_tempvar(32);
    
    // Créez une instruction IRNot qui calcule le NOT logique
    auto instr = std::make_unique<IRInstr>(
         std::make_unique<IRNot>(result, exprTemp, 32)
    );
    cfg->current_bb->add_IRInstr(std::move(instr));
    return result;
}

///////////////////////////////////////////////////////////////////////////////
// visitAssignment : Traitement de "ID = expr;".
///////////////////////////////////////////////////////////////////////////////
antlrcpp::Any IRGenVisitor::visitAssignment(ifccParser::AssignmentContext* ctx)
{
    std::string varName = ctx->ID()->getText();
    std::string exprTemp = this->visit(ctx->expr()).as<std::string>();
    
    // Créez une instruction IRCopy pour effectuer l'affectation : varName <- exprTemp
    auto instr = std::make_unique<IRInstr>(
         std::make_unique<IRCopy>(varName, exprTemp, 32)
    );
    cfg->current_bb->add_IRInstr(std::move(instr));
    return varName;
}

///////////////////////////////////////////////////////////////////////////////
// visitAddSubExpr : Traitement des opérateurs '+' et '-'.
// Exemple : result = expr(0) + expr(1)  ou  result = expr(0) - expr(1)
///////////////////////////////////////////////////////////////////////////////
antlrcpp::Any IRGenVisitor::visitAddSubExpr(ifccParser::AddSubExprContext* ctx)
{
    std::string left = this->visit(ctx->expr(0)).as<std::string>();
    std::string right = this->visit(ctx->expr(1)).as<std::string>();
    std::string result = cfg->create_new_tempvar(32);
    
    if (ctx->op->getText() == "+") {
       auto instr = std::make_unique<IRInstr>(
         std::make_unique<IRAdd>(result, left, right, 32)
       );
       cfg->current_bb->add_IRInstr(std::move(instr));
    } else if (ctx->op->getText() == "-") {
       auto instr = std::make_unique<IRInstr>(
         std::make_unique<IRSub>(result, left, right, 32)
       );
       cfg->current_bb->add_IRInstr(std::move(instr));
    } else {
       std::cerr << "Opérateur inconnu in AddSubExpr: " << ctx->op->getText() << "\n";
       exit(1);
    }
    return result;
}

///////////////////////////////////////////////////////////////////////////////
// visitEgalExpr : Traitement des opérateurs "==" et "!=".
///////////////////////////////////////////////////////////////////////////////
antlrcpp::Any IRGenVisitor::visitEgalExpr(ifccParser::EgalExprContext* ctx)
{
    std::string left = this->visit(ctx->expr(0)).as<std::string>();
    std::string right = this->visit(ctx->expr(1)).as<std::string>();
    std::string result = cfg->create_new_tempvar(32);
    
    if (ctx->op->getText() == "==") {
         auto instr = std::make_unique<IRInstr>(
             std::make_unique<IREgal>(result, left, right, 32)
         );
         cfg->current_bb->add_IRInstr(std::move(instr));
    } else if (ctx->op->getText() == "!=") {
         auto instr = std::make_unique<IRInstr>(
             std::make_unique<IRNotEgal>(result, left, right, 32)
         );
         cfg->current_bb->add_IRInstr(std::move(instr));
    } else {
         std::cerr << "Opérateur inconnu in EgalExpr: " << ctx->op->getText() << "\n";
         exit(1);
    }
    return result;
}

///////////////////////////////////////////////////////////////////////////////
// visitOuExcExpr : Traitement de l'opérateur XOR bit-à-bit ('^').
///////////////////////////////////////////////////////////////////////////////
antlrcpp::Any IRGenVisitor::visitOuExcExpr(ifccParser::OuExcExprContext* ctx)
{
    std::string left = this->visit(ctx->expr(0)).as<std::string>();
    std::string right = this->visit(ctx->expr(1)).as<std::string>();
    std::string result = cfg->create_new_tempvar(32);
    
    auto instr = std::make_unique<IRInstr>(
         std::make_unique<IRXor>(result, left, right, 32)
    );
    cfg->current_bb->add_IRInstr(std::move(instr));
    return result;
}

///////////////////////////////////////////////////////////////////////////////
// visitOuIncExpr : Traitement de l'opérateur OR bit-à-bit ('|').
///////////////////////////////////////////////////////////////////////////////
antlrcpp::Any IRGenVisitor::visitOuIncExpr(ifccParser::OuIncExprContext* ctx)
{
    std::string left = this->visit(ctx->expr(0)).as<std::string>();
    std::string right = this->visit(ctx->expr(1)).as<std::string>();
    std::string result = cfg->create_new_tempvar(32);
    
    auto instr = std::make_unique<IRInstr>(
         std::make_unique<IROr>(result, left, right, 32)
    );
    cfg->current_bb->add_IRInstr(std::move(instr));
    return result;
}

///////////////////////////////////////////////////////////////////////////////
// visitDecl : Traitement d'une déclaration "int ID ('=' expr)?".
///////////////////////////////////////////////////////////////////////////////
antlrcpp::Any IRGenVisitor::visitDecl(ifccParser::DeclContext* ctx)
{
    std::string varName = ctx->ID()->getText();
    if(ctx->expr() != nullptr){
         std::string temp = this->visit(ctx->expr()).as<std::string>();
         auto instr = std::make_unique<IRInstr>(
             std::make_unique<IRCopy>(varName, temp, 32)
         );
         cfg->current_bb->add_IRInstr(std::move(instr));
    } else {
         auto instr = std::make_unique<IRInstr>(
             std::make_unique<IRLdConst>(varName, "0", 32)
         );
         cfg->current_bb->add_IRInstr(std::move(instr));
    }
    return varName;
}
