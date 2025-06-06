#ifndef IR_H
#define IR_H

#include <vector>
#include <string>
#include <iostream>
#include <initializer_list>
#include <ostream>
#include <map>
#include <memory>

#include "SymbolTableVisitor.h"
#include "CodeGenBackend.h"
#include "IRInstr.h"
class CFG;
extern CodeGenBackend* codegenBackend;

class CFG;
class BasicBlock;



class DefFonction {
public:
    DefFonction(const std::string &name, const std::vector<std::string> &params = {})
        : name(name), params(params) {}

    std::string name;
    std::vector<std::string> params;

    void print(std::ostream &os) const;
};


/*---------------------------------------------------
 * BasicBlock : Bloc basique d'instructions IR
 *---------------------------------------------------*/
class BasicBlock {
public:
    BasicBlock(CFG* cfg, std::string entry_label);
    void gen_asm(std::ostream &o);
    void add_IRInstr(std::unique_ptr<IRInstr> instr);
    void print_instrs() const;

    BasicBlock* exit_true;
    BasicBlock* exit_false;
    std::string label;
    CFG* cfg;
    std::vector<std::unique_ptr<IRInstr>> instrs;
    std::string test_var_name; 
};

/*---------------------------------------------------
 * CFG : Control Flow Graph
 *---------------------------------------------------*/
class CFG {
public:
    CFG(DefFonction* ast, SymbolTableVisitor& stv);

    DefFonction* ast;
    BasicBlock* current_bb;
    int maxOffset = 0;
    std::string epilogueLabel;
    bool usesGetChar = false;
    bool usesPutChar = false;
    void add_bb(BasicBlock* bb);
    std::string IR_reg_to_asm(std::string reg);
    void gen_asm(std::ostream& o);
    void gen_asm_prologue(std::ostream& o);
    void gen_asm_epilogue(std::ostream& o);
    SymbolTableVisitor& get_stv() ;
    std::string create_new_tempvar();
    std::string new_BB_name();    

private:
    SymbolTableVisitor stv;
    int nextBBnumber;
    std::vector<BasicBlock*> bbs;
};

#endif
