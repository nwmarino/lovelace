#include "../../include/graph/CFG.hpp"
#include "../../include/machine/MachBasicBlock.hpp"
#include "../../include/machine/MachFunction.hpp"
#include "../../include/machine/MachInstruction.hpp"
#include "../../include/machine/MachObject.hpp"
#include "../../include/machine/MachRegister.hpp"
#include "../../include/machine/RegisterAllocator.hpp"
#include "../../include/machine/RegisterAnalysis.hpp"

#include "../../include/X64/X64.hpp"

#ifdef SPBE_MACHINE_DEBUGGING
#include <iostream>
#endif // SPBE_MACHINE_DEBUGGING

using namespace spbe;

/// Implementation of a linear scan over the intermediate representation to
/// identify live ranges of named producers.
class LinearScan final {
    const MachFunction& m_function;
    std::vector<LiveRange>& m_ranges;

    LiveRange& update_range(MachRegister reg, RegisterClass cls, uint32_t pos) {
        // Attempt to find an existing range for |reg|.
        for (auto& range : m_ranges) {
            // The range has been killed, so we never update it.
            if (range.killed)
                continue;

            if (range.reg == reg) {
                range.end = pos;
                return range;
            }
        }

        // No existing range could be found, so we begin a new one.
        LiveRange range = {};
        range.reg = reg;
        range.start = range.end = pos;
        range.cls = cls;
        range.killed = false;
        range.alloc = MachRegister::NoRegister;

        if (reg.is_physical())
            range.alloc = reg;

        m_ranges.push_back(range);
        return m_ranges.back();
    }

public:
    LinearScan(const MachFunction& function, std::vector<LiveRange>& ranges)
        : m_function(function), m_ranges(ranges) {}

    LinearScan(const LinearScan&) = delete;
    LinearScan& operator = (const LinearScan&) = delete;

    void run() {
        uint32_t position = 0;

        // TODO: Triple loop here should get factored away.
        for (const auto* mbb = m_function.front(); mbb; mbb = mbb->next()) {
            for (const auto& mi : mbb->insts()) {
                for (const auto& mo : mi.operands()) {
                    if (!mo.is_reg() && !mo.is_mem())
                        continue;

                    MachRegister reg;
                    RegisterClass cls;

                    if (mo.is_reg()) {
                        reg = mo.get_reg();
                    } else if (mo.is_mem()) {
                        reg = mo.get_mem_base();
                    }

                    if (reg.is_physical()) {
                        cls = x64::get_class(
                            static_cast<x64::Register>(reg.id()));
                    } else {
                        // reg refers to a virtual register, whose
                        // information is stored in the parent function.
                        const auto& regi = m_function.get_register_info();
                        assert(regi.vregs.count(reg.id()) != 0);
                        cls = regi.vregs.at(reg.id()).cls;
                    }

                    LiveRange& range = update_range(reg, cls, position);
                    if (mo.is_reg() && mo.is_kill()) {
                        range.end = position;
                        range.killed = true;
                    }
                }

                ++position;
            }
        }
    }
};

/// Implementation of a per-function pass that identifies the need for spills
/// around call instructions due to ABI conventions.
class CallsiteAnalysis final {
    MachFunction& m_function;
    const std::vector<LiveRange>& m_ranges;

public:
    CallsiteAnalysis(
            MachFunction& function, const std::vector<LiveRange>& ranges)
        : m_function(function), m_ranges(ranges) {}

    CallsiteAnalysis(const CallsiteAnalysis&) = delete;
    CallsiteAnalysis& operator = (const CallsiteAnalysis&) = delete;
    
    void run() {
        uint32_t position = 0;
        for (auto* mbb = m_function.front(); mbb != nullptr; mbb = mbb->next()) {
            std::vector<MachInstruction> insts;
            insts.reserve(mbb->size());

            for (uint32_t i = 0; i < mbb->size(); ++position, ++i) {
                // TODO: Generalize for other targets.
                if (x64::is_call_opcode(static_cast<x64::Opcode>(mbb->insts().at(i).opcode()))) {
                    std::vector<MachRegister> save = {};

                    for (auto& range : m_ranges) {
                        if (range.overlaps(position)) {
                            MachRegister range_alloc = range.alloc;
                            if (x64::is_caller_saved(static_cast<x64::Register>(range.alloc.id())))
                                save.push_back(range.alloc);
                        }
                    }

                    for (const auto& reg : save) {
                        MachOperand op = MachOperand::create_reg(reg, 8, false);
                        insts.push_back({ x64::PUSH64, { op } });   
                    }

                    insts.push_back(mbb->insts().at(i));
                    
                    for (const auto& reg : save) {
                        MachOperand op = MachOperand::create_reg(reg, 8, true);
                        insts.push_back({ x64::POP64, { op } });
                    }
                } else {
                    insts.push_back(mbb->insts().at(i));
                }
            }

            mbb->insts() = insts;
        }
    }
};

void RegisterAnalysis::run() {
    for (const auto& [name, function] : m_obj.functions()) {
        std::vector<LiveRange> ranges;
        
        LinearScan linscan { *function, ranges };
        linscan.run();

        TargetRegisters tregs;
        switch (m_obj.get_target()->arch()) {
        case Target::Arch::x64:
            tregs = x64::get_registers();
            break;

        default:
            assert(false && "unsupported architecture!");
        }

#ifdef SPBE_MACHINE_DEBUGGING
        std::cerr << "Function '" << name << "' ranges:\n";
        for (const auto& range : ranges) {
            if (range.reg.is_virtual()) {
                std::cerr << 'v' << range.reg.id() - MachRegister::VirtualBarrier;
            } else {
                std::cerr << '%' << x64::to_string(static_cast<x64::Register>(
                    range.reg.id()), 8);
            }

            std::cerr << " [" << range.start << ", " << range.end << "]\n";
        }
#endif // SPBE_MACHINE_DEBUGGING

        RegisterAllocator allocator { *function, tregs, ranges };
        allocator.run();

        FunctionRegisterInfo& regi = function->get_register_info();
        for (const auto& range : ranges) {
            MachRegister reg = range.reg;
            if (reg.is_physical())
                continue;

            regi.vregs[reg.id()].alloc = range.alloc;
        }

        CallsiteAnalysis CAN { *function, ranges };
        CAN.run();
    }
}
