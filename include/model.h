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

/** @brief A TIM image's pixel mode, TimInfo.mode. */
enum TimMode {
    TIM_4BIT = 0,  /**< 4-bit indexed */
    TIM_8BIT = 1,  /**< 8-bit indexed */
    TIM_16BIT = 2, /**< 16-bit direct */
    TIM_24BIT = 3  /**< 24-bit direct */
};

/** @brief A TIM image's header, unpacked: where its CLUT and pixels go. */
typedef struct {
    s16 mode;      /**< a TimMode, the low 3 bits of the TIM flags */
    s16 hasClut;   /**< 1 when the TIM carries a CLUT */
    u32 *clut;     /**< the CLUT's pixel data */
    RECT clutRect; /**< where the CLUT goes in VRAM */
    u32 *pixel;    /**< the image's pixel data */
    RECT pixRect;  /**< where the image goes in VRAM */
    s16 width;     /**< the image width in pixels */
    s16 perUnit;   /**< pixels per VRAM halfword: 4 or 2; not set for 16-bit */
} TimInfo;

/** @brief Steps one animation channel of @p obj: restarts it when another
 *         motion is asked for, then advances its TOD sequence.
 *  @param obj   the animated state (gPlayer or a copy)
 *  @param index the channel, 0 or 1
 *  @param arg   SetTodFrame's last argument
 *  @return 0 once the motion has stopped, 2 at a loop's end, else 1 */
u8 StepAnimation(PlayerState *obj, u16 index, u8 arg);

/** @brief Applies a TOD frame's packets once @p time reaches it.
 *  @param time     the channel's clock
 *  @param data     the frame: its packet count, its time, then the packets
 *  @param objTable the object table the packets act on
 *  @param tmdIds   the TMD id table
 *  @param tmdTable the TMD data the objects are linked to
 *  @param mode     handed to each packet
 *  @param filter   the packet filter
 *  @return the next frame, or @p data while @p time is before it */
u8 *SetTodFrame(u32 time, u8 *data, s32 objTable, s32 tmdIds, s32 tmdTable, s32 mode, s8 filter);

extern s32 gTodTmdIds[]; /**< SetTodFrame's TMD id table */

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
