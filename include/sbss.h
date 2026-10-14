#ifndef SBSS_H
#define SBSS_H

/**
 * @file sbss.h
 * @brief The `.sbss` variables sbss.c defines that common.h does not
 *        declare, for the units that use them.
 */

#include "common.h"

extern s8 gViewerReplay; /**< set to 1 when the level is applied */
extern u8 gRepeatButtons[3]; /**< frames square, cross and circle have been held, for the menus' key repeat */
extern u8 gRepeatDown;     /**< frames down has been held, for the menus' key repeat */
extern u8 gRepeatSides[2]; /**< frames left and right have been held, for the menus' key repeat */
extern u8 gRepeatUp;       /**< frames up has been held, for the menus' key repeat */
extern u8 *sPadBufPtr;     /**< the receive buffers StartPads hands to PadInitDirect */
extern u8 *sPadActPtr;     /**< the actuator bytes StartPads hands to the pad driver */
extern s32 gCdTrackEnd;    /**< the playing CD track's last sector, 20 short of the next track */
extern s32 gCdTrackLoop;   /**< nonzero to restart the CD track when it ends */
extern s16 gCdVolume;      /**< the CD audio volume, 0 to 80, as the fade sets it */
extern s8 gRepeatSpeed;    /**< divides the key repeat's delay and interval; 4 by default */
extern s8 sLastHitMask;    /**< the hit flags of gPlayerHits, or-ed, when last reacted to */
extern u16 gRumbleIndex;   /**< the step of the rumble pattern playing */
extern u8 sPreviewing;     /**< 1 while RefreshToolPreview runs */
extern s32 sDumpOffset;    /**< DumpMemory's offset from its base address */
extern u32 gPropObjCount;  /**< the objects in the stage's prop model file */
extern s32 gVabError;      /**< -1 once a sound bank has failed to load, else 0 */
extern s32 D_800957A4;     /**< cleared when a stage starts; read by no code */
extern s16 sPrevViewY;     /**< the view point's y before this frame's clamp */
extern u8 gCdLoadRetries;  /**< how many times a disc read has been retried */
extern s32 gGridLastAngle; /**< the grid test's last angle */

#endif
