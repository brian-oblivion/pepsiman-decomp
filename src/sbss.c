/**
 * @file sbss.c
 * @brief The game's zero-initialized small data, `.sbss`, in address order.
 *
 * Every variable here belongs to the unit that uses it, where its `extern`
 * declaration lives. The original placed them as common symbols: the
 * linker laid them out itself, interleaving roughly fifteen units'
 * variables, and an object's `.sbss` cannot be split, so no unit can define
 * its own and land at the original addresses. The `D_` entries are zeros
 * between them that no code reads.
 */

#include "common.h"
#include "libgte.h"
#include "libcd.h"

/* MATCHING: the original common-symbol layout, one definition per address. */
s8 gViewerMotion SBSS = 0;
s8 gViewerReplay SBSS = 0;
s32 gTimInfo SBSS = 0;
u8 gRepeatButtons[3] SBSS = {0};
u8 gRepeatDown SBSS = 0;
s32 D_800956DC SBSS = 0; /**< read by no code */
s32 sJumpDistance SBSS = 0;
s32 sJumpPeakY SBSS = 0;
u8 gRepeatSides[2] SBSS = {0};
u8 gRepeatUp SBSS = 0;
u8 D_800956EB SBSS = 0;  /**< read by no code */
s32 D_800956EC SBSS = 0; /**< read by no code */
s32 D_800956F0 SBSS = 0; /**< read by no code */
s16 D_800956F4 SBSS = 0; /**< read by no code */
u8 D_800956F6 SBSS = 0;  /**< read by no code */
u8 gRepeatButtonsOn SBSS = 0;
s32 D_800956F8 SBSS = 0; /**< read by no code */
s32 D_800956FC SBSS = 0; /**< read by no code */
u8 *sPadBufPtr SBSS = NULL;
u8 *sPadActPtr SBSS = NULL;
s32 D_80095708 SBSS = 0; /**< read by no code */
s32 D_8009570C SBSS = 0; /**< read by no code */
s32 gCdTrackEnd SBSS = 0;
s32 gCdTrackPos SBSS = 0;
s32 gCdTrackLoop SBSS = 0;
s32 gCdTrackPlays SBSS = 0;
s32 gCdTrackStart SBSS = 0;
s16 gCdVolume SBSS = 0;
CdlLOC gCdTrackLoc SBSS = {0}; /**< where the CD track starts, as CdControl takes it */
s32 D_8009572C SBSS = 0;       /**< read by no code */
s32 D_80095730 SBSS = 0;       /**< read by no code */
s32 D_80095734 SBSS = 0;       /**< read by no code */
s32 D_80095738 SBSS = 0;       /**< read by no code */
s32 D_8009573C SBSS = 0;       /**< read by no code */
s32 gLookYawNarrow SBSS = 0;
s32 gLookYawWide SBSS = 0;
u16 gEditValue SBSS = 0;
s16 gMenuLine SBSS = 0;
u8 gClearB SBSS = 0;
s32 gActiveBuff SBSS = 0;
u8 gClearG SBSS = 0;
s32 gTmdSortMode SBSS = 0;
u8 gClearR SBSS = 0;
s8 gRepeatSpeed SBSS = 0;
u8 D_8009575E SBSS = 0; /**< read by no code */
s8 sLastHitMask SBSS = 0;
u16 gModeStep SBSS = 0;
s16 D_80095762 SBSS = 0;   /**< read by no code */
u16 gRumbleSmall SBSS = 0; /**< the small motor's strength, for SetPadVibration */
u16 gRumbleLarge SBSS = 0; /**< the large motor's strength, for SetPadVibration */
u16 gWorldIntroStep SBSS = 0;
u16 gFogFade SBSS = 0;
u16 gRumbleIndex SBSS = 0;
s32 gRetries SBSS = 0;
u8 sPreviewing SBSS = 0;
u8 D_80095775 SBSS = 0;  /**< read by no code */
s16 D_80095776 SBSS = 0; /**< read by no code */
s32 D_80095778 SBSS = 0; /**< read by no code */
s32 D_8009577C SBSS = 0; /**< read by no code */
s32 gHitPointSections SBSS = 0;
u8 gPadType SBSS = 0;
s32 sDumpOffset SBSS = 0;
s32 gSection SBSS = 0;
u8 *gLightColors SBSS = NULL;
u32 gStageObjCount SBSS = 0;
u32 gPropObjCount SBSS = 0;
s32 gVabError SBSS = 0;
s8 D_800957A0 SBSS = 0; /**< the saved stage to continue from, 100 added for the second table; -1 for none */
s32 D_800957A4 SBSS = 0;
s32 gVSyncMode SBSS = 0;
s32 D_800957AC SBSS = 0; /**< read by no code */
s16 gSteerYawHalf SBSS = 0;
s32 gGamePad SBSS = 0;
s32 D_800957B8 SBSS = 0; /**< read by no code */
s16 gSteerYaw SBSS = 0;
u32 gStickRawX SBSS = 0;
s32 D_800957C4 SBSS = 0; /**< read by no code */
u32 gStickRawY SBSS = 0;
s32 D_800957CC SBSS = 0; /**< read by no code */
s16 D_800957D0 SBSS = 0; /**< read by no code */
u16 gSpeedFixed SBSS = 0;
u8 D_800957D4 SBSS = 0; /**< read by no code */
u8 gAllTargetsBeaten SBSS = 0;
u8 gPlayLive SBSS = 0;
s16 gRumbleNext SBSS = 0;   /**< the rumble record to chain to when this one ends */
u8 gRumbleEnabled SBSS = 0; /**< nonzero when rumble is switched on in the options */
s16 gMissTimer SBSS = 0;    /**< frames since time ran out, in the time-up sequence */
s16 sPrevViewY SBSS = 0;
u16 gRumbleRepeat SBSS = 0; /**< how many more times the rumble record repeats */
SVECTOR gLocalPos SBSS = {0};
s32 gGamePadTrig SBSS = 0;
u8 gCdLoadRetries SBSS = 0;
s32 gFloorIndex SBSS = 0;
s32 gGridLastAngle SBSS = 0;
s32 D_800957FC SBSS = 0; /**< read by no code */
