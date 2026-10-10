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

/** @brief Rebuilds a coordinate system's matrix from a rotation, keeping
 *         its translation, and marks it changed.
 *  @param rot   the rotation (YXZ order)
 *  @param coord the coordinate system rewritten */
void func_80018AE0(SVECTOR *rot, GsCOORDINATE2 *coord);

#endif
