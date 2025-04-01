#ifndef IROPERATION_H
#define IROPERATION_H

#include <string>
#include <vector>
#include <ostream>
#include <memory>

#include "CodeGenBackend.h"

/**
 * Classe de base pour une opération IR.
 */
class IROperation {
public:
    IROperation(const std::vector<std::string>& params, size_t size)
        : params(params), size(size) {}
    virtual ~IROperation() {}
    virtual void gen_asm(std::ostream &os) const = 0;
protected:
    std::vector<std::string> params;
    size_t size;
};

/// On déclare un pointeur global (ou mieux, un singleton ou une instance dans le CFG) pour le backend.
extern const class CodeGenBackend* codegenBackend;

/**
 * Opération d'addition pour ARM64 utilisant le backend.
 */
class IRAdd : public IROperation {
public:
    IRAdd(const std::string &dest, const std::string &src1, const std::string &src2, size_t size)
        : IROperation({dest, src1, src2}, size) {}
        
    virtual void gen_asm(std::ostream &os) const override {
        // Déléguer à l'objet backend pour générer l'instruction d'addition
        codegenBackend->gen_add(os, params[0], params[1], params[2]);
    }
};

// De même, pour IRLdConst, IRSub, IRMul, etc.
class IRLdConst : public IROperation {
public:
    IRLdConst(const std::string &dest, const std::string &constant, size_t size)
        : IROperation({dest, constant}, size) {}
        
    virtual void gen_asm(std::ostream &os) const override {
        codegenBackend->gen_mov(os, params[0], params[1]);
    }
};


class IRCopy : public IROperation {
public:
    IRCopy(const std::string &dest, const std::string &src, size_t size)
        : IROperation({dest, src}, size) {}
    virtual void gen_asm(std::ostream &os) const override {
         os << "    mov " << params[0] << ", " << params[1] << "\n";
    }
};


class IRSub : public IROperation {
public:
    IRSub(const std::string &dest, const std::string &src1, const std::string &src2, size_t size)
        : IROperation({dest, src1, src2}, size) {}
        
    virtual void gen_asm(std::ostream &os) const override {
        codegenBackend->gen_sub(os, params[0], params[1], params[2]);
    }
};

class IRMul : public IROperation {
public:
    IRMul(const std::string &dest, const std::string &src1, const std::string &src2, size_t size)
        : IROperation({dest, src1, src2}, size) {}
        
    virtual void gen_asm(std::ostream &os) const override {
        codegenBackend->gen_mul(os, params[0], params[1], params[2]);
    }
};

#endif
