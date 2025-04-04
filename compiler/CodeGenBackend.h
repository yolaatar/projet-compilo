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

    virtual void gen_prologue(std::ostream &os, std::string &name, int stackSize) const = 0;
    virtual void gen_epilogue(std::ostream &os) const = 0;
    virtual void gen_return(std::ostream &os, const std::string &src) const = 0;
    virtual void gen_mov(std::ostream &os, const std::string &dest, const std::string &src) const = 0;
    virtual void gen_copy(std::ostream &os, const std::string &dest, const std::string &src) const = 0;
    virtual void gen_add(std::ostream &os, const std::string &dest, const std::string &src1, const std::string &src2) const = 0;
    virtual void gen_sub(std::ostream &os, const std::string &dest, const std::string &src1, const std::string &src2) const = 0;
    virtual void gen_mul(std::ostream &os, const std::string &dest, const std::string &src1, const std::string &src2) const = 0;
    virtual void gen_div(std::ostream &os, const std::string &dest, const std::string &src1, const std::string &src2) const = 0;
    virtual void gen_mod(std::ostream &os, const std::string &dest, const std::string &src1, const std::string &src2) const = 0;
    virtual void gen_call(std::ostream &os, const std::string &func) const = 0;
    virtual void gen_or(std::ostream &os, const std::string &dest, const std::string &src1, const std::string &src2) const = 0;
    virtual void gen_xor(std::ostream &os, const std::string &dest, const std::string &src1, const std::string &src2) const = 0;
    virtual void gen_not(std::ostream &os, const std::string &dest, const std::string &src) const = 0;
    virtual void gen_egal(std::ostream &os, const std::string &dest, const std::string &src1, const std::string &src2) const = 0;
    virtual void gen_notegal(std::ostream &os, const std::string &dest, const std::string &src1, const std::string &src2) const = 0;
    virtual void gen_and(std::ostream &os, const std::string &dest, const std::string &src1, const std::string &src2) const = 0;
    virtual void gen_ge(std::ostream &os, const std::string &dest, const std::string &src1, const std::string &src2) const = 0;
    virtual void gen_gt(std::ostream &os, const std::string &dest, const std::string &src1, const std::string &src2) const = 0;

    virtual std::string getTempPrefix() const = 0;
    virtual std::string getArchitecture() const = 0;    
};

#endif
