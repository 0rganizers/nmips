#ifndef __MGEN_H
#define __MGEN_H

#include <pro.h>
#include <hexrays.hpp>

class nmips_microcode_gen_t : public microcode_filter_t
{
public:
    nmips_microcode_gen_t();
    virtual ~nmips_microcode_gen_t();

    /// check if the filter object is to be appied
    /// \return success
    bool match(codegen_t &cdg) override;

    /// generate microcode for an instruction
    /// \return MERR_... code:
    ///   MERR_OK      - user-defined call generated, go to the next instruction
    ///   MERR_INSN    - not generated - the caller should try the standard way
    ///   else         - error
    merror_t apply(codegen_t &cdg) override;

private:
    // temps, to be used by simple emit version
    qvector<mreg_t> temps;

    mreg_t get_temp(int idx);
    mop_t get_mtemp(int idx, int width);

};

#endif /* __MGEN_H */
