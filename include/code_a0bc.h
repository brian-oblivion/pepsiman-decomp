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
extern GsOT D_800A7318[]; /**< a second pair of ordering tables, cleared to depth 300 */
extern GsOT *D_80095884;  /**< this frame's entry of the second pair (set in main) */
extern MATRIX D_800E4858; /**< the rotation matrix the billboard sprites are drawn with */

/** @brief Draws a sprite as a billboard centred on the current
 *         transformation's origin, into an ordering table.
 *  @param id    which sprite
 *  @param size  its size
 *  @param color its colour, or NULL
 *  @param shift how far its depth is shifted down to an OT index
 *  @param ot    the ordering table */
void func_8001A3D4(u16 id, SVECTOR *size, CVECTOR *color, s32 shift, GsOT *ot);

/** @brief Draws a sprite as a billboard standing on the current
 *         transformation's origin (bottom edge there, centred across),
 *         into an ordering table.
 *  @param id    which sprite
 *  @param size  its size
 *  @param color its colour, or NULL
 *  @param shift how far its depth is shifted down to an OT index
 *  @param ot    the ordering table */
void func_8001A69C(u16 id, SVECTOR *size, CVECTOR *color, s32 shift, GsOT *ot);

/** @brief Draws a numbered sprite at a screen position into an ordering
 *         table.
 *  @param id    which sprite
 *  @param pos   its screen position
 *  @param color its colour, or NULL
 *  @param mode  how it is drawn
 *  @param ot    the ordering table */
void func_8001B354(u16 id, SVECTOR *pos, CVECTOR *color, s32 mode, GsOT *ot);

/** @brief Installs the TMD primitive handlers for a drawing mode.
 *  @param mode which handler set
 *  @return undefined: the definition has no return statement */
s32 func_80020CF8(s32 mode);

#endif
