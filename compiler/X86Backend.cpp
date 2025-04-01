// X86Backend.cpp
#include "X86Backend.h"
#include <iostream>

// Génération du prologue de la fonction main
void X86Backend::gen_prologue(std::ostream &os) const {
    os << ".globl main\n";
    os << "main:\n";
    os << "    pushq %rbp\n";
    os << "    movq %rsp, %rbp\n";
}

// Génération de l'épilogue de la fonction main
void X86Backend::gen_epilogue(std::ostream &os) const {
    os << "end:\n";
    os << "    popq %rbp\n";
    os << "    ret\n";
}

// Génération d'un déplacement (move)
// Note : en x86, l'instruction s'écrit "movl <src>, <dest>"
void X86Backend::gen_mov(std::ostream &os, const std::string &dest, const std::string &src) const {
    os << "    movl " << src << ", " << dest << "\n";
}

// Génération d'une addition : effectue dest = src1 + src2
// Pour x86, on déplace d'abord src1 dans dest, puis on ajoute src2
void X86Backend::gen_add(std::ostream &os, const std::string &dest, const std::string &src1, const std::string &src2) const {
    os << "    movl " << src1 << ", " << dest << "\n";
    os << "    addl " << src2 << ", " << dest << "\n";
}

// Génération d'une soustraction : effectue dest = src1 - src2
void X86Backend::gen_sub(std::ostream &os, const std::string &dest, const std::string &src1, const std::string &src2) const {
    os << "    movl " << src1 << ", " << dest << "\n";
    os << "    subl " << src2 << ", " << dest << "\n";
}

// Génération d'une multiplication (imull) : effectue dest = src1 * src2
void X86Backend::gen_mul(std::ostream &os, const std::string &dest, const std::string &src1, const std::string &src2) const {
    os << "    movl " << src1 << ", " << dest << "\n";
    os << "    imull " << src2 << ", " << dest << "\n";
}

// Génération d'une division entière
// Avant la division, on déplace le dividende dans %eax et on étend le signe avec cltd.
// Le diviseur est fourni en argument, et le quotient est placé dans %eax.
void X86Backend::gen_div(std::ostream &os, const std::string &dest, const std::string &dividend, const std::string &divisor) const {
    os << "    movl " << dividend << ", %eax\n";
    os << "    cltd\n"; // étend %eax en %edx:%eax
    os << "    idivl " << divisor << "\n";
    os << "    movl %eax, " << dest << "\n";
}

// Génération du modulo : résultat dans %edx après idivl, on le transfère vers dest.
void X86Backend::gen_mod(std::ostream &os, const std::string &dest, const std::string &dividend, const std::string &divisor) const {
    os << "    movl " << dividend << ", %eax\n";
    os << "    cltd\n";
    os << "    idivl " << divisor << "\n";
    os << "    movl %edx, " << dest << "\n";
}

// Génération d'une négation sur un registre (par exemple, negl %eax)
void X86Backend::gen_neg(std::ostream &os, const std::string &reg) const {
    os << "    negl " << reg << "\n";
}

// Génération d'une comparaison entre deux opérandes
// L'instruction compare op2 avec op1 : "cmpl op2, op1"
void X86Backend::gen_cmp(std::ostream &os, const std::string &op1, const std::string &op2) const {
    os << "    cmpl " << op2 << ", " << op1 << "\n";
}

// Génération des instructions de condition pour les comparaisons
void X86Backend::gen_sete(std::ostream &os, const std::string &reg8) const {
    os << "    sete " << reg8 << "\n";
}

void X86Backend::gen_setne(std::ostream &os, const std::string &reg8) const {
    os << "    setne " << reg8 << "\n";
}

void X86Backend::gen_setl(std::ostream &os, const std::string &reg8) const {
    os << "    setl " << reg8 << "\n";
}

void X86Backend::gen_setle(std::ostream &os, const std::string &reg8) const {
    os << "    setle " << reg8 << "\n";
}

void X86Backend::gen_setg(std::ostream &os, const std::string &reg8) const {
    os << "    setg " << reg8 << "\n";
}

void X86Backend::gen_setge(std::ostream &os, const std::string &reg8) const {
    os << "    setge " << reg8 << "\n";
}

// Étend un registre 8 bits en un registre 32 bits (movzbl)
void X86Backend::gen_movzbl(std::ostream &os, const std::string &src, const std::string &dest) const {
    os << "    movzbl " << src << ", " << dest << "\n";
}

// Génération d'un AND logique
void X86Backend::gen_andl(std::ostream &os, const std::string &dest, const std::string &src) const {
    os << "    andl " << src << ", " << dest << "\n";
}

// Génération d'un XOR logique
void X86Backend::gen_xorl(std::ostream &os, const std::string &dest, const std::string &src) const {
    os << "    xorl " << src << ", " << dest << "\n";
}

// Génération d'un OR logique
void X86Backend::gen_orl(std::ostream &os, const std::string &dest, const std::string &src) const {
    os << "    orl " << src << ", " << dest << "\n";
}

// Génération d'un appel de fonction
void X86Backend::gen_call(std::ostream &os, const std::string &function) const {
    os << "    call " << function << "\n";
}

// Génération d'un saut inconditionnel vers une étiquette
void X86Backend::gen_jmp(std::ostream &os, const std::string &label) const {
    os << "    jmp " << label << "\n";
}

// Génération d'une étiquette
void X86Backend::gen_label(std::ostream &os, const std::string &label) const {
    os << label << ":\n";
}
