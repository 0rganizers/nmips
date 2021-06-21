#ifndef __CONSTANTS_H
#define __CONSTANTS_H

/**
 * Maximum number of operands for a single instruction.
 * Same as IDA.
 **/
#define MAX_NUM_OPS 8

/**
 * Offset in file where the machine information is located.
 * */
#define ELF_MACHINE_OFFSET 0x12

#define ELF_NANOMIPS 0xf9

#define CCF_COND 0x200000

#endif /* __CONSTANTS_H */
