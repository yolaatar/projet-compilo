// X86Backend.h
#ifndef X86BACKEND_H
#define X86BACKEND_H

#include <ostream>
#include <string>
#include <vector>
#include "CodeGenBackend.h"

// Si vous avez une classe abstraite CodeGenBackend, vous pouvez l'hériter ici.
// Sinon, X86Backend peut être utilisé directement.
class X86Backend  : public CodeGenBackend  {
public:
    int labelCountFalse = 0;
    int labelCountTrue =0;

    virtual ~X86Backend(){}
    virtual void gen_return(std::ostream &os, const std::string &src) const override;
    virtual void gen_mov(std::ostream &os, const std::string &dest, const std::string &src) const override;
    virtual void gen_copy(std::ostream &os, const std::string &dest, const std::string &src) const override;
    virtual void gen_add(std::ostream &os, const std::string &dest, const std::string &src1, const std::string &src2) const override;
    virtual void gen_sub(std::ostream &os, const std::string &dest, const std::string &src1, const std::string &src2) const override;
    virtual void gen_mul(std::ostream &os, const std::string &dest, const std::string &src1, const std::string &src2) const override;
    virtual void gen_div(std::ostream &os, const std::string &dest, const std::string &src1, const std::string &src2) const override;
    virtual void gen_mod(std::ostream &os, const std::string &dest, const std::string &src1, const std::string &src2) const override;
    virtual void gen_call(std::ostream &os, const std::string &func) const override;
    virtual void gen_or(std::ostream &os, const std::string &dest, const std::string &src1, const std::string &src2) const override;
    virtual void gen_xor(std::ostream &os, const std::string &dest, const std::string &src1, const std::string &src2) const override;
    virtual void gen_not(std::ostream &os, const std::string &dest, const std::string &src) const override;
    virtual void gen_egal(std::ostream &os, const std::string &dest, const std::string &src1, const std::string &src2) const override;
    virtual void gen_notegal(std::ostream &os, const std::string &dest, const std::string &src1, const std::string &src2) const override;
    virtual void gen_prologue(std::ostream &os, std::string &name, int stackSize) const override;
    virtual void gen_epilogue(std::ostream &os) const override;
    virtual void gen_and(std::ostream &os, const std::string &dest, const std::string &src1, const std::string &src2) const override;
    virtual void gen_branch(std::ostream &os, const std::string &cond, const std::string &label_then, const std::string &label_else) const;
    virtual void gen_comp(std::ostream &os, const std::string &dest, const std::string &src1, const std::string &src2, const std::string &op) const override;

    virtual std::string getTempPrefix() const override;
    virtual std::string getArchitecture() const override;
};

#endif // X86BACKEND_H