#ifndef IR_H
#define IR_H

#include <vector>
#include <string>
#include <iostream>
#include <initializer_list>
#include <ostream>
#include <map>
#include <memory>
#include "IROperation.h"

//-----------------------------------------------------
// Définition de DefFonction
//-----------------------------------------------------
class DefFonction {
public:
    std::string name;
    std::vector<std::string> params;
    DefFonction(const std::string &name, const std::vector<std::string> &params = {})
        : name(name), params(params) {}
    void print(std::ostream &os) const {
        os << "Function: " << name << "(";
        for (size_t i = 0; i < params.size(); i++) {
            os << params[i];
            if (i < params.size() - 1)
                os << ", ";
        }
        os << ")";
    }
};

class BasicBlock;
class CFG;

/*---------------------------------------------------
 * IRInstr : Instruction IR contenant une opération
 *---------------------------------------------------*/
class IRInstr {
public:
    IRInstr(std::unique_ptr<IROperation> op_)
        : op(std::move(op_)) {}

    void gen_asm(std::ostream &o) {
        op->gen_asm(o);
    }
private:
    std::unique_ptr<IROperation> op;
};

/*---------------------------------------------------
 * BasicBlock : Bloc basique d'instructions IR
 *---------------------------------------------------*/
class BasicBlock {
public:
    BasicBlock(CFG* cfg, std::string entry_label)
        : cfg(cfg), label(entry_label), exit_true(nullptr), exit_false(nullptr) {}

    void gen_asm(std::ostream &o) {
        o << label << ":\n";
        for (auto &instr : instrs) {
            instr->gen_asm(o);
        }
    }

    void add_IRInstr(std::unique_ptr<IRInstr> instr) {
        instrs.push_back(std::move(instr));
    }

    BasicBlock* exit_true;
    BasicBlock* exit_false;
    std::string label;
    CFG* cfg;
    std::vector<std::unique_ptr<IRInstr>> instrs;
};

/*---------------------------------------------------
 * CFG : Control Flow Graph
 *---------------------------------------------------*/
class CFG {
public:
    CFG(DefFonction* ast)
        : ast(ast), nextFreeSymbolIndex(0), nextBBnumber(0), current_bb(nullptr) {}

    DefFonction* ast;
    void add_bb(BasicBlock* bb) {
        bbs.push_back(bb);
        current_bb = bb;
    }

    void gen_asm(std::ostream& o) {
        gen_asm_prologue(o);
        for (auto bb : bbs) {
            bb->gen_asm(o);
        }
        gen_asm_epilogue(o);
    }

    // Exemples d'implémentation, à adapter :
    void gen_asm_prologue(std::ostream& o) {
        //o << ".section __TEXT,__text,regular,pure_instructions\n";
        o << ".globl _" << ast->name << "\n";
        o << "_" << ast->name << ":\n";
        // Allouer la pile, etc.
        o << "    sub sp, sp, #16\n";
    }
    void gen_asm_epilogue(std::ostream& o) {
        o << "    add sp, sp, #16\n";
        o << "    ret\n";
    }

    void add_to_symbol_table(std::string name, size_t t) {
        symbolSizes[name] = t;
        symbolIndex[name] = nextFreeSymbolIndex++;
    }
    std::string create_new_tempvar(size_t t) {
        std::string temp = "w" + std::to_string(nextFreeSymbolIndex);
        add_to_symbol_table(temp, t);
        return temp;
    }
    int get_var_index(std::string name) {
        return symbolIndex[name];
    }
    size_t get_var_type(std::string name) {
        return symbolSizes[name];
    }

    std::string new_BB_name() {
        return "BB" + std::to_string(nextBBnumber++);
    }

    BasicBlock* current_bb;

private:
    std::map<std::string, size_t> symbolSizes;
    std::map<std::string, int> symbolIndex;
    int nextFreeSymbolIndex;
    int nextBBnumber;
    std::vector<BasicBlock*> bbs;
};

#endif
