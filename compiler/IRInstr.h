#ifndef IRINSTR_H
#define IRINSTR_H

#include <vector>
#include <string>
#include <ostream> 

#include "CodeGenBackend.h"

class BasicBlock;

class IRInstr {
    public:
        IRInstr(BasicBlock* bb_, std::vector<std::string> params_) 
            : bb(bb_), params(params_) {}
        virtual ~IRInstr() = default;

        virtual void gen_asm(std::ostream &o) = 0;
        std::vector<std::string> getParams();

    protected:
        BasicBlock *bb;
        std::vector<std::string> params;
};

class IRReturn : public IRInstr {
    public:
        IRReturn(BasicBlock *bb, const std::string &src)
        : IRInstr(bb, {src}) {}
        void gen_asm(std::ostream &o) override;
};

class IRLdConst : public IRInstr {
    public:
        IRLdConst(BasicBlock *bb, const std::string &dest, const std::string &constant)
        : IRInstr(bb, {dest, constant}) {}
        void gen_asm(std::ostream &o) override;
};

class IRCopy : public IRInstr {
    public:
        IRCopy(BasicBlock *bb, const std::string &dest, const std::string &src)
        : IRInstr(bb, {dest, src}) {}
        void gen_asm(std::ostream &o) override;
};

class IRAdd : public IRInstr {
    public:
        IRAdd(BasicBlock *bb, const std::string &dest, const std::string &src1, const std::string &src2)
        : IRInstr(bb, {dest, src1, src2}) {}
        void gen_asm(std::ostream &o) override;
};

class IRSub : public IRInstr {
    public:
        IRSub(BasicBlock *bb, const std::string &dest, const std::string &src1, const std::string &src2)
        : IRInstr(bb, {dest, src1, src2}) {}
        void gen_asm(std::ostream &o) override;
};

class IRMul : public IRInstr {
    public:
        IRMul(BasicBlock *bb, const std::string &dest, const std::string &src1, const std::string &src2)
        : IRInstr(bb, {dest, src1, src2}) {}
        void gen_asm(std::ostream &o) override;
};

class IRDiv : public IRInstr {
    public:
        IRDiv(BasicBlock *bb, const std::string &dest, const std::string &src1, const std::string &src2)
        : IRInstr(bb, {dest, src1, src2}) {}
        void gen_asm(std::ostream &o) override;
};

class IRMod : public IRInstr {
    public:
        IRMod(BasicBlock *bb, const std::string &dest, const std::string &src1, const std::string &src2)
        : IRInstr(bb, {dest, src1, src2}) {}
        void gen_asm(std::ostream &o) override;
};

// Nouvelle instruction pour copier dans un registre (par exemple, pour mettre un argument dans %edi)
class IRMovReg : public IRInstr {
    public:
        // Ici, dest sera un registre (par exemple "%edi") et src est l'opérande à déplacer
        IRMovReg(BasicBlock *bb, const std::string &dest, const std::string &src)
            : IRInstr(bb, {dest, src}) {}
        virtual void gen_asm(std::ostream &o) override;
};

class IRCall : public IRInstr {
    public:
        IRCall(BasicBlock *bb, const std::string &func)
        : IRInstr(bb, {func}) {}
        void gen_asm(std::ostream &o) override;
};

class IRNot : public IRInstr {
    public:
        IRNot(BasicBlock *bb, const std::string &dest, const std::string &src)
        : IRInstr(bb, {dest, src}) {}
        void gen_asm(std::ostream &o) override;
};

class IRXor : public IRInstr {
    public:
        IRXor(BasicBlock *bb, const std::string &dest, const std::string &src1, const std::string &src2)
        : IRInstr(bb, {dest, src1, src2}) {}
        void gen_asm(std::ostream &o) override;
};

class IROr : public IRInstr {
    public:
        IROr(BasicBlock *bb, const std::string &dest, const std::string &src1, const std::string &src2)
        : IRInstr(bb, {dest, src1, src2}) {}
        void gen_asm(std::ostream &o) override;
};

class IREgal : public IRInstr {
    public:
        IREgal(BasicBlock *bb, const std::string &dest, const std::string &src1, const std::string &src2)
        : IRInstr(bb, {dest, src1, src2}) {}
        void gen_asm(std::ostream &o) override;
};

class IRNotEgal : public IRInstr {
    public:
        IRNotEgal(BasicBlock *bb, const std::string &dest, const std::string &src1, const std::string &src2)
        : IRInstr(bb, {dest, src1, src2}) {}
        void gen_asm(std::ostream &o) override;
};

class IRAnd : public IRInstr {
    public:
        IRAnd(BasicBlock *bb, const std::string &dest, const std::string &src1, const std::string &src2)
            : IRInstr(bb, {dest, src1, src2}) {}
    
        void gen_asm(std::ostream &o) override;
    };    

class IRComp : public IRInstr {
    public:
        // Le constructeur prend en plus une chaîne 'op' qui représente l'opérateur ("<", ">", ">=", "<=")
        IRComp(BasicBlock *bb, const std::string &dest, const std::string &src1, const std::string &src2, const std::string &op)
            : IRInstr(bb, {dest, src1, src2}), op(op) {}
        virtual void gen_asm(std::ostream &o) override;
    private:
        std::string op;
    };


#endif
