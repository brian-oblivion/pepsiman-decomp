#ifndef PLAYER_H
#define PLAYER_H

/**
 * @file player.h
 * @brief What other units call in player that needs libgte's types.
 */

#include "common.h"
#include "libgte.h"

/** @brief Projects a world position to the screen.
 *  @param pos the world position
 *  @param out the screen position */
void ProjectToScreen(VECTOR *pos, SVECTOR *out);

/** @brief Defined in player; takes the game state block.
 *  @param state the state block */
void ReactToHits(u8 *state);

/** @brief Defined in player.
 *  @param n not yet known
 *  @return not yet known */
s32 AddBonusLevel(s32 n);

extern SVECTOR D_800A7680[]; /**< rotations; [0].vy is a yaw (heading) */

#endif
