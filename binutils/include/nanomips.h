/* nanomips.h.  nanoMIPS opcode list for GDB, the GNU debugger.
   Copyright (C) 2018 Free Software Foundation, Inc.
   Contributed by MIPS Tech LLC.
   Written by Faraz Shahbazker <faraz.shahbazker@mips.com>

   This file is part of GDB, GAS, and the GNU binutils.

   GDB, GAS, and the GNU binutils are free software; you can redistribute
   them and/or modify them under the terms of the GNU General Public
   License as published by the Free Software Foundation; either version 3,
   or (at your option) any later version.

   GDB, GAS, and the GNU binutils are distributed in the hope that they
   will be useful, but WITHOUT ANY WARRANTY; without even the implied
   warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
   the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this file; see the file COPYING3.  If not, write to the Free
   Software Foundation, 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#ifndef __NANOMIPS_H
#define __NANOMIPS_H

#include "bfd.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Enumerates the various types of nanoMIPS operand.  */
enum nanomips_operand_type {
  /* Described by nanomips_int_operand.  */
  OP_INT,

  /* Described by nanomips_mapped_int_operand.  */
  OP_MAPPED_INT,

  /* Described by nanomips_msb_operand.  */
  OP_MSB,

  /* Described by nanomips_reg_operand.  */
  OP_REG,

  /* Like OP_REG, but can be omitted if the register is the same as the
     previous operand.  */
  OP_OPTIONAL_REG,

  /* Described by nanomips_reg_pair_operand.  */
  OP_REG_PAIR,

  /* Described by nanomips_pcrel_operand.  */
  OP_PCREL,

  /* The register list and frame size for a MIPS16 SAVE or RESTORE
     instruction.  */
  OP_SAVE_RESTORE_LIST,

  /* A register operand that must match the destination register.  */
  OP_REPEAT_DEST_REG,

  /* A register operand that must match the previous register.  */
  OP_REPEAT_PREV_REG,

  /* Described by nanomips_prev_operand.  */
  OP_CHECK_PREV,

  /* A register operand that must not be zero.  */
  OP_NON_ZERO_REG,

 /* The floating-point register list for a nanoMIPS SAVE or RESTORE
     instruction. */
  OP_SAVE_RESTORE_FP_LIST,

  /* Fractured upper immediate PC-offset for nanoMIPS */
  OP_HI20_PCREL,

  /* Fractured upper immediate 20-bit signed integer for nanoMIPS */
  OP_HI20_INT,

  /* Fractured upper immediate 20-bit scaled integer for nanoMIPS */
  OP_HI20_SCALE,

  /* A non-zero PC-relative offset.  */
  OP_NON_ZERO_PCREL_S1,

  /* To check a mapped register against a previous operand.  */
  OP_MAPPED_CHECK_PREV,

  /* Unsigned word operand.  */
  OP_UINT_WORD,

  /* Signed word operand.  */
  OP_INT_WORD,

  /* Immediate PC-relative word operand.  */
  OP_PC_WORD,

  /* Immediate GP-relative word operand.  */
  OP_GPREL_WORD,

  /* Don't care bits.  */
  OP_DONT_CARE,

  /* Immediate unsigned word operand, to be negated.  */
  OP_NEG_INT,

  /* Immediate (non-relocatable) integer operand.  */
  OP_IMM_INT,

  /* Immediate (non-relocatable) word operand.  */
  OP_IMM_WORD,

  /* Base register for limited types of offsets.  */
  OP_BASE_CHECK_OFFSET,

  /* Copy over bits from another part of instruction.  */
  OP_COPY_BITS,

  /* Select bits for a COP0 register.  */
  OP_CP0SEL,
};

/* Enumerates the types of nanoMIPS register.  */
enum nanomips_reg_operand_type {
  /* General registers $0-$31.  Software names like $at can also be used.  */
  OP_REG_GP,

  /* Floating-point registers $f0-$f31.  */
  OP_REG_FP,

  /* DSP accumulator registers $ac0-$ac3.  */
  OP_REG_ACC,

  /* Coprocessor registers in numeric format, $0-$31.  */
  OP_REG_COPRO,

  /* Hardware registers $0-$31.  Mnemonic names like hwr_cpunum can
     also be used in some contexts.  */
  OP_REG_HW,

  /* MSA registers $w0-$w31.  */
  OP_REG_MSA,

  /* MSA control registers $0-$31.  */
  OP_REG_MSA_CTRL,

  /* Co-processor 0 named registers.  */
  OP_REG_CP0,

  /* Co-processor 0 named registers with select.  */
  OP_REG_CP0SEL,

  /* Co-processor 0 named registers with select.  */
  OP_REG_HWRSEL
};

/* Base class for all operands.  */
struct nanomips_operand
{
  /* The type of the operand.  */
  enum nanomips_operand_type type;

  /* The operand occupies SIZE bits of the instruction, starting at LSB.  */
  unsigned short size;
  unsigned short lsb;

  /* These are used to split a value across two different
     parts of the instruction encoding.  */
  unsigned int size_top;
  unsigned int lsb_top;
};

/* Describes an integer operand with a regular encoding pattern.  */
struct nanomips_int_operand
{
  struct nanomips_operand root;

  /* The low ROOT.SIZE bits of MAX_VAL encodes (MAX_VAL + BIAS) << SHIFT.
     The cyclically previous field value encodes 1 << SHIFT less than that,
     and so on.  E.g.

     - for { { T, 4, L }, 14, 0, 0 }, field values 0...14 encode themselves,
       but 15 encodes -1.

     - { { T, 8, L }, 127, 0, 2 } is a normal signed 8-bit operand that is
       shifted left two places.

     - { { T, 3, L }, 8, 0, 0 } is a normal unsigned 3-bit operand except
       that 0 encodes 8.

     - { { ... }, 0, 1, 3 } means that N encodes (N + 1) << 3.  */
  unsigned int max_val;
  int bias;
  unsigned int shift;

  /* True if the operand should be printed as hex rather than decimal.  */
  bfd_boolean print_hex;
};

/* Uses a lookup table to describe a small integer operand.  */
struct nanomips_mapped_int_operand
{
  struct nanomips_operand root;

  /* Maps each encoding value to the integer that it represents.  */
  const int *int_map;

  /* True if the operand should be printed as hex rather than decimal.  */
  bfd_boolean print_hex;
};

/* An operand that encodes the most significant bit position of a bitfield.
   Given a bitfield that spans bits [MSB, LSB], some operands of this type
   encode MSB directly while others encode MSB - LSB.  Each operand of this
   type is preceded by an integer operand that specifies LSB.

   The assembly form varies between instructions.  For some instructions,
   such as EXT, the operand is written as the bitfield size.  For others,
   such as EXTS, it is written in raw MSB - LSB form.  */
struct nanomips_msb_operand
{
  struct nanomips_operand root;

  /* The assembly-level operand encoded by a field value of 0.  */
  int bias;

  /* True if the operand encodes MSB directly, false if it encodes
     MSB - LSB.  */
  bfd_boolean add_lsb;

  /* The maximum value of MSB + 1.  */
  unsigned int opsize;
};

/* Describes a single register operand.  */
struct nanomips_reg_operand
{
  struct nanomips_operand root;

  /* The type of register.  */
  enum nanomips_reg_operand_type reg_type;

  /* If nonnull, REG_MAP[N] gives the register associated with encoding N,
     otherwise the encoding is the same as the register number.  */
  const unsigned char *reg_map;
};

/* Describes an operand that which must match a condition based on the
   previous operand.  */
struct nanomips_check_prev_operand
{
  struct nanomips_operand root;

  bfd_boolean greater_than_ok;
  bfd_boolean less_than_ok;
  bfd_boolean equal_ok;
  bfd_boolean zero_ok;
};

/* Describes an operand that encodes a pair of registers.  */
struct nanomips_reg_pair_operand
{
  struct nanomips_operand root;

  /* The type of register.  */
  enum nanomips_reg_operand_type reg_type;

  /* Encoding N represents REG1_MAP[N], REG2_MAP[N].  */
  unsigned char *reg1_map;
  unsigned char *reg2_map;
};

/* Describes an operand that is calculated relative to a base PC.
   The base PC is usually the address of the following instruction,
   but the rules for MIPS16 instructions like ADDIUPC are more complicated.  */
struct nanomips_pcrel_operand
{
  /* Encodes the offset.  */
  struct nanomips_int_operand root;

  /* The low ALIGN_LOG2 bits of the base PC are cleared to give PC',
     which is then added to the offset encoded by ROOT.  */
  unsigned int align_log2 : 8;

  /* If INCLUDE_ISA_BIT, the ISA bit of the original base PC is then
     reinstated.  This is true for jumps and branches and false for
     PC-relative data instructions.  */
  unsigned int include_isa_bit : 1;

  /* If FLIP_ISA_BIT, the ISA bit of the result is inverted.
     This is true for JALX and false otherwise.  */
  unsigned int flip_isa_bit : 1;
};

/* This structure holds information for a particular instruction.  */

struct nanomips_opcode
{
  /* The name of the instruction.  */
  const char *name;
  /* An optional suffix.  */
  const char *suffix;
  /* A string describing the arguments for this instruction.  */
  const char *args;
  /* The basic opcode for the instruction.  When assembling, this
     opcode is modified by the arguments to produce the actual opcode
     that is used.  If pinfo is INSN_MACRO, then this is 0.  */
  unsigned long match;
  /* If pinfo is not INSN_MACRO, then this is a bit mask for the
     relevant portions of the opcode when disassembling.  If the
     actual opcode anded with the match field equals the opcode field,
     then we have found the correct instruction.  If pinfo is
     INSN_MACRO, then this field is the macro identifier.  */
  unsigned long mask;
  /* For a macro, this is INSN_MACRO.  Otherwise, it is a collection
     of bits describing the instruction, notably any relevant hazard
     information.  */
  unsigned long pinfo;
  /* A collection of additional bits describing the instruction. */
  unsigned long pinfo2;
  /* A collection of bits describing the instruction sets of which this
     instruction or macro is a member. */
  unsigned long membership;
  /* A collection of bits describing the ASE of which this instruction
     or macro is a member.  */
  unsigned long ase;
};

/* Return true if the assembly syntax allows OPERAND to be omitted.  */

static inline bfd_boolean
nanomips_optional_operand_p (const struct nanomips_operand *operand)
{
  return (operand->type == OP_OPTIONAL_REG
	  || operand->type == OP_REPEAT_PREV_REG
	  || (operand->type != OP_INT
	      && operand->size == 0
	      && operand->lsb == 0));
}

/* Return a version of INSN in which the field specified by OPERAND
   has value UVAL.  */

static inline unsigned int
nanomips_insert_operand (const struct nanomips_operand *operand,
			 unsigned int insn, unsigned int uval)
{
  unsigned int mask;
  unsigned int size_bottom = operand->size - operand->size_top;

  mask = (1 << size_bottom) - 1;
  insn &= ~(mask << operand->lsb);
  insn |= (uval & mask) << operand->lsb;

  mask = (1 << operand->size_top) - 1;
  insn &= ~(mask << operand->lsb_top);
  insn |= ((uval & (mask << size_bottom)) >> size_bottom) << operand->lsb_top;
  return insn;
}

/* Extract OPERAND from instruction INSN.  */

static inline unsigned int
nanomips_extract_operand (const struct nanomips_operand *operand,
			  unsigned int insn)
{
  unsigned int uval;
  unsigned int size_bottom = operand->size - operand->size_top;

  uval = (insn >> operand->lsb_top) & ((1 << operand->size_top) - 1);
  uval <<= size_bottom;
  uval |= (insn >> operand->lsb) & ((1 << size_bottom) - 1);
  return uval;
}

/* UVAL is the value encoded by OPERAND.  Return it in signed form.  */

static inline int
nanomips_signed_operand (const struct nanomips_operand *operand,
			 unsigned int uval)
{
  unsigned int sign_bit, mask;

  mask = (1 << operand->size) - 1;
  sign_bit = 1 << (operand->size - 1);
  return ((uval + sign_bit) & mask) - sign_bit;
}

/* Return the integer that OPERAND encodes as UVAL.  */

static inline int
nanomips_decode_int_operand (const struct nanomips_int_operand *operand,
			     unsigned int uval)
{
  uval |= (operand->max_val - uval) & -(1 << operand->root.size);
  uval += operand->bias;
  uval <<= operand->shift;
  return uval;
}

/* Return the maximum value that can be encoded by OPERAND.  */

static inline int
nanomips_int_operand_max (const struct nanomips_int_operand *operand)
{
  return (operand->max_val + operand->bias) << operand->shift;
}

/* Return the minimum value that can be encoded by OPERAND.  */

static inline int
nanomips_int_operand_min (const struct nanomips_int_operand *operand)
{
  unsigned int mask;

  mask = (1 << operand->root.size) - 1;
  return nanomips_int_operand_max (operand) - (mask << operand->shift);
}

/* Return the register that OPERAND encodes as UVAL.  */

static inline int
nanomips_decode_reg_operand (const struct nanomips_reg_operand *operand,
			     unsigned int uval)
{
  if (operand->reg_map)
    uval = operand->reg_map[uval];
  return uval;
}

/* PC-relative operand OPERAND has value UVAL and is relative to BASE_PC.
   Return the address that it encodes.  */

static inline bfd_vma
nanomips_decode_pcrel_operand (const struct nanomips_pcrel_operand *operand,
			       bfd_vma base_pc, unsigned int uval)
{
  bfd_vma addr;

  addr = base_pc & -(1 << operand->align_log2);
  addr += nanomips_decode_int_operand (&operand->root, uval);
  if (operand->include_isa_bit)
    addr |= base_pc & 1;
  if (operand->flip_isa_bit)
    addr ^= 1;
  return addr;
}

/* Describes an operand that encapsulates a mapped register with
   a check against the previous operand.  */
struct nanomips_mapped_check_prev_operand
{
  struct nanomips_operand root;

  enum nanomips_reg_operand_type reg_type;
  const unsigned char *reg_map;

  bfd_boolean greater_than_ok;
  bfd_boolean less_than_ok;
  bfd_boolean equal_ok;
  bfd_boolean zero_ok;
};

/* Describes an operand that encapsulates a base register with
   a check against the type of offset.  */
struct nanomips_base_check_offset_operand
{
  struct nanomips_operand root;

  enum nanomips_reg_operand_type reg_type;

  bfd_boolean const_ok;
  bfd_boolean expr_ok;
};

/* Return true if MO is an instruction that requires 32-bit encoding.  */

static inline bfd_boolean
nanomips_opcode_32bit_p (const struct nanomips_opcode *mo)
{
  return mo->mask >> 16 != 0;
}

static inline int
nanomips_operand_mask (const struct nanomips_operand *operand)
{
  unsigned int mask;

  mask = ((1 << operand->size_top) - 1) << operand->lsb_top;
  mask |= ((1 << (operand->size - operand->size_top)) - 1) << operand->lsb;
  return mask;
}

/* Return the UVAL encoding of REGNO as OPERAND.  */

static inline unsigned int
nanomips_encode_reg_operand (const struct nanomips_operand *operand,
			     int regno)
{
  unsigned int uval;
  const unsigned int num_vals = 1 << operand->size;
  const struct nanomips_reg_operand *reg_op
    = (const struct nanomips_reg_operand *) operand;

  for (uval = 0; uval < num_vals; uval++)
    if (reg_op->reg_map[uval] == regno)
      break;
  return uval;
}


/* Re-organize HI20 bits of OPERAND encoded as UVAL.  */

#define SIGNEX_VALUE(OP) {OP_INT, (unsigned short)(OP->size - 1), 0, 0, 0}

static inline int
nanomips_decode_hi20_operand (const struct nanomips_operand *operand,
			      unsigned int uval)
{
  const struct nanomips_operand op_ext = SIGNEX_VALUE (operand);
  const struct nanomips_operand op_shuffle = {OP_INT, 19, 10, 10, 0};
  unsigned int low19 = nanomips_extract_operand (&op_shuffle, uval);
  return nanomips_insert_operand (&op_ext, uval, low19);
}

/* Decode HI20 signed integer.  */

#define SIGNED_VALUE(OP) {OP_INT, OP->size, 0, 0, 0}

static inline int
nanomips_decode_hi20_int_operand (const struct nanomips_operand *operand,
				  unsigned int uval)
{
  const struct nanomips_operand op_enc = SIGNED_VALUE (operand);
  uval = nanomips_decode_hi20_operand (operand, uval);
  return (nanomips_signed_operand (&op_enc, uval));
}

/* Decode HI20 PCREL  */

#define PCREL_VALUE(OP) { { { OP_PCREL, OP->size, 0, 0, 0}, \
	(unsigned int)((1 << (OP->size - 1)) - 1), 0, 0, FALSE}, 12, 0, 0}

static inline bfd_vma
nanomips_decode_hi20_pcrel_operand (const struct nanomips_operand *operand,
				    bfd_vma base_pc, unsigned int uval)
{
  const struct nanomips_pcrel_operand pcrel_op = PCREL_VALUE (operand);
  uval = nanomips_decode_hi20_operand (operand, uval);
  return nanomips_decode_pcrel_operand (&pcrel_op, base_pc, uval << 12);
}


/* Return true if MO is an instruction that requires 48-bit encoding.  */

static inline bfd_boolean
opcode_48bit_p (const struct nanomips_opcode *mo)
{
  return ((mo->mask >> 16 == 0)
	  && ((mo->match >> 10) == 0x18));
}

/* These are the bits which may be set in the pinfo field of an
   instructions, if it is not equal to INSN_MACRO.  */

/* Writes to operand number N.  */
#define INSN_WRITE_SHIFT            0
#define INSN_WRITE_1                0x00000001
#define INSN_WRITE_2                0x00000002
#define INSN_WRITE_ALL              0x00000003
/* Reads from operand number N.  */
#define INSN_READ_SHIFT             2
#define INSN_READ_1                 0x00000004
#define INSN_READ_2                 0x00000008
#define INSN_READ_3                 0x00000010
#define INSN_READ_ALL               0x0000001c
/* Modifies general purpose register 31.  */
#define INSN_WRITE_GPR_31           0x00000020
/* Reads coprocessor register other than floating point register.  */
#define INSN_COP                    0x00000040
/* Instruction loads value from memory.  */
#define INSN_LOAD_MEMORY	    0x00000080
/* Reads the accumulator register.  */
#define INSN_READ_ACC		    0x00000100
/* Modifies the HI register.  */
#define INSN_WRITE_ACC		    0x00000200
/* Instruction stores value into memory.  */
#define INSN_STORE_MEMORY	0x00000400
/* Instruction uses single precision floating point.  */
#define INSN_FP_S		0x00000800
/* Instruction uses double precision floating point.  */
#define INSN_FP_D		0x00001000
/* A user-defined instruction.  */
#define INSN_UDI                    0x00002000
/* Instruction is actually a macro.  It should be ignored by the
   disassembler, and requires special treatment by the assembler.  */
#define INSN_MACRO                  0xffffffff

/* These are the bits which may be set in the pinfo2 field of an
   instruction. */

/* Instruction is a simple alias (I.E. "move" for daddu/addu/or) */
#define	INSN2_ALIAS		    0x00000001
/* Macro uses single-precision floating-point instructions.  This should
   only be set for macros.  For instructions, FP_S in pinfo carries the
   same information.  */
#define INSN2_M_FP_S		    0x00000002
/* Macro uses double-precision floating-point instructions.  This should
   only be set for macros.  For instructions, FP_D in pinfo carries the
   same information.  */
#define INSN2_M_FP_D		    0x00000004
/* Is an unconditional branch insn. */
#define INSN2_UNCOND_BRANCH	    0x00000008
/* Is a conditional branch insn. */
#define INSN2_COND_BRANCH	    0x00000010
/* This indicates delayed branch converted to compact branch.  */
#define INSN2_CONVERTED_TO_COMPACT  0x00000020
/* Marks the LI macro expansion as special, temporary.  */
#define INSN2_MACRO		    0x00000040
/* Marks the legacy/downgraded MTTGPR format, temporary.  */
#define INSN2_MTTGPR_RC1	    0x00000080

/* Masks used to mark instructions to indicate which MIPS ISA level
   they were introduced in.  INSN_ISA_MASK masks an enumeration that
   specifies the base ISA level(s).  The remainder of a 32-bit
   word constructed using these macros is a bitmask of the remaining
   INSN_* values below.  */

#define INSN_ISA_MASK		  0x00000003ul

/* We cannot start at zero due to ISA_UNKNOWN below.  */
#define INSN_ISAN32R6   1
#define INSN_ISAN64R6	2

#define ISA_UNKNOWN	0               /* Gas internal use.  */

#define ISA_NANOMIPS32R6	INSN_ISAN32R6
#define ISA_NANOMIPS64R6	INSN_ISAN64R6

/* CPU defines, use instead of hardcoding processor number. Keep this
   in sync with bfd/archures.c in order for machine selection to work.  */
#define CPU_UNKNOWN	0               /* Gas internal use.  */

#define CPU_NANOMIPS32R6 32
#define CPU_NANOMIPS64R6 64

#define ISAF(X) (1 << (INSN_ISA##X - 1))

/* The same information in table form: bit INSN_ISA<X> - 1 of index
   INSN_UPTO<Y> - 1 is set if ISA Y includes ISA X.  */
static const unsigned int nanomips_isa_table[] = {
  ISAF(N32R6),
  ISAF(N32R6) | ISAF(N64R6)
};
#undef ISAF

/* DSP ASE */
#define ASE_DSP			0x00000001
#define ASE_DSP64		0x00000002
/* Enhanced VA Scheme */
#define ASE_EVA			0x00000004
/* MCU (MicroController) ASE */
#define ASE_MCU			0x00000008
/* MT ASE */
#define ASE_MT			0x00000010
/* Virtualization ASE */
#define ASE_VIRT		0x00000020
#define ASE_VIRT64		0x00000040
/* MSA Extension  */
#define ASE_MSA			0x00000080
#define ASE_MSA64		0x00000100
/* Cyclic redundancy check (CRC) ASE */
#define ASE_CRC			0x00000200
#define ASE_CRC64		0x00000400
/* Global INValidate Extension. */
#define ASE_GINV		0x00000800
/* The Virtualization ASE has Global INValidate extension instructions
   which are only valid when both ASEs are enabled. */
#define ASE_GINV_VIRT		0x00001000
/* Excluded for low power instruction subset for nanoMIPS.  */
#define ASE_xNMS		0x00002000
/* TLB control ASE.  */
#define ASE_TLB			0x00004000

static inline bfd_boolean
nanomips_cpu_is_member (int cpu, unsigned int mask)
{
  switch (cpu)
    {
    case CPU_NANOMIPS32R6:
      return (mask & INSN_ISA_MASK) == INSN_ISAN32R6;

    case CPU_NANOMIPS64R6:
      return ((mask & INSN_ISA_MASK) == INSN_ISAN32R6)
	     || ((mask & INSN_ISA_MASK) == INSN_ISAN64R6);

    default:
      return FALSE;
    }
}

/* Test for membership in an ISA including chip specific ISAs.  INSN
   is pointer to an element of the opcode table; ISA is the specified
   ISA/ASE bitmask to test against; and CPU is the CPU specific ISA to
   test, or zero if no CPU specific ISA test is desired.  Return true
   if instruction INSN is available to the given ISA and CPU. */
static inline bfd_boolean
nanomips_opcode_is_member (const struct nanomips_opcode *insn,
			   int isa, int ase, int cpu)
{
  /* Test for ISA level compatibility.  */
  if ((isa & INSN_ISA_MASK) != 0
      && (insn->membership & INSN_ISA_MASK) != 0
      && ((nanomips_isa_table[(isa & INSN_ISA_MASK) - 1]
	   >> ((insn->membership & INSN_ISA_MASK) - 1)) & 1) != 0)
    return TRUE;

  /* Test for ASE compatibility.  */
  if (insn->ase != 0 && (ase & insn->ase) == insn->ase)
    return TRUE;

  /* Test for processor-specific extensions.  */
  if (nanomips_cpu_is_member (cpu, insn->membership))
    return TRUE;

  return FALSE;
}

/* This is a list of macro expanded instructions.

   _I appended means immediate
   _A appended means target address of a jump
   _AB appended means address with (possibly zero) base register
   _AC appended means either symbolic address with no base register
   or constant offset with base register.
   _D appended means 64 bit floating point constant
   _S appended means 32 bit floating point constant.  */

enum
{
  M_ABS,
  M_ACLR_AC,
  M_ADD_I,
  M_ADDU_I,
  M_AND_I,
  M_ASET_AC,
  M_BEQ,
  M_BEQ_I,
  M_BGE,
  M_BGE_I,
  M_BGEU,
  M_BGEU_I,
  M_BGEZ,
  M_BGT,
  M_BGT_I,
  M_BGTU,
  M_BGTU_I,
  M_BGTZ,
  M_BLE,
  M_BLE_I,
  M_BLEU,
  M_BLEU_I,
  M_BLEZ,
  M_BLT,
  M_BLT_I,
  M_BLTU,
  M_BLTU_I,
  M_BLTZ,
  M_BNE,
  M_BNE_I,
  M_CACHE_AC,
  M_CACHEE_AC,
  M_DABS,
  M_DADD_I,
  M_DADDU_I,
  M_DLA_AB,
  M_DLI,
  M_DMUL,
  M_DMUL_I,
  M_DSUB_I,
  M_DSUBU_I,
  M_J_A,
  M_JAL_A,
  M_JRADDIUSP,
  M_LA_AB,
  M_LB_AC,
  M_LBE_AC,
  M_LBU_AC,
  M_LBUE_AC,
  M_LBX_AB,
  M_LBUX_AB,
  M_LD_AC,
  M_LDC1_AC,
  M_LDC1X_AB,
  M_LDC2_AC,
  M_LDM_AC,
  M_LDX_AB,
  M_LH_AC,
  M_LHE_AC,
  M_LHU_AC,
  M_LHUE_AC,
  M_LHUX_AB,
  M_LHX_AB,
  M_LI,
  M_LI_D,
  M_LI_DD,
  M_LI_S,
  M_LI_SS,
  M_LL_AC,
  M_LLD_AC,
  M_LLE_AC,
  M_LLDP_AC,
  M_LLWP_AC,
  M_LW_AC,
  M_LWC1_AC,
  M_LWC1X_AB,
  M_LWC2_AC,
  M_LWE_AC,
  M_LWM_AC,
  M_LWU_AC,
  M_LWUX_AB,
  M_LWX_AB,
  M_MUL,
  M_MUL_I,
  M_NOR_I,
  M_OR_I,
  M_PREF_AC,
  M_PREFE_AC,
  M_REM_3I,
  M_DROL,
  M_ROL,
  M_DROL_I,
  M_ROL_I,
  M_ROR_I,
  M_SC_AC,
  M_SCD_AC,
  M_SCE_AC,
  M_SCDP_AC,
  M_SCWP_AC,
  M_SD_AC,
  M_SDC1_AC,
  M_SDC1X_AB,
  M_SDC2_AC,
  M_SDM_AC,
  M_SDX_AB,
  M_SEQ,
  M_SEQ_I,
  M_SGE,
  M_SGE_I,
  M_SGEU,
  M_SGEU_I,
  M_SGT,
  M_SGT_I,
  M_SGTU,
  M_SGTU_I,
  M_SLE,
  M_SLE_I,
  M_SLEU,
  M_SLEU_I,
  M_SLT_I,
  M_SLTU_I,
  M_SNE,
  M_SNE_I,
  M_SB_AC,
  M_SBE_AC,
  M_SBX_AB,
  M_SH_AC,
  M_SHE_AC,
  M_SHX_AB,
  M_SW_AC,
  M_SWE_AC,
  M_SWX_AB,
  M_SWC1_AC,
  M_SWC1X_AB,
  M_SWC2_AC,
  M_SWM_AC,
  M_SUB_I,
  M_SUBU_I,
  M_TEQ_I,
  M_TNE_I,
  M_ULD_AC,
  M_ULH_AC,
  M_ULW_AC,
  M_USH_AC,
  M_USW_AC,
  M_USD_AC,
  M_XOR_I,
  M_BGEZAL,
  M_BLTZAL,
  M_EXT,
  M_INS,
  M_MOD_I,
  M_MODU_I,
  M_DMOD_I,
  M_DMODU_I,
  M_DIV_I,
  M_DIVU_I,
  M_DDIV_I,
  M_DDIVU_I,
  M_NANOMIPS_NUM_MACROS
};

/* These are the bit masks and shift counts used for the different fields
   in the nanoMIPS instruction formats.  No masks are provided for the
   fixed portions of an instruction, since they are not needed.  */

#define NANOMIPSOP_MASK_RS		0x1f
#define NANOMIPSOP_SH_RS		16
#define NANOMIPSOP_MASK_RT		0x1f
#define NANOMIPSOP_SH_RT		21
#define NANOMIPSOP_MASK_RD		0x1f
#define NANOMIPSOP_SH_RD		11
#define NANOMIPSOP_SH_ME		1
#define NANOMIPSOP_SH_MC		4
#define NANOMIPSOP_SH_MD		7
#define NANOMIPSOP_SH_MP		5
#define NANOMIPSOP_SH_MM		7

#define NANOMIPSOP_SH_CP0SEL		5
#define NANOMIPSOP_MASK_CP0SEL		0x1f
#define NANOMIPSOP_SH_HWRSEL		5
#define NANOMIPSOP_MASK_HWRSEL		0x1f

/* Describes a COP0 named register with a fixed select.  */
struct nanomips_cp0_name
{
  const char *name;
  unsigned int num;
  unsigned int sel;
};

/* The reference list of COP0 named register with fixed selects.  */
static const struct nanomips_cp0_name nanomips_cp0_3264r6[] = {
    {"$index",		 0, 0},
    {"$mvpcontrol",	 0, 1},
    {"$mvpconf0",	 0, 2},
    {"$mvpconf1",	 0, 3},
    {"$vpcontrol",	 0, 4},
    {"$random", 	 1, 0},
    {"$vpecontrol",	 1, 1},
    {"$vpeconf0",	 1, 2},
    {"$vpeconf1",	 1, 3},
    {"$yqmask", 	 1, 4},
    {"$vpeschedule",	 1, 5},
    {"$vpeschefback",	 1, 6},
    {"$vpeopt", 	 1, 7},
    {"$entrylo0",	 2, 0},
    {"$tcstatus",	 2, 1},
    {"$tcbind", 	 2, 2},
    {"$tcrestart",	 2, 3},
    {"$tchalt", 	 2, 4},
    {"$tccontext",	 2, 5},
    {"$tcschedule",	 2, 6},
    {"$tcschefback",	 2, 7},
    {"$entrylo1",	 3, 0},
    {"$globalnumber",	 3, 1},
    {"$tcopt",		 3, 7},
    {"$context",	 4, 0},
    {"$contextconfig",	 4, 1},
    {"$userlocal",	 4, 2},
    {"$xcontextconfig",  4, 3},
    {"$debugcontextid",  4, 4},
    {"$memorymapid",	 4, 5},
    {"$pagemask",	 5, 0},
    {"$pagegrain",	 5, 1},
    {"$segctl0",	 5, 2},
    {"$segctl1",	 5, 3},
    {"$segctl2",	 5, 4},
    {"$pwbase", 	 5, 5},
    {"$pwfield",	 5, 6},
    {"$pwsize", 	 5, 7},
    {"$wired",		 6, 0},
    {"$srsconf0",	 6, 1},
    {"$srsconf1",	 6, 2},
    {"$srsconf2",	 6, 3},
    {"$srsconf3",	 6, 4},
    {"$srsconf4",	 6, 5},
    {"$pwctl",		 6, 6},
    {"$hwrena", 	 7, 0},
    {"$badvaddr",	 8, 0},
    {"$badinst",	 8, 1},
    {"$badinstrp",	 8, 2},
    {"$badinstrx",	 8, 3},
    {"$count",		 9, 0},
    {"$entryhi",	10, 0},
    {"$guestctl1",	10, 4},
    {"$guestctl2",	10, 5},
    {"$guestctl3",	10, 6},
    {"$compare",	11, 0},
    {"$guestctl0ext",	11, 4},
    {"$status", 	12, 0},
    {"$intctl", 	12, 1},
    {"$srsctl", 	12, 2},
    {"$srsmap", 	12, 3},
    {"$view_ipl",	12, 4},
    {"$srsmap2",	12, 5},
    {"$guestctl0",	12, 6},
    {"$gtoffset",	12, 7},
    {"$cause",		13, 0},
    {"$view_ripl",	13, 4},
    {"$nestedexc",	13, 5},
    {"$epc",		14, 0},
    {"$nestedepc",	14, 2},
    {"$prid",		15, 0},
    {"$ebase",		15, 1},
    {"$cdmmbase",	15, 2},
    {"$cmgcrbase",	15, 3},
    {"$bevva",		15, 4},
    {"$config", 	16, 0},
    {"$config1",	16, 1},
    {"$config2",	16, 2},
    {"$config3",	16, 3},
    {"$config4",	16, 4},
    {"$config5",	16, 5},
    {"$lladdr", 	17, 0},
    {"$maar", 		17, 1},
    {"$maari",		17, 2},
    {"$watchlo0",	18, 0},
    {"$watchlo1",	18, 1},
    {"$watchlo2",	18, 2},
    {"$watchlo3",	18, 3},
    {"$watchlo4",	18, 4},
    {"$watchlo5",	18, 5},
    {"$watchlo6",	18, 6},
    {"$watchlo7",	18, 7},
    {"$watchlo8",	18, 8},
    {"$watchlo9",	18, 9},
    {"$watchlo10",	18, 10},
    {"$watchlo11",	18, 11},
    {"$watchlo12",	18, 12},
    {"$watchlo13",	18, 13},
    {"$watchlo14",	18, 14},
    {"$watchlo15",	18, 15},
    {"$watchhi0",	19, 0},
    {"$watchhi1",	19, 1},
    {"$watchhi2",	19, 2},
    {"$watchhi3",	19, 3},
    {"$watchhi4",	19, 4},
    {"$watchhi5",	19, 5},
    {"$watchhi6",	19, 6},
    {"$watchhi7",	19, 7},
    {"$watchhi8",	19, 8},
    {"$watchhi9",	19, 9},
    {"$watchhi10",	19, 10},
    {"$watchhi11",	19, 11},
    {"$watchhi12",	19, 12},
    {"$watchhi13",	19, 13},
    {"$watchhi14",	19, 14},
    {"$watchhi15",	19, 15},
    {"$xcontext",	20, 0},
    {"$debug",		23, 0},
    {"$tracecontrol",	23, 1},
    {"$tracecontrol2",	23, 2},
    {"$usertracedata1", 23, 3},
    {"$traceibpc",	23, 4},
    {"$tracedbpc",	23, 5},
    {"$debug2", 	23, 6},
    {"$depc",		24, 0},
    {"$tracecontrol3",	24, 2},
    {"$usertracedata2", 24, 3},
    {"$perfctl0",	25, 0},
    {"$perfcnt0",	25, 1},
    {"$perfctl1",	25, 2},
    {"$perfcnt1",	25, 3},
    {"$perfctl2",	25, 4},
    {"$perfcnt2",	25, 5},
    {"$perfctl3",	25, 6},
    {"$perfcnt3",	25, 7},
    {"$perfctl4",	25, 8},
    {"$perfcnt4",	25, 9},
    {"$perfctl5",	25, 10},
    {"$perfcnt5",	25, 11},
    {"$perfctl6",	25, 12},
    {"$perfcnt6",	25, 13},
    {"$perfctl7",	25, 14},
    {"$perfcnt7",	25, 15},
    {"$errctl", 	26, 0},
    {"$cacheerr",	27, 0},
    {"$itaglo", 	28, 0},
    {"$idatalo", 	28, 1},
    {"$dtaglo", 	28, 2},
    {"$ddatalo", 	28, 3},
    {"$itaghi", 	29, 0},
    {"$idatahi", 	29, 1},
    {"$dtaghi", 	29, 2},
    {"$ddatahi", 	29, 3},
    {"$errorepc",	30, 0},
    {"$desave", 	31, 0},
    {"$kscratch1",	31, 2},
    {"$kscratch2",	31, 3},
    {"$kscratch3",	31, 4},
    {"$kscratch4",	31, 5},
    {"$kscratch5",	31, 6},
    {"$kscratch6",	31, 7},
    {NULL, 0, 0}
};

/* Describes a CP0 named register which permits various select values.  */
  struct nanomips_cp0_select
{
  const char *name;
  unsigned int num;
  unsigned int selmask;
};

/* Currently recognized CP0 select patterns.  */

#define NANOMIPS_CP0SEL_MASK_EVEN	0x55555555
#define NANOMIPS_CP0SEL_MASK_ODD	0xaaaaaaaa
#define NANOMIPS_CP0SEL_MASK_ANY	0xffffffff
#define NANOMIPS_CP0SEL_MASK_EVEN16	0x5555
#define NANOMIPS_CP0SEL_MASK_ODD16	0xaaaa
#define NANOMIPS_CP0SEL_MASK_ANY16	0xffff

/* The reference list of CP0 named register with variable selects.  */
static const struct nanomips_cp0_select nanomips_cp0sel_3264r6[] = {
    {"$watchlo",	18, NANOMIPS_CP0SEL_MASK_ANY16},
    {"$watchhi",	19, NANOMIPS_CP0SEL_MASK_ANY16},
    {"$perfctl",	25, NANOMIPS_CP0SEL_MASK_EVEN16},
    {"$perfcnt",	25, NANOMIPS_CP0SEL_MASK_ANY16},
    {"$taglo",		28, NANOMIPS_CP0SEL_MASK_EVEN},
    {"$datalo", 	28, NANOMIPS_CP0SEL_MASK_ODD},
    {"$taghi",		29, NANOMIPS_CP0SEL_MASK_EVEN},
    {"$datahi", 	29, NANOMIPS_CP0SEL_MASK_ODD},
    {NULL, 0, 0}
};


/* Describes a HWR named register with a fixed select.  If the HWR name
   is remapped from an existing CP0 register name, its cp0_num and cp0_sel
   fields will provide the mapping, else they will both be invalid.  */

struct nanomips_hwr_name
{
  const char *name;
  unsigned int num;
  unsigned int sel;
  unsigned int cp0_num;
  unsigned int cp0_sel;
};

#define INV_RNUM 0xffffffff
#define INV_SEL 0xffffffff

 /* The reference list of named hardware register with fixed selects.  */
static const struct nanomips_hwr_name nanomips_hwr_names_3264r6[] = {
    {"$cpunum",		0,	0,	INV_RNUM,	INV_SEL},
    {"$synci_step",	1,	0,	INV_RNUM,	INV_SEL},
    {"$cc",		2,	0,	9,		0},
    {"$count",		2,	0,	9,		0},
    {"$ccres",		3,	0,	INV_RNUM, 	INV_SEL},
    {"$perfctl0",	4,	0,	25,		0},
    {"$perfcnt0",	4,	1,	25,		1},
    {"$perfctl1",	4,	2,	25,		2},
    {"$perfcnt1",	4,	3,	25,		3},
    {"$perfctl2",	4,	4,	25,		4},
    {"$perfcnt2",	4,	5,	25,		5},
    {"$perfctl3",	4,	6,	25,		6},
    {"$perfcnt3",	4,	7,	25,		7},
    {"$perfctl4",	4,	8,	25,		8},
    {"$perfcnt4",	4,	9,	25,		9},
    {"$perfctl5",	4,	10,	25,		10},
    {"$perfcnt5",	4,	11,	25,		11},
    {"$perfctl6",	4,	12,	25,		12},
    {"$perfcnt6",	4,	13,	25,		13},
    {"$perfctl7",	4,	14,	25,		14},
    {"$perfcnt7",	4,	15,	25,		15},
    {"$perfctl",	4,	0,	25,		0},
    {"$perfcnt",	4,	1,	25,		1},
    {"$xnp",		5,	0,	INV_RNUM,	INV_SEL},
    {"$userlocal",	29,	0,	4,		2},
    {NULL, 		0,	0,	INV_RNUM,	INV_SEL}
};

#define NANOMIPS_CP0SEL_PERFCNT 25
#define NANOMIPS_HWRSEL_PERFCNT 4

/* Don't care stubs in operand formats need special handling.  */
#define NANOMIPS_MIN_DONTCARE_FMT 'A'
#define NANOMIPS_MAX_DONTCARE_FMT 'Q'

#define IS_NANOMIPS_DONTCARE_FMT(x) ((x)[0] == '-'			\
				     && (x)[1] >= NANOMIPS_MIN_DONTCARE_FMT \
				     && (x)[1] <= NANOMIPS_MAX_DONTCARE_FMT)

/* These are the characters which may appears in the args field of a nanoMIPS
   instruction.  They appear in the order in which the fields appear when the
   instruction is used.  Commas and parentheses in the args string are ignored
   when assembling, and written into the output when disassembling.

   Operands for 16-bit nanoMIPS instructions.

   "ma" must be $28
   "mb" 5-bit non-zero GP register at bit 5
   "mc" 3-bit nanoMIPS registers 4-7, 16-19 bit 4
        The same register used as both source and target.
   "md" 3-bit nanoMIPS registers 4-7, 16-19 at bit 7
   "me" 3-bit nanoMIPS registers 4-7, 16-19 at bit 1
   "mf" 3-bit nanoMIPS register 4-7, 16-19 at bit 7.
        Must be larger than the last seen register.
   "mg" 3-bit nanoMIPS register 4-7, 16-19 at bit 4.
	Must be smaller than the last seen register.
   "mh" 3-bit nanoMIPS register 4-7, 16-19 at bit 7.
        Must be at least as large as the last seen register.
   "mi" 3-bit nanoMIPS register 4-7, 16-19 at bit 4.
	May be at most as large as the last seen register.

   "mj" 5-bit nanoMIPS registers at bit 0
   "mk" must be the same as the destination register
   "ml" 3-bit nanoMIPS registers 4-7, 16-19 at bit 4
   "mm" 3-bit nanoMIPS registers 0, 4-7, 17-19 at bit 7
   "mn"	5-bit encoding of a save/restore register list
   "mp" 5-bit nanoMIPS registers at bit 5
   "mq" 2-bit pair at bits [8,3] maps to ($a0,$a1), ($a1,$a2), ($a2,$a3)
        or ($a3,$a4)
   "mr" 2-bit pair at bits [8,3] maps to ($a1,$a0), ($a2,$a1), ($a3,$a2)
        or ($a4,$a3)
   "ms" must be $29
   "mt" must be the same as the previous register
   "mu"	4-bit encoding of nanoMIPS destination register at bit 5
   "mv"	4-bit encoding of nanoMIPS destination register at bit 0
   "mw"	4-bit encoding of nanoMIPS source register at bit 5
   "mx"	4-bit encoding of nanoMIPS source register at bit 0
   "my" must be $31
   "mz" must be literal 0

   "mA" 7-bit relocatable GP offset (0 .. 127) << 2
   "mB" 3-bit immediate at bit 0 (0, 4, 8, 12, 16, 20, 24, 28)
   "mC" 4-bit immediate at bit 0 (0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
        65535, 14, 15)
   "mD" 10-bit signed branch address, split & scaled at bit 0 [S9:1,S10]
   "mE" 7-bit signed branch address, split & scaled at bit 0 [S6:1,S7]
   "mF" 4-bit unsigned branch address, scaled at bit 0 [U4:U1]
   "mG" 4-bit scaled immediate at bit 4, (0 .. 15) << 4
   "mH" 2-bit scaled immediate at bit 1, (0 .. 3) << 1
   "mI" 7-bit immediate at bit 0, (-1 .. 126)
   "mJ" 4-bit scaled immediate at bit 0, (0 .. 15) << 2
   "mK" 3-bit BREAK/SDBBP code at bit 0
   "mL" 2-bit immediate at bit 0, (0 .. 3)
   "mM" 3-bit immediate at bit 0, (1 .. 8)
   "mN" 2-bit split scaled immediate at bits 8 & 3 (0 .. 3) << 2
   "mO" 7-bit immediate GP offset at bit 0, (0 .. 127) << 2
   "mP"	2-bit SYSCALL/HYPCALL code at bit 0
   "mQ"	4-bit immediate signed offset, (s3,s2:s0)
   "mR" 5-bit immediate at bit 0, (0 .. 31) << 2
   "mS" 6-bit immediate at bit 0, (0 .. 63) << 2
   "mZ" must be zero

   Operands for 32-bit nanoMIPS instructions.

   "+1"	18-bit unsigned GP-relative offset, (u17:u0)
   "+2"	18-bit scaled GP-relative offset, (u18:u2) << 2
   "+3"	21-bit scaled GP-relative offset, (u18:u1) << 1
   "+4" 18-bit GP-relative offset, (0 .. 2^18-1) << 3
   "+5"	4-bit encoding of nanoMIPS source register at bit 21
   "+6" 5-bit mask encoding, corresponding to (1 << X) - 1
   "+7" 1-bit register at bit 24, (0,1) => ($a0,$a1)
   "+8" 23-bit un-spec'ed value at bit 3 for UDIs.
   "+9" 7-bit immediate at bit 11, (0 .. 127)

   "+A" 5-bit INS/EXT/DINS/DEXT/DINSM/DEXTM position, which becomes
        LSB.
   "+B" 5-bit INS/DINS size, which becomes MSB
	Requires that "+A" or "+E" occur first to set position.
	Enforces: 0 < (pos+size) <= 32.
   "+C" 5-bit EXT/DEXT size, which becomes MSBD.
	Requires that "+A" or "+E" occur first to set position.
	Enforces: 0 < (pos+size) <= 32.
   "+D"	4-bit encoding of a floating point save/restore register list
   "+E" 5-bit DINSU/DEXTU position, which becomes LSB-32.
   "+F" 5-bit DINSM/DINSU size, which becomes MSB-32.
	Requires that "+A" or "+E" occur first to set position.
	Enforces: 32 < (pos+size) <= 64.
   "+G" 5-bit DEXTM size, which becomes MSBD-32.
	Requires that "+A" or "+E" occur first to set position.
	Enforces: 32 < (pos+size) <= 64.
   "+H" 5-bit DEXTU size, which becomes MSBD.
	Requires that "+A" or "+E" occur first to set position.
	Enforces: 32 < (pos+size) <= 64.
   "+I" 5-bit EXTW/EXTD/PREPEND position, which becomes LSB.
   "+J" 19-bit BREAK/SDBBP function code at bit 0
   "+K"	Tri-part upper 20-bits of immediate value.
   "+L" 10-bit WAIT code at bit 16
   "+M"	18-bit SYSCALL/HYPCALL code at bit 0
   "+N" 9-bit immediate at bit 3, (0 .. 511) << 3

   "+i"	5-bit SYNC code type at bit 16, (0..31)
   "+j"	9-bit signed offset (s7:s0,s8), (-256 .. 255)
   "+k" 5-bit nanoMIPS registers at bit 3
   "+p"	9-bit scaled signed offset, (s7:s2) << 2 for LL/SC*
   "+q" 9-bit scaled signed offset, (s7:s3) << 3 for LLD/SCD*
   "+r"	21-bit PC-relative branch offset (s19:s1,s20) << 1
   "+s"	21-bit immediate offset for PC-relative operation ((s19:s1,s20) + 2) << 1
   "+t" 5-bit non-zero GP register at bit 21
   "+u"	25-bit PC-relative branch offset (s24:s1,s25) << 1
   "+v"	5-bit mapping of R6 ALIGN byte-wise shift to EXTW bit-wise shift.
   "+w"	2-bit shift for scaled address calculation at bit 9, (0..3)
   "+*"	4-bit ROTX shift at bit 7, (0 .. 15) << 1
   "+|"	1-bit ROTX stripe at bit 6, (0 .. 1)

   "."	21-bit scaled GP-relative offset, (u21:u2) << 2
   "<"	5-bit immediate shift value for bit operations, (0..31)
   "|"	3-bit Element count for load/store multiple, (1..8)
   "~"	11-bit branch offset, (s9:s1,s10) << 1
   "^"	5-bit trap code at bit 11, (0..31)
   "b"	5-bit base register at bit 16 for label or symbolic offsets
   "c"	5-bit base register at bit 16, either used with immediate offset
	or skipped with symbolic offset.
   "d"	5-bit destination register specifier at bit 11
   "g"	12-bit unsigned immediate at bit 0, (0..4095)
   "h"	12-bit negative immediate at bit 0, (-4095..0)
   "i"	12-bit unsigned immediate at bit 0, (0..4095)
   "j"	16-bit unsigned immediate at bit 0, (0..65535)
   "k"	5-bit cache operation code at bit 21, (0..31)
   "n"	11-bit encoding of save/restore register list
   "o"	12-bit offset at bit 0, (0..4095)
   "p"	14-bit PC-relative branch offset (s13:s1,s14) << 1
   "r"	5-bit same register at bit 16, used as both source and target
   "s"	5-bit source register specifier at bit 16
   "t"	5-bit target register specifier at bit 21
   "u" 	Tri-part upper 20 bits of address
   "x" 	Tri-part upper 20 bits of address, scaled by 12 bits
   "v"	5-bit same register used as both source and destination at bit 15
   "w"	5-bit same register used as both target and destination at bit 21
   "z"	must be zero register

   Used in special matching contexts:
   "-A"	5 don't care bits at bit 16
   "-B"	1 don't care bit at bit 10
   "-C"	12 don't care bits at bit 0
   "-D"	1 don't care bit at bit 17
   "-E"	3 don't care bits at bit 13
   "-F"	10 don't care bits at bit 16
   "-G"	8 split don't care bits, 3 at bit 9 and 5 at bit 7
   "-H"	9 don't care bits at bit 17
   "-I"	5 don't care bits at bit 21
   "-J"	3 don't care bits at bit 23
   "-K"	2 split don't care bits, 1 at bit 2 and 1 at bit 15
   "-L"	3 don't care bits at bit 6
   "-M"	3 don't care bits at bit 9
   "-N"	6 don't care bits at bit 10
   "-O"	1 don't care bit at bit 12
   "-P"	8 split don't care bits, 4 at bit 10 and 4 at bit 22
   "-Q"	1 don't care bit at bit 11

   "-i" Ignored register operand, internally used for macro expansions.
   "-m" Place-holder to copy 5 bits from bit 11 to bit 21
   "-n" Place-holder to copy 5 bits from bit 11 to bit 16

   Exclusively for 48-bit nanoMIPS instructions:

   "+O" Signed GP-relative 32-bit offset in instruction byte order
   "+P" Immediate signed 32-bit value in instruction byte order
   "+Q"	Unsigned 32-bit value or address in instruction byte order
   "+R" Signed 32-bit value in instruction byte order
   "+S" Signed PC-relative 32-bit offset in instruction byte order

   DSP instructions:
   "0"	5-bit shift value for DSP accumulator at bit 16, (0..63)
   "1"	5-bit position for DSP bit operations at bit 11
   "2" 5-bit size for DSP bit operations at bit 16
   "3" 3-bit byte vector shift at bit 13, (0..7)
   "4" 4-bit hword vector shift at bit 12, (0..15)
   "5" 8-bit unsigned immediate at bit 13, (0..255)
   "7" 2-bit DSP accumulator register at bit 14, (0..3)
   "8" 7-bit DSP control mask at bit 14, (0x3f)
   "@" 10-bit signed immediate at bit 11, (0..1023)

   Coprocessor instructions:
   "E" 5-bit target register
   "G" 5-bit source register
   "H" 5-bit sel field for (D)MTC* and (D)MFC*
   "J" 5-bit select code at bit 11 for named COP1 registers, (0..31)
   "K"	10-bit register+select encoding at bit 11 for named h/w register
   "O"	10-bit register+select encoding at bit 11 for named COP1 register
   "P"	5-bit named COP1 register at bit 16
   "Q"	5-bit select code at bit 11
   "U"	5-bit named HW register at bit 16

   MT instructions:
   "!"	1-bit u-mode for move to/from thread registers at bit 10, (0,1)
   "$"	1-bit high-mode for move to/from thread registers at bit 3, (0,1)
   "*"	2-bit accumulator register at bit 18, (0..3)

   GINV instructions
   "+;"	2-bit global invalidate operation type at bit 21, (0..3)

   Floating point instructions:
   "D" 5-bit destination register
   "R" 5-bit fr destination register
   "S" 5-bit fs source 1 register
   "T" 5-bit ft source 2 register
   "V" 5-bit same register used as floating source and destination or target

   Macro instructions:
   "A" general 32 bit expression
   "I" 32-bit immediate (value placed in imm_expr).
   "F" 64-bit floating point constant in memory
   "L" 64-bit floating point constant in memory
   "f" 32-bit floating point constant in memory
   "l" 32-bit floating point constant in memory

   CP2 instructions:
   "C" 23-bit coprocessor function code at bit 3

   MCU instructions:
   "\"	3-bit position for atomic set/clear operations, (0..7)


   Other:
   "()" parens surrounding optional value
   ","  separates operands
   "+"  start of extension sequence

   Characters used so far, for quick reference when adding more:
   "12345 78 0"
   ".<\|~@^!$*"
   "A CDEFGHIJKL  OP RSTUV    "
   " bcde ghijk  nop rstuvwx z"

   Extension character sequences used so far ("+" followed by the
   following), for quick reference when adding more:
   "123456789 
   "*|;"
   "ABCDEFGHIJKLMNOPQRS       "
   "        ij     pqrstuvw   "

   Extension character sequences used so far ("m" followed by the
   following), for quick reference when adding more:
   ""
   ""
   "ABCDEFGHIJKLMNOPQRS      Z"
   "abcdefghijklmn pqrstuvwxyz"

   Extension character sequences used so far ("-" followed by the
   following), for quick reference when adding more:
   ""
   ""
   "ABCDEFGHIJKLMNOPQ         "
   "        i   mn            "
*/
  
extern const struct nanomips_operand *decode_nanomips_operand (const char *);
extern const struct nanomips_opcode nanomips_opcodes[];
extern const int bfd_nanomips_num_opcodes;
  
#ifdef __cplusplus
}
#endif

#endif /* __NANOMIPS_H */
