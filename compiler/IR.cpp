#include "IR.h"
#include "IRInstr.h"
#include "SymbolTableVisitor.h"
#include <cctype>
#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>
#include <memory>

#include "IR.h"

// ----- Définitions des constructeurs et autres fonctions non-inline -------

// Constructeur de DefFonction
DefFonction::DefFonction(const std::string &name, const std::vector<std::string> &params)
    : name(name), params(params)
{
    // Vous pouvez ajouter du code d'initialisation ici si nécessaire.
}

// Constructeur de BasicBlock
BasicBlock::BasicBlock(CFG* cfg, std::string entry_label)
    : cfg(cfg),
      label(entry_label + "_" + cfg->ast->name),
      exit_true(nullptr),
      exit_false(nullptr)
{
    // Vous pouvez ajouter d'autres initialisations si nécessaire.
}

/**
 * DefFonction
 */
void DefFonction::print(std::ostream &os) const
{
    os << "Function: " << name << "(";
    for (size_t i = 0; i < params.size(); i++)
    {
        os << params[i];
        if (i < params.size() - 1)
            os << ", ";
    }
    os << ")";
}

// ============================================================================
// BasicBlock
// ============================================================================
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

// ============================================================================
// CFG
// ============================================================================
CFG::CFG(DefFonction *ast, SymbolTableVisitor &stv)
    : ast(ast), stv(stv), nextBBnumber(0), current_bb(nullptr) {}

void CFG::add_bb(BasicBlock *bb) {
    bbs.push_back(bb);
    current_bb = bb;
}

static bool isNumber(const std::string &s) {
    if (s.empty())
        return false;
    size_t start = (s[0] == '-') ? 1 : 0;
    for (size_t i = start; i < s.size(); i++) {
        if (!std::isdigit(s[i]))
            return false;
    }
    return true;
}

std::string CFG::IR_reg_to_asm(std::string name) {
    if (!codegenBackend) {
        stv.writeError("Codegen backend not initialized!");
        return "0";
    }
    // Si c'est déjà un registre, le retourner directement.
    if ((name[0] == 'w' || name[0] == 'x') && name.size() == 2 && std::isdigit(name[1]))
        return name;
    
    // Recherche par la clé (nom original) dans la chaîne des scopes.
    Scope* scope = stv.currentScope;
    while (scope != nullptr) {
        if (scope->symbols.find(name) != scope->symbols.end()) {
            int off = scope->symbols[name].offset;
            if (codegenBackend->getArchitecture() == "arm64")
                return "[x29, #" + std::to_string(off) + "]";
            else
                return std::to_string(off) + "(%rbp)";
        }
        scope = scope->parent;
    }
    stv.writeError("Variable " + name + " non trouvée");
    return (codegenBackend->getArchitecture() == "arm64") ? "[x29, #0]" : "0(%rbp)";
}


void CFG::gen_asm(std::ostream &o) {
    if (usesGetChar)
        o << ".extern getchar\n";
    if (usesPutChar)
        o << ".extern putchar\n";

    gen_asm_prologue(o);
    for (auto bb : bbs) {
        bb->gen_asm(o);
    }
    gen_asm_epilogue(o);
}

void CFG::gen_asm_prologue(std::ostream &o) {
    // Utiliser le scope global pour calculer la taille de la pile.
    Scope* global = stv.getGlobalScope();
    int stackSize = (-global->offset + 15) / 16 * 16;  // Arrondi au multiple de 16

    if (codegenBackend->getArchitecture() == "arm64") {
        std::string cleanName = ast->name;
        size_t sharp = cleanName.find('#');
        if (sharp != std::string::npos)
            cleanName = cleanName.substr(0, sharp);
        codegenBackend->gen_prologue(o, cleanName, stackSize);
    }
    else {
        codegenBackend->gen_prologue(o, ast->name, stackSize);
    }
}

void CFG::gen_asm_epilogue(std::ostream &o) {
    codegenBackend->gen_epilogue(o);
}

SymbolTableVisitor &CFG::get_stv() {
    return stv;
}

std::string CFG::create_new_tempvar() {
    return stv.createNewTemp();
}

std::string CFG::new_BB_name() {
    return ".LBB" + std::to_string(nextBBnumber++);
}
