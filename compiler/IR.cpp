#include "IR.h"
#include "IRInstr.h"


extern CodeGenBackend *codegenBackend;

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

    // Génère les instructions du bloc
    for (auto &instr : instrs)
    {
        instr->gen_asm(o);
    }

    // Génère le saut conditionnel (cas if/while)
    if (exit_true != nullptr && exit_false != nullptr)
    {
        std::string testReg = cfg->IR_reg_to_asm(test_var_name);

        if (codegenBackend->getArchitecture() == "X86")
        {
            o << "    movl " << testReg << ", %eax\n";
            o << "    cmpl $0, %eax\n";
            o << "    jne " << exit_true->label << "\n";
            o << "    jmp " << exit_false->label << "\n";
        }
        else if (codegenBackend->getArchitecture() == "arm64")
        {
            o << "    ldr w0, " << testReg << "\n";
            o << "    cmp w0, #0\n";
            o << "    b.ne " << exit_true->label << "\n";
            o << "    b " << exit_false->label << "\n";
        }
    }
    // Saut inconditionnel (cas while → cond ou else/then → merge)
    else if (exit_true != nullptr && exit_false == nullptr)
    {
        if (codegenBackend->getArchitecture() == "X86")
        {
            o << "    jmp " << exit_true->label << "\n";
        }
        else if (codegenBackend->getArchitecture() == "arm64")
        {
            o << "    b " << exit_true->label << "\n";
        }
    }

    // (Pas besoin de gérer le cas où les deux sont null, fin de fonction)
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
CFG::CFG(DefFonction *ast, SymbolTableVisitor &stv) : ast(ast), stv(stv), nextBBnumber(0), current_bb(nullptr) {}

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

std::string CFG::IR_reg_to_asm(std::string name)
{
    if (!codegenBackend)
    {
        stv.writeError("Codegen backend not initialized!");
        return "0";
    }

    // Bypass if it's a known ARM64 register
    if ((name[0] == 'w' || name[0] == 'x') && name.size() == 2 && isdigit(name[1])) {
        return name;  // C’est déjà un registre
    }

 for (auto it = stv.symbolStack.rbegin(); it != stv.symbolStack.rend(); ++it) {
        if (it->find(name) != it->end()) {
            int offset = (*it)[name].offset;
            if (codegenBackend->getArchitecture() == "arm64")
                return "[x29, #" + std::to_string(offset) + "]";
            else
                return std::to_string(offset) + "(%rbp)";
        }
    }

    stv.writeError("Variable " + name + " non trouvée");
    return (codegenBackend->getArchitecture() == "arm64") ? "[x29, #0]" : "0(%rbp)";
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
    gen_asm_epilogue(o);
}

void CFG::gen_asm_prologue(std::ostream &o)
{
    // Compute the total size to reserve on the stack (must be positive)
    int stackSize = (-stv.offset + 15) / 16 * 16; // Round up to nearest 16

    if (codegenBackend->getArchitecture() == "arm64")
    {
        std::string cleanName = ast->name;
        size_t sharp = cleanName.find('#');
        if (sharp != std::string::npos)
            cleanName = cleanName.substr(0, sharp);

        codegenBackend->gen_prologue(o, cleanName, stackSize);
    }
    else
    {
        codegenBackend->gen_prologue(o, ast->name, stackSize); // X86 doesn't need stackSize
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

