#ifndef __MOPT_H
#define __MOPT_H

#include "pro.h"
#include <hexrays.hpp>

struct constant_folding_visitor_t : public mop_visitor_t
{
public:
    int count = 0;
    virtual int idaapi visit_mop(mop_t *op, const tinfo_t *type, bool is_target) override;

private:
    qvector<minsn_t> operations;

    void add_op(mcode_t op, mop_t val);
    void unwrap_tree(mop_t top, bool invert);
    void append_mop(mop_t* dest, mop_t l, mcode_t);
};

struct large_stk_mop_visitor_t : public mop_visitor_t
{
public:
    int count = 0;
    func_t* func = nullptr;
    virtual int idaapi visit_mop(mop_t *op, const tinfo_t *type, bool is_target) override;
};

struct large_stk_opt_t : public optinsn_t
{
public:
    /*large_stk_opt_t();
    virtual ~large_stk_opt_t();*/
    /// Optimize an instruction.
    /// \param blk current basic block. maybe NULL, which means that
    ///            the instruction must be optimized without context
    /// \param ins instruction to optimize; it is always a top-level instruction.
    ///            the callback may not delete the instruction but may
    ///            convert it into nop (see mblock_t::make_nop). to optimize
    ///            sub-instructions, visit them using minsn_visitor_t.
    ///            sub-instructions may not be converted into nop but
    ///            can be converted to "mov x,x". for example:
    ///               add x,0,x => mov x,x
    ///            this callback may change other instructions in the block,
    ///            but should do this with care, e.g. to no break the
    ///            propagation algorithm if called with OPTI_NO_LDXOPT.
    /// \param optflags combination of \ref OPTI_ bits
    /// \return number of changes made to the instruction.
    ///         if after this call the instruction's use/def lists have changed,
    ///         you must mark the block level lists as dirty (see mark_lists_dirty)
    virtual int idaapi func(mblock_t *blk, minsn_t *ins, int optflags) override;
};

#endif /* __MOPT_H */
