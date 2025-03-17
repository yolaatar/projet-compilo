#include "CodeGenVisitor.h"
#include <iostream>
#include <cstdlib>
#include <string>

using namespace std;

// Offset de base pour la zone de variables sur la pile
static const int baseOffset = 8;

///////////////////////////////////////////////////////////////////////////////
// Génération de code pour la fonction main (règle prog)
///////////////////////////////////////////////////////////////////////////////
antlrcpp::Any CodeGenVisitor::visitProg(ifccParser::ProgContext* ctx) 
{
    // Pour cet exemple, nous allouons un espace fixe (16 octets)
    int allocSize = 16;

    // Prologue : génération des directives et allocation de la pile
    cout << ".section __TEXT,__text,regular,pure_instructions\n";
    cout << ".globl _main\n";
    cout << "_main:\n";
    cout << "    sub sp, sp, #" << allocSize << "\n";
    // (Optionnel) Initialisation de la zone de pile si nécessaire
    cout << "    str wzr, [sp, #12]\n";

    // Visite de tous les statements (déclarations, affectations, retours, etc.)
    for (auto stmtCtx : ctx->stmt()) {
        this->visit(stmtCtx);
    }
    
    // Épilogue : restauration de la pile et retour
    cout << "    add sp, sp, #" << allocSize << "\n";
    cout << "    ret\n";
    return 0;
}

///////////////////////////////////////////////////////////////////////////////
// Traitement des déclarations
// "int x;" ou "int x = expr;"
///////////////////////////////////////////////////////////////////////////////
antlrcpp::Any CodeGenVisitor::visitDecl(ifccParser::DeclContext* ctx)
{
    string varName = ctx->ID()->getText();
    // Récupérer l'offset de la variable via la table des symboles
    int offset = baseOffset + symbolTable[varName].index;
    
    if (ctx->expr() != nullptr) {
        // Si une initialisation est présente, évaluer l'expression
        this->visit(ctx->expr());
        cout << "    str w0, [sp, #" << offset << "]\n";
    } else {
        // Sans initialisation, on initialise à 0
        cout << "    mov w0, #0\n";
        cout << "    str w0, [sp, #" << offset << "]\n";
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////////
// Traitement des affectations : "x = expr;"
///////////////////////////////////////////////////////////////////////////////
antlrcpp::Any CodeGenVisitor::visitAssignment(ifccParser::AssignmentContext* ctx)
{
    string varName = ctx->ID()->getText();
    int offset = baseOffset + symbolTable[varName].index;
    
    // Évaluer l'expression de droite
    this->visit(ctx->expr());
    // Stocker le résultat dans la variable (à l'offset calculé)
    cout << "    str w0, [sp, #" << offset << "]\n";
    return 0;
}

///////////////////////////////////////////////////////////////////////////////
// Traitement de l'instruction de retour : "return expr;"
///////////////////////////////////////////////////////////////////////////////
antlrcpp::Any CodeGenVisitor::visitReturn_stmt(ifccParser::Return_stmtContext* ctx)
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
antlrcpp::Any CodeGenVisitor::visitAdditiveExpr(ifccParser::AdditiveExprContext* ctx) 
{
    // Récupère les termes (sous-expressions de type multiplicativeExpr)
    auto terms = ctx->multiplicativeExpr();
    int termCount = terms.size();
    if(termCount == 0) return 0;  // cas inhabituel

    // Évaluer le premier terme ; résultat dans w0
    this->visit(terms[0]);
    cout << "    mov w8, w0\n";  // sauvegarder dans w8

    // Pour chaque terme suivant (i de 1 à termCount - 1)
    for (int i = 1; i < termCount; i++) {
         // L'opérateur entre le (i-1)ème et le ième terme se trouve à l'indice (2*i - 1)
         std::string op = ctx->children[2 * i - 1]->getText();
         // Évaluer le ième terme ; résultat dans w0
         this->visit(terms[i]);
         // Selon l'opérateur, effectuer l'opération avec la valeur intermédiaire (w8)
         if(op == "+") {
             cout << "    add w8, w8, w0\n";  // w8 = w8 + w0
         } else if(op == "-") {
             cout << "    sub w8, w8, w0\n";  // w8 = w8 - w0
         }
    }
    // Placer le résultat final dans w0
    cout << "    mov w0, w8\n";
    return 0;
}

///////////////////////////////////////////////////////////////////////////////
// Traitement de l'expression multiplicative : gère '*' et '/'
// Règle : multiplicativeExpr : primaryExpr ( ('*' | '/') primaryExpr )* ;
///////////////////////////////////////////////////////////////////////////////
antlrcpp::Any CodeGenVisitor::visitMultiplicativeExpr(ifccParser::MultiplicativeExprContext* ctx)
{
    auto factors = ctx->primaryExpr();
    int factorCount = factors.size();
    if(factorCount == 0) return 0;  // cas inhabituel

    // Évaluer le premier facteur ; résultat dans w0
    this->visit(factors[0]);
    cout << "    mov w8, w0\n";  // sauvegarder dans w8

    // Pour chaque facteur suivant (i de 1 à factorCount - 1)
    for (int i = 1; i < factorCount; i++) {
         // L'opérateur se trouve à l'indice (2*i - 1) parmi les enfants
         std::string op = ctx->children[2 * i - 1]->getText();
         // Évaluer le ième facteur ; résultat dans w0
         this->visit(factors[i]);
         if(op == "*") {
             cout << "    mul w8, w8, w0\n";  // w8 = w8 * w0
         } else if(op == "/") {
             cout << "    sdiv w8, w8, w0\n"; // w8 = w8 / w0
         }
    }
    cout << "    mov w0, w8\n";  // résultat final dans w0
    return 0;
}


///////////////////////////////////////////////////////////////////////////////
// Traitement d'une constante (étiquette ConstExpr)
///////////////////////////////////////////////////////////////////////////////
antlrcpp::Any CodeGenVisitor::visitConstExpr(ifccParser::ConstExprContext* ctx) 
{
    int value = stoi(ctx->CONST()->getText());
    cout << "    mov w0, #" << value << "\n";
    return 0;
}

///////////////////////////////////////////////////////////////////////////////
// Traitement d'une variable (étiquette ExprVariable)
///////////////////////////////////////////////////////////////////////////////
antlrcpp::Any CodeGenVisitor::visitExprVariable(ifccParser::ExprVariableContext* ctx) 
{
    std::string varName = ctx->ID()->getText();
    int offset = baseOffset + symbolTable[varName].index;
    cout << "    ldr w0, [sp, #" << offset << "]\n";
    return 0;
}

///////////////////////////////////////////////////////////////////////////////
// Traitement d'une expression entre parenthèses (étiquette ParensExpr)
///////////////////////////////////////////////////////////////////////////////
antlrcpp::Any CodeGenVisitor::visitParensExpr(ifccParser::ParensExprContext* ctx)
{
    // Évaluer simplement l'expression contenue entre parenthèses
    return this->visit(ctx->expr());
}
