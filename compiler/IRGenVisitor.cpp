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
    std::cerr << "visitCompExpr not implemented yet." << std::endl;
    return std::string("0");
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
// Traitement de l'opérateur logique "&&"
///////////////////////////////////////////////////////////////////////////////
antlrcpp::Any IRGenVisitor::visitEtLogExpr(ifccParser::EtLogExprContext* ctx)
{
    std::string left = std::any_cast<std::string>(this->visit(ctx->expr(0)));
    std::string right = std::any_cast<std::string>(this->visit(ctx->expr(1)));
    std::string result = cfg->create_new_tempvar();
    BasicBlock *bb = cfg->current_bb;

    // Étape 1 : left && right <=> (left != 0) && (right != 0)
    // Étape 2 : on génère une comparaison "!=" pour les deux
    std::string condLeft = cfg->create_new_tempvar();
    std::string condRight = cfg->create_new_tempvar();

    // Génère : condLeft = (left != 0)
    bb->add_IRInstr(std::make_unique<IRNotEgal>(bb, condLeft, left, "0"));
    // Génère : condRight = (right != 0)
    bb->add_IRInstr(std::make_unique<IRNotEgal>(bb, condRight, right, "0"));

    // Génère : result = condLeft & condRight
    bb->add_IRInstr(std::make_unique<IRAnd>(bb, result, condLeft, condRight));

    return result;
}
