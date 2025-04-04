#include "IRInstr.h"
#include "IR.h" 

extern const CodeGenBackend* codegenBackend;

std::vector<std::string> IRInstr::getParams(){
    return params;
}

void IRReturn::gen_asm(std::ostream &o) {
    codegenBackend->gen_return(o, bb->cfg->IR_reg_to_asm(params[0]));
}

void IRLdConst::gen_asm(std::ostream &o) {
    codegenBackend->gen_mov(o, bb->cfg->IR_reg_to_asm(params[0]), params[1]);
}

void IRCopy::gen_asm(std::ostream &o) {
    codegenBackend->gen_copy(o,
        bb->cfg->IR_reg_to_asm(params[0]),
        bb->cfg->IR_reg_to_asm(params[1]));
}

void IRAdd::gen_asm(std::ostream &o) {
    codegenBackend->gen_add(o,
        bb->cfg->IR_reg_to_asm(params[0]),
        bb->cfg->IR_reg_to_asm(params[1]),
        bb->cfg->IR_reg_to_asm(params[2]));
}

void IRSub::gen_asm(std::ostream &o) {
    codegenBackend->gen_sub(o,
        bb->cfg->IR_reg_to_asm(params[0]),
        bb->cfg->IR_reg_to_asm(params[1]),
        bb->cfg->IR_reg_to_asm(params[2]));
}

void IRMul::gen_asm(std::ostream &o) {
    codegenBackend->gen_mul(o,
        bb->cfg->IR_reg_to_asm(params[0]),
        bb->cfg->IR_reg_to_asm(params[1]),
        bb->cfg->IR_reg_to_asm(params[2]));
}

void IRDiv::gen_asm(std::ostream &o) {
    codegenBackend->gen_div(o,
        bb->cfg->IR_reg_to_asm(params[0]),
        bb->cfg->IR_reg_to_asm(params[1]),
        bb->cfg->IR_reg_to_asm(params[2]));
}

void IRMod::gen_asm(std::ostream &o) {
    codegenBackend->gen_mod(o,
        bb->cfg->IR_reg_to_asm(params[0]),
        bb->cfg->IR_reg_to_asm(params[1]),
        bb->cfg->IR_reg_to_asm(params[2]));
}

void IRCall::gen_asm(std::ostream &o) {
    codegenBackend->gen_call(o, params[0]); // noms de fonctions = pas besoin d'offset
}

void IRNot::gen_asm(std::ostream &o) {
    codegenBackend->gen_not(o,
        bb->cfg->IR_reg_to_asm(params[0]),
        bb->cfg->IR_reg_to_asm(params[1]));
}

void IRXor::gen_asm(std::ostream &o) {
    codegenBackend->gen_xor(o,
        bb->cfg->IR_reg_to_asm(params[0]),
        bb->cfg->IR_reg_to_asm(params[1]),
        bb->cfg->IR_reg_to_asm(params[2]));
}

void IROr::gen_asm(std::ostream &o) {
    codegenBackend->gen_or(o,
        bb->cfg->IR_reg_to_asm(params[0]),
        bb->cfg->IR_reg_to_asm(params[1]),
        bb->cfg->IR_reg_to_asm(params[2]));
}

void IREgal::gen_asm(std::ostream &o) {
    codegenBackend->gen_egal(o,
        bb->cfg->IR_reg_to_asm(params[0]),
        bb->cfg->IR_reg_to_asm(params[1]),
        bb->cfg->IR_reg_to_asm(params[2]));
}

void IRNotEgal::gen_asm(std::ostream &o) {
    codegenBackend->gen_notegal(o,
        bb->cfg->IR_reg_to_asm(params[0]),
        bb->cfg->IR_reg_to_asm(params[1]),
        bb->cfg->IR_reg_to_asm(params[2]));
}

void IRAnd::gen_asm(std::ostream &o)  {
    codegenBackend->gen_and(o,
        bb->cfg->IR_reg_to_asm(params[0]),
        bb->cfg->IR_reg_to_asm(params[1]),
        bb->cfg->IR_reg_to_asm(params[2]));
}


void IRGt::gen_asm(std::ostream &o) {
    // Utilise la méthode gen_gt du backend pour générer l'assembleur
    codegenBackend->gen_gt(o,
        bb->cfg->IR_reg_to_asm(params[0]),
        bb->cfg->IR_reg_to_asm(params[1]),
        bb->cfg->IR_reg_to_asm(params[2])
    );
}

void IRGe::gen_asm(std::ostream &o) {
    codegenBackend->gen_ge(o,
        bb->cfg->IR_reg_to_asm(params[0]),
        bb->cfg->IR_reg_to_asm(params[1]),
        bb->cfg->IR_reg_to_asm(params[2])
    );
}

void IRPutChar::gen_asm(std::ostream &o) {
    codegenBackend->gen_copy(o, "w0", bb->cfg->IR_reg_to_asm(params[0]));
    codegenBackend->gen_call(o, "putchar");
}


void IRGetChar::gen_asm(std::ostream &o) {
    codegenBackend->gen_call(o, "getchar");
    codegenBackend->gen_copy(o, bb->cfg->IR_reg_to_asm(params[0]), "w0");
}
