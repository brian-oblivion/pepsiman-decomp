#ifndef SPAD_H
#define SPAD_H

/**
 * @file spad.h
 * @brief Running a call on the scratchpad stack, as Psy-Q's SetSpadStack and
 *        ResetSpadStack sample macros do.
 */

/**
 * Saves the stack pointer at the scratchpad top and moves the stack below it.
 * Sony's own form of the macro: the scratchpad address is an input operand
 * and the asm declares what it overwrites.
 */
#define SetSpadStack()                                                              \
    __asm__ volatile("move $8,%0\n\tsw $29,0($8)\n\taddiu $8,$8,-24\n\tmove $29,$8" \
                     :                                                              \
                     : "r"(0x1F8003FC)                                              \
                     : "$8", "memory")

/** Restores the stack pointer that SetSpadStack saved. */
#define ResetSpadStack() __asm__ volatile("addiu $29,$29,24\n\tlw $sp,0($sp)" : : : "memory")

#endif
