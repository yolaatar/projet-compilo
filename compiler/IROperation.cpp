#include "IR.h"

std::string IROperation::getOffset(const std::string &varName) const {
    return std::to_string(bb->cfg->getOffset(varName));
}