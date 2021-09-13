#include "mgen.hpp"
#include "hexrays.hpp"
#include "ins.hpp"
#include "log.hpp"
#include "nmips.hpp"
#include "pro.h"
#include <exception>

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
    case nMIPS_blezc:
    return m_jle;
    case nMIPS_bltic:
    case nMIPS_bltc:
    return m_jl;
    case nMIPS_bltiuc:
    case nMIPS_bltuc:
    return m_jb;
    case nMIPS_beqic:
    case nMIPS_beqc:
    return m_jz;
    case nMIPS_bgeiuc:
    case nMIPS_bgeuc:
    return m_jae;
    case nMIPS_bgeic:
    case nMIPS_bgec:
    case nMIPS_bgezc:
    return m_jge;
    case nMIPS_bneic:
    case nMIPS_bnec:
    return m_jnz;
    }

    return m_jz;
}

merror_t nmips_microcode_gen_t::apply(codegen_t &cdg)
{
    TRACE("[0x%x] apply_micro", cdg.insn.ea);
   /**
    * @note   cdg.emit (the advanced version using pointers) copies the mop_t* arguments, so we can safely pass pointers to local variables.
    */

    switch (cdg.insn.itype) {
    case nMIPS_bc:
    {
        cdg.emit(m_goto, 4, cdg.insn.Op1.addr, 0, 0, 0);
        return MERR_OK;
    }
    break;
    CASE_BRANCH_COND
    {
        auto mop1 = cdg.load_operand(0);
        auto mop2 = cdg.load_operand(1);
        // auto mop3 = cdg.load_operand(2); for some reason, tries to load from jump address
        cdg.emit(code_for_jcnd((nanomips_extra_inst_t)cdg.insn.itype), 4, mop1, mop2, cdg.insn.Op3.addr, 0);
        return MERR_OK;
    }
    break;
    case nMIPS_bgezc:
    case nMIPS_blezc:
    {
        auto mop1 = cdg.load_operand(0);
        mop_t src(mop1, 4);
        mop_t cmp;
        cmp.make_number(0, 4);
        mop_t dst;
        dst.make_gvar(cdg.insn.Op2.addr);

        cdg.emit(code_for_jcnd((nanomips_extra_inst_t)cdg.insn.itype), &src, &cmp, &dst);
        return MERR_OK;
    }
    break;
    case nMIPS_muh:
    {
        auto mop1 = cdg.load_operand(0);
        auto mop2 = cdg.load_operand(1);

        mop_t mmop1(mop1, 4);
        mop_t mmop2(mop2, 4);
        mop_t tmp0 = get_mtemp(0, 8);
        mop_t tmp1 = get_mtemp(1, 8);
        mop_t tmp0_small = get_mtemp(0, 4);

        cdg.emit(m_xdu, &mmop1, NULL, &tmp0);
        cdg.emit(m_xdu, &mmop2, NULL, &tmp1);
        cdg.emit(m_mul, 8, get_temp(0), get_temp(1), get_temp(0), 0);
        cdg.emit(m_high, &tmp0, NULL, &tmp0_small);
        cdg.store_operand(2, tmp0_small);
        
        return MERR_OK;
    }
    break;
    }

    return MERR_INSN;
}

nmips_microcode_gen_t::nmips_microcode_gen_t()
{
    // load temp registers.
    auto all_temps = get_temp_regs();
    auto temp_iter = all_temps.reg.begin();
    size_t temp_size = 16; // TODO: how can we do this without this?

    while (temp_iter != all_temps.reg.end())
    {
        int bit = *temp_iter;
        int width = all_temps.reg.count(bit);
        if (width > temp_size) width = temp_size;
        // we don't want any registers that are also actual proc regs.
        if (mreg2reg(bit, width) == -1)
        {
            qstring tmp_buf;
            get_mreg_name(&tmp_buf, bit, width);
            LOG("saving temp %d.%d (%s)", bit, width, tmp_buf.c_str());
            // TODO: save width?
            temps.add(bit);
        }
        all_temps.reg.inc(temp_iter, width);
    }
}

mreg_t nmips_microcode_gen_t::get_temp(int idx)
{
    return temps.at(idx);
}

mop_t nmips_microcode_gen_t::get_mtemp(int idx, int width)
{
    mreg_t reg = get_temp(idx);
    return mop_t(reg, width);
}

nmips_microcode_gen_t::~nmips_microcode_gen_t()
{

}