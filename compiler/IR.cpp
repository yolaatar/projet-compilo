#include "IR.h"
#include "IRInstr.h"

/**
 * DefFonction
 */
void DefFonction::print(std::ostream &os) const {
    os << "Function: " << name << "(";
    for (size_t i = 0; i < params.size(); i++) {
        os << params[i];
        if (i < params.size() - 1)
            os << ", ";
    }
    os << ")";
}


/**
 * BasicBlock
 */
void BasicBlock::gen_asm(std::ostream &o) {
    o << label << ":\n";
    for (auto &instr : instrs) {
        instr->gen_asm(o);
    }
}

void BasicBlock::add_IRInstr(std::unique_ptr<IRInstr> instr) {
    instrs.push_back(std::move(instr));
}

void BasicBlock::print_instrs() const {
    std::cerr << "=== Instructions in BasicBlock [" << label << "] ===\n";
    for (const auto &instr : instrs) {
        std::cerr << "  - " << typeid(*instr).name() << "(";
        std::vector<std::string> params = instr->getParams();
        for (size_t i = 0; i < params.size(); ++i) {
            std::cerr << params[i];
            if (i + 1 < params.size())
                std::cerr << ", ";
        }
        std::cerr << ")\n";
    }
    std::cerr << "========================================\n";
}

/**
 * CFG
 */
CFG::CFG(DefFonction* ast, SymbolTableVisitor& stv) : 
    ast(ast), stv(stv), nextBBnumber(0), current_bb(nullptr) {}

void CFG::add_bb(BasicBlock* bb){
    bbs.push_back(bb);
    current_bb = bb;
}

std::string CFG::IR_reg_to_asm(std::string name) {
    if (stv.symbolTable.count(name)) {
        int offset = stv.symbolTable[name].offset;
        return std::to_string(offset) + "(%rbp)";
    } 

    stv.writeError("Variable " + name + " non trouvée");
    return "0(%rbp)"; 
}

void CFG::gen_asm(std::ostream& o) {
    gen_asm_prologue(o);
    for (auto bb : bbs) {
        bb->gen_asm(o);
    }
    gen_asm_epilogue(o);
}

void CFG:: gen_asm_prologue(std::ostream& o) {
    codegenBackend->gen_prologue(o, ast->name);
}

void CFG::gen_asm_epilogue(std::ostream& o) {
    codegenBackend->gen_epilogue(o);
}

std::string CFG::create_new_tempvar(){
    return stv.createNewTemp();
}

std::string CFG::new_BB_name() {
    return "BB" + std::to_string(nextBBnumber++);
}
