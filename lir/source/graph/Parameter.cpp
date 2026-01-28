//
//  Copyright (c) 2026 Nicholas Marino
//  All rights reserved.
//

#include "lir/graph/Function.hpp"
#include "lir/graph/Parameter.hpp"

using namespace lir;

Parameter *Parameter::create(Type *type, const std::string &name, 
                             Function *parent) {
    Parameter *param = new Parameter(type, parent, name);
    assert(param);
    
    if (parent)
        parent->add_parameter(param);

    return param;
}

uint32_t Parameter::get_index() const {
    assert(has_parent() && "parameter does not belong to a function!");

    uint32_t i = 0;
    for (const Parameter *param : get_parent()->get_parameters()) {
        if (param == this)
            return i;

        i++;
    }

    assert(false && "parameter is missing from parent function!");
}
