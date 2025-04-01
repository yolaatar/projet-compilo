// X86Backend.h
#ifndef X86BACKEND_H
#define X86BACKEND_H

#include <iostream>
#include <string>
#include "CodeGenBackend.h"

// Si vous avez une classe abstraite CodeGenBackend, vous pouvez l'hériter ici.
// Sinon, X86Backend peut être utilisé directement.
class X86Backend  : public CodeGenBackend  {
public:
    // Génération du prologue de la fonction main
    void gen_prologue(std::ostream &os) const;
    
    // Génération de l'épilogue de la fonction main
    void gen_epilogue(std::ostream &os) const;

    // Instructions de déplacement
    // Génère "movl <src>, <dest>"
    void gen_mov(std::ostream &os, const std::string &dest, const std::string &src) const;
    
    // Instructions arithmétiques
    // Addition : effectue dest = src1 + src2
    void gen_add(std::ostream &os, const std::string &dest, const std::string &src1, const std::string &src2) const;
    // Soustraction : effectue dest = src1 - src2
    void gen_sub(std::ostream &os, const std::string &dest, const std::string &src1, const std::string &src2) const;
    // Multiplication : effectue dest = src1 * src2
    void gen_mul(std::ostream &os, const std::string &dest, const std::string &src1, const std::string &src2) const;
    // Division entière : déplace le dividende dans %eax, étend le signe et effectue idivl divisor
    void gen_div(std::ostream &os, const std::string &dest, const std::string &dividend, const std::string &divisor) const;
    // Modulo : récupère le reste dans %edx après idivl
    void gen_mod(std::ostream &os, const std::string &dest, const std::string &dividend, const std::string &divisor) const;
    
    // Négation (inversion de signe)
    void gen_neg(std::ostream &os, const std::string &reg) const;

    // Comparaisons et instructions conditionnelles
    // Compare op2 avec op1 (instruction "cmpl op2, op1")
    void gen_cmp(std::ostream &os, const std::string &op1, const std::string &op2) const;
    // Instructions conditionnelles sur 8 bits (SETxx)
    void gen_sete(std::ostream &os, const std::string &reg8) const;
    void gen_setne(std::ostream &os, const std::string &reg8) const;
    void gen_setl(std::ostream &os, const std::string &reg8) const;
    void gen_setle(std::ostream &os, const std::string &reg8) const;
    void gen_setg(std::ostream &os, const std::string &reg8) const;
    void gen_setge(std::ostream &os, const std::string &reg8) const;
    // Étend le registre 8 bits en un registre 32 bits (instruction movzbl)
    void gen_movzbl(std::ostream &os, const std::string &src, const std::string &dest) const;

    // Instructions logiques
    void gen_andl(std::ostream &os, const std::string &dest, const std::string &src) const;
    void gen_xorl(std::ostream &os, const std::string &dest, const std::string &src) const;
    void gen_orl(std::ostream &os, const std::string &dest, const std::string &src) const;

    // Appels de fonctions et gestion des sauts
    void gen_call(std::ostream &os, const std::string &function) const;
    void gen_jmp(std::ostream &os, const std::string &label) const;
    void gen_label(std::ostream &os, const std::string &label) const;
};

#endif // X86BACKEND_H