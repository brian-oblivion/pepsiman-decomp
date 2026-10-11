#ifndef COLLISION_H
#define COLLISION_H

/**
 * @file collision.h
 * @brief What other units call in collision that needs libgte's types.
 */

#include "common.h"
#include "libgte.h"

/** @brief Defined in collision: the squared distance between two points.
 *  @param a one point
 *  @param b the other point
 *  @return the squared distance */
s32 DistanceSquared(VECTOR *a, VECTOR *b);

#endif
