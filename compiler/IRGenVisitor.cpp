#include "IRGenVisitor.h"
#include "IRInstr.h"
#include <iostream>
#include <cstdlib>
#include <string>
#include <any>

using namespace std;

IRGenVisitor::IRGenVisitor()
    : cfg(nullptr), backend(nullptr), functionTable(nullptr), hasReturned(false), tempCpt(1)
{
}

///////////////////////////////////////////////////////////////////////////////
// Traitement de l'instruction de retour : "return expr ;"
///////////////////////////////////////////////////////////////////////////////
antlrcpp::Any IRGenVisitor::visitReturn_stmt(ifccParser::Return_stmtContext *ctx)
{
    if (ctx->expr())
    {
        std::string temp = std::any_cast<std::string>(this->visit(ctx->expr()));
        BasicBlock *bb = cfg->current_bb;  // Get the current bb after visiting expr
        auto instr = std::make_unique<IRReturn>(bb, temp);
        bb->add_IRInstr(std::move(instr));
    }
    else
    {
        BasicBlock *bb = cfg->current_bb;
        auto instr = std::make_unique<IRBranch>(bb, "", cfg->epilogueLabel, "");
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
    std::string uniqueName = cfg->get_stv().addToSymbolTable(varName);

    BasicBlock *bb = cfg->current_bb;
    if (ctx->expr() != nullptr)
    {
        std::string exprTemp = std::any_cast<std::string>(visit(ctx->expr()));
        cfg->current_bb->add_IRInstr(std::move(make_unique<IRCopy>(bb, uniqueName, exprTemp)));
    }
    else
    {
        // Initialisation par défaut à 0
        cfg->current_bb->add_IRInstr(make_unique<IRLdConst>(bb, uniqueName, "0"));
    }
    return uniqueName;
}

///////////////////////////////////////////////////////////////////////////////
// Traitement d'une constante (ConstExpr)
///////////////////////////////////////////////////////////////////////////////
antlrcpp::Any IRGenVisitor::visitConstExpr(ifccParser::ConstExprContext *ctx) {
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

antlrcpp::Any IRGenVisitor::visitBlock(ifccParser::BlockContext *ctx)
{
    cfg->get_stv().enterScope();
    for (auto child : ctx->children)
    {
        visit(child);
    }
    cfg->get_stv().exitScope();
    return nullptr;
}

antlrcpp::Any IRGenVisitor::visitIdExpr(ifccParser::IdExprContext *ctx)
{
    string varName = ctx->ID()->getText();
    string uniqueName = cfg->get_stv().getUniqueName(varName);
    return uniqueName; // Retourne le nom unique (s1_a ou s2_a)
}

///////////////////////////////////////////////////////////////////////////////
// Traitement de l'opérateur unaire "-"
////////
// Remplacer la version actuelle de visitMoinsExpr par :
antlrcpp::Any IRGenVisitor::visitMoinsExpr(ifccParser::MoinsExprContext *ctx) {
    std::string exprTemp = std::any_cast<std::string>(visit(ctx->expr()));
    std::string result = cfg->create_new_tempvar();
    
    // Génère directement 0 - expr
    std::string zeroTemp = cfg->create_new_tempvar();
    cfg->current_bb->add_IRInstr(make_unique<IRLdConst>(cfg->current_bb, zeroTemp, "0"));
    cfg->current_bb->add_IRInstr(make_unique<IRSub>(cfg->current_bb, result, zeroTemp, exprTemp));
    
    return result;
}

antlrcpp::Any IRGenVisitor::visitCompExpr(ifccParser::CompExprContext* ctx) {
    std::string left = std::any_cast<std::string>(visit(ctx->expr(0)));
    std::string right;

    if (ctx->expr(1)->getText() == "5" || ctx->expr(1)->getText() == "15") { // Gestion explicite des constantes
        right = "$" + ctx->expr(1)->getText(); 
    } else {
        right = std::any_cast<std::string>(visit(ctx->expr(1)));
    }

    std::string result = cfg->create_new_tempvar();
    cfg->current_bb->add_IRInstr(
        std::make_unique<IRComp>(cfg->current_bb, result, left, right, ctx->op->getText())
    );
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
    // Initialisation des paramètres formels (si présents)
    int paramIndex = 0;
    if (ctx->decl_params())
    {
        for (auto param : ctx->decl_params()->param())
        {
            std::string paramName = param->ID()->getText();
            std::string uniqueName = cfg->get_stv().addToSymbolTable(paramName);

            // Génère une instruction IRParamLoad qui copie w0/w1/etc. → uniqueName
            cfg->current_bb->add_IRInstr(
                std::make_unique<IRParamLoad>(cfg->current_bb, uniqueName, paramIndex++));
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
    Scope *global = cfg->get_stv().getGlobalScope();
    for (const auto &[_, info] : global->symbols)
    {
        {
            if (info.offset < minOffset)
            {
                minOffset = info.offset;
            }
        }
        cfg->maxOffset = -minOffset;

        return 0;
    }
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
antlrcpp::Any IRGenVisitor::visitAssign(ifccParser::AssignContext *ctx)
{
    std::string varName = ctx->ID()->getText();
    std::string exprTemp = std::any_cast<std::string>(this->visit(ctx->expr()));
    BasicBlock *bb = cfg->current_bb;
    // Récupérer le unique name via getUniqueName
    std::string unique = cfg->get_stv().getUniqueName(varName);
    auto instr = std::make_unique<IRCopy>(bb, unique, exprTemp);
    bb->add_IRInstr(std::move(instr));
    return unique;
}

antlrcpp::Any IRGenVisitor::visitPlusAssign(ifccParser::PlusAssignContext* ctx) {
    return generateCompoundAssign(ctx->ID()->getText(), ctx->expr(), "+");
}

antlrcpp::Any IRGenVisitor::visitMinusAssign(ifccParser::MinusAssignContext* ctx) {
    return generateCompoundAssign(ctx->ID()->getText(), ctx->expr(), "-");
}

antlrcpp::Any IRGenVisitor::visitMulAssign(ifccParser::MulAssignContext* ctx) {
    return generateCompoundAssign(ctx->ID()->getText(), ctx->expr(), "*");
}

antlrcpp::Any IRGenVisitor::visitDivAssign(ifccParser::DivAssignContext* ctx) {
    return generateCompoundAssign(ctx->ID()->getText(), ctx->expr(), "/");
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

    
    // 4. Générer le code pour la branche then.
    cfg->add_bb(thenBB);
    cfg->current_bb = thenBB;
    thenBB->exit_true = mergeBB;  
    this->visit(ctx->block(0));  // Traiter le bloc then
    
    // 5. Générer le code pour la branche else.
    
    cfg->add_bb(elseBB);
    cfg->current_bb = elseBB;
    elseBB->exit_false = mergeBB;
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
    BasicBlock* evalLeftBB = cfg->current_bb;
    std::string left = std::any_cast<std::string>(this->visit(ctx->expr(0)));
    BasicBlock* afterLeftBB = cfg->current_bb;
    std::string result = cfg->create_new_tempvar();
    BasicBlock* setFalseBB = new BasicBlock(cfg, cfg->new_BB_name() + "_setFalse");
    BasicBlock* evalRightBB = new BasicBlock(cfg, cfg->new_BB_name() + "_evalRight");
    BasicBlock* mergeBB = new BasicBlock(cfg, cfg->new_BB_name() + "_merge");
    
    afterLeftBB->test_var_name = left;
    afterLeftBB->exit_true = evalRightBB;  // If left is true, evaluate right
    afterLeftBB->exit_false = setFalseBB;  // If left is false, set result to 0
    
    cfg->add_bb(setFalseBB);
    setFalseBB->add_IRInstr(std::make_unique<IRLdConst>(setFalseBB, result, "0"));
    setFalseBB->exit_true = mergeBB;
    
    cfg->add_bb(evalRightBB);
    cfg->current_bb = evalRightBB;
    std::string right = std::any_cast<std::string>(this->visit(ctx->expr(1)));
    evalRightBB->add_IRInstr(std::make_unique<IRCopy>(evalRightBB, result, right));
    evalRightBB->exit_true = mergeBB;
    
    cfg->add_bb(mergeBB);
    cfg->current_bb = mergeBB;
    
    return result;
}


///////////////////////////////////////////////////////////////////////////////
// Traitement du ou paresseux "||"
///////////////////////////////////////////////////////////////////////////////

antlrcpp::Any IRGenVisitor::visitOuParExpr(ifccParser::OuParExprContext* ctx)
{
    BasicBlock* evalLeftBB = cfg->current_bb;  // Block before evaluating left
    std::string left = std::any_cast<std::string>(this->visit(ctx->expr(0)));
    BasicBlock* afterLeftBB = cfg->current_bb;  // Block after evaluating left
    std::string result = cfg->create_new_tempvar();
    BasicBlock* setTrueBB = new BasicBlock(cfg, cfg->new_BB_name() + "_setTrue");
    BasicBlock* evalRightBB = new BasicBlock(cfg, cfg->new_BB_name() + "_evalRight");
    BasicBlock* mergeBB = new BasicBlock(cfg, cfg->new_BB_name() + "_merge");
    
    // Set the conditional jump in the block after left is evaluated
    afterLeftBB->test_var_name = left;
    afterLeftBB->exit_true = setTrueBB;
    afterLeftBB->exit_false = evalRightBB;
    
    cfg->add_bb(setTrueBB);
    setTrueBB->add_IRInstr(std::make_unique<IRLdConst>(setTrueBB, result, "1"));
    setTrueBB->exit_true = mergeBB;
    
    cfg->add_bb(evalRightBB);
    cfg->current_bb = evalRightBB;
    std::string right = std::any_cast<std::string>(this->visit(ctx->expr(1)));
    evalRightBB->add_IRInstr(std::make_unique<IRCopy>(evalRightBB, result, right));
    evalRightBB->exit_true = mergeBB;
    
    cfg->add_bb(mergeBB);
    cfg->current_bb = mergeBB;
    
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

antlrcpp::Any IRGenVisitor::visitCharExpr(ifccParser::CharExprContext* ctx) {
    char c = ctx->CHAR()->getText()[1];
    int value = static_cast<int>(c);

    std::string temp = cfg->create_new_tempvar();
    BasicBlock *bb = cfg->current_bb;
    bb->add_IRInstr(std::make_unique<IRLdConst>(bb, temp, std::to_string(value)));
    return temp;
}

antlrcpp::Any IRGenVisitor::generateCompoundAssign(const std::string& varName, ifccParser::ExprContext* expr, const std::string& op)
{
    std::string exprTemp = std::any_cast<std::string>(this->visit(expr));
    std::string unique = cfg->get_stv().getUniqueName(varName);
    std::string result = cfg->create_new_tempvar();
    BasicBlock* bb = cfg->current_bb;

    if (op == "+") {
        bb->add_IRInstr(std::make_unique<IRAdd>(bb, result, unique, exprTemp));
    } else if (op == "-") {
        bb->add_IRInstr(std::make_unique<IRSub>(bb, result, unique, exprTemp));
    } else if (op == "*") {
        bb->add_IRInstr(std::make_unique<IRMul>(bb, result, unique, exprTemp));
    } else if (op == "/") {
        bb->add_IRInstr(std::make_unique<IRDiv>(bb, result, unique, exprTemp));
    } else {
        std::cerr << "[ERROR] Unknown compound assignment operator: " << op << "\n";
        exit(1);
    }

    bb->add_IRInstr(std::make_unique<IRCopy>(bb, unique, result));
    return unique;
}

