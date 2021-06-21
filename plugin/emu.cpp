#include "ida.hpp"
#include "idp.hpp"
#include "nmips.hpp"
#include "ua.hpp"
#include "xref.hpp"
#include "constants.hpp"

void plugin_ctx_t::handle_operand(insn_t &insn, op_t &op)
{
    switch (op.type) {
        case o_void:
        case o_reg:
        // nothing special here!
        break;

        case o_mem:
        add_dref(insn.ea, op.addr, dr_R);
        break;

        case o_near:
        add_cref(insn.ea, op.addr, fl_JN);
        break;
    }
}

//----------------------------------------------------------------------
// Emulate an instruction
// This function should:
//      - create all xrefs from the instruction
//      - perform any additional analysis of the instruction/program
//        and convert the instruction operands, create comments, etc.
//      - create stack variables
//      - analyze the delayed branches and similar constructs

int plugin_ctx_t::emu(insn_t &insn)
{
    // Not custom instruction!
    if (insn.itype < nMIPS_todo)
    {
        return 0;
    }

    uint32 feature = get_feature((nanomips_extra_inst_t)insn.itype);
    uint32 cond_jmp = CF_JUMP | CCF_COND;
    if ((feature & (cond_jmp)) == cond_jmp)
    {
        //flow into next instruction.
        ea_t next_instr = insn.ea + insn.size;
        add_cref(insn.ea, next_instr, fl_F);
    }

    // handle operands
    for (int i = 0; i < UA_MAXOP; i++)
    {
        if (insn.ops[i].type != o_void)
        {
            handle_operand(insn, insn.ops[i]);
        }
    }
    return 1;
}