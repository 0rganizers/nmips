/* pls.c.  make binutils happy lmao.
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
#include "bfd.h"
#include "disassemble.h"
#include "safe-ctype.h"
#include <assert.h>

// #include "libbfd.h"

bfd_vma
bfd_getb16 (const void *p)
{
  const bfd_byte *addr = (const bfd_byte *) p;
  return (addr[0] << 8) | addr[1];
}

bfd_vma
bfd_getl16 (const void *p)
{
  const bfd_byte *addr = (const bfd_byte *) p;
  return (addr[1] << 8) | addr[0];
}

void
disassemble_init_for_target (struct disassemble_info * info)
{
  if (info == NULL)
    return;

  switch (info->arch)
    {
#ifdef ARCH_aarch64
    case bfd_arch_aarch64:
      info->symbol_is_valid = aarch64_symbol_is_valid;
      info->disassembler_needs_relocs = TRUE;
      break;
#endif
#ifdef ARCH_arm
    case bfd_arch_arm:
      info->symbol_is_valid = arm_symbol_is_valid;
      info->disassembler_needs_relocs = TRUE;
      break;
#endif
#ifdef ARCH_ia64
    case bfd_arch_ia64:
      info->skip_zeroes = 16;
      break;
#endif
#ifdef ARCH_tic4x
    case bfd_arch_tic4x:
      info->skip_zeroes = 32;
      break;
#endif
#ifdef ARCH_mep
    case bfd_arch_mep:
      info->skip_zeroes = 256;
      info->skip_zeroes_at_end = 0;
      break;
#endif
#ifdef ARCH_metag
    case bfd_arch_metag:
      info->disassembler_needs_relocs = TRUE;
      break;
#endif
#ifdef ARCH_m32c
    case bfd_arch_m32c:
      /* This processor in fact is little endian.  The value set here
	 reflects the way opcodes are written in the cgen description.  */
      info->endian = BFD_ENDIAN_BIG;
      if (! info->insn_sets)
	{
	  info->insn_sets = cgen_bitset_create (ISA_MAX);
	  if (info->mach == bfd_mach_m16c)
	    cgen_bitset_set (info->insn_sets, ISA_M16C);
	  else
	    cgen_bitset_set (info->insn_sets, ISA_M32C);
	}
      break;
#endif
#ifdef ARCH_powerpc
    case bfd_arch_powerpc:
#endif
#ifdef ARCH_rs6000
    case bfd_arch_rs6000:
#endif
#if defined (ARCH_powerpc) || defined (ARCH_rs6000)
      disassemble_init_powerpc (info);
      break;
#endif
#ifdef ARCH_wasm32
    case bfd_arch_wasm32:
      disassemble_init_wasm32 (info);
      break;
#endif
#ifdef ARCH_s390
    case bfd_arch_s390:
      disassemble_init_s390 (info);
      break;
#endif
#ifdef ARCH_nanomips
    case bfd_arch_nanomips:
      info->disassembler_needs_relocs = TRUE;
      info->predict_insn_length = nanomips_predict_insn_length;
      break;
#endif
    default:
      break;
    }
}

void
init_disassemble_info (struct disassemble_info *info, void *stream,
		       fprintf_ftype fprintf_func)
{
  memset (info, 0, sizeof (*info));

  info->flavour = bfd_target_unknown_flavour;
  info->arch = bfd_arch_unknown;
  info->endian = BFD_ENDIAN_UNKNOWN;
  info->endian_code = info->endian;
  info->octets_per_byte = 1;
  info->fprintf_func = fprintf_func;
  info->stream = stream;
  // info->read_memory_func = buffer_read_memory;
  // info->memory_error_func = perror_memory;
  // info->print_address_func = generic_print_address;
  // info->symbol_at_address_func = generic_symbol_at_address;
  // info->symbol_is_valid = generic_symbol_is_valid;
  // info->predict_insn_length = generic_predict_insn_length;
  info->display_endian = BFD_ENDIAN_UNKNOWN;
}