#ifndef CODE_1902C_H
#define CODE_1902C_H

/**
 * @file code_1902c.h
 * @brief What other units call in code_1902c that needs libgte's types.
 */

#include "common.h"
#include "libgte.h"

/** @brief Defined in code_1902c: the squared distance between two points.
 *  @param a one point
 *  @param b the other point
 *  @return the squared distance */
s32 func_800297A4(VECTOR *a, VECTOR *b);

#endif
