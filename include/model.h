#ifndef MODEL_H
#define MODEL_H

/**
 * @file model.h
 * @brief What other units call in model that needs Sony's types.
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
    s16 width;     /**< the image width in pixels */
    s16 perUnit;   /**< pixels per VRAM halfword: 4 or 2; not set for 16-bit */
} TimInfo;

/** @brief Rebuilds a coordinate system's matrix from a rotation, keeping
 *         its translation, and marks it changed.
 *  @param rot   the rotation (YXZ order)
 *  @param coord the coordinate system rewritten */
void SetCoordRotation(SVECTOR *rot, GsCOORDINATE2 *coord);

/** @brief Loads the image file at `data`.
 *  @param data the file in memory
 *  @return not yet known */
s32 LoadTim(void *data);

/** @brief Defined in model: sorts a semi-transparent flat quad in a
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
void SortSemiQuad(u16 col, s16 x0, s16 y0, s16 x1, s16 y1, s16 x2, s16 y2, s16 x3, s16 y3, u16 pri);

#endif
