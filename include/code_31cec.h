#ifndef CODE_31CEC_H
#define CODE_31CEC_H

/**
 * @file code_31cec.h
 * @brief What other units call in code_31cec, the movie player, that needs
 *        libcd's types.
 */

#include "common.h"
#include "libcd.h"

/** @brief Defined in code_31cec.
 *  @param arg0 not yet known */
void func_800414EC(s16 arg0);

/** @brief Defined in code_31cec: starts a movie.
 *  @param name the movie file's name
 *  @param loc  where the file is on the disc
 *  @param arg2 not yet known
 *  @param arg3 not yet known
 *  @param arg4 not yet known */
void func_80041BAC(char *name, CdlLOC *loc, s32 arg2, s16 arg3, s16 arg4);

/** @brief Defined in code_31cec. */
void func_80042208(void);

#endif
