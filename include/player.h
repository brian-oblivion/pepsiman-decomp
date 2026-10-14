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
 *  @param out the screen position in vx and vy, its depth in vz */
void ProjectToScreen(VECTOR *pos, SVECTOR *out);

/** @brief Turns this frame's hit records into the action the player
 *         reacts with: a gHitReactions column picked by the running motion
 *         or action, with the invulnerability after it and the bonus level
 *         taken or zeroed. On a board or the roller any hit throws the
 *         player back and loses the ride. Hits are ignored while the player
 *         is not hittable or after leaving the hit tool.
 *  @param state the player's state */
void ReactToHits(PlayerState *state);

/** @brief Adds to the bonus level and clamps it: 0 to 3 (0 to 2 with the
 *         second set of stage tables), and 0 while PlayerState.bonusLocked
 *         is set.
 *  @param n what to add; 0 only reads the level
 *  @return the bonus level after the change */
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
