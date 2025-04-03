#include "ARM64Backend.h"
#include <ostream>

// Génère une instruction de déplacement immediate : mov dest, #src
void ARM64Backend::gen_mov(std::ostream &os, const std::string &dest, const std::string &src) const {
    os << "    mov " << dest << ", #" << src << "\n";
}

// int ARM64Backend::getVarOffset(const std::string &varName) const {
//     // On suppose que le backend a accès à une table des symboles ou qu'on lui transmet l'offset associé.
//     // Par exemple, on pourrait avoir un map<string,int> symbolOffsets.
//     auto it = symbolOffsets.find(varName);
//     if(it != symbolOffsets.end()){
//         return it->second;
//     }
//     // Gérer le cas d'une variable non trouvée (ici, par défaut, 0)
//     return 0;
// }

// std::string ARM64Backend::mapVar(const std::string &varName) const {
//     int offset = getVarOffset(varName);
//     return "[sp, #" + std::to_string(offset) + "]";
// }

// // Génère une instruction d'addition : add dest, src1, src2
// void ARM64Backend::gen_add(std::ostream &os, const std::string &dest, const std::string &src1, const std::string &src2) const {
//     os << "    add " << dest << ", " << src1 << ", " << src2 << "\n";
// }

// // Génère une instruction de soustraction : sub dest, src1, src2
// void ARM64Backend::gen_sub(std::ostream &os, const std::string &dest, const std::string &src1, const std::string &src2) const {
//     os << "    sub " << dest << ", " << src1 << ", " << src2 << "\n";
// }

// // Génère une instruction de multiplication : mul dest, src1, src2
// void ARM64Backend::gen_mul(std::ostream &os, const std::string &dest, const std::string &src1, const std::string &src2) const {
//     os << "    mul " << dest << ", " << src1 << ", " << src2 << "\n";
// }

// // Génère une instruction de division signée : sdiv dest, src1, src2
// void ARM64Backend::gen_div(std::ostream &os, const std::string &dest, const std::string &src1, const std::string &src2) const {
//     os << "    sdiv " << dest << ", " << src1 << ", " << src2 << "\n";
// }

// // Pour modulo, on calcule le quotient puis le reste : remainder = dividend - (quotient * divisor)
// // Ici, on utilise un registre tempora fixe (par exemple, x9) pour stocker le quotient.
// void ARM64Backend::gen_mod(std::ostream &os, const std::string &dest, const std::string &src1, const std::string &src2) const {
//     os << "    sdiv x9, " << src1 << ", " << src2 << "\n";
//     os << "    mul x9, x9, " << src2 << "\n";
//     os << "    sub " << dest << ", " << src1 << ", x9\n";
// }

void ARM64Backend::gen_return(std::ostream &os, const std::string &src) const {
    // Utilise sxtw pour étendre le registre 32 bits (src) en 64 bits dans x0.
    os << "    sxtw x0, " << src << "\n";
}


// // Génère un appel de fonction avec branch with link
// void ARM64Backend::gen_call(std::ostream &os, const std::string &func) const {
//     os << "    bl " << func << "\n";
// }

// // Génère une instruction OR bit-à-bit : orr dest, src1, src2
// void ARM64Backend::gen_or(std::ostream &os, const std::string &dest, const std::string &src1, const std::string &src2) const {
//     os << "    orr " << dest << ", " << src1 << ", " << src2 << "\n";
// }

// // Génère une instruction XOR bit-à-bit : eor dest, src1, src2
// void ARM64Backend::gen_xor(std::ostream &os, const std::string &dest, const std::string &src1, const std::string &src2) const {
//     os << "    eor " << dest << ", " << src1 << ", " << src2 << "\n";
// }

// // Génère une instruction de négation bit-à-bit : mvn dest, src
// // (ARM64 ne possède pas d'instruction "not" directe, mais mvn effectue une inversion binaire)
// void ARM64Backend::gen_not(std::ostream &os, const std::string &dest, const std::string &src) const {
//     os << "    mvn " << dest << ", " << src << "\n";
// }

// // Génère une comparaison pour l'égalité : cmp src1, src2 puis cset dest, eq
// void ARM64Backend::gen_egal(std::ostream &os, const std::string &dest, const std::string &src1, const std::string &src2) const {
//     os << "    cmp " << src1 << ", " << src2 << "\n";
//     os << "    cset " << dest << ", eq\n";
// }

// // Génère une comparaison pour la non-égalité : cmp src1, src2 puis cset dest, ne
// void ARM64Backend::gen_notegal(std::ostream &os, const std::string &dest, const std::string &src1, const std::string &src2) const {
//     os << "    cmp " << src1 << ", " << src2 << "\n";
//     os << "    cset " << dest << ", ne\n";
// }

void ARM64Backend::gen_copy(std::ostream &os, const std::string &dest, const std::string &src) const {
    // Ici, 'dest' doit représenter l'adresse mémoire de la variable (par exemple, "[sp, #offset]")
    // et 'src' le registre contenant la valeur à copier (par exemple, "w0").
    os << "    str " << src << ", " << dest << "\n";
}
