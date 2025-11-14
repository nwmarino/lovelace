#include "graph/Instruction.hpp"
#include "graph/Use.hpp"

using namespace spbe;

Use::Use(Value* value, User* user) : m_value(value), m_user(user) {
    assert(value && "use value cannot be null!");
    assert(user && "use user cannot be null!");

    if (auto* phi_op = dynamic_cast<PhiOperand*>(value)) {
        phi_op->get_value()->add_use(this);
    } else {
        value->add_use(this);
    }
}