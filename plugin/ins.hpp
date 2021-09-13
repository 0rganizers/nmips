#ifndef __INS_H
#define __INS_H

#include <idp.hpp>
#include <map>
#include <string>

enum nanomips_extra_inst_t : uint16
{
    nMIPS_todo = CUSTOM_INSN_ITYPE,

    // save / restore
    nMIPS_save,
    nMIPS_restore_jrc,

    // branch instructions
    nMIPS_bc,
    nMIPS_beqic,
    nMIPS_bgeic,
    nMIPS_bgeiuc,
    nMIPS_bltic,
    nMIPS_bltiuc,
    nMIPS_bneic,

    nMIPS_bltc,
    nMIPS_bltuc,
    nMIPS_bgec,
    nMIPS_bgeuc,
    nMIPS_beqc,
    nMIPS_bnec,

    nMIPS_bgezc,
    nMIPS_blezc,

    // combined instructions
    nMIPS_move_balc,

    // Math ops
    nMIPS_muh,

    // TODO: Actually implement these!
    nMIPS_align,

    nMIPS_bbeqzc,
    nMIPS_bbneqzc,

    nMIPS_bitrevb,
    nMIPS_bitrevw,
    nMIPS_bitswap,
    nMIPS_byterevh,
    nMIPS_byterevw,

    nMIPS_crc32b,
    nMIPS_crc32cb,
    nMIPS_crc32ch,
    nMIPS_crc32cw,
    nMIPS_crc32h,
    nMIPS_crc32w,

    nMIPS_extw,
    nMIPS_ginvi,
    nMIPS_ginvt,

    nMIPS_lhuxs,
    nMIPS_lhxs,
    nMIPS_llwp,
    nMIPS_llwpe,

    nMIPS_mfhc0,
    nMIPS_mthc0,

    nMIPS_mod,
    nMIPS_modu,
    nMIPS_muhu,
    nMIPS_mulu,

    nMIPS_rotx,

    nMIPS_sbx,
    nMIPS_scwp,
    nMIPS_scwpe,
    nMIPS_shx,
    nMIPS_shxs,
    nMIPS_swx,
    nMIPS_swxs,

    nMIPS_sigrie,

    nMIPS_sov,

    nMIPS_tlbinv,
    nMIPS_tlbinvf,
};

struct nanomips_insn_t
{
    nanomips_extra_inst_t itype;
    const char* mnemonic;
    uint32 features;
};

extern std::map<nanomips_extra_inst_t, nanomips_insn_t> nanomips_insn;

#define CASE_BRANCH_COND     case nMIPS_bltic: \
    case nMIPS_bltiuc: \
    case nMIPS_beqic: \
    case nMIPS_bgeic: \
    case nMIPS_bgeiuc: \
    case nMIPS_bneic: \
    case nMIPS_bltc: \
    case nMIPS_bltuc: \
    case nMIPS_bgec: \
    case nMIPS_bgeuc: \
    case nMIPS_beqc: \
    case nMIPS_bnec: \
    case nMIPS_bbeqzc: \
    case nMIPS_bbneqzc:

#endif /* __INS_H */
