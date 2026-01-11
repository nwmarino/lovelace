//
//  Copyright (c) 2025-2026 Nick Marino
//  All rights reserved.
//

#include "lace/codegen/LIRCodegen.hpp"
#include "lace/tree/Defn.hpp"
#include "lace/tree/Type.hpp"
#include "lir/graph/Type.hpp"

using namespace lace;

void Codegen::codegen_cast_integer(lir::Value* value, lir::Type* dest, bool is_signed) {
	if (dest->is_integer_type()) {
        const uint32_t source_size = m_mach.get_size(value->get_type());
        const uint32_t dest_size = m_mach.get_size(dest);

        if (const lir::Integer* integer = dynamic_cast<lir::Integer*>(value)) {
            m_temp = lir::Integer::get(m_cfg, dest, integer->get_value());
        } else if (source_size > dest_size) {
            m_temp = m_builder.build_itrunc(dest, value);
        } else if (source_size) {
            if (is_signed) {
            	m_temp = m_builder.build_sext(dest, value);
            } else {
                m_temp = m_builder.build_zext(dest, value);
            }
        }
    } else if (dest->is_float_type()) {
        if (const lir::Float* fp = dynamic_cast<lir::Float*>(value)) {
            m_temp = lir::Float::get(m_cfg, dest, fp->get_value());
        } else if (is_signed) {
            m_temp = m_builder.build_s2f(dest, value);
        } else {
            m_temp = m_builder.build_u2f(dest, value);
        }
    } else if (dest->is_pointer_type()) {
        m_temp = m_builder.build_i2p(dest, value);
    } else {
        assert(false && "unsupported integer type cast!");
    }
}

void Codegen::codegen_cast_float(lir::Value* value, lir::Type* dest) {
	if (dest->is_integer_type()) {
        m_temp = m_builder.build_f2i(dest, value);
    } else if (dest->is_float_type()) {
        const uint32_t source_size = m_mach.get_size(value->get_type());
        const uint32_t dest_size = m_mach.get_size(dest);

        if (const lir::Float* fp = dynamic_cast<lir::Float*>(value)) {
            m_temp = lir::Float::get(m_cfg, dest, fp->get_value());
        } else if (source_size > dest_size) {
            m_temp = m_builder.build_ftrunc(dest, value);
        } else if (source_size < dest_size) {
            m_temp = m_builder.build_fext(dest, value);
        }
    } else {
        assert(false && "unsupported float type cast!");
    }
}

void Codegen::codegen_cast_array(lir::Value* value, lir::Type* dest) {
	if (dest->is_pointer_type()) {
		m_temp = m_builder.build_reint(dest, value);
	} else {
		assert(false && "unsupported array type cast!");
	}
}

void Codegen::codegen_cast_pointer(lir::Value* value, lir::Type* dest) {
	if (dest->is_integer_type()) {
		m_temp = m_builder.build_p2i(dest, value);
	} else if (dest->is_pointer_type()) {
		if (dynamic_cast<lir::Null*>(value)) {
			m_temp = lir::Null::get(m_cfg, dest);
		} else {
			m_temp = m_builder.build_reint(dest, value);
		}
	} else {
		assert(false && "unsupported pointer type cast!");
	}
}

void Codegen::visit(CastExpr& node) {
	m_vctx = RValue;
	node.get_expr()->accept(*this);

	lir::Type* source = m_temp->get_type();
	lir::Type* dest = lower_type(node.get_type());

	if (*source == *dest)
		return;

	if (source->is_integer_type()) {
		const bool is_signed = node.get_type()->is_signed_integer();
		return codegen_cast_integer(m_temp, dest, is_signed);
	} else if (source->is_float_type()) {
		return codegen_cast_float(m_temp, dest);
	} else if (source->is_array_type()) {
		return codegen_cast_array(m_temp, dest);
	} else if (source->is_pointer_type()) {
		return codegen_cast_pointer(m_temp, dest);
	}

	assert(false && "unsupported type cast!");
}
