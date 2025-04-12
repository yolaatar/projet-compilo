#include "IR.h"
#include "IRInstr.h"

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

/**
 * BasicBlock
 */
BasicBlock::BasicBlock(CFG* cfg, std::string entry_label)
            : cfg(cfg), label(entry_label + "_" + cfg->ast->name), exit_true(nullptr), exit_false(nullptr) {}

void BasicBlock::gen_asm(std::ostream &o)
{
    o << label << ":\n";
    for (auto &instr : instrs)
    {
        instr->gen_asm(o);
    } // ajouter les sauts
    if (exit_true != nullptr && exit_false != nullptr){
        o << "    movl " << cfg->IR_reg_to_asm(test_var_name) << ", %eax\n";
        o << "    cmpl $0, %eax\n";
        o << "    jne " << exit_true->label << "\n";
        o << "    jmp " << exit_false->label << "\n";
    } else if  (exit_true != nullptr && exit_false == nullptr) {
        o << "    jmp " << exit_true->label << "\n";
    } 
}


void BasicBlock::add_IRInstr(std::unique_ptr<IRInstr> instr)
{
    instrs.push_back(std::move(instr));
}

void BasicBlock::print_instrs() const
{
    std::cerr << "=== Instructions in BasicBlock [" << label << "] ===\n";
    for (const auto &instr : instrs)
    {
        std::cerr << "  - " << typeid(*instr).name() << "(";
        std::vector<std::string> params = instr->getParams();
        for (size_t i = 0; i < params.size(); ++i)
        {
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
 CFG::CFG(DefFonction *ast, SymbolTableVisitor &stv)
 : ast(ast), stv(stv), nextBBnumber(0), current_bb(nullptr) {
 epilogueLabel = ".Lend_" + ast->name; // Unique epilogue label
}

void CFG::add_bb(BasicBlock *bb)
{
    bbs.push_back(bb);
    current_bb = bb;
}
/////////
static bool isNumber(const std::string &s)
{
    if (s.empty())
        return false;
    size_t start = (s[0] == '-') ? 1 : 0;
    for (size_t i = start; i < s.size(); i++)
    {
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
    // Si le nom commence par '$', considérer qu'il s'agit d'un immédiat, et le retourner tel quel
    if (!name.empty() && name[0] == '$')
        return name; // Par exemple, "$5" sera retourné tel quel

    // Sinon, rechercher le symbole dans la hiérarchie des scopes
    Scope* scope = stv.currentScope;
    while (scope != nullptr) {
        for (const auto &entry : scope->symbols) {
            if (entry.second.uniqueName == name) {
                int off = entry.second.offset;
                return std::to_string(off) + "(%rbp)";
            }
        }
        scope = scope->parent;
    }
    // Si non trouvé, vous pouvez éventuellement consulter les symboles agrégés
    stv.writeError("Variable " + name + " non trouvée");
    return "0(%rbp)";
}


void CFG::gen_asm(std::ostream &o)
{
    if (usesGetChar)
        o << ".extern getchar\n";
    if (usesPutChar)
        o << ".extern putchar\n";

    gen_asm_prologue(o);
    for (auto bb : bbs)
    {
        bb->gen_asm(o);
    }
    o << epilogueLabel << ":\n";          // Write the unique label
    codegenBackend->gen_epilogue(o);      // Generate epilogue instructions
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

void CFG::gen_asm_epilogue(std::ostream &o)
{
    codegenBackend->gen_epilogue(o);
}

SymbolTableVisitor &CFG::get_stv()
{
    return stv;
}

std::string CFG::create_new_tempvar()
{
    return stv.createNewTemp();
}

std::string CFG::new_BB_name() {
    return ".LBB" + std::to_string(nextBBnumber++);
}

