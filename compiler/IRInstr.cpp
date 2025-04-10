#include "IRInstr.h"
#include "IR.h"

extern CodeGenBackend *codegenBackend;

std::vector<std::string> IRInstr::getParams()
{
    return params;
}

void IRReturn::gen_asm(std::ostream &o)
{
    codegenBackend->gen_return(o, bb->cfg->IR_reg_to_asm(params[0]));
    o << "    jmp " << bb->cfg->epilogueLabel << "\n"; // Jump to epilogue
}

void IRLdConst::gen_asm(std::ostream &o)
{
    codegenBackend->gen_mov(o, bb->cfg->IR_reg_to_asm(params[0]), params[1]);
}

void IRCopy::gen_asm(std::ostream &o)
{
    codegenBackend->gen_copy(o,
                             bb->cfg->IR_reg_to_asm(params[0]),
                             bb->cfg->IR_reg_to_asm(params[1]));
}

void IRAdd::gen_asm(std::ostream &o)
{
    codegenBackend->gen_add(o,
                            bb->cfg->IR_reg_to_asm(params[0]),
                            bb->cfg->IR_reg_to_asm(params[1]),
                            bb->cfg->IR_reg_to_asm(params[2]));
}

void IRSub::gen_asm(std::ostream &o)
{
    codegenBackend->gen_sub(o,
                            bb->cfg->IR_reg_to_asm(params[0]),
                            bb->cfg->IR_reg_to_asm(params[1]),
                            bb->cfg->IR_reg_to_asm(params[2]));
}

void IRMul::gen_asm(std::ostream &o)
{
    codegenBackend->gen_mul(o,
                            bb->cfg->IR_reg_to_asm(params[0]),
                            bb->cfg->IR_reg_to_asm(params[1]),
                            bb->cfg->IR_reg_to_asm(params[2]));
}

void IRDiv::gen_asm(std::ostream &o)
{
    codegenBackend->gen_div(o,
                            bb->cfg->IR_reg_to_asm(params[0]),
                            bb->cfg->IR_reg_to_asm(params[1]),
                            bb->cfg->IR_reg_to_asm(params[2]));
}

void IRMod::gen_asm(std::ostream &o)
{
    codegenBackend->gen_mod(o,
                            bb->cfg->IR_reg_to_asm(params[0]),
                            bb->cfg->IR_reg_to_asm(params[1]),
                            bb->cfg->IR_reg_to_asm(params[2]));
}

void IRMovReg::gen_asm(std::ostream &o)
{
    // Pour IRMovReg, on déplace l'opérande src (après conversion) vers le registre dest
    // Comme dest est déjà un registre (ex : "%edi"), on l'utilise tel quel.
    o << "    movl " << bb->cfg->IR_reg_to_asm(params[1]) << ", " << params[0] << "\n";
}

void IRCall::gen_asm(std::ostream &o)
{
    const bool isARM64 = codegenBackend->getArchitecture() == "arm64";

    // Registres d'arguments selon l'ABI
    std::vector<std::string> argRegs;
    if (isARM64) {
        argRegs = {"w0", "w1", "w2", "w3", "w4", "w5", "w6", "w7"};
    } else { // X86-64
        argRegs = {"%edi", "%esi", "%edx", "%ecx", "%r8d", "%r9d"};
    }

    for (size_t i = 0; i < params.size(); ++i)
    {
        if (i >= argRegs.size()) {
            std::cerr << "[ERROR] Function call '" << funcName << "' with too many arguments (limit: "
                      << argRegs.size() << ").\n";
            exit(1);
        }

        const std::string &src = bb->cfg->IR_reg_to_asm(params[i]);
        const std::string &dest = argRegs[i];
        codegenBackend->gen_copy(o, dest, src);
    }

    // Appel de la fonction
    codegenBackend->gen_call(o, funcName);

    // Récupération de retour
    if (!retVar.empty())
    {
        const std::string &retReg = isARM64 ? "w0" : "%eax";
        codegenBackend->gen_copy(o, bb->cfg->IR_reg_to_asm(retVar), retReg);
    }
}


void IRNot::gen_asm(std::ostream &o)
{
    codegenBackend->gen_not(o,
                            bb->cfg->IR_reg_to_asm(params[0]),
                            bb->cfg->IR_reg_to_asm(params[1]));
}

void IRXor::gen_asm(std::ostream &o)
{
    codegenBackend->gen_xor(o,
                            bb->cfg->IR_reg_to_asm(params[0]),
                            bb->cfg->IR_reg_to_asm(params[1]),
                            bb->cfg->IR_reg_to_asm(params[2]));
}

void IROr::gen_asm(std::ostream &o)
{
    codegenBackend->gen_or(o,
                           bb->cfg->IR_reg_to_asm(params[0]),
                           bb->cfg->IR_reg_to_asm(params[1]),
                           bb->cfg->IR_reg_to_asm(params[2]));
}

void IREgal::gen_asm(std::ostream &o)
{
    codegenBackend->gen_egal(o,
                             bb->cfg->IR_reg_to_asm(params[0]),
                             bb->cfg->IR_reg_to_asm(params[1]),
                             bb->cfg->IR_reg_to_asm(params[2]));
}

void IRNotEgal::gen_asm(std::ostream &o)
{
    codegenBackend->gen_notegal(o,
                                bb->cfg->IR_reg_to_asm(params[0]),
                                bb->cfg->IR_reg_to_asm(params[1]),
                                bb->cfg->IR_reg_to_asm(params[2]));
}

void IRAnd::gen_asm(std::ostream &o)
{
    codegenBackend->gen_and(o,
                            bb->cfg->IR_reg_to_asm(params[0]),
                            bb->cfg->IR_reg_to_asm(params[1]),
                            bb->cfg->IR_reg_to_asm(params[2]));
}

void IRComp::gen_asm(std::ostream &o) {
    codegenBackend->gen_comp(o,
        bb->cfg->IR_reg_to_asm(params[0]),
        bb->cfg->IR_reg_to_asm(params[1]),
        bb->cfg->IR_reg_to_asm(params[2]),
        op);
}

void IRPutChar::gen_asm(std::ostream &o)
{
    std::string architecture = codegenBackend->getArchitecture();
    std::string reg = "";

    if (architecture == "arm64") {
        reg = "w0";
    } else if (architecture == "X86") {
        reg = "%edi"; // 1er argument
    }

    if (reg.empty()) {
        std::cerr << "[ERROR] Architecture inconnue\n";
        exit(-1);
    }

    codegenBackend->gen_copy(o, reg, bb->cfg->IR_reg_to_asm(params[0]));
    codegenBackend->gen_call(o, "putchar");
}

void IRGetChar::gen_asm(std::ostream &o)
{
    codegenBackend->gen_call(o, "getchar");

    std::string architecture = codegenBackend->getArchitecture();
    std::string reg = "";

    if (architecture == "arm64") {
        reg = "w0";
    } else if (architecture == "X86") {
        reg = "%eax"; // ✅ valeur de retour
    }

    if (reg.empty()) {
        std::cerr << "[ERROR] Architecture inconnue\n";
        exit(-1);
    }

    codegenBackend->gen_copy(o, bb->cfg->IR_reg_to_asm(params[0]), reg);
}

void IRBranch::gen_asm(std::ostream &o)
{
    if (params[0].empty()) // Unconditional jump
    {
        o << "    jmp " << params[1] << "\n";
    }
    else
    {
        codegenBackend->gen_branch(o,
                                   bb->cfg->IR_reg_to_asm(params[0]),
                                   params[1],
                                   params[2]);
    }
}
