#include "reg.hpp"
#include "nmips.hpp"
#include "pro.h"
#include "typeinf.hpp"

const int nmips_argregs[] = { A0, A1, A2, A3, A4, A5, A6, A7, -1 };

int get_arg_regs(const int **regs)
{
    *regs = nmips_argregs;
    return qnumber(nmips_argregs) - 1;
}

void add_argregs(argloc_t *argloc, int r, int nregs, int size, bool force_scattered)
{
    QASSERT(10306, size > (nregs-1) * 4);
    QASSERT(10307, r + nregs < qnumber(nmips_argregs));

    if ( force_scattered || (nregs >= 2 && size != 8) )
    {
        scattered_aloc_t *scloc = new scattered_aloc_t;
        int off = 0;
        for ( int i = 0; i < nregs; ++i, ++r, off += 4 )
        {
            argpart_t &regloc = scloc->push_back();
            regloc.off = off;
            regloc.set_reg1(nmips_argregs[r]);
            regloc.size = qmin(size, 4);
            size -= 4;
        }
        argloc->consume_scattered(scloc);
    }
    else if ( size == 8 )
    {
        argloc->set_reg2(nmips_argregs[r], nmips_argregs[r+1]);
    }
    else
    {
        argloc->set_reg1(nmips_argregs[r]);
    }
}

void add_argstack(argloc_t *argloc, int size, int stk_off)
{
    argloc->set_stkoff(stk_off);
}

bool calc_retloc(argloc_t *retloc, const tinfo_t &tif, cm_t, bool& ptr_in_arg)
{
    ptr_in_arg = false;
    if (!tif.is_void())
    {
        int size = tif.get_size();
        int nregs = NUM_REGS(size);
        if (nregs > 2) // only a0, a1 allowed for return value!
        {
            // returned by pointer passed in first argument!
            auto rrel = new rrel_t;
            rrel->off = 0;
            rrel->reg = A0;
            retloc->consume_rrel(rrel);
            ptr_in_arg = true;
        }
        add_argregs(retloc, 0, nregs, size, false);
    }
    return true;
}

bool alloc_args(func_type_data_t *fti, int nfixed)
{
    bool ptr_in_arg = false;
    if ( !calc_retloc(&fti->retloc, fti->rettype, 0 /*fti->get_cc()*/, ptr_in_arg) )
        return false;

    int r = 0;
    const int NUMREGARGS = qnumber(nmips_argregs) - 1;
    if (ptr_in_arg) r++;

    sval_t spoff = 0;
    for (size_t i = 0; i < fti->size(); i++)
    {
        size_t size;
        funcarg_t &fa = fti->at(i);
        const tinfo_t &type = fa.type;
        size = type.get_size();
        // always align to 4 bytes!
        size = align_up(size, 4);
        int nregs = NUM_REGS(size);

        if (nregs <= NUMREGARGS - r) // enough to pass via register arguments
        {
            add_argregs(&fa.argloc, r, nregs, size, false);
            r += nregs;
        } else {
            add_argstack(&fa.argloc, size, spoff);
            spoff += size;
        }
    }

    return true;
}

bool calc_arglocs(func_type_data_t *fti)
{
  return alloc_args(fti, fti->size());
}

//-------------------------------------------------------------------------
bool calc_varglocs(
        func_type_data_t *fti,
        regobjs_t * /*regargs*/,
        int nfixed)
{
    return alloc_args(fti, nfixed);
}