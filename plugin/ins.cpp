#include "ins.hpp"
#include "constants.hpp"
#include "idp.hpp"
#include <vector>

#define EXTRA_INSN(name, features) {nMIPS_ ## name, {nMIPS_ ## name, #name, features}}

std::map<nanomips_extra_inst_t, nanomips_insn_t> nanomips_insn = {
    EXTRA_INSN(bc, CF_JUMP),

    EXTRA_INSN(beqic, CF_JUMP | CCF_COND),
    EXTRA_INSN(bgeic, CF_JUMP | CCF_COND),
    EXTRA_INSN(bgeiuc, CF_JUMP | CCF_COND),
    EXTRA_INSN(bltic, CF_JUMP | CCF_COND),
    EXTRA_INSN(bltiuc, CF_JUMP | CCF_COND),
    EXTRA_INSN(bneic, CF_JUMP | CCF_COND),

    EXTRA_INSN(bltc, CF_JUMP | CCF_COND),
    EXTRA_INSN(bltuc, CF_JUMP | CCF_COND),
    EXTRA_INSN(bgec, CF_JUMP | CCF_COND),
    EXTRA_INSN(bgeuc, CF_JUMP | CCF_COND),
    EXTRA_INSN(beqc, CF_JUMP | CCF_COND),
    EXTRA_INSN(bnec, CF_JUMP | CCF_COND),

    EXTRA_INSN(bgezc, CF_JUMP | CCF_COND),
    EXTRA_INSN(blezc, CF_JUMP | CCF_COND),

    EXTRA_INSN(muh, 0),

    EXTRA_INSN(align, 0),

    EXTRA_INSN(bbeqzc, CF_JUMP | CCF_COND),
    EXTRA_INSN(bbneqzc, CF_JUMP | CCF_COND),

    
};