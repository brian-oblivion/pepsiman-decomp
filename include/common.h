#ifndef COMMON_H
#define COMMON_H

/**
 * @file common.h
 * @brief Included first by every unit: the integer types (types.h),
 *        and small C89 helper macros.
 */

#ifdef HOST_BUILD
/* The host build has no PS1 assembly: the macros that include it are empty,
 * and the PS1 linker's labels are left out. */
#define INCLUDE_ASM(FOLDER, NAME)
#define INCLUDE_RODATA(FOLDER, NAME)
#else
#include "include_asm.h"
#endif
#include "types.h"

/** The byte offset of `member` in `type`: C89's <stddef.h> offsetof, which
 * Sony's stddef.h does not provide. A constant expression. */
#ifndef offsetof
#define offsetof(type, member) ((unsigned int)&((type *)0)->member)
#endif

/** Fails to compile unless `cond`, a constant expression, holds: C89's
 * static assertion, a typedef of an array of size -1. `name` keeps the
 * typedef unique in its unit. No code or data. Used for the layouts a file
 * or the SDK fixes, which must hold on the PS1 and on every host. */
#define COMPILE_ASSERT(cond, name) typedef char static_assert_##name[(cond) ? 1 : -1]

/** The number of elements in the array `arr` (an array, never a pointer).
 * Signed, so `i < ARRAY_COUNT(a)` on an s32 compares signed, as a literal
 * would. */
#define ARRAY_COUNT(arr) ((s32)(sizeof(arr) / sizeof((arr)[0])))

/** `deg` degrees as a PlayStation angle, 4096 (libgte's ONE) to the turn.
 * Exact for multiples of 45 degrees; a constant expression. */
#define ANGLE_DEG(deg) ((deg) * 4096 / 360)

#if !defined(M2CTX) && !defined(PERMUTER) && !defined(HOST_BUILD)
/** Places a definition in small data, the `.sdata` section: the game's
 * small initialized globals, reached through one base register. Used as
 * `static s32 sName SDATA = 320;`. The definition must have an initializer,
 * and its unit's yaml range must be `.sdata`. */
#define SDATA __attribute__((section(".sdata")))
/** Places a zero-initialized definition in `.sbss`, the uninitialized half of
 * small data: `static s32 sName SBSS = 0;`. The `= 0` is required; without
 * an initializer the section attribute has no effect. */
#define SBSS __attribute__((section(".sbss")))
#else
#define SDATA
#define SBSS
#endif

/** Read-only data in the image that the game writes all the same: `const`
 * on the PS1, so that it lands in `.rodata` as in the image; writable on a
 * host build, whose `.rodata` is read-only memory. */
#ifdef HOST_BUILD
#define IMAGE_CONST
#else
#define IMAGE_CONST const
#endif

/** 20.12 fixed point: libgte's ONE is 1 << FIX12_SHIFT, so `n << FIX12_SHIFT`
 * is the integer n as a fixed-point value. */
#define FIX12_SHIFT 12

/** GteLong's type: `int` against psyz's libgte, `long` against Sony's. */
#ifdef HOST_BUILD
#define GTE_LONG_BASE int
#else
#define GTE_LONG_BASE long
#endif
/** libgte's 32-bit integer, the type of MATRIX.t[] and of VECTOR's members:
 * `long` in Sony's headers, `int` in psyz's (the host build). A pointer to
 * one of those members is a `GteLong *`. */
typedef GTE_LONG_BASE GteLong;

/** @brief The main loop's states, gGameMode. */
enum GameMode {
    GM_BOOT = 0,            /**< clears the clear colour and starts the boot load */
    GM_BOOT_LOAD = 1,       /**< LoadBootData */
    GM_SOUND_LOAD = 2,      /**< LoadSoundData */
    GM_OVERLAY0_LOAD = 3,   /**< loads overlay 0 */
    GM_OVERLAY0_WAIT = 4,   /**< waits for overlay 0 */
    GM_OVERLAY0 = 5,        /**< runs overlay 0 */
    GM_MENU_LOAD = 6,       /**< loads overlay 2, the menus; game over and quitting return here */
    GM_MENU_WAIT = 7,       /**< waits for overlay 2 */
    GM_MENU = 8,            /**< runs overlay 2 */
    GM_STAGE_LOAD = 12,     /**< loads the stage overlay */
    GM_STAGE_WAIT = 13,     /**< waits for the stage overlay */
    GM_STAGE = 14,          /**< UpdateStagePlay */
    GM_GRID_TEST_INIT = 15, /**< InitGridTest */
    GM_GRID_TEST = 16,      /**< UpdateGridTest */
    GM_MOTION_VIEWER_INIT = 17, /**< starts the motion viewer */
    GM_MOTION_VIEWER = 19,      /**< UpdateMotionViewer */
    GM_TRAP_VIEWER_LOAD = 20,   /**< loads the stage overlay for the trap viewer */
    GM_TRAP_VIEWER_WAIT = 21,   /**< waits for it */
    GM_TRAP_VIEWER_INIT = 22,   /**< InitStageOverlay */
    GM_TRAP_VIEWER = 23,        /**< UpdateTrapViewer */
    GM_TRAP_LAYOUT_LOAD = 24,   /**< loads the stage overlay for the layout viewer */
    GM_TRAP_LAYOUT_WAIT = 25,   /**< waits for it */
    GM_TRAP_LAYOUT_INIT = 26,   /**< InitStageOverlay */
    GM_TRAP_LAYOUT = 27,        /**< UpdateTrapLayoutViewer */
    GM_EDITOR_LOAD = 28,        /**< loads the stage overlay for the editor */
    GM_EDITOR_WAIT = 29,        /**< waits for it */
    GM_EDITOR_INIT = 30,        /**< InitStageOverlay */
    GM_EDITOR_RELOAD = 31,      /**< waits for a load, then the editor */
    GM_EDITOR = 32,             /**< UpdateEditor */
    GM_HIT_TOOL_LOAD = 33,      /**< loads the stage overlay for the hit-data tool */
    GM_HIT_TOOL_WAIT = 34,      /**< waits for it */
    GM_HIT_TOOL_INIT = 35,      /**< InitStageOverlay */
    GM_HIT_TOOL_RELOAD = 36,    /**< waits for a load, then the tool */
    GM_HIT_TOOL = 37,           /**< UpdateTool */
    GM_WORLD_INTRO_LOAD = 38,   /**< loads the world's intro overlay (18 + world) */
    GM_WORLD_INTRO_WAIT = 39,   /**< waits for it */
    GM_WORLD_INTRO = 40,        /**< runs it */
    GM_LOAD_SCREEN_MODEL = 41,  /**< UpdateLoadScreenModel */
    GM_LOAD_SCREEN = 42,        /**< UpdateLoadScreen */
    GM_LOAD_SCREEN_ACT3 = 43,   /**< UpdateAct3LoadScreen */
    GM_MOVIE = 44,              /**< plays the world's movie */
};

/* --- Globals shared across units ------------------------------------------
 *
 * Only what every unit can use unchanged: a global that some units reach as a
 * small object and others as an array of unknown size is declared in each
 * unit instead. Types come from the access widths and the signedness of the
 * loads; a 4-byte one is s32 until a unit shows it is a pointer.
 */

/** @brief A short-lived effect from the 128-record pool: a sprite or model
 *         drawn at a point for a few frames, then handed back. */
typedef struct {
    s16 frame;  /**< frames drawn so far; the step function ends it */
    s16 delay;  /**< frames to wait before the first one is drawn */
    s16 param0; /**< meaning depends on the kind */
    s16 param1; /**< meaning depends on the kind */
    s16 param2; /**< meaning depends on the kind */
    s16 param3; /**< meaning depends on the kind */
    s16 x;      /**< offset added to the drawing position */
    s16 y;      /**< offset added to the drawing position */
    s16 z;      /**< offset added to the drawing position */
    u8 kind;    /**< index into the step-function table */
    u8 next;    /**< the next record on the active or free list */
} Effect;

extern Effect gEffects[]; /**< the 128-record pool */

/** @brief A 0x78-byte record of an 80-entry table; two pairs of halfwords
 *         are reset together. */
typedef struct {
    u8 unk0[0x10];  /**< not yet known */
    u8 unk10[0x5E]; /**< handed to the dispatch's object on entering state 1 */
    s16 unk6E;      /**< zeroed when the second buffer is cleared */
    s16 unk70;      /**< -1 when the second buffer is cleared */
    s16 unk72;      /**< zeroed when the first buffer is cleared, -1 when banks are installed */
    s16 unk74;      /**< -1 when the first buffer is cleared, zeroed when banks are installed */
    u8 unk76[2];    /**< not yet known */
} Rec78;

extern Rec78 D_800D8D20[]; /**< the 80 records; 2 units */

/** @brief The player's state (gPlayer), 0x3D8 bytes. Its head is the
 *         animation state StepAnimation runs: two channels, each asking
 *         for a motion (a TOD sequence) and playing one. The load screen
 *         keeps a whole copy as its turning model (gLoadScreenModel). */
typedef struct {
    u8 control;           /**< 1 while the pad steers the player; cleared by a miss or the goal */
    u8 unk1;              /**< the steering mode last applied; a repeat only resets yaw */
    u8 hittable;          /**< 1 when a hit can be reacted to; 0 while invulnTimer runs,
                               *   when the player is drawn every other frame pair */
    u8 twoTracks;         /**< nonzero steps both animation channels instead of one */
    u8 unk4;              /**< SetTodFrame's last argument for a single-channel step */
    u8 action;            /**< the action state UpdatePlayerAction runs (jump, slide, hit
                               *   reactions); 0x60 and 0x61 for a demo model */
    u8 motion[2];         /**< the motion each channel is asked to play; for channel 0,
                               *   2 to 4 are the run bands, 8 to 11 the jump */
    u8 curMotion[2];      /**< the motion each channel is playing; 0xFF forces a restart */
    s16 motionTime[2];    /**< each channel's clock */
    u8 padE[0x18 - 0xE];  /**< not yet known */
    u8 *motionData[2];    /**< each channel's position in its sequence */
    s32 motionLen[100];   /**< each motion's length */
    s32 motionStart[100]; /**< each motion's start time */
    s8 motionResult[2];   /**< StepAnimation's result for each channel; 2 at a loop's end */
    u8 pad342[0x344 - 0x342]; /**< not yet known */
    s32 unk344;               /**< (40 - speed) / 3 + 1 */
    s32 x;                    /**< position x */
    s32 y;                    /**< position y, down positive; held at groundY on the ground */
    s32 z;                    /**< position z */
    s32 drawX;                /**< added to x for the drawn position */
    s32 drawY;                /**< added to y for the drawn position */
    s32 drawZ;                /**< added to z for the drawn position */
    s16 unk360;               /**< an angle copied to the camera rotation's x */
    s16 unk362;               /**< an angle added to yaw for the camera rotation's y */
    s16 unk364;               /**< cleared on a reset */
    u8 pad366[0x368 - 0x366]; /**< not yet known */
    s32 lookX;  /**< x of the point the models turn to face (gLookYaw*); cleared by player */
    s32 lookY;  /**< y of that point */
    s32 lookZ;  /**< z of that point; cleared by player */
    s16 unk374; /**< cleared when y passes the goal line */
    s16 unk376; /**< a step counter of the current mode */
    s16 unk378; /**< a second step counter of the current mode */
    s16 unk37A; /**< cleared with unk378 */
    s32 unk37C; /**< set with yaw to the start heading */
    s32 yaw;    /**< the body heading; follows the course heading in bounded steps */
    s32 lean;   /**< the body roll while steering, -15..15; decays to 0 */
    u8 pad388[0x38C - 0x388]; /**< not yet known */
    s16 unk38C;               /**< cleared whenever slideState changes */
    u8 slideState;            /**< 0 running, 1 sliding, 2 dashing */
    u8 unk38F;                /**< 1 also gates a check on jumpPower */
    u16 jumpState;  /**< 0 on the ground, 1 while the button is held, 2 for one step after */
    s16 jumpTime;   /**< steps the button has been held while jumpState is 1 */
    s32 jumpStartY; /**< y when the jump began */
    s16 jumpPower;  /**< the jump's strength, set by how long the button is held */
    u8 pad39A[0x39C - 0x39A]; /**< not yet known */
    s32 unk39C;               /**< with unk3CC, picks which cap y gets */
    s32 unk3A0;               /**< 0, 1 or 2, set as jumpState goes to 2 */
    s16 unk3A4;               /**< a countdown set to 1 by some speed ranges */
    s16 fallSpeed;            /**< added (times 5) to y each step in the air; reset on landing */
    s16 speed;                /**< the running speed; 40 is cruising, 50 after a reset */
    s16 unk3AA;               /**< speed saved when a mode starts */
    u16 speedBand;            /**< 0 below 13 in speed, 1 below 26, else 2 */
    s16 stepX;                /**< speed along the sine of the heading: this step's x move */
    s16 stepZ;                /**< speed along the cosine of the heading: this step's z move */
    u8 pad3B2[0x3B4 - 0x3B2]; /**< not yet known */
    s32 unk3B4;               /**< moved by 50 a step while steering */
    s32 unk3B8;               /**< 1 selects the global cap for y; set when a placed record hits */
    s32 unk3BC;               /**< 1 with unk3B8 also starts the jumpState sequence */
    s32 groundY;              /**< the ground height under the player */
    s32 unk3C4;               /**< the editor keeps y here before snapping to the floor */
    s16 invulnTimer;          /**< counts down after a hit; the player is hittable again at 0 */
    u16 unk3CA;    /**< set to 1 when the stage ends; selects the second ordering tables */
    s32 unk3CC;    /**< with unk39C, picks which cap y gets */
    u8 rideMode;   /**< low nibble: 0 on foot, else the stage's ride (1 to 4) */
    u8 unk3D1;     /**< set to 1 when a stage ends with a mode in rideMode */
    u8 unk3D2;     /**< set to 1 by one mode */
    s8 showShadow; /**< nonzero draws the shadow under the player */
    s8 unk3D4;     /**< 1 while mode 0xF runs; swaps object 10's model */
    u8 unk3D5;     /**< set to 1 by some pickups */
    u8 pad3D6[0x3D8 - 0x3D6]; /**< not yet known */
} PlayerState;

extern PlayerState gPlayer; /**< the player */

/** @brief A 10-byte hit record, one per group of the player's collision
 *         points; collision fills it, player reacts to it. */
typedef struct {
    s8 hit;   /**< the hit flag; 0 when free */
    u8 point; /**< the nearest point of the group */
    u8 kind;  /**< what hit, a row of the HitReaction table */
    u8 unk3;  /**< from the query */
    s16 unk4; /**< from the query */
    s16 unk6; /**< from the query */
    s16 unk8; /**< from the query */
} PlayerHit;

extern PlayerHit gPlayerHits[3]; /**< one per point group */

/* Small: a plain extern. */
extern s32 gTimInfo;        /**< 2 units, 4 functions */
extern u8 gClearB;          /**< the screen clear and fog colour, blue */
extern u8 gClearG;          /**< the screen clear and fog colour, green */
extern u8 gClearR;          /**< the screen clear and fog colour, red */
extern u8 gRepeatButtonsOn; /**< 3 units, 3 functions */
extern s32 gCdTrackPos;     /**< 2 units, 2 functions */
extern s32 gCdTrackPlays;   /**< 2 units, 2 functions */
extern s32 gCdTrackStart;   /**< 2 units, 2 functions */
extern s32 gLookYawNarrow; /**< object 2's turn toward the look point, eased by 8 within +-60 degrees */
extern s32 gLookYawWide; /**< the other objects' turn toward the look point, eased by 50 within +-80 degrees */
extern u16 gEditValue;     /**< the value the highlighted menu line edits */
extern s16 gMenuLine;      /**< the highlighted line of a menu */
extern u32 gStageObjCount; /**< an entry count */
extern s32 gActiveBuff;    /**< the display buffer being drawn this frame, 0 or 1 */
extern s32 gPad;           /**< port 1's held buttons this frame (PAD* bits), after ReadPad */
extern s32 gGamePadTrig;   /**< remapped button flags; 2 units */
extern u16 gModeStep;      /**< the model viewer's state; 2 units */
extern u16 D_80095768;     /**< 2 units, 2 functions; never loaded, sign unknown */
extern u8 D_800957D5;      /**< 2 units, 2 functions; never loaded, sign unknown */
extern u8 D_800957D6;      /**< 2 units, 3 functions; never loaded, sign unknown */
extern u8 gFellOff;        /**< set when the player falls below the floor; never read here */
extern s16 gMissState;     /**< 2 units; set to 2 at the goal when 0 */
extern s32 gLoopI;         /**< a loop counter kept in a global */
extern s32 gSlotTableSize; /**< the slot table's size in bytes */
extern s32 gSlotTable;     /**< the address of the 72-byte slot records */
extern s32 gStickTrigY;  /**< a pad word; its bits step a position or a highlighted line; 2 units */
extern s32 gVSyncTime;   /**< what EndFrame's VSync returned; printed first on the status panel */
extern s32 gRetries;     /**< set to 0xFF on leaving the tool; 2 units */
extern u8 gInEditor;     /**< 2 units */
extern s32 gElapsed;     /**< 2 units */
extern s32 gTimeLeft;    /**< 2 units */
extern s16 D_800957B0;   /**< 2 units */
extern s32 gGamePad;     /**< 2 units */
extern s8 gViewerMotion; /**< a level, kept within 0..120; 2 units */
extern u8 gPadType;      /**< port 1's controller type byte (0x41 digital, 0x73 analog) */
extern u32 gStickRawY;   /**< pad byte 7, an analogue stick axis, 0x80 at rest */
extern u32 gStickRawX;   /**< pad byte 6, the axis gStickX is read from, 0x80 at rest */
extern s16 gSelectedSection; /**< the selected Rec78 entry; 2 units */
extern u16 gCameraYaw;       /**< 2 units */
extern s8 gButtonConfig;     /**< 2 units */
extern s8 *gStageRunObjs;    /**< the group list the zones index; 2 units */
extern s32 gCpuMarkCount;    /**< 2 units */
extern u8 gUnlocks;          /**< 2 units */
extern s32 gFloorData;       /**< the address of the floor data a probe tests; 2 units */
extern u8 gLivePoints[];     /**< a 0x44C0-byte buffer of 0x2C-byte placed records */
extern u8 gLiveQuads[];      /**< a 0x1DB0-byte buffer of 0x4C-byte records */
extern u8 gCdTrackPlaying;   /**< 1 while CD audio is playing; 2 units */
extern u16 gGameMode;        /**< the main loop's state, a GameMode */
extern s16 gCdLoadCount;     /**< files in the current CD load */
extern u8 gLoading; /**< 1 while a load runs: a fixed view and a black screen; 0 the game's view */
extern s8 gPauseState;        /**< nonzero stops the frame counter; 2 units */
extern s32 ClipF;             /**< a word inside libgte's clipf object, as a variable; 2 units */
extern s32 gModelTmd;         /**< the mapped model data's TMD address; 2 units */
extern s8 gCdLoadState;       /**< UpdateCdLoad's step; 6 when every file has been read */
extern u8 gStereo;            /**< nonzero for stereo: the CD mix has no cross-feed; 2 units */
extern s16 gMenuChoice;       /**< what RunMenu returned for the card menus */
extern u16 gToolCardStep;     /**< 2 units, 4 functions */
extern s16 gToolSlot;         /**< 2 units, 4 functions */
extern u16 D_80095A14;        /**< 3 units, 3 functions; never loaded, sign unknown */
extern s32 gHitQuads;         /**< 2 units, 7 functions */
extern s32 gHitPoints;        /**< 2 units, 7 functions */
extern s32 gHitPointSections; /**< entry count of the first record bank; 2 units */
extern s32 gHitQuadSections;  /**< entry count of the second record bank; 2 units */
extern u16 gVabLoadStep;      /**< 2 units, 3 functions */
extern u8 gStage;          /**< the stage being played: stage / 3 is the world, stage % 3 the act */
extern s32 D_80095864;     /**< 3 units, 5 functions */
extern u8 gEffectActive;   /**< 2 units, 2 functions */
extern u8 gEffectFree;     /**< 2 units, 2 functions */
extern u16 gCans;          /**< 5 units, 7 functions */
extern s16 gEditMax;       /**< limit of the value being edited */
extern s16 gMenuLineCount; /**< the count a menu line wraps at */
extern s32 gPadTrig;       /**< port 1's newly pressed buttons this frame, auto-repeat included */
extern s16 D_800E474C;     /**< libgs's PSDIDX; 2 units */
extern u8 *D_800959C0;     /**< the bytes after a BlockHeader */
extern u8 *D_800959C4;     /**< the BlockHeader's second part */
extern s32 D_800959C8;     /**< the BlockHeader's first word */
extern s16 gSoundFade;     /**< 2 units, 2 functions */
extern s32 gBonusLevel;    /**< 2 units; 3 when a stage starts */
extern s16 D_800957BC;     /**< 2 units; a heading */
extern u8 *gFogColors;     /**< 2 units; far (fog) colours, four bytes each */
extern u16 gFogFade;       /**< 2 units; fog fade-in step: 0..31, then 100 */
extern s32 gVSyncMode;     /**< EndFrame's VSync argument: the frames to wait */
extern s32 D_800958AC;     /**< 2 units; 1 picks the second set of stage tables */
extern s8 D_8009599C;      /**< 2 units */
extern u16 gSpeedFixed;    /**< 2 units */
extern u32 gFrameCount;    /**< frames drawn while not paused */
extern s32 gLoopJ;         /**< a second loop counter, or a count, kept in a global */
extern s16 gSectionCount;  /**< number of entries; 2 units */
extern s16 gViewerAngle;   /**< the view's orbit angle, in degrees; 2 units */
extern s32 gSection;       /**< stamped on placed records; 2 units */
extern u16 gSubStep;       /**< a state switched on in main; cleared after placing; 2 units */
extern s16 gToolMode;      /**< an error code; cleared when flag bit 6 is set; 2 units */
extern s32 D_800957F4;     /**< the result of the start-position lookup; 2 units */
extern s32 gTmdSortMode;   /**< SetTmdSortMode's argument for the models */

/* Per-stage tables, three words per stage; a global flag picks the set. */
extern s32 gTimeLimits[][3];    /**< 2 units */
extern s32 gTimeLimitsAlt[][3]; /**< the second set; 2 units */

/* Arrays of unknown size, each reached only at its first element: most are
 * probably members of larger structures, still to be found. */
extern u16 gRumblePatterns[]; /**< 3 units, 6 functions */
extern s32 gCdLoadLoc[];      /**< each file's CdlLOC in the current CD load */
extern s32 gView[];           /**< 4 units, 4 functions */
extern s32 D_8009676C[];      /**< 2 units, 3 functions */
extern u16 D_8009EAB8[];      /**< 2 units, 3 functions; never loaded, sign unknown */
extern s16 D_8009EABA[];      /**< 2 units, 2 functions */
extern u8 D_8009EEC0[];       /**< an object with a position; passed to
                          *   distance tests */
extern s16 D_8009EF20[];      /**< 2 units, 6 functions */
extern u8 D_8009EF48[];       /**< 4 units, 7 functions */
extern s8 D_8009EF4A[];       /**< 2 units, 2 functions */
extern s32 gCdLoadDest[];     /**< each file's load address in the current CD load */
extern s32 gCdLoadSectors[];  /**< each file's length in sectors, less one */
extern s32 gWorldShift[];     /**< 5 units, 22 functions */
extern s32 D_800A9008[];      /**< 3 units, 17 functions */
extern s16 gFloorY[];         /**< 2 units, 6 functions */
extern s32 D_800D39C8[];      /**< 2 units, 2 functions */
extern s32 gTodObjTable[];    /**< 2 units, 5 functions */
extern s32 gSavedStart[]; /**< the state a stage can resume from (stage_flow's SavedStart); [0] is its valid flag */
extern s32 gRefView[];    /**< the game's GsRVIEW2, as words */
extern s32 gFlatLights[]; /**< 3 units, 4 functions */

extern s32 *gAnimations[]; /**< per-sequence data pointers; 2 units */

/* --- Functions called from more than one unit, with the same prototype ---- */

/** @brief Defined in sound.
 *  @param id what to start, by number
 *  @return -1 when a sound bank failed to open; otherwise nothing is
 *          returned (callers ignore the result) */
s32 PlaySe(s32 id);

/** @brief Defined in sound; called from main. */
void InitSound(void);

/** @brief Defined in model; called from main. */
void InitView(void);

/** @brief Defined in model: updates an index from a position, then
 *         checks the position against the data blob.
 *  @param pos a position, three words
 *  @param arg a result buffer for the check
 *  @param cur the current index
 *  @return the updated index, or -1 when the check fails */
s32 FindFloorNear(void *pos, void *arg, s32 cur);

/** @brief Defined in model; called when a stage is reset. */
void StopCdTrack(void);

/** @brief Defined in model; called from main: sets up the three flat
 *         lights, the ambient light and the fog. */
void InitLights(void);

/** @brief Defined in main.
 *  @param count how many times it prints its fixed string */
void PrintTabs(s16 count);

/** @brief Defined in player: on every tenth pickup, plays a cue
 *         (unless the stage forbids it) and updates the pickup display. */
void CheckCanBonus(void);

/** @brief Defined in stage; loads a run of TIM images (each starting
 *         with the word 0x10) into VRAM.
 *  @param p the first TIM */
void LoadTimList(unsigned long *p);

/** @brief Defined in stage (still asm); called from main. */
void DefineHudSprites(void);

/** @brief Defined in stage: queues the draw-mode and clipped
 *         draw-environment packets that split the screen.
 *  @param clip nonzero to queue them; zero does nothing */
void SetLetterbox(s32 clip);

/** @brief Defined in model; issues one CD command.
 *  @param com the CdlXXX command
 *  @return always 0 */
s32 SendCdTrackCommand(u8 com);

/** @brief Defined in model; the CD ready callback main installs.
 *  @param mode the interrupt status */
void OnCdTrackReady(u8 mode);

/** @brief Defined in draw: fills one 8-byte record of a sprite table.
 *  @param id    which record
 *  @param mode  stored at bit 7 of the record's first halfword
 *  @param w     stored in byte 6
 *  @param h     stored in byte 7
 *  @param page  the low byte of the first halfword (read as a byte)
 *  @param u     stored in byte 4
 *  @param v     stored in byte 5
 *  @param clutX its top 12 bits form the low bits of the second halfword
 *  @param clutY stored from bit 6 of the second halfword */
void DefineSprite(u16 id, u8 mode, s32 w, s32 h, s32 page, s32 u, s32 v, u16 clutX, s32 clutY);

/** @brief Defined in editor.
 *  @return 0. */
s32 StepMenuCursor(void);

/** @brief Defined in stage: remaps three buttons of the two button
 *         words by a row of a remap table. */
void RemapButtons(void);

/** @brief Defined in collision. */
void ResetCollision(void);

/** @brief Defined in collision. */
void func_80029838(void);

/** @brief Defined in hit_tool; called from card. */
void ApplyHitPoints(void);

/** @brief Defined in hit_tool; called from card. */
void ApplyHitQuads(void);

/** @brief Defined in player; called from stage_flow. Resets the
 *         stepped value two mode handlers steer, to 40 whole units. */
void ResetSpeed(void);

/** @brief Defined in editor. */
void PrintYesNo(void);

/** @brief Defined in editor. */
void PrintSaveLoad(void);

/** @brief Defined in editor: wraps the highlighted line at `n` lines
 *         and runs an update.
 *  @param n the number of lines
 *  @return the highlighted line when flag bit 5 is set, else -1. */
s32 RunMenu(s16 n);

/** @brief Defined in card. */
void OpenCardEvents(void);

/** @brief Defined in card. */
void CloseCardEvents(void);

/** @brief Defined in card: runs one memory card operation on the
 *         second save file.
 *  @param mode 0 reads, 1 rewrites, 2 creates, 3 formats, 4 erases, 5 checks
 *  @return 0 or -1 */
s32 RunCardOp2(s16 mode);

/** @brief Defined in collision: tests a 0x4C-byte record.
 *  @param rec the record */
void PlaceCollisionQuad(void *rec);

/** @brief Defined in draw_tmd: selects one of 23 data sets by `id`
 *         and points three globals at it.
 *  @param id the set, 0 to 22 (read as a byte) */
void LoadOverlay(s32 id);

/** @brief Defined in stage_flow: one frame of the stage-start sequence.
 *  @param stage the stage whose overlay runs the frame */
void UpdateStagePlay(u8 stage);

/** @brief Defined in stage_overlay: reads a stage's halfword from a table.
 *  @param stage the stage index
 *  @return the table entry */
s32 GetStageVabBank(u16 stage);

/** @brief Defined in stage_overlay. */
void ResetStageOverlay(void);

/** @brief Defined in main.
 *  @param arg not yet known */
void ReadPad(s32 arg);

/** @brief Defined in stage. */
void DrawFrozenFrame(void);

/** @brief Defined in stage_flow. */
void FadeInFog(void);

/** @brief Starts a fade that UpdateSoundFade runs over 50 frames.
 *  @param mode 1 the BGM, 3 the BGM, the sound effects and the CD audio
 *              (it falls through into 4), 4 the CD
 *              audio; 0 none */
void SetSoundFade(u8 mode);

/** @brief Defined in player. */
void EndJump(void);

/** @brief Defined in model. */
void UpdateMotionViewer(void);

/** @brief Defined in draw. */
void InitTmdSortFuncs(void);

/** @brief Defined in player. */
void InitGridTest(void);

/** @brief Defined in player. */
void UpdateGridTest(void);

/** @brief Defined in object. */
void UpdateTrapViewer(void);

/** @brief Defined in object. */
void InitEntryModels(void);

/** @brief Defined in hit_tool. */
void UpdateTool(void);

/** @brief Defined in editor.
 *  @return not yet known */
s32 UpdateEditor(void);

/** @brief Defined in stage_flow: a stage's number.
 *  @param stage the stage
 *  @return not yet known */
s16 InitStageOverlay(u8 stage);

/** @brief Defined in stage_overlay.
 *  @param sel not yet known */
void LoadStageOverlay(u16 sel);


/** @brief Defined in stage. */
void DrawScreenWipe(void);

/** @brief Defined in stage. */
void UpdateActResults(void);

/** @brief Defined in player.
 *  @return not yet known */
s32 UpdatePlayState(void);

/** @brief Defined in main.
 *  @return not yet known */
s32 UpdateRumble(void);

/** @brief Defined in main.
 *  @param offset not yet known
 *  @param a      not yet known
 *  @param b      not yet known */
void SetPadVibration(s32 offset, u8 a, u8 b);

/** @brief Defined in sound: sets a volume.
 *  @param vol the volume */
void SetCdVolume(s32 vol);

#endif
