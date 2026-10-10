#ifndef CODE_7D74_H
#define CODE_7D74_H

/**
 * @file code_7d74.h
 * @brief What other units call in code_7d74 that needs Sony's types.
 */

#include "common.h"
#include "libgte.h"
#include "libgpu.h"
#include "libgs.h"

/** @brief A TIM image's header, unpacked: where its CLUT and pixels go. */
typedef struct {
    s16 mode;      /**< pixel mode, the low 3 bits of the TIM flags */
    s16 hasClut;   /**< 1 when the TIM carries a CLUT */
    u32 *clut;     /**< the CLUT's pixel data */
    RECT clutRect; /**< where the CLUT goes in VRAM */
    u32 *pixel;    /**< the image's pixel data */
    RECT pixRect;  /**< where the image goes in VRAM */
    s16 unk1C;     /**< the image width scaled by the pixel mode */
    s16 unk1E;     /**< 4, 2 or not set, by pixel mode */
} TimInfo;

/** @brief Rebuilds a coordinate system's matrix from a rotation, keeping
 *         its translation, and marks it changed.
 *  @param rot   the rotation (YXZ order)
 *  @param coord the coordinate system rewritten */
void func_80018AE0(SVECTOR *rot, GsCOORDINATE2 *coord);

/** @brief Loads the image file at `data`.
 *  @param data the file in memory
 *  @return not yet known */
s32 func_80017774(void *data);

/** @brief Defined in code_7d74: sorts a semi-transparent flat quad in a
 *         15-bit colour into the current ordering table.
 *  @param col a 15-bit colour, red in the low bits
 *  @param x0 the first corner's x
 *  @param y0 the first corner's y
 *  @param x1 the second corner's x
 *  @param y1 the second corner's y
 *  @param x2 the third corner's x, drawn last
 *  @param y2 the third corner's y, drawn last
 *  @param x3 the fourth corner's x, drawn third
 *  @param y3 the fourth corner's y, drawn third
 *  @param pri the ordering-table priority */
void func_800179F8(u16 col, s16 x0, s16 y0, s16 x1, s16 y1, s16 x2, s16 y2, s16 x3, s16 y3, u16 pri);

#endif
