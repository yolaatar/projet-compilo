#ifndef TYPE_H
#define TYPE_H

#include <string>

enum class BaseType {
    INT,
    FLOAT,
    DOUBLE,
    CHAR,
    VOID
};

class Type {
public:
    BaseType base;
    unsigned int size; // en octets

    Type(BaseType base, unsigned int size) : base(base), size(size) {}
    Type() : base(BaseType::INT), size(4) {}

};

#endif // TYPE_H
