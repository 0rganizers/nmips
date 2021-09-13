/* nanomips-opc.c.  nanoMIPS opcode table.
   Copyright (C) 2018 Free Software Foundation, Inc.
   Contributed by MIPS Tech LLC.
   Written by Faraz Shahbazker <faraz.shahbazker@mips.com>

   This file is part of the GNU opcodes library.

   This library is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   It is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with this file; see the file COPYING.  If not, write to the
   Free Software Foundation, 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#include "sysdep.h"
#include "opcode/nanomips.h"
#include "nanomips-formats.h"

static unsigned char reg_0_map[] = { 0 };
static unsigned char reg_28_map[] = { 28 };
static unsigned char reg_29_map[] = { 29 };
static unsigned char reg_31_map[] = { 31 };
static unsigned char reg_m16_map[] = { 16, 17, 18, 19, 4, 5, 6, 7 };
static unsigned char reg_q_map[] = { 0, 17, 18, 19, 4, 5, 6, 7 };

static unsigned char reg_4to5_map[] = {
  8, 9, 10, 11, 4, 5, 6, 7,
  16, 17, 18, 19, 20, 21, 22, 23
};

static unsigned char reg_4to5_srcmap[] = {
  8, 9, 10, 0, 4, 5, 6, 7,
  16, 17, 18, 19, 20, 21, 22, 23
};

static unsigned char reg_4or5_map[] = { 4, 5 };

static unsigned char reg_gpr2d_map1[] = { 4, 5, 6, 7 };
static unsigned char reg_gpr2d_map2[] = { 5, 6, 7, 8 };

static int int_b_map[] = {
  0, 4, 8, 12, 16, 20, 24, 28
};

static int int_c_map[] = {
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 255, 65535, 14, 15
};

static int word_byte_map[] = {
  0, 0, 0, 0, 0, 0, 0, 0,
  3, 0, 0, 0, 0, 0, 0, 0,
  2, 0, 0, 0, 0, 0, 0, 0,
  1, 0, 0, 0, 0, 0, 0, 0
};

static int int_mask_map[] = {
  0x1, 0x3, 0x7, 0xf,
  0x1f, 0x3f, 0x7f, 0xff,
  0x1ff, 0x3ff, 0x7ff, 0xfff,
  0x1fff, 0x3fff, 0x7fff, 0xffff,
  0x1ffff, 0x3ffff, 0x7ffff, 0xfffff,
  0x1fffff, 0x3fffff, 0x7fffff, 0xffffff,
  0x1ffffff,0x3ffffff,0x7ffffff,0xfffffff,
  0x1fffffff, 0x3fffffff, 0x7fffffff, 0xffffffff
};

/* Return the nanomips_operand structure for the operand at the
   beginning of P.  */

/* FIXME: Unused cases left commented in-place for quick reminder
   of which character strings are unused.  */
const struct nanomips_operand *
decode_nanomips_operand (const char *p)
{
  switch (p[0])
    {
    case 'm':
      switch (p[1])
	{
	case 'a': MAPPED_REG (0, 0, GP, reg_28_map);
	case 'b': SPECIAL (5, 5, NON_ZERO_REG);
	case 'c': OPTIONAL_MAPPED_REG (3, 4, GP, reg_m16_map);
	case 'd': MAPPED_REG (3, 7, GP, reg_m16_map);
	case 'e': MAPPED_REG (3, 1, GP, reg_m16_map);
	case 'f': MAPPED_PREV_CHECK (3, 7, GP, reg_m16_map, TRUE, FALSE, FALSE, FALSE);
	case 'g': MAPPED_PREV_CHECK (3, 4, GP, reg_m16_map, FALSE, TRUE, FALSE, FALSE);
	case 'h': MAPPED_PREV_CHECK (3, 7, GP, reg_m16_map, FALSE, TRUE, TRUE, FALSE);
	case 'i': MAPPED_PREV_CHECK (3, 4, GP, reg_m16_map, TRUE, FALSE, TRUE, FALSE);
	case 'j': REG (5, 0, GP);
	case 'k': SPECIAL (0, 0, REPEAT_DEST_REG);
	case 'l': MAPPED_REG (3, 4, GP, reg_m16_map);
	case 'm': MAPPED_REG (3, 7, GP, reg_q_map);
	case 'n': SPECIAL_SPLIT (5, 0, 1, 9, SAVE_RESTORE_LIST);
	case 'p': REG (5, 5, GP);
	case 'q': SPLIT_MAPPED_REG_PAIR (2, 8, 1, 3, GP, reg_gpr2d_map1,
					 reg_gpr2d_map2);
	case 'r': SPLIT_MAPPED_REG_PAIR (2, 8, 1, 3, GP, reg_gpr2d_map2,
					 reg_gpr2d_map1);
	case 's': MAPPED_REG (0, 0, GP, reg_29_map);
	case 't': SPECIAL (0, 0, REPEAT_PREV_REG);
	case 'u': SPLIT_MAPPED_REG (4, 5, 1, 9, GP, reg_4to5_map);
	case 'v': SPLIT_MAPPED_REG (4, 0, 1, 4, GP, reg_4to5_map);
	case 'w': SPLIT_MAPPED_REG (4, 5, 1, 9, GP, reg_4to5_srcmap);
	case 'x': SPLIT_MAPPED_REG (4, 0, 1, 4, GP, reg_4to5_srcmap);
	case 'y': MAPPED_REG (0, 0, GP, reg_31_map);
	case 'z': UINT (0, 0); 	/* Literal 0 */
	case 'A': INT_ADJ (7, 0, 127, 2, FALSE);	 /* (0 .. 127) << 2 */
	case 'B': MAPPED_INT (3, 0, int_b_map, FALSE);
	case 'C': MAPPED_INT (4, 0, int_c_map, TRUE);
	case 'D': BRANCH_UNORD_SPLIT (10, 1);
	case 'E': BRANCH_UNORD_SPLIT (7, 1);
	case 'F': SPECIAL (4, 0, NON_ZERO_PCREL_S1);
	case 'G': INT_ADJ (4, 4, 15, 4, FALSE);
	case 'H': INT_ADJ (2, 1, 3, 1, FALSE);	 /* (0 .. 3) << 1 */
	case 'I': INT_ADJ (7, 0, 126, 0, FALSE); /* (-1 .. 126) */
	case 'J': INT_ADJ (4, 0, 15, 2, FALSE);	 /* (0 .. 15) << 2 */
	case 'K': HINT (3, 0);
	case 'L': UINT (2, 0);	 /* (0 .. 3) */
	case 'M': INT_ADJ (3, 0, 8, 0, FALSE);   /* (1 .. 8) */
	case 'N': UINT_SPLIT (2, 8, 2, 1, 3); /* split encoded 2-bit offset << 2 */
	case 'O': IMM_INT_ADJ (7, 0, 127, 2, FALSE);	 /* (0 .. 127) << 2 */
	case 'P': HINT (2, 0);
	case 'Q': SINT_SPLIT (4, 0, 0, 1, 4, 0);
	case 'R': INT_ADJ (5, 0, 31, 2, FALSE);	 /* (0 .. 31) << 2 */
	case 'S': INT_ADJ (6, 0, 63, 2, FALSE);	 /* (0 .. 63) << 2 */
	case 'Z': UINT (0, 0);			 /* 0 only */
	}
      break;

    case '-':
      switch (p[1])
	{
	case 'i': REG (0, 0, GP); /* Ignored register operand.  */
	case 'm': SPECIAL_SPLIT (5, 11, 5, 21, COPY_BITS);
	case 'n': SPECIAL_SPLIT (5, 11, 5, 16, COPY_BITS);
	case 'A': SPECIAL (5, 16, DONT_CARE);
	case 'B': SPECIAL (1, 10, DONT_CARE);
	case 'C': SPECIAL (12, 0, DONT_CARE);
	case 'D': SPECIAL (1, 17, DONT_CARE);
	case 'E': SPECIAL (3, 13, DONT_CARE);
	case 'F': SPECIAL (10, 16, DONT_CARE);
	case 'G': SPECIAL_SPLIT (8, 9, 5, 16, DONT_CARE);
	case 'H': SPECIAL (9, 17, DONT_CARE);
	case 'I': SPECIAL (5, 21, DONT_CARE);
	case 'J': SPECIAL (3, 23, DONT_CARE);
	case 'K': SPECIAL_SPLIT (2, 2, 1, 15, DONT_CARE);
	case 'L': SPECIAL (3, 6, DONT_CARE);
	case 'M': SPECIAL (3, 9, DONT_CARE);
	case 'N': SPECIAL (6, 10, DONT_CARE);
	case 'O': SPECIAL (1, 12, DONT_CARE);
	case 'P': SPECIAL_SPLIT (8, 10, 4, 22, DONT_CARE);
	case 'Q': SPECIAL (1, 11, DONT_CARE);
	}
      break;

    case '+':
      switch (p[1])
	{
	case 'A': BIT (5, 0, 0);		 /* (0 .. 31) */
	case 'B': MSB (5, 6, 1, TRUE, 32);	 /* (1 .. 32), 32-bit op */
	case 'C': MSB (5, 6, 1, FALSE, 32);	 /* (1 .. 32), 32-bit op */
	case 'D': SPECIAL (4, 16, SAVE_RESTORE_FP_LIST);
	case 'E': BIT (5, 0, 32);		 /* (32 .. 63) */
	case 'F': MSB (5, 6, 33, TRUE, 64);	 /* (33 .. 64), 64-bit op */
	case 'G': MSB (5, 6, 33, FALSE, 64);	 /* (33 .. 64), 64-bit op */
	case 'H': MSB (5, 6, 1, FALSE, 64);	 /* (1 .. 32), 64-bit op */
	case 'I': BIT (5, 6, 0); /* (0 .. 31) */
	case 'J': HINT (19, 0);
	case 'K': SPECIAL_SPLIT (20, 2, 1, 0, HI20_PCREL); /* tri-part 20-bit */
	case 'L': HINT (10, 16);
	case 'M': HINT (18, 0);
	case 'N': INT_ADJ (9, 3, 511, 3, FALSE);	/* 9-bit << 3 */
	case 'O': SPECIAL_WORD (0, GPREL_WORD);
	case 'P': SPECIAL_WORD (6, IMM_WORD);
	case 'Q': SPECIAL_WORD (0, UINT_WORD);
	case 'R': SPECIAL_WORD (0, INT_WORD);
	case 'S': SPECIAL_WORD (0, PC_WORD);

	case 'i': HINT (5, 16);
	case 'j': SINT_SPLIT (9, 0, 0, 1, 15, 0);
	case 'p': SINT_SPLIT (7, 2, 2, 1, 15, 0); /* split 7-bit signed << 2 */
	case 'q': SINT_SPLIT (6, 3, 3, 1, 15, 0); /* split 6-bit signed << 3 */
	case 'r': BRANCH_UNORD_SPLIT (21, 1); /* split 21-bit signed << 1 */
	case 's': IMM_SINT_SPLIT (21, 1, 1, 1, 0, 2); /* split (21-bit signed + 2) << 1 */
	case 't': SPECIAL (5, 21, NON_ZERO_REG);
	case 'u': BRANCH_UNORD_SPLIT (25, 1);
	case 'v': MAPPED_INT (5, 6, word_byte_map, FALSE);
	case 'w': BIT (2, 9, 0);		/* (0 .. 3) */

	case '*': INT_ADJ (4, 7, 15, 1, FALSE); /* (0 .. 15) << 1 */
	case '|': UINT (1, 6);			/* 0/1 */
	case ';': UINT (2, 21);			/* (0 .. 3) */
	case '1': UINT (18, 0);
	case '2': INT_ADJ (16, 2, (1<<16) - 1, 2, FALSE);
	case '3': INT_ADJ (17, 1, (1<<17) - 1, 1, FALSE);
	case '4': INT_ADJ (18, 3, (1<<18)-1, 3, FALSE); /* 18-bit << 3 */
	case '5': SPLIT_MAPPED_REG (4, 21, 1, 25, GP, reg_4to5_srcmap);
	case '6': MAPPED_INT (5, 6, int_mask_map, TRUE);
	case '7': MAPPED_REG (1, 24, GP, reg_4or5_map);
	case '8': HINT (23, 3);
	case '9': UINT (7, 11);
	}
      break;

    case '.': INT_ADJ (19, 2, (1<<19) - 1, 2, FALSE);
    case '<': BIT (5, 0, 0);			 /* (0 .. 31) */
/*     case '>': BIT (5, 11, 32);			 /\* (32 .. 63) *\/ */
    case '\\': BIT (3, 21, 0);			 /* (0 .. 7) */
    case '|': INT_ADJ (3, 12, 8, 0, FALSE);	/* 1 .. 8 */
    case '~': BRANCH_UNORD_SPLIT (11, 1);	/* split 11-bit signed << 1 */
    case '@': SINT (10, 11);
    case '^': HINT (5, 11);
    case '!': UINT (1, 10);
    case '$': UINT (1, 3);
    case '*': REG (2, 18, ACC);
/*     case '&': REG (2, 23, ACC); */

    case '0': SINT (6, 16);
    case '1': BIT (5, 11, 0);			 /* (0 .. 31) */
    case '2': BIT (5, 16, 0);		/* (0 .. 31) */
    case '3': BIT (3, 13, 0);
    case '4': BIT (4, 12, 0);
    case '5': HINT (8, 13);
    case '7': REG (2, 14, ACC);
    case '8': HINT (7, 14);

    case 'C': HINT (23, 3);
    case 'D': REG (5, 11, FP);
    case 'E': REG (5, 21, COPRO);
    case 'G': REG (5, 16, COPRO);
    case 'H': UINT (5, 11);
    case 'J': SPECIAL (5, 11, CP0SEL);
    case 'K': REG (10, 11, HW);
/*     case 'M': REG (3, 13, CCC); */
/*     case 'N': REG (3, 18, CCC); */
    case 'O': REG (10, 11, CP0);
    case 'P': REG (5, 16, CP0SEL);
/*     case 'Q': UINT (5, 11); */
    case 'R': REG (5, 11, FP);
    case 'S': REG (5, 16, FP);
    case 'T': REG (5, 21, FP);
    case 'U': REG (5, 16, HWRSEL);
    case 'V': OPTIONAL_REG (5, 16, FP);

/*     case 'a': JUMP (26, 0, 1); */
    case 'b': REG (5, 16, GP);
    case 'c': BASE_OFFSET_CHECK (5, 16, TRUE, FALSE);
    case 'd': REG (5, 11, GP);
    case 'e': REG (5, 3, GP);
    case 'g': HINT (12, 0);
    case 'h': SPECIAL (12, 0, NEG_INT);
    case 'i': UINT (12, 0);
    case 'j': UINT (16, 0);
    case 'k': HINT (5, 21);
    case 'n': SPECIAL_SPLIT (11, 2, 10, 16, SAVE_RESTORE_LIST);
    case 'o': UINT (12, 0);
    case 'p': BRANCH_UNORD_SPLIT (14, 1);
/*     case 'q': BRANCH_UNORD_SPLIT (20, 1); - unused */
    case 'r': OPTIONAL_REG (5, 16, GP);
    case 's': REG (5, 16, GP);
    case 't': REG (5, 21, GP);
    case 'u': SPECIAL_SPLIT (20, 2, 1, 0, HI20_INT);  /* tri-part 20-bit */
    case 'v': OPTIONAL_REG (5, 16, GP);
    case 'w': OPTIONAL_REG (5, 21, GP);
    case 'x': SPECIAL_SPLIT (20, 2, 1, 0, HI20_SCALE);  /* tri-part 20-bit */
    case 'z': MAPPED_REG (0, 0, GP, reg_0_map);
    }
  return 0;
}

#define LM	INSN_LOAD_MEMORY
#define SM	INSN_STORE_MEMORY

#define WR_1	INSN_WRITE_1
#define WR_2	INSN_WRITE_2
#define RD_1	INSN_READ_1
#define RD_2	INSN_READ_2
#define RD_3	INSN_READ_3
#define MOD_1	(WR_1|RD_1)
#define MOD_2	(WR_2|RD_2)
#define FP_S	INSN_FP_S
#define FP_D	INSN_FP_D

 /* Write dsp accumulators */
#define WR_a	INSN_WRITE_ACC
 /* Read dsp accumulators */
#define RD_a	INSN_READ_ACC

/* Flags used in pinfo2.  */
#define CTC	INSN2_CONVERTED_TO_COMPACT
#define UBR	INSN2_UNCOND_BRANCH
#define CBR	INSN2_COND_BRANCH
#define ALIAS	INSN2_ALIAS

/* For 32-bit nanoMIPS instructions.  */
#define WR_31	INSN_WRITE_GPR_31

/* nanoMIPS DSP ASE support.  */
#define D32	ASE_DSP

/* nanoMIPS MT ASE support.  */
#define MT32	ASE_MT

/* nanoMIPS MCU (MicroController) ASE support.  */
#define MC	ASE_MCU

/* nanoMIPS Enhanced VA Scheme.  */
#define EVA	ASE_EVA

/* MSA support.  */
#define MSA     ASE_MSA
#define MSA64   ASE_MSA64

/* eXtended Physical Address (XPA) support.  */
#define XPA     ASE_XPA

/* Global INValidate extension.  */
#define GINV	ASE_GINV

/* Cyclic redundancy check instruction (CRC) support.  */
#define CRC	ASE_CRC

/* nanoMIPS instruction subset.  */
#define xNMS	ASE_xNMS
/* TLB manipulations.  */
#define TLB	ASE_TLB

/* Base ISA for nanoMIPS. */
#define I38	INSN_ISAN32R6
#define I70	INSN_ISAN64R6

const struct nanomips_opcode nanomips_opcodes[] = {
/* These instructions appear first so that the disassembler will find
   them first.  The assemblers uses a hash table based on the
   instruction name anyhow.  */
/* name,	suffix,		args,		match,		mask,	     pinfo	pinfo2,	 membership, ase */
/* Pure macros */
{"la",		"",		"t,A(b)",	0,    (int) M_LA_AB,	INSN_MACRO,		0,	I38,	0},
{"dla", 	"",		"t,A(b)",	0,    (int) M_DLA_AB,	INSN_MACRO,		0,	I38,	0},
/* Precedence=1 */
{"aluipc",	"",		"t,+K", 	0xe0000002, 0xfc000002, WR_1,			0,	I38,	0}, /* ALUIPC */
{"break",	"[16]", 	"",		0x1010, 	0xffff,	0,		INSN2_ALIAS,	I38,	0},
{"break",	"[16]", 	"mK",		0x1010, 	0xfff8,	0,			0,	I38,	0},
{"break",	"[32]", 	"",		0x00100000, 0xffffffff,		0,	INSN2_ALIAS,	I38,	0},
{"break",	"[32]", 	"+J",		0x00100000, 0xfff80000,		0,		0,	I38,	0},
{"dvp", 	"",		"-A",		0x20000390, 0xffe0ffff,		0,	INSN2_ALIAS,	I38,	0}, /* DVP */
{"dvp", 	"",		"t,-A", 	0x20000390, 0xfc00ffff,		WR_1,		0,	I38,	0},
{"nop", 	"[16]", 	"",		0x9008,		0xffff,		0,		0,	I38,	0}, /* NOP[16] */
{"nop", 	"[32]", 	"",		0x8000c000, 0xffffffff,		0,		0,	I38,	0}, /* NOP */
{"sdbbp",	"[16]", 	"",		0x1018,		0xffff,		0,	INSN2_ALIAS,	I38,	0},
{"sdbbp",	"[16]", 	"mK",		0x1018,		0xfff8,		0,		0,	I38,	0},
{"sdbbp",	"[32]", 	"",		0x00180000, 0xffffffff,		0,	INSN2_ALIAS,	I38,	0},
{"sdbbp",	"[32]", 	"+J",		0x00180000, 0xfff80000,		0,		0,	I38,	0},
{"move",	"",		"mb,mj",	0x1000,		0xfc00,	WR_1|RD_2,		0,	I38,	0}, /* preceded by BREAK, SDBBP */
{"move",	"",		"d,s",		0x20000290, 0xffe007ff, WR_1|RD_2,	INSN2_ALIAS,	I38,	0}, /* OR */
{"move",	"",		"d,s",		0x20000150, 0xffe007ff, WR_1|RD_2,	INSN2_ALIAS,	I38,	0}, /* ADDU */
{"sigrie",	"",		"+J",		0x00000000, 0xfff80000,	0,			0,	I38,	0},
{"synci",	"[u12]",	"o(b)", 	0x87e03000, 0xffe0f000,	RD_2,			0,	I38,	0}, /* SYNCI[U12] */
{"synci",	"[s9]", 	"+j(b)",	0xa7e01800, 0xffe07f00,	RD_2,			0,	I38,	0}, /* SYNCI[S9] */
{"syncie",	"",		"+j(b)",	0xa7e01a00, 0xffe07f00,	RD_2,			0,	0,	EVA},
{"jrc", 	"[16]", 	"mp",		0xd800, 	0xfc1f,	RD_1,			0,	I38,	0}, /* JRC[16] */
{"jrc", 	"[32]", 	"s",		0x48000000, 0xffe0ffff,	RD_1,		INSN2_ALIAS,	I38,	0}, /* JALRC */
{"jalrc",	"[16]", 	"mp",		0xd810, 	0xfc1f,	WR_31|RD_1,		0,	I38,	0}, /* JALRC[16] */
{"jalrc",	"[16]", 	"my,mp",	0xd810, 	0xfc1f,	WR_31|RD_1,	INSN2_ALIAS,	I38,	0}, /* JALRC[16] */
{"jalrc",	"[32]", 	"s",		0x4be00000, 0xffe0ffff,	RD_1,		INSN2_ALIAS,	I38,	0}, /* JALRC[32] */
{"jalrc",	"[32]", 	"t,s,-C",	0x48000000, 0xfc00f000,	WR_1|RD_2,		0,	I38,	0},
{"jalrc",	"",		"mp,-i",	0xd810, 	0xfc1f,	WR_31|RD_1,	INSN2_ALIAS,	I38,	0}, /* JALRC[16] */
{"jalrc",	"",		"s,-i", 	0x4be00000, 0xffe0ffff,	RD_1,		INSN2_ALIAS,	I38,	0}, /* JALRC[32] */
{"jr",		"",		"mp",		0xd800, 	0xfc1f,	RD_1,	    INSN2_ALIAS|UBR|CTC, I38,	0}, /* JRC */
{"jr",		"",		"s",		0x48000000, 0xffe0ffff,	RD_1,	    INSN2_ALIAS|UBR|CTC, I38,	0}, /* JALRC */
{"jalr",	"",		"my,mp",	0xd810, 	0xfc1f,	WR_31|RD_1, INSN2_ALIAS|UBR|CTC, I38,	0}, /* JALRC[16] */
{"jalr",	"",		"mp",		0xd810, 	0xfc1f,	WR_31|RD_1, INSN2_ALIAS|UBR|CTC, I38,	0}, /* JALRC[16] */
{"jalr",	"",		"s",		0x4be00000, 0xffe0ffff,	RD_1,	    INSN2_ALIAS|UBR|CTC, I38,	0}, /* JALRC */
{"jalr",	"",		"t,s",		0x48000000, 0xfc00ffff,	WR_1|RD_2,  INSN2_ALIAS|UBR|CTC, I38,	0}, /* JALRC */
{"lui", 	"",		"t,u",		0xe0000000, 0xfc000002,	WR_1,			0,	I38,	0},
{"li",		"[16]", 	"md,mI",	0xd000, 	0xfc00,	WR_1,			0,	I38,	0}, /* LI[16] */
{"li",		"",		"mb,mZ",	0x1000, 	0xfc1f,	WR_1,		INSN2_ALIAS,	I38,	0}, /* MOVE[16] */
{"li",		"[32]", 	"+t,j", 	0x00000000, 0xfc1f0000,	WR_1,		INSN2_ALIAS,	I38,	0}, /* ADDIU[32] */
{"li",		"[neg]",	 "t,h", 	0x80008000, 0xfc1ff000,	WR_1,		INSN2_ALIAS,	I38,	0}, /* ADDIU[NEG] */
{"li",		"",		"t,x",		0xe0000000, 0xfc000002,	WR_1,		INSN2_ALIAS,	I38,	0}, /* LUI */
{"li",		"[48]", 	"mp,+Q",	0x6000, 	0xfc1f,	WR_1,			0,	0,	xNMS}, /* LI[48] */
{"li",		"",		"+t,A", 	0,		(int) M_LI,	INSN_MACRO,	INSN2_MACRO,	I38,	0},
{"ext", 	"",		"t,r,+A,+C",	0x8000f000, 0xfc00f820,	WR_1|RD_2,		0,	0,	xNMS},
{"ext", 	"",		"t,r,+A,+C",	0,	   (int) M_EXT,	INSN_MACRO,		0,	I38,	0},

/* Precedence=0 */
{"abs", 	"",		"d,v",		0,	   (int) M_ABS,	INSN_MACRO,		0,	I38,	0},
{"absq_s.ph",	"",		"t,s",		0x2000113f, 0xfc00ffff,	WR_1|RD_2,		0,	0,	D32},
{"absq_s.qb",	"",		"t,s",		0x2000013f, 0xfc00ffff,	WR_1|RD_2,		0,	0,	D32},
{"absq_s.w",	"",		"t,s",		0x2000213f, 0xfc00ffff,	WR_1|RD_2,		0,	0,	D32},
{"add", 	"",		"d,v,t,-B",	0x20000110, 0xfc0003ff, WR_1|RD_2|RD_3,		0,	0,	xNMS},
{"add", 	"",		"t,r,I",	0,    (int) M_ADD_I,	INSN_MACRO,		0,	0,	xNMS},
{"addi",	"",		"t,r,I",	0,    (int) M_ADD_I,	INSN_MACRO,		0,	0,	xNMS},
{"addiu",	"[r1.sp]",	"md,ms,mS",	0x7040, 	0xfc40,	WR_1|RD_2,		0,	I38,	0}, /* ADDIU[R1.SP] */
{"addiu",	"[r2]", 	"md,mc,mB",	0x9000, 	0xfc08,	WR_1|RD_2,		0,	I38,	0}, /* ADDIU[R2] */
{"addiu",	"[rs5]",	"mp,mk,mQ",	0x9008, 	0xfc08,	MOD_1,		INSN2_ALIAS,	I38,	0}, /* ADDIU[RS5] */
{"addiu",	"[rs5]",	"mp,mQ",	0x9008, 	0xfc08,	MOD_1,			0,	I38,	0}, /* ADDIU[RS5], preceded by NOP[16] */
{"addiu",	"[gp.b]",	"t,ma,+1",	0x440c0000, 0xfc1c0000,	WR_1|RD_2,		0,	I38,	0}, /* ADDIU[GP.B] */
{"addiu",	"[gp.w]",	"t,ma,.",	0x40000000, 0xfc000003,	WR_1|RD_2,		0,	I38,	0}, /* ADDIU[GP.W] */
{"addiu",	"[32]", 	"+t,r,j",	0x00000000, 0xfc000000,	WR_1|RD_2,		0,	I38,	0}, /* preceded by SIGRIE */
{"addiu",	"[neg]",	"t,r,h",	0x80008000, 0xfc00f000,	WR_1|RD_2,		0,	I38,	0}, /* ADDIU[NEG] */
{"addiu",	"[48]", 	"mp,mt,+R",	0x6001, 	0xfc1f,	MOD_1,			0,	0,	xNMS}, /* ADDIU[48] */
{"addiu",	"[gp48]",	"mp,ma,+O",	0x6002, 	0xfc1f,	WR_1|RD_2,		0,	0,	xNMS}, /* ADDIU[GP48] */
{"lapc",	"[32]", 	"t,+r", 	0x04000000, 0xfc000000, WR_1,		INSN2_ALIAS,	I38,	0}, /* ADDIUPC */
{"lapc",	"[48]", 	"mp,+S",	0x6003, 	0xfc1f,	WR_1,		INSN2_ALIAS,	0,	xNMS}, /* ADDIUPC[48] */
{"lapc.h",	"",		"t,+r", 	0x04000000, 0xfc000000, WR_1,		INSN2_ALIAS,	I38,	0}, /* ADDIUPC */
{"lapc.b",	"",		"mp,+S",	0x6003, 	0xfc1f,	WR_1,		INSN2_ALIAS,	0,	xNMS}, /* ADDIUPC[48] */
{"addiupc",	"[32]", 	"t,+s", 	0x04000000, 0xfc000000, WR_1,			0,	I38,	0},
{"addiupc",	"[48]", 	"mp,+P",	0x6003, 	0xfc1f,	WR_1,			0,	0,	xNMS}, /* ADDIUPC[48] */
{"addiu.b",	"",		"t,ma,+1",	0x440c0000, 0xfc1c0000,	WR_1|RD_2,	INSN2_ALIAS,	I38,	0}, /* ADDIU[GP.B] */
{"addiu.w",	"",		"t,ma,.",	0x40000000, 0xfc000003,	WR_1|RD_2,	INSN2_ALIAS,	I38,	0}, /* ADDIU[GP.W] */
{"addiu.b32",	"",		"mp,ma,+O",	0x6002, 	0xfc1f,	WR_1|RD_2,	INSN2_ALIAS,	0,	xNMS}, /* ADDIU[GP48] */
{"addq.ph",	"",		"d,s,t",	0x2000000d, 0xfc0007ff, WR_1|RD_2|RD_3,		0,	0,	D32},
{"addqh.ph",	"",		"d,s,t",	0x2000004d, 0xfc0007ff, WR_1|RD_2|RD_3,		0,	0,	D32},
{"addqh.w",	"",		"d,s,t",	0x2000008d, 0xfc0007ff, WR_1|RD_2|RD_3,		0,	0,	D32},
{"addqh_r.ph",	"",		"d,s,t",	0x2000044d, 0xfc0007ff, WR_1|RD_2|RD_3,		0,	0,	D32},
{"addqh_r.w",	"",		"d,s,t",	0x2000048d, 0xfc0007ff, WR_1|RD_2|RD_3,		0,	0,	D32},
{"addq_s.ph",	"",		"d,s,t",	0x2000040d, 0xfc0007ff, WR_1|RD_2|RD_3,		0,	0,	D32},
{"addq_s.w",	"",		"d,s,t,-B",	0x20000305, 0xfc0003ff, WR_1|RD_2|RD_3,		0,	0,	D32},
{"addsc",	"",		"d,s,t,-B",	0x20000385, 0xfc0003ff, WR_1|RD_2|RD_3,		0,	0,	D32},
{"addu",	"[16]", 	"me,mc,md",	0xb000,		0xfc01, WR_1|RD_2|RD_3,		0,	I38,	0}, /* ADDU[16] */
{"addu",	"[4x4]",	"mu,mt,mv",	0x3c00,		0xfd08,	MOD_1|RD_3,		0,	0,	xNMS}, /* ADDU[4X4] */
{"addu",	"[4x4]",	"mu,mv,mk",	0x3c00,		0xfd08,	MOD_1|RD_2,	INSN2_ALIAS,	0,	xNMS}, /* ADDU[4X4] */
{"addu",	"[32]", 	"d,v,t,-B",	0x20000150, 0xfc0003ff, WR_1|RD_2|RD_3,		0,	I38,	0},
{"addu",	"",		"t,r,I",	0,    (int) M_ADDU_I,	INSN_MACRO,		0,	I38,	0},
{"addu.ph",	"",		"d,s,t",	0x2000010d, 0xfc0007ff, WR_1|RD_2|RD_3,		0,	0,	D32},
{"addu.qb",	"",		"d,s,t",	0x200000cd, 0xfc0007ff, WR_1|RD_2|RD_3,		0,	0,	D32},
{"adduh.qb",	"",		"d,s,t",	0x2000014d, 0xfc0007ff, WR_1|RD_2|RD_3,		0,	0,	D32},
{"adduh_r.qb",	"",		"d,s,t",	0x2000054d, 0xfc0007ff, WR_1|RD_2|RD_3,		0,	0,	D32},
{"addu_s.ph",	"",		"d,s,t",	0x2000050d, 0xfc0007ff, WR_1|RD_2|RD_3,		0,	0,	D32},
{"addu_s.qb",	"",		"d,s,t",	0x200004cd, 0xfc0007ff, WR_1|RD_2|RD_3,		0,	0,	D32},
{"addwc",	"",		"d,s,t,-B",	0x200003c5, 0xfc0003ff, WR_1|RD_2|RD_3,		0,	0,	D32},
{"extw",	"",		"d,s,t,+I",	0x2000001f, 0xfc00003f, WR_1|RD_2|RD_3,		0,	I38,	0},
{"align",	"",		"mb,-i,mj,mz",	0x1000, 	0xfc00,	WR_1|RD_3,	INSN2_ALIAS,	I38,	0}, /* MOVE[16] */
{"align",	"",		"d,-i,s,mz",	0x20000290, 0xffe007ff, WR_1|RD_3,	INSN2_ALIAS,	I38,	0}, /* MOVE[32] */
{"align",	"",		"d,s,t,+v",	0x2000001f, 0xfc00003f, WR_1|RD_2|RD_3,	INSN2_ALIAS,	I38,	0}, /* EXTW */
{"and", 	"[16]", 	"md,mk,ml",	0x5008,		0xfc0f,	MOD_1|RD_3,	INSN2_ALIAS,	I38,	0}, /* AND[16] */
{"and", 	"[16]", 	"md,ml,mk",	0x5008,		0xfc0f,	MOD_1|RD_2,	INSN2_ALIAS,	I38,	0}, /* AND[16] */
{"and", 	"[16]", 	"md,ml",	0x5008,		0xfc0f,	MOD_1|RD_2,		0,	I38,	0}, /* AND[16] */
{"and", 	"[32]", 	"d,v,t,-B",	0x20000250, 0xfc0003ff, WR_1|RD_2|RD_3,		0,	I38,	0},
{"and", 	"[32]", 	"t,r,I",	0,    (int) M_AND_I,	INSN_MACRO,		0,	I38,	0},
{"andi",	"[16]", 	"md,mc,mC",	0xf000,		0xfc00,	WR_1|RD_2,		0,	I38,	0}, /* ANDI[16] */
{"andi",	"[32]", 	"t,r,g",	0x80002000, 0xfc00f000,	WR_1|RD_2,		0,	I38,	0},
{"andi",	"",		"t,r,+6",	0x8000f000, 0xfc00f83f,	WR_1|RD_2,	INSN2_ALIAS,	0,	xNMS}, /* EXT */
{"append",	"",		"t,s,1",	0x20000215, 0xfc0007ff,	WR_1|RD_2,		0,	0,	D32},
{"balc",	"[16]", 	"mD",		0x3800, 	0xfc00,		WR_31,		0,	I38,	0}, /* BALC[16] */
{"balc",	"[32]", 	"+u",		0x2a000000, 0xfe000000,		WR_31,		0,	I38,	0},
{"bal", 	"",		"mD",		0x3800, 	0xfc00,	WR_31,	INSN2_ALIAS|UBR|CTC,	I38,	0}, /* BALC[16] */
{"bal", 	"",		"+u",		0x2a000000, 0xfe000000,	WR_31,	INSN2_ALIAS|UBR|CTC,	I38,	0}, /* BALC */
{"balign",	"",		"d,-m,s,+v",	0x2000001f, 0xfc00003f, WR_1|RD_3,	INSN2_ALIAS,	0,	D32}, /* EXTW */
{"balrsc",	"",		"+t,s,-C", 	0x48008000, 0xfc00f000,	WR_1|RD_2,		0,	I38,	0},
{"balrsc",	"",		"s",		0x4be08000, 0xffe0ffff,	RD_1|WR_31,	INSN2_ALIAS,	I38,	0}, /* BALRSC */
{"bbeqzc",	"",		"t,1,~,-D",	0xc8040000, 0xfc1d0000,	RD_1,			0,	0,	xNMS},
{"bbnezc",	"",		"t,1,~,-D",	0xc8140000, 0xfc1d0000,	RD_1,			0,	0,	xNMS},
{"bc",		"[16]", 	"mD",		0x1800, 	0xfc00,	0,			0,	I38,	0}, /* BC[16] */
{"bc",		"[32]", 	"+u",		0x28000000, 0xfe000000,	0,			0,	I38,	0},
{"b",		"",		"mD",		0x1800, 	0xfc00,	0,	INSN2_ALIAS|UBR|CTC,	I38,	0}, /* BC[16] */
{"b",		"",		"+u",		0x28000000, 0xfe000000,	0,	INSN2_ALIAS|UBR|CTC,	I38,	0}, /* BC */
{"beqzc",	"[16]", 	"md,mE",	0x9800, 	0xfc00,	RD_1,			0,	I38,	0}, /* BEQZC[16] */
{"beqzc",	"[32]", 	"t,p",		0x88000000, 0xfc1fc000,	RD_1,		INSN2_ALIAS,	I38,	0}, /* BEQC */
{"beqzc",	"[32]", 	"s,p",		0x88000000, 0xffe0c000,	RD_1,		INSN2_ALIAS,	I38,	0}, /* BEQC */
{"beqz",	"",		"md,mE",	0x9800, 	0xfc00,	RD_1,	INSN2_ALIAS|CBR|CTC,	I38,	0}, /* BEQZC[16] */
{"beqz",	"",		"t,p",		0x88000000, 0xfc1fc000,	RD_1,	INSN2_ALIAS|CBR|CTC,	I38,	0}, /* BEQC */
{"beqz",	"",		"s,p",		0x88000000, 0xffe0c000,	RD_1,	INSN2_ALIAS|CBR|CTC,	I38,	0}, /* BEQC */
{"beqc",	"[16]", 	"md,z,mE",	0x9800, 	0xfc00,	RD_1,		INSN2_ALIAS,	I38,	0}, /* BEQZC[16] */
{"beqc",	"[16]", 	"z,md,mE",	0x9800, 	0xfc00,	RD_1,		INSN2_ALIAS,	I38,	0}, /* BEQZC[16] */
{"beqc",	"[16]", 	"ml,mf,mF",	0xd800, 	0xfc00,	RD_1|RD_2,		0,	0,	xNMS}, /* BEQC[16], with rs3<rt3 && u[4:1]!=0 */
{"beqc",	"[16]", 	"md,mg,mF",	0xd800, 	0xfc00,	RD_1|RD_2,	INSN2_ALIAS,	0,	xNMS}, /* BEQC[16], with operands commutated */
{"beqc",	"[32]", 	"s,t,p",	0x88000000, 0xfc00c000,	RD_1|RD_2,		0,	I38,	0},
{"beq", 	"",		"md,z,mE",	0x9800, 	0xfc00,	RD_1,	INSN2_ALIAS|CBR|CTC,	I38,	0}, /* BEQZC[16] */
{"beq", 	"",		"z,md,mE",	0x9800, 	0xfc00,	RD_1,	INSN2_ALIAS|CBR|CTC,	I38,	0}, /* BEQZC[16] */
{"beq", 	"",		"ml,mf,mF",	0xd800, 	0xfc00,	RD_1|RD_2, INSN2_ALIAS|CBR|CTC,	0,	xNMS}, /* BEQC[16], with rs3<rt3 && u[4:1]!=0 */
{"beq", 	"",		"md,mg,mF",	0xd800, 	0xfc00,	RD_1|RD_2, INSN2_ALIAS|CBR|CTC,	0,	xNMS}, /* BEQC[16], with operands commutated */
{"beq", 	"",		"s,t,p",	0x88000000, 0xfc00c000,	RD_1|RD_2, INSN2_ALIAS|CBR|CTC,	I38,	0}, /* BEQC */
{"beq", 	"",		"s,I,p",	0,    (int) M_BEQ_I,	INSN_MACRO,		0,	I38,	0},
{"beqic",	"",		"t,+9,~",	0xc8000000, 0xfc1c0000,	RD_1,			0,	I38,	0},
{"blezc",	"",		"t,p",		0x88008000, 0xfc1fc000,	RD_1,		INSN2_ALIAS,	I38,	0}, /* BGEC $0, t */
{"blez",	"",		"t,p",		0x88008000, 0xfc1fc000,	RD_1,	INSN2_ALIAS|CBR|CTC,	I38,	0}, /* BGEC $0, t */
{"bgezc",	"",		"s,p",		0x88008000, 0xffe0c000,	RD_1,		INSN2_ALIAS,	I38,	0}, /* BGEC s, $0 */
{"bgez",	"",		"s,p",		0x88008000, 0xffe0c000,	RD_1,	INSN2_ALIAS|CBR|CTC,	I38,	0}, /* BGEC s, $0 */
{"bgec",	"",		"s,t,p",	0x88008000, 0xfc00c000,	RD_1|RD_2,		0,	I38,	0},
{"bge", 	"",		"s,t,p",	0x88008000, 0xfc00c000,	RD_1|RD_2, INSN2_ALIAS|CBR|CTC,	I38,	0}, /* BGEC */
{"bge", 	"",		"s,I,p",	0,    (int) M_BGE_I,	INSN_MACRO,		0,	I38,	0},
{"bgt", 	"",		"s,t,p",	0,    (int) M_BGT,	INSN_MACRO,		0,	I38,	0},
{"bgt", 	"",		"s,I,p",	0,    (int) M_BGT_I,	INSN_MACRO,		0,	I38,	0},
{"bgtu",	"",		"s,t,p",	0,    (int) M_BGTU,	INSN_MACRO,		0,	I38,	0},
{"bgtu",	"",		"s,I,p",	0,    (int) M_BGTU_I,	INSN_MACRO,		0,	I38,	0},
{"ble", 	"",		"s,t,p",	0,    (int) M_BLE,	INSN_MACRO,		0,	I38,	0},
{"ble", 	"",		"s,I,p",	0,    (int) M_BLE_I,	INSN_MACRO,		0,	I38,	0},
{"bleu",	"",		"s,t,p",	0,    (int) M_BLEU,	INSN_MACRO,		0,	I38,	0},
{"bleu",	"",		"s,I,p",	0,    (int) M_BLEU_I,	INSN_MACRO,		0,	I38,	0},
{"bgezal",	"",		"s,p",		0,    (int) M_BGEZAL,	INSN_MACRO,		0,	I38,	0},
{"bgeic",	"",		"t,+9,~",	0xc8080000, 0xfc1c0000,	RD_1,			0,	I38,	0},
{"bgeuc",	"",		"s,t,p",	0x8800c000, 0xfc00c000,	RD_1|RD_2,		0,	I38,	0},
{"bgeu",	"",		"s,t,p",	0x8800c000, 0xfc00c000,	RD_1|RD_2, INSN2_ALIAS|CBR|CTC,	I38,	0}, /* BGEUC */
{"bgeu",	"",		"s,I,p",	0,    (int) M_BGEU_I,	INSN_MACRO,		0,	I38,	0},
{"bgeiuc",	"",		"t,+9,~",	0xc80c0000, 0xfc1c0000,	RD_1,			0,	I38,	0},
{"bitrev",	"",		"t,r",		0x2000313f, 0xfc00ffff,	WR_1|RD_2,		0,	0,	D32}, /* ROTX t,s,7,8,1 */
{"bitrevb",	"",		"t,r",		0x8000d247, 0xfc00ffff,	WR_1|RD_2,	INSN2_ALIAS,	0,	xNMS}, /* ROTX t,s,7,8,1 */
{"bitrevh",	"",		"t,r",		0x8000d40f, 0xfc00ffff,	WR_1|RD_2,	INSN2_ALIAS,	0,	xNMS}, /* ROTX t,s,15,16 */
{"bitrevw",	"",		"t,r",		0x8000d01f, 0xfc00ffff,	WR_1|RD_2,	INSN2_ALIAS,	0,	xNMS}, /* ROTX t,s,31,0*/
{"bitswap",	"",		"t,r",		0x8000d247, 0xfc00ffff,	WR_1|RD_2,	INSN2_ALIAS,	0,	xNMS}, /* ROTX t,s,7,8,1*/
{"bgtzc",	"",		"t,p",		0xa8008000, 0xfc1fc000,	RD_1,		INSN2_ALIAS,	I38,	0}, /* BLTC $0, t */
{"bgtz",	"",		"t,p",		0xa8008000, 0xfc1fc000,	RD_1, 	INSN2_ALIAS|CBR|CTC,	I38,	0}, /* BLTC $0, t */
{"bltzc",	"",		"s,p",		0xa8008000, 0xffe0c000,	RD_1,		INSN2_ALIAS,	I38,	0}, /* BLTC s, $0 */
{"bltz",	"",		"s,p",		0xa8008000, 0xffe0c000,	RD_1,	INSN2_ALIAS|CBR|CTC,	I38,	0}, /* BLTC s, $0 */
{"bltc",	"",		"s,t,p",	0xa8008000, 0xfc00c000,	RD_1|RD_2,		0,	I38,	0},
{"blt", 	"",		"s,t,p",	0xa8008000, 0xfc00c000,	RD_1|RD_2, INSN2_ALIAS|CBR|CTC,	I38,	0}, /* BLTC */
{"blt", 	"",		"s,I,p",	0,    (int) M_BLT_I,	INSN_MACRO,		0,	I38,	0},
{"bltzal",	"",		"s,p",		0,    (int) M_BLTZAL,	INSN_MACRO,		0,	I38,	0},
{"bltic",	"",		"t,+9,~",	0xc8180000, 0xfc1c0000,		RD_1,		0,	I38,	0},
{"bltuc",	"",		"s,t,p",	0xa800c000, 0xfc00c000,	RD_1|RD_2,		0,	I38,	0},
{"bltu",	"",		"s,t,p",	0xa800c000, 0xfc00c000,	RD_1|RD_2, INSN2_ALIAS|CBR|CTC,	I38,	0}, /* BLTUC */
{"bltu",	"",		"s,I,p",	0,    (int) M_BLTU_I,	INSN_MACRO,		0,	I38,	0},
{"bltiuc",	"",		"t,+9,~",	0xc81c0000, 0xfc1c0000, RD_1,			0,	I38,	0},
{"bnezc",	"[16]", 	"md,mE",	0xb800, 	0xfc00, RD_1,			0,	I38,	0}, /* BNEZC[16] */
{"bnezc",	"[32]", 	"t,p",		0xa8000000, 0xfc1fc000, RD_1,		INSN2_ALIAS,	I38,	0}, /* BNEC */
{"bnezc",	"[32]", 	"s,p",		0xa8000000, 0xffe0c000, RD_1,		INSN2_ALIAS,	I38,	0}, /* BNEC */
{"bnez",	"",		"md,mE",	0xb800, 	0xfc00, RD_1,	INSN2_ALIAS|CBR|CTC,	I38,	0}, /* BNEZC[16] */
{"bnez",	"",		"t,p",		0xa8000000, 0xfc1fc000, RD_1,	INSN2_ALIAS|CBR|CTC,	I38,	0}, /* BNEC */
{"bnez",	"",		"s,p",		0xa8000000, 0xffe0c000,	RD_1,		INSN2_ALIAS,	I38,	0}, /* BNEC */
{"bnec",	"[16]", 	"md,z,mE",	0xb800, 	0xfc00,	RD_1,		INSN2_ALIAS,	I38,	0}, /* BNEZC[16] */
{"bnec",	"[16]", 	"z,md,mE",	0xb800, 	0xfc00,	RD_2,		INSN2_ALIAS,	I38,	0}, /* BNEZC[16] */
{"bnec",	"[16]", 	"ml,mh,mF",	0xd800, 	0xfc00,	RD_1|RD_2,		0,	0,	xNMS}, /* BNEC[16], with rs3>=rt3 && u[4:1]!=0 */
{"bnec",	"[16]", 	"md,mi,mF",	0xd800, 	0xfc00,	RD_1|RD_2,	INSN2_ALIAS,	0,	xNMS}, /* BNEC[16], with operands commutated */
{"bnec",	"[32]", 	"s,t,p",	0xa8000000, 0xfc00c000,	RD_1|RD_2,		0,	I38,	0},
{"bne", 	"",		"md,z,mE",	0xb800, 	0xfc00,	RD_1, INSN2_ALIAS|CBR|CTC,	I38,	0}, /* BNEZC[16] */
{"bne", 	"",		"z,md,mE",	0xb800, 	0xfc00,	RD_2, INSN2_ALIAS|CBR|CTC,	I38,	0}, /* BNEZC[16] */
{"bne", 	"",		"ml,mh,mF",	0xd800, 	0xfc00,	RD_1|RD_2, INSN2_ALIAS|CBR|CTC,	0,	xNMS}, /* BNEC[16], with rs3>=rt3 && u[4:1]!=0 */
{"bne", 	"",		"md,mi,mF",	0xd800, 	0xfc00,	RD_1|RD_2, INSN2_ALIAS|CBR|CTC,	0,	xNMS}, /* BNEC[16], with operands commutated */
{"bne", 	"",		"s,t,p",	0xa8000000, 0xfc00c000,	RD_1|RD_2, INSN2_ALIAS|CBR|CTC,	I38,	0}, /* BNEC */
{"bne", 	"",		"s,I,p",	0,    (int) M_BNE_I,	INSN_MACRO,		0,	I38,	0},
{"bneic",	"",		"t,+9,~",	0xc8100000, 0xfc1c0000,	RD_1,			0,	I38,	0},
{"bposge32c",	"",		"p,-I",		0x88044000, 0xfc1fc000,	0,			0,	0,	D32},
{"bposge32",	"",		"p",		0x88044000, 0xffffc000,	0,	INSN2_ALIAS|CBR|CTC,	0,	D32}, /* BPOSGE32C */
{"brsc",	"",		"s,-C",		0x48008000, 0xffe0f000,	RD_1,			0,	I38,	0},
{"byterevh",	"",		"t,r",		0x8000d608, 0xfc00ffff,	WR_1|RD_2,	INSN2_ALIAS,	0,	xNMS}, /* ROTX t,s,8,24 */
{"byterevw",	"",		"t,r",		0x8000d218, 0xfc00ffff,	WR_1|RD_2,	INSN2_ALIAS,	0,	xNMS}, /* ROTX t,s,24,8 */
{"cache",	"",		"k,+j(b)",	0xa4003900, 0xfc007f00,	RD_3,			0,	I38,	0},
{"cache",	"",		"k,A(c)",	0,    (int) M_CACHE_AC,	INSN_MACRO,		0,	I38,	0},
{"cachee",	"",		"k,+j(b)",	0xa4003a00, 0xfc007f00,	RD_3,			0,	0,	EVA},
{"cachee",	"",		"k,A(c)",	0,    (int) M_CACHEE_AC, INSN_MACRO,		0,	0,	EVA},
{"cfc1",	"",		"t,G",		0xa000103b, 0xfc00ffff,	WR_1|RD_2,	INSN2_ALIAS,	I38,	0},
{"cfc1",	"",		"t,S",		0xa000103b, 0xfc00ffff,	WR_1|RD_2,		0,	I38,	0},
{"cftc1",	"",		"t,G",		0x20001e30, 0xfc00ffff,	WR_1|RD_2,	INSN2_ALIAS,	0,	MT32}, /* MFTR */
{"cftc1",	"",		"t,S",		0x20001e30, 0xfc00ffff,	WR_1|RD_2,	INSN2_ALIAS,	0,	MT32}, /* MFTR */
{"cftc2",	"",		"t,G",		0x20002e30, 0xfc00ffff,	WR_1|RD_2,	INSN2_ALIAS,	0,	MT32}, /* MFTR */
{"clo", 	"",		"t,s",		0x20004b3f, 0xfc00ffff,	WR_1|RD_2,		0,	0,	xNMS},
{"clz", 	"",		"t,s",		0x20005b3f, 0xfc00ffff,	WR_1|RD_2,		0,	0,	xNMS},
{"cmp.eq.ph",	"",		"s,t,-N",	0x20000005, 0xfc0003ff,	WR_1|RD_2,		0,	0,	D32},
{"cmp.le.ph",	"",		"s,t,-N",	0x20000085, 0xfc0003ff,	WR_1|RD_2,		0,	0,	D32},
{"cmp.lt.ph",	"",		"s,t,-N",	0x20000045, 0xfc0003ff,	WR_1|RD_2,		0,	0,	D32},
{"cmpgdu.eq.qb", "",		"d,s,t,-B",	0x20000185, 0xfc0003ff, WR_1|RD_2|RD_3,		0,	0,	D32},
{"cmpgdu.le.qb", "",		"d,s,t,-B",	0x20000205, 0xfc0003ff, WR_1|RD_2|RD_3,		0,	0,	D32},
{"cmpgdu.lt.qb", "",		"d,s,t,-B",	0x200001c5, 0xfc0003ff, WR_1|RD_2|RD_3,		0,	0,	D32},
{"cmpgu.eq.qb", "",		"d,s,t,-B",	0x200000c5, 0xfc0003ff, WR_1|RD_2|RD_3,		0,	0,	D32},
{"cmpgu.le.qb", "",		"d,s,t,-B",	0x20000145, 0xfc0003ff, WR_1|RD_2|RD_3,		0,	0,	D32},
{"cmpgu.lt.qb", "",		"d,s,t,-B",	0x20000105, 0xfc0003ff, WR_1|RD_2|RD_3,		0,	0,	D32},
{"cmpu.eq.qb",	"",		"s,t,-N",	0x20000245, 0xfc0003ff,	WR_1|RD_2,		0,	0,	D32},
{"cmpu.le.qb",	"",		"s,t,-N",	0x200002c5, 0xfc0003ff,	WR_1|RD_2,		0,	0,	D32},
{"cmpu.lt.qb",	"",		"s,t,-N",	0x20000285, 0xfc0003ff,	WR_1|RD_2,		0,	0,	D32},
{"cop2_1",	"",		"C",		0x20000002, 0xfc000007,		0,		0,	I38,	0},
{"crc32b",	"",		"t,r,-E",	0x200003e8, 0xfc001fff,	WR_1|RD_2,		0,	0,	CRC},
{"crc32h",	"",		"t,r,-E",	0x200007e8, 0xfc001fff,	WR_1|RD_2,		0,	0,	CRC},
{"crc32w",	"",		"t,r,-E",	0x20000be8, 0xfc001fff,	WR_1|RD_2,		0,	0,	CRC},
{"crc32cb",	"",		"t,r,-E",	0x200013e8, 0xfc001fff,	WR_1|RD_2,		0,	0,	CRC},
{"crc32ch",	"",		"t,r,-E",	0x200017e8, 0xfc001fff,	WR_1|RD_2,		0,	0,	CRC},
{"crc32cw",	"",		"t,r,-E",	0x20001be8, 0xfc001fff,	WR_1|RD_2,		0,	0,	CRC},
{"ctc1",	"",		"t,G",		0xa000183b, 0xfc00ffff,	RD_1|WR_2,	INSN2_ALIAS,	I38,	0},
{"ctc1",	"",		"t,S",		0xa000183b, 0xfc00ffff,	RD_1|WR_2,		0,	I38,	0},
{"cttc1",	"",		"t,G",		0x20001e70, 0xfc00ffff,	RD_1|WR_2,	INSN2_ALIAS,	0,	MT32}, /* MTTR */
{"cttc1",	"",		"t,S",		0x20001e70, 0xfc00ffff,	RD_1|WR_2,	INSN2_ALIAS,	0,	MT32}, /* MTTR */
{"cttc2",	"",		"t,G",		0x20002e70, 0xfc00ffff,	RD_1|WR_2,	INSN2_ALIAS,	0,	MT32}, /* MTTR */
{"dabs",	"",		"d,v",		0,	  (int) M_DABS,	INSN_MACRO,		0,	I38,	0},
{"daddiu",	"[neg]",	"t,r,h",	0x80008000, 0xfc00f000,	WR_1|RD_2,		0,	I38,	0}, /* DADDIU[NEG] */
{"deret",	"",		"-F",		0x2000e37f, 0xfc00ffff,		0,		0,	I38,	0},
{"di",		"",		"",		0x2000477f, 0xffffffff,	0,		INSN2_ALIAS,	I38,	0},
{"di",		"",		"w,-A",		0x2000477f, 0xfc00ffff,	WR_1,			0,	I38,	0},
{"div", 	"",		"d,v,t,-B",	0x20000118, 0xfc0003ff, WR_1|RD_2|RD_3,		0,	I38,	0},
{"div", 	"",		"d,v,I",	0,    (int) M_DIV_I,	INSN_MACRO,		0,	I38,	0},
{"divu",	"",		"d,v,t,-B",	0x20000198, 0xfc0003ff, WR_1|RD_2|RD_3,		0,	I38,	0},
{"divu",	"",		"d,v,I",	0,    (int) M_DIVU_I,	INSN_MACRO,		0,	I38,	0},
{"dmt", 	"",		"",		0x20010ab0, 0xffffffff,	0,		INSN2_ALIAS,	0,	MT32},
{"dmt", 	"",		"t",		0x20010ab0, 0xfc1fffff,	WR_1,			0,	0,	MT32},
{"dpa.w.ph",	"",		"7,s,t",	0x200000bf, 0xfc003fff,	MOD_1|RD_2|RD_3,	0,	0,	D32},
{"dpaqx_s.w.ph", "",		"7,s,t",	0x200022bf, 0xfc003fff,	MOD_1|RD_2|RD_3,	0,	0,	D32},
{"dpaqx_sa.w.ph","",		"7,s,t",	0x200032bf, 0xfc003fff,	MOD_1|RD_2|RD_3,	0,	0,	D32},
{"dpaq_s.w.ph", "",		"7,s,t",	0x200002bf, 0xfc003fff,	MOD_1|RD_2|RD_3,	0,	0,	D32},
{"dpaq_sa.l.w", "",		"7,s,t",	0x200012bf, 0xfc003fff,	MOD_1|RD_2|RD_3,	0,	0,	D32},
{"dpau.h.qbl",	"",		"7,s,t",	0x200020bf, 0xfc003fff,	MOD_1|RD_2|RD_3,	0,	0,	D32},
{"dpau.h.qbr",	"",		"7,s,t",	0x200030bf, 0xfc003fff,	MOD_1|RD_2|RD_3,	0,	0,	D32},
{"dpax.w.ph",	"",		"7,s,t",	0x200010bf, 0xfc003fff,	MOD_1|RD_2|RD_3,	0,	0,	D32},
{"dps.w.ph",	"",		"7,s,t",	0x200004bf, 0xfc003fff,	MOD_1|RD_2|RD_3,	0,	0,	D32},
{"dpsqx_s.w.ph", "",		"7,s,t",	0x200026bf, 0xfc003fff,	MOD_1|RD_2|RD_3,	0,	0,	D32},
{"dpsqx_sa.w.ph", "",		"7,s,t",	0x200036bf, 0xfc003fff,	MOD_1|RD_2|RD_3,	0,	0,	D32},
{"dpsq_s.w.ph", "",		"7,s,t",	0x200006bf, 0xfc003fff,	MOD_1|RD_2|RD_3,	0,	0,	D32},
{"dpsq_sa.l.w", "",		"7,s,t",	0x200016bf, 0xfc003fff,	MOD_1|RD_2|RD_3,	0,	0,	D32},
{"dpsu.h.qbl",	"",		"7,s,t",	0x200024bf, 0xfc003fff,	MOD_1|RD_2|RD_3,	0,	0,	D32},
{"dpsu.h.qbr",	"",		"7,s,t",	0x200034bf, 0xfc003fff,	MOD_1|RD_2|RD_3,	0,	0,	D32},
{"dpsx.w.ph",	"",		"7,s,t",	0x200014bf, 0xfc003fff,	MOD_1|RD_2|RD_3,	0,	0,	D32},
{"dvpe",	"",		"",		0x20000ab0, 0xffffffff,	0,		INSN2_ALIAS,	0,	MT32}, /* DVPE */
{"dvpe",	"",		"t",		0x20000ab0, 0xfc1fffff,	WR_1,			0,	0,	MT32},
{"ehb", 	"",		"-G",		0x8000c003, 0xffe0f1ff,	0,			0,	I38,	0},
{"ei",		"",		"",		0x2000577f, 0xffffffff,	0,		INSN2_ALIAS,	I38,	0},
{"ei",		"",		"t,-A",		0x2000577f, 0xfc00ffff,	WR_1,			0,	I38,	0},
{"emt", 	"",		"",		0x20010eb0, 0xffffffff,	0,		INSN2_ALIAS,	0,	MT32},
{"emt", 	"",		"t",		0x20010eb0, 0xfc1fffff, WR_1,			0,	0,	MT32},
{"eret",	"",		"-H",		0x2000f37f, 0xfc01ffff,	0,			0,	I38,	0},
{"eretnc",	"",		"-H",		0x2001f37f, 0xfc01ffff,	0,			0,	I38,	0},
{"evpe",	"",		"",		0x20000eb0, 0xffffffff,	0,		INSN2_ALIAS,	0,	MT32}, /* EVPE */
{"evpe",	"",		"t",		0x20000eb0, 0xfc1fffff,	WR_1,			0,	0,	MT32},
{"evp", 	"",		"-A",		0x20000790, 0xffe0ffff,	0,		INSN2_ALIAS,	I38,	0}, /* EVP */
{"evp", 	"",		"t,-A",		0x20000790, 0xfc00ffff,	WR_1,			0,	I38,	0},
{"extp",	"",		"t,7,2",	0x2000267f, 0xfc003fff,	WR_1|RD_2,		0,	0,	D32},
{"extpdp",	"",		"t,7,2",	0x2000367f, 0xfc003fff,	WR_1|RD_2,		0,	0,	D32},
{"extpdpv",	"",		"t,7,s",	0x200038bf, 0xfc003fff, WR_1|RD_2|RD_3,		0,	0,	D32},
{"extpv",	"",		"t,7,s",	0x200028bf, 0xfc003fff, WR_1|RD_2|RD_3,		0,	0,	D32},
{"extr.w",	"",		"t,7,2",	0x20000e7f, 0xfc003fff,	WR_1|RD_2,		0,	0,	D32},
{"extrv.w",	"",		"t,7,s",	0x20000ebf, 0xfc003fff, WR_1|RD_2|RD_3,		0,	0,	D32},
{"extrv_r.w",	"",		"t,7,s",	0x20001ebf, 0xfc003fff, WR_1|RD_2|RD_3,		0,	0,	D32},
{"extrv_rs.w",	"",		"t,7,s",	0x20002ebf, 0xfc003fff, WR_1|RD_2|RD_3,		0,	0,	D32},
{"extrv_s.h",	"",		"t,7,s",	0x20003ebf, 0xfc003fff, WR_1|RD_2|RD_3,		0,	0,	D32},
{"extr_r.w",	"",		"t,7,2",	0x20001e7f, 0xfc003fff,	WR_1|RD_2,		0,	0,	D32},
{"extr_rs.w",	"",		"t,7,2",	0x20002e7f, 0xfc003fff,	WR_1|RD_2,		0,	0,	D32},
{"extr_s.h",	"",		"t,7,2",	0x20003e7f, 0xfc003fff,	WR_1|RD_2,		0,	0,	D32},
{"fork",	"",		"d,s,t,-B",	0x20000228, 0xfc0003ff,	WR_1|RD_2|RD_3,		0,	0,	MT32},
{"ginvi",	"",		"s,-I",		0x20001f7f, 0xfc00ffff, RD_1,			0,	0,	GINV},
{"ginvt",	"",		"s,+;,-J", 	0x20000f7f, 0xfc00ffff, RD_1,			0,	0,	GINV},
{"ins", 	"",		"t,r,+A,+B",	0x8000e000, 0xfc00f820,	WR_1|RD_2,		0,	0,	xNMS},
{"ins", 	"",		"t,r,+A,+B",	0,    (int) M_INS,	INSN_MACRO,		0,	I38,	0},
{"insv",	"",		"t,s",		0x2000413f, 0xfc00ffff,	WR_1|RD_2,		0,	0,	D32},
{"j",		"",		"mp",		0xd800, 	0xfc1f,	RD_1,	INSN2_ALIAS|UBR|CTC,	I38,	0},
{"j",		"",		"+u",		0x28000000, 0xfe000000,	0, 	INSN2_ALIAS|UBR|CTC,	I38,	0}, /* BC */
{"j",		"",		"s",		0x48000000, 0xffe0ffff,	RD_1,	INSN2_ALIAS|UBR|CTC,	I38,	0}, /* JALRC */
{"jrc.hb",	"",		"s",		0x48001000, 0xffe0ffff,	RD_1,		INSN2_ALIAS,	I38,	0}, /* JALRC.HB */
{"jr.hb",	"",		"s",		0x48001000, 0xffe0ffff,	RD_1,	INSN2_ALIAS|UBR|CTC,	I38,	0}, /* JALRC.HB */
{"jalrc.hb",	"",		"s",		0x4be01000, 0xffe0ffff,	RD_1,		INSN2_ALIAS,	I38,	0}, /* JALRC.HB */
{"jalrc.hb",	"",		"t,s,-C",	0x48001000, 0xfc00f000,	WR_1|RD_2,		0,	I38,	0},
{"jalr.hb",	"",		"s",		0x4be01000, 0xffe0ffff,	RD_1,	INSN2_ALIAS|UBR|CTC,	I38,	0}, /* JALRC.HB */
{"jalr.hb",	"",		"t,s",		0x48001000, 0xfc00ffff,	WR_1|RD_2, INSN2_ALIAS|UBR|CTC,	I38,	0}, /* JALRC.HB */
/* SVR4 PIC code requires special handling for jal, so it must be a macro.  */
{"jal", 	"",		"my,mp",	0xd810, 	0xfc1f,	WR_31|RD_1, INSN2_ALIAS|UBR|CTC, I38,	0}, /* JALRC[16] */
{"jal", 	"",		"mp",		0xd810, 	0xfc1f,	WR_31|RD_1, INSN2_ALIAS|UBR|CTC, I38,	0}, /* JALRC[16] */
{"jal", 	"",		"s",		0x4be00000, 0xffe0ffff,	RD_1,	INSN2_ALIAS|UBR|CTC,	I38,	0}, /* JALRC */
{"jal", 	"",		"t,s",		0x48000000, 0xfc00ffff,	WR_1|RD_2, INSN2_ALIAS|UBR|CTC, I38,	0}, /* JALRC */
{"jal", 	"",		"A",		0,    (int) M_JAL_A,	INSN_MACRO,		0,	I38,	0},
{"jal", 	"",		"+u",		0x2a000000, 0xfe000000,	WR_31,	INSN2_ALIAS|UBR|CTC,	I38,	0}, /* BALC */
{"lb",		"[16]", 	"md,mL(ml)",	0x5c00, 	0xfc0c,	WR_1|RD_3,		0,	I38,	0}, /* LB[16] */
{"lb",		"[gp]", 	"t,+1(ma)",	0x44000000, 0xfc1c0000,	WR_1|RD_3,		0,	I38,	0}, /* LB[GP] */
{"lb",		"[u12]",	"t,o(b)",	0x84000000, 0xfc00f000,	WR_1|RD_3,		0,	I38,	0}, /* LB[U12] */
{"lb",		"[s9]", 	"t,+j(b)",	0xa4000000, 0xfc007f00,	WR_1|RD_3,		0,	I38,	0}, /* LB[S9] */
{"lb",		"",		"t,A(c)",	0,	(int) M_LB_AC,	INSN_MACRO,		0,	I38,	0},
{"lb",		"",		"t,A(b)",	0,	(int) M_LBX_AB,	INSN_MACRO,		0,	I38,	0},
{"lbe", 	"", 		"t,+j(b)",	0xa4000200, 0xfc007f00,	WR_1|RD_3,		0,	0,	EVA},
{"lbe", 	"", 		"t,A(c)",	0,	(int) M_LBE_AC,	INSN_MACRO,		0,	0,	EVA},
{"lbu", 	"[16]", 	"md,mL(ml)",	0x5c08, 	0xfc0c,	WR_1|RD_3,		0,	I38,	0}, /* LBU[16] */
{"lbu", 	"[gp]", 	"t,+1(ma)",	0x44080000, 0xfc1c0000,	WR_1|RD_3,		0,	I38,	0}, /* LBU[GP] */
{"lbu", 	"[u12]",	"t,o(b)",	0x84002000, 0xfc00f000,	WR_1|RD_3,		0,	I38,	0}, /* LBU[U12] */
{"lbu", 	"[s9]", 	"t,+j(b)",	0xa4001000, 0xfc007f00,	WR_1|RD_3,		0,	I38,	0}, /* LBU[S9] */
{"lbu", 	"",		"t,A(c)",	0,	(int) M_LBU_AC,	INSN_MACRO,		0,	I38,	0},
{"lbu", 	"",		"t,A(b)",	0,     (int) M_LBUX_AB,	INSN_MACRO,		0,	I38,	0},
{"lbue",	"",		"t,+j(b)",	0xa4001200, 0xfc007f00,	WR_1|RD_3,		0,	0,	EVA},
{"lbue",	"",		"t,A(c)",	0,     (int) M_LBUE_AC,	INSN_MACRO,		0,	0,	EVA},
{"lbux",	"",		"d,s(t)",	0x20000107, 0xfc0007ff, WR_1|RD_2|RD_3,		0,	I38,	0},
{"lbx", 	"",		"d,s(t)",	0x20000007, 0xfc0007ff, WR_1|RD_2|RD_3,		0,	I38,	0},
{"ld",		"",		"t,A(c)",	0,	(int) M_LD_AC,	INSN_MACRO,		0,	I38,	0},
{"ld",		"",		"t,A(b)",	0,	(int) M_LDX_AB,	INSN_MACRO,		0,	I38,	0},
{"lh",		"[16]", 	"md,mH(ml)",	0x7c00, 	0xfc09,	WR_1|RD_3,		0,	I38,	0}, /* LH[16] */
{"lh",		"[gp]", 	"t,+3(ma)",	0x44100000, 0xfc1c0001,	WR_1|RD_3,		0,	I38,	0}, /* LH[GP] */
{"lh",		"[u12]",	 "t,o(b)",	0x84004000, 0xfc00f000,	WR_1|RD_3,		0,	I38,	0}, /* LH[U12] */
{"lh",		"[s9]", 	"t,+j(b)",	0xa4002000, 0xfc007f00,	WR_1|RD_3,		0,	I38,	0}, /* LH[S9] */
{"lh",		"",		"t,A(c)",	0,	(int) M_LH_AC,	INSN_MACRO,		0,	I38,	0},
{"lh",		"",		"t,A(b)",	0,	(int) M_LHX_AB,	INSN_MACRO,		0,	I38,	0},
{"lhe", 	"",		"t,+j(b)",	0xa4002200, 0xfc007f00,	WR_1|RD_3,		0,	0,	EVA},
{"lhe", 	"",		"t,A(c)",	0,	(int) M_LHE_AC,	INSN_MACRO,		0,	0,	EVA},
{"lhu", 	"[16]", 	"md,mH(ml)",	0x7c08,		0xfc09,	WR_1|RD_3,		0,	I38,	0}, /* LHU[16] */
{"lhu", 	"[gp]", 	"t,+3(ma)",	0x44100001, 0xfc1c0001,	WR_1|RD_3,		0,	I38,	0}, /* LHU[GP] */
{"lhu", 	"[u12]",	 "t,o(b)",	0x84006000, 0xfc00f000,	WR_1|RD_3,		0,	I38,	0}, /* LHU[U12] */
{"lhu", 	"[s9]", 	"t,+j(b)",	0xa4003000, 0xfc007f00,	WR_1|RD_3,		0,	I38,	0}, /* LHU[S9] */
{"lhu", 	"",		"t,A(c)",	0,	(int) M_LHU_AC,	INSN_MACRO,		0,	I38,	0},
{"lhu", 	"",		"t,A(b)",	0,     (int) M_LHUX_AB,	INSN_MACRO,		0,	I38,	0},
{"lhue",	"",		"t,+j(b)",	0xa4003200, 0xfc007f00,	WR_1|RD_3,		0,	0,	EVA},
{"lhue",	"",		"t,A(c)",	0,     (int) M_LHUE_AC,	INSN_MACRO,		0,	0,	EVA},
{"lhux",	"",		"d,s(t)",	0x20000307, 0xfc0007ff, WR_1|RD_2|RD_3,		0,	I38,	0},
{"lhuxs",	"",		"d,s(t)",	0x20000347, 0xfc0007ff, WR_1|RD_2|RD_3,		0,	I38,	0},
{"lhx", 	"",		"d,s(t)",	0x20000207, 0xfc0007ff, WR_1|RD_2|RD_3,		0,	I38,	0},
{"lhxs",	"",		"d,s(t)",	0x20000247, 0xfc0007ff, WR_1|RD_2|RD_3,		0,	I38,	0},
{"li.d",	"",		"t,F",		0,	(int) M_LI_D,	INSN_MACRO,		0,	I38,	0},
{"li.s",	"",		"t,f",		0,	(int) M_LI_S,	INSN_MACRO,		0,	I38,	0},
{"ll",		"",		"t,+p(b)",	0xa4005100, 0xfc007f03,	WR_1|RD_3,		0,	I38,	0},
{"ll",		"",		"t,A(c)",	0,	(int) M_LL_AC,	INSN_MACRO,		0,	I38,	0},
{"lle", 	"",		"t,+p(b)",	0xa4005200, 0xfc007f03,	WR_1|RD_3,		0,	0,	EVA},
{"lle", 	"",		"t,A(c)",	0,	(int) M_LLE_AC,	INSN_MACRO,		0,	0,	EVA},
{"llwp",	"",		"t,e,(b),-K",	0xa4005101, 0xfc007f03, WR_1|WR_2|RD_3,		0,	0,	xNMS},
{"llwp",	"",		"t,e,A(c)",	0,	(int) M_LLWP_AC, INSN_MACRO,		0,	0,	xNMS},
{"llwpe",	"",		"t,e,(b),-K",	0xa4005201, 0xfc007f03, WR_1|WR_2|RD_3,		0,	0,	EVA},
{"llwpe",	"",		"t,e,A(c)",	0,	(int) M_LLWP_AC, INSN_MACRO,		0,	0,	EVA},
{"lsa", 	"",		"d,v,t,+w,-L",	0x2000000f, 0xfc00003f, WR_1|RD_2|RD_3,		0,	I38,	0},
{"lw",		"[16]", 	"md,mJ(ml)",	0x1400, 	0xfc00,	WR_1|RD_3,		0,	I38,	0}, /* LW[16] */
{"lw",		"[4x4]",	"mu,mN(mv)",	0x7400, 	0xfc00,	WR_1|RD_3,		0,	0,	xNMS}, /* LW[4X4] */
{"lw",		"[sp]", 	"mp,mR(ms)",	0x3400, 	0xfc00,	WR_1|RD_3,		0,	I38,	0}, /* LW[SP] */
{"lw",		"[gp16]",	"md,mO(ma)",	0x5400, 	0xfc00,	WR_1|RD_3,		0,	I38,	0}, /* LW[GP16] */
{"lw",		"[gp]", 	"t,.(ma)",	0x40000002, 0xfc000003,	WR_1|RD_3,		0,	I38,	0}, /* LW[GP] */
{"lw",		"[gp16]",	"md,mA(ma)",	0x5400, 	0xfc00,	WR_1|RD_3,		0,	I38,	0}, /* LW[GP16] */
{"lw",		"[u12]",	"t,o(b)",	0x84008000, 0xfc00f000,	WR_1|RD_3,		0,	I38,	0}, /* LW[U12] */
{"lw",		"[s9]", 	"t,+j(b)",	0xa4004000, 0xfc007f00,	WR_1|RD_3,		0,	I38,	0}, /* LW[S9] */
{"lw",		"",		"t,A(c)",	0,	(int) M_LW_AC,	INSN_MACRO,		0,	I38,	0},
{"lw",		"",		"t,A(b)",	0,	(int) M_LWX_AB,	INSN_MACRO,		0,	I38,	0},
{"lwe", 	"",		"t,+j(b)",	0xa4004200, 0xfc007f00,	WR_1|RD_3,		0,	0,	EVA},
{"lwe", 	"",		"t,A(c)",	0,	(int) M_LWE_AC,	INSN_MACRO,		0,	0,	EVA},
{"lwm", 	"",		"t,+j(b),|",	0xa4000400, 0xfc000f00,	WR_1|RD_3,		0,	0,	xNMS}, /* LWM */
{"lwpc",	"[48]", 	"mp,+S",	0x600b, 	0xfc1f,	WR_1,			0,	0,	xNMS}, /* LWPC[48] */
{"lwx", 	"",		"d,s(t)",	0x20000407, 0xfc0007ff, WR_1|RD_2|RD_3,		0,	I38,	0},
{"lwxs",	"[16]", 	"me,ml(md)",	0x5001, 	0xfc01, WR_1|RD_2|RD_3,		0,	I38,	0}, /* LWXS[16] */
{"lwxs",	"[32]", 	"d,s(t)",	0x20000447, 0xfc0007ff, WR_1|RD_2|RD_3,		0,	I38,	0},
{"madd",	"[dsp]",	"7,s,t",	0x20000abf, 0xfc003fff, MOD_1|RD_2|RD_3,	0,	0,	D32}, /* MADD[DSP] */
{"maddu",	"[dsp]",	"7,s,t",	0x20001abf, 0xfc003fff, MOD_1|RD_2|RD_3,	0,	0,	D32}, /* MADDU[DSP] */
{"maq_s.w.phl", "",		"7,s,t",	0x20001a7f, 0xfc003fff, MOD_1|RD_2|RD_3,	0,	0,	D32},
{"maq_s.w.phr", "",		"7,s,t",	0x20000a7f, 0xfc003fff, MOD_1|RD_2|RD_3,	0,	0,	D32},
{"maq_sa.w.phl", "",		"7,s,t",	0x20003a7f, 0xfc003fff, MOD_1|RD_2|RD_3,	0,	0,	D32},
{"maq_sa.w.phr", "",		"7,s,t",	0x20002a7f, 0xfc003fff, MOD_1|RD_2|RD_3,	0,	0,	D32},
{"mfc0",	"",		"t,O",		0x20000030, 0xfc0007ff,	WR_1,		INSN2_ALIAS,	I38,	0}, /* MFC0 with named register */
{"mfc0",	"",		"t,P,J",	0x20000030, 0xfc0007ff,	WR_1,		INSN2_ALIAS,	I38,	0}, /* MFC0 with named register & select */
{"mfc0",	"",		"t,G,J,-B",	0x20000030, 0xfc0003ff,	WR_1,			0,	I38,	0},
{"mfhc0",	"",		"t,O",		0x20000038, 0xfc0007ff,	WR_1,		INSN2_ALIAS,	I38,	0}, /* MFHC0 with named register */
{"mfhc0",	"",		"t,P,J",	0x20000038, 0xfc0007ff,	WR_1,		INSN2_ALIAS,	I38,	0}, /* MFHC0 with named register & select*/
{"mfhc0",	"",		"t,G,J,-B",	0x20000038, 0xfc0003ff,	WR_1,			0,	I38,	0},
{"mfhi",	"[dsp]",	"t,7,-A",	0x2000007f, 0xfc003fff,	WR_1|RD_2,		0,	0,	D32}, /* MFHI[DSP] */
{"mflo",	"[dsp]",	"t,7,-A",	0x2000107f, 0xfc003fff,	WR_1|RD_2,		0,	0,	D32}, /* MFLO[DSP] */
{"mftc0",	"",		"t,O",		0x20000230, 0xfc0007ff,	WR_1,		INSN2_ALIAS,	0,	MT32}, /* MFTR with named register */
{"mftc0",	"",		"t,P,J",	0x20000230, 0xfc0007ff,	WR_1,		INSN2_ALIAS,	0,	MT32}, /* MFTR with named register & select */
{"mfthc0",	"",		"t,O",		0x20000238, 0xfc0007ff,	WR_1,		INSN2_ALIAS,	0,	MT32}, /* MFTR with named register */
{"mfthc0",	"",		"t,P,J",	0x20000238, 0xfc0007ff,	WR_1,		INSN2_ALIAS,	0,	MT32}, /* MFTR with named register & select*/
{"mftc1",	"",		"t,S",		0x20001630, 0xfc00ffff,	WR_1|RD_2|FP_S,	INSN2_ALIAS,	0,	MT32}, /* MFTR */
{"mftc1",	"",		"t,G",		0x20001630, 0xfc00ffff,	WR_1|RD_2|FP_S,	INSN2_ALIAS,	0,	MT32}, /* MFTR */
{"mftc2",	"",		"t,G",		0x20002630, 0xfc00ffff,	WR_1,		INSN2_ALIAS,	0,	MT32}, /* MFTR */
{"mftdsp",	"",		"t",		0x20100e30, 0xfc1fffff,	WR_1,		INSN2_ALIAS,	0,	MT32}, /* MFTR */
{"mftgpr",	"",		"t,s",		0x20000630, 0xfc00ffff,	WR_1|RD_2,	INSN2_ALIAS,	0,	MT32}, /* MFTR */
{"mfthc1",	"",		"t,S",		0x20001638, 0xfc00ffff,	WR_1|RD_2|FP_D,	INSN2_ALIAS,	0,	MT32}, /* MFTR */
{"mfthc1",	"",		"t,G",		0x20001638, 0xfc00ffff,	WR_1|RD_2|FP_D,	INSN2_ALIAS,	0,	MT32}, /* MFTR */
{"mfthc2",	"",		"t,G",		0x20002638, 0xfc00ffff,	WR_1,		INSN2_ALIAS,	0,	MT32}, /* MFTR */
{"mfthi",	"",		"t",		0x20010e30, 0xfc1fffff,	WR_1|RD_a,	INSN2_ALIAS,	0,	MT32}, /* MFTR */
{"mfthi",	"",		"t,*",		0x20010e30, 0xfc13ffff,	WR_1|RD_a,	INSN2_ALIAS,	0,	MT32}, /* MFTR */
{"mftlo",	"",		"t",		0x20000e30, 0xfc1fffff,	WR_1|RD_a,	INSN2_ALIAS,	0,	MT32}, /* MFTR */
{"mftlo",	"",		"t,*",		0x20000e30, 0xfc13ffff,	WR_1|RD_a,	INSN2_ALIAS,	0,	MT32}, /* MFTR */
{"mftr",	"",		"t,s,!,H,$",	0x20000230, 0xfc0003f7,	WR_1,			0,	0,	MT32},
{"mod", 	"",		"d,v,t,-B",	0x20000158, 0xfc0003ff, WR_1|RD_2|RD_3,		0,	I38,	0},
{"mod", 	"",		"d,v,I",	0,	(int) M_MOD_I,	INSN_MACRO,		0,	I38,	0},
{"modsub",	"",		"d,s,t,-B",	0x20000295, 0xfc0003ff, WR_1|RD_2|RD_3,		0,	0,	D32},
{"modu",	"",		"d,v,t,-B",	0x200001d8, 0xfc0003ff, WR_1|RD_2|RD_3,		0,	I38,	0},
{"modu",	"",		"d,v,I",	0,	(int) M_MODU_I,	INSN_MACRO,		0,	I38,	0},
{"move.balc",	"",		"+7,+5,+r",	0x08000000, 0xfc000000,	WR_1|RD_2,		0,	0,	xNMS},
{"move.bal",	"",		"+7,+5,+r",	0x08000000, 0xfc000000,	WR_1|RD_2,	INSN2_ALIAS|UBR|CTC, 0,	xNMS}, /* MOVE.BALC */
{"movep",	"",		"mq,mx,mw",	0xbc00, 	0xfc00,	WR_1|RD_2|RD_3,		0,	0,	xNMS}, /* MOVEP */
{"movep",	"",		"mr,mw,mx",	0xbc00, 	0xfc00,	WR_1|RD_2|RD_3,	INSN2_ALIAS,	0,	xNMS}, /* MOVEP */
{"movep",	"[rev]",	"mv,mu,mq",	0xfc00, 	0xfc00,	WR_1|WR_2|RD_3,		0,	0,	xNMS}, /* MOVEP[REV] */
{"movep",	"[rev]",	"mu,mv,mr",	0xfc00, 	0xfc00,	WR_1|WR_2|RD_3,	INSN2_ALIAS,	0,	xNMS}, /* MOVEP[REV] */
{"movn",	"",		"d,v,t",	0x20000610, 0xfc0007ff, WR_1|RD_2|RD_3,		0,	I38,	0},
{"movz",	"",		"d,v,t",	0x20000210, 0xfc0007ff, WR_1|RD_2|RD_3,		0,	I38,	0},
{"msub",	"[dsp]",	"7,s,t",	0x20002abf, 0xfc003fff,MOD_1|RD_2|RD_3,		0,	0,	D32}, /* MSUB[DSP] */
{"msubu",	"[dsp]",	"7,s,t",	0x20003abf, 0xfc003fff,MOD_1|RD_2|RD_3,		0,	0,	D32}, /* MSUBU[DSP] */
{"mtc0",	"",		"t,O",		0x20000070, 0xfc0007ff,	RD_1,		INSN2_ALIAS,	I38,	0}, /* MTC0 with named register */
{"mtc0",	"",		"t,P,J",	0x20000070, 0xfc0007ff,	RD_1,		INSN2_ALIAS,	I38,	0}, /* MTCO with named register & select */
{"mtc0",	"",		"t,G,J,-B",	0x20000070, 0xfc0003ff,	RD_1,			0,	I38,	0},
{"mthc0",	"",		"t,O",		0x20000078, 0xfc0007ff,	RD_1,		INSN2_ALIAS,	I38,	0}, /* MTHC0 with named register */
{"mthc0",	"",		"t,P,J",	0x20000078, 0xfc0007ff,	RD_1,		INSN2_ALIAS,	I38,	0}, /* MTHC0 with named register & select */
{"mthc0",	"",		"t,G,J,-B",	0x20000078, 0xfc0003ff,	RD_1,			0,	I38,	0},
{"mthi",	"[dsp]",	"s,7,-I",	0x2000207f, 0xfc003fff,	WR_1|RD_2,		0,	0,	D32}, /* MTHI[DSP] */
{"mthlip",	"",		"s,7,-I",	0x2000027f, 0xfc003fff,	WR_1|RD_2,		0,	0,	D32},
{"mtlo",	"[dsp]",	"s,7,-I",	0x2000307f, 0xfc003fff,	WR_1|RD_2,		0,	0,	D32}, /* MTLO[DSP] */
{"mttc0",	"",		"t,O",		0x20000270, 0xfc0007ff,	RD_1,		INSN2_ALIAS,	0,	MT32}, /* MTTR with named register */
{"mttc0",	"",		"t,P,J",	0x20000270, 0xfc0007ff,	RD_1,		INSN2_ALIAS,	0,	MT32}, /* MTTR with named register & select */
{"mtthc0",	"",		"t,O",		0x20000278, 0xfc0007ff,	RD_1,		INSN2_ALIAS,	0,	MT32}, /* MTTR with named register*/
{"mtthc0",	"",		"t,P,J",	0x20000278, 0xfc0007ff,	RD_1,		INSN2_ALIAS,	0,	MT32}, /* MTTR with named register & select */
{"mttc1",	"",		"t,S",		0x20001670, 0xfc00ffff,	RD_1|WR_2|FP_S,	INSN2_ALIAS,	0,	MT32}, /* MTTR */
{"mttc1",	"",		"t,G",		0x20001670, 0xfc00ffff,	RD_1|WR_2|FP_S,	INSN2_ALIAS,	0,	MT32}, /* MTTR */
{"mttc2",	"",		"t,G",		0x20002670, 0xfc00ffff,	RD_1,		INSN2_ALIAS,	0,	MT32}, /* MTTR */
{"mttgpr",	"",		"s,t",		0x20000670, 0xfc00ffff,	RD_1|WR_2, INSN2_ALIAS|INSN2_MTTGPR_RC1, 0, MT32}, /* MTTR */
{"mttgpr",	"",		"t,s",		0x20000670, 0xfc00ffff,	RD_1|WR_2,	INSN2_ALIAS,	0,	MT32}, /* MTTR */
{"mtthc1",	"",		"t,S",		0x20001678, 0xfc00ffff,	RD_1|WR_2|FP_D,	INSN2_ALIAS,	0,	MT32}, /* MTTR */
{"mtthc1",	"",		"t,G",		0x20001678, 0xfc00ffff,	RD_1|WR_2|FP_D,	INSN2_ALIAS,	0,	MT32}, /* MTTR */
{"mtthc2",	"",		"t,G",		0x20002678, 0xfc00ffff,	RD_1,		INSN2_ALIAS,	0,	MT32}, /* MTTR */
{"mtthi",	"",		"t",		0x20010e70, 0xfc1fffff,	RD_1|WR_a,	INSN2_ALIAS,	0,	MT32}, /* MTTR */
{"mtthi",	"",		"t,*",		0x20010e70, 0xfc13ffff,	RD_1|WR_a,	INSN2_ALIAS,	0,	MT32}, /* MTTR */
{"mttlo",	"",		"t",		0x20000e70, 0xfc1fffff,	RD_1|WR_a,	INSN2_ALIAS,	0,	MT32}, /* MTTR */
{"mttlo",	"",		"t,*",		0x20000e70, 0xfc13ffff,	RD_1|WR_a,	INSN2_ALIAS,	0,	MT32}, /* MTTR */
{"mttdsp",	"",		"t",		0x20100e70, 0xfc1fffff,	RD_1,		INSN2_ALIAS,	0,	MT32}, /* MTTR */
{"mttr",	"",		"t,s,!,H,$",	0x20000270, 0xfc0003f7,	RD_1,			0,	0,	MT32},
{"muh", 	"",		"d,v,t,-B",	0x20000058, 0xfc0003ff, WR_1|RD_2|RD_3,		0,	I38,	0},
{"muhu",	"",		"d,v,t,-B",	0x200000d8, 0xfc0003ff, WR_1|RD_2|RD_3,		0,	I38,	0},
{"mul", 	"[4x4]",	"mu,mt,mv",	0x3c08, 	0xfd08,	MOD_1|RD_3,		0,	0,	xNMS}, /* MUL[4X4] */
{"mul", 	"[4x4]",	"mu,mv,mk",	0x3c08, 	0xfd08,	MOD_1|RD_2,	INSN2_ALIAS,	0,	xNMS}, /* MUL[4X4] */
{"mul", 	"[32]", 	"d,v,t,-B",	0x20000018, 0xfc0003ff, WR_1|RD_2|RD_3,		0,	I38,	0},
{"mul", 	"",		"d,v,I",	0,	(int) M_MUL_I,	INSN_MACRO,		0,	I38,	0},
{"mul.ph",	"",		"d,s,t",	0x2000002d, 0xfc0007ff, WR_1|RD_2|RD_3,		0,	0,	D32},
{"muleq_s.w.phl", "",		"d,s,t,-B",	0x20000025, 0xfc0003ff, WR_1|RD_2|RD_3,		0,	0,	D32},
{"muleq_s.w.phr", "",		"d,s,t,-B",	0x20000065, 0xfc0003ff, WR_1|RD_2|RD_3,		0,	0,	D32},
{"muleu_s.ph.qbl", "",		"d,s,t,-B",	0x20000095, 0xfc0003ff, WR_1|RD_2|RD_3,		0,	0,	D32},
{"muleu_s.ph.qbr", "",		"d,s,t,-B",	0x200000d5, 0xfc0003ff, WR_1|RD_2|RD_3,		0,	0,	D32},
{"mulq_rs.ph",	"",		"d,s,t,-B",	0x20000115, 0xfc0003ff, WR_1|RD_2|RD_3,		0,	0,	D32},
{"mulq_rs.w",	"",		"d,s,t,-B",	0x20000195, 0xfc0003ff, WR_1|RD_2|RD_3,		0,	0,	D32},
{"mulq_s.ph",	"",		"d,s,t,-B",	0x20000155, 0xfc0003ff, WR_1|RD_2|RD_3,		0,	0,	D32},
{"mulq_s.w",	"",		"d,s,t,-B",	0x200001d5, 0xfc0003ff, WR_1|RD_2|RD_3,		0,	0,	D32},
{"mulsa.w.ph",	"",		"7,s,t",	0x20002cbf, 0xfc003fff, WR_1|RD_2|RD_3,		0,	0,	D32},
{"mulsaq_s.w.ph", "",		"7,s,t",	0x20003cbf, 0xfc003fff, WR_1|RD_2|RD_3,		0,	0,	D32},
{"mult",	"[dsp]",	"7,s,t",	0x20000cbf, 0xfc003fff, WR_1|RD_2|RD_3,		0,	0,	D32}, /* MULT[DSP] */
{"multu",	"[dsp]",	"7,s,t",	0x20001cbf, 0xfc003fff, WR_1|RD_2|RD_3,		0,	0,	D32}, /* MULTU[DSP] */
{"mulu",	"",		"d,v,t,-B",	0x20000098, 0xfc0003ff, WR_1|RD_2|RD_3,		0,	I38,	0},
{"mul_s.ph",	"",		"d,s,t",	0x2000042d, 0xfc0007ff, WR_1|RD_2|RD_3,		0,	0,	D32},
{"neg", 	"",		"d,w",		0x20000190, 0xfc1f07ff, WR_1|RD_2,	INSN2_ALIAS,	0,	xNMS}, /* SUB */
{"negu",	"",		"d,w",		0x200001d0, 0xfc1f07ff, WR_1|RD_2,	INSN2_ALIAS,	I38,	0}, /* SUBU */
{"not", 	"[16]", 	"md,ml",	0x5000, 	0xfc0f,	WR_1|RD_2,		0,	I38,	0}, /* NOT[16] */
{"not", 	"[32]", 	"d,v",		0x200002d0, 0xffe007ff, WR_1|RD_2,	INSN2_ALIAS,	I38,	0}, /* NOR */
{"nor", 	"",		"d,v,t,-B",	0x200002d0, 0xfc0003ff, WR_1|RD_2|RD_3,		0,	I38,	0},
{"nor", 	"",		"t,r,I",	0,    (int) M_NOR_I,	INSN_MACRO,		0,	I38,	0},
{"or",		"[16]", 	"md,mk,ml",	0x500c, 	0xfc0f,	WR_1|RD_3,	INSN2_ALIAS,	I38,	0}, /* OR[16] */
{"or",		"[16]", 	"md,ml,mk",	0x500c, 	0xfc0f,	WR_1|RD_2,	INSN2_ALIAS,	I38,	0}, /* OR[16] */
{"or",		"[16]", 	"md,ml",	0x500c, 	0xfc0f,	WR_1|RD_2,		0,	I38,	0}, /* OR[16] */
{"or",		"[32]", 	"d,v,t,-B",	0x20000290, 0xfc0003ff, WR_1|RD_2|RD_3,		0,	I38,	0},
{"or",		"",		"t,r,I",	0,	(int) M_OR_I,	INSN_MACRO,		0,	I38,	0},
{"ori", 	"",		"t,r,g",	0x80000000, 0xfc00f000,	WR_1|RD_2,		0,	I38,	0},
{"pause",	"",		"-M",		0x8000c005, 0xfffff1ff,	0,			0,	I38,	0},
{"packrl.ph",	"",		"d,s,t,-B",	0x200001ad, 0xfc0003ff, WR_1|RD_2|RD_3,		0,	0,	D32},
{"pick.ph",	"",		"d,s,t,-B",	0x2000022d, 0xfc0003ff, WR_1|RD_2|RD_3,		0,	0,	D32},
{"pick.qb",	"",		"d,s,t,-B",	0x200001ed, 0xfc0003ff, WR_1|RD_2|RD_3,		0,	0,	D32},
{"preceq.w.phl", "",		"t,s",		0x2000513f, 0xfc00ffff,	WR_1|RD_2,		0,	0,	D32},
{"preceq.w.phr", "",		"t,s",		0x2000613f, 0xfc00ffff,	WR_1|RD_2,		0,	0,	D32},
{"precequ.ph.qbl", "",		"t,s",		0x2000713f, 0xfc00ffff,	WR_1|RD_2,		0,	0,	D32},
{"precequ.ph.qbla", "", 	"t,s",		0x2000733f, 0xfc00ffff,	WR_1|RD_2,		0,	0,	D32},
{"precequ.ph.qbr", "",		"t,s",		0x2000913f, 0xfc00ffff,	WR_1|RD_2,		0,	0,	D32},
{"precequ.ph.qbra", "", 	"t,s",		0x2000933f, 0xfc00ffff,	WR_1|RD_2,		0,	0,	D32},
{"preceu.ph.qbl", "",		"t,s",		0x2000b13f, 0xfc00ffff,	WR_1|RD_2,		0,	0,	D32},
{"preceu.ph.qbla", "",		"t,s",		0x2000b33f, 0xfc00ffff,	WR_1|RD_2,		0,	0,	D32},
{"preceu.ph.qbr", "",		"t,s",		0x2000d13f, 0xfc00ffff,	WR_1|RD_2,		0,	0,	D32},
{"preceu.ph.qbra", "",		"t,s",		0x2000d33f, 0xfc00ffff,	WR_1|RD_2,		0,	0,	D32},
{"precr.qb.ph", "",		"d,s,t,-B",	0x2000006d, 0xfc0003ff, WR_1|RD_2|RD_3,		0,	0,	D32},
{"precrq.ph.w", "",		"d,s,t,-B",	0x200000ed, 0xfc0003ff, WR_1|RD_2|RD_3,		0,	0,	D32},
{"precrq.qb.ph", "",		"d,s,t,-B",	0x200000ad, 0xfc0003ff, WR_1|RD_2|RD_3,		0,	0,	D32},
{"precrqu_s.qb.ph", "", 	"d,s,t,-B",	0x2000016d, 0xfc0003ff, WR_1|RD_2|RD_3,		0,	0,	D32},
{"precrq_rs.ph.w", "",		"d,s,t,-B",	0x2000012d, 0xfc0003ff, WR_1|RD_2|RD_3,		0,	0,	D32},
{"precr_sra.ph.w", "",		"t,s,1",	0x200003cd, 0xfc0007ff,	WR_1|RD_2,		0,	0,	D32},
{"precr_sra_r.ph.w", "",	"t,s,1",	0x200007cd, 0xfc0007ff,	WR_1|RD_2,		0,	0,	D32},
{"pref",	"[u12]",	"k,o(b)",	0x84003000, 0xfc00f000,	RD_3,			0,	I38,	0}, /* PREF[U12] */
{"pref",	"[s9]", 	"k,+j(b)",	0xa4001800, 0xfc007f00,	RD_3,			0,	I38,	0}, /* PREF[S9], preceded by SYNCI[S9] */
{"pref",	"",		"k,A(c)",	0,     (int) M_PREF_AC,	INSN_MACRO,		0,	I38,	0},
{"prefe",	"",		"k,+j(b)",	0xa4001a00, 0xfc007f00,	RD_3,			0,	0,	EVA}, /* preceded by SYNCIE */
{"prefe",	"",		"k,A(c)",	0,    (int) M_PREFE_AC,	INSN_MACRO,		0,	0,	EVA},
{"prepend",	"",		"d,-n,t,+I",	0x2000001f, 0xfc00003f, WR_1|RD_3,	INSN2_ALIAS,	0,	D32}, /* EXTW */
{"raddu.w.qb",	"",		"t,s",		0x2000f13f, 0xfc00ffff,	WR_1|RD_2,		0,	0,	D32},
{"rddsp",	"",		"t",		0x201fc67f, 0xfc1fffff,	WR_1,		INSN2_ALIAS,	0,	D32},
{"rddsp",	"",		"t,8",		0x2000067f, 0xfc003fff,	WR_1,			0,	0,	D32},
{"rdhwr",	"",		"t,K",		0x200001c0, 0xfc0007ff,	WR_1|RD_2,	INSN2_ALIAS,	0,	xNMS}, /* RDHWR with sel=0 */
{"rdhwr",	"",		"t,U,J",	0x200001c0, 0xfc0007ff,	WR_1|RD_2,	INSN2_ALIAS,	0,	xNMS},
{"rdhwr",	"",		"t,G,H,-B",	0x200001c0, 0xfc0003ff,	WR_1|RD_2,		0,	0,	xNMS}, /* RDWHR */
{"rdpgpr",	"",		"t,s",		0x2000e17f, 0xfc00ffff,	WR_1|RD_2,		0,	I38,	0},
{"rem", 	"",		"d,v,t",	0x20000158, 0xfc0007ff, WR_1|RD_2|RD_3,	INSN2_ALIAS,	I38,	0}, /* MOD */
{"rem", 	"",		"d,v,I",	0,	(int) M_REM_3I,	INSN_MACRO,		0,	I38,	0},
{"remu",	"",		"d,v,t",	0x200001d8, 0xfc0007ff, WR_1|RD_2|RD_3,	INSN2_ALIAS,	I38,	0}, /* MODU */
{"repl.ph",	"",		"t,@,-B",	0x2000003d, 0xfc0003ff,	WR_1,			0,	0,	D32},
{"repl.qb",	"",		"t,5,-O",	0x200005ff, 0xfc000fff,	WR_1,			0,	0,	D32},
{"replv.ph",	"",		"t,s",		0x2000033f, 0xfc00ffff,	WR_1|RD_2,		0,	0,	D32},
{"replv.qb",	"",		"t,s",		0x2000133f, 0xfc00ffff,	WR_1|RD_2,		0,	0,	D32},
{"restore",	"[32]", 	"+N,n", 	0x80003002, 0xfc00f003,	0,			0,	I38,	0},
{"restore.jrc", "[16]", 	"mG",		0x1d00, 	0xff0f,	0,			0,	I38,	0}, /* RESTORE.JRC[16] */
{"restore.jrc", "[16]", 	"mG,mn",	0x1d00, 	0xfd00,	0,			0,	I38,	0}, /* RESTORE.JRC[16], preceded by RESTORE[16] */
{"restore.jrc", "[32]", 	"+N",		0x80003003, 0xfffff007,	0,			0,	I38,	0}, /* RESTORE.JRC[32] */
{"restore.jrc", "[32]", 	"+N,n", 	0x80003003, 0xfc00f003,	0,			0,	I38,	0},
{"jraddiusp",	"",		"mG",		0x1d00, 	0xff0f,	0,		INSN2_ALIAS,	I38,	0}, /* RESTORE.JRC[16] */
{"jraddiusp",	"",		"+N",		0x80003003, 0xfffff007,	0,		INSN2_ALIAS,	I38,	0}, /* RESTORE.JRC[32] */
{"jraddiusp",	"",		"I",		0,   (int) M_JRADDIUSP,	INSN_MACRO,		0,	I38,	0},
{"rol", 	"",		"d,v,t",	0,	(int) M_ROL,	INSN_MACRO,		0,	I38,	0},
{"rol", 	"",		"d,v,I",	0,	(int) M_ROL_I,	INSN_MACRO,		0,	I38,	0},
{"rotrv",	"",		"d,s,t,-B",	0x200000d0, 0xfc0003ff, WR_1|RD_2|RD_3,		0,	I38,	0},
{"rotr",	"",		"t,r,<,-M",	0x8000c0c0, 0xfc00f1e0,	WR_1|RD_2,		0,	I38,	0},
{"rotr",	"",		"d,v,t",	0x200000d0, 0xfc0007ff, WR_1|RD_2|RD_3,	INSN2_ALIAS,	I38,	0}, /* ROTRV */
{"ror", 	"",		"t,r,<",	0x8000c0c0, 0xfc00ffe0,	WR_1|RD_2,	INSN2_ALIAS,	I38,	0}, /* ROTR */
{"ror", 	"",		"d,v,t",	0x200000d0, 0xfc0007ff, WR_1|RD_2|RD_3,	INSN2_ALIAS,	I38,	0}, /* ROTRV */
{"ror", 	"",		"d,v,I",	0,    (int) M_ROR_I,	INSN_MACRO,		0,	I38,	0},
{"rorv",	"",		"t,r,<",	0x8000c0c0, 0xfc00ffe0,	WR_1|RD_2,	INSN2_ALIAS,	I38,	0}, /* ROTR */
{"rorv",	"",		"d,v,t",	0x200000d0, 0xfc0007ff, WR_1|RD_2|RD_3,	INSN2_ALIAS,	I38,	0}, /* ROTRV */
{"rotl",	"",		"d,v,t",	0,    (int) M_ROL,	INSN_MACRO,		0,	I38,	0},
{"rotl",	"",		"d,v,I",	0,    (int) M_ROL_I,	INSN_MACRO,		0,	I38,	0},
{"wsbh",	"",		"t,r",		0x8000d608, 0xfc00ffff,	WR_1|RD_2,	INSN2_ALIAS,	0,	xNMS}, /* ROTX t,s,8,24*/
{"rotx",	"",		"t,r,<,+*",	0x8000d000, 0xfc00f860, WR_1|RD_2,	INSN2_ALIAS,	0,	xNMS},
{"rotx",	"",		"t,r,<,+*,+|",	0x8000d000, 0xfc00f820, WR_1|RD_2,		0,	0,	xNMS},
{"save",	"[16]", 	"mG",		0x1c00, 	0xff0f,	0,			0,	I38,	0}, /* SAVE[16] */
{"save",	"[16]", 	"mG,mn",	0x1c00, 	0xfd00,	0,			0,	I38,	0}, /* SAVE[16] */
{"save",	"[32]", 	"+N,n",		0x80003000, 0xfc00f003,	0,			0,	I38,	0},
{"sb",		"[16]", 	"mm,mL(ml)",	0x5c04, 	0xfc0c,	RD_1|RD_3,		0,	I38,	0}, /* SB[16] */
{"sb",		"[gp]", 	"t,+1(ma)",	0x44040000, 0xfc1c0000,	RD_1|RD_3,		0,	I38,	0}, /* SB[GP] */
{"sb",		"[u12]",	"t,o(b)",	0x84001000, 0xfc00f000,	RD_1|RD_3,		0,	I38,	0}, /* SB[U12] */
{"sb",		"[s9]", 	"t,+j(b)",	0xa4000800, 0xfc007f00,	RD_1|RD_3,		0,	I38,	0}, /* SB[S9] */
{"sb",		"",		"t,A(c)",	0,	(int) M_SB_AC,	INSN_MACRO,		0,	I38,	0},
{"sb",		"",		"t,A(b)",	0,	(int) M_SBX_AB,	INSN_MACRO,		0,	0,	xNMS},
{"sbe", 	"",		"t,+j(b)",	0xa4000a00, 0xfc007f00,	RD_1|RD_3,		0,	0,	EVA},
{"sbe", 	"",		"t,A(c)",	0,	(int) M_SBE_AC,	INSN_MACRO,		0,	0,	EVA},
{"sbx", 	"",		"d,s(t)",	0x20000087, 0xfc0007ff, RD_1|RD_2|RD_3,		0,	0,	xNMS},
{"sc",		"",		"t,+p(b)",	0xa4005900, 0xfc007f03,	MOD_1|RD_3,		0,	I38,	0},
{"sc",		"",		"t,A(c)",	0,	(int) M_SC_AC,	INSN_MACRO,		0,	I38,	0},
{"sce", 	"",		"t,+p(b)",	0xa4005a00, 0xfc007f03,	MOD_1|RD_3,		0,	0,	EVA},
{"sce", 	"",		"t,A(c)",	0,	(int) M_SCE_AC,	INSN_MACRO,		0,	0,	EVA},
{"scwp",	"",		"t,e,(b),-K",	0xa4005901, 0xfc007f03,	MOD_1|WR_2|RD_3,		0,	0,	xNMS},
{"scwp",	"",		"t,e,A(c)",	0,     (int) M_SCWP_AC,	INSN_MACRO,		0,	0,	xNMS},
{"scwpe",	"",		"t,e,(b),-K",	0xa4005a01, 0xfc007f03,	MOD_1|WR_2|RD_3,		0,	0,	EVA},
{"scwpe",	"",		"t,e,A(c)",	0,     (int) M_SCWP_AC,	INSN_MACRO,		0,	0,	EVA},
{"sd",		"",		"t,A(c)",	0,	(int) M_SD_AC,	INSN_MACRO,		0,	I38,	0},
{"sd",		"",		"t,A(b)",	0,	(int) M_SDX_AB,	INSN_MACRO,		0,	I38,	0},
{"seb", 	"", 		"t,r,-N",	0x20000008, 0xfc0003ff,	WR_1|RD_2,		0,	0,	xNMS},
{"seh", 	"", 		"t,r,-N",	0x20000048, 0xfc0003ff,	WR_1|RD_2,		0,	I38,	0},
{"seqi",	"",		"t,r,i",	0x80006000, 0xfc00f000, WR_1|RD_2,		0,	I38,	0},
{"seq", 	"", 		"d,v,t",	0,	(int) M_SEQ,	INSN_MACRO,		0,	I38,	0},
{"seq", 	"", 		"d,v,I",	0,	(int) M_SEQ_I,	INSN_MACRO,		0,	I38,	0},
{"sge", 	"", 		"d,v,t",	0,	(int) M_SGE,	INSN_MACRO,		0,	I38,	0},
{"sge", 	"", 		"d,v,I",	0,	(int) M_SGE_I,	INSN_MACRO,		0,	I38,	0},
{"sgeu",	"",		"d,v,t",	0,	(int) M_SGEU,	INSN_MACRO,		0,	I38,	0},
{"sgeu",	"",		"d,v,I",	0,     (int) M_SGEU_I,	INSN_MACRO,		0,	I38,	0},
{"sgt", 	"", 		"d,v,t",	0,	(int) M_SGT,	INSN_MACRO,		0,	I38,	0},
{"sgt", 	"", 		"d,v,I",	0,	(int) M_SGT_I,	INSN_MACRO,		0,	I38,	0},
{"sgtu",	"",		"d,v,t",	0,	(int) M_SGTU,	INSN_MACRO,		0,	I38,	0},
{"sgtu",	"",		"d,v,I",	0,     (int) M_SGTU_I,	INSN_MACRO,		0,	I38,	0},
{"sh",		"[16]", 	"mm,mH(ml)",	0x7c01, 	0xfc09,	RD_1|RD_3,		0,	I38,	0}, /* SH[16] */
{"sh",		"[gp]", 	"t,+3(ma)",	0x44140000, 0xfc1c0001,	RD_1|RD_3,		0,	I38,	0}, /* SH[GP] */
{"sh",		"[u12]",	 "t,o(b)",	0x84005000, 0xfc00f000,	RD_1|RD_3,		0,	I38,	0}, /* SH[U12] */
{"sh",		"[s9]", 	"t,+j(b)",	0xa4002800, 0xfc007f00,	RD_1|RD_3,		0,	I38,	0}, /* SH[S9] */
{"sh",		"",		"t,A(c)", 	0,	(int) M_SH_AC,	INSN_MACRO,	0,	I38,	0},
{"sh",		"",		"t,A(b)", 	0,	(int) M_SHX_AB,	INSN_MACRO,	0,	0,	xNMS}, /* SHX */
{"she", 	"",		"t,+j(b)",	0xa4002a00, 0xfc007f00,	RD_1|RD_3,		0,	0,	EVA},
{"she", 	"",		"t,A(c)",	0,	(int) M_SHE_AC,	INSN_MACRO,		0,	0,	EVA},
{"shilo",	"",		"7,0,-P",	0x2000001d, 0xfc0003ff,	MOD_1,			0,	0,	D32},
{"shilov",	"",		"7,s,-I",	0x2000127f, 0xfc003fff,	MOD_1|RD_2,		0,	0,	D32},
{"shll.ph",	"",		"t,s,4",	0x200003b5, 0xfc000fff,	WR_1|RD_2,		0,	0,	D32},
{"shll.qb",	"",		"t,s,3",	0x2000087f, 0xfc001fff,	WR_1|RD_2,		0,	0,	D32},
{"shllv.ph",	"",		"d,t,s",	0x2000038d, 0xfc0007ff, WR_1|RD_2|RD_3,		0,	0,	D32},
{"shllv.qb",	"",		"d,t,s,-B",	0x20000395, 0xfc0003ff, WR_1|RD_2|RD_3,		0,	0,	D32},
{"shllv_s.ph",	"",		"d,t,s",	0x2000078d, 0xfc0007ff, WR_1|RD_2|RD_3,		0,	0,	D32},
{"shllv_s.w",	"",		"d,t,s,-B",	0x200003d5, 0xfc0003ff, WR_1|RD_2|RD_3,		0,	0,	D32},
{"shll_s.ph",	"",		"t,s,4",	0x20000bb5, 0xfc000fff,	WR_1|RD_2,		0,	0,	D32},
{"shll_s.w",	"",		"t,s,1,-B",	0x200003f5, 0xfc0003ff,	WR_1|RD_2,		0,	0,	D32},
{"shra.ph",	"",		"t,s,4",	0x20000335, 0xfc000fff,	WR_1|RD_2,		0,	0,	D32},
{"shra.qb",	"",		"t,s,3",	0x200001ff, 0xfc001fff,	WR_1|RD_2,		0,	0,	D32},
{"shrav.ph",	"",		"d,t,s",	0x2000018d, 0xfc0007ff, WR_1|RD_2|RD_3,		0,	0,	D32},
{"shrav.qb",	"",		"d,t,s",	0x200001cd, 0xfc0007ff, WR_1|RD_2|RD_3,		0,	0,	D32},
{"shrav_r.ph",	"",		"d,t,s",	0x2000058d, 0xfc0007ff, WR_1|RD_2|RD_3,		0,	0,	D32},
{"shrav_r.qb",	"",		"d,t,s",	0x200005cd, 0xfc0007ff, WR_1|RD_2|RD_3,		0,	0,	D32},
{"shrav_r.w",	"",		"d,t,s,-B",	0x200002d5, 0xfc0003ff, WR_1|RD_2|RD_3,		0,	0,	D32},
{"shra_r.ph",	"",		"t,s,4,-Q",	0x20000735, 0xfc0007ff,	WR_1|RD_2,		0,	0,	D32},
{"shra_r.qb",	"",		"t,s,3",	0x200011ff, 0xfc001fff,	WR_1|RD_2,		0,	0,	D32},
{"shra_r.w",	"",		"t,s,1,-B",	0x200002f5, 0xfc0003ff,	WR_1|RD_2,		0,	0,	D32},
{"shrl.ph",	"",		"t,s,4",	0x200003ff, 0xfc000fff,	WR_1|RD_2,		0,	0,	D32},
{"shrl.qb",	"",		"t,s,3",	0x2000187f, 0xfc001fff,	WR_1|RD_2,		0,	0,	D32},
{"shrlv.ph",	"",		"d,t,s,-B",	0x20000315, 0xfc0003ff, WR_1|RD_2|RD_3,		0,	0,	D32},
{"shrlv.qb",	"",		"d,t,s,-B",	0x20000355, 0xfc0003ff, WR_1|RD_2|RD_3,		0,	0,	D32},
{"shx", 	"",		"d,s(t)",	0x20000287, 0xfc0007ff, RD_1|RD_2|RD_3,		0,	0,	xNMS},
{"shxs",	"",		"d,s(t)",	0x200002c7, 0xfc0007ff, RD_1|RD_2|RD_3,		0,	0,	xNMS},
{"sync_wmb",	"",		"",		0x8004c006, 0xffffffff,		0,	INSN2_ALIAS,	I38,	0}, /* SYNC */
{"sync_mb",	"",		"",		0x8010c006, 0xffffffff,		0,	INSN2_ALIAS,	I38,	0}, /* SYNC */
{"sync_acquire", "",		"",		0x8011c006, 0xffffffff,		0,	INSN2_ALIAS,	I38,	0}, /* SYNC */
{"sync_release", "",		"",		0x8012c006, 0xffffffff,		0,	INSN2_ALIAS,	I38,	0}, /* SYNC */
{"sync_rmb",	"",		"",		0x8013c006, 0xffffffff,		0,	INSN2_ALIAS,	I38,	0}, /* SYNC */
{"sync_ginv",	"",		"",		0x8014c006, 0xffffffff,		0,	INSN2_ALIAS,	0,	GINV}, /* SYNC */
{"sync",	"",		"",		0x8000c006, 0xffffffff,		0,	INSN2_ALIAS,	I38,	0}, /* SYNC */
{"sync",	"",		"+i,-M",	0x8000c006, 0xffe0f1ff,		0,		0,	I38,	0},
{"sle", 	"",		"d,v,t",	0,	(int) M_SLE,	INSN_MACRO,		0,	I38,	0},
{"sle", 	"",		"d,v,I",	0,	(int) M_SLE_I,	INSN_MACRO,		0,	I38,	0},
{"sleu",	"",		"d,v,t",	0,	(int) M_SLEU,	INSN_MACRO,		0,	I38,	0},
{"sleu",	"",		"d,v,I",	0,	(int) M_SLEU_I,	INSN_MACRO,		0,	I38,	0},
{"sllv",	"",		"d,s,t,-B",	0x20000010, 0xfc0003ff, WR_1|RD_2|RD_3,		0,	I38,	0},
{"sll", 	"[16]", 	"md,mc,mM",	0x3000,		0xfc08,	WR_1|RD_2,		0,	I38,	0}, /* SLL[16] */
{"sll", 	"[32]", 	"t,r,<,-M",	0x8000c000, 0xfc00f1e0,	WR_1|RD_2,		0,	I38,	0}, /* preceded by EHB, PAUSE, SYNC */
{"sll", 	"[32]", 	"d,v,t",	0x20000010, 0xfc0007ff, WR_1|RD_2|RD_3,	INSN2_ALIAS,	I38,	0}, /* SLLV */
{"slt", 	"",		"d,v,t,-B",	0x20000350, 0xfc0003ff, WR_1|RD_2|RD_3,		0,	I38,	0},
{"slt", 	"",		"d,v,I",	0,	(int) M_SLT_I,	INSN_MACRO,		0,	I38,	0},
{"slti",	"",		"t,r,i",	0x80004000, 0xfc00f000,	WR_1|RD_2,		0,	I38,	0},
{"sltiu",	"",		"t,r,i",	0x80005000, 0xfc00f000,	WR_1|RD_2,		0,	I38,	0},
{"sltu",	"",		"d,v,t,-B",	0x20000390, 0xfc0003ff, WR_1|RD_2|RD_3,		0,	I38,	0}, /* preceded by DVP */
{"sltu",	"",		"d,v,I",	0,	(int) M_SLTU_I,	INSN_MACRO,		0,	I38,	0},
{"sne", 	"",		"d,v,t",	0,	(int) M_SNE,	INSN_MACRO,		0,	I38,	0},
{"sne", 	"",		"d,v,I",	0,	(int) M_SNE_I,	INSN_MACRO,		0,	I38,	0},
{"sov", 	"",		"d,v,t,-B",	0x200003d0, 0xfc0003ff, WR_1|RD_2|RD_3,		0,	I38,	0},
{"srav",	"",		"d,s,t,-B",	0x20000090, 0xfc0003ff, WR_1|RD_2|RD_3,		0,	I38,	0},
{"sra", 	"",		"t,r,<,-M",	0x8000c080, 0xfc00f1e0,	WR_1|RD_2,		0,	I38,	0},
{"sra", 	"",		"d,v,t",	0x20000090, 0xfc0007ff, WR_1|RD_2|RD_3,	INSN2_ALIAS,	I38,	0}, /* SRAV */
{"srlv",	"",		"d,s,t,-B",	0x20000050, 0xfc0003ff, WR_1|RD_2|RD_3,		0,	I38,	0},
{"srl", 	"[16]", 	"md,mc,mM",	0x3008, 	0xfc08,	WR_1|RD_2,		0,	I38,	0}, /* SRL[16] */
{"srl", 	"[32]", 	"t,r,<,-M",	0x8000c040, 0xfc00f1e0,	WR_1|RD_2,		0,	I38,	0},
{"srl", 	"[32]", 	"d,v,t",	0x20000050, 0xfc0007ff, WR_1|RD_2|RD_3,	INSN2_ALIAS,	I38,	0}, /* SRLV */
{"sub", 	"",		"d,v,t,-B",	0x20000190, 0xfc0003ff, WR_1|RD_2|RD_3,		0,	0,	xNMS},
{"sub", 	"",		"t,r,I",	0,	(int) M_SUB_I,	INSN_MACRO,		0,	0,	xNMS},
{"subq.ph",	"",		"d,s,t",	0x2000020d, 0xfc0007ff, WR_1|RD_2|RD_3,		0,	0,	D32},
{"subqh.ph",	"",		"d,s,t",	0x2000024d, 0xfc0007ff, WR_1|RD_2|RD_3,		0,	0,	D32},
{"subqh.w",	"",		"d,s,t",	0x2000028d, 0xfc0007ff, WR_1|RD_2|RD_3,		0,	0,	D32},
{"subqh_r.ph",	"",		"d,s,t",	0x2000064d, 0xfc0007ff, WR_1|RD_2|RD_3,		0,	0,	D32},
{"subqh_r.w",	"",		"d,s,t",	0x2000068d, 0xfc0007ff, WR_1|RD_2|RD_3,		0,	0,	D32},
{"subq_s.ph",	"",		"d,s,t",	0x2000060d, 0xfc0007ff, WR_1|RD_2|RD_3,		0,	0,	D32},
{"subq_s.w",	"",		"d,s,t,-B",	0x20000345, 0xfc0003ff, WR_1|RD_2|RD_3,		0,	0,	D32},
{"subu",	"[16]", 	"me,mc,md",	0xb001, 	0xfc01, WR_1|RD_2|RD_3,		0,	I38,	0}, /* SUBU[16] */
{"subu",	"[32]", 	"d,v,t,-B",	0x200001d0, 0xfc0003ff, WR_1|RD_2|RD_3,		0,	I38,	0},
{"subu",	"",		"d,v,I",	0,	(int) M_SUBU_I,	INSN_MACRO,		0,	I38,	0},
{"subu.ph",	"",		"d,s,t",	0x2000030d, 0xfc0007ff, WR_1|RD_2|RD_3,		0,	0,	D32},
{"subu.qb",	"",		"d,s,t",	0x200002cd, 0xfc0007ff, WR_1|RD_2|RD_3,		0,	0,	D32},
{"subuh.qb",	"",		"d,s,t",	0x2000034d, 0xfc0007ff, WR_1|RD_2|RD_3,		0,	0,	D32},
{"subuh_r.qb",	"",		"d,s,t",	0x2000074d, 0xfc0007ff, WR_1|RD_2|RD_3,		0,	0,	D32},
{"subu_s.ph",	"",		"d,s,t",	0x2000070d, 0xfc0007ff, WR_1|RD_2|RD_3,		0,	0,	D32},
{"subu_s.qb",	"",		"d,s,t",	0x200006cd, 0xfc0007ff, WR_1|RD_2|RD_3,		0,	0,	D32},
{"sw",		"[16]", 	"mm,mJ(ml)",	0x9400, 	0xfc00,	RD_1|RD_3,		0,	I38,	0}, /* SW[16] */
{"sw",		"[sp]", 	"mp,mR(ms)",	0xb400, 	0xfc00,	RD_1|RD_3,		0,	I38,	0}, /* SW[SP] */
{"sw",		"[4x4]",	"mw,mN(mv)",	0xf400, 	0xfc00,	RD_1|RD_3,		0,	0,	xNMS}, /* SW[4X4] */
{"sw",		"[gp16]",	"mm,mO(ma)",	0xd400, 	0xfc00,	RD_1|RD_3,		0,	I38,	0}, /* SW[GP16] */
{"sw",		"[gp]", 	"t,.(ma)",	0x40000003, 0xfc000003,	RD_1|RD_3,		0,	I38,	0}, /* SW[GP] */
{"sw",		"[gp16]",	"mm,mA(ma)",	0xd400, 	0xfc00,	RD_1|RD_3,		0,	I38,	0}, /* SW[GP16] */
{"sw",		"[u12]",	"t,o(b)",	0x84009000, 0xfc00f000,	RD_1|RD_3,		0,	I38,	0}, /* SW[U12] */
{"sw",		"[s9]", 	"t,+j(b)",	0xa4004800, 0xfc007f00,	RD_1|RD_3,		0,	I38,	0}, /* SW[S9] */
{"sw",		"",		"t,A(c)",	0,	 (int) M_SW_AC,	INSN_MACRO,		0,	I38,	0},
{"sw",		"",		"t,A(b)",	0,	(int) M_SWX_AB,	INSN_MACRO,		0,	0,	xNMS}, /* SWX */
{"swe", 	"",		"t,+j(b)",	0xa4004a00, 0xfc007f00,	RD_1|RD_3,		0,	0,	EVA},
{"swe", 	"",		"t,A(c)",	0,      (int) M_SWE_AC,	INSN_MACRO,		0,	0,	EVA},
{"swm", 	"",		"t,+j(b),|",	0xa4000c00, 0xfc000f00,	RD_1|RD_3,		0,	0,	xNMS}, /* SWM */
{"swpc",	"[48]", 	"mp,+S",	0x600f, 	0xfc1f,	WR_1,			0,	0,	xNMS}, /* SWPC[48] */
{"swx", 	"",		"d,s(t)",	0x20000487, 0xfc0007ff, RD_1|RD_2|RD_3,		0,	0,	xNMS},
{"swxs",	"",		"d,s(t)",	0x200004c7, 0xfc0007ff, RD_1|RD_2|RD_3,		0,	0,	xNMS},
{"syscall",	"[16]", 	"",		0x1008, 	0xffff,		0,	INSN2_ALIAS,	I38,	0}, /* SYSCALL[16] */
{"syscall",	"[16]", 	"mP",		0x1008, 	0xfffc,		0,		0,	I38,	0}, /* SYSCALL[16] */
{"syscall",	"[32]", 	"",		0x00080000, 0xffffffff,		0,	INSN2_ALIAS,	I38,	0},
{"syscall",	"[32]", 	"+M",		0x00080000, 0xfffc0000,		0,		0,	I38,	0},
{"teq", 	"",		"s,t",		0x20000000, 0xfc00ffff,	RD_1|RD_2,	INSN2_ALIAS,	0,	xNMS}, /* TEQ */
{"teq", 	"",		"s,t,^",	0x20000000, 0xfc0007ff,	RD_1|RD_2,		0,	0,	xNMS}, /* TEQ */
{"teq", 	"",		"s,I",		0, 	 (int) M_TEQ_I,	INSN_MACRO,		0,	0,	xNMS},
{"tne", 	"",		"s,t",		0x20000400, 0xfc00ffff,	RD_1|RD_2,	INSN2_ALIAS,	0,	xNMS}, /* TNE */
{"tne", 	"",		"s,t,^",	0x20000400, 0xfc0007ff,	RD_1|RD_2,		0,	0,	xNMS}, /* TNE */
{"tne", 	"",		"s,I",		0, 	 (int) M_TNE_I,	INSN_MACRO,		0,	0,	xNMS},
{"tlbinv",	"",		"-F",		0x2000077f, 0xfc00ffff,		0,		0,	0,	TLB},
{"tlbinvf",	"",		"-F",		0x2000177f, 0xfc00ffff,		0,		0,	0,	TLB},
{"tlbp",	"",		"-F",		0x2000037f, 0xfc00ffff,		0,		0,	0,	TLB},
{"tlbr",	"",		"-F",		0x2000137f, 0xfc00ffff,		0,		0,	0,	TLB},
{"tlbwi",	"",		"-F",		0x2000237f, 0xfc00ffff,		0,		0,	0,	TLB},
{"tlbwr",	"",		"-F",		0x2000337f, 0xfc00ffff,		0,		0,	0,	TLB},
{"ualh",	"",		"t,+j(b)",	0xa4002100, 0xfc007f00,	WR_1|RD_3,		0,	0,	xNMS},
{"ualw",	"",		"t,+j(b)",	0xa4001500, 0xfc007f00,	WR_1|RD_3,	INSN2_ALIAS,	0,	xNMS}, /* UALWM */
{"ualwm",	"",		"t,+j(b),|",	0xa4000500, 0xfc000f00,	WR_1|RD_3,		0,	0,	xNMS}, /* UALWM */
{"uash",	"",		"t,+j(b)",	0xa4002900, 0xfc007f00,	RD_1|RD_3,		0,	0,	xNMS},
{"uasw",	"",		"t,+j(b)",	0xa4001d00, 0xfc007f00,	RD_1|RD_3,	INSN2_ALIAS,	0,	xNMS}, /* UASWM */
{"uaswm",	"",		"t,+j(b),|",	0xa4000d00, 0xfc000f00,	RD_1|RD_3,		0,	0,	xNMS}, /* UASWM */
{"uld", 	"",		"t,A(c)",	0,	(int) M_ULD_AC,	INSN_MACRO,		0,	0,	xNMS},
{"ulh", 	"",		"t,A(c)",	0,	(int) M_ULH_AC,	INSN_MACRO,		0,	0,	xNMS},
{"ulw", 	"",		"t,A(c)",	0,	(int) M_ULW_AC,	INSN_MACRO,		0,	0,	xNMS},
{"usd", 	"",		"t,A(c)",	0,	(int) M_USD_AC,	INSN_MACRO,		0,	0,	xNMS},
{"ush", 	"",		"t,A(c)",	0,	(int) M_USH_AC,	INSN_MACRO,		0,	0,	xNMS},
{"usw", 	"",		"t,A(c)",	0,	(int) M_USW_AC,	INSN_MACRO,		0,	0,	xNMS},
{"wait",	"",		"",		0x2000c37f, 0xffffffff,		0,	INSN2_ALIAS,	I38,	0},
{"wait",	"",		"+L",		0x2000c37f, 0xfc00ffff,		0,		0,	I38,	0},
{"wrdsp",	"",		"t",		0x201fd67f, 0xfc1fffff,	RD_1,		INSN2_ALIAS,	0,	D32},
{"wrdsp",	"",		"t,8",		0x2000167f, 0xfc003fff,	RD_1,			0,	0,	D32},
{"wrpgpr",	"",		"t,r",		0x2000f17f, 0xfc00ffff,	WR_1|RD_2,		0,	I38,	0},
{"xor", 	"[16]", 	"md,mk,ml",	0x5004, 	0xfc0f,	WR_1|RD_3,	INSN2_ALIAS,	I38,	0}, /* XOR[16] */
{"xor", 	"[16]", 	"md,ml,mk",	0x5004, 	0xfc0f,	WR_1|RD_2,	INSN2_ALIAS,	I38,	0}, /* XOR[16] */
{"xor", 	"[16]", 	"md,ml",	0x5004, 	0xfc0f,	WR_1|RD_2,		0,	I38,	0}, /* XOR[16] */
{"xor", 	"[32]", 	"d,v,t,-B",	0x20000310, 0xfc0003ff, WR_1|RD_2|RD_3,		0,	I38,	0},
{"xor", 	"",		"t,r,I",	0,	(int) M_XOR_I,	INSN_MACRO,		0,	I38,	0},
{"xori",	"",		"t,r,g",	0x80001000, 0xfc00f000,	WR_1|RD_2,		0,	I38,	0},
{"yield",	"",		"s",		0x20000268, 0xffe0ffff,	RD_1,		INSN2_ALIAS,	0,	MT32},
{"yield",	"",		"t,s,-N",	0x20000268, 0xfc0003ff,	WR_1|RD_2,		0,	0,	MT32},
};

const int bfd_nanomips_num_opcodes =
  ((sizeof nanomips_opcodes) / (sizeof (nanomips_opcodes[0])));
