#ifndef IROPERAND_H
#define IROPERAND_H

#include <string>


namespace ir {

enum class Type {
    Int,
    Float,
    IntLiteral,
    FloatLiteral,
    IntPtr,
    FloatPtr,
    null
};

std::string toString(Type t);

struct Operand {
    std::string name;
    Type type;
    bool operator < (const Operand& op) const{      // Overload the operator for Operand
        return name < op.name;
    }
    Operand(std::string = "null", Type = Type::null);
};

}
#endif
