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
    } else {
        argRegs = {"%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9"};
    }

    for (size_t i = 0; i < params.size(); ++i)
    {
        if (i >= argRegs.size()) {
            std::cerr << "[ERROR] Function call with too many arguments (limit: "
                      << argRegs.size() << ").\n";
            exit(1); // ou ajouter gestion pile plus tard
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


void IRJump::gen_asm(std::ostream &o)
{
    // On suppose que codegenBackend possède une méthode gen_jump qui prend le label cible
    codegenBackend->gen_jump(o, params[0]);
}

void IRBranch::gen_asm(std::ostream &o)
{
    // On suppose que params[0] contient la condition, params[1] le label "then" et params[2] le label "else"
    // Ici, les labels devraient déjà être locaux (commençant par un point)
    codegenBackend->gen_branch(o,
                               bb->cfg->IR_reg_to_asm(params[0]),
                               params[1], // doit être ".BBX"
                               params[2]  // doit être ".BBY"
    );
}

void IRJumpCond::gen_asm(std::ostream &o)
{
    codegenBackend->gen_jump_cond(o,
                                  bb->cfg->IR_reg_to_asm(params[0]), // La condition
                                  params[1],                         // Label du bloc "then"
                                  params[2]);                        // Label du bloc "else"
}

// void IROrPar::gen_asm(std::ostream &o)
// {
//     codegenBackend->gen_orPar(o,
//                               bb->cfg->IR_reg_to_asm(params[0]),
//                               bb->cfg->IR_reg_to_asm(params[1]),
//                               bb->cfg->IR_reg_to_asm(params[2]));
// }

// void IRAndPar::gen_asm(std::ostream &o)
// {
//     codegenBackend->gen_andPar(o,
//                                bb->cfg->IR_reg_to_asm(params[0]),
//                                bb->cfg->IR_reg_to_asm(params[1]),
//                                bb->cfg->IR_reg_to_asm(params[2]));
// }