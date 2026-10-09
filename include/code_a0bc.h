#ifndef CODE_A0BC_H
#define CODE_A0BC_H

/**
 * @file code_a0bc.h
 * @brief What other units call in code_a0bc: the sprite drawer and the
 *        ordering tables it draws into.
 */

#include "common.h"
#include "libgte.h"
#include "libgpu.h"
#include "libgs.h"

extern GsOT D_800ACEA8[]; /**< the ordering tables the sprite drawer is handed */

/** @brief Draws a sprite into an ordering table.
 *  @param id    which sprite
 *  @param size  its size
 *  @param color its colour
 *  @param mode  how it is drawn
 *  @param ot    the ordering table */
void func_8001A3D4(s32 id, SVECTOR *size, CVECTOR *color, s32 mode, GsOT *ot);

#endif
