#include "CodeGenVisitor.h"
#include "SymbolTableVisitor.h"

CodeGenVisitor::CodeGenVisitor(SymbolTableVisitor stv){
    this->stv = stv;
}

antlrcpp::Any CodeGenVisitor::visitProg(ifccParser::ProgContext *ctx) 
{
    std::cout<< ".globl main\n" ;
    std::cout<< " main: \n" ;
    std::cout << "    pushq %rbp\n";
    std::cout << "    movq %rsp, %rbp\n";

    visitChildren(ctx);
    
    std::cout << "    ret\n";

    return 0;
}


antlrcpp::Any CodeGenVisitor::visitReturn_stmt(ifccParser::Return_stmtContext *ctx)
{
    visitChildren(ctx);

    std::cout << "    popq %rbp\n";

    return 0;
}

antlrcpp::Any CodeGenVisitor::visitAddSubExpr(ifccParser::AddSubExprContext *ctx) {
    visit(ctx->expr(0));

    std::string tmp = newTemp();
    int offset = stv.symbolTable[tmp].offset;
    std::cout << "    movl %eax, "<<offset<<"(%rbp)\n";

    visit(ctx->expr(1));

    if (ctx->op->getText() == "+"){
        std::cout << "    addl "<< offset <<"(%rbp), %eax\n";
    } else if (ctx->op->getText() == "-") {
        std::cout << "    subl %eax, "<< offset <<"(%rbp)\n";
        std::cout << "    movl "<< offset <<"(%rbp), %eax\n";
    } else {
        std::cout << "\nImpossible case, exit\n";
        exit(-1);
    }

    return 0;
}


antlrcpp::Any CodeGenVisitor::visitMulDivExpr(ifccParser::MulDivExprContext *ctx) {
    // Parcours les sous-expressions gauche et droite
    this->visit(ctx->expr(0));  // Première expression (gauche)
    std::string tmp = newTemp();
    int offset = stv.symbolTable[tmp].offset;
    if (ctx->op->getText()=="*"){ //MULTIPLICATION
        std::cout << "    movl %eax, "<< std::to_string(offset)<<"(%rbp)  \n";

        this->visit(ctx->expr(1));  // Deuxième expression (droite)
        std::cout << "    imull "<< std::to_string(offset)<<"(%rbp), %eax\n";
    }
    else if (ctx->op->getText()=="/"){ //DIVISION
        std::cout << "    movl %eax, "<< std::to_string(offset)<<"(%rbp)  \n";

        this->visit(ctx->expr(1));  // Deuxième expression (droite)
        std::string tmp2 = newTemp();
        int offset2 = stv.symbolTable[tmp2].offset;
        std::cout << "    movl %eax, "<< std::to_string(offset2)<<"(%rbp)  \n";
        std::cout << "    movl "<< std::to_string(offset)<<"(%rbp), %eax  \n"; // je met le dividende dans le eax (la division en assembleur est comme ça)
        std::cout << "    cltd\n";  // Signe-étendre EAX en EDX:EAX
        std::cout << "    idivl "<< std::to_string(offset2)<<"(%rbp)\n";
    }
    else if (ctx->op->getText()=="%"){ // MODULO
        std::cout << "    movl %eax, "<< std::to_string(offset)<<"(%rbp)  \n";

        this->visit(ctx->expr(1));  // Deuxième expression (droite)
        std::string tmp2 = newTemp();
        int offset2 = stv.symbolTable[tmp2].offset;
        std::cout << "    movl %eax, "<< std::to_string(offset2)<<"(%rbp)  \n";
        std::cout << "    movl "<< std::to_string(offset)<<"(%rbp), %eax  \n"; // je met le dividende dans le eax (la division en assembleur est comme ça)
        std::cout << "    cltd\n";  // Signe-étendre EAX en EDX:EAX
        std::cout << "    idivl "<< std::to_string(offset2)<<"(%rbp)\n";
        std::cout << "    movl %edx, %eax  \n";
    }
    
    return 0;
}

antlrcpp::Any CodeGenVisitor::visitIdExpr(ifccParser::IdExprContext *ctx){
    std::string varName = ctx->ID()->getText();
    int offset = stv.symbolTable[varName].offset;

    std::cout << "    movl "<<offset<<"(%rbp), %eax\n";

    return 0;
}

antlrcpp::Any CodeGenVisitor::visitDecl(ifccParser::DeclContext *ctx){
    if(ctx->expr() != nullptr){
        std::string varName = ctx->ID()->getText();
        int offset = stv.symbolTable[varName].offset;

        visit(ctx->expr());

        std::cout << "    movl %eax, "<<offset<<"(%rbp)\n";
    }

    return 0;
}

antlrcpp::Any CodeGenVisitor::visitConstExpr(ifccParser::ConstExprContext *ctx){
    int val = stoi(ctx->CONST()->getText());

    std::cout << "    movl $"<<val<<", %eax\n";

    return 0;
}

antlrcpp::Any CodeGenVisitor::visitAssignment(ifccParser::AssignmentContext *ctx){
    std::string varName = ctx->ID()->getText();
    int offset = stv.symbolTable[varName].offset;

    visit(ctx->expr());

    std::cout << "    movl %eax, "<<offset<<"(%rbp)\n";

    return 0;
}

std::string CodeGenVisitor::newTemp() {
    while (stv.symbolTable.find("tmp"+std::to_string(tempCpt)) != stv.symbolTable.end())
    {
        tempCpt++;
    }

    std::string varName = "tmp" + std::to_string(tempCpt);
    stv.addToSymbolTable(varName);

    return "tmp" + std::to_string(tempCpt++);
}