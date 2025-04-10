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
    BasicBlock *bb = cfg->current_bb;
    if (ctx->expr())
    {
        std::string temp = std::any_cast<std::string>(this->visit(ctx->expr()));
        auto instr = std::make_unique<IRReturn>(bb, temp);
        bb->add_IRInstr(std::move(instr));
    }
    else
    {
        // Void return: jump to epilogue
        ostringstream jumpInstr;
        jumpInstr << "    jmp " << cfg->epilogueLabel << "\n";
        auto instr = std::make_unique<IRBranch>(bb, "", jumpInstr.str(), "");
        bb->add_IRInstr(std::move(instr));
    }

    hasReturned = true;
    return std::string("");
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
        std::string arch = codegenBackend->getArchitecture();
        std::vector<std::string> paramRegs;
        if (arch == "arm64")
        {
            paramRegs = {"w0", "w1", "w2", "w3", "w4", "w5", "w6", "w7"};
        }
        else if (arch == "X86")
        {
            paramRegs = {"%edi", "%esi", "%edx", "%ecx", "%r8d", "%r9d"}; // 32-bit subregisters for 32-bit data
        }
        else
        {
            std::cerr << "[ERROR] Unsupported architecture: " << arch << "\n";
            exit(1);
        }

        size_t index = 0;
        for (auto param : ctx->decl_params()->param())
        {
            if (index >= paramRegs.size())
            {
                std::cerr << "[ERROR] Too many parameters for function\n";
                exit(1);
            }
            std::string paramName = param->ID()->getText();
            std::string sourceReg = paramRegs[index++];
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
    for (const auto &[_, info] : cfg->get_stv().symbolStack.front())
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


/////////////////////////////////////////////////////////////////////////////
// Traitement de l'opérateur logique "&&"
///////////////////////////////////////////////////////////////////////////////
antlrcpp::Any IRGenVisitor::visitEtParExpr(ifccParser::EtParExprContext* ctx)
{
    std::string result = cfg->create_new_tempvar();
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
// Traitement du "while"
///////////////////////////////////////////////////////////////////////////////

antlrcpp::Any IRGenVisitor::visitOuParExpr(ifccParser::OuParExprContext* ctx)
{
    std::string result = cfg->create_new_tempvar();
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

