#ifndef __NANOMIPS_DIS_H
#define __NANOMIPS_DIS_H

#include <string.h>
#define PACKAGE 1
#define PACKAGE_VERSION 1
#include "dis-asm.h"
#include "nanomips.h"

#ifdef __cplusplus
extern "C" {
#endif


size_t nanomips_disasm_instr(bfd_vma memaddr_base, disassemble_info *info, struct nanomips_opcode *op, struct nanomips_operand** out_operands);
void nanomips_disasm_operands (struct disassemble_info *info,
		 const struct nanomips_opcode *opcode,
		 bfd_uint64_t insn, bfd_vma insn_pc, unsigned int length, struct nanomips_operand** out_operands);

/* Used to track the state carried over from previous operands in
   an instruction.  */

struct nanomips_print_arg_state
{
  /* The value of the last OP_INT seen.  We only use this for OP_MSB,
     where the value is known to be unsigned and small.  */
  unsigned int last_int;

  /* The type and number of the last OP_REG seen.  We only use this for
     OP_REPEAT_DEST_REG and OP_REPEAT_PREV_REG.  */
  enum nanomips_reg_operand_type last_reg_type;
  unsigned int last_regno;
  unsigned int dest_regno;
  unsigned int seen_dest;
};

/* Initialize STATE for the start of an instruction.  */

static inline void
init_print_arg_state (struct nanomips_print_arg_state *state)
{
  memset (state, 0, sizeof (*state));
}

/* Record information about a register operand.  */

static void
nanomips_seen_register (struct nanomips_print_arg_state *state,
			unsigned int regno,
			enum nanomips_reg_operand_type reg_type)
{
  state->last_reg_type = reg_type;
  state->last_regno = regno;

  if (!state->seen_dest)
    {
      state->seen_dest = 1;
      state->dest_regno = regno;
    }
}


/* Check if register+select map to a valid CP0 select sequence.  */

static bfd_boolean
validate_cp0_reg_operand (unsigned int uval)
{
  int i;
  unsigned int regno, selnum;
  regno = uval >> NANOMIPSOP_SH_CP0SEL;
  selnum = uval & NANOMIPSOP_MASK_CP0SEL;

  for (i = 0; nanomips_cp0_3264r6[i].name; i++)
    if (regno == nanomips_cp0_3264r6[i].num
	&& selnum == nanomips_cp0_3264r6[i].sel)
      break;
    else if (regno < nanomips_cp0_3264r6[i].num)
      return FALSE;

  if (nanomips_cp0_3264r6[i].name == NULL)
    return FALSE;

  return TRUE;
}

/* Validate the arguments for INSN, which is described by OPCODE.
   Use DECODE_OPERAND to get the encoding of each operand.  */

static bfd_boolean
validate_insn_args (const struct nanomips_opcode *opcode,
		    const struct nanomips_operand *(*decode_operand) (const char *),
		    unsigned int insn, struct disassemble_info *info)
{
  struct nanomips_print_arg_state state;
  const struct nanomips_operand *operand;
  const char *s;
  unsigned int uval;

  init_print_arg_state (&state);
  for (s = opcode->args; *s; ++s)
    {
      switch (*s)
	{
	case ',':
	case '(':
	case ')':
	  break;

	case '#':
	  ++s;
	  break;

	default:
	  operand = decode_operand (s);

	  if (!operand)
	    continue;

	  uval = nanomips_extract_operand (operand, insn);
	  switch (operand->type)
	    {
	    case OP_REG:
	    case OP_OPTIONAL_REG:
	    case OP_BASE_CHECK_OFFSET:
	      {
		const struct nanomips_reg_operand *reg_op;

		reg_op = (const struct nanomips_reg_operand *) operand;

		if (operand->type == OP_REG
		    && reg_op->reg_type == OP_REG_CP0
		    && !validate_cp0_reg_operand (uval))
		  return FALSE;

		uval = nanomips_decode_reg_operand (reg_op, uval);
		nanomips_seen_register (&state, uval, reg_op->reg_type);
	      }
	      break;

	    case OP_CHECK_PREV:
	      {
		const struct nanomips_check_prev_operand *prev_op
		  = (const struct nanomips_check_prev_operand *) operand;

		if (!prev_op->zero_ok && uval == 0)
		  return FALSE;

		if (((prev_op->less_than_ok && uval < state.last_regno)
		     || (prev_op->greater_than_ok && uval > state.last_regno)
		     || (prev_op->equal_ok && uval == state.last_regno)))
		  break;

		return FALSE;
	      }

	    case OP_MAPPED_CHECK_PREV:
	      {
		const struct nanomips_mapped_check_prev_operand *prev_op =
		  (const struct nanomips_mapped_check_prev_operand *) operand;
		unsigned int last_uval =
		  nanomips_encode_reg_operand (operand, state.last_regno);

		if (((prev_op->less_than_ok && uval < last_uval)
		     || (prev_op->greater_than_ok && uval > last_uval)
		     || (prev_op->equal_ok && uval == last_uval)))
		  break;

		return FALSE;
	      }

	    case OP_NON_ZERO_REG:
	      if (uval == 0)
		return FALSE;
	      break;

	    case OP_NON_ZERO_PCREL_S1:
	      if (uval == 0 && (info->flags & INSN_HAS_RELOC) == 0)
		return FALSE;
	      break;

	    case OP_SAVE_RESTORE_LIST:
	      {
		/* The operand for SAVE/RESTORE is split into 3 pieces
		   rather than just 2 but we only support a 2-way split
		   decode the last bit of the instruction here.  */
		if (opcode->mask >> 16 != 0 && ((insn >> 20) & 0x1) != 0)
		  return FALSE;
	      }
	      break;

	    case OP_IMM_INT:
	    case OP_IMM_WORD:
	    case OP_NEG_INT:
	    case OP_INT:
	    case OP_MAPPED_INT:
	    case OP_MSB:
	    case OP_REG_PAIR:
	    case OP_PCREL:
	    case OP_REPEAT_PREV_REG:
	    case OP_REPEAT_DEST_REG:
	    case OP_SAVE_RESTORE_FP_LIST:
	    case OP_HI20_INT:
	    case OP_HI20_PCREL:
	    case OP_INT_WORD:
	    case OP_UINT_WORD:
	    case OP_PC_WORD:
	    case OP_GPREL_WORD:
	    case OP_DONT_CARE:
	    case OP_HI20_SCALE:
	    case OP_COPY_BITS:
	    case OP_CP0SEL:
	      break;
	    }

	  if (*s == 'm' || *s == '+' || *s == '-' || *s == '`')
	    ++s;
	}
    }
  return TRUE;
}

#ifdef __cplusplus
}
#endif

#endif /* __NANOMIPS_DIS_H */
