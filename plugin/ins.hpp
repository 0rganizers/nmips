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
    case nMIPS_bnec:

#endif /* __INS_H */
