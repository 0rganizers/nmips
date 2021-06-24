#include "ida.hpp"
#include "idp.hpp"
#include "ins.hpp"
#include "log.hpp"
#include "nmips.hpp"
#include "ua.hpp"
#include "xref.hpp"
#include "constants.hpp"
#include "jumptable.hpp"
#include <nalt.hpp>
#include <pro.h>
#include <allins.hpp>

void decode_phrase(op_t op, int& base, int& scale)
{
    scale = op.specval & 0b11111;
    base = (op.specval >> 5) & 0b11111;
}

//-------------------------------------------------------------------------
// 3 bgeiuc rA, #size, default
// 2 la      rB, rJumps
// 1 lwxs    rB, rA(rB)
// 0 brsc (jrc) rB // actually jrc, since we map brsc to jrc!

static const char nmips_depends[][4] =
{
    { 1 },  // 0 depends on lwxs
    { 2 },            // 1 depends on la
    { 3 },            // 2 depends on nothing
    { 0 },            // 3 depends on nothing
};

struct nmips_jump_pattern_t : public jump_pattern_t
{
protected:
    enum { rA, rB };
    enum
    {
        BODY_NJPI     = 2,  // ldb.x rA, [rJumps,rA']
        ELBASE_NJPI   = 1,  // add1    rA, rElbase, rA'
    };
    // arc_t &pm;
    ea_t jumptable_ea;
    int  num_elems;
    ea_t elbase; //lint !e958 padding is required
    ea_t  default_ea;

public:
  nmips_jump_pattern_t(switch_info_t *_si)
    : jump_pattern_t(_si, nmips_depends, rB),
    //   pm(*(arc_t *)_pm),
      jumptable_ea(BADADDR),
      num_elems(-1),
      elbase(BADADDR),
      default_ea(BADADDR)
  {
    // modifying_r32_spoils_r64 = false;
    // si->flags |= SWI_HXNOLOWCASE;
    si->flags |= SWI_ELBASE | SWI_J32;
    non_spoiled_reg = -1; // no register is never spoiled!
  }

  virtual void process_delay_slot(ea_t &ea, bool branch) const override;
  virtual bool equal_ops(const op_t &x, const op_t &y) const override;
  virtual bool handle_mov(tracked_regs_t &_regs) override;
//   virtual void check_spoiled(tracked_regs_t *_regs) const override;

//   bool jpi4() override;  // sub rA, rA', #minv
  bool jpi3() override;  // cmp followed by the conditional jump or 'brhi/lo'
  bool jpi2() override;  // ldb.x rA, [rJumps,rA']
  bool jpi1() override;  // add1 rA, rElbase, rA'
  bool jpi0() override;  // j [rA] | bi [rA] | bih [rA]

  //lint -esym(1762, arc_jump_pattern_t::finish) member function could be made const
  bool finish();

protected:
//   static inline bool optype_supported(const op_t &x);

  // helpers
  // brhs rA, #size, default | brlo rA, #size, body
//   bool jpi_cmp_jump(const op_t **op_var);
//   // bhi default | bls body with optional 'b default'
//   bool jpi_condjump();
//   // cmp rA, #size
//   bool jpi_cmp_ncases(const op_t **op_var);

  // prepare and track rC
//   bool analyze_cond(cond_t cond, ea_t jump);
};

void nmips_jump_pattern_t::process_delay_slot(ea_t &ea, bool branch) const
{
    // LOG("process_delay_slot");
}

bool nmips_jump_pattern_t::equal_ops(const op_t &x, const op_t &y) const
{
  if ( x.type != y.type )
    return false;
  // ignore difference in the data size of registers
  switch ( x.type )
  {
    case o_void:
      // consider spoiled values as not equal
      return false;
    case o_reg:
      return x.reg == y.reg;
    case o_displ:
      return x.phrase == y.phrase && x.addr == y.addr;
    case o_condjump:
      // we do not track the condition flags
      return true;
  }
  return false;
}

bool nmips_jump_pattern_t::handle_mov(tracked_regs_t &_regs)
{
    return false;
//   const op_t *src = &insn.Op2;
//   const op_t *dst = &insn.Op1;
//   switch ( insn.itype )
//   {
//     case MIPS_la:
//     case ARC_lsl:
//     case ARC_lsr:
//     case ARC_sub:
//     case ARC_xor:
//     case ARC_or:
//       if ( insn.Op3.type != o_imm || insn.Op3.value != 0 )
//         return false;
//       // no break
//     case ARC_ld:
//     case ARC_mov:
//       break;
//     case ARC_st:
//       std::swap(src, dst);
//       break;
//     default:
//       return false;
//   }
//   if ( !optype_supported(*src) || optype_supported(*dst) )
//     return false;
//   return set_moved(*dst, *src, _regs);
}

bool nmips_jump_pattern_t::jpi3()
{
    if (insn.itype != nMIPS_bgeiuc) return false;

    num_elems = insn.Op2.value;
    si->set_jtable_size(num_elems);
    default_ea = insn.Op3.addr;
    si->defjump = default_ea;

    trackop(insn.Op1, rA);

    return true;
}

bool nmips_jump_pattern_t::jpi2()
{
    if (insn.itype != MIPS_la) return false;

    jumptable_ea = insn.Op2.addr;
    si->jumps = jumptable_ea;

    trackop(insn.Op1, rB);

    return true;
}

bool nmips_jump_pattern_t::jpi1()
{
    if (insn.itype != MIPS_lwxs) return false;

    int base = 0, scale = 0;
    decode_phrase(insn.Op2, base, scale);

    track(scale, rA, dt_dword);
    track(base, rB, dt_dword);
    si->regnum = scale;
    si->regdtype = dt_dword;

    return true;
}

bool nmips_jump_pattern_t::jpi0()
{
    if (insn.itype != MIPS_jrc) return false;

    elbase = insn.ea + insn.size;
    si->elbase = elbase;
    si->set_shift(1);
    trackop(insn.Op1, rB);
    return true;
}

bool nmips_jump_pattern_t::finish()
{
    // TODO!
    return true;
}

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
        if (insn.itype == MIPS_bal || insn.itype == MIPS_jal)
            add_cref(insn.ea, op.addr, fl_CN);
        else
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
    // we need to handle li ourselves, since IDA really seems to want to create crefs based on the li value :/
    // this makes decompiled output look really weird.
    if (insn.itype < nMIPS_todo && insn.itype != MIPS_li)
    {
        return 0;
    }

    // if (insn.itype == MIPS_bal)
    // {
    //     LOG("[0x%x] emu bal, Op1.type = %d", insn.ea, insn.Op1.type);
    // }

    uint32 feature = get_feature((nanomips_extra_inst_t)insn.itype);
    bool flow = true;
    // unconditional jump, no flow!
    if ((feature & CF_JUMP) == CF_JUMP && (feature & CCF_COND) == 0)
    {
        flow = false;
    }
    if (flow)
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

//----------------------------------------------------------------------
static int is_jump_pattern(switch_info_t *si, const insn_t &insn, procmod_t *pm)
{
  nmips_jump_pattern_t jp(si);
  if ( !jp.match(insn) || !jp.finish() )
    return JT_NONE;
  return JT_SWITCH;
}

bool plugin_ctx_t::is_switch(switch_info_t *si, const insn_t *insn)
{
    struct nanomips_opcode op = {};
    nanomips_decoded_op operands[MAX_NUM_OPS] = {};
    size_t insn_size = nanomips_disasm_instr(insn->ea, &disasm_info, &op, operands);
    if (insn_size <= 0) return false;

    if (strcmp(op.name, "brsc") != 0) return false;

    static is_pattern_t *const patterns[] =
    {
        is_jump_pattern,
    };
    bool res = check_for_table_jump(si, *insn, patterns, qnumber(patterns));

    LOG("[0x%x] is_switch = %s", insn->ea, res ? "true" : "false");
    return res;
}

int plugin_ctx_t::may_be_func(insn_t *insn, int state)
{
    // save instructions definitely mark the beginning of a function, regardless of state!
    if (insn->itype == MIPS_save)
    {
        return 100;
    }

    if (state == 0 || state == 1) // creating functions
    {
        ea_t cref_addr;
        for ( cref_addr = get_first_cref_to(insn->ea);
              cref_addr != BADADDR;
              cref_addr = get_next_cref_to(insn->ea, cref_addr) )
        {
            insn_t cref_insn;
            if (decode_insn(&cref_insn, cref_addr) > 0)
            {
                // if we have a cref that bal's to here, then it must also be a function!
                if (cref_insn.itype == MIPS_bal || cref_insn.itype == MIPS_jal)
                {
                    return 100;
                }
            }
        }
    }

    return 0;
}