#include "../../include/machine/MachObject.hpp"

using namespace spbe;

MachObject::~MachObject() {
    for (auto& [name, function] : m_functions) {
        delete function;
        function = nullptr;
    }

    m_functions.clear();
}
