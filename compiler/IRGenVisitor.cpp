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
    auto valAny = this->visit(ctx->expr());
    std::string temp;
    BasicBlock *bb = cfg->current_bb;

    if (valAny.type() == typeid(int)) {
        int val = std::any_cast<int>(valAny);
        temp = cfg->create_new_tempvar();
        bb->add_IRInstr(std::make_unique<IRLdConst>(bb, temp, std::to_string(val)));
        std::cerr << "[FOLD RETURN] return " << val << "\n";
    } else {
        temp = std::any_cast<std::string>(valAny);
    }

    bb->add_IRInstr(std::make_unique<IRReturn>(bb, temp));
    hasReturned = true;
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
        auto val = visit(ctx->expr());

        if (val.type() == typeid(int))
        {
            int constVal = std::any_cast<int>(val);
            constMap[varName] = constVal;
            bb->add_IRInstr(std::make_unique<IRLdConst>(bb, varName, std::to_string(constVal)));
            return constVal;
        }
        else
        {
            std::string temp = std::any_cast<std::string>(val);
            constMap.erase(varName); // Pas une constante
            bb->add_IRInstr(std::make_unique<IRCopy>(bb, varName, temp));
            return varName;
        }
    }
    else
    {
        constMap[varName] = 0;
        bb->add_IRInstr(std::make_unique<IRLdConst>(bb, varName, "0"));
        return 0;
    }
}

///////////////////////////////////////////////////////////////////////////////
// Traitement d'une constante (ConstExpr)
///////////////////////////////////////////////////////////////////////////////
antlrcpp::Any IRGenVisitor::visitConstExpr(ifccParser::ConstExprContext *ctx)
{
    return std::stoi(ctx->CONST()->getText());  // retourne directement un entier ✅
}


///////////////////////////////////////////////////////////////////////////////
// Traitement d'une variable (IdExpr)
///////////////////////////////////////////////////////////////////////////////
antlrcpp::Any IRGenVisitor::visitIdExpr(ifccParser::IdExprContext *ctx)
{
    std::string varName = ctx->ID()->getText();
    if (constMap.find(varName) != constMap.end())
    {
        return constMap[varName]; // Propagation directe
    }
    return varName; // Pas une constante, retourne le nom
}

///////////////////////////////////////////////////////////////////////////////
// Traitement de l'opérateur unaire "-"
///////////////////////////////////////////////////////////////////////////////
antlrcpp::Any IRGenVisitor::visitMoinsExpr(ifccParser::MoinsExprContext *ctx)
{
    auto val = visit(ctx->expr());

    if (val.type() == typeid(int))
    {
        int folded = -std::any_cast<int>(val);
        std::cerr << "[FOLD] -" << std::any_cast<int>(val) << " = " << folded << "\n";
        return folded;
    }

    std::string exprTemp = std::any_cast<std::string>(val);
    std::string result = cfg->create_new_tempvar();
    std::string zeroTemp = cfg->create_new_tempvar();

    BasicBlock *bb = cfg->current_bb;
    bb->add_IRInstr(std::make_unique<IRLdConst>(bb, zeroTemp, "0"));
    bb->add_IRInstr(std::make_unique<IRSub>(bb, result, zeroTemp, exprTemp));

    return result;
}


antlrcpp::Any IRGenVisitor::visitCompExpr(ifccParser::CompExprContext *ctx)
{
    auto leftAny = visit(ctx->expr(0));
    auto rightAny = visit(ctx->expr(1));
    std::string op = ctx->op->getText();

    bool isLeftConst = leftAny.type() == typeid(int);
    bool isRightConst = rightAny.type() == typeid(int);

    if (isLeftConst && isRightConst)
    {
        int l = std::any_cast<int>(leftAny);
        int r = std::any_cast<int>(rightAny);
        bool res = false;

        if (op == "<") res = l < r;
        else if (op == "<=") res = l <= r;
        else if (op == ">") res = l > r;
        else if (op == ">=") res = l >= r;

        std::cerr << "[FOLD] " << l << " " << op << " " << r << " => " << res << "\n";
        return static_cast<int>(res);
    }

    std::string left = isLeftConst ? std::to_string(std::any_cast<int>(leftAny)) : std::any_cast<std::string>(leftAny);
    std::string right = isRightConst ? std::to_string(std::any_cast<int>(rightAny)) : std::any_cast<std::string>(rightAny);
    std::string result = cfg->create_new_tempvar();
    BasicBlock *bb = cfg->current_bb;

    bb->add_IRInstr(std::make_unique<IRComp>(bb, result, left, right, op));
    return result;
}


///////////////////////////////////////////////////////////////////////////////
// Expression multiplicative (*, /, %)
///////////////////////////////////////////////////////////////////////////////
antlrcpp::Any IRGenVisitor::visitMulDivExpr(ifccParser::MulDivExprContext *ctx)
{
    auto leftAny = visit(ctx->expr(0));
    auto rightAny = visit(ctx->expr(1));
    std::string op = ctx->op->getText();

    bool leftIsConst = leftAny.type() == typeid(int);
    bool rightIsConst = rightAny.type() == typeid(int);

    if (leftIsConst && rightIsConst)
    {
        int lhs = std::any_cast<int>(leftAny);
        int rhs = std::any_cast<int>(rightAny);
        int folded;

        if (op == "*")
            folded = lhs * rhs;
        else if (op == "/" && rhs != 0)
            folded = lhs / rhs;
        else if (op == "%" && rhs != 0)
            folded = lhs % rhs;
        else {
            std::cerr << "[FOLD-ERROR] Division/modulo par zéro dans une expression constante.\n";
            folded = 0; // Valeur par défaut sécurisée
        }

        std::cerr << "[FOLD] " << lhs << " " << op << " " << rhs << " = " << folded << std::endl;
        return folded;
    }

    // Sinon : au moins un des deux est une variable ou temporaire
    std::string left = leftIsConst ? gen_const(std::any_cast<int>(leftAny)) : std::any_cast<std::string>(leftAny);
    std::string right = rightIsConst ? gen_const(std::any_cast<int>(rightAny)) : std::any_cast<std::string>(rightAny);

    std::string result = cfg->create_new_tempvar();
    BasicBlock *bb = cfg->current_bb;

    if (op == "*")
        bb->add_IRInstr(std::make_unique<IRMul>(bb, result, left, right));
    else if (op == "/")
        bb->add_IRInstr(std::make_unique<IRDiv>(bb, result, left, right));
    else if (op == "%")
        bb->add_IRInstr(std::make_unique<IRMod>(bb, result, left, right));
    else {
        std::cerr << "[ERROR] Opérateur inconnu dans MulDivExpr: " << op << "\n";
        exit(1);
    }

    return result;
}


std::string IRGenVisitor::gen_const(int value)
{
    std::string temp = cfg->create_new_tempvar();
    cfg->current_bb->add_IRInstr(std::make_unique<IRLdConst>(cfg->current_bb, temp, std::to_string(value)));
    return temp;
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
    auto exprVal = this->visit(ctx->expr());

    // 🔍 Propagation de constante si possible
    if (exprVal.type() == typeid(int)) {
        int val = std::any_cast<int>(exprVal);
        return (val == 0) ? 1 : 0;
    }

    std::string exprTemp = std::any_cast<std::string>(exprVal);
    std::string result = cfg->create_new_tempvar();
    BasicBlock *bb = cfg->current_bb;
    bb->add_IRInstr(std::make_unique<IRNot>(bb, result, exprTemp));
    return result;
}


///////////////////////////////////////////////////////////////////////////////
// Affectation
///////////////////////////////////////////////////////////////////////////////
antlrcpp::Any IRGenVisitor::visitAssignment(ifccParser::AssignmentContext *ctx)
{
    std::string varName = ctx->ID()->getText();
    auto val = visit(ctx->expr());
    BasicBlock *bb = cfg->current_bb;

    if (val.type() == typeid(int))
    {
        int constVal = std::any_cast<int>(val);
        constMap[varName] = constVal;
        bb->add_IRInstr(std::make_unique<IRLdConst>(bb, varName, std::to_string(constVal)));
        return constVal;
    }
    else
    {
        std::string exprTemp = std::any_cast<std::string>(val);
        constMap.erase(varName); // plus une constante connue
        if (varName != exprTemp)
        {
            bb->add_IRInstr(std::make_unique<IRCopy>(bb, varName, exprTemp));
        }
        return varName;
    }
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
    auto leftAny = visit(ctx->expr(0));
    auto rightAny = visit(ctx->expr(1));

    bool leftIsConst = leftAny.type() == typeid(int);
    bool rightIsConst = rightAny.type() == typeid(int);

    if (leftIsConst && rightIsConst)
    {
        int lhs = std::any_cast<int>(leftAny);
        int rhs = std::any_cast<int>(rightAny);
        int folded = (ctx->op->getText() == "+") ? lhs + rhs : lhs - rhs;
        std::cerr << "[FOLD] " << lhs << " " << ctx->op->getText() << " " << rhs << " = " << folded << "\n";
        return folded;
    }
    std::string left = leftIsConst
                           ? gen_const(std::any_cast<int>(leftAny))
                           : std::any_cast<std::string>(leftAny);

    std::string right = rightIsConst
                            ? gen_const(std::any_cast<int>(rightAny))
                            : std::any_cast<std::string>(rightAny);

    std::string result = cfg->create_new_tempvar();
    BasicBlock *bb = cfg->current_bb;

    if (ctx->op->getText() == "+")
        bb->add_IRInstr(std::make_unique<IRAdd>(bb, result, left, right));
    else if (ctx->op->getText() == "-")
        bb->add_IRInstr(std::make_unique<IRSub>(bb, result, left, right));

    return result;
}

///////////////////////////////////////////////////////////////////////////////
// Comparaison d'égalité (==, !=)
///////////////////////////////////////////////////////////////////////////////
antlrcpp::Any IRGenVisitor::visitEgalExpr(ifccParser::EgalExprContext *ctx)
{
    auto leftAny = visit(ctx->expr(0));
    auto rightAny = visit(ctx->expr(1));

    bool leftConst = leftAny.type() == typeid(int);
    bool rightConst = rightAny.type() == typeid(int);

    if (leftConst && rightConst)
    {
        int l = std::any_cast<int>(leftAny);
        int r = std::any_cast<int>(rightAny);
        bool result = (ctx->op->getText() == "==") ? (l == r) : (l != r);
        std::cerr << "[FOLD] " << l << " " << ctx->op->getText() << " " << r << " => " << result << "\n";
        return static_cast<int>(result);
    }

    std::string left = leftConst ? std::to_string(std::any_cast<int>(leftAny)) : std::any_cast<std::string>(leftAny);
    std::string right = rightConst ? std::to_string(std::any_cast<int>(rightAny)) : std::any_cast<std::string>(rightAny);
    std::string result = cfg->create_new_tempvar();
    BasicBlock *bb = cfg->current_bb;

    if (ctx->op->getText() == "==")
        bb->add_IRInstr(std::make_unique<IREgal>(bb, result, left, right));
    else
        bb->add_IRInstr(std::make_unique<IRNotEgal>(bb, result, left, right));

    return result;
}


///////////////////////////////////////////////////////////////////////////////
// XOR bit-à-bit
///////////////////////////////////////////////////////////////////////////////
antlrcpp::Any IRGenVisitor::visitOuExcExpr(ifccParser::OuExcExprContext *ctx)
{
    auto leftAny = visit(ctx->expr(0));
    auto rightAny = visit(ctx->expr(1));

    if (leftAny.type() == typeid(int) && rightAny.type() == typeid(int))
    {
        int l = std::any_cast<int>(leftAny);
        int r = std::any_cast<int>(rightAny);
        return l ^ r;
    }

    std::string left = std::any_cast<std::string>(leftAny);
    std::string right = std::any_cast<std::string>(rightAny);
    std::string result = cfg->create_new_tempvar();
    BasicBlock *bb = cfg->current_bb;

    bb->add_IRInstr(std::make_unique<IRXor>(bb, result, left, right));
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
    auto leftAny = visit(ctx->expr(0));
    auto rightAny = visit(ctx->expr(1));

    if (leftAny.type() == typeid(int) && rightAny.type() == typeid(int))
    {
        int lhs = std::any_cast<int>(leftAny);
        int rhs = std::any_cast<int>(rightAny);
        return lhs | rhs;
    }

    std::string lhs = std::any_cast<std::string>(leftAny);
    std::string rhs = std::any_cast<std::string>(rightAny);
    std::string result = cfg->create_new_tempvar();
    cfg->current_bb->add_IRInstr(std::make_unique<IROr>(cfg->current_bb, result, lhs, rhs));
    return result;
}

///////////////////////////////////////////////////////////////////////////////
// Traitement de l'opérateur logique "&"
///////////////////////////////////////////////////////////////////////////////
antlrcpp::Any IRGenVisitor::visitEtLogExpr(ifccParser::EtLogExprContext *ctx)
{
    auto leftAny = visit(ctx->expr(0));
    auto rightAny = visit(ctx->expr(1));

    if (leftAny.type() == typeid(int) && rightAny.type() == typeid(int))
    {
        int lhs = std::any_cast<int>(leftAny);
        int rhs = std::any_cast<int>(rightAny);
        return lhs & rhs;
    }

    std::string lhs = std::any_cast<std::string>(leftAny);
    std::string rhs = std::any_cast<std::string>(rightAny);
    std::string result = cfg->create_new_tempvar();
    cfg->current_bb->add_IRInstr(std::make_unique<IRAnd>(cfg->current_bb, result, lhs, rhs));
    return result;
}

///////////////////////////////////////////////////////////////////////////////
// Traitement du "if - else"
///////////////////////////////////////////////////////////////////////////////
antlrcpp::Any IRGenVisitor::visitIf_stmt(ifccParser::If_stmtContext *ctx)
{

    // Conserver le bloc courant
    BasicBlock *currentBB = cfg->current_bb;

    // 1. Évaluer la condition et obtenir son temporary
    currentBB->test_var_name = std::any_cast<std::string>(this->visit(ctx->expr()));

    // 2. Créer les BasicBlocks pour la branche then, la branche else et le bloc de fusion (merge) pour cet if
    BasicBlock *thenBB = new BasicBlock(cfg, cfg->new_BB_name());
    thenBB->label += "_then";
    BasicBlock *elseBB = new BasicBlock(cfg, cfg->new_BB_name());
    elseBB->label += "_else";
    BasicBlock *mergeBB = new BasicBlock(cfg, cfg->new_BB_name());
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
    this->visit(ctx->block(0)); // Traiter le bloc then

    // 5. Générer le code pour la branche else.
    cfg->add_bb(elseBB);
    cfg->current_bb = elseBB;
    if (ctx->block().size() > 1)
    {
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
antlrcpp::Any IRGenVisitor::visitEtParExpr(ifccParser::EtParExprContext *ctx)
{
    BasicBlock *bb = cfg->current_bb;

    auto leftAny = this->visit(ctx->expr(0));
    bool leftIsConst = leftAny.type() == typeid(int);
    std::string left = leftIsConst ? std::to_string(std::any_cast<int>(leftAny))
                                   : std::any_cast<std::string>(leftAny);

    auto rightAny = this->visit(ctx->expr(1));
    bool rightIsConst = rightAny.type() == typeid(int);
    std::string right = rightIsConst ? std::to_string(std::any_cast<int>(rightAny))
                                     : std::any_cast<std::string>(rightAny);

    std::string result = cfg->create_new_tempvar();
    std::string zero = cfg->create_new_tempvar();
    bb->add_IRInstr(std::make_unique<IRLdConst>(bb, zero, "0"));

    // Création des blocs
    BasicBlock *evalRight = new BasicBlock(cfg, cfg->new_BB_name());
    BasicBlock *end = new BasicBlock(cfg, cfg->new_BB_name());

    // Générer les branches
    bb->exit_true = evalRight;
    bb->exit_false = end;

    // Corps du bloc right
    cfg->add_bb(evalRight);
    cfg->current_bb = evalRight;
    evalRight->add_IRInstr(std::make_unique<IRCopy>(evalRight, result, right));

    // Bloc de fin
    cfg->add_bb(end);
    cfg->current_bb = end;
    end->add_IRInstr(std::make_unique<IRCopy>(end, result, zero));

    return result;
}


///////////////////////////////////////////////////////////////////////////////
// Traitement du "while"
///////////////////////////////////////////////////////////////////////////////

antlrcpp::Any IRGenVisitor::visitOuParExpr(ifccParser::OuParExprContext *ctx)
{
    BasicBlock *bb = cfg->current_bb;

    auto leftAny = this->visit(ctx->expr(0));
    bool leftIsConst = leftAny.type() == typeid(int);
    std::string left = leftIsConst ? std::to_string(std::any_cast<int>(leftAny))
                                   : std::any_cast<std::string>(leftAny);

    auto rightAny = this->visit(ctx->expr(1));
    bool rightIsConst = rightAny.type() == typeid(int);
    std::string right = rightIsConst ? std::to_string(std::any_cast<int>(rightAny))
                                     : std::any_cast<std::string>(rightAny);

    std::string result = cfg->create_new_tempvar();
    std::string one = cfg->create_new_tempvar();
    bb->add_IRInstr(std::make_unique<IRLdConst>(bb, one, "1"));

    // Création des blocs
    BasicBlock *evalRight = new BasicBlock(cfg, cfg->new_BB_name());
    BasicBlock *end = new BasicBlock(cfg, cfg->new_BB_name());

    // Générer le saut conditionnel : si left ≠ 0 on va à end, sinon on évalue right
    bb->exit_true = end;
    bb->exit_false = evalRight;

    // Bloc qui évalue right
    cfg->add_bb(evalRight);
    cfg->current_bb = evalRight;
    evalRight->add_IRInstr(std::make_unique<IRCopy>(evalRight, result, right));

    // Bloc final
    cfg->add_bb(end);
    cfg->current_bb = end;
    end->add_IRInstr(std::make_unique<IRCopy>(end, result, one));

    return result;
}


antlrcpp::Any IRGenVisitor::visitWhile_stmt(ifccParser::While_stmtContext *ctx)
{
    BasicBlock *currentBB = cfg->current_bb;

    BasicBlock *condBB = new BasicBlock(cfg, cfg->new_BB_name());
    condBB->label += "_cond";
    BasicBlock *bodyBB = new BasicBlock(cfg, cfg->new_BB_name());
    bodyBB->label += "_body";
    BasicBlock *exitBB = new BasicBlock(cfg, cfg->new_BB_name());
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
