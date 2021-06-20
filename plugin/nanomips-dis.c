#include "nanomips-dis.h"

size_t nanomips_disasm_instr(bfd_vma memaddr_base, disassemble_info *info, struct nanomips_opcode *out_op, nanomips_decoded_op* out_operands)
{
    const struct nanomips_opcode *op, *opend;
    void *is = info->stream;
    bfd_byte buffer[2];
    bfd_uint64_t higher = 0;
    unsigned int length;
    int status;
    bfd_uint64_t insn;
    const struct nanomips_arch_choice *chosen_arch;

    int nanomips_processor;
    int nanomips_ase;
    int nanomips_isa;

    nanomips_isa = ISA_NANOMIPS32R6;
    nanomips_processor = CPU_NANOMIPS32R6;
    nanomips_ase = ASE_xNMS | ASE_TLB; // Standard stuff;

    bfd_vma memaddr = memaddr_base;

    info->bytes_per_chunk = 2;
    info->display_endian = info->endian;
    info->insn_info_valid = 1;
    info->branch_delay_insns = 0;
    info->data_size = 0;
    info->insn_type = dis_nonbranch;
    info->target = 0;
    info->target2 = 0;

    status = (*info->read_memory_func) (memaddr, buffer, 2, info);
    if (status != 0)
    {
        (*info->memory_error_func) (status, memaddr, info);
        return -1;
    }

    length = 2;

    if (info->endian == BFD_ENDIAN_BIG)
        insn = bfd_getb16 (buffer);
    else
        insn = bfd_getl16 (buffer);

    if ((insn & 0xfc00) == 0x6000)
    {
        unsigned imm;
        /* This is a 48-bit nanoMIPS instruction. */
        status = (*info->read_memory_func) (memaddr + 2, buffer, 2, info);
        if (status != 0)
        {
            (*info->memory_error_func) (status, memaddr + 2, info);
            return -1;
        }
        if (info->endian == BFD_ENDIAN_BIG)
            imm = bfd_getb16 (buffer);
        else
            imm = bfd_getl16 (buffer);
        higher = (imm << 16);

        status = (*info->read_memory_func) (memaddr + 4, buffer, 2, info);
        if (status != 0)
        {
            (*info->memory_error_func) (status, memaddr + 4, info);
            return -1;
        }

        if (info->endian == BFD_ENDIAN_BIG)
            imm = bfd_getb16 (buffer);
        else
            imm = bfd_getl16 (buffer);
        higher = higher | imm;

        length += 4;
    }
    else if ((insn & 0x1000) == 0x0)
    {
        /* This is a 32-bit nanoMIPS instruction.  */
        higher = insn;

        status = (*info->read_memory_func) (memaddr + 2, buffer, 2, info);
        if (status != 0)
        {
            (*info->memory_error_func) (status, memaddr + 2, info);
            return -1;
        }

        if (info->endian == BFD_ENDIAN_BIG)
            insn = bfd_getb16 (buffer);
        else
            insn = bfd_getl16 (buffer);

        insn = insn | (higher << 16);

        length += 2;
    }


    const struct nanomips_opcode *opcodes;
    int num_opcodes;
    struct nanomips_operand const *(*decode) (const char *);

    opcodes = nanomips_opcodes;
    num_opcodes = bfd_nanomips_num_opcodes;
    decode = decode_nanomips_operand;

    opend = opcodes + num_opcodes;
    for (op = opcodes; op < opend; op++)
    {
        if (op->pinfo != INSN_MACRO
        && (insn & op->mask) == op->match
        && ((length == 2 && (op->mask & 0xffff0000) == 0)
            || (length == 6
            && (op->mask & 0xffff0000) == 0)
            || (length == 4 && (op->mask & 0xffff0000) != 0)))
        {
        if (!nanomips_opcode_is_member (op, nanomips_isa, nanomips_ase,
                        nanomips_processor)
            || (op->pinfo2 & INSN2_CONVERTED_TO_COMPACT))
            continue;

        if (!validate_insn_args (op, decode, insn, info))
            continue;

        if (length == 6)
            insn |= (higher << 32);

        if (op->args[0])
            nanomips_disasm_operands(info, op, insn, memaddr, length, out_operands);

        // if (op->args[0])
        //     print_insn_args (info, op, decode, insn, memaddr, length);

        /* Figure out instruction type and branch delay information.  */
        if ((op->pinfo2 & INSN2_UNCOND_BRANCH) != 0)
            {
            if ((op->pinfo & (INSN_WRITE_GPR_31 | INSN_WRITE_1)) != 0)
            info->insn_type = dis_jsr;
            else
            info->insn_type = dis_branch;
            }
        else if ((op->pinfo2 & INSN2_COND_BRANCH) != 0)
            {
            if ((op->pinfo & INSN_WRITE_GPR_31) != 0)
            info->insn_type = dis_condjsr;
            else
            info->insn_type = dis_condbranch;
            }
        else if ((op->pinfo & (INSN_STORE_MEMORY | INSN_LOAD_MEMORY)) != 0)
            info->insn_type = dis_dref;

        memcpy(out_op, op, sizeof(*op));

        return length;
        }
    }

  info->insn_type = dis_noninsn;
  return 0;
}

void nanomips_disasm_operands (struct disassemble_info *info,
		 const struct nanomips_opcode *opcode,
		 bfd_uint64_t insn, bfd_vma insn_pc, unsigned int length, nanomips_decoded_op* out_operands)
{
  const fprintf_ftype infprintf = info->fprintf_func;
  void *is = info->stream;
  struct nanomips_print_arg_state state;
  const struct nanomips_operand *operand;
  const char *s;
  bfd_boolean pending_sep = FALSE;
  bfd_boolean pending_space = TRUE;

  init_print_arg_state (&state);
  for (s = opcode->args; *s; ++s)
    {
      switch (*s)
	{
	case ',':
	  pending_sep = TRUE;
	  break;
	case '(':
	  if (pending_sep)
	    {
	      pending_sep = FALSE;
	    }
	  /* fall-through */
	case ')':
	  break;

	case '#':
	  ++s;
	  break;

	default:
	  operand = decode_nanomips_operand (s);
	  if (!operand)
	    {
	      /* xgettext:c-format */
	      infprintf (is,
			 ("# internal error, undefined operand in `%s %s'"),
			 opcode->name, opcode->args);
	      return;
	    }

	  /* Defer printing the comma separator for CP0-select values since
	     the preceding register output is also defered.  */
	  if (operand->type != OP_DONT_CARE
	      && operand->type != OP_CP0SEL
	      && pending_sep)
	    {
	      pending_sep = FALSE;
	    }

	//   if (operand->type != OP_DONT_CARE && pending_space)
	//     infprintf (is, "\t");
	  pending_space = FALSE;

	  {
	    bfd_vma base_pc = 0;
	    bfd_boolean have_reloc = ((info->flags & INSN_HAS_RELOC) != 0);

	    if (!have_reloc)
	      base_pc = insn_pc;

	    if ((operand->type == OP_PCREL
		 || operand->type == OP_HI20_PCREL
		 || operand->type == OP_NON_ZERO_PCREL_S1
		 || operand->type == OP_PC_WORD)
		&& !have_reloc)
	      base_pc += length;

        out_operands->base_pc = base_pc;
        out_operands->op = operand;

        // TODO: extract operand here??
	    if (operand->type == OP_INT_WORD
		|| operand->type == OP_UINT_WORD
		|| operand->type == OP_PC_WORD
		|| operand->type == OP_GPREL_WORD
		|| operand->type == OP_IMM_WORD)
            out_operands->val = insn >> 32;
	    //   print_insn_arg (info, &state, opcode, operand, base_pc,
		// 	      insn >> 32);
	    else if (operand->type != OP_DONT_CARE)
            out_operands->val = nanomips_extract_operand(operand, insn);
	    //   print_insn_arg (info, &state, opcode, operand, base_pc,
		// 	      nanomips_extract_operand (operand, insn));

        // *out_operands = operand;
        // memcpy(out_operands, operand, sizeof(*operand));

        out_operands++;
	  }
	  if (*s == 'm' || *s == '+' || *s == '-' || *s == '`')
	    ++s;
	  break;
	}
    }
}