#include "IRGenVisitor.h"
#include <iostream>
#include <cstdlib>
#include <string>

using namespace std;

// Offset de base pour la zone de variables sur la pile
static const int baseOffset = 8;

///////////////////////////////////////////////////////////////////////////////
// Génération de code pour la fonction main (règle prog)
///////////////////////////////////////////////////////////////////////////////
antlrcpp::Any IRGenVisitor::visitProg(ifccParser::ProgContext* ctx)
{
    // Création du BasicBlock d'entrée
    BasicBlock* entryBB = new BasicBlock(cfg, cfg->new_BB_name());
    cfg->add_bb(entryBB);
    cfg->current_bb = entryBB;

    // Visite de tous les statements pour construire le corps du CFG
    for (auto stmtCtx : ctx->stmt()) {
        this->visit(stmtCtx);
    }

    // On peut ici ajouter des instructions d'épilogue IR si nécessaire.
    return 0;
}


///////////////////////////////////////////////////////////////////////////////
// Traitement des déclarations
// "int x;" ou "int x = expr;"
///////////////////////////////////////////////////////////////////////////////
antlrcpp::Any IRGenVisitor::visitDecl(ifccParser::DeclContext* ctx)
{
    std::string varName = ctx->ID()->getText();
    // On suppose ici que le CFG possède une méthode create_new_tempvar pour générer un nouveau temporary,
    // mais pour une déclaration, on utilise directement le nom de la variable.
    
    if(ctx->expr() != nullptr) {
        // Si une initialisation est présente, évaluer l'expression et récupérer le temporary résultant.
        std::string temp = this->visit(ctx->expr()).as<std::string>();
        // Créer une instruction de copie : varName <- temp
        auto instr = std::make_unique<IRInstr>(
            std::make_unique<IRCopy>(varName, temp, 32)
        );
        cfg->current_bb->add_IRInstr(std::move(instr));
    } else {
        // Sans initialisation, on initialise à 0.
        auto instr = std::make_unique<IRInstr>(
            std::make_unique<IRLdConst>(varName, "0", 32)
        );
        cfg->current_bb->add_IRInstr(std::move(instr));
    }
    // On retourne le nom de la variable comme résultat IR de la déclaration.
    return varName;
}


///////////////////////////////////////////////////////////////////////////////
// Traitement des affectations : "x = expr;"
///////////////////////////////////////////////////////////////////////////////
antlrcpp::Any IRGenVisitor::visitAssignment(ifccParser::AssignmentContext* ctx)
{
    std::string varName = ctx->ID()->getText();
    // Évaluer l'expression de droite et récupérer le temporary (ou le nom de variable) contenant le résultat.
    std::string temp = this->visit(ctx->expr()).as<std::string>();
    
    // Créer une instruction IRCopy pour assigner la valeur de temp à varName.
    auto instr = std::make_unique<IRInstr>(
         std::make_unique<IRCopy>(varName, temp, 32)
    );
    cfg->current_bb->add_IRInstr(std::move(instr));
    
    return varName; // On retourne le nom de la variable.
}


///////////////////////////////////////////////////////////////////////////////
// Traitement de l'instruction de retour : "return expr;"
///////////////////////////////////////////////////////////////////////////////
antlrcpp::Any IRGenVisitor::visitReturn_stmt(ifccParser::Return_stmtContext* ctx)
{
    // Évaluer l'expression à retourner (le résultat sera dans w0)
    this->visit(ctx->expr());
    // Le résultat en w0 sera utilisé comme code de retour
    return 0;
}

///////////////////////////////////////////////////////////////////////////////
// Traitement de l'expression additive : gère '+' et '-'
// Règle : additiveExpr : multiplicativeExpr ( ('+' | '-') multiplicativeExpr )* ;
///////////////////////////////////////////////////////////////////////////////
antlrcpp::Any IRGenVisitor::visitAdditiveExpr(ifccParser::AdditiveExprContext* ctx) 
{
    // On suppose que la règle est :
    // additiveExpr : multiplicativeExpr ( ('+' | '-') multiplicativeExpr )* ;

    // On évalue le premier terme et on récupère son temporary
    std::string tempResult = this->visit(ctx->multiplicativeExpr(0)).as<std::string>();

    // Pour chaque terme suivant, on combine avec l'opération appropriée.
    int termCount = ctx->multiplicativeExpr().size();
    for (int i = 1; i < termCount; i++) {
         // Récupérer l'opérateur à l'indice (2*i - 1)
         std::string op = ctx->children[2 * i - 1]->getText();
         // Évaluer le ième terme et obtenir son temporary
         std::string tempOperand = this->visit(ctx->multiplicativeExpr(i)).as<std::string>();

         // Créer un nouveau temporary pour stocker le résultat de l'opération
         std::string tempNew = cfg->create_new_tempvar(32);

         if(op == "+") {
             auto instr = std::make_unique<IRInstr>(
                 std::make_unique<IRAdd>(tempNew, tempResult, tempOperand, 32)
             );
             cfg->current_bb->add_IRInstr(std::move(instr));
         } else if(op == "-") {
             // Vous auriez une instruction IRSub similaire
             auto instr = std::make_unique<IRInstr>(
                 std::make_unique<IRSub>(tempNew, tempResult, tempOperand, 32)
             );
             cfg->current_bb->add_IRInstr(std::move(instr));
         }
         // Mettre à jour le résultat intermédiaire
         tempResult = tempNew;
    }
    // Retourner le temporary contenant le résultat final
    return tempResult;
}


///////////////////////////////////////////////////////////////////////////////
// Traitement de l'expression multiplicative : gère '*' et '/'
// Règle : multiplicativeExpr : primaryExpr ( ('*' | '/') primaryExpr )* ;
///////////////////////////////////////////////////////////////////////////////
antlrcpp::Any IRGenVisitor::visitMultiplicativeExpr(ifccParser::MultiplicativeExprContext* ctx)
{
    // On suppose que la règle est :
    // multiplicativeExpr : primaryExpr ( ('*' | '/') primaryExpr )* ;
    std::string tempResult = this->visit(ctx->primaryExpr(0)).as<std::string>();

    int factorCount = ctx->primaryExpr().size();
    for (int i = 1; i < factorCount; i++) {
         std::string op = ctx->children[2 * i - 1]->getText();
         std::string tempOperand = this->visit(ctx->primaryExpr(i)).as<std::string>();

         std::string tempNew = cfg->create_new_tempvar(32);

         if(op == "*") {
             auto instr = std::make_unique<IRInstr>(
                 std::make_unique<IRMul>(tempNew, tempResult, tempOperand, 32)
             );
             cfg->current_bb->add_IRInstr(std::move(instr));
         } else if(op == "/") {
             // Créez une instruction IRDiv similaire (non montrée ici)
         }
         tempResult = tempNew;
    }
    return tempResult;
}



///////////////////////////////////////////////////////////////////////////////
// Traitement d'une constante (étiquette ConstExpr)
///////////////////////////////////////////////////////////////////////////////
antlrcpp::Any IRGenVisitor::visitConstExpr(ifccParser::ConstExprContext* ctx) 
{
    int value = std::stoi(ctx->CONST()->getText());
    // Créer un temporary pour contenir le résultat (vous pouvez adapter la méthode create_new_tempvar)
    std::string temp = cfg->create_new_tempvar(32);

    // Créer une instruction IRLdConst pour charger la constante dans temp
    auto instr = std::make_unique<IRInstr>(
        std::make_unique<IRLdConst>(temp, std::to_string(value), 32)
    );
    cfg->current_bb->add_IRInstr(std::move(instr));

    // Retourner le nom du temporary pour usage ultérieur (vous pouvez choisir de retourner une valeur via antlrcpp::Any)
    return temp;
}


///////////////////////////////////////////////////////////////////////////////
// Traitement d'une variable (étiquette ExprVariable)
///////////////////////////////////////////////////////////////////////////////
antlrcpp::Any IRGenVisitor::visitExprVariable(ifccParser::ExprVariableContext* ctx) 
{
    std::string varName = ctx->ID()->getText();
    // Ici, on peut aussi générer une instruction "load" si votre IR le nécessite,
    // mais dans cette version, on suppose que le nom de la variable est directement exploitable.
    return varName;
}


///////////////////////////////////////////////////////////////////////////////
// Traitement d'une expression entre parenthèses (étiquette ParensExpr)
///////////////////////////////////////////////////////////////////////////////
antlrcpp::Any IRGenVisitor::visitParensExpr(ifccParser::ParensExprContext* ctx)
{
    // Évaluer simplement l'expression contenue entre parenthèses
    return this->visit(ctx->expr());
}
