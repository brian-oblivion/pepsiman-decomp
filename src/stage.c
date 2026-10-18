/**
 * @file stage.c
 * @brief The running stages' shared engine: the stage models, the course
 *        camera, the HUD, the screen wipe, the results screens and the
 *        effects.
 *
 * LoadStageData (LoadAct3Data for an act-3 chase) maps a stage pack's
 * scenery and props models and keeps pointers to its other parts: the floor
 * data, the course path, the floor boundaries, each section's runs of
 * objects, and the light, fog and sky colours. Each frame UpdateCamera steps
 * the section along the path and swings the camera after it, DrawStage (or
 * DrawRepeatingStage, the chases' endless corridor) draws the section, and
 * DrawHud draws the cans, the bonus level, the countdown, the progress bar,
 * the time and the lives. StartActResults and UpdateActResults run an act's
 * results, counting the cans and storing records; StartWorldResults and
 * UpdateWorldResults a world's. SpawnEffect, UpdateEffects and gEffectSteps
 * run the effect pool. Also here: the letterbox, the button remap, the
 * frozen frame, the screen wipe and a CPU-time meter no code calls.
 */

#include "common.h"
#include "sbss.h"
#include "libgte.h"
#include "libgpu.h"
#include "libetc.h"
#include "libgs.h"
#include "libcd.h"
#include "draw.h"
#include "model.h"
#include "spad.h"

/** @brief Eight bytes, copied together as one unaligned block. */
typedef struct {
    u8 b[8]; /**< the bytes */
} Bytes8;

/** @brief Sixteen signed bytes, copied together as one unaligned block. */
typedef struct {
    s8 b[4][4]; /**< four rows of four */
} Bytes16;

/** RemapButtons' table, a row per gButtonConfig: for the pad's three face
 * buttons (0x20, 0x40, 0x80), the action bit each one sets, as a shift
 * from 0x20; the fourth byte is padding. */
const Bytes16 sButtonMaps = {{
    {2, 0, 1, 0},
    {2, 1, 0, 0},
    {1, 0, 2, 0},
    {0, 1, 2, 0},
}};

extern s32 gWipeFrame;     /**< the screen wipe's frame, 0 to 24; each image shows for 3 */
extern s32 gTallyCans;     /**< the cans the act results have counted up so far */
extern s32 gResultsActive; /**< 1 while a results screen runs */
extern s32 gResultsTimer;  /**< frames into the current results step */
extern s32 gWipeActive;    /**< 1 while the screen wipe runs */

/** @brief The results screens' steps, gResultsStep. An act's runs them
 *         in order, skipping the record steps when no record falls; a
 *         world's uses RES_SLIDE_IN, then RES_WORLD_DONE. */
enum ResultsStep {
    RES_SLIDE_IN = 0,    /**< the lines slide in on a timer */
    RES_TALLY_CANS = 10, /**< counts the cans up; a life every 25 */
    RES_ALL_CANS = 11,   /**< all 100 cans: a flashing tile and a life */
    RES_BEST_CANS = 15,  /**< a can record: stores it */
    RES_SLIDE_TIME = 20, /**< the time lines slide in */
    RES_CHECK_TIME = 30, /**< tests the time against the record */
    RES_BEST_TIME = 35,  /**< a time record: stores it */
    RES_END = 50,        /**< after 60 frames, the unlocks and the act's totals */
    RES_WORLD_DONE = 10  /**< a world's results are finished */
};

extern s32 gResultsStep;        /**< a ResultsStep */
extern s8 gResultsSliding[8];   /**< per results line: 1 while it slides in */
extern s8 gResultsSlide[8];     /**< per results line: its slide offset, counting up to 0 */
extern u8 sActResultsSlide[];   /**< each results line's slide-in start, for an act */
extern u8 sWorldResultsSlide[]; /**< each results line's slide-in start, for a world */
/* MATCHING: both declared above as arrays of unknown size before they are
 * defined, so the copies of them split their addresses. */
u8 sActResultsSlide[] SDATA = {0xF0, 0xE7, 0xF7, 0xF7, 0xF7, 0xF7, 0xF7, 0xF7};
u8 sWorldResultsSlide[] SDATA = {0xF0, 0xF7, 0xF7, 0xF7, 0xF7, 0xF7, 0xF7, 0xF7};

/** @brief An 8-byte boundary record: a point and the direction its sign
 *         test projects onto. */
typedef struct {
    s16 x;  /**< the point's x */
    s16 y;  /**< the point's y */
    s16 dx; /**< the direction's x */
    s16 dy; /**< the direction's y */
} Edge;

extern Edge *gBoundaryEdges; /**< stage header part 4: the floor blocks' boundaries */

extern GsDOBJ2 gStageObjs[];      /**< the scenery model's objects */
extern GsDOBJ2 gPropObjs[];       /**< the props model's objects */
extern GsCOORDINATE2 gStageCoord; /**< the coordinate system both stage models hang from */

extern s32 gCpuMarkTimes[]; /**< each CPU-time mark's scanline count, 0xFF for none */
extern u8 sCpuMarkR[];      /**< each CPU-time mark's colour, red */
extern u8 sCpuMarkG[];      /**< green */
extern u8 sCpuMarkB[];      /**< blue */

/** @brief A run of entries in a draw list: where it starts and how long. */
typedef struct {
    s16 start; /**< the first entry's index */
    s16 count; /**< the number of entries */
} Run;

/* MATCHING: a per-unit view; object types the same list as Zone4. */
extern Run *gStageRuns;  /**< stage header part 5: each section's run of scenery objects */
extern Run *gPropRuns;   /**< stage header part 6: each section's run of props */
extern s8 *gPropRunObjs; /**< the prop indices the runs of gPropRuns cover; -1 for none */

extern DR_STP sDrawStpMain; /**< the letterbox's draw-mode packet in the 3D ordering table */
extern DR_ENV sDrawEnvMain; /**< the letterbox's clipped draw environment */
extern DR_STP sDrawStpSub;  /**< the draw-mode packet at the head of the HUD's ordering table */
extern DR_ENV sDrawEnvSub;  /**< the full-screen draw environment the HUD is drawn in */

/**
 * @brief Letterboxes the 3D view. With `clip` set it queues, in the 3D ordering
 *        table's last entry, a draw environment that clips drawing to the 175
 *        lines from line 32, and at the head of the HUD's table one that gives
 *        the full 320x240 screen back. Called once a frame from main.
 */
void SetLetterbox(s32 clip) {
    DRAWENV env;
    /* MATCHING: retail's frame has 8 more bytes above the environment. */
    s32 unused[2];

    if (clip) {
        GetDrawEnv(&env);
        env.clip.x = 0;
        env.clip.y = PSDIDX * 240 + 32;
        env.clip.w = 319;
        env.clip.h = 175;
        SetDrawStp(&sDrawStpMain, 1);
        addPrim(gOt[gActiveBuff].org + 0xFFF, &sDrawStpMain);
        SetDrawEnv(&sDrawEnvMain, &env);
        addPrim(gOt[gActiveBuff].org + 0xFFF, &sDrawEnvMain);
        env.clip.x = 0;
        env.clip.y = PSDIDX * 240;
        env.clip.w = 319;
        env.clip.h = 239;
        SetDrawStp(&sDrawStpSub, 1);
        addPrim(gSubOt[gActiveBuff].org, &sDrawStpSub);
        SetDrawEnv(&sDrawEnvSub, &env);
        addPrim(gSubOt[gActiveBuff].org, &sDrawEnvSub);
    }
}

/* MATCHING: per-unit views; this loader stores the stage pointers plainly. */
extern Edge *gSectionEdges; /**< stage header part 3: the course path, a point per section, after a count */
extern s32 gStageStart;        /**< stage header part 7: the record the start heading comes from */
extern s32 gLightColors;       /**< stage header part 10: light colours, four bytes each */
extern s8 *sSectionCamHeights; /**< stage header part 8: each section's camera height, in tens */
extern s32 gSkyColors;         /**< stage header part 11: the sky's colour sets */
extern s32 gStageRunCount;     /**< the number of runs in gStageRuns */
extern s32 gPropRunCount;      /**< the number of runs in gPropRuns */

/**
 * @brief Takes in a stage pack loaded to RAM: maps its scenery and props models
 *        and links their objects to gStageCoord, resets the section and floor
 *        indices, and points the stage's globals at the pack's other parts: the
 *        floor data, the course path, the floor boundaries, the scenery and
 *        props runs, the start record, the camera heights and the fog, light
 *        and sky colours.
 * @param hdr the pack, a table of its parts' offsets from its start
 */
void LoadStageData(unsigned long *hdr) {
    u32 i;
    GsDOBJ2 *obj;
    unsigned long *tmd;

    tmd = (unsigned long *)(hdr[0] + (s32)hdr);
    tmd++;
    GsMapModelingData(tmd);
    tmd++;
    gStageObjCount = *tmd;
    tmd++;
    for (i = 0; i < gStageObjCount; i++) {
        GsLinkObject4((unsigned long)tmd, &gStageObjs[i], i);
    }
    obj = gStageObjs;
    for (i = 0; i < gStageObjCount; i++) {
        obj->coord2 = &gStageCoord;
        obj->attribute = 0x200;
        obj++;
    }
    tmd = (unsigned long *)(hdr[1] + (s32)hdr);
    tmd++;
    GsMapModelingData(tmd);
    tmd++;
    gPropObjCount = *tmd;
    tmd++;
    for (i = 0; i < gPropObjCount; i++) {
        GsLinkObject4((unsigned long)tmd, &gPropObjs[i], i);
    }
    obj = gPropObjs;
    for (i = 0; i < gPropObjCount; i++) {
        obj->coord2 = &gStageCoord;
        obj->attribute = 0;
        obj++;
    }
    gSection = 0;
    gFloorIndex = 0;
    gFloorData = hdr[2] + (s32)hdr;
    gSectionEdges = (Edge *)(hdr[3] + (s32)hdr + 4);
    gBoundaryEdges = (Edge *)(hdr[4] + (s32)hdr + 4);
    gStageRunCount = *(s32 *)(hdr[5] + (s32)hdr);
    gStageRuns = (Run *)(hdr[5] + (s32)hdr + 4);
    gStageRunObjs = (s8 *)(hdr[5] + (s32)hdr + gStageRunCount * 4 + 4);
    gPropRunCount = *(s32 *)(hdr[6] + (s32)hdr);
    gPropRuns = (Run *)(hdr[6] + (s32)hdr + 4);
    gStageStart = hdr[7] + (s32)hdr;
    sSectionCamHeights = (s8 *)(hdr[8] + (s32)hdr);
    gFogColors = (u8 *)(hdr[9] + (s32)hdr);
    gLightColors = hdr[10] + (s32)hdr;
    gSkyColors = hdr[11] + (s32)hdr;
    gPropRunObjs = (s8 *)(hdr[6] + (s32)hdr + gPropRunCount * 4 + 4);
}

extern GsDOBJ2 sAct3Objs[];          /**< the act-3 pack's third model's objects */
extern GsCOORDINATE2 sAct3Coords[];  /**< their coordinate systems */
extern GsDOBJ2 sPieceObjs[];         /**< the piece objects; LoadAct3Data links from [10] */
extern GsCOORDINATE2 sPieceCoords[]; /**< each piece object's coordinate system */

/**
 * @brief Maps the stage TMD and links its objects to the stage object table.
 * @param tmd the file, at its leading word
 */
static __inline__ void linkStage(unsigned long *tmd) {
    u32 i;
    GsDOBJ2 *obj;

    tmd++;
    GsMapModelingData(tmd);
    tmd++;
    gStageObjCount = *tmd;
    tmd++;
    for (i = 0; i < gStageObjCount; i++) {
        GsLinkObject4((unsigned long)tmd, &gStageObjs[i], i);
    }
    obj = gStageObjs;
    for (i = 0; i < gStageObjCount; i++) {
        obj->coord2 = &gStageCoord;
        obj->attribute = 0x200;
        obj++;
    }
}

/**
 * @brief Maps the props TMD and links its objects to the props object table.
 * @param tmd the file, at its leading word
 */
static __inline__ void linkProps(unsigned long *tmd) {
    u32 i;
    GsDOBJ2 *obj;

    tmd++;
    GsMapModelingData(tmd);
    tmd++;
    gPropObjCount = *tmd;
    tmd++;
    for (i = 0; i < gPropObjCount; i++) {
        GsLinkObject4((unsigned long)tmd, &gPropObjs[i], i);
    }
    obj = gPropObjs;
    for (i = 0; i < gPropObjCount; i++) {
        obj->coord2 = &gStageCoord;
        obj->attribute = 0;
        obj++;
    }
}

/**
 * @brief LoadStageData's counterpart for an act-3 chase pack: links the scenery
 *        and props, a third model whose objects each have their own coordinate
 *        system, and the piece objects from slot 10 on; reads animations 51 on
 *        from the fifth part, with their lengths and first frames into gPlayer;
 *        and defines the four 255x64 strips that start the stage's sprites.
 * @param pack the pack, a table of its parts' offsets from its start
 */
void LoadAct3Data(s32 *pack) {
    s32 i;
    s32 n;
    u8 *t;

    linkStage((unsigned long *)(pack[0] + (s32)pack));
    linkProps((unsigned long *)(pack[1] + (s32)pack));
    GsMapModelingData((unsigned long *)(pack[2] + (s32)pack + 4));
    n = *(s32 *)(pack[2] + (s32)pack + 8);
    for (i = 0; i < n; i++) {
        GsLinkObject4((unsigned long)(pack[2] + (s32)pack + 12), &sAct3Objs[i], i);
        sAct3Objs[i].coord2 = &sAct3Coords[i];
        sAct3Objs[i].attribute = 0;
    }
    GsMapModelingData((unsigned long *)(pack[3] + (s32)pack + 4));
    n = *(s32 *)(pack[3] + (s32)pack + 8);
    for (i = 0; i < n; i++) {
        GsLinkObject4((unsigned long)(pack[3] + (s32)pack + 12), &sPieceObjs[i + 10], i);
        sPieceObjs[i + 10].coord2 = &sPieceCoords[i + 10];
        sPieceObjs[i + 10].attribute = 0;
    }
    t = (u8 *)(pack[4] + (s32)pack);
    n = *(s32 *)t / 4;
    for (i = 0; i < n; i++) {
        gAnimations[i + 51] = (s32 *)(((s32 *)t)[i] + (s32)t) + 1;
        gPlayer.motionLen[i + 51] = *gAnimations[i + 51];
        gAnimations[i + 51]++;
        gPlayer.motionStart[i + 51] = gAnimations[i + 51][1];
    }
    DefineSprite(SPR_STAGE, 0, 0xFF, 0x40, 0x1C, 0, 0, 0, 0x1FE);
    DefineSprite(SPR_STAGE + 1, 0, 0xFF, 0x40, 0x1C, 0, 0x40, 0, 0x1FE);
    DefineSprite(SPR_STAGE + 2, 0, 0xFF, 0x40, 0x1C, 0, 0x80, 0, 0x1FE);
    DefineSprite(SPR_STAGE + 3, 0, 0xFF, 0x40, 0x1C, 0, 0xC0, 0, 0x1FE);
}

/**
 * @brief Draws the current section's scenery and props: places the stage at (x,
 *        y, z), the player's position negated, and sorts every object of
 *        gSection's two runs into the current ordering table, skipping an index
 *        past the model's object count (and a prop of -1).
 * @param x the stage's x
 * @param y the stage's y
 * @param z the stage's z
 */
void DrawStage(s16 x, s16 y, s16 z) {
    s32 i;
    s32 k;
    SVECTOR rot;
    MATRIX ls;

    gStageCoord.coord.t[0] = x;
    gStageCoord.coord.t[1] = y;
    gStageCoord.coord.t[2] = z;
    rot.vx = rot.vy = rot.vz = 0;
    SetCoordRotation(&rot, &gStageCoord);
    GsGetLs(&gStageCoord, &ls);
    GsSetLsMatrix(&ls);
    SetSpadStack();
    for (i = 0; i < gStageRuns[gSection].count; i++) {
        k = gStageRunObjs[gStageRuns[gSection].start + i];
        if ((u32)k < gStageObjCount) {
            GsSortObject4J(&gStageObjs[k], gCurOt, 2, getScratchAddr(0));
        }
    }
    for (i = 0; i < gPropRuns[gSection].count; i++) {
        k = gPropRunObjs[gPropRuns[gSection].start + i];
        if ((u32)k < gPropObjCount && k != -1) {
            GsSortObject4J(&gPropObjs[k], gCurOt, 2, getScratchAddr(0));
        }
    }
    ResetSpadStack();
}

/** The length of one tile of a repeating stage, along x. */
#define REPEAT_TILE_LEN 5000
/** Added to a position so that it divides as a positive number: a whole,
 * even number of tiles, so neither the remainder nor the tile's parity
 * changes. */
#define REPEAT_BIAS (2000 * REPEAT_TILE_LEN)

/**
 * @brief Draws a stage that repeats along x, the act-3 chases' endless
 *        corridor: the tiles before, under and after `pos`. Objects 1 and 2
 *        alternate; 0 is the tile the run starts in and 3 the one it ends in.
 * @param pos    the position along x
 * @param unused unused
 * @param z      the stage's z
 * @param range  how far the run goes: past `-range` the end tile is drawn
 */
void DrawRepeatingStage(s32 pos, s32 unused, s32 z, s32 range) {
    s32 i;
    s32 k;
    s32 d;
    s32 r;
    SVECTOR rot;
    MATRIX ls;

    for (i = -1; i < 2; i++) {
        k = (((pos + REPEAT_BIAS) / REPEAT_TILE_LEN + i) & 1) + 1;
        d = pos - i * REPEAT_TILE_LEN;
        if (d >= 0) {
            if (d >= REPEAT_TILE_LEN) {
                continue;
            }
            k = 0;
        }
        if (d < -range) {
            if (d < -(range + REPEAT_TILE_LEN)) {
                continue;
            }
            k = 3;
        }
        if (pos >= 0) {
            r = pos % REPEAT_TILE_LEN;
        } else {
            r = (pos + REPEAT_BIAS) % REPEAT_TILE_LEN;
        }
        gStageCoord.coord.t[0] = r + (i - 1) * REPEAT_TILE_LEN;
        gStageCoord.coord.t[1] = 0;
        gStageCoord.coord.t[2] = z;
        rot.vx = rot.vy = rot.vz = 0;
        SetCoordRotation(&rot, &gStageCoord);
        GsGetLs(&gStageCoord, &ls);
        GsSetLsMatrix(&ls);
        GsSortObject4J(&gStageObjs[k], gCurOt, 2, getScratchAddr(0));
        GsSortObject4J(&gPropObjs[k], gCurOt, 2, getScratchAddr(0));
    }
}

/**
 * @brief Steps a floor-block index across the floor boundaries: one block on
 *        when (x, y) has passed the next boundary, one back when it is behind
 *        the block's own. Finds the block under a point from the block of a
 *        point near it.
 * @param index the floor-block index, stepped in place
 * @param x     the point's x
 * @param y     the point's other horizontal coordinate, the world's z
 */
void StepFloorIndex(s32 *index, s16 x, s16 y) {
    s32 i;
    s32 j;
    s32 d;
    Edge *e;
    Edge *f;

    /* MATCHING: integer address sums put the scaled index first; pointer
     * arithmetic puts the base first and changes the register choice. */
    i = *index;
    e = (Edge *)(i * 8 + (s32)gBoundaryEdges) + 1;
    d = e->dx * (x - e->x) + e->dy * (y - e->y);
    if (d >= 0) {
        *index = i + 1;
    }
    j = *index;
    f = (Edge *)(j * 8 + (s32)gBoundaryEdges);
    d = -f->dx * (x - f->x) + -f->dy * (y - f->y);
    if (d >= 0) {
        *index = j - 1;
    }
}

/**
 * @brief Draws one glyph of the time display and steps the pen left.
 * @param pos the pen position
 * @param id  the glyph's sprite id
 */
static __inline__ void putDigit(SVECTOR *pos, u16 id) {
    pos->vx -= 6;
    DrawSprite(id, pos, NULL, 0, &gSubOt[gActiveBuff]);
}

/**
 * @brief Draws a time in the HUD's small digits as minutes:seconds:hundredths,
 *        right to left from `pos`; the minutes take one or two digits.
 * @param pos    the pen, right of the last digit; moved left past the time
 * @param frames the time, in frames (30 a second)
 */
void DrawHudTime(SVECTOR *pos, s32 frames) {
    s32 n;
    s32 m;
    s32 d;
    s32 k;
    s32 t;
    s32 sec;

    n = frames * 100 / 30;
    /* MATCHING: d copied from n, then divided; d = n / 10 gives d a
     * different register. */
    d = n;
    d /= 10;
    putDigit(pos, n % 10 + SPR_SMALL_DIGIT);
    putDigit(pos, d % 10 + SPR_SMALL_DIGIT);
    putDigit(pos, SPR_SMALL_COLON);
    sec = frames / 30;
    m = sec / 60;
    n = sec % 60;
    k = 1;
    d = n;
    d /= 10;
    putDigit(pos, n % 10 + SPR_SMALL_DIGIT);
    putDigit(pos, d % 10 + SPR_SMALL_DIGIT);
    /* MATCHING: the minutes move into n between the digit and the colon. */
    n = m;
    putDigit(pos, SPR_SMALL_COLON);
    for (; k < 11; k *= 10) {
        t = n / k;
        if (t != 0 || (k == 1 && n == 0)) {
            putDigit(pos, t % 10 + SPR_SMALL_DIGIT);
        }
    }
}

/**
 * @brief Defines the sprites of the HUD, the results screens, the effects, the
 *        tools' markers and the pause caption, in the textures LoadTimList has
 *        just put in VRAM. Called from main once those are loaded.
 */
void DefineHudSprites(void) {
    s32 i;

    DefineSprite(SPR_CAN, 0, 0x40, 0x40, 0x1E, 0, 0, 0, 0x1F9);
    /* MATCHING: the u ternary tests i >= 6 first; i < 6 swaps the two
     * strength-reduced registers. */
    for (i = 0; i < 10; i++) {
        DefineSprite(i + SPR_RUNNER, 0, 0x20, 0x20, 0x1E,
                     (u8)(i >= 6 ? i * 32 - 0x80 : i * 32 + 0x40), i < 6 ? 0 : 0x20, 0x10, 0x1F9);
    }
    for (i = 0; i < 10; i++) {
        DefineSprite(i + SPR_DIGIT, 0, 0x10, 0x10, 0x1E, (u8)(i * 16), 0x40, 0x20, 0x1F9);
    }
    for (i = 0; i < 11; i++) {
        DefineSprite(i + SPR_SMALL_DIGIT, 0, 8, 8, 0x1E, (u8)(i * 8), 0x58, 0x30, 0x1F9);
    }
    for (i = 0; i < 8; i++) {
        DefineSprite(i + SPR_BURST, 0, 0x20, 0x20, 0x1E, (u8)(i * 32), 0x60, 0x40, 0x1F9);
    }
    DefineSprite(SPR_BLOB, 0, 0x20, 0x20, 0x1E, 0xA0, 0x40, 0x50, 0x1F9);
    DefineSprite(SPR_UNDRAWN, 0, 0x58, 0x28, 0x1E, 0, 0xC0, 0x60, 0x1F9);
    DefineSprite(SPR_FLOAT, 0, 0x20, 0x20, 0x1E, 0xC0, 0x20, 0x70, 0x1F9);
    DefineSprite(SPR_LIVES, 0, 0x20, 0x30, 0x1E, 0xE0, 0x20, 0x90, 0x1F9);
    DefineSprite(SPR_HURRY, 0, 0x90, 0x18, 0x1E, 0x58, 0xC0, 0xB0, 0x1F9);
    DefineSprite(SPR_HUD_TIME, 0, 0x18, 8, 0x1E, 0x58, 0x58, 0x30, 0x1F9);
    for (i = 0; i < 8; i++) {
        DefineSprite(i + SPR_BIG_DIGIT, 0, 0x20, 0x20, 0x1E, (u8)(i * 32), 0x80, 0x80, 0x1F9);
    }
    for (i = 0; i < 2; i++) {
        DefineSprite(i + SPR_BIG_DIGIT + 8, 0, 0x20, 0x20, 0x1E, (u8)(i * 32), 0xA0, 0x80, 0x1F9);
    }
    DefineSprite(SPR_PROGRESS_BAR, 0, 0x60, 0x10, 0x1E, 0x60, 0xE0, 0xA0, 0x1F9);
    for (i = 0; i < 4; i++) {
        DefineSprite(i + SPR_PROGRESS_MARK, 0, 8, 0x10, 0x1E, (u8)(i * 8 - 0x40), 0xE0, 0xA0, 0x1F9);
    }
    for (i = 0; i < 12; i++) {
        DefineSprite(i + SPR_RESULT_DIGIT, 0, 0x10, 0x10, 0x1E, (u8)(i * 16 + 0x40), 0xB0, 0x2D0, 0x1CA);
    }
    for (i = 0; i < 12; i++) {
        DefineSprite(i + SPR_RESULT_SHADOW, 0, 0x10, 0x10, 0x1E, (u8)(i * 16 + 0x40), 0xA0, 0x2D0, 0x1CA);
    }
    DefineSprite(SPR_ACT_LABEL, 0, 0x50, 0x10, 0x1B, 0x80, 0xC0, 0x2D0, 0x1C9);
    DefineSprite(SPR_ACT_LABEL + 1, 0, 0x50, 0x10, 0x1B, 0x80, 0xD0, 0x2D0, 0x1C9);
    DefineSprite(SPR_RESULT_BANNER, 0, 0x80, 0x20, 0x1B, 0x80, 0xE0, 0x2D0, 0x1C9);
    DefineSprite(SPR_WORLD_LABEL, 0, 0x80, 0x20, 0x1C, 0x80, 0xC0, 0x2D0, 0x1C9);
    for (i = 0; i < 4; i++) {
        DefineSprite(i + SPR_WORLD_DIGIT, 0, 0x20, 0x20, 0x1C, (u8)(i * 32 - 0x80), 0xE0, 0x2D0, 0x1C9);
    }
    for (i = 0; i < 4; i++) {
        DefineSprite(i + SPR_CANS_LABEL, 0, 0x80, 0x10, 0x1C, 0, (u8)(i * 16 - 0x40), 0x2D0, 0x1C9);
    }
    DefineSprite(SPR_ALL_CANS, 0, 0x68, 0x10, 0x1E, 0x80, 0xF0, 0xD0, 0x1F9);
    DefineSprite(SPR_NEW_RECORD, 0, 0x80, 0x10, 0x1E, 0, 0xF0, 0xC0, 0x1F9);
    DefineSprite(SPR_MARKER, 0, 0x10, 0x10, 0x0F, 0, 0xC0, 0x3C0, 0xFF);
    DefineSprite(SPR_POINTER, 0, 0x20, 0x20, 0x0F, 0, 0xD0, 0x3D0, 0xFF);
    DefineSprite(SPR_PAUSE, 0, 0x80, 0x20, 0x0F, 0x20, 0xC0, 0x3E0, 0xFF);
}

extern u32 sBonusDist; /**< speed summed each frame; its hundreds pick the bonus sprite's frame */

/**
 * @brief Draws a sprite scaled and rotated about a pivot; defined in draw.
 * @param id    the sprite
 * @param quad  its placement: x, y, the pivot x and y, the 4.12 x and y
 *              scales, the angle and the centring flag
 * @param color its colour, or NULL
 * @param otz   the ordering-table entry
 * @param ot    the ordering table
 */
/* MATCHING: draw takes a Sprite2D *, a type local to that unit; this
 * unit passes the same eight halfwords as an array. */
void DrawSpriteTransformed(u16 id, s16 *quad, CVECTOR *color, u16 otz, GsOT *ot);
s32 SpawnEffect(s32 kind, s16 x, s16 y, s16 z, s16 delay);

/**
 * @brief Draws the running HUD on the HUD's ordering table: the can count and
 *        icon, the bonus level's running figure (blinking at level 1,
 *        larger at 2, largest above), the seconds left (red under 11,
 *        blinking under 5), the marker on the course progress bar, the
 *        elapsed time and the lives.
 *        With 11 seconds left it starts the hurry banner and its sound; each
 *        frame it adds the player's speed to sBonusDist.
 */
void DrawHud(void) {
    s32 n;
    s32 k;
    s32 q;
    s32 digits;
    SVECTOR pos;
    s16 quad[8];
    SColor color;

    n = gCans;
    digits = 0;
    for (k = 1; k < 101; k *= 10) {
        if (n / k == 0) {
            break;
        }
        digits++;
    }
    if (n == 0) {
        pos.vx = -120;
        pos.vy = -96;
        DrawSprite(SPR_DIGIT, &pos, NULL, 0, &gSubOt[gActiveBuff]);
    }
    k = 1;
    pos.vx = (digits - 1) * 12 - 120;
    pos.vy = -96;
    for (; k < 101; k *= 10) {
        q = n / k;
        if (q != 0) {
            DrawSprite(q % 10 + SPR_DIGIT, &pos, NULL, 0, &gSubOt[gActiveBuff]);
            pos.vx -= 12;
        }
    }
    quad[0] = quad[2] = -0x90;
    quad[1] = quad[3] = -0x68;
    quad[4] = quad[5] = 0x800;
    quad[6] = 0;
    quad[7] = 0;
    DrawSpriteTransformed(SPR_CAN, quad, NULL, 0, &gSubOt[gActiveBuff]);
    quad[0] = quad[2] = -0x40;
    quad[1] = quad[3] = -0x58;
    quad[7] = 1;
    quad[6] = 0;
    sBonusDist += gPlayer.speed;
    switch (gBonusLevel) {
        case 1:
            quad[4] = quad[5] = 0x800;
            if ((gFrameCount >> 1) & 1) {
                DrawSpriteTransformed(sBonusDist / 100 % 10 + SPR_RUNNER, quad, NULL, 0,
                                      &gSubOt[gActiveBuff]);
            }
            break;
        case 2:
            quad[4] = quad[5] = 0xB50;
            DrawSpriteTransformed(sBonusDist / 100 % 10 + SPR_RUNNER, quad, NULL, 0,
                                  &gSubOt[gActiveBuff]);
            break;
        case 0:
            break;
        default:
            quad[4] = quad[5] = 0x1000;
            DrawSpriteTransformed(sBonusDist / 100 % 10 + SPR_RUNNER, quad, NULL, 0,
                                  &gSubOt[gActiveBuff]);
            break;
    }
    if (gTimeLeft == 0x14A) {
        PlaySe(SE_HURRY);
        SpawnEffect(EFFECT_HURRY, 0, 0, 0, 0);
    }
    n = gTimeLeft;
    n /= 30;
    color.r = BLEND_OPAQUE;
    if (n < 11) {
        color.g = 0x80;
        color.b = color.cd = 0;
    } else {
        color.g = color.b = color.cd = 0x80;
    }
    if (n >= 5 || ((gFrameCount >> 1) & 1)) {
        /* MATCHING: a digit count whose result is never used; the
         * divisions stay. */
        for (k = 1; k < 101; k *= 10) {
            if (n / k == 0) {
                break;
            }
        }
        if (n == 0) {
            pos.vx = 4;
            pos.vy = -0x68;
            DrawSprite(SPR_BIG_DIGIT, &pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
        }
        k = 1;
        pos.vx = 4;
        pos.vy = -0x68;
        for (; k < 101; k *= 10) {
            q = n / k;
            if (q != 0) {
                DrawSprite(q % 10 + SPR_BIG_DIGIT, &pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
                pos.vx -= 18;
            }
        }
    }
    color.r = BLEND_ADD;
    color.g = color.b = color.cd = 0x80;
    pos.vx = (gSection * 11 << 15) / ((s32 *)gSectionEdges)[-1] / 4096 + 0x34;
    pos.vy = -0x69;
    DrawSprite(((gFrameCount >> 2) & 3) + SPR_PROGRESS_MARK, &pos, (CVECTOR *)&color, 0,
               &gSubOt[gActiveBuff]);
    k = 1;
    pos.vx = 0x34;
    pos.vy = -0x64;
    DrawSprite(SPR_PROGRESS_BAR, &pos, NULL, 0, &gSubOt[gActiveBuff]);
    pos.vx = 0x34;
    pos.vy = -0x54;
    DrawSprite(SPR_HUD_TIME, &pos, NULL, 0, &gSubOt[gActiveBuff]);
    pos.vx = 0x90;
    pos.vy = -0x54;
    DrawHudTime(&pos, gElapsed);
    n = gRetries;
    pos.vx = 0x68;
    pos.vy = 0x4C;
    for (; k < 10001; k *= 10) {
        q = n / k;
        if (q != 0) {
            DrawSprite(q % 10 + SPR_DIGIT, &pos, NULL, 0, &gSubOt[gActiveBuff]);
            pos.vx -= 12;
        }
    }
    color.r = BLEND_OPAQUE;
    color.g = color.b = color.cd = 0x80;
    pos.vx = 0x70;
    pos.vy = 0x38;
    DrawSprite(SPR_LIVES, &pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
}

/**
 * @brief Absolute value.
 * @param v the value
 * @return |v|
 */
static __inline__ s32 iabs(s32 v) {
    return v >= 0 ? v : -v;
}

/**
 * @brief Intersects the line through (x0, y0) and (x1, y1) with the circle of
 *        radius 500 about the origin and writes the crossing nearer the first
 *        point. A line steeper than 45 degrees is solved a quarter turn round
 *        and turned back. A line that misses leaves `out` as it was. No code
 *        calls it.
 * @param out the crossing, x then y
 * @param x0  the first point's x
 * @param y0  the first point's y
 * @param x1  the second point's x
 * @param y1  the second point's y
 */
void LineCircleIntersect(s16 *out, s16 x0, s16 y0, s16 x1, s16 y1) {
    s32 swapped;
    s32 t;
    s32 m;
    s32 b;
    s32 c;
    s32 a;
    s32 r0;
    s32 r1;
    s32 h;
    s32 nb;
    s32 sq;

    swapped = 0;
    if (iabs(x1 - x0) < iabs(y1 - y0)) {
        t = x0;
        x0 = y0;
        y0 = -t;
        t = x1;
        /* MATCHING: the negation goes through m (free until the slope). */
        m = -t;
        x1 = y1;
        y1 = m;
        swapped = 1;
    }
    m = ((y1 - y0) << 16) / (x1 - x0);
    a = (y1 - y0) * (y1 - y0) * 4 / ((x1 - x0) * (x1 - x0));
    c = y0 - ((m * x0) >> 16);
    b = (m * c) >> 15;
    a += 4;
    /* MATCHING: one local carries the discriminant and then the half
     * leading coefficient, b * b computed into it first. */
    h = b * b;
    h -= a * (c * c - 250000);
    if (h >= 0) {
        sq = SquareRoot0(h);
        h = a >> 1;
        nb = -b;
        r1 = (nb + sq) / h;
        r0 = (nb - sq) / h;
        if (iabs(x0 - r1) <= iabs(x0 - r0)) {
            if (swapped) {
                out[1] = r1;
                out[0] = -(((m * r1) >> 16) + c);
            } else {
                out[0] = r1;
                out[1] = ((m * r1) >> 16) + c;
            }
        } else {
            if (swapped) {
                out[1] = r0;
                out[0] = -(((m * r0) >> 16) + c);
            } else {
                out[0] = r0;
                out[1] = ((m * r0) >> 16) + c;
            }
        }
    }
}

extern s32 sCamTurnStep; /**< how far gCameraYaw turns this frame toward the section's heading */
extern s16 sCamOffsetX;  /**< the view point's x offset from the player, on gCameraYaw */
extern s16 sCamOffsetZ;  /**< its z offset */
extern s16 sCamHeight;   /**< the camera height, eased by 10 toward the section's */

/** The view, gView's words read as a GsRVIEW2. */
#define sView (*(GsRVIEW2 *)gView)

/**
 * @brief Steps a boundary index across the edges either side of (x, y).
 * @param index the index to step
 * @param tbl   the edge table
 * @param x     the point's x
 * @param y     the point's y
 */
static __inline__ void stepEdge(s32 *index, Edge *tbl, s16 x, s16 y) {
    s32 i;
    s32 j;
    s32 d;
    Edge *e;
    Edge *f;

    i = *index;
    e = (Edge *)(i * 8 + (s32)tbl) + 1;
    d = e->dx * (x - e->x) + e->dy * (y - e->y);
    if (d >= 0) {
        *index = i + 1;
    }
    j = *index;
    f = (Edge *)(j * 8 + (s32)tbl);
    d = -f->dx * (x - f->x) + -f->dy * (y - f->y);
    if (d >= 0) {
        *index = j - 1;
    }
}

/**
 * @brief The course camera, once a frame: steps gSection along the course path
 *        to (x, z), turns gCameraYaw toward that section's heading at a rate
 *        that eases off as it closes, puts the view point 400 units behind the
 *        player on that yaw and the section's camera height above `floorY`
 *        (eased by 10 a frame, its climb and drop limited per frame), aims it
 *        150 units above the midpoint of the floor and the player, and sets the
 *        view.
 * @param x      the player's x
 * @param y      the player's height
 * @param z      the player's z, the course's other horizontal axis
 * @param floorY the floor's height under the player
 * @return the section's heading, which the stage takes as the course's yaw
 *         (4096 to a turn)
 */
s16 UpdateCamera(s16 x, s16 y, s16 z, s16 floorY) {
    Edge *tbl;
    Edge *e;
    /* MATCHING: s16, extended only at the return; as s32 it takes a's
     * register and a gets another. */
    s16 angle;
    s32 b;
    s32 a;
    s32 old;
    s32 v;
    s32 off;
    /* MATCHING: retail's frame has 8 more bytes below the saved registers. */
    s32 unused[2];

    tbl = gSectionEdges;
    stepEdge(&gSection, tbl, x, z);
    e = (Edge *)(gSection * 8 + (s32)tbl);
    angle = a = ratan2(e[1].x - e[0].x, e[1].y - e[0].y);
    a &= 0xFFF;
    b = gCameraYaw & 0xFFF;
    if (iabs(a - b) > 0x800) {
        if (a < 0x800) {
            a += 0x1000;
        }
        if (b < 0x800) {
            b += 0x1000;
        }
    }
    /* MATCHING: the distance re-derived in each test; a local for it gets
     * a register retail gives to a. */
    if (iabs(a - b) > 50) {
        if (sCamTurnStep < 20) {
            sCamTurnStep += 2;
        }
    } else if (iabs(a - b) > 30) {
        sCamTurnStep = 10;
    } else if (iabs(a - b) > 5) {
        sCamTurnStep = 5;
    } else if (a == b) {
        sCamTurnStep = 0;
    } else {
        sCamTurnStep = 1;
    }
    if (a - b > 0) {
        gCameraYaw += sCamTurnStep;
    } else {
        gCameraYaw -= sCamTurnStep;
    }
    sCamOffsetX = rsin((s16)gCameraYaw + ANGLE_DEG(180)) * 400 / 4096;
    sCamOffsetZ = rcos((s16)gCameraYaw + ANGLE_DEG(180)) * 400 / 4096;
    sView.vpz = sCamOffsetZ;
    sView.vpx = sCamOffsetX;
    sPrevViewY = sView.vpy;
    if (sSectionCamHeights[gSection] * 10 < sCamHeight) {
        sCamHeight -= 10;
    }
    if (sCamHeight < sSectionCamHeights[gSection] * 10) {
        sCamHeight += 10;
    }
    old = sPrevViewY;
    v = floorY + sCamHeight;
    sView.vpy = v;
    if (v - old > 200) {
        sView.vpy = old + 50;
    } else {
        /* MATCHING: the rest of the clamp reads old through the dead a
         * (permuter find); it moves a's register for the whole function. */
        a = old;
        if (v - a > 50) {
            sView.vpy = a + 20;
        } else if (v - a > 20) {
            sView.vpy = a + 10;
        }
    }
    if (sView.vpy - sPrevViewY < -200) {
        sView.vpy = sPrevViewY - 50;
    } else if (sView.vpy - sPrevViewY < -50) {
        sView.vpy = sPrevViewY - 20;
    } else if (sView.vpy - sPrevViewY < -20) {
        sView.vpy = sPrevViewY - 10;
    }
    /* MATCHING: a fresh local, so the sum stays in one register; -floorY
     * first loads floorY first. */
    off = (-floorY + y) / 2 - 150;
    sView.vrx = 0;
    sView.vry = floorY + off;
    sView.vrz = 0;
    sView.rz = 0;
    sView.super = NULL;
    GsSetRefView2L(&sView);
    return angle;
}

/**
 * @brief Draws the CPU-time meter down the screen's left edge: for each mark
 *        MarkCpuTime took this frame, a bar in its colour from the previous
 *        mark's scanline to its own; then a white tick, and clears the marks.
 *        No code calls it.
 */
void DrawCpuMeter(void) {
    POLY_F4 *p;
    s32 i;

    for (i = 0; i < gCpuMarkCount; i++) {
        if (gCpuMarkTimes[i] != 0xFF) {
            p = (POLY_F4 *)GsOUT_PACKET_P;
            setPolyF4(p);
            p->x0 = p->x2 = -160;
            p->x1 = p->x3 = -152;
            p->y0 = p->y1 = (gCpuMarkTimes[i] >> 1) - 120;
            if (i != 0) {
                p->y3 = (gCpuMarkTimes[i - 1] >> 1) - 120;
            } else {
                p->y3 = -120;
            }
            p->y2 = p->y3;
            p->r0 = sCpuMarkR[i];
            p->g0 = sCpuMarkG[i];
            p->b0 = sCpuMarkB[i];
            addPrim(gCurOt->org, p);
            p++;
            GsOUT_PACKET_P = (u8 *)p;
        }
    }
    p = (POLY_F4 *)GsOUT_PACKET_P;
    setPolyF4(p);
    p->x0 = p->x2 = -160;
    p->x1 = p->x3 = -144;
    p->y0 = p->y1 = 8;
    p->y2 = p->y3 = 9;
    setRGB0(p, 0xFF, 0xFF, 0xFF);
    gCpuMarkCount = 0;
    addPrim(gCurOt->org, p);
    p++;
    GsOUT_PACKET_P = (u8 *)p;
}

/**
 * @brief Freezes the picture: copies the frame just drawn to VRAM at (640, 0)
 *        and draws it back over the screen as two half-screen sprites, blended
 *        half and half, on the HUD's ordering table.
 */
void DrawFrozenFrame(void) {
    RECT rect;
    SVECTOR pos;
    CVECTOR color;

    DefineSprite(SPR_SCREEN_LEFT, 2, 0xA0, 0xF0, 10, 0, 0, 0, 0);
    DefineSprite(SPR_SCREEN_RIGHT, 2, 0xA0, 0xF0, 12, 0x20, 0, 0, 0);
    rect.x = 0;
    rect.y = PSDIDX * 240;
    rect.w = 320;
    rect.h = 240;
    MoveImage(&rect, 640, 0);
    color.r = BLEND_HALF;
    color.g = color.b = color.cd = 0x80;
    pos.vx = -160;
    pos.vy = -120;
    DrawSprite(SPR_SCREEN_LEFT, &pos, &color, 0, &gSubOt[gActiveBuff]);
    pos.vx = 0;
    pos.vy = -120;
    DrawSprite(SPR_SCREEN_RIGHT, &pos, &color, 0, &gSubOt[gActiveBuff]);
}

/**
 * @brief DrawFrozenFrame's opaque variant: draws the copied frame at a
 *        brightness, in the 3D ordering table's last entry, behind the scene.
 *        No code calls it.
 * @param level the brightness, 0x80 for unchanged
 */
void DrawFrozenFrameFaded(u8 level) {
    RECT rect;
    SVECTOR pos;
    CVECTOR color;

    DefineSprite(SPR_SCREEN_LEFT, 2, 0xA0, 0xF0, 10, 0, 0, 0, 0);
    DefineSprite(SPR_SCREEN_RIGHT, 2, 0xA0, 0xF0, 12, 0x20, 0, 0, 0);
    rect.x = 0;
    rect.y = PSDIDX * 240;
    rect.w = 320;
    rect.h = 240;
    MoveImage(&rect, 640, 0);
    /* MATCHING: an s8 store gives li -1; a u_char one gives li 0xFF. */
    *(s8 *)&color.r = BLEND_OPAQUE;
    color.g = color.b = color.cd = level;
    pos.vx = -160;
    pos.vy = -120;
    DrawSprite(SPR_SCREEN_LEFT, &pos, &color, 0xFFF, &gOt[gActiveBuff]);
    pos.vx = 0;
    pos.vy = -120;
    DrawSprite(SPR_SCREEN_RIGHT, &pos, &color, 0xFFF, &gOt[gActiveBuff]);
}

/**
 * @brief Draws the screen wipe while it runs: a full-screen quad on the HUD's
 *        ordering table showing one of eight 128x96 images from VRAM, each for
 *        3 frames, then ends the wipe after 24 frames. Called each frame by the
 *        stage flow.
 */
void DrawScreenWipe(void) {
    POLY_FT4 *p;
    s32 n;

    if (gWipeActive != 0) {
        if (gWipeFrame >= 24) {
            gWipeActive = 0;
            return;
        }
        n = gWipeFrame / 3;
        p = (POLY_FT4 *)GsOUT_PACKET_P;
        setPolyFT4(p);
        p->r0 = p->g0 = p->b0 = 0x80;
        p->x0 = p->x2 = -160;
        p->x1 = p->x3 = 160;
        p->y0 = p->y1 = -120;
        p->y2 = p->y3 = 120;
        p->u0 = p->u2 = (n % 2) << 7;
        p->v0 = p->v1 = (n / 2 % 2) * 96;
        p->clut = getClut(720, n + 448);
        p->u1 = p->u3 = p->u0 + 0x7F;
        p->v2 = p->v3 = p->v0 + 0x5F;
        p->tpage = getTPage(0, 0, n / 4 * 64 + 704, 256);
        addPrim(gSubOt[gActiveBuff].org, p);
        gWipeFrame++;
        p++;
        GsOUT_PACKET_P = (u8 *)p;
    }
}

/**
 * @brief Draws a results-screen can count as "n/100", right to left from `pos`,
 *        each glyph over its shadow: the 100, the slash, then the one to three
 *        digits of `n`.
 * @param pos the pen, right of the 100; moved left past the number
 * @param n   the count
 */
void DrawResultNumber(SVECTOR *pos, s32 n) {
    CVECTOR color;
    CVECTOR shadow;
    u16 d;
    s32 t;
    /* MATCHING: dead x keeps both magic constants in prologue registers. */
    u32 x;

    x = (s16)n / 10;
    color.r = BLEND_ADD;
    shadow.r = BLEND_SUB;
    shadow.g = shadow.b = shadow.cd = 0x80;
    color.g = color.b = color.cd = 0x80;
    pos->vx -= 16;
    DrawSprite(SPR_RESULT_DIGIT, pos, &color, 0, &gSubOt[gActiveBuff]);
    DrawSprite(SPR_RESULT_SHADOW, pos, &shadow, 0, &gSubOt[gActiveBuff]);
    pos->vx -= 12;
    DrawSprite(SPR_RESULT_DIGIT, pos, &color, 0, &gSubOt[gActiveBuff]);
    DrawSprite(SPR_RESULT_SHADOW, pos, &shadow, 0, &gSubOt[gActiveBuff]);
    pos->vx -= 12;
    DrawSprite(SPR_RESULT_DIGIT + 1, pos, &color, 0, &gSubOt[gActiveBuff]);
    DrawSprite(SPR_RESULT_SHADOW + 1, pos, &shadow, 0, &gSubOt[gActiveBuff]);
    pos->vx -= 12;
    DrawSprite(SPR_RESULT_SLASH, pos, &color, 0, &gSubOt[gActiveBuff]);
    DrawSprite(SPR_RESULT_SHADOW + 11, pos, &shadow, 0, &gSubOt[gActiveBuff]);
    pos->vx -= 12;
    t = n / 10;
    x = (u32)n / 100;
    d = n % 10;
    DrawSprite(d + SPR_RESULT_DIGIT, pos, &color, 0, &gSubOt[gActiveBuff]);
    DrawSprite(d + SPR_RESULT_SHADOW, pos, &shadow, 0, &gSubOt[gActiveBuff]);
    if (t != 0) {
        pos->vx -= 12;
        d = t % 10;
        DrawSprite(d + SPR_RESULT_DIGIT, pos, &color, 0, &gSubOt[gActiveBuff]);
        DrawSprite(d + SPR_RESULT_SHADOW, pos, &shadow, 0, &gSubOt[gActiveBuff]);
    }
    if (n / 100 != 0) {
        pos->vx -= 12;
        d = n / 100 % 10;
        DrawSprite(d + SPR_RESULT_DIGIT, pos, &color, 0, &gSubOt[gActiveBuff]);
        DrawSprite(d + SPR_RESULT_SHADOW, pos, &shadow, 0, &gSubOt[gActiveBuff]);
    }
}

/**
 * @brief Draws a time on the results screen as minutes:seconds:hundredths in
 *        the results digits, right to left from `pos`; the minutes take one or
 *        two digits.
 * @param pos    the pen, right of the last digit; moved left past the time
 * @param frames the time, in frames (30 a second)
 */
void DrawResultTime(SVECTOR *pos, s32 frames) {
    s32 n;
    s32 m;
    s32 d;
    s32 k;
    s32 t;
    s32 sec;
    SColor color;
    /* MATCHING: retail fills a second colour at sp+0x20 it never passes. */
    SColor shadow;

    color.r = BLEND_OPAQUE;
    shadow.r = BLEND_SUB;
    shadow.g = shadow.b = shadow.cd = 0x80;
    color.g = color.b = color.cd = 0x80;
    n = frames * 100 / 30;
    /* MATCHING: d copied from n, then divided (as in DrawHudTime). */
    d = n;
    d /= 10;
    pos->vx -= 16;
    DrawSprite(n % 10 + SPR_RESULT_DIGIT, pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
    pos->vx -= 12;
    DrawSprite(d % 10 + SPR_RESULT_DIGIT, pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
    pos->vx -= 12;
    DrawSprite(SPR_RESULT_COLON, pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
    sec = frames / 30;
    m = sec / 60;
    n = sec % 60;
    k = 1;
    d = n;
    d /= 10;
    pos->vx -= 12;
    DrawSprite(n % 10 + SPR_RESULT_DIGIT, pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
    pos->vx -= 12;
    DrawSprite(d % 10 + SPR_RESULT_DIGIT, pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
    n = m;
    pos->vx -= 12;
    DrawSprite(SPR_RESULT_COLON, pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
    for (; k < 11; k *= 10) {
        t = n / k;
        if (t != 0 || (k == 1 && n == 0)) {
            pos->vx -= 12;
            DrawSprite(t % 10 + SPR_RESULT_DIGIT, pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
        }
    }
}

/* MATCHING: 8-byte arrays (rank % 3 indexes them); as scalars indexed
 * through & the stores in UpdateActResults schedule the address first. */
extern s32 gActCans[2];  /**< the cans of the world's acts 1 and 2, for the world results */
extern s32 gActTimes[2]; /**< those acts' times, in frames */
/* MATCHING: per-unit views; main.c reads the same pairs as Pair8. */
extern s32 gBestCans[][2];  /**< the can records, by world and act */
extern s32 gBestTimes[][2]; /**< the time records, in frames, by world and act */
extern s32 gTargetTimes[][2]; /**< target times by world and act; beating all sets gAllTargetsBeaten */

void DrawResultNumber(SVECTOR *pos, s32 n);

/**
 * @brief One frame of the act results screen while it runs. It steps
 *        gResultsStep: the lines slide in, the cans are counted up (a sound
 *        every 10, a life every 25), all 100 give a flashing caption and one
 *        more life, a can or time record is stored with its caption, and at the
 *        end the target times and the 800-can total are checked for their
 *        unlocks and the act's cans and time are kept for the world results.
 *        Then it slides the lines on and draws the lives, the act label, the
 *        banner and the cans, best cans, time and best time rows. Called each
 *        frame by the stage flow.
 */
void UpdateActResults(void) {
    s32 i;
    s32 k;
    s32 q;
    s32 n;
    s32 place;
    s32 rank;
    SVECTOR pos;
    s16 quad[8];
    SColor color;
    s16 tile[4];

    if (gResultsActive == 0) {
        return;
    }
    place = gStage;
    if (place >= 0) {
        if (place < 6) {
            rank = place;
        } else if (place < 15) {
            if (place >= 9) {
                rank = place - 3;
            }
        }
    }
    color.r = BLEND_OPAQUE;
    color.g = color.b = color.cd = 0x80;
    switch (gResultsStep) {
        case RES_SLIDE_IN:
            switch (gResultsTimer) {
                case 0:
                    gResultsSliding[0] = 1;
                    break;
                case 24:
                    gResultsSlide[1] = -24;
                    break;
                case 40:
                    gResultsSliding[1] = 1;
                    break;
                case 64:
                    gResultsSliding[2] = 1;
                    break;
                case 68:
                    gResultsSliding[3] = 1;
                    break;
                case 80:
                    gResultsStep = RES_TALLY_CANS;
                    break;
            }
            gResultsTimer++;
            break;
        case RES_TALLY_CANS:
            if (gTallyCans < gCans) {
                if (gTallyCans % 10 == 0) {
                    PlaySe(SE_TALLY);
                }
                gTallyCans++;
                if (gTallyCans != 0 && gTallyCans % 25 == 0) {
                    gRetries++;
                    PlaySe(SE_POWER_UP);
                }
            }
            if (gTallyCans == gCans) {
                if (gTallyCans == 100) {
                    PlaySe(SE_ALL_CANS);
                    tile[1] = 0x1FB;
                    tile[2] = 0x10;
                    tile[3] = 1;
                    gResultsSlide[6] = -9;
                    tile[0] = 0;
                    gResultsTimer = 0;
                    gResultsStep = RES_ALL_CANS;
                } else if (gBestCans[rank / 3][rank % 3] < gTallyCans) {
                    PlaySe(SE_NEW_RECORD);
                    gResultsSlide[7] = -9;
                    gResultsTimer = 0;
                    gResultsStep = RES_BEST_CANS;
                } else {
                    gResultsTimer = 0;
                    gResultsStep = RES_SLIDE_TIME;
                }
            }
            break;
        case RES_ALL_CANS:
            if (gResultsSlide[6] == 0) {
                if (gResultsTimer < 36) {
                    tile[1] = 0x1FB;
                    tile[2] = 0x10;
                    tile[3] = 1;
                    tile[0] = gResultsTimer % 14 * 16;
                }
                if (++gResultsTimer == 60) {
                    gRetries++;
                    PlaySe(SE_POWER_UP);
                    gResultsSlide[6] = 1;
                }
            } else if (++gResultsSlide[6] == 9) {
                if (gTallyCans > gBestCans[rank / 3][rank % 3]) {
                    PlaySe(SE_NEW_RECORD);
                    gResultsSlide[7] = -9;
                    gResultsTimer = 0;
                    gResultsStep = RES_BEST_CANS;
                } else {
                    gResultsTimer = 0;
                    gResultsStep = RES_SLIDE_TIME;
                }
            }
            break;
        case RES_BEST_CANS:
            tile[3] = 1;
            tile[1] = 0x1FA;
            tile[2] = 0x10;
            tile[0] = gResultsTimer % 14 * 16;
            if (gResultsSlide[7] == 0) {
                if (++gResultsTimer == 60) {
                    gBestCans[rank / 3][rank % 3] = gTallyCans;
                    gResultsSlide[7] = 1;
                }
            } else if (++gResultsSlide[7] == 9) {
                gResultsTimer = 0;
                gResultsStep = RES_SLIDE_TIME;
            }
            break;
        case RES_SLIDE_TIME:
            switch (gResultsTimer) {
                case 15:
                    gResultsSliding[4] = 1;
                    break;
                case 19:
                    gResultsSliding[5] = 1;
                    break;
                case 31:
                    gResultsStep = RES_CHECK_TIME;
                    break;
            }
            gResultsTimer++;
            break;
        case RES_CHECK_TIME:
            if (gElapsed < gBestTimes[rank / 3][rank % 3]) {
                PlaySe(SE_NEW_RECORD);
                gResultsSlide[7] = -9;
                gResultsTimer = 0;
                gResultsStep = RES_BEST_TIME;
            } else {
                gResultsTimer = 0;
                gResultsStep = RES_END;
            }
            break;
        case RES_BEST_TIME:
            tile[3] = 1;
            tile[1] = 0x1FA;
            tile[2] = 0x10;
            tile[0] = gResultsTimer % 14 * 16;
            if (gResultsSlide[7] == 0) {
                if (++gResultsTimer == 60) {
                    gBestTimes[rank / 3][rank % 3] = gElapsed;
                    gResultsSlide[7] = 1;
                }
            } else if (++gResultsSlide[7] == 9) {
                gResultsTimer = 0;
                gResultsStep = RES_END;
            }
            break;
        case RES_END:
            if (++gResultsTimer == 60) {
                n = 0;
                for (i = 0; i < 4; i++) {
                    if (gBestTimes[i][0] >= gTargetTimes[i][0]) {
                        n = 1;
                    }
                    if (gBestTimes[i][1] >= gTargetTimes[i][1]) {
                        n = 1;
                    }
                }
                if (n == 0) {
                    gAllTargetsBeaten = 1;
                }
                n = 0;
                for (i = 0; i < 4; i++) {
                    n += gBestCans[i][0] + gBestCans[i][1];
                }
                if (n == 800) {
                    gUnlocks |= 4;
                }
                gResultsActive = 0;
                gActCans[rank % 3] = gTallyCans;
                gActTimes[rank % 3] = gElapsed;
            }
            break;
    }
    for (i = 0; i < 6; i++) {
        if (gResultsSliding[i] != 0) {
            if (gResultsSlide[i] != 0) {
                gResultsSlide[i]++;
            } else {
                gResultsSliding[i] = 0;
            }
        }
    }
    k = 1;
    n = gRetries;
    pos.vx = 0x68;
    pos.vy = -0x44;
    for (; k < 10001; k *= 10) {
        q = n / k;
        if (q != 0) {
            DrawSprite(q % 10 + SPR_DIGIT, &pos, NULL, 0, &gSubOt[gActiveBuff]);
            pos.vx -= 12;
        }
    }
    color.r = BLEND_OPAQUE;
    color.g = color.b = color.cd = 0x80;
    pos.vx = 0x70;
    pos.vy = -0x58;
    DrawSprite(SPR_LIVES, &pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
    color.r = BLEND_OPAQUE;
    color.g = color.b = 0x80;
    color.cd = 0;
    pos.vy = -16;
    pos.vx = gResultsSlide[6] * 32 - 0x34;
    DrawSprite(SPR_ALL_CANS, &pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
    pos.vy = -16;
    pos.vx = gResultsSlide[7] * 32 - 0x40;
    DrawSprite(SPR_NEW_RECORD, &pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
    color.r = BLEND_OPAQUE;
    color.g = color.b = color.cd = 0x80;
    pos.vx = -0x28;
    pos.vy = gResultsSlide[0] * 4 - 0x48;
    DrawSprite(rank % 3 + SPR_ACT_LABEL, &pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
    if (gResultsSlide[1] != -25) {
        quad[1] = -0x20;
        quad[3] = -0x40;
        quad[4] = quad[5] = (-gResultsSlide[1] << 8) + 0x1000;
        quad[0] = 0;
        quad[2] = 0;
        quad[6] = 0;
        quad[7] = 1;
        DrawSpriteTransformed(SPR_RESULT_BANNER, quad, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
    }
    pos.vy = 0;
    pos.vx = gResultsSlide[2] * 32 - 0x80;
    DrawSprite(SPR_CANS_LABEL, &pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
    pos.vx = gResultsSlide[2] * 32 + 0x80;
    DrawResultNumber(&pos, gTallyCans);
    pos.vy = 0x18;
    pos.vx = gResultsSlide[3] * 32 - 0x80;
    DrawSprite(SPR_BEST_CANS_LABEL, &pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
    pos.vx = gResultsSlide[3] * 32 + 0x80;
    DrawResultNumber(&pos, gBestCans[rank / 3][rank % 3]);
    pos.vy = 0x38;
    pos.vx = gResultsSlide[4] * 32 - 0x80;
    DrawSprite(SPR_TIME_LABEL, &pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
    pos.vx = gResultsSlide[4] * 32 + 0x80;
    DrawResultTime(&pos, gElapsed);
    pos.vy = 0x50;
    pos.vx = gResultsSlide[5] * 32 - 0x80;
    DrawSprite(SPR_BEST_TIME_LABEL, &pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
    pos.vx = gResultsSlide[5] * 32 + 0x80;
    DrawResultTime(&pos, gBestTimes[rank / 3][rank % 3]);
}

extern s32 gWorldCans; /**< the can count the world results show under the second act label */
extern s32 gWorldTime; /**< the time, in frames, shown beside it */

/**
 * @brief One frame of the world results screen while it runs: slides its seven
 *        lines in over 300 frames and then ends; draws the world's title and
 *        number, act 1's label with its cans and time, and the second label
 *        with gWorldCans and gWorldTime. Called by the chase overlays.
 */
void UpdateWorldResults(void) {
    s32 i;
    s32 k;
    s32 rank;
    SVECTOR pos;
    SColor color;

    if (gResultsActive == 0) {
        return;
    }
    k = gStage;
    if (k >= 0) {
        if (k < 6) {
            rank = k;
        } else if (k < 15) {
            if (k >= 9) {
                rank = k - 3;
            }
        }
    }
    color.r = BLEND_OPAQUE;
    color.g = color.b = color.cd = 0x80;
    switch (gResultsStep) {
        case RES_SLIDE_IN:
            switch (gResultsTimer) {
                case 0:
                    gResultsSliding[0] = 1;
                    break;
                case 60:
                    gResultsSliding[1] = 1;
                    break;
                case 90:
                    gResultsSliding[2] = 1;
                    break;
                case 94:
                    gResultsSliding[3] = 1;
                    break;
                case 120:
                    gResultsSliding[4] = 1;
                    break;
                case 150:
                    gResultsSliding[5] = 1;
                    break;
                case 154:
                    gResultsSliding[6] = 1;
                    break;
                case 300:
                    gResultsStep = RES_WORLD_DONE;
                    break;
            }
            gResultsTimer++;
            break;
        case RES_WORLD_DONE:
            gResultsActive = 0;
            break;
    }
    for (i = 0; i < 7; i++) {
        if (gResultsSliding[i] != 0) {
            if (gResultsSlide[i] != 0) {
                gResultsSlide[i]++;
            } else {
                gResultsSliding[i] = 0;
            }
        }
    }
    pos.vx = -80;
    pos.vy = gResultsSlide[0] * 4 - 88;
    DrawSprite(SPR_WORLD_LABEL, &pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
    pos.vx = 48;
    DrawSprite(rank / 3 + SPR_WORLD_DIGIT, &pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
    pos.vy = -48;
    pos.vx = gResultsSlide[1] * 32 - 40;
    DrawSprite(SPR_ACT_LABEL, &pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
    pos.vy = -24;
    pos.vx = gResultsSlide[2] * 32 - 128;
    DrawSprite(SPR_CANS_LABEL, &pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
    pos.vx = gResultsSlide[2] * 32 + 128;
    DrawResultNumber(&pos, gActCans[0]);
    pos.vy = 0;
    pos.vx = gResultsSlide[3] * 32 - 128;
    DrawSprite(SPR_TIME_LABEL, &pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
    pos.vx = gResultsSlide[3] * 32 + 128;
    DrawResultTime(&pos, gActTimes[0]);
    pos.vy = 24;
    pos.vx = gResultsSlide[4] * 32 - 40;
    DrawSprite(SPR_ACT_LABEL + 1, &pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
    pos.vy = 48;
    pos.vx = gResultsSlide[5] * 32 - 128;
    DrawSprite(SPR_CANS_LABEL, &pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
    pos.vx = gResultsSlide[5] * 32 + 128;
    DrawResultNumber(&pos, gWorldCans);
    pos.vy = 72;
    pos.vx = gResultsSlide[6] * 32 - 128;
    DrawSprite(SPR_TIME_LABEL, &pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
    pos.vx = gResultsSlide[6] * 32 + 128;
    DrawResultTime(&pos, gWorldTime);
}

/**
 * @brief Copies the pad's words, gPadTrig and gPad, into the game's,
 *        gGamePadTrig and gGamePad, with the three face buttons (0x20, 0x40,
 *        0x80) moved to the bits gButtonConfig's row of sButtonMaps gives them.
 *        Called once a frame from main.
 */
void RemapButtons(void) {
    Bytes16 tbl;
    s32 k;
    u32 a;
    u32 b;

    /* MATCHING: the row index in a local, indexed per use; a row pointer
     * moves the address add above the first andi. */
    tbl = sButtonMaps;
    a = gPadTrig;
    b = gPad;
    k = gButtonConfig;
    gGamePadTrig = a;
    gGamePad = b;
    gGamePadTrig = a & ~0xE0;
    gGamePad = b & ~0xE0;
    gGamePadTrig |= ((a & 0x20) >> 5) << (tbl.b[k][0] + 5);
    gGamePadTrig |= ((a & 0x40) >> 6) << (tbl.b[k][1] + 5);
    gGamePadTrig |= ((a & 0x80) >> 7) << (tbl.b[k][2] + 5);
    gGamePad |= ((b & 0x20) >> 5) << (tbl.b[k][0] + 5);
    gGamePad |= ((b & 0x40) >> 6) << (tbl.b[k][1] + 5);
    gGamePad |= ((b & 0x80) >> 7) << (tbl.b[k][2] + 5);
}

/**
 * @brief Maps the stage TMD and links its objects to the stage object table, as
 *        linkStage does inline. No code calls it.
 * @param tmd the file, at its leading word
 */
void LinkStageTmd(unsigned long *tmd) {
    u32 i;
    GsDOBJ2 *obj;

    tmd++;
    GsMapModelingData(tmd);
    tmd++;
    gStageObjCount = *tmd;
    tmd++;
    for (i = 0; i < gStageObjCount; i++) {
        GsLinkObject4((unsigned long)tmd, &gStageObjs[i], i);
    }
    obj = gStageObjs;
    for (i = 0; i < gStageObjCount; i++) {
        obj->coord2 = &gStageCoord;
        obj->attribute = 0x200;
        obj++;
    }
}

/**
 * @brief Maps the props TMD and links its objects to the props object table, as
 *        linkProps does inline. No code calls it.
 * @param tmd the file, at its leading word
 */
void LinkPropsTmd(unsigned long *tmd) {
    u32 i;
    GsDOBJ2 *obj;

    tmd++;
    GsMapModelingData(tmd);
    tmd++;
    gPropObjCount = *tmd;
    tmd++;
    for (i = 0; i < gPropObjCount; i++) {
        GsLinkObject4((unsigned long)tmd, &gPropObjs[i], i);
    }
    obj = gPropObjs;
    for (i = 0; i < gPropObjCount; i++) {
        obj->coord2 = &gStageCoord;
        obj->attribute = 0;
        obj++;
    }
}

/**
 * @brief Loads a run of TIM images into VRAM, each one's CLUT and then its
 *        pixels, until a word that is not a TIM's leading 0x10.
 */
void LoadTimList(unsigned long *p) {
    RECT rect;
    GsIMAGE tim;

    while (*p == 0x10) {
        GsGetTimInfo(p + 1, &tim);
        rect.x = tim.cx;
        rect.y = tim.cy;
        rect.w = tim.cw;
        rect.h = tim.ch;
        LoadImage(&rect, (u_long *)tim.clut);
        rect.x = tim.px;
        rect.y = tim.py;
        rect.w = tim.pw;
        rect.h = tim.ph;
        LoadImage(&rect, (u_long *)tim.pixel);
        p += 2;
        p += *p >> 2;
        p += *p >> 2;
    }
}

/**
 * @brief Projects the first point onto the line through (x0, y0) and (x1, y1):
 *        writes the step from (x0, y0) to the point of that line nearest the
 *        origin (not the point itself). No code calls it.
 * @param out the step, x then y
 * @param x0  the first point's x
 * @param y0  the first point's y
 * @param x1  the second point's x
 * @param y1  the second point's y
 */
void NearestPointOffset(s16 *out, s16 x0, s16 y0, s16 x1, s16 y1) {
    s32 dx;
    s32 dy;
    s32 t;
    s32 d;

    dx = x1 - x0;
    dy = y1 - y0;
    t = -(x0 * dx + y0 * dy);
    d = dx * dx + dy * dy;
    out[0] = t * dx / d;
    out[1] = t * dy / d;
}

/**
 * @brief Takes a CPU-time mark for DrawCpuMeter: the scanline count now, from
 *        VSync(1), and the colour its bar is drawn in; at most 16 a frame. No
 *        code calls it.
 * @param r the bar's red
 * @param g its green
 * @param b its blue
 */
void MarkCpuTime(s32 r, s32 g, s32 b) {
    if (gCpuMarkCount < 16) {
        gCpuMarkTimes[gCpuMarkCount] = VSync(1);
        sCpuMarkR[gCpuMarkCount] = r;
        sCpuMarkG[gCpuMarkCount] = g;
        sCpuMarkB[gCpuMarkCount] = b;
        gCpuMarkCount++;
    }
}

/**
 * @brief MISS_TO_MENU's step: computes nine values from `level` and keeps
 *        none of them.
 * @param level the step's progress, clamped to 0x80
 */
void StepQuitEffect(s32 level) {
    s32 i;
    s32 v;
    s32 base;

    struct {
        SVECTOR a;
        SVECTOR b;
        CVECTOR c;
    } st;

    if (level > 0x80) {
        level = 0x80;
    }
    for (i = 8; i >= 0; i--) {
        base = i * 3 + 0x40;
        v = base - level;
        if (v < 0) {
            v = 0;
        }
        st.a.vy = st.a.pad = st.a.vx = st.a.vz = 0;
        st.b.vx = st.b.vy = v * 32 + 0x1000;
        st.b.vz = (v << 12) / 360 * 3;
        st.b.pad = 1;
        st.c.r = 1;
        st.c.g = st.c.b = st.c.cd = 0x10;
    }
}

/**
 * @brief Starts the screen wipe from its first image, with its sound.
 */
void StartScreenWipe(void) {
    gWipeFrame = 0;
    gWipeActive = 1;
    PlaySe(SE_WIPE);
}

/**
 * @brief Whether the screen wipe still runs.
 * @return nonzero while it runs
 */
s32 IsScreenWipeActive(void) {
    return gWipeActive;
}

/**
 * @brief Starts the act results screen: its first step, the timer and the can
 *        count at 0, and each line's slide offset from sActResultsSlide.
 */
void StartActResults(void) {
    s32 i;
    Bytes8 buf;

    buf = *(Bytes8 *)sActResultsSlide;
    gResultsStep = RES_SLIDE_IN;
    gResultsTimer = 0;
    gResultsActive = 1;
    gTallyCans = 0;
    for (i = 0; i < 8; i++) {
        gResultsSlide[i] = buf.b[i];
        gResultsSliding[i] = 0;
    }
}

/**
 * @brief Starts the world results screen: its first step, the timer at 0, and
 *        each line's slide offset from sWorldResultsSlide.
 */
void StartWorldResults(void) {
    s32 i;
    Bytes8 buf;

    buf = *(Bytes8 *)sWorldResultsSlide;
    gResultsStep = RES_SLIDE_IN;
    gResultsTimer = 0;
    gResultsActive = 1;
    gTallyCans = 0;
    for (i = 0; i < 8; i++) {
        gResultsSlide[i] = buf.b[i];
        gResultsSliding[i] = 0;
    }
}

/**
 * @brief Whether a results screen still runs; the stages wait on it.
 * @return nonzero while it runs
 */
s32 IsResultsActive(void) {
    return gResultsActive;
}

/* MATCHING: inlined, so each call rebuilds the two struct addresses. */
/**
 * @brief Sets the GS local-screen matrix to a pure translation (x, y, z);
 *        the sprite steps below expand it in place.
 * @param x translation x
 * @param y translation y
 * @param z translation z
 */
static __inline__ void setLs(s16 x, s16 y, s16 z) {
    GsCOORDINATE2 coord;
    MATRIX ls;

    GsInitCoordinate2(WORLD, &coord);
    coord.coord.t[0] = x;
    coord.coord.t[1] = y;
    coord.coord.t[2] = z;
    coord.flg = 0;
    GsGetLs(&coord, &ls);
    GsSetLsMatrix(&ls);
}

/**
 * @brief EFFECT_TWINKLE: a puff, SPR_BURST's sixth frame, as an additive
 *        standing billboard whose width and height swell and shrink over 15
 *        frames, after its delay.
 * @param p  the effect
 * @param ot the ordering table UpdateEffects chose for it
 * @param x  the stage's x offset, added to the effect's own
 * @param y  the stage's y offset
 * @param z  the stage's z offset
 * @return nonzero on its last frame, else 0
 */
s32 StepTwinkle(Effect *p, GsOT *ot, s16 x, s16 y, s16 z) {
    SVECTOR size;
    CVECTOR color;

    if (p->delay == 0) {
        setLs(x + p->x, y + p->y, z + p->z);
        /* MATCHING: retail shifts the scaled sine with srl. */
        size.vx = (u32)(rcos((p->frame * 3 << 13) / 360) * 25) >> 9;
        size.vy = (u32)(rsin((p->frame << 15) / 360) * 25) >> 8;
        color.r = BLEND_ADD;
        color.g = color.b = color.cd = 0x80;
        DrawStandingBillboard(SPR_BURST + 5, &size, &color, 2, ot);
        return ++p->frame == 15;
    }
    p->delay--;
    return 0;
}

/**
 * @brief EFFECT_GLINT: an additive bar on the screen itself, its corner at the
 *        effect's x and y and its z the width, that spreads about its centre
 *        and fades over 8 frames. It ignores the stage's offset and the delay.
 * @param p  the effect
 * @param ot the ordering table UpdateEffects chose for it
 * @param x  the stage's x offset, added to the effect's own
 * @param y  the stage's y offset
 * @param z  the stage's z offset
 * @return nonzero on its last frame, else 0
 */
s32 StepGlint(Effect *p, GsOT *ot, s16 x, s16 y, s16 z) {
    s16 c[2];
    POLY_F4 *f;
    DR_TPAGE *t;

    c[0] = p->x + (p->z >> 1) + 2;
    c[1] = p->y + 4;
    f = (POLY_F4 *)GsOUT_PACKET_P;
    setPolyF4(f);
    setSemiTrans(f, 1);
    f->x0 = p->x;
    f->x1 = p->x + p->z;
    f->x2 = f->x0 + 4;
    f->x3 = f->x1 + 4;
    f->y0 = f->y1 = p->y;
    f->y2 = f->y3 = p->y + 8;
    f->x0 = c[0] + ((c[0] - f->x0) * ((p->frame << 9) + 0x1000) >> 12);
    f->x1 = c[0] + ((c[0] - f->x1) * ((p->frame << 9) + 0x1000) >> 12);
    f->x2 = c[0] + ((c[0] - f->x2) * ((p->frame << 9) + 0x1000) >> 12);
    f->x3 = c[0] + ((c[0] - f->x3) * ((p->frame << 9) + 0x1000) >> 12);
    f->y0 = c[1] + ((c[1] - f->y0) * ((p->frame << 10) + 0x1000) >> 12);
    f->y1 = c[1] + ((c[1] - f->y1) * ((p->frame << 10) + 0x1000) >> 12);
    f->y2 = c[1] + ((c[1] - f->y2) * ((p->frame << 10) + 0x1000) >> 12);
    f->y3 = c[1] + ((c[1] - f->y3) * ((p->frame << 10) + 0x1000) >> 12);
    f->r0 = f->g0 = f->b0 = ~(p->frame << 5);
    addPrim(ot->org, f);
    f++;
    GsOUT_PACKET_P = (u8 *)f;
    t = (DR_TPAGE *)GsOUT_PACKET_P;
    setDrawTPage(t, 1, 1, getTPage(0, 1, 0, 0));
    addPrim(ot->org, t);
    t++;
    GsOUT_PACKET_P = (u8 *)t;
    return ++p->frame == 8;
}

/**
 * @brief EFFECT_CAN_COLLECT: after its delay, takes the can's screen position
 *        and flies it to the HUD's can icon over 8 frames, shrinking, turning
 *        and fading; then adds the can to gCans and checks the can bonus.
 * @param p  the effect
 * @param ot the ordering table UpdateEffects chose for it
 * @param x  the stage's x offset, added to the effect's own
 * @param y  the stage's y offset
 * @param z  the stage's z offset
 * @return nonzero on its last frame, else 0
 */
s32 StepCanCollect(Effect *p, GsOT *ot, s16 x, s16 y, s16 z) {
    s16 q[8];
    CVECTOR color;
    s32 unused[2];

    if (p->delay == 0) {
        if (p->frame == 0) {
            setLs(x, y, z);
            RotTransPers((SVECTOR *)&p->x, (long *)q, NULL, NULL);
            p->x = q[0];
            p->y = q[1];
        }
        q[0] = q[2] = p->x + ((-136 - p->x) * p->frame >> 3);
        q[1] = q[3] = p->y + ((-88 - p->y) * p->frame >> 3);
        q[4] = q[5] = 0x1000 - (p->frame << 8);
        q[6] = p->frame << 8;
        q[7] = 1;
        color.r = BLEND_HALF;
        color.g = color.b = color.cd = ~(p->frame << 4);
        DrawSpriteTransformed(SPR_CAN, q, &color, 0, &gSubOt[gActiveBuff]);
        if (++p->frame == 8) {
            gCans++;
            CheckCanBonus();
            return 1;
        }
        return 0;
    }
    p->delay--;
    return 0;
}

/**
 * @brief EFFECT_FIVE_CANS: StepCanCollect for five cans at once, which circle
 *        in toward their common path as they fly to the HUD; adds five cans.
 * @param p  the effect
 * @param ot the ordering table UpdateEffects chose for it
 * @param x  the stage's x offset, added to the effect's own
 * @param y  the stage's y offset
 * @param z  the stage's z offset
 * @return nonzero on its last frame, else 0
 */
s32 StepFiveCans(Effect *p, GsOT *ot, s16 x, s16 y, s16 z) {
    s16 q[8];
    CVECTOR color;
    s32 unused[2];
    s32 i;

    if (p->delay == 0) {
        if (p->frame == 0) {
            setLs(x, y, z);
            RotTransPers((SVECTOR *)&p->x, (long *)q, NULL, NULL);
            p->x = q[0];
            p->y = q[1];
        }
        for (i = 0; i < 5; i++) {
            q[0] = p->x + ((-136 - p->x) * p->frame >> 3);
            q[1] = p->y + ((-88 - p->y) * p->frame >> 3);
            q[0] = q[2] =
                q[0] + rsin(((p->frame * 20 + i * 72) << 12) / 360) * (64 - p->frame * 8) / 4096;
            q[1] = q[3] =
                q[1] + rcos(((p->frame * 20 + i * 72) << 12) / 360) * (64 - p->frame * 8) / 4096;
            q[4] = q[5] = 0x1000 - (p->frame << 8);
            q[6] = -(((p->frame * 20 + i * 72 + 90) << 12) / 360);
            q[7] = 1;
            color.r = BLEND_HALF;
            color.g = color.b = color.cd = ~(p->frame << 4);
            DrawSpriteTransformed(SPR_CAN, q, &color, 0, &gSubOt[gActiveBuff]);
        }
        if (++p->frame == 8) {
            gCans += 5;
            CheckCanBonus();
            return 1;
        }
        return 0;
    }
    p->delay--;
    return 0;
}

/**
 * @brief EFFECT_HURRY: the hurry banner, after its delay, slides in from the
 *        left for 16 frames, flashes from white to red for 16 and slides out to
 *        the right in red for 16.
 * @param p the effect
 * @return nonzero on its last frame, else 0
 */
s32 StepHurry(Effect *p) {
    SVECTOR pos;
    SColor color;

    if (p->delay == 0) {
        if (p->frame < 16) {
            pos.vx = -72 - (16 - p->frame) * 15;
            pos.vy = -12;
            color.r = BLEND_OPAQUE;
            color.g = color.b = color.cd = 0x80;
            DrawSprite(SPR_HURRY, &pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
        } else if (p->frame < 32) {
            pos.vx = -72;
            pos.vy = -12;
            color.r = BLEND_OPAQUE;
            color.g = 0x80;
            color.b = color.cd = (8 - (p->frame - 16) % 8) * 16;
            DrawSprite(SPR_HURRY, &pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
        } else {
            pos.vx = (p->frame - 32) * 15 - 72;
            pos.vy = -12;
            color.r = BLEND_OPAQUE;
            color.g = 0x80;
            color.b = color.cd = 0;
            DrawSprite(SPR_HURRY, &pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
        }
        return ++p->frame == 48;
    }
    p->delay--;
    return 0;
}

extern SVECTOR sPieceRots[]; /**< each piece object's rotation */

/**
 * @brief EFFECT_FLYING_PIECE: throws piece object 10 + param0 along x by param1
 *        a frame, in a half-sine arc param2 high over 30 frames, spinning about
 *        x by param3 degrees a frame and leaving dust every 5 frames.
 * @param p  the effect
 * @param ot the ordering table UpdateEffects chose for it
 * @param x  the stage's x offset, added to the effect's own
 * @param y  the stage's y offset
 * @param z  the stage's z offset
 * @return nonzero on its last frame, else 0
 */
s32 StepFlyingPiece(Effect *p, GsOT *ot, s16 x, s16 y, s16 z) {
    MATRIX m;

    if (p->delay == 0) {
        GsInitCoordinate2(WORLD, &sPieceCoords[p->param0 + 10]);
        sPieceCoords[p->param0 + 10].coord.t[0] = p->x + x;
        sPieceCoords[p->param0 + 10].coord.t[1] =
            p->y - rsin(p->frame * 3 * 8192 / 360) * p->param2 / 4096 + y;
        sPieceCoords[p->param0 + 10].coord.t[2] = p->z + z;
        sPieceCoords[p->param0 + 10].flg = 0;
        sPieceRots[p->param0 + 10].vx = (p->frame * p->param3 << 12) / 360;
        sPieceRots[p->param0 + 10].vy = 0;
        sPieceRots[p->param0 + 10].vz = 0;
        SetCoordRotation(&sPieceRots[p->param0 + 10], &sPieceCoords[p->param0 + 10]);
        GsGetLs(&sPieceCoords[p->param0 + 10], &m);
        GsSetLsMatrix(&m);
        GsGetLw(&sPieceCoords[p->param0 + 10], &m);
        GsSetLightMatrix(&m);
        if (p->frame % 5 == 0) {
            SpawnEffect(EFFECT_DUST, p->x,
                        p->y - rsin(p->frame * 3 * 8192 / 360) * p->param2 / 4096, p->z, 0);
        }
        p->x += p->param1;
        GsSortObject4(&sPieceObjs[p->param0 + 10], ot, 2, getScratchAddr(0));
        return ++p->frame > 29;
    }
    p->delay--;
    return 0;
}

/**
 * @brief EFFECT_SKID_DUST: an additive blob that grows and fades over 32 frames
 *        while it slides toward the angle param0 at speed param1, which drops
 *        by one a frame, and rises by param2 a frame.
 * @param p  the effect
 * @param ot the ordering table UpdateEffects chose for it
 * @param x  the stage's x offset, added to the effect's own
 * @param y  the stage's y offset
 * @param z  the stage's z offset
 * @return nonzero on its last frame, else 0
 */
s32 StepSkidDust(Effect *p, GsOT *ot, s16 x, s16 y, s16 z) {
    SVECTOR size;
    CVECTOR color;

    if (p->delay == 0) {
        setLs(x + p->x, y + p->y, z + p->z);
        size.vx = p->frame * 10 + 100;
        size.vy = p->frame * 10 + 100;
        color.r = BLEND_ADD;
        color.g = color.b = color.cd = 0x80 - (p->frame << 2);
        DrawBillboard(SPR_BLOB, &size, &color, 2, ot);
        if (p->param1 != 0) {
            p->param1--;
        }
        p->x += rsin(p->param0) * p->param1 / 4096;
        p->z += rcos(p->param0) * p->param1 / 4096;
        p->y -= p->param2;
        return ++p->frame == 32;
    }
    p->delay--;
    return 0;
}

/** @brief One frame of an effect; returns nonzero when it ends. */
typedef s32 (*EffectStep)(Effect *p, GsOT *ot, s16 x, s16 y, s16 z);

/** The disc file table's 15th to 24th copies (disc_files.inc), which no
 * code here reads; they sit between player's gHitReactions and
 * gEffectSteps. This one names the 15th copy's arrays. */
#define DISC_FILE(x) sFiles15##x
#include "disc_files.inc"
/** Names the arrays of the table's 16th copy. */
#define DISC_FILE(x) sFiles16##x
#include "disc_files.inc"
/** Names the arrays of the table's 17th copy. */
#define DISC_FILE(x) sFiles17##x
#include "disc_files.inc"
/** Names the arrays of the table's 18th copy. */
#define DISC_FILE(x) sFiles18##x
#include "disc_files.inc"
/** Names the arrays of the table's 19th copy. */
#define DISC_FILE(x) sFiles19##x
#include "disc_files.inc"
/** Names the arrays of the table's 20th copy. */
#define DISC_FILE(x) sFiles20##x
#include "disc_files.inc"
/** Names the arrays of the table's 21th copy. */
#define DISC_FILE(x) sFiles21##x
#include "disc_files.inc"
/** Names the arrays of the table's 22th copy. */
#define DISC_FILE(x) sFiles22##x
#include "disc_files.inc"
/** Names the arrays of the table's 23th copy. */
#define DISC_FILE(x) sFiles23##x
#include "disc_files.inc"
/** Names the arrays of the table's 24th copy. */
#define DISC_FILE(x) sFiles24##x
#include "disc_files.inc"

s32 StepBurst(Effect *p, GsOT *ot, s16 x, s16 y, s16 z);
s32 StepSmoke(Effect *p, GsOT *ot, s16 x, s16 y, s16 z);
s32 StepFlash(Effect *p, GsOT *ot, s16 x, s16 y, s16 z);
s32 StepDust(Effect *p, GsOT *ot, s16 x, s16 y, s16 z);
s32 StepDarkDust(Effect *p, GsOT *ot, s16 x, s16 y, s16 z);
s32 StepFloat(Effect *p, GsOT *ot, s16 x, s16 y, s16 z);

/** Each EffectKind's step function. StepHurry takes the effect alone. */
EffectStep gEffectSteps[] = {
    StepBurst,             /* EFFECT_BURST */
    StepTwinkle,           /* EFFECT_TWINKLE */
    StepSmoke,             /* EFFECT_SMOKE */
    StepFlash,             /* EFFECT_FLASH */
    StepDust,              /* EFFECT_DUST */
    StepGlint,             /* EFFECT_GLINT */
    StepDarkDust,          /* EFFECT_DARK_DUST */
    StepCanCollect,        /* EFFECT_CAN_COLLECT */
    StepFiveCans,          /* EFFECT_FIVE_CANS */
    StepFloat,             /* EFFECT_FLOAT */
    (EffectStep)StepHurry, /* EFFECT_HURRY */
    StepFlyingPiece,       /* EFFECT_FLYING_PIECE */
    StepSkidDust,          /* EFFECT_SKID_DUST */
};

/**
 * @brief Runs one frame of every active effect, from the most recently spawned:
 *        an effect more than about 495 units from the player along x goes into
 *        the 3D ordering table, a nearer one into the HUD's. A finished effect
 *        goes back to the free list.
 * @param unused unused; the callers pass the HUD's ordering table
 * @param x      the stage's x offset, the player's x negated
 * @param y      the stage's y offset
 * @param z      the stage's z offset
 */
void UpdateEffects(s32 unused, s16 x, s16 y, s16 z) {
    u8 i;
    u8 prev;
    u8 next;
    s32 d;
    s32 done;
    Effect *slot;

    prev = 0xFF;
    if ((s8)gEffectActive < 0) {
        return;
    }
    i = gEffectActive;
    do {
        slot = &gEffects[i];
        d = x + slot->x;
        next = slot->next;
        if (d * (d * 2) > 0x77A0F) {
            done = gEffectSteps[slot->kind](slot, &gOt[gActiveBuff], x, y, z);
        } else {
            done = gEffectSteps[slot->kind](slot, &gSubOt[gActiveBuff], x, y, z);
        }
        if (done) {
            if (prev == 0xFF) {
                gEffectActive = gEffects[i].next;
            } else {
                gEffects[prev].next = gEffects[i].next;
            }
            gEffects[i].next = gEffectFree;
            gEffectFree = i;
        } else {
            prev = i;
        }
        if ((s8)next < 0) {
            break;
        }
        i = next;
    } while (1);
}

/**
 * @brief Starts an effect: moves a record from the free list to the head of the
 *        active list and sets its kind, position and delay; the caller fills in
 *        its params.
 * @param kind  an EffectKind
 * @param x     its x
 * @param y     its y
 * @param z     its z
 * @param delay the frames before it is first drawn
 * @return 0, or -1 when every record is in use
 */
s32 SpawnEffect(s32 kind, s16 x, s16 y, s16 z, s16 delay) {
    u8 prev;
    Effect *slot;

    if ((s8)gEffectFree < 0) {
        return -1;
    }
    prev = gEffectActive;
    gEffectActive = gEffectFree;
    gEffectFree = gEffects[gEffectActive].next;
    gEffects[gEffectActive].next = prev;
    gEffects[gEffectActive].kind = kind;
    slot = &gEffects[gEffectActive];
    slot->x = x;
    slot->y = y;
    slot->z = z;
    slot->frame = 0;
    slot->delay = delay;
    return 0;
}

/**
 * @brief Sets the local-screen matrix to a pure translation (x, y, z), as setLs
 *        does inline. No code calls it.
 * @param x translation x
 * @param y translation y
 * @param z translation z
 */
void SetLsTranslation(s16 x, s16 y, s16 z) {
    GsCOORDINATE2 coord;
    MATRIX ls;

    GsInitCoordinate2(WORLD, &coord);
    coord.coord.t[0] = x;
    coord.coord.t[1] = y;
    coord.coord.t[2] = z;
    coord.flg = 0;
    GsGetLs(&coord, &ls);
    GsSetLsMatrix(&ls);
}

/**
 * @brief EFFECT_BURST: after its delay, plays the 8 SPR_BURST frames as an
 *        additive billboard, 150 units across.
 * @param p  the effect
 * @param ot the ordering table UpdateEffects chose for it
 * @param x  the stage's x offset, added to the effect's own
 * @param y  the stage's y offset
 * @param z  the stage's z offset
 * @return nonzero on its last frame, else 0
 */
s32 StepBurst(Effect *p, GsOT *ot, s16 x, s16 y, s16 z) {
    SVECTOR size;
    CVECTOR color;

    if (p->delay == 0) {
        setLs(x + p->x, y + p->y, z + p->z);
        size.vy = size.vx = 0x96;
        color.r = BLEND_ADD;
        color.g = color.b = color.cd = 0x80;
        DrawBillboard((u16)(p->frame + SPR_BURST), &size, &color, 2, ot);
        return ++p->frame == 8;
    }
    p->delay--;
    return 0;
}

/**
 * @brief EFFECT_SMOKE: a puff that grows slowly, rises 4 a frame, drifts along
 *        z and fades over 32 frames, after its delay.
 * @param p  the effect
 * @param ot the ordering table UpdateEffects chose for it
 * @param x  the stage's x offset, added to the effect's own
 * @param y  the stage's y offset
 * @param z  the stage's z offset
 * @return nonzero on its last frame, else 0
 */
s32 StepSmoke(Effect *p, GsOT *ot, s16 x, s16 y, s16 z) {
    SVECTOR size;
    CVECTOR color;

    if (p->delay == 0) {
        setLs(x + p->x, y + p->y, z + p->z);
        size.vx = p->frame + 0x14;
        size.vy = p->frame + 0x14;
        color.r = BLEND_ADD;
        color.g = color.b = color.cd = 0x80 - (p->frame << 2);
        DrawBillboard(SPR_BURST + 5, &size, &color, 2, ot);
        p->y -= 4;
        p->z += p->frame >> 2;
        return ++p->frame == 0x20;
    }
    p->delay--;
    return 0;
}

/**
 * @brief EFFECT_FLASH: SPR_BURST's first frame, 50 units across, fading over 8
 *        frames after its delay.
 * @param p  the effect
 * @param ot the ordering table UpdateEffects chose for it
 * @param x  the stage's x offset, added to the effect's own
 * @param y  the stage's y offset
 * @param z  the stage's z offset
 * @return nonzero on its last frame, else 0
 */
s32 StepFlash(Effect *p, GsOT *ot, s16 x, s16 y, s16 z) {
    SVECTOR size;
    CVECTOR color;

    if (p->delay == 0) {
        setLs(x + p->x, y + p->y, z + p->z);
        size.vy = size.vx = 0x32;
        color.r = BLEND_ADD;
        color.g = color.b = color.cd = 0x80 - (p->frame << 4);
        DrawBillboard(SPR_BURST, &size, &color, 2, ot);
        return ++p->frame == 8;
    }
    p->delay--;
    return 0;
}

/**
 * @brief EFFECT_DUST: an additive blob that grows from 100 units and fades over
 *        8 frames, after its delay.
 * @param p  the effect
 * @param ot the ordering table UpdateEffects chose for it
 * @param x  the stage's x offset, added to the effect's own
 * @param y  the stage's y offset
 * @param z  the stage's z offset
 * @return nonzero on its last frame, else 0
 */
s32 StepDust(Effect *p, GsOT *ot, s16 x, s16 y, s16 z) {
    SVECTOR size;
    CVECTOR color;

    if (p->delay == 0) {
        setLs(x + p->x, y + p->y, z + p->z);
        size.vx = p->frame * 10 + 100;
        size.vy = p->frame * 10 + 100;
        color.r = BLEND_ADD;
        color.g = color.b = color.cd = 0x80 - (p->frame << 4);
        DrawBillboard(SPR_BLOB, &size, &color, 2, ot);
        return ++p->frame == 8;
    }
    p->delay--;
    return 0;
}

/**
 * @brief EFFECT_DARK_DUST: StepDust's blob, subtracted from the picture instead
 *        of added.
 * @param p  the effect
 * @param ot the ordering table UpdateEffects chose for it
 * @param x  the stage's x offset, added to the effect's own
 * @param y  the stage's y offset
 * @param z  the stage's z offset
 * @return nonzero on its last frame, else 0
 */
s32 StepDarkDust(Effect *p, GsOT *ot, s16 x, s16 y, s16 z) {
    SVECTOR size;
    CVECTOR color;

    if (p->delay == 0) {
        setLs(x + p->x, y + p->y, z + p->z);
        size.vx = p->frame * 10 + 100;
        size.vy = p->frame * 10 + 100;
        color.r = BLEND_SUB;
        color.g = color.b = color.cd = 0x80 - (p->frame << 4);
        DrawBillboard(SPR_BLOB, &size, &color, 2, ot);
        return ++p->frame == 8;
    }
    p->delay--;
    return 0;
}

/**
 * @brief EFFECT_FLOAT: SPR_FLOAT, half-blended and 100 units across, moving
 *        param0 a frame along y until it has moved param1, after its delay.
 * @param p  the effect
 * @param ot the ordering table UpdateEffects chose for it
 * @param x  the stage's x offset, added to the effect's own
 * @param y  the stage's y offset
 * @param z  the stage's z offset
 * @return nonzero on its last frame, else 0
 */
s32 StepFloat(Effect *p, GsOT *ot, s16 x, s16 y, s16 z) {
    SVECTOR size;
    CVECTOR color;

    if (p->delay == 0) {
        setLs(x + p->x, y + p->y + p->param0 * p->frame, z + p->z);
        size.vy = size.vx = 100;
        color.r = BLEND_HALF;
        color.g = color.b = color.cd = 0x80;
        DrawBillboard(SPR_FLOAT, &size, &color, 2, ot);
        return ++p->frame * p->param0 >= p->param1;
    }
    p->delay--;
    return 0;
}
