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

    if (valAny.type() == typeid(int))
    {
        int val = std::any_cast<int>(valAny);
        temp = cfg->create_new_tempvar();
        bb->add_IRInstr(std::make_unique<IRLdConst>(bb, temp, std::to_string(val)));
        std::cerr << "[FOLD RETURN] return " << val << "\n";
    }
    else
    {
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
    return std::stoi(ctx->CONST()->getText()); // retourne directement un entier ✅
}

///////////////////////////////////////////////////////////////////////////////
// Traitement d'une variable (IdExpr)
///////////////////////////////////////////////////////////////////////////////
antlrcpp::Any IRGenVisitor::visitIdExpr(ifccParser::IdExprContext *ctx)
{
    std::string varName = ctx->ID()->getText();
    return varName;
}

///////////////////////////////////////////////////////////////////////////////
// Traitement de l'opérateur unaire "-"
///////////////////////////////////////////////////////////////////////////////
antlrcpp::Any IRGenVisitor::visitMoinsExpr(ifccParser::MoinsExprContext *ctx)
{
    auto val = visit(ctx->expr());

    if (val.type() == typeid(std::string))
    {
        std::string var = std::any_cast<std::string>(val);
        if (allowConstProp && constMap.find(var) != constMap.end())
            val = constMap[var];
    }

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

    // Propagation via constMap si besoin
    if (leftAny.type() == typeid(std::string))
    {
        std::string var = std::any_cast<std::string>(leftAny);
        if (allowConstProp && constMap.find(var) != constMap.end())
            leftAny = constMap[var];
    }
    if (rightAny.type() == typeid(std::string))
    {
        std::string var = std::any_cast<std::string>(rightAny);
        if (allowConstProp && constMap.find(var) != constMap.end())
            rightAny = constMap[var];
    }

    std::string op = ctx->op->getText();
    bool isLeftConst = leftAny.type() == typeid(int);
    bool isRightConst = rightAny.type() == typeid(int);

    if (allowConstProp && isLeftConst && isRightConst)
    {
        int l = std::any_cast<int>(leftAny);
        int r = std::any_cast<int>(rightAny);
        bool res = false;

        if (op == "<")
            res = l < r;
        else if (op == "<=")
            res = l <= r;
        else if (op == ">")
            res = l > r;
        else if (op == ">=")
            res = l >= r;

        std::cerr << "[FOLD COMP] " << l << " " << op << " " << r << " = " << res << "\n";
        return static_cast<int>(res);
    }

    std::string left, right;

    // Si gauche est une constante mais pas foldée, on génère un LdConst
    if (isLeftConst)
    {
        int val = std::any_cast<int>(leftAny);
        left = cfg->create_new_tempvar();
        cfg->current_bb->add_IRInstr(std::make_unique<IRLdConst>(cfg->current_bb, left, std::to_string(val)));
    }
    else
    {
        left = std::any_cast<std::string>(leftAny);
    }

    if (isRightConst)
    {
        int val = std::any_cast<int>(rightAny);
        right = cfg->create_new_tempvar();
        cfg->current_bb->add_IRInstr(std::make_unique<IRLdConst>(cfg->current_bb, right, std::to_string(val)));
    }
    else
    {
        right = std::any_cast<std::string>(rightAny);
    }

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

    // Détection et remplacement si la variable est constante
    if (leftAny.type() == typeid(std::string))
    {
        std::string var = std::any_cast<std::string>(leftAny);
        if (allowConstProp && constMap.find(var) != constMap.end())
            leftAny = constMap[var];
    }

    if (rightAny.type() == typeid(std::string))
    {
        std::string var = std::any_cast<std::string>(rightAny);
        if (allowConstProp && constMap.find(var) != constMap.end())
            rightAny = constMap[var];
    }

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
        else
        {
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
    else
    {
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

    if (exprVal.type() == typeid(std::string))
    {
        std::string var = std::any_cast<std::string>(exprVal);
        if (allowConstProp && constMap.find(var) != constMap.end())
            exprVal = constMap[var];
    }

    // 🔍 Propagation de constante si possible
    if (exprVal.type() == typeid(int))
    {
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
    std::cerr << "[ASSIGN] " << varName << " = " << ctx->expr()->getText() << std::endl;

    // 🛑 Si RHS utilise la variable → on désactive la propagation avant la visite
    bool usesSelf = ctx->expr()->getText().find(varName) != std::string::npos;
    bool prev = allowConstProp;
    if (usesSelf)
    {
        std::cerr << "[CONSTMAP INVALIDATE] RHS uses " << varName << ", we remove it from constMap\n";
        constMap.erase(varName);
        allowConstProp = false;
    }

    auto val = visit(ctx->expr());

    if (usesSelf)
        allowConstProp = prev;

    std::cerr << "[VAL] type = " << (val.type() == typeid(int) ? "int" : "string") << std::endl;

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
        constMap.erase(varName);
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

    std::cerr << "[AddSubExpr] left = " << ctx->expr(0)->getText()
              << ", right = " << ctx->expr(1)->getText()
              << ", allowConstProp = " << allowConstProp << std::endl;

    if (leftAny.type() == typeid(std::string))
    {
        std::string var = std::any_cast<std::string>(leftAny);
        if (allowConstProp && constMap.find(var) != constMap.end())
            leftAny = constMap[var];
    }

    if (rightAny.type() == typeid(std::string))
    {
        std::string var = std::any_cast<std::string>(rightAny);
        if (allowConstProp && constMap.find(var) != constMap.end())
            rightAny = constMap[var];
    }

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

    std::string left, right;

    if (leftIsConst)
    {
        left = gen_const(std::any_cast<int>(leftAny));
    }
    else if (leftAny.type() == typeid(std::string))
    {
        left = std::any_cast<std::string>(leftAny);
    }
    else
    {
        std::cerr << "[ERROR] Unexpected leftAny type in AddSubExpr: " << leftAny.type().name() << "\n";
        exit(1);
    }

    if (rightIsConst)
    {
        right = gen_const(std::any_cast<int>(rightAny));
    }
    else if (rightAny.type() == typeid(std::string))
    {
        right = std::any_cast<std::string>(rightAny);
    }
    else
    {
        std::cerr << "[ERROR] Unexpected rightAny type in AddSubExpr: " << rightAny.type().name() << "\n";
        exit(1);
    }

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

    // Détection et remplacement si la variable est constante
    if (leftAny.type() == typeid(std::string))
    {
        std::string var = std::any_cast<std::string>(leftAny);
        if (allowConstProp && constMap.find(var) != constMap.end())
            leftAny = constMap[var];
    }
    if (rightAny.type() == typeid(std::string))
    {
        std::string var = std::any_cast<std::string>(rightAny);
        if (allowConstProp && constMap.find(var) != constMap.end())
            rightAny = constMap[var];
    }

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

    // Détection et remplacement si la variable est constante
    if (leftAny.type() == typeid(std::string))
    {
        std::string var = std::any_cast<std::string>(leftAny);
        if (allowConstProp && constMap.find(var) != constMap.end())
            leftAny = constMap[var];
    }
    if (rightAny.type() == typeid(std::string))
    {
        std::string var = std::any_cast<std::string>(rightAny);
        if (allowConstProp && constMap.find(var) != constMap.end())
            rightAny = constMap[var];
    }

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

    // Détection et remplacement si la variable est constante
    if (leftAny.type() == typeid(std::string))
    {
        std::string var = std::any_cast<std::string>(leftAny);
        if (allowConstProp && constMap.find(var) != constMap.end())
            leftAny = constMap[var];
    }
    if (rightAny.type() == typeid(std::string))
    {
        std::string var = std::any_cast<std::string>(rightAny);
        if (allowConstProp && constMap.find(var) != constMap.end())
            rightAny = constMap[var];
    }

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

    // Détection et remplacement si la variable est constante
    if (leftAny.type() == typeid(std::string))
    {
        std::string var = std::any_cast<std::string>(leftAny);
        if (allowConstProp && constMap.find(var) != constMap.end())
            leftAny = constMap[var];
    }
    if (rightAny.type() == typeid(std::string))
    {
        std::string var = std::any_cast<std::string>(rightAny);
        if (allowConstProp && constMap.find(var) != constMap.end())
            rightAny = constMap[var];
    }

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
    BasicBlock *currentBB = cfg->current_bb;

    // 🔧 Corrigé : cast sécurisé avec propagation constante
    bool prev = allowConstProp;
    allowConstProp = false;
    antlrcpp::Any condAny = this->visit(ctx->expr());
    allowConstProp = prev;
    if (condAny.type() == typeid(int))
    {
        int value = std::any_cast<int>(condAny);
        std::string constVar = cfg->create_new_tempvar();
        cfg->current_bb->add_IRInstr(std::make_unique<IRLdConst>(cfg->current_bb, constVar, std::to_string(value)));
        currentBB->test_var_name = constVar;
    }
    else
    {
        currentBB->test_var_name = std::any_cast<std::string>(condAny);
    }

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
    cfg->add_bb(thenBB);
    this->visit(ctx->block(0));

    cfg->add_bb(elseBB);
    cfg->current_bb = elseBB;
    if (ctx->block().size() > 1)
    {
        this->visit(ctx->block(1));
    }

    cfg->add_bb(mergeBB);
    cfg->current_bb = mergeBB;

    return std::string("0");
}

/////////////////////////////////////////////////////////////////////////////
// Traitement de l'opérateur logique "&&"
///////////////////////////////////////////////////////////////////////////////
antlrcpp::Any IRGenVisitor::visitEtParExpr(ifccParser::EtParExprContext *ctx)
{
    bool previous = allowConstProp;
    allowConstProp = false;
    auto leftAny = visit(ctx->expr(0));
    allowConstProp = previous;

    previous = allowConstProp;
    allowConstProp = false;
    auto rightAny = visit(ctx->expr(1));
    allowConstProp = previous;

    // 🔁 Propagation si constante connue
    if (leftAny.type() == typeid(std::string))
    {
        std::string var = std::any_cast<std::string>(leftAny);
        if (constMap.find(var) != constMap.end())
            leftAny = constMap[var];
    }
    if (rightAny.type() == typeid(std::string))
    {
        std::string var = std::any_cast<std::string>(rightAny);
        if (constMap.find(var) != constMap.end())
            rightAny = constMap[var];
    }

    // 🔥 Fold si les deux sont des constantes
    if (leftAny.type() == typeid(int) && rightAny.type() == typeid(int))
    {
        int l = std::any_cast<int>(leftAny);
        int r = std::any_cast<int>(rightAny);
        int res = (l != 0) && (r != 0) ? 1 : 0;
        std::cerr << "[FOLD &&] " << l << " && " << r << " = " << res << "\n";
        return res;
    }

    // ⚡ Fold court-circuit gauche == 0
    if (leftAny.type() == typeid(int))
    {
        int l = std::any_cast<int>(leftAny);
        if (l == 0)
        {
            std::cerr << "[FOLD &&] " << l << " && ? = 0\n";
            return 0;
        }
    }

    // Génération IR
    BasicBlock *bb = cfg->current_bb;
    std::string result = cfg->create_new_tempvar();
    std::string left = leftAny.type() == typeid(int)
                           ? std::to_string(std::any_cast<int>(leftAny))
                           : std::any_cast<std::string>(leftAny);
    std::string zero = cfg->create_new_tempvar();
    bb->add_IRInstr(std::make_unique<IRLdConst>(bb, zero, "0"));

    // Création des blocs
    BasicBlock *evalRight = new BasicBlock(cfg, cfg->new_BB_name());
    BasicBlock *end = new BasicBlock(cfg, cfg->new_BB_name());

    bb->exit_true = evalRight;
    bb->exit_false = end;
    bb->test_var_name = left;

    // ------ Bloc droite ------
    cfg->add_bb(evalRight);
    cfg->current_bb = evalRight;

    std::string right = rightAny.type() == typeid(int)
                            ? std::to_string(std::any_cast<int>(rightAny))
                            : std::any_cast<std::string>(rightAny);

    evalRight->add_IRInstr(std::make_unique<IRCopy>(evalRight, result, right));
    evalRight->exit_true = end;

    // ------ Bloc de fin ------
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

    bool previous = allowConstProp;
    allowConstProp = false;
    auto leftAny = visit(ctx->expr(0));
    allowConstProp = previous;

    previous = allowConstProp;
    allowConstProp = false;
    auto rightAny = visit(ctx->expr(1));
    allowConstProp = previous;

    // Propagation si gauche est une variable constante
    if (leftAny.type() == typeid(std::string))
    {
        std::string var = std::any_cast<std::string>(leftAny);
        if (constMap.find(var) != constMap.end())
            leftAny = constMap[var];
    }
    if (rightAny.type() == typeid(std::string))
    {
        std::string var = std::any_cast<std::string>(rightAny);
        if (constMap.find(var) != constMap.end())
            rightAny = constMap[var];
    }

    // 💥 FOLD complet
    if (leftAny.type() == typeid(int) && rightAny.type() == typeid(int))
    {
        int l = std::any_cast<int>(leftAny);
        int r = std::any_cast<int>(rightAny);
        int res = (l != 0) || (r != 0) ? 1 : 0;
        std::cerr << "[FOLD ||] " << l << " || " << r << " = " << res << "\n";
        return res;
    }

    // 🚪 Court-circuit gauche == vrai
    if (leftAny.type() == typeid(int))
    {
        int l = std::any_cast<int>(leftAny);
        if (l != 0)
        {
            std::cerr << "[FOLD ||] " << l << " || ? = 1\n";
            return 1;
        }
    }

    std::string result = cfg->create_new_tempvar();
    std::string left = leftAny.type() == typeid(int)
                           ? std::to_string(std::any_cast<int>(leftAny))
                           : std::any_cast<std::string>(leftAny);

    std::string one = cfg->create_new_tempvar();
    bb->add_IRInstr(std::make_unique<IRLdConst>(bb, one, "1"));

    // Création des blocs
    BasicBlock *evalRight = new BasicBlock(cfg, cfg->new_BB_name());
    BasicBlock *end = new BasicBlock(cfg, cfg->new_BB_name());

    bb->exit_true = end; // Si gauche est vrai → court-circuit
    bb->exit_false = evalRight;
    bb->test_var_name = left;

    // ------ Bloc droit ------
    cfg->add_bb(evalRight);
    cfg->current_bb = evalRight;

    std::string right = rightAny.type() == typeid(int)
                            ? std::to_string(std::any_cast<int>(rightAny))
                            : std::any_cast<std::string>(rightAny);

    evalRight->add_IRInstr(std::make_unique<IRCopy>(evalRight, result, right));
    evalRight->exit_true = end;

    // ------ Bloc fin ------
    cfg->add_bb(end);
    cfg->current_bb = end;
    end->add_IRInstr(std::make_unique<IRCopy>(end, result, one));

    return result;
}

antlrcpp::Any IRGenVisitor::visitWhile_stmt(ifccParser::While_stmtContext *ctx)
{
    BasicBlock *currentBB = cfg->current_bb;

    BasicBlock *condBB = new BasicBlock(cfg, cfg->new_BB_name() + "_cond");
    BasicBlock *bodyBB = new BasicBlock(cfg, cfg->new_BB_name() + "_body");
    BasicBlock *exitBB = new BasicBlock(cfg, cfg->new_BB_name() + "_exit");

    currentBB->exit_true = condBB;
    condBB->exit_true = bodyBB;
    condBB->exit_false = exitBB;
    bodyBB->exit_true = condBB;

    cfg->add_bb(condBB);
    cfg->current_bb = condBB;

    // ⛔️ Désactiver temporairement la propagation pour éviter de plier i < 5
    bool prev = allowConstProp;
    allowConstProp = false;

    auto condAny = this->visit(ctx->expr());

    allowConstProp = prev;

    if (condAny.type() == typeid(int))
    {
        std::string tmp = cfg->create_new_tempvar();
        condBB->add_IRInstr(std::make_unique<IRLdConst>(condBB, tmp, std::to_string(std::any_cast<int>(condAny))));
        condBB->test_var_name = tmp;
    }
    else
    {
        condBB->test_var_name = std::any_cast<std::string>(condAny);
    }

    cfg->add_bb(bodyBB);
    cfg->current_bb = bodyBB;
    this->visit(ctx->block());

    cfg->add_bb(exitBB);
    cfg->current_bb = exitBB;

    return std::string("0");
}
