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
antlrcpp::Any IRGenVisitor::visitReturn_stmt(ifccParser::Return_stmtContext *ctx)
{
    std::string temp = std::any_cast<std::string>(this->visit(ctx->expr()));
    BasicBlock *bb = cfg->current_bb;
    auto instr = std::make_unique<IRReturn>(bb, temp);
    bb->add_IRInstr(std::move(instr));

    hasReturned = true; // Marque que le return a été rencontré
    return temp;
}

///////////////////////////////////////////////////////////////////////////////
// visitDecl : Traitement d'une déclaration "int ID ('=' expr)?".
///////////////////////////////////////////////////////////////////////////////
antlrcpp::Any IRGenVisitor::visitDecl(ifccParser::DeclContext *ctx)
{
    std::string varName = ctx->ID()->getText();
    BasicBlock *bb = cfg->current_bb;
    if (ctx->expr() != nullptr)
    {
        std::string temp = std::any_cast<std::string>(this->visit(ctx->expr()));
        auto instr = std::make_unique<IRCopy>(bb, varName, temp);
        cfg->current_bb->add_IRInstr(std::move(instr));
    }
    else
    {
        auto instr = std::make_unique<IRLdConst>(bb, varName, "0");
        cfg->current_bb->add_IRInstr(std::move(instr));
    }
    return varName;
}

///////////////////////////////////////////////////////////////////////////////
// Traitement d'une constante (ConstExpr)
///////////////////////////////////////////////////////////////////////////////
antlrcpp::Any IRGenVisitor::visitConstExpr(ifccParser::ConstExprContext *ctx)
{
    int value = std::stoi(ctx->CONST()->getText());
    std::string temp = cfg->create_new_tempvar();
    BasicBlock *bb = cfg->current_bb;
    auto instr = std::make_unique<IRLdConst>(bb, temp, std::to_string(value));
    bb->add_IRInstr(std::move(instr));
    // Retourner explicitement une std::string dans le std::any
    return temp;
}

///////////////////////////////////////////////////////////////////////////////
// Traitement d'une variable (IdExpr)
///////////////////////////////////////////////////////////////////////////////
antlrcpp::Any IRGenVisitor::visitIdExpr(ifccParser::IdExprContext *ctx)
{
    std::string varName = ctx->ID()->getText();
    return varName; // Retourne une std::string
}

///////////////////////////////////////////////////////////////////////////////
// Traitement de l'opérateur unaire "-"
///////////////////////////////////////////////////////////////////////////////
antlrcpp::Any IRGenVisitor::visitMoinsExpr(ifccParser::MoinsExprContext *ctx)
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
antlrcpp::Any IRGenVisitor::visitCompExpr(ifccParser::CompExprContext *ctx)
{
    // Évaluer les deux sous-expressions
    std::string left = std::any_cast<std::string>(this->visit(ctx->expr(0)));
    std::string right = std::any_cast<std::string>(this->visit(ctx->expr(1)));
    // Créer un nouveau temporary pour le résultat
    std::string result = cfg->create_new_tempvar();
    BasicBlock *bb = cfg->current_bb;

    std::string op = ctx->op->getText();
    if (op == ">=")
    {
        bb->add_IRInstr(std::make_unique<IRGe>(bb, result, left, right));
    }
    else if (op == ">")
    {
        bb->add_IRInstr(std::make_unique<IRGt>(bb, result, left, right));
    }
    else if (op == "==")
    {
        bb->add_IRInstr(std::make_unique<IREgal>(bb, result, left, right));
    }
    else if (op == "!=")
    {
        bb->add_IRInstr(std::make_unique<IRNotEgal>(bb, result, left, right));
    }
    else if (op == "<=")
    {
        bb->add_IRInstr(std::make_unique<IRCompInfEg>(bb, result, left, right));
    }
    else if (op == "<")
    {
        bb->add_IRInstr(std::make_unique<IRCompInf>(bb, result, left, right));
    }
    else
    {
        std::cerr << "Opérateur de comparaison non implémenté: " << op << "\n";
        exit(1);
    }
    return result;
}

///////////////////////////////////////////////////////////////////////////////
// Expression multiplicative (*, /, %)
///////////////////////////////////////////////////////////////////////////////
antlrcpp::Any IRGenVisitor::visitMulDivExpr(ifccParser::MulDivExprContext *ctx)
{
    std::string left = std::any_cast<std::string>(this->visit(ctx->expr(0)));
    std::string right = std::any_cast<std::string>(this->visit(ctx->expr(1)));
    std::string result = cfg->create_new_tempvar();
    BasicBlock *bb = cfg->current_bb;

    if (ctx->op->getText() == "*")
    {
        auto instr = std::make_unique<IRMul>(bb, result, left, right);
        cfg->current_bb->add_IRInstr(std::move(instr));
    }
    else if (ctx->op->getText() == "/")
    {
        auto instr = std::make_unique<IRDiv>(bb, result, left, right);
        cfg->current_bb->add_IRInstr(std::move(instr));
    }
    else if (ctx->op->getText() == "%")
    {
        auto instr = std::make_unique<IRMod>(bb, result, left, right);
        cfg->current_bb->add_IRInstr(std::move(instr));
    }
    else
    {
        std::cerr << "Opérateur MulDivExpr inconnu: " << ctx->op->getText() << "\n";
        exit(1);
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////////
// Traitement de la fonction main
///////////////////////////////////////////////////////////////////////////////
antlrcpp::Any IRGenVisitor::visitProg(ifccParser::ProgContext *ctx)
{
    // Crée le BasicBlock d'entrée pour cette fonction
    BasicBlock *entryBB = new BasicBlock(cfg, cfg->new_BB_name());
    cfg->add_bb(entryBB);
    cfg->current_bb = entryBB;

    // Étape 1 : gérer les paramètres
    if (ctx->decl_params())
    {
        int index = 0;
        for (auto param : ctx->decl_params()->param())
        {
            std::string paramName = param->ID()->getText();

            // Paramètres ARM64 : x0, x1, x2, ..., jusqu'à x7
            std::string sourceReg = "w" + std::to_string(index++);
            std::string localSlot = cfg->IR_reg_to_asm(paramName);

            // Génère : str wX, [sp, offset]
            auto instr = std::make_unique<IRCopy>(cfg->current_bb, paramName, sourceReg);
            cfg->current_bb->add_IRInstr(std::move(instr));
        }
    }

    // Étape 2 : générer le corps de la fonction
    for (auto instCtx : ctx->inst())
    {
        if (hasReturned)
            break; // Évite de générer après un return
        this->visit(instCtx);
    }

    // Étape 3 : calcul du maxOffset pour l'allocation stack
    int minOffset = 0;
    for (const auto &[_, info] : cfg->get_stv().symbolTable)
    {
        if (info.offset < minOffset)
        {
            minOffset = info.offset;
        }
    }
    cfg->maxOffset = -minOffset;

    return 0;
}

antlrcpp::Any IRGenVisitor::visitAxiom(ifccParser::AxiomContext *ctx)
{
    for (auto prog : ctx->prog())
    {
        this->visit(prog); // chaque fonction
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////////
// Traitement du "!"
///////////////////////////////////////////////////////////////////////////////
antlrcpp::Any IRGenVisitor::visitNotExpr(ifccParser::NotExprContext *ctx)
{
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
antlrcpp::Any IRGenVisitor::visitAssignment(ifccParser::AssignmentContext *ctx)
{
    std::string varName = ctx->ID()->getText();
    std::string exprTemp = std::any_cast<std::string>(this->visit(ctx->expr()));
    BasicBlock *bb = cfg->current_bb;

    if (varName != exprTemp)
    { // éviter les copies inutiles
        auto instr = std::make_unique<IRCopy>(bb, varName, exprTemp);
        cfg->current_bb->add_IRInstr(std::move(instr));
    }

    return varName;
}

antlrcpp::Any IRGenVisitor::visitParExpr(ifccParser::ParExprContext *ctx)
{
    return this->visit(ctx->expr());
}

///////////////////////////////////////////////////////////////////////////////
// Additions et soustractions
///////////////////////////////////////////////////////////////////////////////

antlrcpp::Any IRGenVisitor::visitAddSubExpr(ifccParser::AddSubExprContext *ctx)
{
    // Évalue les sous-expressions et s'assure de renvoyer des std::string
    std::string left = std::any_cast<std::string>(this->visit(ctx->expr(0)));
    std::string right = std::any_cast<std::string>(this->visit(ctx->expr(1)));
    std::string result = cfg->create_new_tempvar();
    BasicBlock *bb = cfg->current_bb;

    if (ctx->op->getText() == "+")
    {
        auto instr = std::make_unique<IRAdd>(bb, result, left, right);
        bb->add_IRInstr(std::move(instr));
    }
    else if (ctx->op->getText() == "-")
    {
        auto instr = std::make_unique<IRSub>(bb, result, left, right);
        bb->add_IRInstr(std::move(instr));
    }
    else
    {
        std::cerr << "Opérateur inconnu in AddSubExpr: " << ctx->op->getText() << "\n";
        exit(1);
    }
    return result;
}

///////////////////////////////////////////////////////////////////////////////
// Comparaison d'égalité (==, !=)
///////////////////////////////////////////////////////////////////////////////
antlrcpp::Any IRGenVisitor::visitEgalExpr(ifccParser::EgalExprContext *ctx)
{
    std::string left = std::any_cast<std::string>(this->visit(ctx->expr(0)));
    std::string right = std::any_cast<std::string>(this->visit(ctx->expr(1)));
    std::string result = cfg->create_new_tempvar();
    BasicBlock *bb = cfg->current_bb;

    if (ctx->op->getText() == "==")
    {
        auto instr = std::make_unique<IREgal>(bb, result, left, right);
        cfg->current_bb->add_IRInstr(std::move(instr));
    }
    else if (ctx->op->getText() == "!=")
    {
        auto instr = std::make_unique<IRNotEgal>(bb, result, left, right);
        cfg->current_bb->add_IRInstr(std::move(instr));
    }
    else
    {
        std::cerr << "Opérateur inconnu in EgalExpr: " << ctx->op->getText() << "\n";
        exit(1);
    }
    return result;
}

///////////////////////////////////////////////////////////////////////////////
// XOR bit-à-bit
///////////////////////////////////////////////////////////////////////////////
antlrcpp::Any IRGenVisitor::visitOuExcExpr(ifccParser::OuExcExprContext *ctx)
{
    std::string left = std::any_cast<std::string>(this->visit(ctx->expr(0)));
    std::string right = std::any_cast<std::string>(this->visit(ctx->expr(1)));
    std::string result = cfg->create_new_tempvar();
    BasicBlock *bb = cfg->current_bb;
    auto instr = std::make_unique<IRXor>(bb, result, left, right);
    cfg->current_bb->add_IRInstr(std::move(instr));
    return result;
}

antlrcpp::Any IRGenVisitor::visitFunction_call(ifccParser::Function_callContext *ctx)
{
    std::string name = ctx->ID()->getText();
    BasicBlock *bb = cfg->current_bb;

    // 🔒 Vérification dans la table des fonctions
    if (functionTable && functionTable->find(name) == functionTable->end())
    {
        std::cerr << "[ERROR] Function '" << name << "' is not declared\n";
        exit(1);
    }

    // Vérifie le nombre de paramètres si on a une signature
    if (functionTable)
    {
        const auto &sig = (*functionTable)[name];
        size_t expected = sig.paramsTypes.size();
        size_t actual = ctx->expr().size();

        if (expected != actual)
        {
            std::cerr << "[ERROR] Function '" << name << "' expects " << expected
                      << " arguments but got " << actual << "\n";
            exit(1);
        }
    }

    // 🔁 Cas spéciaux
    if (name == "getchar")
    {
        cfg->usesGetChar = true;
        std::string result = cfg->create_new_tempvar();
        bb->add_IRInstr(std::make_unique<IRGetChar>(bb, result));
        return result;
    }
    else if (name == "putchar")
    {
        cfg->usesPutChar = true;
        std::string arg = std::any_cast<std::string>(visit(ctx->expr(0)));
        bb->add_IRInstr(std::make_unique<IRPutChar>(bb, arg));
        return arg;
    }

    // 🔁 Cas générique : fonction utilisateur
    std::vector<std::string> arguments;
    for (auto exprCtx : ctx->expr())
    {
        arguments.push_back(std::any_cast<std::string>(visit(exprCtx)));
    }

    std::string returnVar = cfg->create_new_tempvar(); // Pour le résultat de l'appel
    bb->add_IRInstr(std::make_unique<IRCall>(bb, name, arguments, returnVar));
    return returnVar;
}


///////////////////////////////////////////////////////////////////////////////
// OR bit-à-bit
///////////////////////////////////////////////////////////////////////////////
antlrcpp::Any IRGenVisitor::visitOuIncExpr(ifccParser::OuIncExprContext *ctx)
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
antlrcpp::Any IRGenVisitor::visitEtLogExpr(ifccParser::EtLogExprContext *ctx)
{
    std::string left = std::any_cast<std::string>(this->visit(ctx->expr(0)));
    std::string right = std::any_cast<std::string>(this->visit(ctx->expr(1)));
    std::string result = cfg->create_new_tempvar();
    BasicBlock *bb = cfg->current_bb;

    // Génère directement un AND bitwise sur left et right
    bb->add_IRInstr(std::make_unique<IRAnd>(bb, result, left, right));

    return result;
}
/*
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
antlrcpp::Any IRGenVisitor::visitEtParExpr(ifccParser::EtParExprContext* ctx)
{
    std::string left = std::any_cast<std::string>(this->visit(ctx->expr(0)));
    std::string right = std::any_cast<std::string>(this->visit(ctx->expr(1)));
    std::string result = cfg->create_new_tempvar();
    BasicBlock *bb = cfg->current_bb;
    auto instr = std::make_unique<IRAndPar>(bb, result, left, right);
    bb->add_IRInstr(std::move(instr));
    return result;
}

///////////////////////////////////////////////////////////////////////////////
// Traitement de l'opérateur logique "||"
///////////////////////////////////////////////////////////////////////////////
antlrcpp::Any IRGenVisitor::visitOuParExpr(ifccParser::OuParExprContext* ctx)
{
    std::string left = std::any_cast<std::string>(this->visit(ctx->expr(0)));
    std::string right = std::any_cast<std::string>(this->visit(ctx->expr(1)));
    std::string result = cfg->create_new_tempvar();
    BasicBlock *bb = cfg->current_bb;
    auto instr = std::make_unique<IROrPar>(bb, result, left, right);
    bb->add_IRInstr(std::move(instr));
    return result;
}
*/

///////////////////////////////////////////////////////////////////////////////
// IF THEN ELSE
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Traitement du "if - else"
///////////////////////////////////////////////////////////////////////////////
antlrcpp::Any IRGenVisitor::visitIf_stmt(ifccParser::If_stmtContext *ctx)
{
    BasicBlock* currentBB = cfg->current_bb;
    std::string condTemp = std::any_cast<std::string>(this->visit(ctx->expr()));
    BasicBlock* thenBB = new BasicBlock(cfg, cfg->new_BB_name());
    BasicBlock* mergeBB = new BasicBlock(cfg, cfg->new_BB_name());

    if (ctx->block().size() > 1) {
         BasicBlock* elseBB = new BasicBlock(cfg, cfg->new_BB_name());
         currentBB->add_IRInstr(std::make_unique<IRJumpCond>(currentBB, condTemp, thenBB->label, elseBB->label));

         cfg->add_bb(thenBB);
         cfg->current_bb = thenBB;
         this->visit(ctx->block(0));
         thenBB->add_IRInstr(std::make_unique<IRJump>(thenBB, mergeBB->label));

         cfg->add_bb(elseBB);
         cfg->current_bb = elseBB;
         this->visit(ctx->block(1));
         elseBB->add_IRInstr(std::make_unique<IRJump>(elseBB, mergeBB->label));
    } else {
         // Si pas de clause else, le faux chemin va directement dans mergeBB.
         currentBB->add_IRInstr(std::make_unique<IRJumpCond>(currentBB, condTemp, thenBB->label, mergeBB->label));
         cfg->add_bb(thenBB);
         cfg->current_bb = thenBB;
         this->visit(ctx->block(0));
         thenBB->add_IRInstr(std::make_unique<IRJump>(thenBB, mergeBB->label));
    }
    
    cfg->add_bb(mergeBB);
    cfg->current_bb = mergeBB;

    return std::string("");
}


/////////////////////////////////////////////////////////////////////////////
// Traitement de l'opérateur logique "&&"
///////////////////////////////////////////////////////////////////////////////
antlrcpp::Any IRGenVisitor::visitEtParExpr(ifccParser::EtParExprContext *ctx)
{
    std::string left = std::any_cast<std::string>(this->visit(ctx->expr(0)));
    std::string right = std::any_cast<std::string>(this->visit(ctx->expr(1)));
    std::string result = cfg->create_new_tempvar();
    BasicBlock *bb = cfg->current_bb;
    auto instr = std::make_unique<IRAndPar>(bb, result, left, right);
    bb->add_IRInstr(std::move(instr));
    return result;
}

///////////////////////////////////////////////////////////////////////////////
// Traitement de l'opérateur logique "||"
///////////////////////////////////////////////////////////////////////////////
antlrcpp::Any IRGenVisitor::visitOuParExpr(ifccParser::OuParExprContext *ctx)
{
    std::string left = std::any_cast<std::string>(this->visit(ctx->expr(0)));
    std::string right = std::any_cast<std::string>(this->visit(ctx->expr(1)));
    std::string result = cfg->create_new_tempvar();
    BasicBlock *bb = cfg->current_bb;
    auto instr = std::make_unique<IROrPar>(bb, result, left, right);
    bb->add_IRInstr(std::move(instr));
    return result;
}


antlrcpp::Any IRGenVisitor::visitWhile_stmt(ifccParser::While_stmtContext *ctx) {
    // 1. Création des BasicBlocks nécessaires
    BasicBlock* condBB = new BasicBlock(cfg, cfg->new_BB_name());
    BasicBlock* bodyBB = new BasicBlock(cfg, cfg->new_BB_name());
    BasicBlock* afterBB = new BasicBlock(cfg, cfg->new_BB_name());

    // 2. Saut immédiat vers le bloc condition depuis le bloc actuel
    cfg->current_bb->add_IRInstr(std::make_unique<IRJump>(cfg->current_bb, condBB->label));

    // 3. Bloc de la condition
    cfg->add_bb(condBB);
    cfg->current_bb = condBB;

    std::string condTemp = std::any_cast<std::string>(visit(ctx->expr()));
    condBB->add_IRInstr(std::make_unique<IRJumpCond>(condBB, condTemp, bodyBB->label, afterBB->label));

    // 4. Bloc du corps de la boucle
    cfg->add_bb(bodyBB);
    cfg->current_bb = bodyBB;
    visit(ctx->block()); // Corps du while

    bodyBB->add_IRInstr(std::make_unique<IRJump>(bodyBB, condBB->label)); // Retour à la condition

    // 5. Bloc de sortie
    cfg->add_bb(afterBB);
    cfg->current_bb = afterBB;

    return nullptr;
}
