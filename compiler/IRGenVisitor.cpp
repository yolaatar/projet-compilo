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
    
    return result;visit(ctx->expr(0));
    std::string tmp = newTemp();
    int offset = stv.symbolTable[tmp].offset;
    std::cout << "    movl %eax, "<<offset<<"(%rbp)\n";
    visit(ctx->expr(1));
    std::cout << "    cmpl "<< offset <<"(%rbp), %eax\n";
    if (ctx->op->getText() == "<") {
        std::cout << "    setg %al\n";  // Si inférieur (SF ≠ OF), AL = 1
    } else if (ctx->op->getText() == "<=") {
        std::cout << "    setge %al\n"; // Si inférieur ou égal (ZF=1 ou SF ≠ OF), AL = 1
    } else if (ctx->op->getText() == ">") {
        std::cout << "    setl %al\n";  // Si supérieur (ZF=0 et SF=OF), AL = 1
    } else if (ctx->op->getText() == ">=") {
        std::cout << "    setle %al\n"; // Si supérieur ou égal (SF=OF), AL = 1
    }
    
    std::cout << "    movzbl %al, %eax\n"; // Étend AL (8 bits) en EAX (32 bits)
    return 0;
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

///////////////////////////////////////////////////////////////////////////////
// Traitement du "if - else"
///////////////////////////////////////////////////////////////////////////////
antlrcpp::Any IRGenVisitor::visitIf_stmt(ifccParser::If_stmtContext* ctx)
{
    
    // Conserver le bloc courant
    BasicBlock* currentBB = cfg->current_bb;
    
    // 1. Évaluer la condition et obtenir son temporary
    currentBB->test_var_name = std::any_cast<std::string>(this->visit(ctx->expr()));
    
    // 2. Créer les BasicBlocks pour la branche then, la branche else et le bloc de fusion (merge) pour cet if
    BasicBlock* thenBB = new BasicBlock(cfg, cfg->new_BB_name());
    thenBB->label += "_then";
    BasicBlock* elseBB = new BasicBlock(cfg, cfg->new_BB_name());
    elseBB->label += "_else";
    BasicBlock* mergeBB = new BasicBlock(cfg, cfg->new_BB_name());
    mergeBB->label += "_merge";

    mergeBB->exit_true = currentBB->exit_true;
    mergeBB->exit_false = currentBB->exit_false;

    currentBB->exit_true = thenBB;
    currentBB->exit_false = elseBB;

    thenBB->exit_true = mergeBB;
    elseBB->exit_false = mergeBB;

    cfg->current_bb = thenBB;
    
    // 4. Générer le code pour la branche then.
    cfg->add_bb(thenBB);  
    this->visit(ctx->block(0));  // Traiter le bloc then
    
    // 5. Générer le code pour la branche else.
    cfg->add_bb(elseBB);
    cfg->current_bb = elseBB;
    if (ctx->block().size() > 1) {
        this->visit(ctx->block(1)); // Traiter le bloc else s'il existe
    }
    
    // 6. Ajouter le bloc de fusion et le définir comme bloc courant
    cfg->add_bb(mergeBB);
    cfg->current_bb = mergeBB;
    
    return std::string("0");
}


///////////////////////////////////////////////////////////////////////////////
// Traitement du "while"
///////////////////////////////////////////////////////////////////////////////
antlrcpp::Any IRGenVisitor::visitWhile_stmt(ifccParser::While_stmtContext* ctx)
{
    BasicBlock* currentBB = cfg->current_bb;

    BasicBlock* condBB = new BasicBlock(cfg, cfg->new_BB_name()); 
    condBB->label += "_cond";
    BasicBlock* bodyBB = new BasicBlock(cfg, cfg->new_BB_name()); 
    bodyBB->label += "_body";
    BasicBlock* exitBB = new BasicBlock(cfg, cfg->new_BB_name());  
    exitBB->label += "_exit";

    exitBB->exit_true = currentBB->exit_true;
    exitBB->exit_false = currentBB->exit_false;

    currentBB->exit_true = condBB;
    
    condBB->exit_false = exitBB;
    condBB->exit_true = bodyBB;
    bodyBB->exit_true = condBB;

    cfg->add_bb(condBB);
    cfg->current_bb = condBB;
    std::string cond = std::any_cast<std::string>(this->visit(ctx->expr()));
    condBB->test_var_name = cond;
    
    cfg->add_bb(bodyBB);
    cfg->current_bb = bodyBB;
    this->visit(ctx->block());
    
    cfg->add_bb(exitBB);
    cfg->current_bb = exitBB;
    
    return std::string("0");
}

