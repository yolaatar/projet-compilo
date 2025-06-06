#include "X86Backend.h"
#include <iostream>
#include <cctype>

void X86Backend::gen_return(std::ostream &os, const std::string &src) const {
    os << "    movl " << src << ", %eax\n";
}

void X86Backend::gen_mov(std::ostream &os, const std::string &dest, const std::string &src) const {
    if (isdigit(src[0]) || (src[0] == '-' && isdigit(src[1]))) {
        os << "    movl $" << src << ", " << dest << "\n";
    } else {
        os << "    movl " << src << ", %eax\n";
        os << "    movl %eax, " << dest << "\n";
    }
}

void X86Backend::gen_add(std::ostream &os, const std::string &dest,
                         const std::string &src1, const std::string &src2) const {
    os << "    movl " << src1 << ", %eax\n";
    os << "    addl " << src2 << ", %eax\n";
    os << "    movl %eax, " << dest << "\n";
}

void X86Backend::gen_sub(std::ostream &os, const std::string &dest,
                         const std::string &src1, const std::string &src2) const {
    os << "    movl " << src1 << ", %eax\n";
    os << "    subl " << src2 << ", %eax\n";
    os << "    movl %eax, " << dest << "\n";
}

void X86Backend::gen_mul(std::ostream &os, const std::string &dest,
                         const std::string &src1, const std::string &src2) const {
    os << "    movl " << src1 << ", %eax\n";
    os << "    imull " << src2 << ", %eax\n";
    os << "    movl %eax, " << dest << "\n";
}

void X86Backend::gen_div(std::ostream &os, const std::string &dest,
                         const std::string &src1, const std::string &src2) const {
    os << "    movl " << src1 << ", %eax\n";
    os << "    cltd\n";
    os << "    idivl " << src2 << "\n";
    os << "    movl %eax, " << dest << "\n";
}

void X86Backend::gen_mod(std::ostream &os, const std::string &dest,
                         const std::string &src1, const std::string &src2) const {
    os << "    movl " << src1 << ", %eax\n";
    os << "    cltd\n";
    os << "    idivl " << src2 << "\n";
    os << "    movl %edx, " << dest << "\n";
}

void X86Backend::gen_not(std::ostream &os, const std::string &dest,
                         const std::string &src) const {
    os << "    cmpl $0, " << src << "\n";
    os << "    sete %al\n";
    os << "    movzbl %al, %eax\n";
    os << "    movl %eax, " << dest << "\n";
}

void X86Backend::gen_egal(std::ostream &os, const std::string &dest,
                          const std::string &src1, const std::string &src2) const {
    os << "    movl " << src1 << ", %eax\n";
    os << "    cmpl " << src2 << ", %eax\n";
    os << "    sete %al\n";
    os << "    movzbl %al, %eax\n";
    os << "    movl %eax, " << dest << "\n";
}

void X86Backend::gen_notegal(std::ostream &os, const std::string &dest,
                             const std::string &src1, const std::string &src2) const {
    os << "    movl " << src1 << ", %eax\n";
    os << "    cmpl " << src2 << ", %eax\n";
    os << "    setne %al\n";
    os << "    movzbl %al, %eax\n";
    os << "    movl %eax, " << dest << "\n";
}

void X86Backend::gen_xor(std::ostream &os, const std::string &dest,
                         const std::string &src1, const std::string &src2) const {
    std::cerr << "[gen_xor] dest = " << dest << ", src1 = " << src1 << ", src2 = " << src2 << "\n";
    os << "    movl " << src1 << ", %eax\n";
    os << "    xorl " << src2 << ", %eax\n";
    os << "    movl %eax, " << dest << "\n";
}

void X86Backend::gen_or(std::ostream &os, const std::string &dest,
                        const std::string &src1, const std::string &src2) const {
    std::cerr << "[gen_or] dest = " << dest << ", src1 = " << src1 << ", src2 = " << src2 << "\n";
    os << "    movl " << src1 << ", %eax\n";
    os << "    orl " << src2 << ", %eax\n";
    os << "    movl %eax, " << dest << "\n";
}

void X86Backend::gen_call(std::ostream &os, const std::string &func) const {
    os << "    call " << func << "\n";
}

void X86Backend::gen_prologue(std::ostream &os, std::string &name, int stackSize) const {
    os << ".globl " << name << "\n";
    os << name << ":\n";
    os << "    pushq %rbp\n";
    os << "    movq %rsp, %rbp\n";
    if (stackSize > 0) {
        os << "    subq $" << stackSize << ", %rsp\n";
    }
}

void X86Backend::gen_epilogue(std::ostream &os) const {
    os << "    movq %rbp, %rsp\n";  // Restore %rsp
    os << "    popq %rbp\n";
    os << "    ret\n";
}

void X86Backend::gen_copy(std::ostream &os, const std::string &dest, const std::string &src) const {
    bool srcIsReg = src[0] == '%';
    bool destIsReg = dest[0] == '%';

    if (srcIsReg && destIsReg)
    {
        os << "    movl " << src << ", " << dest << "\n";
    }
    else if (srcIsReg && !destIsReg)
    {
        os << "    movl " << src << ", " << dest << "\n";
    }
    else if (!srcIsReg && destIsReg)
    {
        os << "    movl " << src << ", " << dest << "\n";
    }
    else // both are memory
    {
        os << "    movl " << src << ", %eax\n";
        os << "    movl %eax, " << dest << "\n";
    }
}

void X86Backend::gen_and(std::ostream &os,
    const std::string &dest,
    const std::string &src1,
    const std::string &src2) const {
    os << "    movl " << src1 << ", %eax\n";
    os << "    andl " << src2 << ", %eax\n";
    os << "    movl %eax, " << dest << "\n";
}


std::string X86Backend::getTempPrefix() const {
    return "!tmp";
}

std::string X86Backend::getArchitecture() const {
    return "X86";
}

void X86Backend::gen_comp(std::ostream &os, const std::string &dest,
    const std::string &src1, const std::string &src2,
    const std::string &op) const {
    // Charger src1 dans %eax pour éviter de comparer deux adresses mémoire.
    os << "    movl " << src1 << ", %eax\n";
    // Comparer le contenu de %eax (src1) avec src2.
    os << "    cmpl " << src2 << ", %eax\n";
    // Choisir l'instruction set selon l'opérateur.
    if (op == ">")
        os << "    setg %al\n";
    else if (op == "<")
        os << "    setl %al\n";
    else if (op == ">=")
        os << "    setge %al\n";
    else if (op == "<=")
        os << "    setle %al\n";
    else
        os << "    ; opérateur de comparaison non supporté: " << op << "\n";
        os << "    movzbl %al, %eax\n";
        os << "    movl %eax, " << dest << "\n";
}

void X86Backend::gen_branch(std::ostream &os, const std::string &cond, const std::string &label_then, const std::string &label_else) const {
    std::cerr << "[X86Backend] gen_branch not implemented\n";
}
