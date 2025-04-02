#include "SymbolTableVisitor.h"
#include "IROperation.h"

antlrcpp::Any SymbolTableVisitor::visitDecl(ifccParser::DeclContext *ctx)
{
    std::string varName = ctx->ID()->getText();

    if(symbolTable.find(varName) != symbolTable.end()){
        writeWarning(varName+"is already defined");
    } else {
        addToSymbolTable(varName);
    }

    if(ctx->expr() != nullptr){
        symbolTable[varName].initialised = true;
        visit(ctx->expr());
    } 

    return 0;
}

antlrcpp::Any SymbolTableVisitor::visitAssignment(ifccParser::AssignmentContext *ctx)
{
    std::string varName = ctx->ID()->getText();

    if(symbolTable.find(varName) == symbolTable.end()){
        writeError(varName+" is not defined");
        exit(EXIT_FAILURE);
    } else {
        symbolTable[varName].initialised = true;
        visit(ctx->expr());
    }

    return 0;
}

antlrcpp::Any SymbolTableVisitor::visitIdExpr(ifccParser::IdExprContext *ctx){
    std::string varName = ctx->ID()->getText();

    if(symbolTable.find(varName) == symbolTable.end()){
        writeError(varName+" is not defined");
        exit(EXIT_FAILURE);
    } else {
        if(!symbolTable[varName].initialised){
            writeError(varName+" is not initialised");
            exit(EXIT_FAILURE);
        } else {
            symbolTable[varName].used = true;
        }
    }

    return 0;
}

antlrcpp::Any SymbolTableVisitor::visitReturn_stmt(ifccParser::Return_stmtContext *ctx){
    visitChildren(ctx);

    checkSymbolTable();

    return 0;
}

void SymbolTableVisitor::addToSymbolTable(std::string s){
    if(symbolTable.find(s) != symbolTable.end()){
        writeWarning(s+" is already defined");
    } else {
        symbolTable[s] = SymbolTableStruct{};
        symbolTable[s].offset = -offset;
        offset += INTSIZE;
    }
}

std::string SymbolTableVisitor::createNewTemp() {
    std::string prefix = codegenBackend->getTempPrefix();
    std::string temp = prefix + std::to_string(offset);
    addToSymbolTable(temp);
    return temp;
}

void SymbolTableVisitor::checkSymbolTable(){
    for(auto const& item : symbolTable){
        if(!item.second.initialised || !item.second.used){
            writeWarning(item.first+" is defined but not used");
        }
    }
}


void SymbolTableVisitor::writeWarning(std::string message){
    warning++;
    std::cerr << "[WARNING] " << message << "\n";
}

void SymbolTableVisitor::writeError(std::string message){
    error++;
    std::cerr << "[ERROR] " << message << "\n";
}