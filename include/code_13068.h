#ifndef CODE_13068_H
#define CODE_13068_H

/**
 * @file code_13068.h
 * @brief What other units call in code_13068 that needs libgte's types.
 */

#include "common.h"
#include "libgte.h"

/** @brief Projects a world position to the screen.
 *  @param pos the world position
 *  @param out the screen position */
void func_800230E0(VECTOR *pos, SVECTOR *out);

extern SVECTOR D_800A7680[]; /**< rotations; [0].vy is a yaw (heading) */

#endif
