#include "mgen.hpp"
#include "hexrays.hpp"
#include "log.hpp"
#include "nmips.hpp"

bool nmips_microcode_gen_t::match(codegen_t &cdg)
{
    if (cdg.insn.itype > nMIPS_todo)
    {
        return true;
    }

    return false;
}

mcode_t code_for_jcnd(nanomips_extra_inst_t jcnd_inst)
{
    switch (jcnd_inst) {
    case nMIPS_bltic:
    return m_jl;
    case nMIPS_bltiuc:
    return m_jb;
    case nMIPS_beqic:
    return m_jz;
    case nMIPS_bgeiuc:
    return m_jae;
    case nMIPS_bgeic:
    return m_jge;
    case nMIPS_bneic:
    return m_jnz;
    }

    return m_jz;
}

merror_t nmips_microcode_gen_t::apply(codegen_t &cdg)
{
    LOG("[0x%x] apply_micro", cdg.insn.ea);

    switch (cdg.insn.itype) {
    case nMIPS_bc:
    {
        cdg.emit(m_goto, 4, cdg.insn.Op1.addr, 0, 0, 0);
        return MERR_OK;
    }
    break;
    case nMIPS_bltic:
    case nMIPS_bltiuc:
    case nMIPS_beqic:
    case nMIPS_bgeic:
    case nMIPS_bgeiuc:
    case nMIPS_bneic:
    {
        auto mop1 = cdg.load_operand(0);
        auto mop2 = cdg.load_operand(1);
        // auto mop3 = cdg.load_operand(2); for some reason, tries to load from jump address
        cdg.emit(code_for_jcnd((nanomips_extra_inst_t)cdg.insn.itype), 4, mop1, mop2, cdg.insn.Op3.addr, 0);
        return MERR_OK;
    }
    break;
    }

    return MERR_INSN;
}

nmips_microcode_gen_t::nmips_microcode_gen_t()
{

}

nmips_microcode_gen_t::~nmips_microcode_gen_t()
{

}