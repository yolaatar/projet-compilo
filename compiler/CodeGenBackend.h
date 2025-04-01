#ifndef CODEGENBACKEND_H
#define CODEGENBACKEND_H

#include <ostream>
#include <string>

/**
 * Interface abstraite pour la génération d'assembleur spécifique à une architecture.
 * Cette interface déclare les méthodes nécessaires pour générer des instructions.
 */
class CodeGenBackend {
public:
    virtual ~CodeGenBackend() {}

    virtual void gen_mov(std::ostream &os, const std::string &dest, const std::string &src) const = 0;
    virtual void gen_add(std::ostream &os, const std::string &dest, const std::string &src1, const std::string &src2) const = 0;
    virtual void gen_sub(std::ostream &os, const std::string &dest, const std::string &src1, const std::string &src2) const = 0;
    virtual void gen_mul(std::ostream &os, const std::string &dest, const std::string &src1, const std::string &src2) const = 0;
    virtual void gen_div(std::ostream &os, const std::string &dest, const std::string &dividend, const std::string &divisor) const = 0;
    virtual void gen_mod(std::ostream &os, const std::string &dest, const std::string &dividend, const std::string &divisor) const = 0;
    virtual void gen_neg(std::ostream &os, const std::string &reg) const = 0;
    virtual void gen_cmp(std::ostream &os, const std::string &op1, const std::string &op2) const = 0;
    virtual void gen_sete(std::ostream &os, const std::string &reg8) const = 0;
    virtual void gen_setne(std::ostream &os, const std::string &reg8) const = 0;
    virtual void gen_setl(std::ostream &os, const std::string &reg8) const  = 0;
    virtual void gen_setle(std::ostream &os, const std::string &reg8) const = 0;
    virtual void gen_setg(std::ostream &os, const std::string &reg8) const = 0;
    virtual void gen_setge(std::ostream &os, const std::string &reg8) const  = 0;
    virtual void gen_movzbl(std::ostream &os, const std::string &src, const std::string &dest) const = 0;
    virtual void gen_andl(std::ostream &os, const std::string &dest, const std::string &src) const = 0;
    virtual void gen_xorl(std::ostream &os, const std::string &dest, const std::string &src) const = 0;
    virtual void gen_orl(std::ostream &os, const std::string &dest, const std::string &src) const = 0;
    virtual void gen_call(std::ostream &os, const std::string &function) const = 0;
    virtual void gen_jmp(std::ostream &os, const std::string &label) const = 0;
    virtual void gen_label(std::ostream &os, const std::string &label) const = 0;
    
};

#endif
