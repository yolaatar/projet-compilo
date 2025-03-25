#include "IR.h"

IRInstr::IRInstr(BasicBlock* bb_, Operation op, Type t, vector<string> params) : bb(bb_), op(op), t(t), params(params){}

void IRInstr::gen_asm(ostream &o){
    // TODO : generate x86 assembly code generation for this IR instruction
}


BasicBlock::BasicBlock(CFG* cfg, string entry_label) : cfg(cfg), label(entry_label){}

void BasicBlock::gen_asm(ostream &o){
    // TODO : x86 assembly code generation for this basic block (very simple)
}

void BasicBlock::add_IRInstr(IRInstr::Operation op, Type t, vector<string> params){

}


CFG::CFG(DefFonction* ast) : ast(ast){}

void CFG::gen_asm(ostream& o){

}

string CFG::IR_reg_to_asm(string reg){

} /**< helper method: inputs a IR reg or input variable, returns e.g. "-24(%rbp)" for the proper value of 24 */

void CFG::gen_asm_prologue(ostream& o){

}

void CFG::gen_asm_epilogue(ostream& o){

}

void CFG::add_to_symbol_table(string name, Type t){

}

string CFG::create_new_tempvar(Type t){

}

int CFG::get_var_index(string name){

}

Type CFG::get_var_type(string name){

}

string CFG::new_BB_name(){
    
}
