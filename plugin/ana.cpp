#include "idp.hpp"
#include "nmips.hpp"
#include "log.hpp"
#include "constants.hpp"
#include "ua.hpp"
#include <ida.hpp>
#include <allins.hpp>
#include <map>
#include <string>

#define MIPS_SAVE_RESTORE_TYPE o_idpspec4

std::map<std::string, uint16> opcode_mapping = {
    {"move", MIPS_move},
    {"movep", MIPS_movep},

    /// Logical ops
    {"and", MIPS_and},
    
    /// Memory ops
    {"lw", MIPS_lw},
    {"lwpc", MIPS_lw},
    {"lbu", MIPS_lbu},
    {"sw", MIPS_sw},
    {"sb", MIPS_sb},
    {"lwxs", MIPS_lwxs},

    /// Math ops
    {"addiu", MIPS_addiu},
    {"addu", MIPS_addu},
    {"subu", MIPS_subu},
    {"sra", MIPS_sra},
    {"sll", MIPS_sll},
    {"div", MIPS_div},

    /// Branching
    {"jalrc", MIPS_jalrc},
    {"bc", nMIPS_bc},
    {"jrc", MIPS_jrc},
    {"beqzc", MIPS_beqzc},
    {"bnezc", MIPS_bnezc},
    {"balc", MIPS_bal},
    // custom branch instructions
    {"bltic", nMIPS_bltic},
    {"bltiuc", nMIPS_bltiuc},
    {"beqic", nMIPS_beqic},
    {"bgeiuc", nMIPS_bgeiuc},
    {"bgeic", nMIPS_bgeic},
    {"bneic", nMIPS_bneic},
    {"brsc", MIPS_jrc},

    /// Specials
    {"li", MIPS_li},
    {"lapc", MIPS_la},
    {"aluipc", MIPS_la},
    {"save", MIPS_save},
    {"restore", MIPS_restore},
    {"restore.jrc", nMIPS_restore_jrc}
};

bool plugin_ctx_t::fill_opcode(insn_t &insn, struct nanomips_opcode& op)
{
#define cmp_op(exp_name) (strcmp(op.name, exp_name) == 0)

    std::string op_name(op.name);

    auto it = opcode_mapping.find(op_name);
    if (it != opcode_mapping.end()) {
        insn.itype = it->second;
    } else {
        // LOG("[0x%x] opcode %s not implemented!", insn.ea, op.name);
        insn.itype = nMIPS_todo;
        return false;
    }
    
    return true;
}

size_t remap_register(size_t reg)
{
    // t4 / t5
    if (reg == 2 || reg == 3) return reg + 10;

    // t0 - t3
    if (reg >= 12 && reg <= 15) return reg - 4;

    return reg;
}

int plugin_ctx_t::fill_operand(insn_t &insn, struct nanomips_opcode& opcode, nanomips_decoded_op &op, int idx)
{
    struct nanomips_operand* operand = op.op;
    unsigned int uval = op.val;
    bfd_vma base_pc = op.base_pc;
    op_t* res = &insn.ops[idx];
    res->dtype = dt_dword;
    bool did_something = true;

    // only used for save restore list.
    bool mode16 = opcode.mask >> 16 == 0;
    unsigned int bitmask = 0;
    #define set_reg(num) bitmask = (bitmask | (1 << (num)))
    unsigned int freg, fp, gp, ra;
    int count;
    
    int next_idx = idx+1;

    switch (op.op->type)
    {
        case OP_INT:
        case OP_IMM_INT:
        {
            const struct nanomips_int_operand *int_op;

            int_op = (const struct nanomips_int_operand *) operand;
            uval = nanomips_decode_int_operand (int_op, uval);
            res->type = o_imm;
            res->value = uval;
        }
        break;

        case OP_MAPPED_INT:
        {
            const struct nanomips_mapped_int_operand *mint_op;

            mint_op = (const struct nanomips_mapped_int_operand *) operand;
            uval = mint_op->int_map[uval];
            res->type = o_imm;
            res->value = uval;
        }
        break;

        case OP_REG:
        case OP_OPTIONAL_REG:
        case OP_MAPPED_CHECK_PREV:
        case OP_BASE_CHECK_OFFSET:
        {
            const struct nanomips_reg_operand *reg_op;

            reg_op = (const struct nanomips_reg_operand *) operand;
            uval = nanomips_decode_reg_operand (reg_op, uval);
            uval = remap_register(uval);
            res->type = o_reg;
            res->reg = uval; // TODO change mapping here!
        }
        break;

        case OP_REG_PAIR:
        {
            const struct nanomips_reg_pair_operand *pair_op;

            pair_op = (const struct nanomips_reg_pair_operand *) operand;
            int reg1 = pair_op->reg1_map[uval];
            int reg2 = pair_op->reg2_map[uval];
            res->type = o_reg;
            res->reg = remap_register(reg1);
            insn.ops[next_idx].type = o_reg;
            insn.ops[next_idx].dtype = dt_dword;
            insn.ops[next_idx].reg = remap_register(reg2);
            next_idx++;
        }
        break;

        case OP_PCREL:
        {
            const struct nanomips_pcrel_operand *pcrel_op;

            pcrel_op = (const struct nanomips_pcrel_operand *) operand;
            bfd_vma address = nanomips_decode_pcrel_operand (pcrel_op, base_pc, uval);

            res->type = o_mem;
            res->addr = address;
        }
        break;

        case OP_CHECK_PREV:
        case OP_NON_ZERO_REG:
        {
            res->type = o_reg;
            res->reg = remap_register(uval & 31);
        }
        break;

        case OP_NEG_INT:
            res->type = o_imm;
            res->value = -uval;
        break;

        case OP_REPEAT_PREV_REG:
            res->type = o_reg;
            res->reg = ana_state.last_reg;
        break;

        case OP_REPEAT_DEST_REG:
            res->type = o_reg;
            res->reg = ana_state.dest_reg;
        break;

        case OP_PC_WORD:
            res->type = o_mem;
	        res->addr = base_pc + (((uval >> 16) & 0xffff) | (uval << 16));
        break;

        case OP_SAVE_RESTORE_LIST:
            res->type = MIPS_SAVE_RESTORE_TYPE;
            // specval is a bitmap of the registers to save.
            
            fp = gp = ra = 0;

            if (mode16)
            {
                freg = 30 | (uval >> 4);
                count = uval & 0xf;
            }
            else
            {
                freg = (uval >> 6) & 0x1f;
                count = (uval >> 1) & 0xf;
                if (count > 0)
                    gp = uval & 1;
            }

            if (freg == 30 && count > 0)
                fp = 1;
            if ((freg == 31 && count > 0) || (freg == 30 && count > 1))
                ra = 1;

            if (freg + count == 45)
                gp = 1;

            count = count - gp;
            if (fp && count > 0)
            {
                freg = (freg & 0x10) | ((freg + 1) % 32);
                count--;
            }

            if (ra && count > 0)
            {
                freg = (freg & 0x10) | ((freg + 1) % 32);
                count--;
            }

            if (fp)
            {
                set_reg(30);
            }

            if (ra)
            {
                set_reg(31);
            }

            if (gp)
                set_reg(28);

            if (count > 0)
            {
                for (int i = 0; i < count; i++)
                {
                    set_reg(freg + i);
                }
            }
            res->specval = bitmask;
        break;

        // TODO: this decodes wrong, not sure why??
        case OP_HI20_PCREL:
        {
            res->type = o_mem;
            
            if (false)
            {
                uval = nanomips_decode_hi20_int_operand (operand, uval);
                res->addr = uval;
            }
            else
            {
                res->addr = nanomips_decode_hi20_pcrel_operand (operand, base_pc, uval);
            }
        }
        break;

        default:
            LOG("[0x%x] Operand %d not yet implemented!", insn.ea, op.op->type);
            did_something = false;
            next_idx = idx;
        break;
    }

    if (did_something)
    {
        switch (res->type) {
            case o_reg:
                ana_state.record_register(res->reg);
            break;
            case o_imm:
                ana_state.record_int(res->value);
            break;
        }
    }

    return next_idx;

}

//--------------------------------------------------------------------------
// Analyze an instruction and fill the 'insn' structure
size_t plugin_ctx_t::ana(insn_t &insn)
{
    ensure_mgen_installed();
    // if (ana_state.last_instr_48bits)
    // {
    //     // emit a single nop to cover those bytes!
    //     insn.itype = MIPS_nop;
    //     ana_state = {};
    //     return 2;
    // }
    // reset.
    ana_state = {};

    // check if this should be a fake jrc
    auto it = fake_jrc_insn.find(insn.ea);
    if (it != fake_jrc_insn.end())
    {
        insn.itype = MIPS_jrc;
        insn.Op1.type = o_reg;
        insn.Op1.dtype = dt_dword;
        insn.Op1.reg = str2reg("ra");
        insn.size = it->second;
        return insn.size;
    }

    struct nanomips_opcode op = {};
    nanomips_decoded_op operands[MAX_NUM_OPS] = {};
    size_t insn_size = nanomips_disasm_instr(insn.ea, &disasm_info, &op, operands);
    // LOG("Decoded instruction of size: %d", insn_size);
    if (insn_size <= 0) return insn_size;

    // LOG("[0x%x] Decoded %s (%d)", insn.ea, op.name, insn_size);

    // if (strcmp(op.name, "lwpc") == 0)
    // {
    //     LOG("[LWPC] operand 1: %d", operands[1].op->type);
    // }

    bool remapped = fill_opcode(insn, op);
    if (!remapped) return insn_size;

    int op_idx = 0;

    for (int i = 0; i < MAX_NUM_OPS; i++) {
        nanomips_decoded_op curr_op = operands[i];
        if (curr_op.op == NULL) break;

        // don't care operand, we should skip!
        if (curr_op.op->type == OP_DONT_CARE) {
            continue;
        }

        op_idx = fill_operand(insn, op, curr_op, op_idx);
    }

    // so that post process can modify this.
    insn.size = insn_size;

    post_process(insn);

    // if (insn_size == 6) 
    // {
    //     ana_state.last_instr_48bits = true;
    //     insn.ops[0].type = o_void;
    //     insn.ops[1].type = o_void;
    //     insn.ops[2].type = o_void;
    //     insn_size = 4;
    // }


    return insn.size;
}

void plugin_ctx_t::post_process(insn_t &insn)
{
    switch (insn.itype) {
        case MIPS_lw:
        case MIPS_lb:
        case MIPS_lbu:
        case MIPS_sw:
        case MIPS_sb:
        // switch to o_disp operand
        if (insn.ops[2].type == o_reg) {
            insn.ops[2].type = o_void;
            insn.ops[1].type = o_displ;
            insn.ops[1].addr = insn.ops[1].value;
            insn.ops[1].phrase = insn.ops[2].reg;
        }
        if (insn.itype == MIPS_lb || insn.itype == MIPS_sb)
        {
            insn.ops[1].dtype = dt_byte;
        }
        break;

        case MIPS_j:
        case MIPS_bal:
        case nMIPS_bc:
            insn.ops[0].type = o_near;
        break;

        // IDA only knows 32bit version, aka jalrc dst, src.
        // so fake that here!
        case MIPS_jalrc:
        if (insn.Op2.type == o_void)
        {
            insn.Op2 = insn.Op1;
            insn.Op1.type = o_reg;
            insn.Op1.reg = str2reg("ra");
        }
        break;

        case MIPS_beqz:
        case MIPS_beqzc:
        case MIPS_bnez:
        case MIPS_bnezc:
            insn.ops[1].type = o_near;
        break;

        case nMIPS_beqic:
        case nMIPS_bgeiuc:
        case nMIPS_bgeic:
        case nMIPS_bltic:
        case nMIPS_bltiuc:
        case nMIPS_bneic:
            insn.ops[2].type = o_near;
        break;

        // convert into a restore, followed by a jrc instruction.
        // restore also needs to have an empty first op.
        case nMIPS_restore_jrc:
            insn.itype = MIPS_restore;
            fake_jrc_insn[insn.ea+1] = insn.size - 1;
            insn.size = 1;
        case MIPS_save:
        case MIPS_restore:
            insn.Op3 = insn.Op2;
            insn.Op2 = insn.Op1;
            insn.Op1.type = MIPS_SAVE_RESTORE_TYPE;
            insn.Op1.specval = 0;
            insn.Op1.dtype = 0;
            insn.flags = INSN_64BIT;
            insn.Op1.clr_shown(); // Hide
        break;
    }
}

void insn_analysis_state_t::record_register(unsigned int reg)
{
    this->last_reg = reg;
    if (!this->seen_dest)
    {
        this->seen_dest = true;
        this->dest_reg = reg;
    }
}

void insn_analysis_state_t::record_int(bfd_vma val)
{
    this->last_int = val;
}