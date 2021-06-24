#include "mopt.hpp"
#include "funcs.hpp"
#include "hexrays.hpp"
#include "log.hpp"
#include "pro.h"
#include "reg.hpp"
#include "ua.hpp"
#include "struct.hpp"
#include "frame.hpp"

#define OPROP_VISITED 0x40

int constant_folding_visitor_t::visit_mop(mop_t *op, const tinfo_t *type, bool is_target)
{
    operations.clear();
    if ((op->oprops & OPROP_VISITED) == OPROP_VISITED) return 0;
    if (!op->is_insn(m_add) && !op->is_insn(m_sub)) return 0;
    unwrap_tree(*op, false);

    sval_t const_val = 0;

    for (auto &elem : operations)
    {
        uint64 val = 0;
        if (elem.l.is_constant(&val))
        {
            sval_t trunc_val = (sval_t)val;
            if (elem.opcode == m_sub) trunc_val = -trunc_val;
            const_val += trunc_val;
        }
    }

    if (const_val == 0) return 0;

    // LOG("[0x%x] calculated constant: 0x%x", curins->ea, const_val);

    mop_t* parent = op;
    mop_t const_op;
    const_op.make_number(const_val, 4);
    append_mop(parent, const_op, m_add);
    parent = &parent->d->r;

    for (auto &elem : operations)
    {
        if (!elem.l.is_constant())
        {
            append_mop(parent, elem.l, elem.opcode);
            parent = &parent->d->r;
            count++;
        }
    }
    op->oprops |= OPROP_VISITED;
    parent->make_number(0, 4); // make empty constant to finish this.

    return 0;
}

void constant_folding_visitor_t::add_op(mcode_t op, mop_t val)
{
    minsn_t insert(curins->ea);
    insert.opcode = op;
    insert.l = val;
    operations.push_back(insert);
}

void constant_folding_visitor_t::unwrap_tree(mop_t top, bool invert)
{
    // if we need to invert, sub, otherwise add
    mcode_t operation_c = invert ? m_sub : m_add;
    // constant value
    if (top.is_constant())
    {
        add_op(operation_c, top);
    } else if (top.is_insn())
    {
        minsn_t* ins = top.d;
        if (ins->opcode == m_add)
        {
            unwrap_tree(ins->l, invert);
            unwrap_tree(ins->r, invert);
        } else if (ins->opcode == m_sub)
        {
            unwrap_tree(ins->l, invert);
            unwrap_tree(ins->r, !invert);
        } else {
            add_op(operation_c, top);
        }
    } else {
        add_op(operation_c, top);
    }
}

void constant_folding_visitor_t::append_mop(mop_t* dest, mop_t l, mcode_t c)
{
    minsn_t comb(curins->ea);
    comb.opcode = m_add;
    if (c == m_sub)
    {
        l.apply_ld_mcode(m_neg, curins->ea, 4);
    }
    comb.l = l;
    comb.d.erase();
    comb.d.size = 4;
    dest->create_from_insn(&comb);
}

int large_stk_mop_visitor_t::visit_mop(mop_t *op, const tinfo_t *type, bool is_target)
{
    // Turn all stack var references to var_10, back to stack pointer calculations
    // this way, we can first do our own constant folding + algebraic simplification and should get correct stack vars!
    if (op->t == mop_a && op->a->t == mop_S)
    {
        uval_t p_off = 0;
        op->a->get_stkvar(&p_off);
        sval_t frame_off = soff_to_fpoff(func, p_off);
        // this is an actual reference to var_10!
        if (frame_off == -0x10)
        {
            minsn_t sp_calc(curins->ea);
            sp_calc.opcode = m_add;
            sp_calc.l.make_reg(reg2mreg(SP), 4);
            sval_t spd = get_spd(func, curins->ea);
            if (spd < 0) spd = -spd;
            sp_calc.r.make_number(spd - 0x10, 4);
            sp_calc.d.erase();
            sp_calc.d.size = 4;
            op->create_from_insn(&sp_calc);
            prune = true;
            count++;
        }
    }
    return 0;
}

#define IPROP_VISITED (1 << 28)

int large_stk_opt_t::func(mblock_t *blk, minsn_t *ins, int optflags)
{
    if ((ins->iprops & IPROP_VISITED) == IPROP_VISITED) return 0;
    ea_t loc = 0;
    loc = ins->ea;
    if (blk == NULL)
    {
        LOG("[0x%x] blk == NULL", loc);
        return 0;
    }
    if (blk->mba->maturity != MMAT_GLBOPT1)
    {
        return 0;
    }

    // LOG("[0x%x] running for insn", loc);

    func_t* func = get_func(blk->mba->entry_ea);

    large_stk_mop_visitor_t visitor;
    visitor.blk = blk;
    visitor.func = func;
    ins->for_all_ops(visitor);

    constant_folding_visitor_t const_visitor;
    ins->for_all_ops(const_visitor);

    int count = visitor.count + const_visitor.count;
    
    if (count > 0)
    {
        blk->mark_lists_dirty();
    }
    ins->iprops |= IPROP_VISITED;
    return count;
}