/**
 * @file player.c
 * @brief The player: Pepsiman's frame step (running, steering, jumping,
 *        falling and landing), his reactions to hits and the actions they
 *        start, the rides and the held object, his shadow, the stage's
 *        lights and fog, and the play state's clock, miss and retry.
 *
 * UpdatePlayer runs once a stage frame. It checks for a fall off the
 * course, applies gravity, hangs the ride and the held object from the
 * player's model, steps the speed, jump and slide of whatever the player
 * rides (on foot and the roller and cover here, the boards in their
 * stages' overlays), moves the player along the course heading and steers
 * him within 30 degrees of it. ReactToHits turns the hit records into a
 * PlayerAction from gHitReactions, and UpdatePlayerAction plays the action
 * out: its motions, the knockdown and throw arcs that LandPlayer ends, and
 * getting up again while the bonus level allows it, or a miss.
 * UpdatePlayState runs the clock, the HUD and the miss and retry sequence.
 * Around these sit the drawing helpers (the shadow blob, the ride models,
 * the flat lights and fog), the dust puffs, and a debug grid test screen.
 */

#include "common.h"
#include "sbss.h"
#include "libetc.h"
#include "libgte.h"
#include "libgpu.h"
#include "libgs.h"
#include "libcd.h"
#include "draw.h"
#include "player.h"
#include "object.h"
#include "model.h"
#include "rand.h"


/* MATCHING: retail reaches these through a split lui/%lo pair, so each is
 * an array of unknown size here. */

extern s32 gFloorFlags; /**< the floor's attribute bits, after gFloorY; 0x40 is a drop the player falls off */
extern s16 sSteerDrift; /**< SteerRoller's drift: +-4 a step while steering, back by 2, within +-10 */

/* MATCHING: cc1 splits this load (its lui sits in a branch delay slot, away
 * from the lw), so it is an array here, though both halves use one register. */

/** @brief A point of a 21-by-16 grid spanning the 320x240 screen, 16
 *         pixels apart, stored row by row. */
typedef struct {
    s32 x; /**< column offset from the screen centre */
    s32 y; /**< row offset from the screen centre */
    s32 z; /**< always 0 */
} GridPoint;

extern s32 sGridAngle;          /**< UpdateGridTest's angle, moved by L1 and R1 */
extern GridPoint sGridPoints[]; /**< UpdateGridTest's 21 x 16 grid */
extern DVECTOR sGridScreen[];   /**< each grid point projected to the screen */
extern s32 D_800DED50[]; /**< the depth word of grid point 158 (row 7, column 11, by the screen centre), inside sGridPoints */
extern s32 gGridLastValue; /**< the grid test's last table value */
extern s32 gGridLastX;     /**< the grid test's first point on screen, x */
extern s32 gGridLastY;     /**< the grid test's first point on screen, y */

/* MATCHING: a per-unit view; stage stores it as an s32. */
extern u8 *gLightColors; /**< the stage's light colours, four bytes each */

/** @brief The three flat lights of the scene: one overhead-front, two behind to the sides. */
typedef struct {
    GsF_LIGHT l[3]; /**< one per light index */
} LightTable;

/** @brief The scene's three flat lights: gFlatLights seen as a LightTable. */
/* MATCHING: a struct lvalue keeps the table's base in a register. */
#define sLights (*(LightTable *)gFlatLights)

/** @brief A 72-byte record of the shared slot table, as the item
 *         spawner fills it; collision's Slot48 is the same record. */
typedef struct {
    s32 x; /**< set to minus the view offset's x when a stage object is placed */
    s32 y; /**< cleared, or 0x3C or 0x32, when a stage object is placed */
    s32 z; /**< set to minus the view offset's z when a stage object is placed */
    u8 padC[0x18 - 0xC];
    s16 rotX; /**< -0x88, or turned with the speed */
    s16 rotY; /**< 0xAA when a stage object is placed */
    s16 rotZ; /**< -0x88 for one stage object */
    u8 pad1E[0x24 - 0x1E];
    s16 unk24; /**< cleared on a spawn */
    s16 unk26; /**< cleared on a spawn */
    u8 pad28[0x34 - 0x28];
    s16 switchNo; /**< 0xBA on a spawn */
    s16 model;    /**< its model slot; -1 when free */
    u8 pad38[0x42 - 0x38];
    u8 type; /**< 0xFF on a spawn */
    u8 act;  /**< 0x69 on a spawn */
    u8 pad44[0x48 - 0x44];
} SlotRec;

/** @brief A 0x78-byte model slot, paired with the SlotRec of the same
 *         index; main's ModelSlot is the same record. */
typedef struct {
    u8 pad0[0x58];
    s32 parent; /**< its coordinate system's super: the one the model hangs from */
    u8 pad5C[0x78 - 0x5C];
} ObjRec;

extern SlotRec gModeObjects[]; /**< the mode objects' records, one per gModeObjectModels slot */
/* MATCHING: main declares this as its ModelSlot; this unit's view names the word at 0x58. */
extern ObjRec gModeObjectModels[]; /**< the mode objects' model slots: the rides and the held object */
/**
 * @brief Places and sorts a model for a record (defined in object): here a
 *        ride or the held object, at the player.
 * @param rec the model slot to draw
 * @param r   the record giving its position and angles
 */
/* MATCHING: object types these ObjectModel and Trap; this unit's records are views of them. */
void DrawObjectModel(ObjectModel *rec, Trap *r);

/* MATCHING: retail reaches these through a split lui/%lo pair. */
extern u8 D_800D3358[]; /**< [0] is 1 once a lost ride is placed as a stage object; its flung piece sets -1 when gone */

void ApplySteer(u8 mode, u16 a, s16 b);
void SteerPlayer(void);
s32 CheckFall(void);
s32 ApplyGravity(void);
s32 UpdateRunSpeed(void);
s32 UpdateSlideDash(void);
void UpdateJump(void);
s32 AttachModeObject(void);
void SpawnModeObject(void);
void SteerRoller(void);
void UpdateRollerSpeed(void);
void UpdateCoverSpeed(void);
void SteerCover(void);
s32 UpdateSpeedBand(void);
void PolarOffset(s32 deg, s32 radius, VECTOR *out);
void AttachHeldItem(void);
s32 TrySlide(void);
/* MATCHING: editor stores it as u8; this unit's tests load it lb. */
extern s8 gToolExited; /**< 1 after leaving the hit tool: no fall check, and hits are ignored */

/** @brief How the player reacts to one kind of hit. b[0] to b[4] are the
 *         PlayerAction it starts while running, jumping, sliding, dashing
 *         and in any other action (kinds 15 to 18 instead pick b[2], b[1]
 *         or b[0] by the hit group: head, body, legs); b[5] 1 gives 150
 *         frames of invulnerability, else 18; b[6] 1 zeroes the bonus
 *         level; b[7] 1 takes one bonus level. */
typedef struct {
    u8 b[8]; /**< the columns above */
} HitReaction;

/* The disc file table's thirteenth and fourteenth copies (disc_files.inc),
 * which no code here reads; they sit between draw_tmd's copy and
 * gHitReactions. */
#define DISC_FILE(x) sFiles13##x /**< names the 13th copy's array x */
#include "disc_files.inc"
#define DISC_FILE(x) sFiles14##x /**< names the 14th copy's array x */
#include "disc_files.inc"

/** The HitReaction of each kind of hit, by PlayerHit.kind; 0 starts no
 *  action. */
HitReaction gHitReactions[] = {
    /*  0 */ {{0, 0, 0, 0, 0, 0, 0, 0}},
    /*  1 */ {{ACT_TRIP_START, 0, 0, 0, 0, 0, 0, 0}},
    /*  2 */ {{ACT_KNOCKDOWN_START, ACT_KNOCKDOWN_START, ACT_THROWN_BACK_START, ACT_KNOCKDOWN_BOUNCE_START, 0, 1, 0, 1}},
    /*  3 */ {{ACT_KNOCKDOWN_BOUNCE_START, ACT_KNOCKDOWN_BOUNCE_START, ACT_THROWN_BACK_START, ACT_KNOCKDOWN_BOUNCE_START, 0, 1, 0, 1}},
    /*  4 */
    {{ACT_KNOCKDOWN_BOUNCE_START, ACT_KNOCKDOWN_BOUNCE_START, ACT_THROWN_BACK_START,
      ACT_KNOCKDOWN_BOUNCE_START, ACT_KNOCKDOWN_BOUNCE_START, 0, 0, 1}},
    /*  5 */ {{ACT_THROWN_BACK_START, ACT_THROWN_BACK_START, 0, 0, 0, 1, 0, 1}},
    /*  6 */ {{ACT_THROWN_BACK_START, ACT_THROWN_BACK_START, ACT_THROWN_BACK_START, ACT_THROWN_BACK_BOUNCE_START, 0, 1, 0, 1}},
    /*  7 */ {{ACT_REEL_START, ACT_REEL_START, 0, 0, 0, 1, 0, 1}},
    /*  8 */ {{ACT_REEL_START, ACT_REEL_START, ACT_REEL_START, ACT_THROWN_BACK_BOUNCE_START, 0, 1, 0, 1}},
    /*  9 */ {{ACT_LAUNCH_START, ACT_LAUNCH_START, 0, ACT_LAUNCH_START, 0, 1, 0, 1}},
    /* 10 */ {{ACT_SPIN_START, ACT_SPIN_START, ACT_SPIN_START, ACT_SPIN_START, 0, 1, 0, 1}},
    /* 11 */ {{ACT_TWIRL_START, ACT_TWIRL_START, ACT_THROWN_BACK_START, ACT_TWIRL_START, 0, 1, 0, 1}},
    /* 12 */ {{ACT_BLOWN_AWAY_START, ACT_BLOWN_AWAY_START, ACT_BLOWN_AWAY_START, ACT_BLOWN_AWAY_START, ACT_BLOWN_AWAY_START, 1, 1, 0}},
    /* 13 */ {{ACT_FLATTENED_START, ACT_FLATTENED_START, ACT_FLATTENED_START, ACT_FLATTENED_START, ACT_FLATTENED_START, 1, 1, 0}},
    /* 14 */ {{ACT_SCREEN_HIT_START, ACT_SCREEN_HIT_START, ACT_SCREEN_HIT_START, ACT_SCREEN_HIT_START, ACT_SCREEN_HIT_START, 1, 1, 0}},
    /* 15 */ {{ACT_KNOCKDOWN_BOUNCE_START, ACT_FALL_FLAT_START, ACT_HEAD_HIT_START, 0, 0, 1, 0, 1}},
    /* 16 */ {{ACT_KNOCKDOWN_BOUNCE_START, ACT_REEL_START, ACT_HEAD_HIT_START, 0, 0, 1, 0, 1}},
    /* 17 */ {{ACT_KNOCKDOWN_BOUNCE_START, ACT_THROWN_BACK_START, ACT_HEAD_HIT_START, 0, 0, 1, 0, 1}},
    /* 18 */ {{ACT_SPIN_START, ACT_REEL_START, ACT_HEAD_HIT_START, 0, 0, 1, 0, 1}},
    /* 19 */ {{ACT_KNOCKDOWN_START, ACT_KNOCKDOWN_START, 0, 0, 0, 1, 0, 1}},
    /* 20 */ {{ACT_KNOCKDOWN_BOUNCE_START, ACT_KNOCKDOWN_BOUNCE_START, 0, 0, 0, 1, 0, 1}},
    /* 21 */ {{ACT_DASH_START, ACT_DASH_START, ACT_DASH_START, ACT_DASH_START, ACT_DASH_START, 1, 0, 0}},
    /* 22 */ {{ACT_KNOCKDOWN_START, ACT_KNOCKDOWN_START, ACT_KNOCKDOWN_START, ACT_KNOCKDOWN_START, ACT_KNOCKDOWN_START, 1, 0, 1}},
};

/* Overlay entry points: the stage modes' own code. */
/** @brief The board's speed step (STAGE_1_2's overlay): rises toward 80
 *         on its own, faster with up held; down held brakes it to 40. */
void func_800F8A58(void);
/** @brief The board's jump (STAGE_1_2's overlay): the on-foot jump with
 *         stronger power and the ride's jump motions. */
void func_800F8B60(void);
/** @brief The second board's speed step (STAGE_5_2's overlay): builds up
 *         to 100 and holds it there. */
void func_800F6FA0(void);
/** @brief The second board's jump (STAGE_5_2's overlay). */
void func_800F70A8(void);
/** @brief The board's steering (STAGE_1_2's overlay): steers, leans and
 *         drifts the player sideways. */
void func_800F8D64(void);
/** @brief The second board's steering (STAGE_5_2's overlay). */
void func_800F729C(void);
void RecentreSteer(void);
void SetStageFog(void);
void UpdateBrake(void);

/**
 * @brief Starts a rumble pattern (defined in main): its step's wait,
 *        duration, motor strengths, repeats and next step.
 * @param table the pattern table, gRumblePatterns
 * @param index the step to start, a RUMBLE_ value
 */
/* MATCHING: a per-unit view; main takes a RumbleStep * and a u16. */
void StartRumble(u16 *table, s32 index);
/**
 * @brief Starts an effect (defined in stage): takes a free effect slot and
 *        sets its kind, position and start delay.
 * @param kind  the effect, an EFFECT_ value
 * @param x     where it is, x
 * @param y     where it is, y
 * @param z     where it is, z
 * @param delay the frames before it starts
 * @return 0, or -1 when no effect slot is free
 */
/* MATCHING: stage defines x..n as s16; this unit's calls pass them
 * unextended, so its prototype takes s32. */
s32 SpawnEffect(s32 kind, s32 x, s32 y, s32 z, s32 delay);
extern s32 gGamePad;
extern u16 sViewSpeed;     /**< follows speed by 1 a frame */
extern s16 gSteerYawHalf;  /**< moved by half of gSteerYaw's steering steps */
extern s32 sJumpPeakY;     /**< the lowest y (highest point) since the jump began */
extern s32 sJumpDistance;  /**< the distance from sJumpStart to sJumpLanding; 0 until landing */
extern s32 sJumpStart[];   /**< where the jump began, x y z */
extern s32 sJumpLanding[]; /**< where the jump landed, x y z */

/** @brief Three words moved as one, for a block copy. */
typedef struct {
    s32 w[3]; /**< the words */
} Words3;

extern s32 sSavedPos[]; /**< the player's position, saved and held through the miss sequence */
/* MATCHING: a per-unit view; object declares it the same way, editor
 * as words. */
extern SwitchBox gSwitches[]; /**< the stage's 200 switches; a time-up clears them, and 196 or 198 on holds off its action */
extern u8 D_800959B0; /**< cleared when time runs out; no code reads it */
extern s16 gMissTimer;
/** @brief Draws the running HUD: the cans, the bonus level's figure, the
 *         time left, the course progress and the lives (defined in stage). */
void DrawHud(void);
/** @brief Restarts the stage after a miss, with the player back at the
 *         course start and a full bonus level (defined in stage_flow). */
/* MATCHING: stage_flow defines this with a u8 parameter; this unit's call passes none. */
void RestartStage();
void StepQuitEffect(s32 level);
/* MATCHING: stage_flow stores this as u8; this unit's test loads it lb. */
extern s8 gSkipLandRumble; /**< 1 after a start or restart: the next landing does not rumble */
s32 DistanceSquared(VECTOR *a, VECTOR *b);
/**
 * @brief Empty; called with the player when a jump starts (defined in model).
 * @param g the player's state
 */
/* MATCHING: model defines this as an empty void(void); this unit's call
 * passes the state block in $a0. */
void OnJumpStart(PlayerState *g);

/** @brief Draws the player's shadow while PlayerState.showShadow is set: a
 *         subtractive blob on the floor under the drawn position, smaller
 *         and fainter the higher the player is above the ground. */
void DrawPlayerShadow(void) {
    GsCOORDINATE2 coord;
    MATRIX ls;
    SVECTOR size;
    s32 unused[2];
    CVECTOR color;
    s32 d;

    if (gPlayer.showShadow != 0) {
        GsInitCoordinate2(WORLD, &coord);
        coord.coord.t[0] = gPlayer.x + gPlayer.drawX + gWorldShift[0];
        coord.coord.t[1] = gFloorY[0];
        coord.coord.t[2] = gPlayer.z + gPlayer.drawZ + gWorldShift[2];
        GsGetLs(&coord, &ls);
        GsSetLsMatrix(&ls);
        d = (gPlayer.groundY - gPlayer.y) / 10;
        color.r = BLEND_SUB;
        size.vx = 100 - d;
        size.vy = size.vx / 2;
        color.g = color.b = color.cd = 0x40 - d;
        DrawBillboard(SPR_BLOB, &size, &color, 2, &gSubOt[gActiveBuff]);
    }
}

/** @brief Sets up the grid test screen: the 21 x 16 grid of points 16
 *         pixels apart about the screen centre, viewed from 1000 units in
 *         front of it. */
void InitGridTest(void) {
    for (gLoopJ = 0; gLoopJ < 16; gLoopJ++) {
        for (gLoopI = 0; gLoopI < 21; gLoopI++) {
            sGridPoints[gLoopJ * 21 + gLoopI].x = gLoopI * 16 - 160;
            sGridPoints[gLoopJ * 21 + gLoopI].y = gLoopJ * 16 - 120;
            sGridPoints[gLoopJ * 21 + gLoopI].z = 0;
        }
    }
    gRefView[0] = 0;
    sGridAngle = 0;
    gRefView[1] = 0;
    gRefView[2] = 1000;
    gRefView[3] = 0;
    gRefView[4] = 0;
    gRefView[5] = 0;
}

/** @brief The grid test screen's frame: draws each grid point as a dot and
 *         the grid as a checkerboard of flat quads. The d-pad moves the
 *         viewpoint, L1 and R1 turn sGridAngle (which moves nothing),
 *         triangle zeroes it, and Start leaves for GM_OVERLAY0_LOAD. The
 *         angle, the first point's screen position and one point's depth
 *         are left in the gGridLast globals. */
/* MATCHING: the parity mask through y, the first loop's local, keeps 0xFF out of the preheader. */
void UpdateGridTest(void) {
    DVECTOR sxy;
    SVECTOR v;
    MATRIX ls;
    GsCOORDINATE2 coord;
    GsLINE line;
    POLY_F4 poly;
    long p;
    long flag;
    s32 x;
    s32 y;
    POLY_F4 *pp;

    if (gPadTrig & PADRup) {
        sGridAngle = 0;
    }
    if (gPad & PADL1) {
        sGridAngle += 2;
    }
    if (gPad & PADR1) {
        sGridAngle -= 2;
    }
    if (gPadTrig & PADstart) {
        gGameMode = GM_OVERLAY0_LOAD;
    }
    if (gPad & PADLleft) {
        gRefView[0]++;
    }
    if (gPad & PADLright) {
        gRefView[0]--;
    }
    if (gPad & PADLup) {
        gRefView[1]++;
    }
    if (gPad & PADLdown) {
        gRefView[1]--;
    }
    line.attribute = 0;
    line.b = line.g = line.r = 0xFF;
    for (gLoopJ = 0; gLoopJ < 16; gLoopJ++) {
        for (gLoopI = 0; gLoopI < 21; gLoopI++) {
            x = sGridPoints[gLoopJ * 21 + gLoopI].x;
            y = sGridPoints[gLoopJ * 21 + gLoopI].y;
            GsInitCoordinate2(WORLD, &coord);
            coord.coord.t[2] = 0;
            coord.flg = 0;
            coord.coord.t[0] = x;
            coord.coord.t[1] = y;
            GsGetLs(&coord, &ls);
            GsSetLsMatrix(&ls);
            v.vx = v.vy = v.vz = 0;
            sGridPoints[gLoopJ * 21 + gLoopI].z = RotTransPers(&v, (long *)&sxy, &p, &flag);
            sGridScreen[gLoopJ * 21 + gLoopI].vx = line.x0 = line.x1 = sxy.vx;
            sGridScreen[gLoopJ * 21 + gLoopI].vy = line.y0 = line.y1 = sxy.vy;
            GsSortLine(&line, &gOt[gActiveBuff], 0);
        }
    }
    pp = &poly;
    for (gLoopJ = 0; gLoopJ < 15; gLoopJ++) {
        for (gLoopI = 0; gLoopI < 20; gLoopI++) {
            SetPolyF4(pp);
            pp->x0 = sGridScreen[gLoopJ * 21 + gLoopI].vx;
            pp->y0 = sGridScreen[gLoopJ * 21 + gLoopI].vy;
            pp->x1 = sGridScreen[gLoopJ * 21 + (gLoopI + 1)].vx;
            pp->y1 = sGridScreen[gLoopJ * 21 + (gLoopI + 1)].vy;
            pp->x3 = sGridScreen[(gLoopJ + 1) * 21 + (gLoopI + 1)].vx;
            pp->y3 = sGridScreen[(gLoopJ + 1) * 21 + (gLoopI + 1)].vy;
            pp->x2 = sGridScreen[(gLoopJ + 1) * 21 + gLoopI].vx;
            pp->y2 = sGridScreen[(gLoopJ + 1) * 21 + gLoopI].vy;
            y = 1;
            if (gLoopJ & y) {
                if (gLoopI & y) {
                    pp->r0 = 0x70;
                    pp->g0 = 0x70;
                    pp->b0 = 0x80;
                } else {
                    pp->r0 = 0xFF;
                    pp->g0 = 0xFF;
                    pp->b0 = 0xFF;
                }
            } else if (gLoopI & 1) {
                pp->r0 = 0xFF;
                pp->g0 = 0xFF;
                pp->b0 = 0xFF;
            } else {
                pp->r0 = 0x70;
                pp->g0 = 0x70;
                pp->b0 = 0x80;
            }
            GsSortPoly(pp, &gOt[gActiveBuff], sGridPoints[gLoopJ * 21 + gLoopI].z >> 6);
        }
    }
    gGridLastAngle = sGridAngle;
    gGridLastValue = D_800DED50[0];
    gGridLastX = sGridScreen[0].vx;
    gGridLastY = sGridScreen[0].vy;
}

/** @brief Empty and never called. */
void UnusedHook1(void) {}

/** @brief Empty and never called. */
void UnusedHook2(void) {}

/**
 * @brief Puts a dust puff (EFFECT_DUST) at a random spot within 80 units of
 *        a position in x and z, starting next frame: the dust kicked up at
 *        the player's feet.
 * @param pos where the dust goes, each coordinate taken as 16 bits
 */
void ScatterDust(VECTOR *pos) {
    SpawnEffect(EFFECT_DUST, (s16)pos->vx + (rand() % 160 - 80), (s16)pos->vy,
                (s16)pos->vz + (rand() % 160 - 80), 1);
}

/**
 * @brief ScatterDust with dark, subtractive dust (EFFECT_DARK_DUST); no code
 *        calls it.
 * @param pos where the dust goes, each coordinate taken as 16 bits
 */
void ScatterDarkDust(VECTOR *pos) {
    SpawnEffect(EFFECT_DARK_DUST, (s16)pos->vx + (rand() % 160 - 80), (s16)pos->vy,
                (s16)pos->vz + (rand() % 160 - 80), 1);
}

/** @brief Empty and never called. */
void UnusedHook3(void) {}

/* MATCHING: the unused 88 bytes put p and flag at sp+0xE8 and the frame at 0x100. */
void ProjectToScreen(VECTOR *pos, SVECTOR *out) {
    DVECTOR sxy;
    SVECTOR v;
    MATRIX ls;
    GsCOORDINATE2 coord;
    s32 unused[22];
    long p;
    long flag;

    GsInitCoordinate2(WORLD, &coord);
    coord.coord.t[0] = pos->vx;
    coord.coord.t[1] = pos->vy;
    coord.coord.t[2] = pos->vz;
    coord.flg = 0;
    GsGetLs(&coord, &ls);
    GsSetLsMatrix(&ls);
    v.vz = 0;
    v.vy = 0;
    v.vx = 0;
    out->vz = RotTransPers(&v, (long *)&sxy, &p, &flag);
    out->vx = sxy.vx;
    out->vy = sxy.vy;
}

/**
 * @brief Finds where a point given in a model part's frame is in the world,
 *        and leaves that part's local-to-screen matrix set for drawing.
 * @param coord the part's coordinate system
 * @param pos   the point, in the part's frame
 * @param out   the point in world coordinates
 */
/* MATCHING: the unused pair puts flag at sp+0x70 and the frame at 0x88. */
void TransformByCoord(GsCOORDINATE2 *coord, SVECTOR *pos, VECTOR *out) {
    MATRIX world;
    MATRIX local;
    SVECTOR v;
    VECTOR t;
    s32 unused[2];
    long flag;

    v.vx = pos->vx;
    v.vy = pos->vy;
    v.vz = pos->vz;
    GsGetLws(coord, &local, &world);
    GsSetLsMatrix(&local);
    RotTrans(&v, &t, &flag);
    out->vx = t.vx;
    out->vy = t.vy;
    out->vz = t.vz;
    GsSetLsMatrix(&world);
}

/**
 * @brief The player's frame step, run once a stage frame: falling, gravity,
 *        speed, jumps and slides, movement and steering for the ride.
 *
 * In order: the fall check (skipped after leaving the hit tool), the speed
 * band, gravity, the ride (while the pad steers) and the held object hung
 * from the player's model, and a lost ride left behind; then the ride's
 * speed step (on foot also the brake, dash, jump and slide), the stage's
 * speed settings (gStageParams bits 22 and 20: a set speed for the next
 * step, or a 30-unit push back or forward along the course on the ground),
 * the move along the course heading (the roller along its own), the ride's
 * steering, the heading held within 30 degrees of the course, and the
 * body's position and rotation. gWorldShift then puts the player at the
 * origin.
 */
/* MATCHING: g keeps the gPlayer base in a register for the clamp's store at the join;
 * cases 1 and 2 of the second switch are separate arms, merged by cross-jumping. */
void UpdatePlayer(void) {
    s32 t;
    s32 x;
    s32 v;
    s32 hi;
    PlayerState *g;

    if (gToolExited != 1) {
        CheckFall();
    }
    UpdateSpeedBand();
    ApplyGravity();
    if (gPlayer.control == 1) {
        AttachModeObject();
    }
    AttachHeldItem();
    SpawnModeObject();
    switch (gPlayer.rideMode & RIDE_MASK) {
        case RIDE_NONE:
            UpdateRunSpeed();
            UpdateSlideDash();
            UpdateJump();
            TrySlide();
            break;
        case RIDE_BOARD:
            func_800F8A58();
            func_800F8B60();
            break;
        case RIDE_BOARD_2:
            func_800F6FA0();
            func_800F70A8();
            break;
        case RIDE_ROLLER:
            UpdateRollerSpeed();
            break;
        case RIDE_COVER:
            UpdateCoverSpeed();
            break;
    }
    gPlayer.unk344 = (40 - gPlayer.speed) / 3 + 1;
    t = gPlayer.speed;
    switch (((u32)gStageParams >> 22) & 3) {
        case 1:
            if (t != 0) {
                t = 70;
            }
            break;
        case 2:
            if (t != 0) {
                t = 90;
            }
            break;
        case 3:
            if (gPlayer.y >= gPlayer.groundY) {
                PolarOffset(gCourse.rot.vy * 360 / 4096 - 180, 30, (VECTOR *)gOffsetVec);
                gPlayer.x += gOffsetVec[0];
                gPlayer.z += gOffsetVec[2];
            }
            break;
    }
    switch (((u32)gStageParams >> 20) & 3) {
        case 1:
            if (gPlayer.speed != 0) {
                t = 30;
            }
            break;
        case 2:
            if (gPlayer.speed != 0) {
                t = 30;
            }
            break;
        case 3:
            if (gPlayer.y >= gPlayer.groundY) {
                PolarOffset(gCourse.rot.vy * 360 / 4096, 30, (VECTOR *)gOffsetVec);
                gPlayer.x += gOffsetVec[0];
                gPlayer.z += gOffsetVec[2];
            }
            break;
    }
    gSpeedFixed = t << 4;
    switch (gPlayer.rideMode & RIDE_MASK) {
        case RIDE_NONE:
        case RIDE_BOARD:
        case RIDE_COVER:
        case RIDE_BOARD_2:
            gPlayer.stepX = rsin(gCourse.rot.vy) * gPlayer.speed >> 12;
            gPlayer.stepZ = rcos(gCourse.rot.vy) * gPlayer.speed >> 12;
            gPlayer.x -= gPlayer.stepX;
            gPlayer.z -= gPlayer.stepZ;
            break;
        case RIDE_ROLLER:
            gPlayer.stepX = rsin(gPlayer.yaw) * gPlayer.speed >> 12;
            gPlayer.stepZ = rcos(gPlayer.yaw) * gPlayer.speed >> 12;
            gPlayer.x -= gPlayer.stepX;
            gPlayer.z -= gPlayer.stepZ;
            break;
    }
    switch (gPlayer.rideMode & RIDE_MASK) {
        case RIDE_NONE:
            SteerPlayer();
            break;
        case RIDE_BOARD:
            func_800F8D64();
            break;
        case RIDE_BOARD_2:
            func_800F729C();
            break;
        case RIDE_ROLLER:
            SteerRoller();
            break;
        case RIDE_COVER:
            SteerCover();
            break;
    }
    g = &gPlayer;
    x = g->yaw;
    v = gCourse.rot.vy - 0x155;
    if (x >= v) {
        hi = gCourse.rot.vy + 0x155;
        if (x <= hi) {
            v = x;
        } else {
            v = hi;
        }
    }
    g->yaw = v;
    gBodyRot[1] = gPlayer.yaw;
    gBodyRot[2] = gPlayer.lean;
    gTodCoords[0].coord.t[0] = gPlayer.drawX;
    gTodCoords[0].coord.t[1] = gPlayer.y + gPlayer.drawY;
    gTodCoords[0].coord.t[2] = gPlayer.drawZ;
    SetCoordRotation((SVECTOR *)gBodyRot, gTodCoords);
    gWorldShift[1] = 0;
    gWorldShift[0] = -gPlayer.x;
    gWorldShift[2] = -gPlayer.z;
}

/** @brief On-foot steering: left or right on a digital pad, or an analog
 *         pad in analog mode, steers and side-steps the player, and nothing
 *         held recentres; without control the heading is the course's. */
void SteerPlayer(void) {
    if (gPlayer.control != 0) {
        switch (gPadType) {
            case PAD_TYPE_ANALOG:
                if (gPad & PADLleft) {
                    ApplySteer(3, 16, 15);
                } else if (gPad & PADLright) {
                    ApplySteer(2, 16, 15);
                } else {
                    RecentreSteer();
                }
                break;
            case PAD_TYPE_DIGITAL:
                if (gPad & PADLleft) {
                    ApplySteer(3, 16, 15);
                } else if (gPad & PADLright) {
                    ApplySteer(2, 16, 15);
                } else {
                    RecentreSteer();
                }
                break;
            default:
                RecentreSteer();
                break;
        }
        if (gPlayer.control != 0) {
            return;
        }
    }
    gPlayer.yaw = gCourse.rot.vy;
}

/**
 * @brief Moves the player some units in a direction measured from the
 *        course heading: 0 degrees is forward, 90 and -90 sideways.
 * @param deg  the direction, in degrees from the course heading
 * @param dist how far
 */
static __inline__ void pushBack(s32 deg, s32 dist) {
    s32 angle;
    SVECTOR *rot;

    rot = &gCourse.rot;
    angle = ANGLE_DEG(deg);
    gPlayer.x -= rsin(rot->vy + angle) * dist >> FIX12_SHIFT;
    gPlayer.z -= rcos(rot->vy + angle) * dist >> FIX12_SHIFT;
}

/**
 * @brief One steering step to one side: turns the heading 2 * a that way
 *        (gSteerYaw held within 22.5 degrees of the course heading and,
 *        except on the roller, the heading kept on that side of it), leans
 *        the body and side-steps the player. Standing on a ride quad, the
 *        player is kept at the ground's height. When mode repeats
 *        PlayerState.lastSteer the step only resets the heading.
 * @param mode 3 steers one way (left on foot), 2 the other
 * @param a    half the turn, in 4096ths of a circle
 * @param b    the side-step distance
 */
/* MATCHING: the (u16) read is the separate lhu retail stores when the heading is in range. */
void ApplySteer(u8 mode, u16 a, s16 b) {
    s16 yaw;
    s32 w;

    if (gPlayer.lastSteer != mode) {
        switch (mode) {
            case 3:
                yaw = gCourse.rot.vy;
                if (yaw - 0x2D < gPlayer.yaw) {
                    gPlayer.yaw = yaw - 0x2D;
                }
                gSteerYaw -= a * 2;
                gSteerYawHalf -= a;
                gPlayer.yaw = gSteerYaw;
                if ((gPlayer.rideMode & RIDE_MASK) != RIDE_ROLLER && yaw < gPlayer.yaw) {
                    gPlayer.yaw = yaw;
                }
                if (gPlayer.lean < 0) {
                    gPlayer.lean += 0x22;
                }
                if (gPlayer.lean < 16) {
                    w = gPlayer.lean;
                } else {
                    w = 15;
                }
                gPlayer.lean = w;
                gPlayer.steerSum -= 50;
                pushBack(-90, b);
                break;
            case 2:
                yaw = gCourse.rot.vy;
                if (gPlayer.yaw < yaw + 0x4000) {
                    gPlayer.yaw = yaw + 0x2D;
                }
                gSteerYaw += a * 2;
                gSteerYawHalf += a;
                gPlayer.yaw = gSteerYaw;
                if ((gPlayer.rideMode & RIDE_MASK) != RIDE_ROLLER && gPlayer.yaw < yaw) {
                    gPlayer.yaw = yaw;
                }
                if (gPlayer.lean > 0) {
                    gPlayer.lean -= 0x22;
                }
                if (gPlayer.lean >= -15) {
                    w = gPlayer.lean;
                } else {
                    w = -15;
                }
                gPlayer.lean = w;
                gPlayer.steerSum += 50;
                pushBack(90, b);
                break;
        }
        gSteerYaw = gSteerYaw < gCourse.rot.vy - 0x100   ? gCourse.rot.vy - 0x100
                    : gCourse.rot.vy + 0x100 < gSteerYaw ? gCourse.rot.vy + 0x100
                                                         : (u16)gSteerYaw;
        if (gPlayer.onRideQuad == 1 && gPlayer.onQuadTop == 1 && gPlayer.airborne == 0) {
            gPlayer.y = gPlayer.groundY;
        }
    } else {
        gPlayer.yaw = gCourse.rot.vy;
    }
}

/** @brief Steers back to centre while no direction is held: three times a
 *         call, eases the lean toward 0 by 1 and the heading toward the
 *         course heading by 56 units (by 11 once within 56), with gSteerYaw
 *         following it. */
/* MATCHING: the inline ternary abs (not an if on d) lets the first +56 test reuse the
 * loaded value when it skips the store. */
void RecentreSteer(void) {
    u16 i;
    s32 yaw;
    s32 d;

    for (i = 0; i < 3; i++) {
        if (gPlayer.lean < 0) {
            gPlayer.lean++;
        }
        if (gPlayer.lean > 0) {
            gPlayer.lean--;
        }
        yaw = gCourse.rot.vy;
        d = gPlayer.yaw - yaw;
        if ((d < 0 ? -d : d) > 56) {
            if (gPlayer.yaw < yaw) {
                gPlayer.yaw += 56;
            }
            if (gPlayer.yaw > yaw) {
                gPlayer.yaw -= 56;
            }
        } else {
            if (gPlayer.yaw < yaw) {
                gPlayer.yaw += 11;
            }
            if (gPlayer.yaw > yaw) {
                gPlayer.yaw -= 11;
            }
        }
        gSteerYaw = gPlayer.yaw;
    }
}

/**
 * @brief Drops the player off the course where the floor is a drop
 *        (gFloorFlags 0x40), unless time is up.
 *
 * A ride is lost (a cover only once 140 below the floor) and control is
 * taken away. Once the player is 100 below the floor the miss starts
 * (MISS_FELL) and the fall speeds up; at 1000 below he plays MOT_FALL_OFF.
 * @return nothing: no value is set
 */
/* MATCHING: non-void with no return keeps the last beqz delay slot a nop. */
s32 CheckFall(void) {
    s32 unused[2];

    if ((gFloorFlags & 0x40) && gPlayer.motion[0] != MOT_TIME_UP) {
        if (gPlayer.rideMode & RIDE_MASK) {
            gPlayer.falling = 1;
            if ((gPlayer.rideMode & RIDE_MASK) != RIDE_COVER) {
                gPlayer.jumpPower = 20;
                gPlayer.rideMode &= RIDE_ITEM_MASK;
            }
            ResetSpeed();
            gPlayer.control = 0;
        }
        gPlayer.groundY = 30000;
        if (gPlayer.y >= gFloorY[0] + 100) {
            if (gMissState == MISS_NONE) {
                gMissState = MISS_FELL;
            }
            gFellOff = 1;
            gPlayer.control = 0;
            gPlayer.fallSpeed += 2;
            ResetSpeed();
            gPlayer.actTimer = 0;
            if (gPlayer.y > gFloorY[0] + 0x8C && (gPlayer.rideMode & RIDE_MASK) == RIDE_COVER) {
                gPlayer.rideMode &= RIDE_ITEM_MASK;
            }
            gPlayer.falling = 1;
            if (gPlayer.y > gFloorY[0] + 1000) {
                gPlayer.motion[0] = MOT_FALL_OFF;
                gPlayer.fallSpeed = 0;
                gPlayer.rideMode &= RIDE_ITEM_MASK;
            }
        }
    }
}

/**
 * @brief Gravity, outside the arcs the jump code flies: snaps the player to
 *        the ground within 50 units, else drops him fallSpeed * 5 a step
 *        until he lands on the ground or on top of a ride quad.
 *
 * It also keeps the jump's highest points (sJumpPeakY, PlayerState.peakY).
 * Jump motions, and ride jumps except on a cover, are left alone. The
 * first landing after a jump records sJumpLanding and sJumpDistance and
 * rumbles, unless gSkipLandRumble is set.
 * @return nothing: no value is set
 */
/* MATCHING: y reloaded, reused for rideQuadY and killed by the dead y = 0 keeps retail's copy. */
s32 ApplyGravity(void) {
    s32 x;
    s32 d;
    s32 y;
    s32 z;

    x = gPlayer.y;
    if (x < sJumpPeakY) {
        sJumpPeakY = x;
    }
    y = gPlayer.y;
    if (y < gPlayer.peakY) {
        gPlayer.peakY = y;
    }
    if ((u32)(gPlayer.motion[0] - MOT_JUMP) < 4) {
        goto end;
    }
    if ((u32)(gPlayer.motion[0] - MOT_RIDE_JUMP) < 2 && (gPlayer.rideMode & RIDE_MASK) != RIDE_COVER) {
        goto end;
    }
    d = gPlayer.groundY - y;
    y = 0;
    if ((d >= 0 ? d : -d) < 50) {
        gPlayer.y = gPlayer.groundY;
    } else {
        gPlayer.y = x + (u16)gPlayer.fallSpeed * 5;
    }
    if (gPlayer.onRideQuad == 1 && gPlayer.onQuadTop == 1) {
        gPlayer.fallSpeed = 0;
        gPlayer.peakY = 0;
        gPlayer.y = gPlayer.rideQuadY;
        goto end;
    }
    d = gPlayer.peakY;
    y = gPlayer.rideQuadY;
    if (d <= y && gPlayer.onRideQuad == 1) {
        if (gPlayer.y >= y) {
            gPlayer.y = y;
            gPlayer.fallSpeed = 0;
            gPlayer.peakY = 0;
            gPlayer.onQuadTop = gPlayer.onRideQuad;
        } else {
            gPlayer.fallSpeed++;
        }
        goto end;
    }
    if (gPlayer.y >= gPlayer.groundY) {
        z = gFloorY[0];
        gPlayer.fallSpeed = 0;
        gPlayer.peakY = 0;
        gPlayer.onQuadTop = 0;
        gPlayer.y = z;
        if (sJumpDistance == 0) {
            sJumpLanding[0] = gPlayer.x;
            sJumpLanding[1] = z;
            sJumpLanding[2] = gPlayer.z;
            sJumpDistance = SquareRoot0(DistanceSquared((VECTOR *)sJumpStart, (VECTOR *)sJumpLanding));
            if (gSkipLandRumble == 0) {
                StartRumble(gRumblePatterns, RUMBLE_JUMP_LAND);
            }
        }
    } else {
        gPlayer.fallSpeed++;
    }
end:;
}

/* MATCHING: the volatile read keeps the later kind reads as reloads, as retail. */
void ReactToHits(PlayerState *state) {
    u8 v;
    s16 i;
    u8 m;
    PlayerHit *p;

    v = 0;
    if (state->hittable == 0) {
        return;
    }
    if (gToolExited == 1) {
        return;
    }
    for (i = 0; i < 3; i++) {
        p = &gPlayerHits[i];
        if (p->hit == 0) {
            continue;
        }
        gHitRecover = 0;
        if ((u32)(*(volatile u8 *)&p->kind - 15) >= 4) {
            m = state->motion[0];
            switch (state->action) {
                case ACT_RUN:
                case ACT_RIDE_STRAIGHT:
                    switch (m) {
                        case MOT_RIDE:
                            if ((gPlayer.rideMode & RIDE_MASK) != RIDE_COVER) {
                                break;
                            }
                        case MOT_RUN_SLOW:
                        case MOT_RUN:
                        case MOT_RUN_FAST:
                            v = gHitReactions[gPlayerHits[i].kind].b[0];
                            break;
                        case MOT_JUMP:
                        case MOT_JUMP_HIGH:
                        case MOT_JUMP_MID:
                        case MOT_JUMP_LOW:
                            v = gHitReactions[gPlayerHits[i].kind].b[1];
                            break;
                        case MOT_SLIDE_START:
                        case MOT_SLIDE:
                        case MOT_SLIDE_END:
                            v = gHitReactions[gPlayerHits[i].kind].b[2];
                            break;
                    }
                    break;
                case ACT_DASH_START:
                case ACT_DASH:
                    v = gHitReactions[p->kind].b[3];
                    break;
                case ACT_BRAKE_START:
                case ACT_BRAKE:
                    v = gHitReactions[p->kind].b[0];
                    break;
                default:
                    switch (gPlayer.rideMode & RIDE_MASK) {
                        case RIDE_BOARD:
                        case RIDE_ROLLER:
                        case RIDE_BOARD_2:
                            v = ACT_THROWN_BACK_START;
                            break;
                        case RIDE_COVER:
                        default:
                            v = gHitReactions[gPlayerHits[i].kind].b[4];
                            break;
                    }
                    break;
            }
            switch (gPlayer.rideMode & RIDE_MASK) {
                case RIDE_BOARD:
                case RIDE_ROLLER:
                case RIDE_BOARD_2:
                    v = ACT_THROWN_BACK_START;
                    break;
            }
        } else {
            v = gHitReactions[p->kind].b[2 - i];
            if (gPlayer.rideMode & RIDE_MASK) {
                v = ACT_THROWN_BACK_START;
            }
        }
        switch (gPlayer.rideMode & RIDE_MASK) {
            case RIDE_BOARD:
            case RIDE_ROLLER:
            case RIDE_BOARD_2:
                gPlayer.dropRide = 1;
                gPlayer.bonusLocked = 1;
                gBonusLevel = 0;
                gPlayer.rideMode &= RIDE_ITEM_MASK;
                state->hittable = 0;
                gHitRecover = 1;
                state->invulnTimer = 750;
                state->action = v;
                if (gPlayerHits[i].kind != 0) {
                    EndJump();
                }
                sLastHitMask = gPlayerHits[0].hit | gPlayerHits[1].hit | gPlayerHits[2].hit;
                return;
            case RIDE_NONE:
            default:
                if (v == 0) {
                    continue;
                }
                if (gHitReactions[gPlayerHits[i].kind].b[5] == 1) {
                    state->hittable = 0;
                    state->invulnTimer = 150;
                    gHitRecover = 1;
                } else {
                    state->hittable = 0;
                    state->invulnTimer = 18;
                }
                if (gHitReactions[gPlayerHits[i].kind].b[6] == 1) {
                    gPlayer.bonusLocked = 1;
                    gBonusLevel = 0;
                }
                if (gHitReactions[gPlayerHits[i].kind].b[7] == 1 &&
                    (s8)(gPlayerHits[0].hit | gPlayerHits[1].hit | gPlayerHits[2].hit) != sLastHitMask) {
                    AddBonusLevel(-1);
                }
                state->action = v;
                if (gPlayerHits[i].kind != 0) {
                    EndJump();
                }
                sLastHitMask = gPlayerHits[0].hit | gPlayerHits[1].hit | gPlayerHits[2].hit;
                return;
        }
    }
    sLastHitMask = 0;
}

/**
 * @brief Eases between two values (defined in model): step of steps
 *        steps of the way from from to to.
 * @param from  the value at step 0
 * @param to    the value at the last step
 * @param step  how far along
 * @param steps how many steps the whole way takes
 * @return the value at step; to itself once step reaches steps
 */
/* MATCHING: model defines this with s16/u16 parameters and an s16 result; this unit's
 * calls pass halfwords unextended and store the result as a word. */
s32 Lerp(s32 from, s32 to, s32 step, s32 steps);
/* MATCHING: empty void(void) in model; unprototyped here, called with two args and one. */
void OnJumpLand();

s32 LandPlayer(PlayerState *g);
void ResetPlayerAction(void);

/**
 * @brief The player's action and motion step, run once a stage frame.
 *
 * It counts invulnerability down (held while a hit record is still set),
 * steps the animation channels, and starts any pending *_START action: its
 * motion, timers, sound and rumble. Then the playing motion does its frame:
 * footsteps while running, the jump's arc and landing, the knockdown, throw
 * and launch arcs (LandPlayer), the skids, spins, reels and twirls, and at
 * their end either getting up (ResetPlayerAction) while the bonus level is
 * above 0, or a miss (MISS_RETRY).
 * @param state the player's state
 * @return nothing: no value is set
 */
/* MATCHING: s32 with no return keeps the delay slots before the exit nops; d is one pseudo for
 * every step so global-alloc gives it $a1, the step's argument register. */
s32 UpdatePlayerAction(PlayerState *state) {
    u8 m;
    s16 t;
    s32 v;
    s8 r;
    s32 d;

    m = state->motion[0];
    t = --gPlayer.invulnTimer;
    if (t < 0) {
        gPlayer.invulnTimer = 0;
        gPlayer.hittable = 1;
    } else {
        if (gPlayerHits[0].hit | gPlayerHits[1].hit | gPlayerHits[2].hit) {
            if (t == 1) {
                gPlayer.invulnTimer = 2;
            }
        }
        gPlayer.hittable = 0;
    }
    if (state->twoTracks == 0) {
        state->motionResult[0] = StepAnimation(state, 0, state->animFilter);
    } else {
        state->motionResult[0] = StepAnimation(state, 0, 1);
        state->motionResult[1] = StepAnimation(state, 1, 2);
    }
top:
    switch (state->action) {
        case ACT_TRIP_START:
            state->action = ACT_TRIP;
            state->motion[0] = MOT_TRIP;
            state->control = 0;
            StartRumble(gRumblePatterns, RUMBLE_TRIP);
            break;
        case ACT_TRIP_HARD_START:
            state->action = ACT_TRIP_HARD;
            state->motion[0] = MOT_TRIP;
            state->control = 0;
            state->actTimer2 = 0;
            state->actTimer = 0;
            StartRumble(gRumblePatterns, RUMBLE_KNOCKDOWN);
            break;
        case ACT_TOD_FILTER_START:
            state->action = ACT_TOD_FILTER;
            state->animFilter = 0x80;
            break;
        case ACT_LOOK_RESET_START:
            state->action = ACT_LOOK_RESET;
            state->actTimer = 0;
            state->lookX = 0;
            state->lookZ = 0;
            break;
        case ACT_LOOK_RESET:
            if (state->actTimer < 60) {
                state->actTimer++;
            }
            gLookYawNarrow = Lerp(gLookYawNarrow, 0, state->actTimer, 60);
            /* MATCHING: actTimer read through a byte pointer keeps the load below the store to gLookYawNarrow. */
            gLookYawWide = Lerp(gLookYawWide, 0, *(s16 *)((u8 *)state + 0x374), 60);

            if (gLookYawNarrow == 0) {
                state->animFilter = 0;
                state->action = ACT_RUN;
            }
            break;
        case ACT_SPIN_START:
            state->action = ACT_SPIN;
            state->motion[0] = MOT_SPIN;
            state->control = 0;
            state->actTimer2 = 0;
            state->actTimer = 0;
            StartRumble(gRumblePatterns, RUMBLE_SPIN);
            break;
        case ACT_STOP_START:
            if ((u8)(m - MOT_RUN_SLOW) < 3) {
                state->action = ACT_STOP;
                state->motion[0] = MOT_STOP;
                state->actTimer = 0;
                state->control = 0;
                state->startSpeed = state->speed;
            }
            break;
        case ACT_TIME_UP_START:
            state->action = ACT_TIME_UP;
            state->motion[0] = MOT_TIME_UP;
            state->actTimer = 0;
            state->control = 0;
            state->startSpeed = state->speed;
            break;
        case ACT_STOP_WAIT_START:
            state->action = ACT_STOP_WAIT;
            state->control = 0;
            state->actTimer = 0;
            if (state->speed == 0) {
                state->motion[0] = MOT_STOP_WAIT;
            } else {
                state->motion[0] = MOT_STOP;
                state->actTimer = 0;
                state->startSpeed = state->speed;
            }
            break;
        case ACT_STOP_ITEM_START:
            state->action = ACT_STOP_ITEM;
            state->control = 0;
            if (state->speed == 0) {
                state->motionResult[0] = 1;
                state->actTimer = 0;
                state->motion[0] = MOT_STOP_ITEM;
            } else {
                state->motion[0] = MOT_STOP;
                state->actTimer = 0;
                state->startSpeed = state->speed;
            }
            break;
        case ACT_STOP_POSE_START:
            state->action = ACT_STOP_POSE;
            state->control = 0;
            if (state->speed != 0) {
                state->motion[0] = MOT_STOP;
                state->actTimer = 0;
                state->startSpeed = state->speed;
            } else {
                state->motionResult[0] = 1;
                state->motion[0] = MOT_STOP_POSE;
            }
            break;
        case ACT_LAUNCH_START:
            PlaySe(SE_LAUNCH);
            state->action = ACT_LAUNCH;
            state->motion[0] = MOT_LAUNCH;
            state->jumpPower = 70;
            state->actTimer2 = 0;
            state->actTimer = 0;
            state->control = 0;
            break;
        case ACT_DASH_START:
            state->action = ACT_DASH;
            state->motion[0] = MOT_DASH;
            state->actTimer2 = 0;
            state->startSpeed = state->speed;
            StartRumble(gRumblePatterns, RUMBLE_DASH);
        case ACT_DASH:
            if (state->actTimer2 < 5) {
                DrawFrozenFrame();
                gSpeedFixed = 0x5A0;
            } else {
                gSpeedFixed = 0x460;
            }
            state->actTimer2++;
            state->speed = (s16)gSpeedFixed >> 4;
            ScatterDust((VECTOR *)&state->x);
            if (state->actTimer2 >= 30) {
                state->action = ACT_RUN;
                if (gPlayer.y >= gPlayer.groundY && (u32)(gPlayer.motion[0] - MOT_SLIDE_START) >= 3) {
                    state->motion[0] = MOT_RUN_FAST;
                }
            }
            break;
        case ACT_BRAKE_START:
            PlaySe(SE_SLIDE);
            state->action = ACT_BRAKE;
            state->control = 0;
            state->motion[0] = MOT_BRAKE;
            state->actTimer3 = 0;
            state->actTimer2 = 0;
            state->actTimer = 0;
            state->startSpeed = state->speed;
            StartRumble(gRumblePatterns, RUMBLE_BRAKE);
        case ACT_BRAKE:
            gSpeedFixed -= 0x20;
            if (++state->actTimer3 >= 16 || (s16)gSpeedFixed < 0) {
                gSpeedFixed = 0;
                state->action = ACT_BRAKE_RISE;
                state->motion[0] = MOT_BRAKE_RISE;
                state->control = 1;
            }
            state->speed = (s16)gSpeedFixed >> 4;
            ScatterDust((VECTOR *)&state->x);
            break;
        case ACT_BRAKE_RISE:
            if (state->motionResult[0] == 1) {
                state->motion[0] = MOT_RUN_FAST;
                state->action = ACT_RUN;
                state->slideState = SLIDE_NONE;
                gSpeedFixed = 0x320;
            }
            break;
        case ACT_KNOCKDOWN_START:
            state->action = ACT_KNOCKDOWN;
            state->motion[0] = MOT_KNOCKDOWN;
            state->jumpPower = 70;
            state->actTimer2 = 0;
            state->actTimer = 0;
            state->control = 0;
            StartRumble(gRumblePatterns, RUMBLE_KNOCKDOWN);
            break;
        case ACT_KNOCKDOWN_BOUNCE_START:
            state->action = ACT_KNOCKDOWN_BOUNCE;
            state->motion[0] = MOT_KNOCKDOWN;
            state->jumpPower = 70;
            state->actTimer2 = 0;
            state->actTimer = 0;
            state->control = 0;
            StartRumble(gRumblePatterns, RUMBLE_KNOCKDOWN);
            break;
        case ACT_THROWN_BACK_START:
            state->action = ACT_THROWN_BACK;
            state->motion[0] = MOT_THROWN_BACK;
            if (state->speed < 90) {
                state->speed = 70;
            }
            state->control = 0;
            state->jumpPower = state->speed;
            /* MATCHING: *= -1 reloads the halfword with lh. */
            state->speed *= -1;
            StartRumble(gRumblePatterns, RUMBLE_HIT);
            break;
        case ACT_THROWN_BACK_BOUNCE_START:
            state->action = ACT_THROWN_BACK_BOUNCE;
            state->motion[0] = MOT_THROWN_BACK;
            if (state->speed < 90) {
                state->speed = 70;
            }
            state->control = 0;
            state->jumpPower = state->speed;
            state->speed *= -1;
            StartRumble(gRumblePatterns, RUMBLE_HIT);
            break;
        case ACT_REEL_START:
            state->action = ACT_REEL;
            state->motion[0] = MOT_REEL;
            ResetSpeed();
            state->actTimer = 10;
            state->actTimer2 = 0;
            state->control = 0;
            StartRumble(gRumblePatterns, RUMBLE_REEL);
            break;
        case ACT_HEAD_HIT_START:
            state->action = ACT_HEAD_HIT;
            state->motion[0] = MOT_HEAD_HIT;
            state->actTimer2 = 0;
            state->actTimer = 0;
            state->control = 0;
            if (AddBonusLevel(0) <= 0) {
                state->action = ACT_THROWN_BACK_START;
                goto top;
            }
            StartRumble(gRumblePatterns, RUMBLE_HIT);
            ResetSpeed();
            break;
        case ACT_FALL_SKID_START:
            state->action = ACT_FALL_SKID;
            state->motion[0] = MOT_FALL_SKID;
            state->actTimer2 = 0;
            state->actTimer = 0;
            state->control = 0;
            StartRumble(gRumblePatterns, RUMBLE_HIT);
            break;
        case ACT_FALL_FLAT_START:
            state->action = ACT_FALL_FLAT;
            state->motion[0] = MOT_FALL_FLAT;
            ResetSpeed();
            state->control = 0;
            StartRumble(gRumblePatterns, RUMBLE_HIT);
            break;
        case ACT_TWIRL_START:
            state->action = ACT_TWIRL;
            state->motion[0] = MOT_TWIRL;
            ResetSpeed();
            state->actTimer4 = 0;
            state->actTimer3 = 0;
            state->actTimer2 = 0;
            state->actTimer = 0;
            state->control = 0;
            StartRumble(gRumblePatterns, RUMBLE_TWIRL);
            break;
        case ACT_SCREEN_HIT_START:
            if (rand() & 1) {
                state->action = ACT_SCREEN_HIT_TWIRL;
                state->motion[0] = MOT_TWIRL;
            } else {
                state->action = ACT_SCREEN_HIT_FLY;
                state->motion[0] = MOT_THROWN_BACK;
            }
            ResetSpeed();
            state->actTimer4 = 0;
            state->actTimer3 = 0;
            state->actTimer2 = 0;
            state->actTimer = 0;
            state->control = 0;
            state->hittable = 0;
            state->invulnTimer = 150;
            StartRumble(gRumblePatterns, RUMBLE_HIT);
            break;
        case ACT_FLATTENED_START:
            state->action = ACT_FLATTENED;
            state->motion[0] = MOT_FLATTENED;
            state->flattened = 1;
            ResetSpeed();
            state->actTimer4 = 0;
            state->actTimer3 = 0;
            state->actTimer2 = 0;
            state->actTimer = 0;
            state->control = 0;
            StartRumble(gRumblePatterns, RUMBLE_FLATTENED);
            break;
        case ACT_BLOWN_AWAY_START:
            gMissState = MISS_DYING;
            state->action = ACT_BLOWN_AWAY;
            state->motion[0] = MOT_THROWN_BACK;
            ResetSpeed();
            state->jumpPower = 300;
            /* MATCHING: a bare barrier keeps the call's arguments below this store. */
            __asm__("");
            state->actTimer4 = 0;
            state->actTimer3 = 0;
            state->actTimer2 = 0;
            state->actTimer = 0;
            state->control = 0;
            StartRumble(gRumblePatterns, RUMBLE_HIT);
            break;
        case ACT_RIDE_CENTRE_START:
            switch (state->motion[0]) {
                case MOT_RIDE:
                    break;
                case MOT_RIDE_LEAN_B_IN:
                    if (state->motionResult[0] == 0) {
                        state->motion[0] = MOT_RIDE_LEAN_B_OUT;
                    }
                    break;
                case MOT_RIDE_LEAN_A_IN:
                    if (state->motionResult[0] == 0) {
                        state->motion[0] = MOT_RIDE_LEAN_A_OUT;
                    }
                    break;
                case MOT_RIDE_LEAN_A_OUT:
                case MOT_RIDE_LEAN_B_OUT:
                    if (state->motionResult[0] == 0) {
                        state->action = ACT_RIDE_CENTRE;
                        state->motion[0] = MOT_RIDE;
                    }
                    break;
            }
            break;
        case ACT_RIDE_LEAN_A_START:
            switch (state->motion[0]) {
                case MOT_RIDE_LEAN_A_OUT:
                case MOT_RIDE_LEAN_B_IN:
                    break;
                case MOT_RIDE:
                case MOT_RIDE_LEAN_B_OUT:
                    if (state->motionResult[0] == 0) {
                        state->action = ACT_RIDE_LEAN_A;
                        state->motion[0] = MOT_RIDE_LEAN_A_IN;
                    }
                    break;
            }
            break;
        case ACT_RIDE_LEAN_B_START:
            switch (state->motion[0]) {
                case MOT_RIDE_LEAN_A_IN:
                    break;
                case MOT_RIDE:
                case MOT_RIDE_LEAN_A_OUT:
                    if (state->motionResult[0] == 0) {
                        state->action = ACT_RIDE_LEAN_B;
                        state->motion[0] = MOT_RIDE_LEAN_B_IN;
                    }
                    break;
            }
            break;
        case ACT_RIDE_LEAN_A_IN:
            state->motion[0] = MOT_RIDE_LEAN_A_IN;
            break;
        case ACT_RIDE_LEAN_A_OUT:
            state->motion[0] = MOT_RIDE_LEAN_A_OUT;
            break;
        case ACT_ROLL_STRAIGHT:
        case ACT_RIDE_STRAIGHT:
            state->motion[0] = MOT_RIDE;
            break;
    }
    switch (state->motion[0]) {
        case MOT_RUN_SLOW:
            if (gGameMode == GM_STAGE && gLoading == 0 && (gFrameCount & 7) == 0) {
                PlaySe(SE_STEP);
            }
            break;
        case MOT_RUN:
            if (gGameMode == GM_STAGE && gLoading == 0 && gFrameCount % 6 == 0) {
                PlaySe(SE_STEP);
            }
            break;
        case MOT_RUN_FAST:
            if (gGameMode == GM_STAGE && gLoading == 0 && (gFrameCount & 3) == 0) {
                PlaySe(SE_STEP);
            }
            break;
        case MOT_JUMP:
        case MOT_JUMP_HIGH:
        case MOT_JUMP_MID:
        case MOT_JUMP_LOW:
        case MOT_RIDE_JUMP:
        case MOT_RIDE_JUMP_2:
            state->jumpPower -= 5;
            d = state->jumpPower;
            state->y -= d;
            if (state->peakY <= gPlayer.rideQuadY && state->onRideQuad == 1) {
                if (state->y >= gPlayer.rideQuadY) {
                    state->y = gPlayer.rideQuadY;
                    OnJumpLand(&gPlayer, d);
                    /* MATCHING: case 0 shares the default body, which the compare tree reaches first. */
                    switch ((s8)gPlayer.slideState) {
                        case SLIDE_NONE:
                        default:
                            state->motion[0] = MOT_RUN;
                            break;
                        case SLIDE_DASHING:
                            state->motion[0] = MOT_DASH;
                            break;
                    }
                    ScatterDust((VECTOR *)&state->x);
                    state->airborne = 0;
                    state->onQuadTop = 1;
                    StartRumble(gRumblePatterns, RUMBLE_JUMP_LAND);
                }
            } else if (state->y >= state->groundY) {
                state->y = gFloorY[0];
                OnJumpLand(&gPlayer);
                switch ((s8)gPlayer.slideState) {
                    case SLIDE_NONE:
                    default:
                        state->motion[0] = MOT_RUN;
                        break;
                    case SLIDE_DASHING:
                        state->motion[0] = MOT_DASH;
                        break;
                }
                ScatterDust((VECTOR *)&state->x);
                state->airborne = 0;
                state->onQuadTop = 0;
                StartRumble(gRumblePatterns, RUMBLE_JUMP_LAND);
                sJumpLanding[0] = state->x;
                sJumpLanding[1] = state->y;
                sJumpLanding[2] = state->z;
                sJumpDistance =
                    SquareRoot0(DistanceSquared((VECTOR *)sJumpStart, (VECTOR *)sJumpLanding));
            }
            break;
        case MOT_TRIP:
            state->speed -= 2;
            if (state->speed <= 0) {
                ResetSpeed();
            }
            /* MATCHING: cases MOT_SLIDE_END, MOT_GET_UP and MOT_GET_UP_3 branch here; their own calls would be the copies
             * cross-jumping keeps. */
            if (state->motionResult[0] == 0 &&
                (state->action == ACT_TRIP || state->action == ACT_TRIP_HARD)) {
            reset:
                ResetPlayerAction();
            }
            break;
        case MOT_SPIN:
        case MOT_SPIN_2:
        case MOT_SPIN_3:
            state->speed -= 4;
            if (state->speed <= 0) {
                ResetSpeed();
            }
            if (gPlayer.actTimer == 0) {
                PlaySe(SE_SPIN);
            }
            state->control = 0;
            gPlayer.actTimer++;
            gPlayer.bodyYaw += ANGLE_DEG(gPlayer.actTimer / 2);
            if (state->motionResult[0] == 0) {
                gPlayer.actTimer2 = (gPlayer.actTimer2 + 20) % 360;
                if (gPlayer.actTimer2 == 80 || gPlayer.actTimer2 == 260) {
                    state->motion[0] = MOT_SPIN_2;
                }
                if (gPlayer.actTimer2 == 160 || gPlayer.actTimer2 == 340) {
                    state->motion[0] = MOT_SPIN_3;
                }
                v = rsin(ANGLE_DEG(gPlayer.actTimer2));
                /* MATCHING: one expression, so the table's %hi is built before the abs. */
                gPlayer.y = gFloorY[0] - ((v * 100 >> 12) >= 0 ? (v * 100 >> 12) : -(v * 100 >> 12));

                if (gPlayer.actTimer >= 30) {
                    state->bodyYaw = 0;
                    ResetSpeed();
                    if (AddBonusLevel(0) != 0) {
                        ResetPlayerAction();
                    } else {
                        gPlayer.actTimer = 0;
                        state->motion[0] = MOT_TWIRL_FALL;
                    }
                }
            }
            gBodyRot[1] = gPlayer.yaw + gPlayer.bodyYaw;
            gTodCoords[0].coord.t[0] = gPlayer.drawX;
            gTodCoords[0].coord.t[1] = gPlayer.y + gPlayer.drawY;
            gTodCoords[0].coord.t[2] = gPlayer.drawZ;
            SetCoordRotation((SVECTOR *)gBodyRot, gTodCoords);
            break;
        case MOT_STOP:
            if (state->actTimer < 10) {
                state->actTimer++;
            }
            state->speed = Lerp(state->startSpeed, 0, state->actTimer, 10);
            switch (state->action) {
                case ACT_STOP:
                    if (state->motionResult[0] == 0) {
                        state->motion[0] = MOT_STAND;
                    }
                    break;
                case ACT_STOP_WAIT:
                    if (state->motionResult[0] == 0) {
                        state->motion[0] = MOT_STOP_WAIT;
                    }
                    break;
                case ACT_STOP_ITEM:
                    if (state->motionResult[0] == 0) {
                        state->motion[0] = MOT_STOP_ITEM;
                    }
                    break;
                case ACT_STOP_POSE:
                    if (state->motionResult[0] == 0) {
                        state->motion[0] = MOT_STOP_POSE;
                    }
                    break;
            }
            break;
        case MOT_TIME_UP:
            if (state->actTimer < 60) {
                state->actTimer++;
            }
            state->speed = Lerp(state->startSpeed, 0, state->actTimer, 60);
            break;
        case MOT_STAND:
            state->control = 0;
            break;
        case MOT_STOP_WAIT:
            gPlayer.altModel = 1;
            if (state->motionResult[0] == 0) {
                if (++state->actTimer >= 46) {
                    state->action = ACT_RUN;
                    state->motion[0] = MOT_STAND;
                    gPlayer.altModel = 0;
                }
            }
            break;
        case MOT_STOP_ITEM:
            if (++state->actTimer == 13) {
                PlaySe(SE_ITEM_1);
            }
            if (state->actTimer == 20) {
                PlaySe(SE_ITEM_2);
            }
            if (state->actTimer == 40) {
                state->rideMode = (state->rideMode & RIDE_MASK) | (RIDE_ITEM << 4);
            }
            if (state->actTimer == 90) {
                PlaySe(SE_ITEM_TICK);
            }
            if (state->actTimer == 110) {
                PlaySe(SE_ITEM_TICK);
            }
            if (state->actTimer == 130) {
                PlaySe(SE_ITEM_TICK);
            }
            if (state->actTimer == 90) {
                state->twoTracks = 1;
                state->motion[1] = MOT_STAND;
            }
            if (state->motionResult[1] == 0) {
                state->twoTracks = 0;
                state->motion[1] = MOT_OFF;
                state->curMotion[1] = MOT_OFF;
            }
        case MOT_STOP_POSE:
            if (state->motionResult[0] == 0) {
                state->action = ACT_RUN;
                state->motion[0] = MOT_STAND;
                state->rideMode &= RIDE_MASK;
            }
            break;
        case MOT_SLIDE_START:
            state->twoTracks = 0;
            if (state->motionResult[0] == 0) {
                state->motion[0] = MOT_SLIDE;
                if (state->speed >= 0xB0) {
                    state->slideTimer = 1;
                }
                if ((u32)((u16)state->speed - 0x97) < 0x19) {
                    state->slideTimer = 1;
                }
                if ((u32)((u16)state->speed - 0x7E) < 0x19) {
                    state->slideTimer = 1;
                }
                if (state->speed < 0x7E) {
                    state->slideTimer = 1;
                }
            }
            if (gPlayer.y == gPlayer.groundY) {
                ScatterDust((VECTOR *)&state->x);
            }
            StartRumble(gRumblePatterns, RUMBLE_SLIDE);
            break;
        case MOT_SLIDE:
            if (gFrameCount % 3 == 0) {
                ScatterDust((VECTOR *)&state->x);
            }
            if (state->slideTimer == 0) {
                state->motion[0] = MOT_SLIDE_END;
            }
            state->slideTimer--;
            break;
        case MOT_LAUNCH:
            state->speed -= 4;
            if (state->speed <= 0) {
                ResetSpeed();
            }
            if (state->jumpPower > 0) {
                state->jumpPower -= 7;
                d = state->jumpPower;
                state->y -= d;
            }
            r = LandPlayer(state);
            /* MATCHING: three nested tests; one condition folds into a range check. */
            if (r != 0)
                if (r >= 0)
                    if (r < 3) {
                        state->motion[0] = MOT_THROWN_LAND;
                    }
            break;
        case MOT_KNOCKDOWN:
            state->speed -= 2;
            if (state->speed <= 0) {
                ResetSpeed();
            }
            state->jumpPower -= 7;
            d = state->jumpPower;
            state->y -= d;
            r = LandPlayer(state);
            if (r != 0)
                if (r >= 0)
                    if (r < 3) {
                        switch (state->action) {
                            case ACT_KNOCKDOWN_BOUNCE:
                                StartRumble(gRumblePatterns, RUMBLE_BOUNCE);
                                state->motion[0] = MOT_BOUNCE;
                                break;
                            case ACT_KNOCKDOWN:
                                state->motion[0] = MOT_KNOCKDOWN_SKID;
                                break;
                        }
                    }
            break;
        case MOT_KNOCKDOWN_SKID:
            state->speed -= 2;
            if (state->speed >= 0x15) {
                ScatterDust((VECTOR *)&gPlayer.x);
            }
            if (state->speed <= 0) {
                ResetSpeed();
                if (++state->actTimer >= 16) {
                    state->actTimer = 0;
                    state->motion[0] = MOT_KNOCKDOWN_LIE;
                }
            }
            break;
        case MOT_KNOCKDOWN_LIE:
            state->actTimer++;
            if (state->motionResult[0] == 0) {
                if (AddBonusLevel(0) != 0) {
                    state->actTimer2 = 0;
                    state->motion[0] = MOT_GET_UP;
                } else {
                    if (state->actTimer == 30) {
                        PlaySe(SE_MISS);
                    }
                    if (state->actTimer >= 61) {
                        gMissState = MISS_RETRY;
                    }
                }
            }
            break;
        case MOT_DOWN:
            if (state->motionResult[0] == 0) {
                switch (state->action) {
                    case ACT_SPIN:
                    case ACT_LAUNCH:
                    case ACT_KNOCKDOWN:
                    case ACT_THROWN_BACK:
                        state->actTimer2 = 0;
                        state->motion[0] = MOT_GET_UP;
                        break;
                    case ACT_KNOCKDOWN_BOUNCE:
                    case ACT_THROWN_BACK_BOUNCE:
                        state->actTimer2 = 0;
                        state->motion[0] = MOT_GET_UP_2;
                        break;
                }
            }
            break;
        case MOT_BOUNCE:
            if (state->motionResult[0] == 0) {
                state->actTimer++;
                ScatterDust((VECTOR *)&gPlayer.x);
                state->motion[0] = MOT_BOUNCE;
                state->curMotion[0] = MOT_OFF;
            }
            if (state->actTimer == 2) {
                state->actTimer = 0;
                state->motion[0] = MOT_SKID;
            }
            break;
        case MOT_SKID:
            state->speed -= 1;
            if (state->speed >= 6) {
                ScatterDust((VECTOR *)&gPlayer.x);
                StartRumble(gRumblePatterns, RUMBLE_SKID);
            }
            if (state->speed <= 0) {
                ResetSpeed();
                if (++state->actTimer >= 16) {
                    switch (state->action) {
                        default:
                            if (AddBonusLevel(0) != 0) {
                                state->actTimer = 0;
                                state->actTimer2 = 0;
                                state->motion[0] = MOT_GET_UP;
                            } else {
                                if (state->actTimer == 30) {
                                    PlaySe(SE_MISS);
                                }
                                if (state->actTimer >= 61) {
                                    gMissState = MISS_RETRY;
                                }
                            }
                            break;
                        case ACT_HEAD_HIT:
                            if (AddBonusLevel(0) != 0) {
                                state->actTimer2 = 0;
                                state->motion[0] = MOT_STAGGER;
                            } else {
                                if (state->actTimer == 30) {
                                    PlaySe(SE_MISS);
                                }
                                if (state->actTimer >= 61) {
                                    gMissState = MISS_RETRY;
                                }
                            }
                            break;
                        case ACT_TWIRL:
                            if (AddBonusLevel(0) != 0) {
                                state->motion[0] = MOT_GET_UP_2;
                            } else {
                                if (state->actTimer == 30) {
                                    PlaySe(SE_MISS);
                                }
                                if (state->actTimer >= 61) {
                                    gMissState = MISS_RETRY;
                                }
                            }
                            break;
                        case ACT_BLOWN_AWAY:
                            gMissState = MISS_DYING;
                            if (state->actTimer >= 61) {
                                if (gStage == STAGE_2_2 && gSection == 0) {
                                    state->motion[0] = MOT_GET_UP_2;
                                    gMissState = MISS_NONE;
                                } else {
                                    gMissState = MISS_RETRY;
                                }
                            }
                            if ((gStage != STAGE_2_2 || gSection != 0) && state->actTimer == 30) {
                                PlaySe(SE_MISS);
                            }
                            break;
                    }
                }
            }
            break;
        case MOT_THROWN_BACK:
            if (state->action == ACT_SCREEN_HIT_FLY) {
                gPlayer.drawX = Lerp(0, gView[0], state->actTimer3, 5);
                gPlayer.drawY = Lerp(0, -50, state->actTimer3, 5);
                gPlayer.drawZ = Lerp(0, gView[2], state->actTimer3, 5);
                if (++state->actTimer3 >= 5) {
                    StartRumble(gRumblePatterns, RUMBLE_REEL);
                    state->motion[0] = MOT_REEL;
                    state->actTimer4 = 0;
                    state->actTimer3 = 0;
                    state->actTimer2 = 0;
                    state->actTimer = 0;
                }
                gTodCoords[0].coord.t[0] = gPlayer.drawX;
                gTodCoords[0].coord.t[1] = gPlayer.y + gPlayer.drawY;
                gTodCoords[0].coord.t[2] = gPlayer.drawZ;
                SetCoordRotation((SVECTOR *)gBodyRot, gTodCoords);
                break;
            }
            state->speed += 4;
            if (state->speed >= 0) {
                ResetSpeed();
            }
            state->jumpPower -= 7;
            d = state->jumpPower;
            state->y -= d;
            r = LandPlayer(state);
            if (r != 0)
                if (r >= 0)
                    if (r < 3) {
                        state->motion[0] = MOT_THROWN_LAND;
                    }
            break;
        case MOT_THROWN_LAND:
            ScatterDust((VECTOR *)&gPlayer.x);
            if (state->motionResult[0] == 0) {
                switch (state->action) {
                    case ACT_LAUNCH:
                    case ACT_THROWN_BACK:
                    case ACT_BLOWN_AWAY:
                        state->motion[0] = MOT_THROWN_LAND_END;
                        break;
                    case ACT_THROWN_BACK_BOUNCE:
                        StartRumble(gRumblePatterns, RUMBLE_BOUNCE);
                        state->motion[0] = MOT_BOUNCE_BACK;
                        break;
                }
            }
            break;
        case MOT_THROWN_LAND_END:
            if (state->motionResult[0] == 0) {
                state->actTimer2 = 0;
                state->actTimer = 0;
                state->motion[0] = MOT_SKID;
            }
            break;
        case MOT_BOUNCE_BACK:
            if (state->motionResult[0] == 0) {
                state->actTimer++;
                ScatterDust((VECTOR *)&gPlayer.x);
                state->motion[0] = MOT_BOUNCE_BACK;
                state->curMotion[0] = MOT_OFF;
            }
            state->actTimer2++;
            if (state->actTimer >= 2) {
                if (AddBonusLevel(0) > 0) {
                    switch (state->action) {
                        case ACT_THROWN_BACK_BOUNCE:
                            state->actTimer = 0;
                            state->motion[0] = MOT_SKID;
                            break;
                        case ACT_REEL:
                            state->actTimer = 0;
                            state->actTimer2 = 0;
                            state->motion[0] = MOT_GET_UP;
                            state->speed = 0;
                            state->actTimer = 0;
                            break;
                    }
                } else {
                    StartRumble(gRumblePatterns, RUMBLE_HARD_LAND);
                    state->motion[0] = MOT_SKID;
                    state->actTimer = 0;
                    if (state->actTimer2 == 30) {
                        PlaySe(SE_MISS);
                    }
                    if (state->actTimer2 >= 61) {
                        gMissState = MISS_RETRY;
                    }
                }
            }
            break;
        case MOT_REEL:
            switch (state->action) {
                case ACT_SCREEN_HIT_TWIRL:
                    gBodyRot[1] = gPlayer.bodyYaw = gCourse.rot.vy + 0x800;
                case ACT_SCREEN_HIT_FLY:
                    gTodCoords[0].coord.t[0] = gPlayer.drawX;
                    gTodCoords[0].coord.t[1] = gPlayer.y + gPlayer.drawY;
                    gTodCoords[0].coord.t[2] = gPlayer.drawZ;
                    SetCoordRotation((SVECTOR *)gBodyRot, gTodCoords);
                    if (++state->actTimer3 < 10) {
                        /* MATCHING: x - (5 - r) keeps the -5 on the loaded word. */
                        gView[0] = gView[0] - (5 - rand() % 10);
                        gView[1] = gView[1] - (5 - rand() % 10);
                        gView[2] = gView[2] - (5 - rand() % 10);
                        GsSetRefView2((GsRVIEW2 *)gView);
                    }
                    if (state->actTimer3 >= 31) {
                        gPlayer.drawY += 2;
                    }
                    gMissState = MISS_DYING;
                    if (state->actTimer3 >= 121) {
                        gMissState = MISS_RETRY;
                    }
                    break;
                default:
                    state->actTimer--;
                    state->actTimer2++;
                    if (state->actTimer <= 0) {
                        state->actTimer = 1;
                    }
                    if (state->actTimer2 & 1) {
                        gPlayer.bodyRotX = -(state->actTimer * 0x5000 / 360);
                    } else {
                        gPlayer.bodyRotX = state->actTimer * 0x5000 / 360;
                    }
                    gBodyRot[0] = gPlayer.bodyRotX;
                    gTodCoords[0].coord.t[0] = gPlayer.drawX;
                    gTodCoords[0].coord.t[1] = gPlayer.y + gPlayer.drawY;
                    gTodCoords[0].coord.t[2] = gPlayer.drawZ;
                    SetCoordRotation((SVECTOR *)gBodyRot, gTodCoords);
                    if (state->motionResult[0] == 0) {
                        gPlayer.bodyRotX = 0;
                        gBodyRot[0] = 0;
                        state->motion[0] = MOT_REEL_END;
                    }
                    break;
            }
            break;
        case MOT_REEL_END:
            if (state->motionResult[0] == 0) {
                state->actTimer = 0;
                state->actTimer2 = 0;
                state->speed = -5;
                StartRumble(gRumblePatterns, RUMBLE_BOUNCE);
                state->motion[0] = MOT_BOUNCE_BACK;
            }
            break;
        case MOT_FALL_LIE:
            switch (state->action) {
                default:
                    if (state->motionResult[0] == 0 && ++state->actTimer >= 16) {
                        if (AddBonusLevel(0) != 0) {
                            state->motion[0] = MOT_FALL_RISE;
                        } else {
                            if (state->actTimer == 30) {
                                PlaySe(SE_MISS);
                            }
                            if (state->actTimer >= 61) {
                                gMissState = MISS_RETRY;
                            }
                        }
                    }
                    break;
                case ACT_FALL_SKID:
                    if (state->speed >= 0x15) {
                        ScatterDust((VECTOR *)&gPlayer.x);
                    }
                    state->speed -= 2;
                    if (state->speed <= 0) {
                        ResetSpeed();
                    }
                    if (state->motionResult[0] == 0 && state->speed == 0 && ++state->actTimer >= 16) {
                        if (AddBonusLevel(0) != 0) {
                            state->motion[0] = MOT_FALL_RISE;
                        } else {
                            if (state->actTimer == 30) {
                                PlaySe(SE_MISS);
                            }
                            if (state->actTimer >= 61) {
                                gMissState = MISS_RETRY;
                            }
                        }
                    }
                    break;
                case ACT_FALL_FLAT:
                    if (state->motionResult[0] == 0 && ++state->actTimer >= 16) {
                        if (AddBonusLevel(0) != 0) {
                            state->actTimer2 = 0;
                            state->motion[0] = MOT_GET_UP_3;
                            state->actTimer = 0;
                        } else {
                            if (state->actTimer == 30) {
                                PlaySe(SE_MISS);
                            }
                            if (state->actTimer >= 61) {
                                gMissState = MISS_RETRY;
                            }
                        }
                    }
                    break;
            }
            break;
        case MOT_FALL_RISE:
            if (state->motionResult[0] == 0) {
                state->actTimer2 = 0;
                state->motion[0] = MOT_GET_UP;
            }
            break;
        case MOT_HEAD_HIT:
            if (state->motionResult[0] == 0) {
                state->actTimer2 = 0;
                state->motion[0] = MOT_STAGGER;
            }
            break;
        case MOT_STAGGER:
            gPlayer.drawX = rsin(gCourse.rot.vy + 0x400) * 20 >> 12;
            gPlayer.drawZ = rcos(gCourse.rot.vy + 0x400) * 20 >> 12;
            gTodCoords[0].coord.t[2] = gPlayer.drawZ;
            gTodCoords[0].coord.t[0] = gPlayer.drawX;
            gTodCoords[0].coord.t[1] = gPlayer.y + gPlayer.drawY;
            SetCoordRotation((SVECTOR *)gBodyRot, gTodCoords);
            gPlayer.drawZ = 0;
            gPlayer.drawX = 0;
        case MOT_GET_UP_2:
            if (++state->actTimer2 == 1) {
                PlaySe(SE_GET_UP);
            }
        case MOT_SLIDE_END:
            if (state->motionResult[0] == 0) {
                goto reset;
            }
            break;
        case MOT_FALL_SKID:
            state->speed -= 2;
            if (state->speed <= 0) {
                ResetSpeed();
            }
            if (state->motionResult[0] == 0) {
                state->motion[0] = MOT_FALL_LIE;
            }
            break;
        case MOT_FALL_FLAT:
            if (state->motionResult[0] == 0) {
                r = LandPlayer(state);
                if (r != 0)
                    if (r >= 0)
                        if (r < 3) {
                            state->motion[0] = MOT_FALL_LIE;
                        }
            }
            break;
        case MOT_GET_UP:
        case MOT_GET_UP_3:
            if (++state->actTimer2 == 1) {
                PlaySe(SE_GET_UP);
            }
            if (state->motionResult[0] == 0 && ++state->actTimer >= 16) {
                goto reset;
            }

            break;
        case MOT_TWIRL:
            switch (state->action) {
                case ACT_SCREEN_HIT_TWIRL:
                    gPlayer.bodyYaw = state->actTimer2 =
                        Lerp(state->actTimer2, 0x6000, state->actTimer, 30);
                    gPlayer.drawX = Lerp(0, gView[0], state->actTimer3, 15);
                    gPlayer.drawY = Lerp(0, -50, state->actTimer3, 15);
                    gPlayer.drawZ = Lerp(0, gView[2], state->actTimer3, 15);
                    state->actTimer++;
                    if (++state->actTimer3 >= 14) {
                        StartRumble(gRumblePatterns, RUMBLE_REEL);
                        state->motion[0] = MOT_REEL;
                        state->actTimer4 = 0;
                        state->actTimer3 = 0;
                        state->actTimer2 = 0;
                        state->actTimer = 0;
                    }
                    break;
                case ACT_TWIRL:
                    if (++state->actTimer3 >= 41) {
                        state->actTimer3 = 60;
                        state->motion[0] = MOT_TWIRL_FALL;
                        state->actTimer = 0;
                    }
                    gPlayer.bodyYaw = state->actTimer2 =
                        Lerp(state->actTimer2, 0x5800, state->actTimer3, 60);
                    break;
            }
            gBodyRot[1] = gPlayer.bodyYaw;
            gTodCoords[0].coord.t[0] = gPlayer.drawX;
            gTodCoords[0].coord.t[1] = gPlayer.y + gPlayer.drawY;
            gTodCoords[0].coord.t[2] = gPlayer.drawZ;
            SetCoordRotation((SVECTOR *)gBodyRot, gTodCoords);
            break;
        case MOT_TWIRL_FALL:
            if (state->motionResult[0] == 0) {
                StartRumble(gRumblePatterns, RUMBLE_HARD_LAND);
                state->actTimer = 0;
                state->motion[0] = MOT_SKID;
            }
            break;
        case MOT_FLATTENED:
            gMissState = MISS_DYING;
            if (++state->actTimer >= 91) {
                gMissState = MISS_RETRY;
            }
            break;
        case MOT_RIDE:
        case MOT_RIDE_LEAN_A_IN:
        case MOT_RIDE_LEAN_A_OUT:
            if ((u32)((gPlayer.rideMode & RIDE_MASK) - RIDE_ROLLER) < 2 && state->motionResult[0] == 0) {
                state->curMotion[0] = MOT_OFF;
            }
            break;
    }
}

/**
 * @brief On-foot speed, while in control and not jumping: rises by 1 a step
 *        to the cruising 40 and eases back to it from above (up and down
 *        only hold it there); a dash holds 70. At a run loop's end the run
 *        motion follows the speed band.
 * @return nothing: no value is set
 */
/* MATCHING: non-void with no return keeps two bnez delay slots nops. */
s32 UpdateRunSpeed(void) {
    s16 v;

    if (gPlayer.control != 0 && (u32)(gPlayer.motion[0] - MOT_JUMP) >= 4) {
        if (gPlayer.action != ACT_DASH) {
            v = gPlayer.speed;
            if (v < 40) {
                gSpeedFixed += 16;
            } else if (gPad & PADLup) {
                gSpeedFixed += 32;
                if ((s16)gSpeedFixed >> 4 > 40) {
                    gSpeedFixed = 640;
                }
            } else if (gPad & PADLdown) {
                gSpeedFixed -= 32;
                if ((s16)gSpeedFixed >> 4 < 40) {
                    gSpeedFixed = 640;
                }
            } else if (v > 40) {
                gSpeedFixed -= 16;
            } else {
                gSpeedFixed += 16;
            }
            gPlayer.speed = (s16)gSpeedFixed >> 4;
            if (gPlayer.speedBand == 2 && gPlayer.motionResult[0] == 2) {
                gPlayer.motion[0] = MOT_RUN_FAST;
            }
            if (gPlayer.speedBand == 1 && gPlayer.motionResult[0] == 2) {
                gPlayer.motion[0] = MOT_RUN;
            }
            if (gPlayer.speedBand == 0 && gPlayer.motionResult[0] == 2) {
                gPlayer.motion[0] = MOT_RUN_SLOW;
            }
        } else {
            gSpeedFixed = 0x460;
            gPlayer.speed = 0x46;
        }
    }
}

/**
 * @brief On-foot brake and dash: running on the ground, the slide button
 *        (PADRdown) with down held starts a brake, and with up held at
 *        cruising speed (38 to 42) a dash; a dash ends when the action is
 *        back to ACT_RUN.
 * @return nothing: no value is set
 */
/* MATCHING: non-void with no return keeps li 0x71 first in the call block. */
s32 UpdateSlideDash(void) {
    s32 btn;

    if (gPlayer.control != 0) {
        switch ((s8)gPlayer.slideState) {
            case SLIDE_NONE:
                gPlayer.slideState = SLIDE_NONE;
                gPlayer.unk38C = 0;
                if ((u32)(gPlayer.motion[0] - MOT_RUN_SLOW) < 3 && gPlayer.y >= gPlayer.groundY &&
                    (gGamePadTrig & PADRdown)) {
                    btn = gPad;
                    if (btn & PADLdown) {
                        gPlayer.slideState = SLIDE_BRAKING;
                        gPlayer.unk38C = 0;
                        gPlayer.action = ACT_BRAKE_START;
                    }
                    if (btn & PADLup) {
                        if ((u32)((u16)gPlayer.speed - 0x26) < 5) {
                            gPlayer.slideState = SLIDE_DASHING;
                            gPlayer.action = ACT_DASH_START;
                            PlaySe(SE_DASH);
                            gPlayer.unk38C = 0;
                        }
                    }
                }
                break;
            case SLIDE_BRAKING:
                UpdateBrake();
                break;
            case SLIDE_DASHING:
                if (gPlayer.action == ACT_RUN) {
                    gPlayer.slideState = SLIDE_NONE;
                    gPlayer.unk38C = 0;
                }
                break;
        }
    }
}

/** @brief The on-foot jump: the jump button (PADRright) on the ground or a
 *         ride quad's top starts one and records sJumpStart; holding it sets
 *         the jump power each step, and the step it is let go (3, 7 or
 *         later) makes a low, mid or high jump. UpdatePlayerAction flies
 *         the arc. */
/* MATCHING: as UpdateCoverJump for the switch; the volatile read keeps the unk348 and
 * unk350 loads below the stores to sJumpDistance, sJumpStart[1] and sJumpPeakY. */
void UpdateJump(void) {
    s16 n;

    if (gPlayer.control != 0) {
        if ((gGamePadTrig & PADRright) &&
            (gPlayer.y >= gPlayer.groundY || (gPlayer.onRideQuad == 1 && gPlayer.onQuadTop == 1)) &&
            gPlayer.jumpState == JUMP_NONE) {
            sJumpDistance = 0;
            sJumpStart[1] = gPlayer.y;
            sJumpPeakY = gPlayer.y;
            sJumpStart[0] = *(volatile s32 *)&gPlayer.x;
            sJumpStart[2] = gPlayer.z;
            gPlayer.airborne = 1;
            gPlayer.onQuadTop = 0;
            gPlayer.jumpState = JUMP_RISING;
            gPlayer.jumpTime = 0;
            gPlayer.peakY = 0;
            OnJumpStart(&gPlayer);
            gPlayer.motion[0] = MOT_JUMP;
            gPlayer.jumpPower = 0x23;
            gPlayer.jumpStartY = gPlayer.y;
            ScatterDust((VECTOR *)&gPlayer.x);
            PlaySe(SE_JUMP);
        }
        if (gPlayer.jumpState == JUMP_RISING) {
            n = ++gPlayer.jumpTime;
            if (n < 3) {
                gPlayer.jumpPower = 0x19;
            } else if ((u16)(n - 4) < 3) {
                gPlayer.jumpPower = 0x12;
            } else if (n >= 9) {
                gPlayer.jumpPower = 0x14;
                gPlayer.jumpState = JUMP_RELEASED;
                gPlayer.motion[0] = MOT_JUMP_HIGH;
                gPlayer.jumpHeight = 2;
            } else if (gGamePad & PADRright) {
                gPlayer.jumpPower = 0x14;
            } else {
                gPlayer.jumpState = JUMP_RELEASED;
                switch (n) {
                    case 3:
                        gPlayer.motion[0] = MOT_JUMP_LOW;
                        gPlayer.jumpHeight = 0;
                        break;
                    case 7:
                        gPlayer.motion[0] = MOT_JUMP_MID;
                        gPlayer.jumpHeight = 1;
                        break;
                    default:
                        gPlayer.motion[0] = MOT_JUMP_HIGH;
                        gPlayer.jumpHeight = 2;
                        break;
                }
            }
        }
        if (gPlayer.jumpState == JUMP_RELEASED) {
            gPlayer.jumpState = JUMP_NONE;
        }
    }
}

/**
 * @brief Sets the view words behind the player: the viewpoint 400 units
 *        back along the course heading, the reference point 900 ahead at
 *        the player's height; eases sViewSpeed toward the speed. No code
 *        calls it, and it never hands gRefView to the library.
 * @return nothing: no value is set
 */
/* MATCHING: non-void with no return keeps the last branch's delay slot a nop. */
s32 SetPlayerView(void) {
    gRefView[0] = 0;
    gRefView[1] = rsin((gViewerAngle << 12) / 360) * 500 / 4096 + gPlayer.groundY;
    gRefView[2] = rcos((gViewerAngle << 12) / 360) * 600 / 4096;
    gRefView[4] = gPlayer.y;
    gRefView[0] = rsin(gCourse.rot.vy) * 400 / 4096;
    gRefView[2] = rcos(gCourse.rot.vy) * 400 / 4096;
    gRefView[3] = rsin(gCourse.rot.vy - 0x800) * 900 / 4096;
    gRefView[5] = rcos(gCourse.rot.vy - 0x800) * 900 / 4096;
    if (sViewSpeed < gPlayer.speed) {
        sViewSpeed++;
    }
    if (sViewSpeed > gPlayer.speed) {
        sViewSpeed--;
    }
}

/** @brief Sets the scene's three flat lights, one from above in front and
 *         two from behind to the sides, in the stage's light colour
 *         (gStageParams bits 12); sets the fog too once it has faded in, or
 *         on a world's third act. */
/* MATCHING: the index-first integer sum gives retail's addu operand order. */
void UpdateStageLights(void) {
    u8 *c;

    c = (u8 *)((((u32)gStageParams >> 12) & 3) * 4 + (s32)gLightColors);
    sLights.l[0].r = sLights.l[1].r = sLights.l[2].r = c[0];
    sLights.l[0].g = sLights.l[1].g = sLights.l[2].g = c[1];
    sLights.l[0].b = sLights.l[1].b = sLights.l[2].b = c[2];
    sLights.l[0].vx = 0;
    sLights.l[0].vy = 100;
    sLights.l[0].vz = 100;
    GsSetFlatLight(0, &sLights.l[0]);
    sLights.l[1].vx = 86;
    sLights.l[1].vy = 100;
    sLights.l[1].vz = -50;
    GsSetFlatLight(1, &sLights.l[1]);
    sLights.l[2].vx = -86;
    sLights.l[2].vy = 100;
    sLights.l[2].vz = -50;
    GsSetFlatLight(2, &sLights.l[2]);
    if (gFogFade == 100 || gStage % 3 == 2) {
        SetStageFog();
    }
}

/**
 * @brief The play state's frame step (gMissState), skipped on a world's
 *        third act; no value is set as its result.
 *
 * While playing it clamps the bonus level, runs the clock while the level
 * is above 0, fades the fog in and draws the HUD. At time 0 the player
 * stops (a ride is lost) and MISS_TIME_UP freezes the play and holds his
 * position. A fall stops the music and waits 60 frames; a retry restarts
 * the stage or, with no retries left, goes to the menus; MISS_TO_MENU
 * steps the quit effect and leaves after 0x180 frames.
 */
/* MATCHING: the empty case -2 sets the switch's compare tree; non-void with no return keeps
 * the guards' delay slots nops; the unused words give retail's frame. */
s32 UpdatePlayState(void) {
    s32 unused[4];
    u16 i;
    s32 x;
    s16 t;

    if (gStage % 3 == 2) {
        goto end;
    }
top:
    switch (gMissState) {
        case MISS_HALT:
            break;
        case MISS_DYING:
            DrawHud();
            break;
        case MISS_NONE:
            x = gBonusLevel;
            if (gStageTableSet == 1) {
                if (x >= 2) {
                    gBonusLevel = 2;
                }
            } else if (x >= 3) {
                gBonusLevel = 3;
            }
            if (gBonusLevel <= 0) {
                gBonusLevel = 0;
            }
            if (gPlayer.bonusLocked == 1) {
                gBonusLevel = 0;
            }
            if (gBonusLevel != 0 && gPlayer.bonusLocked != 1 && (gStage != STAGE_2_2 || gSection != 0)) {
                gElapsed++;
                gTimeLeft--;
            }
            if (gStage % 3 == 2) {
                break;
            }
            FadeInFog();
            DrawHud();
            if (gInEditor == 1) {
                break;
            }
            if (gTimeLeft > 0) {
                break;
            }
            gTimeLeft = 0;
            gPlayer.control = 0;
            gPlayer.hittable = 0;
            gPlayer.invulnTimer = 500;
            gPlayer.slideState = SLIDE_NONE;
            gPlayer.unk38C = 0;
            if (gPlayer.rideMode & RIDE_MASK) {
                gPlayer.action = ACT_RUN;
                gPlayer.rideMode &= RIDE_ITEM_MASK;
                gPlayer.dropRide = 1;
                gPlayer.jumpPower = 0;
                gPlayer.jumpState = JUMP_NONE;
            }
            if (gSwitches[196].state == SWITCH_ON) {
                break;
            }
            if (gSwitches[198].state == SWITCH_ON) {
                break;
            }
            if (gPlayer.action == ACT_RUN || gPlayer.action == ACT_DASH || gPlayer.action == ACT_BRAKE) {
                if ((s16)gSpeedFixed >> 4 < 15) {
                    gSpeedFixed = 0xF0;
                }
                gPlayer.action = ACT_TIME_UP_START;
                gMissState = MISS_TIME_UP;
                D_800959B0 = 0;
                gMissTimer = 0;
                gPlayer.speed = (s16)gSpeedFixed >> 4;
                SetSoundFade(FADE_CD);
                goto top;
            }
            break;
        case MISS_TIME_UP:
            gMissTimer++;
            DrawFrozenFrame();
            if (gMissTimer == 10) {
                for (i = 0; i < 200; i++) {
                    gSwitches[i].state = SWITCH_OFF;
                }
                gFellOff = 0;
                gPlayLive = 0;
                *(Words3 *)sSavedPos = *(Words3 *)&gPlayer.x;
            }
            if (gMissTimer >= 10) {
                gPlayer.speed = 0;
                gSpeedFixed = 0;
                *(Words3 *)&gPlayer.x = *(Words3 *)sSavedPos;
            }
            if (gMissTimer > 100) {
                t = gMissTimer - 100;
                if (t > 60) {
                    gMissState = MISS_RETRY;
                }
            }
            break;
        case MISS_FELL:
            StopCdTrack();
            PlaySe(SE_FALL);
            gMissTimer = 0;
            gMissState = MISS_FELL_WAIT;
            break;
        case MISS_FELL_WAIT:
            t = ++gMissTimer;
            if (t > 60) {
                gMissState = MISS_RETRY;
            }
            break;
        case MISS_RETRY:
            ClipF = 0;
            if (gRetries == 0) {
                gPlayer.flattened = 0;
                gGameMode = GM_MENU_LOAD;
                gModeStep = 0xFFFF;
            } else {
                RestartStage();
                gRetries--;
            }
            break;
        case MISS_TO_MENU:
            t = ++gMissTimer;
            StepQuitEffect(t);
            if (t == 0x180) {
                gGameMode = GM_MENU_LOAD;
            }
            break;
    }
end:;
}

/** @brief Draws the ride's model where the player's model carries it: a
 *         board at the feet, a roller turning with the speed (with its
 *         rolling sound every 20 frames), a cover on the body; and the held
 *         object while PlayerState.rideMode shows it. */
void DrawModeObject(void) {
    switch (gPlayer.rideMode & RIDE_MASK) {
        case RIDE_BOARD:
            gModeObjects[0].rotX = -0x88;
            gModeObjects[0].rotY = 0xAA;
            gModeObjects[0].y = 0;
            gModeObjectModels[0].parent = (s32)&gTodCoords[16];
            gModeObjects[0].x = -gWorldShift[0];
            gModeObjects[0].z = -gWorldShift[2];
            DrawObjectModel((ObjectModel *)&gModeObjectModels[gModeObjects[0].model],
                            (Trap *)&gModeObjects[0]);
            break;
        case RIDE_BOARD_2:
            gModeObjects[5].rotX = -0x88;
            gModeObjects[5].rotZ = -0x88;
            gModeObjects[5].rotY = 0xAA;
            gModeObjects[5].y = 0;
            gModeObjectModels[5].parent = (s32)&gTodCoords[16];
            gModeObjects[5].x = -gWorldShift[0];
            gModeObjects[5].z = -gWorldShift[2];
            DrawObjectModel((ObjectModel *)&gModeObjectModels[gModeObjects[5].model],
                            (Trap *)&gModeObjects[5]);
            break;
        case RIDE_ROLLER:
            switch (gStage) {
                case STAGE_2_1:
                    gModeObjects[3].y = 0x3C;
                    gModeObjectModels[3].parent = (s32)&gTodCoords[0];
                    gModeObjects[3].x = -gWorldShift[0];
                    gModeObjects[3].z = -gWorldShift[2];
                    gModeObjects[3].rotX += ANGLE_DEG(gPlayer.speed / 2);
                    DrawObjectModel((ObjectModel *)&gModeObjectModels[gModeObjects[3].model],
                                    (Trap *)&gModeObjects[3]);
                    if (gFrameCount % 20 == 0) {
                        PlaySe(SE_ROLLER);
                    }
                    break;
                case STAGE_4_2:
                    gModeObjects[6].y = 0x32;
                    gModeObjectModels[6].parent = (s32)&gTodCoords[0];
                    gModeObjects[6].x = -gWorldShift[0];
                    gModeObjects[6].z = -gWorldShift[2];
                    gModeObjects[6].rotX += ANGLE_DEG(gPlayer.speed / 2);
                    DrawObjectModel((ObjectModel *)&gModeObjectModels[gModeObjects[6].model],
                                    (Trap *)&gModeObjects[6]);
                    if (gFrameCount % 20 == 0) {
                        PlaySe(SE_ROLLER);
                    }
                    break;
            }
            break;
        case RIDE_COVER:
            switch (gStage) {
                case STAGE_1_1:
                    gModeObjects[4].y = 0;
                    gModeObjectModels[4].parent = (s32)&gTodCoords[1];
                    gModeObjects[4].x = -gWorldShift[0];
                    gModeObjects[4].z = -gWorldShift[2];
                    DrawObjectModel((ObjectModel *)&gModeObjectModels[gModeObjects[4].model],
                                    (Trap *)&gModeObjects[4]);
                    break;
                case STAGE_5_1:
                    gModeObjects[2].y = 0;
                    gModeObjectModels[2].parent = (s32)&gTodCoords[1];
                    gModeObjects[2].x = -gWorldShift[0];
                    gModeObjects[2].z = -gWorldShift[2];
                    DrawObjectModel((ObjectModel *)&gModeObjectModels[gModeObjects[2].model],
                                    (Trap *)&gModeObjects[2]);
                    break;
            }
            break;
    }
    if ((gPlayer.rideMode >> 4) == RIDE_ITEM) {
        gModeObjects[1].y = 0;
        gModeObjects[1].x = -gWorldShift[0];
        gModeObjects[1].z = -gWorldShift[2];
        DrawObjectModel((ObjectModel *)&gModeObjectModels[gModeObjects[1].model],
                        (Trap *)&gModeObjects[1]);
    }
}

/**
 * @brief Hangs the ride's model from the player's model and, for a board or
 *        roller not already in a ride motion, starts MOT_RIDE.
 * @return nothing: no value is set
 */
/* MATCHING: non-void with no return makes the guards fill their delay slots with the lui. */
s32 AttachModeObject(void) {
    switch (gPlayer.rideMode & RIDE_MASK) {
        case RIDE_BOARD:
            if ((u32)(gPlayer.motion[0] - MOT_RIDE) >= 7) {
                gPlayer.motion[0] = MOT_RIDE;
                gModeObjectModels[0].parent = (s32)&gTodCoords[16];
                gModeObjects[0].model = 0;
            }
            break;
        case RIDE_BOARD_2:
            if ((u32)(gPlayer.motion[0] - MOT_RIDE) >= 7) {
                gPlayer.motion[0] = MOT_RIDE;
                gModeObjectModels[5].parent = (s32)&gTodCoords[16];
                gModeObjects[5].model = 5;
            }
            break;
        case RIDE_ROLLER:
            if ((u32)(gPlayer.motion[0] - MOT_RIDE) >= 3) {
                gPlayer.motion[0] = MOT_RIDE;
                switch (gStage) {
                    case STAGE_2_1:
                        gModeObjectModels[3].parent = (s32)&gTodCoords[0];
                        gModeObjects[3].model = 3;
                        break;
                    case STAGE_4_2:
                        gModeObjectModels[6].parent = (s32)&gTodCoords[0];
                        gModeObjects[6].model = 6;
                        break;
                }
            }
            break;
        case RIDE_COVER:
            switch (gStage) {
                case STAGE_1_1:
                    gModeObjectModels[4].parent = (s32)&gTodCoords[1];
                    gModeObjects[4].model = 4;
                    break;
                case STAGE_5_1:
                    gModeObjectModels[2].parent = (s32)&gTodCoords[1];
                    gModeObjects[2].model = 2;
                    break;
            }
            break;
    }
}

/**
 * @brief Finds the first free record (no model) of the slot table.
 * @param slot the table's first record
 * @return the free record's index, or -1 when every one is in use
 */
static __inline__ s16 findFreeSlot(SlotRec *slot) {
    u32 i;

    for (i = 0; i < gSlotTableSize / sizeof(SlotRec); i++) {
        if (slot->model == -1) {
            return i;
        }
        slot++;
    }
    return -1;
}

/** @brief Leaves a lost ride behind (PlayerState.dropRide): the stage's ride
 *         model goes, no longer hung from the player, into a free slot
 *         record as a stage object, and the dropped-ride flag is set. */
void SpawnModeObject(void) {
    SlotRec *slot;
    s16 idx;

    slot = (SlotRec *)gSlotTable;
    if (gPlayer.dropRide == 1) {
        idx = findFreeSlot(slot);
        if (idx != -1) {
            slot += idx;
            switch (gStage) {
                case STAGE_1_1:
                    gModeObjectModels[4].parent = 0;
                    slot->model = 4;
                    break;
                case STAGE_1_2:
                    gModeObjectModels[0].parent = 0;
                    slot->model = 0;
                    break;
                case STAGE_2_1:
                    gModeObjectModels[3].parent = 0;
                    slot->model = 3;
                    break;
                case STAGE_4_2:
                    gModeObjectModels[6].parent = 0;
                    slot->model = 6;
                    break;
                case STAGE_5_1:
                    gModeObjectModels[2].parent = 0;
                    slot->model = 2;
                    break;
                case STAGE_5_2:
                    gModeObjectModels[5].parent = 0;
                    slot->model = 5;
                    break;
            }
            slot->switchNo = 0xBA;
            slot->type = 0xFF;
            slot->act = 0x69;
            slot->unk26 = 0;
            slot->unk24 = 0;
            D_800D3358[0] = 1;
        }
        gPlayer.dropRide = 0;
    }
}

/** @brief Roller steering: left or right steers, side-steps 5 units and
 *         builds a drift (within 10) that pushes the player a unit sideways
 *         at its full value; nothing held rolls straight and lets the drift
 *         decay. The player rides 90 (STAGE_2_1) or 80 (STAGE_4_2) above
 *         the ground, and the heading stays within 10 degrees of the
 *         course. */
/* MATCHING: PushPlayer's body as an inline helper; calling it with the s16 step
 * keeps the step's sign extension ahead of the rsin call. */
void SteerRoller(void) {
    s16 step;
    s32 v;
    s32 x;
    PlayerState *g;
    s32 hi;

    if (gPlayer.control != 0) {
        if (gPad & PADLleft) {
            ApplySteer(3, 16, 5);
            sSteerDrift -= 4;
        } else if (gPad & PADLright) {
            ApplySteer(2, 16, 5);
            sSteerDrift += 4;
        } else {
            gPlayer.action = ACT_ROLL_STRAIGHT;
            if (sSteerDrift != 0) {
                if (sSteerDrift < 0) {
                    sSteerDrift += 2;
                } else {
                    sSteerDrift -= 2;
                }
            }
        }
        switch (gStage) {
            case STAGE_2_1:
                gPlayer.y = gPlayer.groundY - 0x5A;
                break;
            case STAGE_4_2:
                gPlayer.y = gPlayer.groundY - 0x50;
                break;
        }
        sSteerDrift = sSteerDrift < -10 ? -10 : sSteerDrift > 10 ? 10 : sSteerDrift;
        step = sSteerDrift / 10;
        pushBack(90, step);
        /* MATCHING: as in SteerCover. */
        __asm__("");
        g = &gPlayer;
        x = g->yaw;
        v = gCourse.rot.vy - 0x71;
        if (x >= v) {
            hi = gCourse.rot.vy + 0x71;
            if (x <= hi) {
                v = x;
            } else {
                v = hi;
            }
        }
        g->yaw = v;
    }
}

/** @brief Roller speed, while in control and in a ride motion: as on foot,
 *         but cruising at 35. */
/* MATCHING: the unsigned range test is the single subtract-and-compare retail does. */
void UpdateRollerSpeed(void) {
    s16 v;

    if (gPlayer.control != 0 && (u32)(gPlayer.motion[0] - MOT_RIDE) < 3) {
        v = gPlayer.speed;
        if (v < 35) {
            gSpeedFixed += 16;
        } else if (gPad & PADLup) {
            gSpeedFixed += 32;
            if ((s16)gSpeedFixed >> 4 > 35) {
                gSpeedFixed = 560;
            }
        } else if (gPad & PADLdown) {
            gSpeedFixed -= 32;
            if ((s16)gSpeedFixed >> 4 < 35) {
                gSpeedFixed = 560;
            }
        } else if (v > 35) {
            gSpeedFixed -= 16;
        } else {
            gSpeedFixed += 16;
        }
        gPlayer.speed = (s16)gSpeedFixed >> 4;
    }
}

/** @brief Cover speed, while in control and in MOT_RIDE: as on foot, but
 *         cruising at 30. */
void UpdateCoverSpeed(void) {
    s16 v;

    if (gPlayer.control != 0 && gPlayer.motion[0] == MOT_RIDE) {
        v = gPlayer.speed;
        if (v < 30) {
            gSpeedFixed += 16;
        } else if (gPad & PADLup) {
            gSpeedFixed += 32;
            if ((s16)gSpeedFixed >> 4 > 30) {
                gSpeedFixed = 480;
            }
        } else if (gPad & PADLdown) {
            gSpeedFixed -= 32;
            if ((s16)gSpeedFixed >> 4 < 30) {
                gSpeedFixed = 480;
            }
        } else if (v > 30) {
            gSpeedFixed -= 16;
        } else {
            gSpeedFixed += 16;
        }
        gPlayer.speed = (s16)gSpeedFixed >> 4;
    }
}

/** @brief UpdateJump without the jump start's record, playing MOT_JUMP or
 *         MOT_JUMP_HIGH; no code calls it. */
/* MATCHING: the switch lays the default arm out last, after its own base reload, as retail. */
void UpdateCoverJump(void) {
    s16 n;

    if (gPlayer.control != 0) {
        if ((gGamePadTrig & PADRright) &&
            (gPlayer.y >= gPlayer.groundY || (gPlayer.onRideQuad == 1 && gPlayer.onQuadTop == 1)) &&
            gPlayer.jumpState == JUMP_NONE) {
            gPlayer.airborne = 1;
            gPlayer.onQuadTop = 0;
            gPlayer.jumpState = JUMP_RISING;
            gPlayer.jumpTime = 0;
            gPlayer.peakY = 0;
            OnJumpStart(&gPlayer);
            gPlayer.motion[0] = MOT_JUMP;
            gPlayer.jumpPower = 0x23;
            gPlayer.jumpStartY = gPlayer.y;
            ScatterDust((VECTOR *)&gPlayer.x);
            PlaySe(SE_JUMP);
        }
        if (gPlayer.jumpState == JUMP_RISING) {
            n = ++gPlayer.jumpTime;
            if (n < 3) {
                gPlayer.jumpPower = 0x19;
            } else if ((u16)(n - 4) < 3) {
                gPlayer.jumpPower = 0x12;
            } else if (n >= 9) {
                gPlayer.jumpPower = 0x14;
                gPlayer.jumpState = JUMP_RELEASED;
                gPlayer.motion[0] = MOT_JUMP;
                gPlayer.jumpHeight = 2;
            } else if (gGamePad & PADRright) {
                gPlayer.jumpPower = 0x14;
            } else {
                gPlayer.jumpState = JUMP_RELEASED;
                switch (n) {
                    case 3:
                        gPlayer.motion[0] = MOT_JUMP_HIGH;
                        gPlayer.jumpHeight = 0;
                        break;
                    case 7:
                        gPlayer.motion[0] = MOT_JUMP_HIGH;
                        gPlayer.jumpHeight = 1;
                        break;
                    default:
                        gPlayer.motion[0] = MOT_JUMP_HIGH;
                        gPlayer.jumpHeight = 2;
                        break;
                }
            }
        }
        if (gPlayer.jumpState == JUMP_RELEASED) {
            gPlayer.jumpState = JUMP_NONE;
        }
    }
}

/** @brief Cover steering, mirrored: right steers mode 3 and left mode 2,
 *         side-stepping 8 units; nothing held on the ground rides straight.
 *         The drift stays within 7, so its tenth never pushes the player.
 *         The heading stays within 10 degrees of the course. */
void SteerCover(void) {
    s16 step;
    s32 v;
    s32 x;
    PlayerState *g;
    s32 hi;

    if (gPlayer.control != 0) {
        if (gPad & PADLright) {
            ApplySteer(3, 16, 8);
            sSteerDrift -= 4;
        } else if (gPad & PADLleft) {
            ApplySteer(2, 16, 8);
            sSteerDrift += 4;
        } else if (gPlayer.y >= gPlayer.groundY) {
            gPlayer.action = ACT_RIDE_STRAIGHT;
            if (sSteerDrift != 0) {
                if (sSteerDrift < 0) {
                    sSteerDrift += 2;
                } else {
                    sSteerDrift -= 2;
                }
            }
        }
        sSteerDrift = sSteerDrift < -7 ? -7 : sSteerDrift > 7 ? 7 : sSteerDrift;
        step = sSteerDrift / 10;
        pushBack(90, step);
        /* MATCHING: the barrier keeps the yaw and unk380 loads below the unk350 store;
         * g keeps the gPlayer base in a register for the store at the join. */
        __asm__("");
        g = &gPlayer;
        x = g->yaw;
        v = gCourse.rot.vy - 0x71;
        if (x >= v) {
            hi = gCourse.rot.vy + 0x71;
            if (x <= hi) {
                v = x;
            } else {
                v = hi;
            }
        }
        g->yaw = v;
    }
}

/**
 * @brief Ends a knockdown, throw or launch arc: lands a falling player on a
 *        ride quad's top or the ground, with the landing sound and a hard
 *        rumble.
 * @param g the player's state
 * @return 2 on a ride quad, 1 on the ground, 0 still in the air
 */
s32 LandPlayer(PlayerState *g) {
    s32 ret;

    ret = 0;
    if (g->peakY <= g->rideQuadY && g->onRideQuad == 1) {
        if (g->y >= gPlayer.rideQuadY) {
            g->y = gPlayer.rideQuadY;
            ret = 2;
            PlaySe(SE_LAND);
        }
    } else if (g->y >= g->groundY) {
        g->y = g->groundY;
        ret = 1;
        PlaySe(SE_LAND);
    }
    if (ret != 0) {
        StartRumble(gRumblePatterns, RUMBLE_HARD_LAND);
    }
    return ret;
}

s32 AddBonusLevel(s32 n) {
    gBonusLevel += n;
    if (gStageTableSet == 1) {
        if (gBonusLevel >= 2) {
            gBonusLevel = 2;
        }
    } else if (gBonusLevel >= 3) {
        gBonusLevel = 3;
    }
    if (gBonusLevel <= 0) {
        gBonusLevel = 0;
    }
    if (gPlayer.bonusLocked == 1) {
        gBonusLevel = 0;
    }
    return gBonusLevel;
}

/**
 * @brief The board stage's side-step: moves the player some units in a
 *        direction measured from the course heading (0 is forward).
 * @param deg  the direction, in degrees from the course heading
 * @param dist how far
 */
/* MATCHING: the local pointer lets angle and the gPlayer base share $s0. */
void PushPlayer(s32 deg, s32 dist) {
    s32 angle;
    SVECTOR *rot;

    rot = &gCourse.rot;
    angle = ANGLE_DEG(deg);
    gPlayer.x -= rsin(rot->vy + angle) * dist >> FIX12_SHIFT;
    gPlayer.z -= rcos(rot->vy + angle) * dist >> FIX12_SHIFT;
}

/**
 * @brief Sets PlayerState.speedBand from the speed: 0 below 13, 1 below
 *        26, else 2.
 * @return nothing: no value is set
 */
/* MATCHING: non-void with no return keeps the first delay slot a nop. */
s32 UpdateSpeedBand(void) {
    s16 v;

    v = gPlayer.speed;
    if (v < 13) {
        gPlayer.speedBand = 0;
    } else if (v >= 26) {
        gPlayer.speedBand = 2;
    } else {
        gPlayer.speedBand = 1;
    }
}

/** @brief Back to running under control at the end of a hit's action:
 *         MOT_RUN_SLOW (MOT_RIDE on a cover), ACT_RUN, no slide; after a
 *         hit reaction that asked for it, 60 frames of invulnerability. */
void ResetPlayerAction(void) {
    gPlayer.motion[0] = MOT_RUN_SLOW;
    gPlayer.action = ACT_RUN;
    gPlayer.control = 1;
    gPlayer.slideState = SLIDE_NONE;
    if (gHitRecover == 1) {
        gPlayer.hittable = 0;
        gPlayer.invulnTimer = 60;
        gHitRecover = 0;
    }
    if ((gPlayer.rideMode & RIDE_MASK) == RIDE_COVER) {
        gPlayer.motion[0] = MOT_RIDE;
    }
}

/** @brief On every tenth can: raises the bonus level, with the power-up
 *         sound unless the level is already full or locked. */
/* MATCHING: `one` is the 1 retail keeps in $a0 for both compares. */
void CheckCanBonus(void) {
    s32 one;

    if (gCans != 0 && gCans % 10 == 0) {
        one = 1;
        if (gStageTableSet == one) {
            if (gBonusLevel == 2) {
                goto skip;
            }
        } else if (gBonusLevel == 3) {
            goto skip;
        }
        if (gPlayer.bonusLocked != one) {
            PlaySe(SE_POWER_UP);
        }
    skip:
        AddBonusLevel(1);
    }
}

/** @brief Empty and never called. */
void UnusedHook4(void) {}

/** @brief Empty; runs every frame of SLIDE_BRAKING. */
void UpdateBrake(void) {}

/**
 * @brief Starts a slide (MOT_SLIDE_START, with its sound) on the slide
 *        button without up held, while in control, not already sliding,
 *        and on the ground or past the strong part of a jump's rise.
 * @return nothing: no value is set
 */
/* MATCHING: non-void with no return keeps the second bnez's delay slot a nop. */
s32 TrySlide(void) {
    if (gPlayer.control != 0 && (gPlayer.motion[0] < MOT_SLIDE_START || gPlayer.motion[0] >= MOT_JUMP) &&
        (gPlayer.airborne != 1 || gPlayer.jumpPower < 6) && !(gPad & PADLup) &&
        (gGamePadTrig & PADRdown)) {
        PlaySe(SE_SLIDE);
        gPlayer.motion[0] = MOT_SLIDE_START;
    }
}

/** @brief Cuts a jump short: no jump power left, and the jump released. */
void EndJump(void) {
    gPlayer.jumpPower = 0;
    gPlayer.jumpState = JUMP_RELEASED;
}

/**
 * @brief The x and z of a point at a distance and angle from the origin;
 *        y is left as it was.
 * @param deg    the angle, in degrees
 * @param radius the distance
 * @param out    receives x and z
 */
void PolarOffset(s32 deg, s32 radius, VECTOR *out) {
    s32 angle;

    angle = ANGLE_DEG(deg);
    out->vx = rsin(angle) * radius >> FIX12_SHIFT;
    out->vz = rcos(angle) * radius >> FIX12_SHIFT;
}

/** @brief Sets the fog: from 3000 to 8000 in depth, in the stage's far
 *         colour (gStageParams bits 8 pick it from gFogColors). */
void SetStageFog(void) {
    s32 i;

    i = ((u32)gStageParams >> 8) & 3;
    SetFogNearFar(3000, 8000, 250);
    SetFarColor(gFogColors[i * 4], gFogColors[i * 4 + 1], gFogColors[i * 4 + 2]);
}

/**
 * @brief Sorts a flat quad into the sprite ordering table, its corners given
 *        in order round the edge; no code calls it.
 * @param col a 15-bit colour, five bits each of red, green and blue
 * @param x0  the first corner's x
 * @param y0  the first corner's y
 * @param x1  the second corner's x
 * @param y1  the second corner's y
 * @param x2  the third corner's x
 * @param y2  the third corner's y
 * @param x3  the fourth corner's x
 * @param y3  the fourth corner's y
 * @param pri its depth in the ordering table
 */
/* MATCHING: the red channel through its own local moves the spill to y2, as retail. */
void SortQuad15(u16 col, s16 x0, s16 y0, s16 x1, s16 y1, s16 x2, s16 y2, s16 x3, s16 y3, u16 pri) {
    POLY_F4 poly;
    POLY_F4 *p;
    s32 r;

    p = &poly;
    SetPolyF4(p);
    r = col & 0x1F;
    p->r0 = r << 3;
    p->g0 = ((col >> 5) & 0x1F) << 3;
    p->b0 = ((col >> 10) & 0x1F) << 3;
    p->x0 = x0;
    p->y0 = y0;
    p->x1 = x1;
    p->y1 = y1;
    p->x2 = x3;
    p->y2 = y3;
    p->x3 = x2;
    p->y3 = y2;
    GsSortPoly(p, &gSubOt[gActiveBuff], pri);
}

/** @brief Stops the player, with the stepped speed back at 40 for the speed
 *         steps to build from. */
void ResetSpeed(void) {
    gPlayer.speed = 0;
    gSpeedFixed = 0x280;
}

/** @brief Stops the player dead: the speed and the stepped speed both 0. */
void StopSpeed(void) {
    gPlayer.speed = 0;
    gSpeedFixed = 0;
}

/** @brief While the player holds the stop pose's object (RIDE_ITEM in
 *         PlayerState.rideMode's high nibble), hangs its model from part 8
 *         of the player's model. */
void AttachHeldItem(void) {
    if ((gPlayer.rideMode >> 4) == RIDE_ITEM) {
        gModeObjectModels[1].parent = (s32)&gTodCoords[8];
        gModeObjects[1].model = 1;
    }
}
