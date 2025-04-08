#ifndef ARM64BACKEND_H
#define ARM64BACKEND_H

#include "CodeGenBackend.h"
#include <ostream>
#include <string>
#include <vector>

class ARM64Backend : public CodeGenBackend {
public:
    virtual ~ARM64Backend() {}

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
    virtual std::string getTempPrefix() const override;
    virtual std::string getArchitecture() const override;
    virtual std::string adjustMemOperand(const std::string &op) const ;
    virtual std::string loadOperand(const std::string &operand, const std::string &targetReg) const;
    virtual void gen_jump_cond(std::ostream &os, const std::string &cond,const std::string &labelTrue,const std::string &labelFalse) const;
    virtual void gen_branch(std::ostream &os, const std::string &cond, const std::string &label_then, const std::string &label_else) const;
    virtual void gen_jump(std::ostream &os, const std::string &target) const;

    

    virtual void gen_comp(std::ostream &os, const std::string &dest, const std::string &src1, const std::string &src2, const std::string &op) const = 0;

};
#endif
