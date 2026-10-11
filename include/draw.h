#ifndef DRAW_H
#define DRAW_H

/**
 * @file draw.h
 * @brief What other units call in draw: the sprite drawer and the
 *        ordering tables it draws into.
 */

#include "common.h"
#include "libgte.h"
#include "libgpu.h"
#include "libgs.h"

/** @brief A sprite colour's first byte (CVECTOR.r): how the sprite blends
 *         with what is behind it, the GPU's semi-transparency mode. The
 *         other three bytes (g, b, cd) carry its red, green and blue. */
enum SpriteBlend {
    BLEND_OPAQUE = -1, /**< not semi-transparent */
    BLEND_HALF = 0,    /**< half the sprite plus half the background */
    BLEND_ADD = 1,     /**< added to the background */
    BLEND_SUB = 2      /**< subtracted from the background */
};

extern GsOT gSubOt[]; /**< the ordering tables the sprite drawer is handed */
extern GsOT gOt[];    /**< a second pair of ordering tables, cleared to depth 300 */
extern GsOT *gCurOt;  /**< this frame's entry of the second pair (set in main) */

/** @brief Draws a sprite as a billboard centred on the current
 *         transformation's origin, into an ordering table.
 *  @param id    which sprite
 *  @param size  its size
 *  @param color its colour, or NULL
 *  @param shift how far its depth is shifted down to an OT index
 *  @param ot    the ordering table */
void DrawBillboard(u16 id, SVECTOR *size, CVECTOR *color, s32 shift, GsOT *ot);

/** @brief Draws a sprite as a billboard standing on the current
 *         transformation's origin (bottom edge there, centred across),
 *         into an ordering table.
 *  @param id    which sprite
 *  @param size  its size
 *  @param color its colour, or NULL
 *  @param shift how far its depth is shifted down to an OT index
 *  @param ot    the ordering table */
void DrawStandingBillboard(u16 id, SVECTOR *size, CVECTOR *color, s32 shift, GsOT *ot);

/** @brief Draws a numbered sprite at a screen position into an ordering
 *         table.
 *  @param id    which sprite
 *  @param pos   its screen position
 *  @param color its colour, or NULL
 *  @param mode  how it is drawn
 *  @param ot    the ordering table */
void DrawSprite(u16 id, SVECTOR *pos, CVECTOR *color, s32 mode, GsOT *ot);

/** @brief DrawStandingBillboard with the colour depth-cued into the fog.
 *  @param id    which sprite
 *  @param size  its size
 *  @param color its colour, or NULL for mid grey
 *  @param shift how far its depth is shifted down to an OT index
 *  @param ot    the ordering table */
void DrawStandingBillboardFog(u16 id, SVECTOR *size, CVECTOR *color, s32 shift, GsOT *ot);

/** @brief Picks how lit gouraud-textured TMD polygons are drawn.
 *  @param mode 0 or 1 lit with an environment map over it, 2 wireframe,
 *              3 environment map only, 4 lit, 5 lit semi-transparent
 *  @return undefined: the definition has no return statement */
s32 SetTmdSortMode(s32 mode);

#endif
