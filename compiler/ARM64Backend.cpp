#include "ARM64Backend.h"
#include <iostream>
#include <cctype>

void ARM64Backend::gen_return(std::ostream &os, const std::string &src) const {
    os << "    ldr w0, " << src << "\n";
}

void ARM64Backend::gen_mov(std::ostream &os, const std::string &dest, const std::string &src) const {
    // Adjust dest if it’s x86-style or a plain offset
    std::string adjusted_dest = dest;
    if (dest.find("(%rbp)") != std::string::npos) {
        adjusted_dest = "[x29, #" + dest.substr(0, dest.find("(%rbp)")) + "]";
    } else if (dest.find('[') == std::string::npos) {
        adjusted_dest = "[x29, #" + dest + "]";
    }

    if (isdigit(src[0]) || (src[0] == '-' && isdigit(src[1]))) {
        os << "    mov w0, #" << src << "\n";
        os << "    str w0, " << adjusted_dest << "\n";
    } else {
        // Adjust src if it’s x86-style or a plain offset
        std::string adjusted_src = src;
        if (src.find("(%rbp)") != std::string::npos) {
            adjusted_src = "[x29, #" + src.substr(0, src.find("(%rbp)")) + "]";
        } else if (src.find('[') == std::string::npos) {
            adjusted_src = "[x29, #" + src + "]";
        }
        os << "    ldr w0, " << adjusted_src << "\n";
        os << "    str w0, " << adjusted_dest << "\n";
    }
}

void ARM64Backend::gen_add(std::ostream &os, const std::string &dest,
                           const std::string &src1, const std::string &src2) const {
    os << "    ldr w0, " << src1 << "\n";     // Load src1 to w0
    os << "    ldr w1, " << src2 << "\n";     // Load src2 to w1
    os << "    add w0, w0, w1\n";            // Add w1 to w0
    os << "    str w0, " << dest << "\n";     // Store result to dest
}

void ARM64Backend::gen_sub(std::ostream &os, const std::string &dest,
                           const std::string &src1, const std::string &src2) const {
    os << "    ldr w0, " << src1 << "\n";     // Load src1 to w0
    os << "    ldr w1, " << src2 << "\n";     // Load src2 to w1
    os << "    sub w0, w0, w1\n";            // Subtract w1 from w0
    os << "    str w0, " << dest << "\n";     // Store result to dest
}

void ARM64Backend::gen_mul(std::ostream &os, const std::string &dest,
                           const std::string &src1, const std::string &src2) const {
    os << "    ldr w0, " << src1 << "\n";     // Load src1 to w0
    os << "    ldr w1, " << src2 << "\n";     // Load src2 to w1
    os << "    mul w0, w0, w1\n";            // Multiply w0 by w1
    os << "    str w0, " << dest << "\n";     // Store result to dest
}

void ARM64Backend::gen_div(std::ostream &os, const std::string &dest,
                           const std::string &src1, const std::string &src2) const {
    os << "    ldr w0, " << src1 << "\n";     // Load src1 to w0 (dividend)
    os << "    ldr w1, " << src2 << "\n";     // Load src2 to w1 (divisor)
    os << "    sdiv w0, w0, w1\n";           // Signed divide w0 by w1
    os << "    str w0, " << dest << "\n";     // Store quotient to dest
}

void ARM64Backend::gen_mod(std::ostream &os, const std::string &dest,
                           const std::string &src1, const std::string &src2) const {
    os << "    ldr w0, " << src1 << "\n";     // Load src1 to w0 (dividend)
    os << "    ldr w1, " << src2 << "\n";     // Load src2 to w1 (divisor)
    os << "    sdiv w2, w0, w1\n";           // Signed divide w0 by w1, quotient in w2
    os << "    msub w0, w2, w1, w0\n";       // w0 = w0 - (w2 * w1), remainder in w0
    os << "    str w0, " << dest << "\n";     // Store remainder to dest
}

void ARM64Backend::gen_not(std::ostream &os, const std::string &dest,
                           const std::string &src) const {
    os << "    ldr w0, " << src << "\n";      // Load src to w0
    os << "    cmp w0, #0\n";                // Compare with 0
    os << "    cset w0, eq\n";               // Set w0 to 1 if equal (src == 0), else 0
    os << "    str w0, " << dest << "\n";     // Store result to dest
}

void ARM64Backend::gen_egal(std::ostream &os, const std::string &dest,
                            const std::string &src1, const std::string &src2) const {
    os << "    ldr w0, " << src1 << "\n";     // Load src1 to w0
    os << "    ldr w1, " << src2 << "\n";     // Load src2 to w1
    os << "    cmp w0, w1\n";                // Compare w0 and w1
    os << "    cset w0, eq\n";               // Set w0 to 1 if equal, else 0
    os << "    str w0, " << dest << "\n";     // Store result to dest
}

void ARM64Backend::gen_notegal(std::ostream &os, const std::string &dest,
                               const std::string &src1, const std::string &src2) const {
    os << "    ldr w0, " << src1 << "\n";     // Load src1 to w0
    os << "    ldr w1, " << src2 << "\n";     // Load src2 to w1
    os << "    cmp w0, w1\n";                // Compare w0 and w1
    os << "    cset w0, ne\n";               // Set w0 to 1 if not equal, else 0
    os << "    str w0, " << dest << "\n";     // Store result to dest
}

void ARM64Backend::gen_xor(std::ostream &os, const std::string &dest,
                           const std::string &src1, const std::string &src2) const {
    std::cerr << "[gen_xor] dest = " << dest << ", src1 = " << src1 << ", src2 = " << src2 << "\n";
    os << "    ldr w0, " << src1 << "\n";     // Load src1 to w0
    os << "    ldr w1, " << src2 << "\n";     // Load src2 to w1
    os << "    eor w0, w0, w1\n";            // XOR w0 with w1
    os << "    str w0, " << dest << "\n";     // Store result to dest
}

void ARM64Backend::gen_or(std::ostream &os, const std::string &dest,
                          const std::string &src1, const std::string &src2) const {
    std::cerr << "[gen_or] dest = " << dest << ", src1 = " << src1 << ", src2 = " << src2 << "\n";
    os << "    ldr w0, " << src1 << "\n";     // Load src1 to w0
    os << "    ldr w1, " << src2 << "\n";     // Load src2 to w1
    os << "    orr w0, w0, w1\n";            // OR w0 with w1
    os << "    str w0, " << dest << "\n";     // Store result to dest
}

void ARM64Backend::gen_call(std::ostream &os, const std::string &func) const {
    os << "    bl _" << func << "\n";  // NOTE: underscore before function
}


void ARM64Backend::gen_prologue(std::ostream &os, std::string &name, int stackSize) const {
    os << ".globl _" << name << "\n";
    os << "_" << name << ":\n";

    // Standard prologue
    os << "    stp x29, x30, [sp, #-16]!\n";
    os << "    mov x29, sp\n";

    // Allocate space for local variables (aligned)
    int aligned = ((stackSize + 15) / 16) * 16;
    if (aligned > 0) {
        os << "    sub sp, sp, #" << aligned << "\n";
    }
}

void ARM64Backend::gen_epilogue(std::ostream &os) const {
    os << "    mov sp, x29\n";  // Restore sp before popping
    os << "    ldp x29, x30, [sp], #16\n";
    os << "    ret\n";
}


void ARM64Backend::gen_copy(std::ostream &os, const std::string &dest,
                            const std::string &src) const {
    bool srcIsMem = src.find('[') != std::string::npos;
    bool destIsMem = dest.find('[') != std::string::npos;

    if (srcIsMem && destIsMem) {
        os << "    ldr w0, " << src << "\n";
        os << "    str w0, " << dest << "\n";
    } else if (srcIsMem && !destIsMem) {
        os << "    ldr " << dest << ", " << src << "\n";
    } else if (!srcIsMem && destIsMem) {
        os << "    str " << src << ", " << dest << "\n";
    } else { // register to register
        os << "    mov " << dest << ", " << src << "\n";
    }
}


void ARM64Backend::gen_and(std::ostream &os,
                           const std::string &dest,
                           const std::string &src1,
                           const std::string &src2) const {
    os << loadOperand(src1, "w0");
    os << loadOperand(src2, "w1");
    os << "    and w0, w0, w1\n";            // AND w0 with w1
    os << "    str w0, " << dest << "\n";     // Store result to dest
}

std::string ARM64Backend::getTempPrefix() const {
    return "!tmp";
}

std::string ARM64Backend::getArchitecture() const {
    return "arm64";
}

std::string ARM64Backend::adjustMemOperand(const std::string &op) const {
    if (op.find("(%rbp)") != std::string::npos)
        return "[x29, #" + op.substr(0, op.find("(%rbp)")) + "]";
    if (op.find('[') == std::string::npos)
        return "[x29, #" + op + "]";
    return op;
}

std::string ARM64Backend::loadOperand(const std::string &operand, const std::string &targetReg) const {
    // Vérifie si l'opérande est un immédiat en détectant le caractère '#' en première position.
    if (!operand.empty() && operand[0] == '#') {
        // Si c'est un immédiat, on génère une instruction mov.
        return "    mov " + targetReg + ", " + operand + "\n";
    }
    // Sinon, on suppose que l'opérande est déjà au format mémoire (ex. "[x29, #...]" ou similaire)
    return "    ldr " + targetReg + ", " + operand + "\n";
}

void ARM64Backend::gen_ge(std::ostream &os, const std::string &dest,
                           const std::string &src1, const std::string &src2) const {
    os << "    ldr x0, " << src1 << "\n";
    os << "    ldr x1, " << src2 << "\n";
    os << "    cmp x0, x1\n";
    os << "    cset x0, ge\n"; // Renvoie 1 si x0 >= x1, sinon 0
    os << "    str x0, " << dest << "\n";
}

void ARM64Backend::gen_gt(std::ostream &os, const std::string &dest,
                           const std::string &src1, const std::string &src2) const {
    os << "    ldr x0, " << src1 << "\n";
    os << "    ldr x1, " << src2 << "\n";
    os << "    cmp x0, x1\n";
    os << "    cset x0, gt\n"; // Renvoie 1 si x0 > x1, sinon 0
    os << "    str x0, " << dest << "\n";
}

std::string makeLocalLabel(const std::string &label) {
    if (!label.empty() && label[0] == '.') {
        return label.substr(1);
    }
    return label;
}



// Génère un saut inconditionnel vers le label cible.
void ARM64Backend::gen_jump(std::ostream &os, const std::string &target) const {
    os << "    b " << target << "\n";
}

void ARM64Backend::gen_branch(std::ostream &os, const std::string &cond,
                                const std::string &label_then, const std::string &label_else) const {
    // On suppose que 'cond' est déjà dans un registre (par exemple, x0).
    // On émet la branche conditionnelle en utilisant les labels locaux.
    os << "    cbz x0, " << label_else << "\n";
    os << "    b " << label_then << "\n";
}

void ARM64Backend::gen_jump_cond(std::ostream &os, const std::string &cond,
                                 const std::string &labelTrue,
                                 const std::string &labelFalse) const {
    // Charger la valeur de 'cond' dans w0.
    os << "    ldr w0, " << cond << "\n";
    // Si w0 n'est pas zéro, branche vers labelTrue.
    os << "    cbnz w0, " << labelTrue << "\n";
    // Sinon, saute inconditionnellement vers labelFalse.
    os << "    b " << labelFalse << "\n";
}

