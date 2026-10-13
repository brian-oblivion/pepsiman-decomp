#ifndef WORLD_INTRO_H
#define WORLD_INTRO_H

/**
 * @file world_intro.h
 * @brief What the five world intro overlays (ovl18 to ovl22) share: the
 *        model list they walk, the camera clock and the view they drive.
 */

#include "common.h"
#include "libgte.h"
#include "libgs.h"

/** @brief The model list as the model draw walks it. */
typedef struct {
    GsDOBJ2 *objs; /**< the models */
    s32 count;     /**< how many */
} DrawList;

/** @brief The record holding the intro's camera clock at +0x24. */
typedef struct {
    u8 pad0[0x24]; /**< not yet known */
    u16 timer;     /**< the intro's camera clock */
    s16 title;     /**< the act-3 chase's title clock */
} IntroTimer;

/** @brief The record whose word at +0x24 is the intro's camera clock. */
extern IntroTimer D_800D39D8;

/** @brief The game's reference view, as the GsRVIEW2 it is. */
#define sView (*(GsRVIEW2 *)gRefView)

#endif
