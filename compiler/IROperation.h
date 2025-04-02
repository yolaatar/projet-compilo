#ifndef IROPERATION_H
#define IROPERATION_H

#include <string>
#include <vector>
#include <ostream>
#include <memory>

#include "CodeGenBackend.h"

class BasicBlock;

/**
 * Classe de base pour une opération IR.
 */
class IROperation {
public:
    // NB : IROperation n'apporte aucune abstraction logique de plus que IRInstr (on devrait l'enlever mais tant pis)
    IROperation(BasicBlock *bb, const std::vector<std::string>& params, size_t size)
        : bb(bb), params(params), size(size) {}
    virtual ~IROperation() {}
    virtual void gen_asm(std::ostream &os) const = 0;
protected:
    BasicBlock *bb;
    std::vector<std::string> params;
    size_t size;

    std::string getOffset(const std::string &varName) const;
};

/// On déclare un pointeur global (ou mieux, un singleton ou une instance dans le CFG) pour le backend.
extern const CodeGenBackend* codegenBackend;

/**
 * Opération d'addition.
 * Génère par exemple : add dest, src1, src2
 */
class IRAdd : public IROperation {
public:
    IRAdd(BasicBlock *bb, const std::string &dest, const std::string &src1, const std::string &src2, size_t size)
        : IROperation(bb, {dest, src1, src2}, size) {}
    virtual void gen_asm(std::ostream &os) const override {
        codegenBackend->gen_add(os, getOffset(params[0]), getOffset(params[1]), getOffset(params[2]));
    }
};

/**
 * Opération de chargement d'une constante.
 * Génère par exemple : mov dest, #constant
 */
class IRLdConst : public IROperation {
public:
    IRLdConst(BasicBlock *bb, const std::string &dest, const std::string &constant, size_t size)
        : IROperation(bb, {dest, constant}, size) {}
    virtual void gen_asm(std::ostream &os) const override {
        codegenBackend->gen_mov(os, getOffset(params[0]), params[1]);
    }
};

/**
 * Opération de copie (affectation).
 * Génère par exemple : mov dest, src
 */
class IRCopy : public IROperation {
public:
    IRCopy(BasicBlock *bb, const std::string &dest, const std::string &src, size_t size)
        : IROperation(bb, {dest, src}, size) {}
    virtual void gen_asm(std::ostream &os) const override {
        codegenBackend->gen_copy(os, getOffset(params[0]), getOffset(params[1]));
    }
};

/**
 * Opération de soustraction.
 * Génère par exemple : sub dest, src1, src2
 */
class IRSub : public IROperation {
public:
    IRSub(BasicBlock *bb, const std::string &dest, const std::string &src1, const std::string &src2, size_t size)
        : IROperation(bb, {dest, src1, src2}, size) {}
    virtual void gen_asm(std::ostream &os) const override {
        codegenBackend->gen_sub(os, params[0], params[1], params[2]);
    }
};

/**
 * Opération de multiplication.
 * Génère par exemple : mul dest, src1, src2
 */
class IRMul : public IROperation {
public:
    IRMul(BasicBlock *bb, const std::string &dest, const std::string &src1, const std::string &src2, size_t size)
        : IROperation(bb, {dest, src1, src2}, size) {}
    virtual void gen_asm(std::ostream &os) const override {
        codegenBackend->gen_mul(os, params[0], params[1], params[2]);
    }
};

/**
 * Opération de division.
 * Génère par exemple : sdiv dest, src1, src2
 */
class IRDiv : public IROperation {
public:
    IRDiv(BasicBlock *bb, const std::string &dest, const std::string &src1, const std::string &src2, size_t size)
        : IROperation(bb, {dest, src1, src2}, size) {}
    virtual void gen_asm(std::ostream &os) const override {
        codegenBackend->gen_div(os, params[0], params[1], params[2]);
    }
};

/**
 * Opération de modulo.
 */
class IRMod : public IROperation {
public:
    IRMod(BasicBlock *bb, const std::string &dest, const std::string &src1, const std::string &src2, size_t size)
        : IROperation(bb, {dest, src1, src2}, size) {}
    virtual void gen_asm(std::ostream &os) const override {
        codegenBackend->gen_mod(os, params[0], params[1], params[2]);
    }
};

/**
 * Opération de retour.
 * Génère par exemple : mov x0, src puis ret
 */
class IRReturn : public IROperation {
public:
    IRReturn(BasicBlock *bb, const std::string &src, size_t size)
        : IROperation(bb, {src}, size) {}
    virtual void gen_asm(std::ostream &os) const override {
        codegenBackend->gen_return(os, params[0]);
    }
};

/**
 * Opération d'appel de fonction.
 */
class IRCall : public IROperation {
public:
    IRCall(BasicBlock *bb, const std::string &func, size_t size)
        : IROperation(bb, {func}, size) {}
    virtual void gen_asm(std::ostream &os) const override {
        codegenBackend->gen_call(os, params[0]);
    }
};

/**
 * Opération de négation logique (NOT).
 */
class IRNot : public IROperation {
public:
    IRNot(BasicBlock *bb, const std::string &dest, const std::string &src, size_t size)
        : IROperation(bb, {dest, src}, size) {}
    virtual void gen_asm(std::ostream &os) const override {
        codegenBackend->gen_not(os, params[0], params[1]);
    }
};

/**
 * Opération XOR bit-à-bit.
 */
class IRXor : public IROperation {
public:
    IRXor(BasicBlock *bb, const std::string &dest, const std::string &src1, const std::string &src2, size_t size)
        : IROperation(bb, {dest, src1, src2}, size) {}
    virtual void gen_asm(std::ostream &os) const override {
        codegenBackend->gen_xor(os, params[0], params[1], params[2]);
    }
};

/**
 * Opération OR bit-à-bit.
 */
class IROr : public IROperation {
public:
    IROr(BasicBlock *bb, const std::string &dest, const std::string &src1, const std::string &src2, size_t size)
        : IROperation(bb, {dest, src1, src2}, size) {}
    virtual void gen_asm(std::ostream &os) const override {
        codegenBackend->gen_or(os, params[0], params[1], params[2]);
    }
};

/**
 * Opération d'égalité (==).
 */
class IREgal : public IROperation {
public:
    IREgal(BasicBlock *bb, const std::string &dest, const std::string &src1, const std::string &src2, size_t size)
        : IROperation(bb, {dest, src1, src2}, size) {}
    virtual void gen_asm(std::ostream &os) const override {
        codegenBackend->gen_egal(os, params[0], params[1], params[2]);
    }
};

/**
 * Opération de non-égalité (!=).
 */
class IRNotEgal : public IROperation {
public:
    IRNotEgal(BasicBlock *bb, const std::string &dest, const std::string &src1, const std::string &src2, size_t size)
        : IROperation(bb, {dest, src1, src2}, size) {}
    virtual void gen_asm(std::ostream &os) const override {
        codegenBackend->gen_notegal(os, params[0], params[1], params[2]);
    }
};

#endif
