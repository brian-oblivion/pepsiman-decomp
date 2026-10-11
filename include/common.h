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

/* Small: a plain extern. */
extern s32 gTimInfo;       /**< 2 units, 4 functions */
extern u8 gClearB;         /**< the screen clear and fog colour, blue */
extern u8 gClearG;         /**< the screen clear and fog colour, green */
extern u8 gClearR;         /**< the screen clear and fog colour, red */
extern u8 D_800956F7;      /**< 3 units, 3 functions */
extern s32 gCdTrackPos;    /**< 2 units, 2 functions */
extern s32 gCdTrackPlays;  /**< 2 units, 2 functions */
extern s32 gCdTrackStart;  /**< 2 units, 2 functions */
extern s32 D_80095740;     /**< camera-facing y angle, eased by 8; 2 units */
extern s32 D_80095744;     /**< camera-facing y angle, eased by 50; 2 units */
extern u16 D_80095748;     /**< 2 units, 2 functions */
extern s16 D_8009574A;     /**< the highlighted line of a three-line menu */
extern u32 gStageObjCount; /**< an entry count */
extern s32 gActiveBuff;    /**< the display buffer being drawn this frame, 0 or 1 */
extern s32 gPad;           /**< port 1's held buttons this frame (PAD* bits), after ReadPad */
extern s32 gGamePadTrig;   /**< remapped button flags; 2 units */
extern u16 gModeStep;      /**< the model viewer's state; 2 units */
extern u16 D_80095768;     /**< 2 units, 2 functions; never loaded, sign unknown */
extern u8 D_800957D5;      /**< 2 units, 2 functions; never loaded, sign unknown */
extern u8 D_800957D6;      /**< 2 units, 3 functions; never loaded, sign unknown */
extern u8 D_800958EC;      /**< 2 units, 4 functions; never loaded, sign unknown */
extern s16 D_80095858;     /**< 2 units; set to 2 at the goal when 0 */
extern s32 D_800958CC;     /**< 2 units, 2 functions; a loop counter kept in a global */
extern s32 D_800959B4;     /**< 2 units, 2 functions */
extern s32 D_800959B8;     /**< 2 units, 2 functions */
extern s32 D_80095958;   /**< a pad word; its bits step a position or a highlighted line; 2 units */
extern s32 gVSyncTime;   /**< what EndFrame's VSync returned; printed first on the status panel */
extern s32 gRetries;     /**< set to 0xFF on leaving the tool; 2 units */
extern u8 D_800958F8;    /**< 2 units */
extern s32 gElapsed;     /**< 2 units */
extern s32 gTimeLeft;    /**< 2 units */
extern s16 D_800957B0;   /**< 2 units */
extern s32 gGamePad;     /**< 2 units */
extern s8 gViewerMotion; /**< a level, kept within 0..120; 2 units */
extern u8 gPadType;      /**< port 1's controller type byte (0x41 digital, 0x73 analog) */
extern u32 D_800957C8;   /**< an analogue stick axis, 0x80 at rest; 2 units */
extern u32 D_800957C0;   /**< the other analogue stick axis, 0x80 at rest; 2 units */
extern s16 D_800958E2;   /**< the selected Rec78 entry; 2 units */
extern u16 gCameraYaw;   /**< 2 units */
extern s8 gButtonConfig; /**< 2 units */
extern s8 *gStageRunObjs;  /**< the group list the zones index; 2 units */
extern s32 gCpuMarkCount;  /**< 2 units */
extern u8 gUnlocks;        /**< 2 units */
extern s32 gFloorData;     /**< the address of the floor data a probe tests; 2 units */
extern u8 gLivePoints[];   /**< a 0x44C0-byte buffer of 0x2C-byte placed records */
extern u8 gLiveQuads[];    /**< a 0x1DB0-byte buffer of 0x4C-byte records */
extern u8 gCdTrackPlaying; /**< 1 while CD audio is playing; 2 units */
extern u16 gGameMode;      /**< the main loop's state */
extern s16 gCdLoadCount;   /**< files in the current CD load */
extern u8 gLoading; /**< 1 while a load runs: a fixed view and a black screen; 0 the game's view */
extern s8 gPauseState;        /**< nonzero stops the frame counter; 2 units */
extern s32 ClipF;             /**< a word inside libgte's clipf object, as a variable; 2 units */
extern s32 D_80095904;        /**< the mapped model data's TMD address; 2 units */
extern s8 gCdLoadState;       /**< UpdateCdLoad's step; 6 when every file has been read */
extern u8 gStereo;            /**< nonzero for stereo: the CD mix has no cross-feed; 2 units */
extern s16 D_800959E0;        /**< 2 units, 2 functions */
extern u16 gToolCardStep;     /**< 2 units, 4 functions */
extern s16 gToolSlot;         /**< 2 units, 4 functions */
extern u16 D_80095A14;        /**< 3 units, 3 functions; never loaded, sign unknown */
extern s32 gHitQuads;         /**< 2 units, 7 functions */
extern s32 gHitPoints;        /**< 2 units, 7 functions */
extern s32 gHitPointSections; /**< entry count of the first record bank; 2 units */
extern s32 gHitQuadSections;  /**< entry count of the second record bank; 2 units */
extern u16 gVabLoadStep;      /**< 2 units, 3 functions */
extern u8 gStage;         /**< the stage being played: stage / 3 is the world, stage % 3 the act */
extern s32 D_80095864;    /**< 3 units, 5 functions */
extern u8 gEffectActive;  /**< 2 units, 2 functions */
extern u8 gEffectFree;    /**< 2 units, 2 functions */
extern u16 gCans;         /**< 5 units, 7 functions */
extern s16 D_800958B0;    /**< limit of the value being edited */
extern s16 D_800958B2;    /**< the count a menu line wraps at */
extern s32 gPadTrig;      /**< port 1's newly pressed buttons this frame, auto-repeat included */
extern s16 D_800E474C;    /**< libgs's PSDIDX; 2 units */
extern u8 *D_800959C0;    /**< the bytes after a BlockHeader */
extern u8 *D_800959C4;    /**< the BlockHeader's second part */
extern s32 D_800959C8;    /**< the BlockHeader's first word */
extern s16 gSoundFade;    /**< 2 units, 2 functions */
extern s32 D_800958A8;    /**< 2 units; 3 when a stage starts */
extern s16 D_800957BC;    /**< 2 units; a heading */
extern u8 *gFogColors;    /**< 2 units; far (fog) colours, four bytes each */
extern u16 gFogFade;      /**< 2 units; fog fade-in step: 0..31, then 100 */
extern s32 gVSyncMode;    /**< EndFrame's VSync argument: the frames to wait */
extern s32 D_800958AC;    /**< 2 units; 1 picks the second set of stage tables */
extern s8 D_8009599C;     /**< 2 units */
extern u16 D_800957D2;    /**< 2 units */
extern u32 gFrameCount;   /**< frames drawn while not paused */
extern s32 D_800958D0;    /**< a loop counter or count kept in a global; 2 units */
extern s16 gSectionCount; /**< number of entries; 2 units */
extern s16 gViewerAngle;  /**< the view's orbit angle, in degrees; 2 units */
extern s32 gSection;      /**< stamped on placed records; 2 units */
extern u16 gSubStep;      /**< a state switched on in main; cleared after placing; 2 units */
extern s16 gToolMode;     /**< an error code; cleared when flag bit 6 is set; 2 units */
extern s32 D_800957F4;    /**< the result of the start-position lookup; 2 units */
extern s32 D_80095758;    /**< 2 units */

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
extern u8 D_8009EB78[];       /**< 8 units, 69 functions */
extern u8 D_8009EB7E[];       /**< 2 units, 2 functions */
extern u8 D_8009EEC0[];       /**< an object with a position; passed to
                          *   distance tests */
extern s16 D_8009EF20[];      /**< 2 units, 6 functions */
extern u8 D_8009EF48[];       /**< 4 units, 7 functions */
extern s8 D_8009EF4A[];       /**< 2 units, 2 functions */
extern s32 gCdLoadDest[];     /**< each file's load address in the current CD load */
extern u8 gPlayerHits[];      /**< 2 units, 4 functions */
extern s32 gCdLoadSectors[];  /**< each file's length in sectors, less one */
extern s32 gWorldShift[];     /**< 5 units, 22 functions */
extern s32 D_800A9008[];      /**< 3 units, 17 functions */
extern s16 D_800AC858[];      /**< 2 units, 6 functions */
extern s32 D_800D39C8[];      /**< 2 units, 2 functions */
extern s32 gTodObjTable[];    /**< 2 units, 5 functions */
extern s32 D_800D86B8[];      /**< 2 units, 3 functions */
extern s32 gRefView[];        /**< the game's GsRVIEW2, as words */
extern s32 gFlatLights[];     /**< 3 units, 4 functions */

extern s32 *gAnimations[]; /**< per-sequence data pointers; 2 units */

/* --- Functions called from more than one unit, with the same prototype ---- */

/** @brief Defined in code_31cec.
 *  @param id what to start, by number
 *  @return -1 when a sound bank failed to open; otherwise nothing is
 *          returned (callers ignore the result) */
s32 PlaySe(s32 id);

/** @brief Defined in code_31cec; called from main. */
void InitSound(void);

/** @brief Defined in code_7d74; called from main. */
void InitView(void);

/** @brief Defined in code_7d74: updates an index from a position, then
 *         checks the position against the data blob.
 *  @param pos a position, three words
 *  @param arg a result buffer for the check
 *  @param cur the current index
 *  @return the updated index, or -1 when the check fails */
s32 FindFloorNear(void *pos, void *arg, s32 cur);

/** @brief Defined in code_7d74; called when a stage is reset. */
void StopCdTrack(void);

/** @brief Defined in code_7d74; called from main: sets up the three flat
 *         lights, the ambient light and the fog. */
void InitLights(void);

/** @brief Defined in main.
 *  @param count how many times it prints its fixed string */
void PrintTabs(s16 count);

/** @brief Defined in code_13068: on every tenth pickup, plays a cue
 *         (unless the stage forbids it) and updates the pickup display. */
void func_80028448(void);

/** @brief Defined in code_29f54; loads a run of TIM images (each starting
 *         with the word 0x10) into VRAM.
 *  @param p the first TIM */
void LoadTimList(unsigned long *p);

/** @brief Defined in code_29f54 (still asm); called from main. */
void DefineHudSprites(void);

/** @brief Defined in code_29f54: queues the draw-mode and clipped
 *         draw-environment packets that split the screen.
 *  @param clip nonzero to queue them; zero does nothing */
void SetLetterbox(s32 clip);

/** @brief Defined in code_7d74; issues one CD command.
 *  @param com the CdlXXX command
 *  @return always 0 */
s32 SendCdTrackCommand(u8 com);

/** @brief Defined in code_7d74; the CD ready callback main installs.
 *  @param mode the interrupt status */
void OnCdTrackReady(u8 mode);

/** @brief Defined in code_a0bc: fills one 8-byte record of a sprite table.
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

/** @brief Defined in code_1dc24.
 *  @return 0. */
s32 func_800330D4(void);

/** @brief Defined in code_29f54: remaps three buttons of the two button
 *         words by a row of a remap table. */
void RemapButtons(void);

/** @brief Defined in code_1902c. */
void ResetCollision(void);

/** @brief Defined in code_1902c. */
void func_80029838(void);

/** @brief Defined in code_24748; called from code_27bc8. */
void ApplyHitPoints(void);

/** @brief Defined in code_24748; called from code_27bc8. */
void ApplyHitQuads(void);

/** @brief Defined in code_13068; called from code_308ec. Resets the
 *         stepped value two mode handlers steer, to 40 whole units. */
void func_800287C0(void);

/** @brief Defined in code_1dc24. */
void func_80033C90(void);

/** @brief Defined in code_1dc24. */
void func_80033D3C(void);

/** @brief Defined in code_1dc24: wraps the highlighted line at `n` lines
 *         and runs an update.
 *  @param n the number of lines
 *  @return the highlighted line when flag bit 5 is set, else -1. */
s32 func_8003356C(s16 n);

/** @brief Defined in code_27bc8. */
void OpenCardEvents(void);

/** @brief Defined in code_27bc8. */
void CloseCardEvents(void);

/** @brief Defined in code_27bc8: runs one memory card operation on the
 *         second save file.
 *  @param mode 0 reads, 1 rewrites, 2 creates, 3 formats, 4 erases, 5 checks
 *  @return 0 or -1 */
s32 RunCardOp2(s16 mode);

/** @brief Defined in code_1902c: tests a 0x4C-byte record.
 *  @param rec the record */
void PlaceCollisionQuad(void *rec);

/** @brief Defined in code_11dc4: selects one of 23 data sets by `id`
 *         and points three globals at it.
 *  @param id the set, 0 to 22 (read as a byte) */
void LoadOverlay(s32 id);

/** @brief Defined in code_308ec: one frame of the stage-start sequence.
 *  @param stage the stage whose overlay runs the frame */
void UpdateStagePlay(u8 stage);

/** @brief Defined in code_31958: reads a stage's halfword from a table.
 *  @param stage the stage index
 *  @return the table entry */
s32 GetStageVabBank(u16 stage);

/** @brief Defined in code_31958. */
void ResetStageOverlay(void);

/** @brief Defined in main.
 *  @param arg not yet known */
void ReadPad(s32 arg);

/** @brief Defined in code_29f54. */
void DrawFrozenFrame(void);

/** @brief Defined in code_308ec. */
void FadeInFog(void);

/** @brief Starts a fade that UpdateSoundFade runs over 50 frames.
 *  @param mode 1 the BGM, 3 the BGM, the sound effects and the CD audio
 *              (it falls through into 4), 4 the CD
 *              audio; 0 none */
void SetSoundFade(u8 mode);

/** @brief Defined in code_13068. */
void func_800285B0(void);

/** @brief Defined in code_7d74. */
void UpdateMotionViewer(void);

/** @brief Defined in code_a0bc. */
void InitTmdSortFuncs(void);

/** @brief Defined in code_13068. */
void func_800229A8(void);

/** @brief Defined in code_13068. */
void func_80022A74(void);

/** @brief Defined in code_1a098. */
void func_8002B04C(void);

/** @brief Defined in code_1a098. */
void func_8002CC24(void);

/** @brief Defined in code_24748. */
void UpdateTool(void);

/** @brief Defined in code_1dc24.
 *  @return not yet known */
s32 func_8002D424(void);

/** @brief Defined in code_308ec: a stage's number.
 *  @param stage the stage
 *  @return not yet known */
s16 InitStageOverlay(u8 stage);

/** @brief Defined in code_31958.
 *  @param sel not yet known */
void LoadStageOverlay(u16 sel);


/** @brief Defined in code_29f54. */
void DrawScreenWipe(void);

/** @brief Defined in code_29f54. */
void UpdateActResults(void);

/** @brief Defined in code_13068.
 *  @return not yet known */
s32 func_80026D9C(void);

/** @brief Defined in main.
 *  @return not yet known */
s32 UpdateRumble(void);

/** @brief Defined in main.
 *  @param offset not yet known
 *  @param a      not yet known
 *  @param b      not yet known */
void SetPadVibration(s32 offset, u8 a, u8 b);

/** @brief Defined in code_31cec: sets a volume.
 *  @param vol the volume */
void SetCdVolume(s32 vol);

#endif
