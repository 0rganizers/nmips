#ifndef __REG_H
#define __REG_H

#include <pro.h>
#include <typeinf.hpp>

#define NUM_REGS(size) ((size + 3) / 4)

enum RegNo
{
    Zero, At, T4, T5, A0, A1, A2, A3,
    A4, A5, A6, A7, T0, T1, T2, T3,
    S0, S1, S2, S3, S4, S5, S6, S7,
    T8, T9, K0, K1, GP, SP, FP, RA
};

/**
 * @brief  Get a -1 terminated array of the registers used for arguments.
 * @note   
 * @param  **regs: 
 * @retval Number of actual registers (e.g. sizeof(regs) - 1).
 */
int get_arg_regs(const int **regs);

/**
 * @brief  Add registers to argloc.
 * @note   
 * @param  *argloc: 
 * @param  r: Starting register.
 * @param  nregs: Number of registers to use.
 * @param  size: Total size of type.
 * @param  force_scattered: Whether we should force scattered or not.
 * @retval None
 */
void add_argregs(argloc_t *argloc, int r, int nregs, int size, bool force_scattered);

/**
 * @brief  Position argloc on the stack.
 * @note   
 * @param  *argloc: 
 * @param  size: 
 * @param  stk_off: Indicates the offset from the base, left by any previous arguments on the stack.
 * @retval None
 */
void add_argstack(argloc_t *argloc, int size, int stk_off);

/**
 * @brief  Calculates the location of the return argument.
 * @note   
 * @param  *retloc: 
 * @param  &tif: 
 * @retval Success or failure.
 */
bool calc_retloc(argloc_t *retloc, const tinfo_t &tif, cm_t /*cc*/, bool& ptr_in_arg);

/**
 * @brief  Allocates the arguments for the function.
 * @note   
 * @param  *fti: 
 * @param  nfixed: 
 * @retval 
 */
bool alloc_args(func_type_data_t *fti, int nfixed);

bool calc_arglocs(func_type_data_t *fti);

// Not needed, since no special params passed.
bool calc_varglocs(
        func_type_data_t *fti,
        regobjs_t * /*regargs*/,
        int nfixed);
#endif /* __REG_H */
