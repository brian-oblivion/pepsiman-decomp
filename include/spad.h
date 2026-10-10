#ifndef SPAD_H
#define SPAD_H

/**
 * @file spad.h
 * @brief Running a call on the scratchpad stack, as Psy-Q's SetSpadStack and
 *        ResetSpadStack sample macros do.
 */

/* Retail moves the stack pointer to the top of the 1 KiB scratchpad around a
 * deep call (GsSortObject4J) and back afterwards. That has no C spelling, so
 * these are plain __asm__ sequences with no operands: the one sanctioned use
 * besides include/gte.h (CLAUDE.md, hard rule 7). They use $8/$9 as retail
 * does without telling cc1, so nothing may be live in those registers across
 * them; the byte match is what checks it. */

/** Saves the stack pointer at the scratchpad top and moves the stack below it. */
#define SetSpadStack()                                                                    \
    __asm__ volatile("lui $9,0x1F80\n\tori $9,$9,0x3FC\n\tmove $8,$9\n\tsw $29,0($8)\n\t" \
                     "addiu $8,$8,-24\n\tmove $29,$8")

/** Restores the stack pointer that SetSpadStack saved. */
#define ResetSpadStack() __asm__ volatile("addiu $29,$29,24\n\tlw $29,0($29)")

#endif
