#include "IRGenVisitor.h"
#include "IRInstr.h"
#include <iostream>
#include <cstdlib>
#include <string>
#include <any>

using namespace std;

///////////////////////////////////////////////////////////////////////////////
// Traitement de l'instruction de retour : "return expr ;"
///////////////////////////////////////////////////////////////////////////////
antlrcpp::Any IRGenVisitor::visitReturn_stmt(ifccParser::Return_stmtContext* ctx)
{
    std::string temp = std::any_cast<std::string>(this->visit(ctx->expr()));
    BasicBlock *bb = cfg->current_bb;
    auto instr = std::make_unique<IRReturn>(bb, temp);
    bb->add_IRInstr(std::move(instr));
    return temp;
}

///////////////////////////////////////////////////////////////////////////////
// visitDecl : Traitement d'une déclaration "int ID ('=' expr)?".
///////////////////////////////////////////////////////////////////////////////
antlrcpp::Any IRGenVisitor::visitDecl(ifccParser::DeclContext* ctx)
{
    std::string varName = ctx->ID()->getText(); 
    BasicBlock *bb = cfg->current_bb;
    if(ctx->expr() != nullptr){
         std::string temp = std::any_cast<std::string>(this->visit(ctx->expr()));
         auto instr = std::make_unique<IRCopy>(bb, varName, temp);
         cfg->current_bb->add_IRInstr(std::move(instr));
    } else {
         auto instr = std::make_unique<IRLdConst>(bb, varName, "0");
         cfg->current_bb->add_IRInstr(std::move(instr));
    }
    return varName;
}

///////////////////////////////////////////////////////////////////////////////
// Traitement d'une constante (ConstExpr)
///////////////////////////////////////////////////////////////////////////////
antlrcpp::Any IRGenVisitor::visitConstExpr(ifccParser::ConstExprContext* ctx)
{
    int value = std::stoi(ctx->CONST()->getText());
    std::string temp = cfg->create_new_tempvar();
    BasicBlock *bb = cfg->current_bb;
    auto instr = std::make_unique<IRLdConst>(bb, temp, std::to_string(value));
    cfg->current_bb->add_IRInstr(std::move(instr));
    return temp;
}

///////////////////////////////////////////////////////////////////////////////
// Traitement d'une variable (IdExpr)
///////////////////////////////////////////////////////////////////////////////
antlrcpp::Any IRGenVisitor::visitIdExpr(ifccParser::IdExprContext* ctx)
{
    std::string varName = ctx->ID()->getText();
    return varName;
}

///////////////////////////////////////////////////////////////////////////////
// Traitement de l'opérateur unaire "-"
///////////////////////////////////////////////////////////////////////////////
antlrcpp::Any IRGenVisitor::visitMoinsExpr(ifccParser::MoinsExprContext* ctx)
{
    std::string exprTemp = std::any_cast<std::string>(this->visit(ctx->expr()));
    std::string result = cfg->create_new_tempvar();
    BasicBlock *bb = cfg->current_bb;
    auto zeroTemp = cfg->create_new_tempvar();
    auto loadZero = std::make_unique<IRLdConst>(bb, zeroTemp, "0");
    bb->add_IRInstr(std::move(loadZero));
    auto subInstr = std::make_unique<IRSub>(bb, result, zeroTemp, exprTemp);
    bb->add_IRInstr(std::move(subInstr));
    return result;
}

///////////////////////////////////////////////////////////////////////////////
// Comparaisons binaires (==, !=)
///////////////////////////////////////////////////////////////////////////////
antlrcpp::Any IRGenVisitor::visitCompExpr(ifccParser::CompExprContext* ctx)
{
    // Visiter la première sous-expression
    std::string left = std::any_cast<std::string>(this->visit(ctx->expr(0)));
    // Visiter la deuxième sous-expression
    std::string right = std::any_cast<std::string>(this->visit(ctx->expr(1)));
    // Créer une variable temporaire pour stocker le résultat de la comparaison
    std::string result = cfg->create_new_tempvar();
    BasicBlock *bb = cfg->current_bb;
    
    // Créer une instruction IRComp avec l'opérateur récupéré (par exemple, ">", "<", etc.)
    auto instr = std::make_unique<IRComp>(bb, result, left, right, ctx->op->getText());
    bb->add_IRInstr(std::move(instr));
    
    return result;
}

///////////////////////////////////////////////////////////////////////////////
// Expression multiplicative (*, /, %)
///////////////////////////////////////////////////////////////////////////////
antlrcpp::Any IRGenVisitor::visitMulDivExpr(ifccParser::MulDivExprContext* ctx)
{
    std::string left = std::any_cast<std::string>(this->visit(ctx->expr(0)));
    std::string right = std::any_cast<std::string>(this->visit(ctx->expr(1)));
    std::string result = cfg->create_new_tempvar();
    BasicBlock *bb = cfg->current_bb;

    if (ctx->op->getText() == "*") {
        auto instr = std::make_unique<IRMul>(bb, result, left, right);
        cfg->current_bb->add_IRInstr(std::move(instr));
    } else if (ctx->op->getText() == "/") {
        auto instr = std::make_unique<IRDiv>(bb, result, left, right);
        cfg->current_bb->add_IRInstr(std::move(instr));
    } else if (ctx->op->getText() == "%") {
        auto instr = std::make_unique<IRMod>(bb, result, left, right);
        cfg->current_bb->add_IRInstr(std::move(instr));
    } else {
        std::cerr << "Opérateur MulDivExpr inconnu: " << ctx->op->getText() << "\n";
        exit(1);
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////
// Traitement de la fonction main
///////////////////////////////////////////////////////////////////////////////
antlrcpp::Any IRGenVisitor::visitProg(ifccParser::ProgContext* ctx)
{
    BasicBlock* entryBB = new BasicBlock(cfg, cfg->new_BB_name());
    cfg->add_bb(entryBB);
    cfg->current_bb = entryBB;

    for (auto instCtx : ctx->inst()) {
        this->visit(instCtx);
        if (instCtx->getText().find("return") != std::string::npos) {
            break; // On arrête de générer du code après le return.
        }
    }

    return 0;
}

///////////////////////////////////////////////////////////////////////////////
// Traitement du "!"
///////////////////////////////////////////////////////////////////////////////
antlrcpp::Any IRGenVisitor::visitNotExpr(ifccParser::NotExprContext* ctx) {
    std::string exprTemp = std::any_cast<std::string>(this->visit(ctx->expr()));
    std::string result = cfg->create_new_tempvar();
    BasicBlock *bb = cfg->current_bb;
    auto instr = std::make_unique<IRNot>(bb, result, exprTemp);
    cfg->current_bb->add_IRInstr(std::move(instr));
    return result;
}

///////////////////////////////////////////////////////////////////////////////
// Affectation
///////////////////////////////////////////////////////////////////////////////
antlrcpp::Any IRGenVisitor::visitAssignment(ifccParser::AssignmentContext* ctx)
{
    std::string varName = ctx->ID()->getText();
    std::string exprTemp = std::any_cast<std::string>(this->visit(ctx->expr()));
    BasicBlock *bb = cfg->current_bb;
    auto instr = std::make_unique<IRCopy>(bb, varName, exprTemp);
    cfg->current_bb->add_IRInstr(std::move(instr));
    return varName;
}

antlrcpp::Any IRGenVisitor::visitParExpr(ifccParser::ParExprContext *ctx) {
    return this->visit(ctx->expr());
}

///////////////////////////////////////////////////////////////////////////////
// Additions et soustractions
///////////////////////////////////////////////////////////////////////////////
antlrcpp::Any IRGenVisitor::visitAddSubExpr(ifccParser::AddSubExprContext* ctx)
{
    std::string left = std::any_cast<std::string>(this->visit(ctx->expr(0)));
    std::string right = std::any_cast<std::string>(this->visit(ctx->expr(1)));
    std::string result = cfg->create_new_tempvar();
    BasicBlock *bb = cfg->current_bb;

    if (ctx->op->getText() == "+") {
       auto instr = std::make_unique<IRAdd>(bb, result, left, right);
       cfg->current_bb->add_IRInstr(std::move(instr));
    } else if (ctx->op->getText() == "-") {
       auto instr = std::make_unique<IRSub>(bb, result, left, right);
       cfg->current_bb->add_IRInstr(std::move(instr));
    } else {
       std::cerr << "Opérateur inconnu in AddSubExpr: " << ctx->op->getText() << "\n";
       exit(1);
    }
    return result;
}

///////////////////////////////////////////////////////////////////////////////
// Comparaison d'égalité (==, !=)
///////////////////////////////////////////////////////////////////////////////
antlrcpp::Any IRGenVisitor::visitEgalExpr(ifccParser::EgalExprContext* ctx)
{
    std::string left = std::any_cast<std::string>(this->visit(ctx->expr(0)));
    std::string right = std::any_cast<std::string>(this->visit(ctx->expr(1)));
    std::string result = cfg->create_new_tempvar();
    BasicBlock *bb = cfg->current_bb;

    if (ctx->op->getText() == "==") {
         auto instr = std::make_unique<IREgal>(bb, result, left, right);
         cfg->current_bb->add_IRInstr(std::move(instr));
    } else if (ctx->op->getText() == "!=") {
         auto instr = std::make_unique<IRNotEgal>(bb, result, left, right);
         cfg->current_bb->add_IRInstr(std::move(instr));
    } else {
         std::cerr << "Opérateur inconnu in EgalExpr: " << ctx->op->getText() << "\n";
         exit(1);
    }
    return result;
}

///////////////////////////////////////////////////////////////////////////////
// XOR bit-à-bit
///////////////////////////////////////////////////////////////////////////////
antlrcpp::Any IRGenVisitor::visitOuExcExpr(ifccParser::OuExcExprContext* ctx)
{
    std::string left = std::any_cast<std::string>(this->visit(ctx->expr(0)));
    std::string right = std::any_cast<std::string>(this->visit(ctx->expr(1)));
    std::string result = cfg->create_new_tempvar();
    BasicBlock *bb = cfg->current_bb;
    auto instr = std::make_unique<IRXor>(bb, result, left, right);
    cfg->current_bb->add_IRInstr(std::move(instr));
    return result;
}

///////////////////////////////////////////////////////////////////////////////
// OR bit-à-bit
///////////////////////////////////////////////////////////////////////////////
antlrcpp::Any IRGenVisitor::visitOuIncExpr(ifccParser::OuIncExprContext* ctx)
{
    std::string left = std::any_cast<std::string>(this->visit(ctx->expr(0)));
    std::string right = std::any_cast<std::string>(this->visit(ctx->expr(1)));
    std::string result = cfg->create_new_tempvar();
    BasicBlock *bb = cfg->current_bb;
    auto instr = std::make_unique<IROr>(bb, result, left, right);
    cfg->current_bb->add_IRInstr(std::move(instr));
    return result;
}

///////////////////////////////////////////////////////////////////////////////
// Traitement de l'opérateur logique "&"
///////////////////////////////////////////////////////////////////////////////
antlrcpp::Any IRGenVisitor::visitEtLogExpr(ifccParser::EtLogExprContext* ctx)
{
    std::string left = std::any_cast<std::string>(this->visit(ctx->expr(0)));
    std::string right = std::any_cast<std::string>(this->visit(ctx->expr(1)));
    std::string result = cfg->create_new_tempvar();
    BasicBlock *bb = cfg->current_bb;
    auto instr = std::make_unique<IRAnd>(bb, result, left, right);
    bb->add_IRInstr(std::move(instr));
    return result;
}

///////////////////////////////////////////////////////////////////////////////
// Traitement de l'opérateur logique "&&"
///////////////////////////////////////////////////////////////////////////////
// antlrcpp::Any IRGenVisitor::visitEtParExpr(ifccParser::EtParExprContext* ctx)
// {
//     std::string left = std::any_cast<std::string>(this->visit(ctx->expr(0)));
//     std::string right = std::any_cast<std::string>(this->visit(ctx->expr(1)));
//     std::string result = cfg->create_new_tempvar();
//     BasicBlock *bb = cfg->current_bb;
//     auto instr = std::make_unique<IRAndPar>(bb, result, left, right);
//     bb->add_IRInstr(std::move(instr));
//     return result;
// }

antlrcpp::Any IRGenVisitor::visitEtParExpr(ifccParser::EtParExprContext* ctx) {
    std::string result = cfg->create_new_tempvar(); // variable finale
    std::string left = std::any_cast<std::string>(this->visit(ctx->expr(0)));
    std::string zero = cfg->create_new_tempvar();
    BasicBlock *bb = cfg->current_bb;
    bb->add_IRInstr(std::make_unique<IRLdConst>(bb, zero, "0"));
    // Création des blocs
    BasicBlock* evalRight = new BasicBlock(cfg, cfg->new_BB_name());
    BasicBlock* end = new BasicBlock(cfg, cfg->new_BB_name());
    // Générer un saut conditionnel
    bb->exit_true = evalRight;
    bb->exit_false = end;
    bb->cond_var = left;
    // Compléter le bloc de droite
    cfg->add_bb(evalRight);
    cfg->current_bb = evalRight;
    std::string right = std::any_cast<std::string>(this->visit(ctx->expr(1)));
    evalRight->add_IRInstr(std::make_unique<IRCopy>(evalRight, result, right));
    //evalRight->exit_true = end;
    // Bloc de fin
    cfg->add_bb(end);
    cfg->current_bb = end;
    auto setFalse = std::make_unique<IRCopy>(end, result, zero);
    end->add_IRInstr(std::move(setFalse));
    return result;
}


///////////////////////////////////////////////////////////////////////////////
// Traitement de l'opérateur logique "||"
///////////////////////////////////////////////////////////////////////////////
// antlrcpp::Any IRGenVisitor::visitOuParExpr(ifccParser::OuParExprContext* ctx)
// {
//     std::string left = std::any_cast<std::string>(this->visit(ctx->expr(0)));
//     std::string right = std::any_cast<std::string>(this->visit(ctx->expr(1)));
//     std::string result = cfg->create_new_tempvar();
//     BasicBlock *bb = cfg->current_bb;
//     auto instr = std::make_unique<IROrPar>(bb, result, left, right);
//     bb->add_IRInstr(std::move(instr));
//     return result;
// }

antlrcpp::Any IRGenVisitor::visitOuParExpr(ifccParser::OuParExprContext* ctx) {
    std::string result = cfg->create_new_tempvar(); // variable finale
    std::string left = std::any_cast<std::string>(this->visit(ctx->expr(0)));
    std::string one = cfg->create_new_tempvar();
    BasicBlock *bb = cfg->current_bb;
    bb->add_IRInstr(std::make_unique<IRLdConst>(bb, one, "1"));
    
    // Création des blocs
    BasicBlock* evalRight = new BasicBlock(cfg, cfg->new_BB_name());
    BasicBlock* end = new BasicBlock(cfg, cfg->new_BB_name());
    
    // Générer un saut conditionnel pour "left"
    bb->exit_true = end;      // Si "left" est vrai, on va directement à la fin
    bb->exit_false = evalRight;  // Sinon, on évalue l'expression "right"
    bb->cond_var = left;      // La condition de gauche est stockée dans "cond_var"
    
    // Compléter le bloc de droite
    cfg->add_bb(evalRight);
    cfg->current_bb = evalRight;
    std::string right = std::any_cast<std::string>(this->visit(ctx->expr(1)));
    evalRight->add_IRInstr(std::make_unique<IRCopy>(evalRight, result, right)); // Résultat est à droite
    //evalRight->exit_true = end;
    
    // Bloc de fin (si l'une des deux expressions est "vraie", résultat = 1)
    cfg->add_bb(end);
    cfg->current_bb = end;
    auto setFalse = std::make_unique<IRCopy>(end, result, one); // Par défaut, mettre 1
    end->add_IRInstr(std::move(setFalse));
    
    return result;
}



///////////////////////////////////////////////////////////////////////////////
// Traitement de l'appel de fonction
///////////////////////////////////////////////////////////////////////////////
antlrcpp::Any IRGenVisitor::visitFunction_call(ifccParser::Function_callContext* ctx) {
    std::string funcName = ctx->ID()->getText();
    BasicBlock *bb = cfg->current_bb;
    
    // Si la fonction a des arguments (par exemple, pour putchar, il y en a un)
    if (ctx->expr().size() > 0) {
        // Pour simplifier, on considère ici le cas d'un seul argument.
        std::string arg = std::any_cast<std::string>(this->visit(ctx->expr(0)));
        // Générer une instruction pour déplacer l'argument dans %edi (premier argument)
        auto movInstr = std::make_unique<IRMovReg>(bb, "%edi", arg);
        bb->add_IRInstr(std::move(movInstr));
    }
    
    // Générer l'instruction d'appel de fonction
    auto callInstr = std::make_unique<IRCall>(bb, funcName);
    bb->add_IRInstr(std::move(callInstr));
    
    // Après l'appel, le résultat est dans %eax (selon la convention x86)
    std::string temp = cfg->create_new_tempvar();
    auto copyInstr = std::make_unique<IRCopy>(bb, temp, "%eax");
    bb->add_IRInstr(std::move(copyInstr));
    
    return temp;
}