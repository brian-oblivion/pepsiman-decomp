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

/** @brief Defined in player; reacts to the player's hit records.
 *  @param state the state block */
void ReactToHits(PlayerState *state);

/** @brief Defined in player.
 *  @param n not yet known
 *  @return not yet known */
s32 AddBonusLevel(s32 n);

/** @brief The course where the player is: the heading the player steers
 *         around, and the point a stage start or restart puts the
 *         player at. */
typedef struct {
    SVECTOR rot; /**< rot.vy is the course heading; the editor keeps a
                  *   saved heading in rot.vz */
    s32 x;       /**< the start point's x */
    s32 y;       /**< the start point's y */
    s32 z;       /**< the start point's z */
} CoursePoint;

extern CoursePoint gCourse; /**< the course at the player */

#endif
