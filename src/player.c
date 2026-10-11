#include "common.h"
#include "libetc.h"
#include "libgte.h"
#include "libgpu.h"
#include "libgs.h"
#include "draw.h"
#include "player.h"
#include "object.h"
#include "model.h"
#include "rand.h"

/** @brief The player's state, gPlayer, as far as this unit reads it. The
 *         shared header declares it as a byte array; other units keep
 *         their own views of it. */
typedef struct {
    u8 control;  /**< 1 while the pad steers the player; cleared by a miss or the goal */
    u8 unk1;     /**< the steering mode last applied; a repeat only resets yaw */
    u8 hittable; /**< 1 when a hit can be reacted to; 0 while invulnTimer runs */
    u8 unk3;     /**< nonzero steps two animation tracks instead of one */
    u8 unk4;     /**< the argument of the single-track step */
    u8 action;   /**< the action state UpdatePlayerAction runs (jump, slide, hit reactions) */
    u8 motion;   /**< the animation sequence asked for; 2 to 4 the run bands, 8 to 11 the jump */
    u8 unk7;     /**< 0x13 or 0xFF; not yet known */
    u8 unk8;     /**< set to 0xFF; not yet known */
    u8 unk9;     /**< set to 0xFF; not yet known */
    u8 padA[0x340 - 0xA];
    s8 animResult;  /**< StepAnimation's result for the first track; 2 at a loop's end */
    s8 animResult2; /**< StepAnimation's result for the second track */
    u8 pad342[0x344 - 0x342];
    s32 unk344; /**< (40 - speed) / 3 + 1 */
    s32 x;      /**< position x */
    s32 y;      /**< position y, down positive; held at groundY on the ground */
    s32 z;      /**< position z */
    s32 drawX;  /**< added to x for the drawn position */
    s32 drawY;  /**< added to y for the drawn position */
    s32 drawZ;  /**< added to z for the drawn position */
    s16 unk360; /**< an angle copied to the camera rotation's x */
    s16 unk362; /**< an angle added to yaw for the camera rotation's y */
    u8 pad364[0x368 - 0x364];
    s32 unk368; /**< cleared; not yet known */
    u8 pad36C[0x370 - 0x36C];
    s32 unk370; /**< cleared; not yet known */
    s16 unk374; /**< cleared when y passes the goal line */
    s16 unk376; /**< a step counter of the current mode */
    s16 unk378; /**< a second step counter of the current mode */
    s16 unk37A; /**< cleared with unk378 */
    u8 pad37C[0x380 - 0x37C];
    s32 yaw;  /**< the body heading; follows the course heading in bounded steps */
    s32 lean; /**< the body roll while steering, -15..15; decays to 0 */
    u8 pad388[0x38C - 0x388];
    s16 unk38C;     /**< cleared whenever slideState changes */
    u8 slideState;  /**< 0 running, 1 sliding, 2 dashing */
    u8 unk38F;      /**< 1 also gates a check on jumpPower */
    u16 jumpState;  /**< 0 on the ground, 1 while the button is held, 2 for one step after */
    s16 jumpTime;   /**< steps the button has been held while jumpState is 1 */
    s32 jumpStartY; /**< y when the jump began */
    s16 jumpPower;  /**< the jump's strength, set by how long the button is held */
    u8 pad39A[0x39C - 0x39A];
    s32 unk39C;    /**< with unk3CC, picks which cap y gets */
    s32 unk3A0;    /**< 0, 1 or 2, set as jumpState goes to 2 */
    s16 unk3A4;    /**< a countdown set to 1 by some speed ranges */
    s16 fallSpeed; /**< added (times 5) to y each step in the air; reset on landing */
    s16 speed;     /**< the running speed; 40 is cruising */
    s16 unk3AA;    /**< speed saved when a mode starts */
    u16 speedBand; /**< 0 below 13 in speed, 1 below 26, else 2 */
    s16 stepX;     /**< speed along the sine of the heading: this step's x move */
    s16 stepZ;     /**< speed along the cosine of the heading: this step's z move */
    u8 pad3B2[0x3B4 - 0x3B2];
    s32 unk3B4;  /**< moved by 50 a step while steering */
    s32 unk3B8;  /**< 1 selects the global cap for y */
    s32 unk3BC;  /**< 1 with unk3B8 also starts the jumpState sequence */
    s32 groundY; /**< the ground height under the player */
    u8 pad3C4[0x3C8 - 0x3C4];
    s16 invulnTimer; /**< counts down after a hit; the player is hittable again at 0 */
    s16 unk3CA;      /**< set to 1 when the stage ends */
    s32 unk3CC;      /**< with unk39C, picks which cap y gets */
    u8 rideMode;     /**< low nibble: 0 on foot, else the stage's ride (1 to 4) */
    u8 unk3D1;       /**< set to 1 when a stage ends with a mode in rideMode */
    u8 unk3D2;       /**< set to 1 by one mode */
    s8 showShadow;   /**< nonzero draws the shadow under the player */
    u8 unk3D4;       /**< 1 while mode 0xF runs */
    u8 unk3D5;       /**< set to 1 by some pickups */
} GameState;

/* MATCHING: a struct lvalue keeps the base in a register; array offsets fold into %lo. */
#define sGame (*(GameState *)gPlayer)

/* MATCHING: retail reaches these through a split lui/%lo pair, so each is
 * an array of unknown size here. */
extern s32 D_800D8440[];
extern u8 D_800D8960[];
extern s16 D_800D38DE[];
extern s8 D_8009EF4D[];

extern s32 D_800AC860;
extern s16 D_800959B2;
/* MATCHING: cc1 splits this load (its lui sits in a branch delay slot, away
 * from the lw), so it is an array here, though both halves use one register. */
extern s32 D_8009EF44[];

/** @brief A point of a 21-by-16 grid spanning the 320x240 screen, 16
 *         pixels apart, stored row by row. */
typedef struct {
    s32 x; /**< column offset from the screen centre */
    s32 y; /**< row offset from the screen centre */
    s32 z; /**< always 0 */
} GridPoint;

extern s32 D_800959A8;
extern GridPoint D_800DE5E0[];
extern DVECTOR D_800DE0A0[];
extern s32 D_800DED50[];
extern s32 D_800957F8;
extern s32 D_80095800;
extern s32 D_80095804;
extern s32 D_8009580C;

/* MATCHING: a per-unit view; stage stores it as an s32. */
extern u8 *D_80095790;

/** @brief The three flat lights of the scene: one overhead-front, two behind to the sides. */
typedef struct {
    GsF_LIGHT l[3]; /**< one per light index */
} LightTable;

/* MATCHING: a struct lvalue keeps the table's base in a register. */
#define sLights (*(LightTable *)gFlatLights)
extern s32 D_8009EEF8[];

/** @brief A 72-byte record of the shared slot table, as the item
 *         spawner fills it; collision's Slot48 is the same record. */
typedef struct {
    s32 unk0; /**< set to minus the view offset's x when a stage object is placed */
    s32 unk4; /**< cleared, or 0x3C or 0x32, when a stage object is placed */
    s32 unk8; /**< set to minus the view offset's z when a stage object is placed */
    u8 padC[0x18 - 0xC];
    s16 unk18; /**< -0x88, or turned by half of unk3A8 degrees */
    s16 unk1A; /**< 0xAA when a stage object is placed */
    s16 unk1C; /**< -0x88 for one stage object */
    u8 pad1E[0x24 - 0x1E];
    s16 unk24; /**< cleared on a spawn */
    s16 unk26; /**< cleared on a spawn */
    u8 pad28[0x34 - 0x28];
    s16 unk34; /**< 0xBA on a spawn */
    s16 unk36; /**< -1 when the record is free, else its kind */
    u8 pad38[0x42 - 0x38];
    u8 unk42; /**< 0xFF on a spawn */
    u8 unk43; /**< 0x69 on a spawn */
    u8 pad44[0x48 - 0x44];
} SlotRec;

/** @brief A 0x78-byte model slot, paired with the SlotRec of the same
 *         index; main's ModelSlot is the same record. */
typedef struct {
    u8 pad0[0x58];
    s32 unk58; /**< the address of the coordinate system the model hangs from */
    u8 pad5C[0x78 - 0x5C];
} ObjRec;

extern SlotRec D_800D3860[];
/* MATCHING: main declares this as its ModelSlot; this unit's view names the word at 0x58. */
extern ObjRec D_800D8370[];
/* MATCHING: object types these Rec78 and Rec48; this unit's records are views of them. */
void DrawObjectModel(Rec78 *rec, Rec48 *r);

/* MATCHING: retail reaches these through a split lui/%lo pair. */
extern u8 D_8009EF49[];
extern s32 D_800D85A8[];
extern s32 D_800D83C8[];
extern s32 D_800D8530[];
extern s32 D_800D8698[];
extern s32 D_800D84B8[];
extern s32 D_800D8620[];
extern u8 D_800D3358[];
extern s16 D_800D3896[];
extern s16 D_800D3926[];
extern s16 D_800D396E[];
extern s16 D_800D39B6[];
extern s16 D_800D39FE[];
extern s16 D_800D3A46[];

void ApplySteer(u8 mode, u16 a, s16 b);
void SteerPlayer(void);
s32 CheckFall(void);
s32 ApplyGravity(void);
s32 UpdateRunSpeed(void);
s32 UpdateSlideDash(void);
void UpdateJump(void);
s32 AttachModeObject(void);
void SpawnModeObject(void);
void SteerMode2(void);
void UpdateSpeedMode2(void);
void UpdateSpeedMode3(void);
void SteerMode3(void);
s32 UpdateSpeedBand(void);
void PolarOffset(s32 deg, s32 radius, VECTOR *out);
void AttachMode5Object(void);
s32 TrySlide(void);
/* MATCHING: editor stores it as u8; this unit's tests load it lb. */
extern s8 D_80095962;

/** @brief One of gPlayerHits' three records; a live one is reacted to. */
typedef struct {
    s8 unk0; /**< nonzero when the slot is live */
    u8 pad1;
    u8 unk2; /**< what hit, a row of the HitReaction table */
    u8 pad3[7];
} HitSlot;

/* MATCHING: common.h declares the table as bytes; this unit reads its records. */
#define sHits ((HitSlot *)gPlayerHits)

/** @brief How the player reacts to one kind of hit: the action per state. */
typedef struct {
    u8 b[8]; /**< per stage mode and flag */
} HitReaction;

extern HitReaction gHitReactions[];
extern s8 D_8009575F;
/* Overlay entry points: the stage modes' own code. */
void func_800F8A58(void);
void func_800F8B60(void);
void func_800F6FA0(void);
void func_800F70A8(void);
void func_800F8D64(void);
void func_800F729C(void);
void RecentreSteer(void);
void SetStageFog(void);
void func_80028500(void);

/* MATCHING: stage defines x..n as s16; this unit's calls pass them
 * unextended, so its prototype takes s32. */
s32 SpawnEffect(s32 kind, s32 x, s32 y, s32 z, s32 delay);
void StartRumble(u16 *table, s32 index);
extern s32 gGamePad;
extern u16 D_8009587E;
extern s16 D_800957B0;
extern s32 D_800956E4;
extern s32 D_800956E0;
extern s32 D_800DF5A0[];
extern s32 D_800DF5B0[];

/** @brief Three words moved as one, for a block copy. */
typedef struct {
    s32 w[3]; /**< the words */
} Words3;

extern s32 D_800DF5C0[];
/* MATCHING: a per-unit view; object declares it the same way, editor
 * as words. */
extern Rec5C D_800CF080[];
extern u8 D_800959B0;
extern s16 gMissTimer;
void DrawHud(void);
/* MATCHING: stage_flow defines this with a u8 parameter; this unit's call passes none. */
void RestartStage();
void func_8003E360(s32 level);
/* MATCHING: stage_flow stores this as u8; this unit's test loads it lb. */
extern s8 D_80095900;
s32 DistanceSquared(VECTOR *a, VECTOR *b);
/* MATCHING: model defines this as an empty void(void); this unit's call
 * passes the state block in $a0. */
void func_80018CA4(GameState *g);

void DrawPlayerShadow(void) {
    GsCOORDINATE2 coord;
    MATRIX ls;
    SVECTOR size;
    s32 unused[2];
    CVECTOR color;
    s32 d;

    if (sGame.showShadow != 0) {
        GsInitCoordinate2(WORLD, &coord);
        coord.coord.t[0] = sGame.x + sGame.drawX + gWorldShift[0];
        coord.coord.t[1] = gFloorY[0];
        coord.coord.t[2] = sGame.z + sGame.drawZ + gWorldShift[2];
        GsGetLs(&coord, &ls);
        GsSetLsMatrix(&ls);
        d = (sGame.groundY - sGame.y) / 10;
        color.r = 2;
        size.vx = 100 - d;
        size.vy = size.vx / 2;
        color.g = color.b = color.cd = 0x40 - d;
        DrawBillboard(0x12D, &size, &color, 2, &gSubOt[gActiveBuff]);
    }
}

void InitGridTest(void) {
    for (D_800958D0 = 0; D_800958D0 < 16; D_800958D0++) {
        for (D_800958CC = 0; D_800958CC < 21; D_800958CC++) {
            D_800DE5E0[D_800958D0 * 21 + D_800958CC].x = D_800958CC * 16 - 160;
            D_800DE5E0[D_800958D0 * 21 + D_800958CC].y = D_800958D0 * 16 - 120;
            D_800DE5E0[D_800958D0 * 21 + D_800958CC].z = 0;
        }
    }
    gRefView[0] = 0;
    D_800959A8 = 0;
    gRefView[1] = 0;
    gRefView[2] = 1000;
    gRefView[3] = 0;
    gRefView[4] = 0;
    gRefView[5] = 0;
}

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
        D_800959A8 = 0;
    }
    if (gPad & PADL1) {
        D_800959A8 += 2;
    }
    if (gPad & PADR1) {
        D_800959A8 -= 2;
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
    for (D_800958D0 = 0; D_800958D0 < 16; D_800958D0++) {
        for (D_800958CC = 0; D_800958CC < 21; D_800958CC++) {
            x = D_800DE5E0[D_800958D0 * 21 + D_800958CC].x;
            y = D_800DE5E0[D_800958D0 * 21 + D_800958CC].y;
            GsInitCoordinate2(WORLD, &coord);
            coord.coord.t[2] = 0;
            coord.flg = 0;
            coord.coord.t[0] = x;
            coord.coord.t[1] = y;
            GsGetLs(&coord, &ls);
            GsSetLsMatrix(&ls);
            v.vx = v.vy = v.vz = 0;
            D_800DE5E0[D_800958D0 * 21 + D_800958CC].z = RotTransPers(&v, (long *)&sxy, &p, &flag);
            D_800DE0A0[D_800958D0 * 21 + D_800958CC].vx = line.x0 = line.x1 = sxy.vx;
            D_800DE0A0[D_800958D0 * 21 + D_800958CC].vy = line.y0 = line.y1 = sxy.vy;
            GsSortLine(&line, &gOt[gActiveBuff], 0);
        }
    }
    pp = &poly;
    for (D_800958D0 = 0; D_800958D0 < 15; D_800958D0++) {
        for (D_800958CC = 0; D_800958CC < 20; D_800958CC++) {
            SetPolyF4(pp);
            pp->x0 = D_800DE0A0[D_800958D0 * 21 + D_800958CC].vx;
            pp->y0 = D_800DE0A0[D_800958D0 * 21 + D_800958CC].vy;
            pp->x1 = D_800DE0A0[D_800958D0 * 21 + (D_800958CC + 1)].vx;
            pp->y1 = D_800DE0A0[D_800958D0 * 21 + (D_800958CC + 1)].vy;
            pp->x3 = D_800DE0A0[(D_800958D0 + 1) * 21 + (D_800958CC + 1)].vx;
            pp->y3 = D_800DE0A0[(D_800958D0 + 1) * 21 + (D_800958CC + 1)].vy;
            pp->x2 = D_800DE0A0[(D_800958D0 + 1) * 21 + D_800958CC].vx;
            pp->y2 = D_800DE0A0[(D_800958D0 + 1) * 21 + D_800958CC].vy;
            y = 1;
            if (D_800958D0 & y) {
                if (D_800958CC & y) {
                    pp->r0 = 0x70;
                    pp->g0 = 0x70;
                    pp->b0 = 0x80;
                } else {
                    pp->r0 = 0xFF;
                    pp->g0 = 0xFF;
                    pp->b0 = 0xFF;
                }
            } else if (D_800958CC & 1) {
                pp->r0 = 0xFF;
                pp->g0 = 0xFF;
                pp->b0 = 0xFF;
            } else {
                pp->r0 = 0x70;
                pp->g0 = 0x70;
                pp->b0 = 0x80;
            }
            GsSortPoly(pp, &gOt[gActiveBuff], D_800DE5E0[D_800958D0 * 21 + D_800958CC].z >> 6);
        }
    }
    D_800957F8 = D_800959A8;
    D_80095800 = D_800DED50[0];
    D_80095804 = D_800DE0A0[0].vx;
    D_8009580C = D_800DE0A0[0].vy;
}

void func_80022F58(void) {}

void func_80022F60(void) {}

void ScatterEffect4(VECTOR *pos) {
    SpawnEffect(4, (s16)pos->vx + (rand() % 160 - 80), (s16)pos->vy,
                (s16)pos->vz + (rand() % 160 - 80), 1);
}

void ScatterEffect6(VECTOR *pos) {
    SpawnEffect(6, (s16)pos->vx + (rand() % 160 - 80), (s16)pos->vy,
                (s16)pos->vz + (rand() % 160 - 80), 1);
}

void func_800230D8(void) {}

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

/* MATCHING: g keeps the sGame base in a register for the clamp's store at the join;
 * cases 1 and 2 of the second switch are separate arms, merged by cross-jumping. */
void UpdatePlayer(void) {
    s32 t;
    s32 x;
    s32 v;
    s32 hi;
    GameState *g;

    if (D_80095962 != 1) {
        CheckFall();
    }
    UpdateSpeedBand();
    ApplyGravity();
    if (sGame.control == 1) {
        AttachModeObject();
    }
    AttachMode5Object();
    SpawnModeObject();
    switch (sGame.rideMode & 0xF) {
        case 0:
            UpdateRunSpeed();
            UpdateSlideDash();
            UpdateJump();
            TrySlide();
            break;
        case 1:
            func_800F8A58();
            func_800F8B60();
            break;
        case 4:
            func_800F6FA0();
            func_800F70A8();
            break;
        case 2:
            UpdateSpeedMode2();
            break;
        case 3:
            UpdateSpeedMode3();
            break;
    }
    sGame.unk344 = (40 - sGame.speed) / 3 + 1;
    t = sGame.speed;
    switch (((u32)D_80095864 >> 22) & 3) {
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
            if (sGame.y >= sGame.groundY) {
                PolarOffset(D_800A7680->vy * 360 / 4096 - 180, 30, (VECTOR *)D_800D39C8);
                sGame.x += D_800D39C8[0];
                sGame.z += D_800D39C8[2];
            }
            break;
    }
    switch (((u32)D_80095864 >> 20) & 3) {
        case 1:
            if (D_8009EF20[0] != 0) {
                t = 30;
            }
            break;
        case 2:
            if (D_8009EF20[0] != 0) {
                t = 30;
            }
            break;
        case 3:
            if (sGame.y >= sGame.groundY) {
                PolarOffset(D_800A7680->vy * 360 / 4096, 30, (VECTOR *)D_800D39C8);
                sGame.x += D_800D39C8[0];
                sGame.z += D_800D39C8[2];
            }
            break;
    }
    gSpeedFixed = t << 4;
    switch (D_8009EF48[0] & 0xF) {
        case 0:
        case 1:
        case 3:
        case 4:
            sGame.stepX = rsin(D_800A7680->vy) * sGame.speed >> 12;
            sGame.stepZ = rcos(D_800A7680->vy) * sGame.speed >> 12;
            sGame.x -= sGame.stepX;
            sGame.z -= sGame.stepZ;
            break;
        case 2:
            sGame.stepX = rsin(sGame.yaw) * sGame.speed >> 12;
            sGame.stepZ = rcos(sGame.yaw) * sGame.speed >> 12;
            sGame.x -= sGame.stepX;
            sGame.z -= sGame.stepZ;
            break;
    }
    switch (D_8009EF48[0] & 0xF) {
        case 0:
            SteerPlayer();
            break;
        case 1:
            func_800F8D64();
            break;
        case 4:
            func_800F729C();
            break;
        case 2:
            SteerMode2();
            break;
        case 3:
            SteerMode3();
            break;
    }
    g = &sGame;
    x = g->yaw;
    v = D_800A7680->vy - 0x155;
    if (x >= v) {
        hi = D_800A7680->vy + 0x155;
        if (x <= hi) {
            v = x;
        } else {
            v = hi;
        }
    }
    g->yaw = v;
    D_8009EAB8[1] = sGame.yaw;
    D_8009EAB8[2] = sGame.lean;
    D_800D86E0[0].coord.t[0] = sGame.drawX;
    D_800D86E0[0].coord.t[1] = sGame.y + sGame.drawY;
    D_800D86E0[0].coord.t[2] = sGame.drawZ;
    SetCoordRotation((SVECTOR *)D_8009EAB8, D_800D86E0);
    gWorldShift[1] = 0;
    gWorldShift[0] = -sGame.x;
    gWorldShift[2] = -sGame.z;
}

void SteerPlayer(void) {
    if (sGame.control != 0) {
        switch (gPadType) {
            case 0x73:
                if (gPad & PADLleft) {
                    ApplySteer(3, 16, 15);
                } else if (gPad & PADLright) {
                    ApplySteer(2, 16, 15);
                } else {
                    RecentreSteer();
                }
                break;
            case 0x41:
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
        if (sGame.control != 0) {
            return;
        }
    }
    D_8009EEF8[0] = D_800A7680[0].vy;
}

static __inline__ void pushBack(s32 deg, s32 dist) {
    s32 angle;
    SVECTOR *rot;

    rot = D_800A7680;
    angle = ANGLE_DEG(deg);
    sGame.x -= rsin(rot->vy + angle) * dist >> FIX12_SHIFT;
    sGame.z -= rcos(rot->vy + angle) * dist >> FIX12_SHIFT;
}

/* MATCHING: the (u16) read is the separate lhu retail stores when the heading is in range. */
void ApplySteer(u8 mode, u16 a, s16 b) {
    s16 yaw;
    s32 w;

    if (sGame.unk1 != mode) {
        switch (mode) {
            case 3:
                yaw = D_800A7680->vy;
                if (yaw - 0x2D < sGame.yaw) {
                    sGame.yaw = yaw - 0x2D;
                }
                D_800957BC -= a * 2;
                D_800957B0 -= a;
                sGame.yaw = D_800957BC;
                if ((sGame.rideMode & 0xF) != 2 && yaw < sGame.yaw) {
                    sGame.yaw = yaw;
                }
                if (sGame.lean < 0) {
                    sGame.lean += 0x22;
                }
                if (sGame.lean < 16) {
                    w = sGame.lean;
                } else {
                    w = 15;
                }
                sGame.lean = w;
                sGame.unk3B4 -= 50;
                pushBack(-90, b);
                break;
            case 2:
                yaw = D_800A7680->vy;
                if (sGame.yaw < yaw + 0x4000) {
                    sGame.yaw = yaw + 0x2D;
                }
                D_800957BC += a * 2;
                D_800957B0 += a;
                sGame.yaw = D_800957BC;
                if ((sGame.rideMode & 0xF) != 2 && sGame.yaw < yaw) {
                    sGame.yaw = yaw;
                }
                if (sGame.lean > 0) {
                    sGame.lean -= 0x22;
                }
                if (sGame.lean >= -15) {
                    w = sGame.lean;
                } else {
                    w = -15;
                }
                sGame.lean = w;
                sGame.unk3B4 += 50;
                pushBack(90, b);
                break;
        }
        D_800957BC = D_800957BC < D_800A7680->vy - 0x100   ? D_800A7680->vy - 0x100
                     : D_800A7680->vy + 0x100 < D_800957BC ? D_800A7680->vy + 0x100
                                                           : (u16)D_800957BC;
        if (sGame.unk3B8 == 1 && sGame.unk3BC == 1 && sGame.unk38F == 0) {
            sGame.y = sGame.groundY;
        }
    } else {
        sGame.yaw = D_800A7680->vy;
    }
}

/* MATCHING: the inline ternary abs (not an if on d) lets the first +56 test reuse the
 * loaded value when it skips the store. */
void RecentreSteer(void) {
    u16 i;
    s32 yaw;
    s32 d;

    for (i = 0; i < 3; i++) {
        if (sGame.lean < 0) {
            sGame.lean++;
        }
        if (sGame.lean > 0) {
            sGame.lean--;
        }
        yaw = D_800A7680->vy;
        d = sGame.yaw - yaw;
        if ((d < 0 ? -d : d) > 56) {
            if (sGame.yaw < yaw) {
                sGame.yaw += 56;
            }
            if (sGame.yaw > yaw) {
                sGame.yaw -= 56;
            }
        } else {
            if (sGame.yaw < yaw) {
                sGame.yaw += 11;
            }
            if (sGame.yaw > yaw) {
                sGame.yaw -= 11;
            }
        }
        D_800957BC = sGame.yaw;
    }
}

/* MATCHING: non-void with no return keeps the last beqz delay slot a nop. */
s32 CheckFall(void) {
    s32 unused[2];

    if ((D_800AC860 & 0x40) && sGame.motion != 0xE) {
        if (sGame.rideMode & 0xF) {
            sGame.unk3CA = 1;
            if ((sGame.rideMode & 0xF) != 3) {
                sGame.jumpPower = 20;
                sGame.rideMode &= 0xF0;
            }
            ResetSpeed();
            sGame.control = 0;
        }
        sGame.groundY = 30000;
        if (sGame.y >= gFloorY[0] + 100) {
            if (gMissState == 0) {
                gMissState = 2;
            }
            D_800958EC = 1;
            sGame.control = 0;
            sGame.fallSpeed += 2;
            ResetSpeed();
            sGame.unk374 = 0;
            if (sGame.y > gFloorY[0] + 0x8C && (sGame.rideMode & 0xF) == 3) {
                sGame.rideMode &= 0xF0;
            }
            sGame.unk3CA = 1;
            if (sGame.y > gFloorY[0] + 1000) {
                sGame.motion = 12;
                sGame.fallSpeed = 0;
                sGame.rideMode &= 0xF0;
            }
        }
    }
}

/* MATCHING: y reloaded, reused for unk3CC and killed by the dead y = 0 keeps retail's copy. */
s32 ApplyGravity(void) {
    s32 x;
    s32 d;
    s32 y;
    s32 z;

    x = sGame.y;
    if (x < D_800956E4) {
        D_800956E4 = x;
    }
    y = sGame.y;
    if (y < sGame.unk39C) {
        sGame.unk39C = y;
    }
    if ((u32)(sGame.motion - 8) < 4) {
        goto end;
    }
    if ((u32)(sGame.motion - 0x38) < 2 && (sGame.rideMode & 0xF) != 3) {
        goto end;
    }
    d = sGame.groundY - y;
    y = 0;
    if ((d >= 0 ? d : -d) < 50) {
        sGame.y = sGame.groundY;
    } else {
        sGame.y = x + (u16)sGame.fallSpeed * 5;
    }
    if (sGame.unk3B8 == 1 && sGame.unk3BC == 1) {
        sGame.fallSpeed = 0;
        sGame.unk39C = 0;
        sGame.y = sGame.unk3CC;
        goto end;
    }
    d = sGame.unk39C;
    y = sGame.unk3CC;
    if (d <= y && sGame.unk3B8 == 1) {
        if (sGame.y >= y) {
            sGame.y = y;
            sGame.fallSpeed = 0;
            sGame.unk39C = 0;
            sGame.unk3BC = sGame.unk3B8;
        } else {
            sGame.fallSpeed++;
        }
        goto end;
    }
    if (sGame.y >= sGame.groundY) {
        z = gFloorY[0];
        sGame.fallSpeed = 0;
        sGame.unk39C = 0;
        sGame.unk3BC = 0;
        sGame.y = z;
        if (D_800956E0 == 0) {
            D_800DF5B0[0] = sGame.x;
            D_800DF5B0[1] = z;
            D_800DF5B0[2] = sGame.z;
            D_800956E0 = SquareRoot0(DistanceSquared((VECTOR *)D_800DF5A0, (VECTOR *)D_800DF5B0));
            if (D_80095900 == 0) {
                StartRumble(gRumblePatterns, 13);
            }
        }
    } else {
        sGame.fallSpeed++;
    }
end:;
}

/* MATCHING: the volatile read keeps the later unk2 reads as reloads, as retail. */
void ReactToHits(u8 *state) {
    u8 v;
    s16 i;
    u8 m;
    HitSlot *p;

    v = 0;
    if (((GameState *)state)->hittable == 0) {
        return;
    }
    if (D_80095962 == 1) {
        return;
    }
    for (i = 0; i < 3; i++) {
        p = &sHits[i];
        if (p->unk0 == 0) {
            continue;
        }
        D_8009599C = 0;
        if ((u32)(*(volatile u8 *)&p->unk2 - 15) >= 4) {
            m = ((GameState *)state)->motion;
            switch (((GameState *)state)->action) {
                case 0:
                case 0x5E:
                    switch (m) {
                        case 0x33:
                            if ((sGame.rideMode & 0xF) != 3) {
                                break;
                            }
                        case 2:
                        case 3:
                        case 4:
                            v = gHitReactions[sHits[i].unk2].b[0];
                            break;
                        case 8:
                        case 9:
                        case 10:
                        case 11:
                            v = gHitReactions[sHits[i].unk2].b[1];
                            break;
                        case 5:
                        case 6:
                        case 7:
                            v = gHitReactions[sHits[i].unk2].b[2];
                            break;
                    }
                    break;
                case 0x39:
                case 0x3A:
                    v = gHitReactions[p->unk2].b[3];
                    break;
                case 0x3B:
                case 0x3C:
                    v = gHitReactions[p->unk2].b[0];
                    break;
                default:
                    switch (sGame.rideMode & 0xF) {
                        case 1:
                        case 2:
                        case 4:
                            v = 0x42;
                            break;
                        case 3:
                        default:
                            v = gHitReactions[sHits[i].unk2].b[4];
                            break;
                    }
                    break;
            }
            switch (sGame.rideMode & 0xF) {
                case 1:
                case 2:
                case 4:
                    v = 0x42;
                    break;
            }
        } else {
            v = gHitReactions[p->unk2].b[2 - i];
            if (sGame.rideMode & 0xF) {
                v = 0x42;
            }
        }
        switch (sGame.rideMode & 0xF) {
            case 1:
            case 2:
            case 4:
                sGame.unk3D1 = 1;
                sGame.unk3D5 = 1;
                gBonusLevel = 0;
                sGame.rideMode &= 0xF0;
                ((GameState *)state)->hittable = 0;
                D_8009599C = 1;
                ((GameState *)state)->invulnTimer = 750;
                ((GameState *)state)->action = v;
                if (sHits[i].unk2 != 0) {
                    EndJump();
                }
                D_8009575F = sHits[0].unk0 | sHits[1].unk0 | sHits[2].unk0;
                return;
            case 0:
            default:
                if (v == 0) {
                    continue;
                }
                if (gHitReactions[sHits[i].unk2].b[5] == 1) {
                    ((GameState *)state)->hittable = 0;
                    ((GameState *)state)->invulnTimer = 150;
                    D_8009599C = 1;
                } else {
                    ((GameState *)state)->hittable = 0;
                    ((GameState *)state)->invulnTimer = 18;
                }
                if (gHitReactions[sHits[i].unk2].b[6] == 1) {
                    sGame.unk3D5 = 1;
                    gBonusLevel = 0;
                }
                if (gHitReactions[sHits[i].unk2].b[7] == 1 &&
                    (s8)(sHits[0].unk0 | sHits[1].unk0 | sHits[2].unk0) != D_8009575F) {
                    AddBonusLevel(-1);
                }
                ((GameState *)state)->action = v;
                if (sHits[i].unk2 != 0) {
                    EndJump();
                }
                D_8009575F = sHits[0].unk0 | sHits[1].unk0 | sHits[2].unk0;
                return;
        }
    }
    D_8009575F = 0;
}

/* MATCHING: model types the object as its own view; this unit passes the state block. */
u8 StepAnimation(GameState *obj, u16 index, u8 arg);
/* MATCHING: model defines this with s16/u16 parameters and an s16 result; this unit's
 * calls pass halfwords unextended and store the result as a word. */
s32 Lerp(s32 from, s32 to, s32 step, s32 steps);
/* MATCHING: empty void(void) in model; unprototyped here, called with two args and one. */
void func_80018CAC();

s32 LandPlayer(GameState *g);
void ResetPlayerAction(void);

/* MATCHING: s32 with no return keeps the delay slots before the exit nops; d is one pseudo for
 * every step so global-alloc gives it $a1, the step's argument register. */
s32 UpdatePlayerAction(GameState *state) {
    u8 m;
    s16 t;
    s32 v;
    s8 r;
    s32 d;

    m = state->motion;
    t = --sGame.invulnTimer;
    if (t < 0) {
        sGame.invulnTimer = 0;
        sGame.hittable = 1;
    } else {
        if (sHits[0].unk0 | sHits[1].unk0 | sHits[2].unk0) {
            if (t == 1) {
                sGame.invulnTimer = 2;
            }
        }
        sGame.hittable = 0;
    }
    if (state->unk3 == 0) {
        state->animResult = StepAnimation(state, 0, state->unk4);
    } else {
        state->animResult = StepAnimation(state, 0, 1);
        state->animResult2 = StepAnimation(state, 1, 2);
    }
top:
    switch (state->action) {
        case 0x7:
            state->action = 8;
            state->motion = 0xD;
            state->control = 0;
            StartRumble(gRumblePatterns, 1);
            break;
        case 0x9:
            state->action = 0xA;
            state->motion = 0xD;
            state->control = 0;
            state->unk376 = 0;
            state->unk374 = 0;
            StartRumble(gRumblePatterns, 2);
            break;
        case 0xF:
            state->action = 0x10;
            state->unk4 = 0x80;
            break;
        case 0x11:
            state->action = 0x12;
            state->unk374 = 0;
            state->unk368 = 0;
            state->unk370 = 0;
            break;
        case 0x12:
            if (state->unk374 < 60) {
                state->unk374++;
            }
            D_80095740 = Lerp(D_80095740, 0, state->unk374, 60);
            /* MATCHING: a byte-pointer read keeps the load below the store to D_80095740. */
            D_80095744 = Lerp(D_80095744, 0, *(s16 *)((u8 *)state + 0x374), 60);

            if (D_80095740 == 0) {
                state->unk4 = 0;
                state->action = 0;
            }
            break;
        case 0x13:
            state->action = 0x14;
            state->motion = 0x15;
            state->control = 0;
            state->unk376 = 0;
            state->unk374 = 0;
            StartRumble(gRumblePatterns, 7);
            break;
        case 0x17:
            if ((u8)(m - 2) < 3) {
                state->action = 0x18;
                state->motion = 0x10;
                state->unk374 = 0;
                state->control = 0;
                state->unk3AA = state->speed;
            }
            break;
        case 0x23:
            state->action = 0x24;
            state->motion = 0xE;
            state->unk374 = 0;
            state->control = 0;
            state->unk3AA = state->speed;
            break;
        case 0x19:
            state->action = 0x1A;
            state->control = 0;
            state->unk374 = 0;
            if (state->speed == 0) {
                state->motion = 0xF;
            } else {
                state->motion = 0x10;
                state->unk374 = 0;
                state->unk3AA = state->speed;
            }
            break;
        case 0x1B:
            state->action = 0x1C;
            state->control = 0;
            if (state->speed == 0) {
                state->animResult = 1;
                state->unk374 = 0;
                state->motion = 0x11;
            } else {
                state->motion = 0x10;
                state->unk374 = 0;
                state->unk3AA = state->speed;
            }
            break;
        case 0x1F:
            state->action = 0x20;
            state->control = 0;
            if (state->speed != 0) {
                state->motion = 0x10;
                state->unk374 = 0;
                state->unk3AA = state->speed;
            } else {
                state->animResult = 1;
                state->motion = 0x12;
            }
            break;
        case 0x27:
            PlaySe(0x3C);
            state->action = 0x28;
            state->motion = 0x14;
            state->jumpPower = 70;
            state->unk376 = 0;
            state->unk374 = 0;
            state->control = 0;
            break;
        case 0x39:
            state->action = 0x3A;
            state->motion = 0x1A;
            state->unk376 = 0;
            state->unk3AA = state->speed;
            StartRumble(gRumblePatterns, 10);
        case 0x3A:
            if (state->unk376 < 5) {
                DrawFrozenFrame();
                gSpeedFixed = 0x5A0;
            } else {
                gSpeedFixed = 0x460;
            }
            state->unk376++;
            state->speed = (s16)gSpeedFixed >> 4;
            ScatterEffect4((VECTOR *)&state->x);
            if (state->unk376 >= 30) {
                state->action = 0;
                if (sGame.y >= sGame.groundY && (u32)(sGame.motion - 5) >= 3) {
                    state->motion = 4;
                }
            }
            break;
        case 0x3B:
            PlaySe(0x16);
            state->action = 0x3C;
            state->control = 0;
            state->motion = 0x18;
            state->unk378 = 0;
            state->unk376 = 0;
            state->unk374 = 0;
            state->unk3AA = state->speed;
            StartRumble(gRumblePatterns, 12);
        case 0x3C:
            gSpeedFixed -= 0x20;
            if (++state->unk378 >= 16 || (s16)gSpeedFixed < 0) {
                gSpeedFixed = 0;
                state->action = 0x3D;
                state->motion = 0x19;
                state->control = 1;
            }
            state->speed = (s16)gSpeedFixed >> 4;
            ScatterEffect4((VECTOR *)&state->x);
            break;
        case 0x3D:
            if (state->animResult == 1) {
                state->motion = 4;
                state->action = 0;
                state->slideState = 0;
                gSpeedFixed = 0x320;
            }
            break;
        case 0x3E:
            state->action = 0x3F;
            state->motion = 0x1B;
            state->jumpPower = 70;
            state->unk376 = 0;
            state->unk374 = 0;
            state->control = 0;
            StartRumble(gRumblePatterns, 2);
            break;
        case 0x40:
            state->action = 0x41;
            state->motion = 0x1B;
            state->jumpPower = 70;
            state->unk376 = 0;
            state->unk374 = 0;
            state->control = 0;
            StartRumble(gRumblePatterns, 2);
            break;
        case 0x42:
            state->action = 0x43;
            state->motion = 0x23;
            if (state->speed < 90) {
                state->speed = 70;
            }
            state->control = 0;
            state->jumpPower = state->speed;
            /* MATCHING: *= -1 reloads the halfword with lh. */
            state->speed *= -1;
            StartRumble(gRumblePatterns, 4);
            break;
        case 0x44:
            state->action = 0x45;
            state->motion = 0x23;
            if (state->speed < 90) {
                state->speed = 70;
            }
            state->control = 0;
            state->jumpPower = state->speed;
            state->speed *= -1;
            StartRumble(gRumblePatterns, 4);
            break;
        case 0x46:
            state->action = 0x47;
            state->motion = 0x28;
            ResetSpeed();
            state->unk374 = 10;
            state->unk376 = 0;
            state->control = 0;
            StartRumble(gRumblePatterns, 5);
            break;
        case 0x48:
            state->action = 0x49;
            state->motion = 0x2C;
            state->unk376 = 0;
            state->unk374 = 0;
            state->control = 0;
            if (AddBonusLevel(0) <= 0) {
                state->action = 0x42;
                goto top;
            }
            StartRumble(gRumblePatterns, 4);
            ResetSpeed();
            break;
        case 0x4A:
            state->action = 0x4B;
            state->motion = 0x2E;
            state->unk376 = 0;
            state->unk374 = 0;
            state->control = 0;
            StartRumble(gRumblePatterns, 4);
            break;
        case 0x4C:
            state->action = 0x4D;
            state->motion = 0x2F;
            ResetSpeed();
            state->control = 0;
            StartRumble(gRumblePatterns, 4);
            break;
        case 0x4E:
            state->action = 0x4F;
            state->motion = 0x30;
            ResetSpeed();
            state->unk37A = 0;
            state->unk378 = 0;
            state->unk376 = 0;
            state->unk374 = 0;
            state->control = 0;
            StartRumble(gRumblePatterns, 8);
            break;
        case 0x50:
            if (rand() & 1) {
                state->action = 0x51;
                state->motion = 0x30;
            } else {
                state->action = 0x55;
                state->motion = 0x23;
            }
            ResetSpeed();
            state->unk37A = 0;
            state->unk378 = 0;
            state->unk376 = 0;
            state->unk374 = 0;
            state->control = 0;
            state->hittable = 0;
            state->invulnTimer = 150;
            StartRumble(gRumblePatterns, 4);
            break;
        case 0x52:
            state->action = 0x53;
            state->motion = 0x32;
            state->unk3D2 = 1;
            ResetSpeed();
            state->unk37A = 0;
            state->unk378 = 0;
            state->unk376 = 0;
            state->unk374 = 0;
            state->control = 0;
            StartRumble(gRumblePatterns, 9);
            break;
        case 0x56:
            gMissState = -1;
            state->action = 0x57;
            state->motion = 0x23;
            ResetSpeed();
            state->jumpPower = 300;
            /* MATCHING: a bare barrier keeps the call's arguments below this store. */
            __asm__("");
            state->unk37A = 0;
            state->unk378 = 0;
            state->unk376 = 0;
            state->unk374 = 0;
            state->control = 0;
            StartRumble(gRumblePatterns, 4);
            break;
        case 0x33:
            switch (state->motion) {
                case 0x33:
                    break;
                case 0x36:
                    if (state->animResult == 0) {
                        state->motion = 0x37;
                    }
                    break;
                case 0x34:
                    if (state->animResult == 0) {
                        state->motion = 0x35;
                    }
                    break;
                case 0x35:
                case 0x37:
                    if (state->animResult == 0) {
                        state->action = 0x34;
                        state->motion = 0x33;
                    }
                    break;
            }
            break;
        case 0x35:
            switch (state->motion) {
                case 0x35:
                case 0x36:
                    break;
                case 0x33:
                case 0x37:
                    if (state->animResult == 0) {
                        state->action = 0x36;
                        state->motion = 0x34;
                    }
                    break;
            }
            break;
        case 0x37:
            switch (state->motion) {
                case 0x34:
                    break;
                case 0x33:
                case 0x35:
                    if (state->animResult == 0) {
                        state->action = 0x38;
                        state->motion = 0x36;
                    }
                    break;
            }
            break;
        case 0x5A:
            state->motion = 0x34;
            break;
        case 0x5C:
            state->motion = 0x35;
            break;
        case 0x58:
        case 0x5E:
            state->motion = 0x33;
            break;
    }
    switch (state->motion) {
        case 2:
            if (gGameMode == GM_STAGE && gLoading == 0 && (gFrameCount & 7) == 0) {
                PlaySe(0x13);
            }
            break;
        case 3:
            if (gGameMode == GM_STAGE && gLoading == 0 && gFrameCount % 6 == 0) {
                PlaySe(0x13);
            }
            break;
        case 4:
            if (gGameMode == GM_STAGE && gLoading == 0 && (gFrameCount & 3) == 0) {
                PlaySe(0x13);
            }
            break;
        case 8:
        case 9:
        case 0xA:
        case 0xB:
        case 0x38:
        case 0x39:
            state->jumpPower -= 5;
            d = state->jumpPower;
            state->y -= d;
            if (state->unk39C <= sGame.unk3CC && state->unk3B8 == 1) {
                if (state->y >= sGame.unk3CC) {
                    state->y = sGame.unk3CC;
                    func_80018CAC(&sGame, d);
                    /* MATCHING: case 0 shares the default body, which the compare tree reaches first. */
                    switch ((s8)sGame.slideState) {
                        case 0:
                        default:
                            state->motion = 3;
                            break;
                        case 2:
                            state->motion = 0x1A;
                            break;
                    }
                    ScatterEffect4((VECTOR *)&state->x);
                    state->unk38F = 0;
                    state->unk3BC = 1;
                    StartRumble(gRumblePatterns, 13);
                }
            } else if (state->y >= state->groundY) {
                state->y = gFloorY[0];
                func_80018CAC(&sGame);
                switch ((s8)sGame.slideState) {
                    case 0:
                    default:
                        state->motion = 3;
                        break;
                    case 2:
                        state->motion = 0x1A;
                        break;
                }
                ScatterEffect4((VECTOR *)&state->x);
                state->unk38F = 0;
                state->unk3BC = 0;
                StartRumble(gRumblePatterns, 13);
                D_800DF5B0[0] = state->x;
                D_800DF5B0[1] = state->y;
                D_800DF5B0[2] = state->z;
                D_800956E0 = SquareRoot0(DistanceSquared((VECTOR *)D_800DF5A0, (VECTOR *)D_800DF5B0));
            }
            break;
        case 0xD:
            state->speed -= 2;
            if (state->speed <= 0) {
                ResetSpeed();
            }
            /* MATCHING: cases 7, 0x1F and 0x26 branch here; their own calls would be the copies
             * cross-jumping keeps. */
            if (state->animResult == 0 && (state->action == 8 || state->action == 0xA)) {
            reset:
                ResetPlayerAction();
            }
            break;
        case 0x15:
        case 0x16:
        case 0x17:
            state->speed -= 4;
            if (state->speed <= 0) {
                ResetSpeed();
            }
            if (sGame.unk374 == 0) {
                PlaySe(0x1A);
            }
            state->control = 0;
            sGame.unk374++;
            sGame.unk362 += ANGLE_DEG(sGame.unk374 / 2);
            if (state->animResult == 0) {
                sGame.unk376 = (sGame.unk376 + 20) % 360;
                if (sGame.unk376 == 80 || sGame.unk376 == 260) {
                    state->motion = 0x16;
                }
                if (sGame.unk376 == 160 || sGame.unk376 == 340) {
                    state->motion = 0x17;
                }
                v = rsin(ANGLE_DEG(sGame.unk376));
                /* MATCHING: one expression, so the table's %hi is built before the abs. */
                sGame.y = gFloorY[0] - ((v * 100 >> 12) >= 0 ? (v * 100 >> 12) : -(v * 100 >> 12));

                if (sGame.unk374 >= 30) {
                    state->unk362 = 0;
                    ResetSpeed();
                    if (AddBonusLevel(0) != 0) {
                        ResetPlayerAction();
                    } else {
                        sGame.unk374 = 0;
                        state->motion = 0x31;
                    }
                }
            }
            D_8009EAB8[1] = sGame.yaw + sGame.unk362;
            D_800D86E0[0].coord.t[0] = sGame.drawX;
            D_800D86E0[0].coord.t[1] = sGame.y + sGame.drawY;
            D_800D86E0[0].coord.t[2] = sGame.drawZ;
            SetCoordRotation((SVECTOR *)D_8009EAB8, D_800D86E0);
            break;
        case 0x10:
            if (state->unk374 < 10) {
                state->unk374++;
            }
            state->speed = Lerp(state->unk3AA, 0, state->unk374, 10);
            switch (state->action) {
                case 0x18:
                    if (state->animResult == 0) {
                        state->motion = 0x13;
                    }
                    break;
                case 0x1A:
                    if (state->animResult == 0) {
                        state->motion = 0xF;
                    }
                    break;
                case 0x1C:
                    if (state->animResult == 0) {
                        state->motion = 0x11;
                    }
                    break;
                case 0x20:
                    if (state->animResult == 0) {
                        state->motion = 0x12;
                    }
                    break;
            }
            break;
        case 0xE:
            if (state->unk374 < 60) {
                state->unk374++;
            }
            state->speed = Lerp(state->unk3AA, 0, state->unk374, 60);
            break;
        case 0x13:
            state->control = 0;
            break;
        case 0xF:
            sGame.unk3D4 = 1;
            if (state->animResult == 0) {
                if (++state->unk374 >= 46) {
                    state->action = 0;
                    state->motion = 0x13;
                    sGame.unk3D4 = 0;
                }
            }
            break;
        case 0x11:
            if (++state->unk374 == 13) {
                PlaySe(0x30);
            }
            if (state->unk374 == 20) {
                PlaySe(0x31);
            }
            if (state->unk374 == 40) {
                state->rideMode = (state->rideMode & 0xF) | 0x50;
            }
            if (state->unk374 == 90) {
                PlaySe(0x2B);
            }
            if (state->unk374 == 110) {
                PlaySe(0x2B);
            }
            if (state->unk374 == 130) {
                PlaySe(0x2B);
            }
            if (state->unk374 == 90) {
                state->unk3 = 1;
                state->unk7 = 0x13;
            }
            if (state->animResult2 == 0) {
                state->unk3 = 0;
                state->unk7 = 0xFF;
                state->unk9 = 0xFF;
            }
        case 0x12:
            if (state->animResult == 0) {
                state->action = 0;
                state->motion = 0x13;
                state->rideMode &= 0xF;
            }
            break;
        case 5:
            state->unk3 = 0;
            if (state->animResult == 0) {
                state->motion = 6;
                if (state->speed >= 0xB0) {
                    state->unk3A4 = 1;
                }
                if ((u32)((u16)state->speed - 0x97) < 0x19) {
                    state->unk3A4 = 1;
                }
                if ((u32)((u16)state->speed - 0x7E) < 0x19) {
                    state->unk3A4 = 1;
                }
                if (state->speed < 0x7E) {
                    state->unk3A4 = 1;
                }
            }
            if (sGame.y == sGame.groundY) {
                ScatterEffect4((VECTOR *)&state->x);
            }
            StartRumble(gRumblePatterns, 14);
            break;
        case 6:
            if (gFrameCount % 3 == 0) {
                ScatterEffect4((VECTOR *)&state->x);
            }
            if (state->unk3A4 == 0) {
                state->motion = 7;
            }
            state->unk3A4--;
            break;
        case 0x14:
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
                        state->motion = 0x24;
                    }
            break;
        case 0x1B:
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
                            case 0x41:
                                StartRumble(gRumblePatterns, 3);
                                state->motion = 0x20;
                                break;
                            case 0x3F:
                                state->motion = 0x1C;
                                break;
                        }
                    }
            break;
        case 0x1C:
            state->speed -= 2;
            if (state->speed >= 0x15) {
                ScatterEffect4((VECTOR *)D_8009EEC0);
            }
            if (state->speed <= 0) {
                ResetSpeed();
                if (++state->unk374 >= 16) {
                    state->unk374 = 0;
                    state->motion = 0x1D;
                }
            }
            break;
        case 0x1D:
            state->unk374++;
            if (state->animResult == 0) {
                if (AddBonusLevel(0) != 0) {
                    state->unk376 = 0;
                    state->motion = 0x1F;
                } else {
                    if (state->unk374 == 30) {
                        PlaySe(0x34);
                    }
                    if (state->unk374 >= 61) {
                        gMissState = 100;
                    }
                }
            }
            break;
        case 0x1E:
            if (state->animResult == 0) {
                switch (state->action) {
                    case 0x14:
                    case 0x28:
                    case 0x3F:
                    case 0x43:
                        state->unk376 = 0;
                        state->motion = 0x1F;
                        break;
                    case 0x41:
                    case 0x45:
                        state->unk376 = 0;
                        state->motion = 0x22;
                        break;
                }
            }
            break;
        case 0x20:
            if (state->animResult == 0) {
                state->unk374++;
                ScatterEffect4((VECTOR *)D_8009EEC0);
                state->motion = 0x20;
                state->unk8 = 0xFF;
            }
            if (state->unk374 == 2) {
                state->unk374 = 0;
                state->motion = 0x21;
            }
            break;
        case 0x21:
            state->speed -= 1;
            if (state->speed >= 6) {
                ScatterEffect4((VECTOR *)D_8009EEC0);
                StartRumble(gRumblePatterns, 11);
            }
            if (state->speed <= 0) {
                ResetSpeed();
                if (++state->unk374 >= 16) {
                    switch (state->action) {
                        default:
                            if (AddBonusLevel(0) != 0) {
                                state->unk374 = 0;
                                state->unk376 = 0;
                                state->motion = 0x1F;
                            } else {
                                if (state->unk374 == 30) {
                                    PlaySe(0x34);
                                }
                                if (state->unk374 >= 61) {
                                    gMissState = 100;
                                }
                            }
                            break;
                        case 0x49:
                            if (AddBonusLevel(0) != 0) {
                                state->unk376 = 0;
                                state->motion = 0x2D;
                            } else {
                                if (state->unk374 == 30) {
                                    PlaySe(0x34);
                                }
                                if (state->unk374 >= 61) {
                                    gMissState = 100;
                                }
                            }
                            break;
                        case 0x4F:
                            if (AddBonusLevel(0) != 0) {
                                state->motion = 0x22;
                            } else {
                                if (state->unk374 == 30) {
                                    PlaySe(0x34);
                                }
                                if (state->unk374 >= 61) {
                                    gMissState = 100;
                                }
                            }
                            break;
                        case 0x57:
                            gMissState = -1;
                            if (state->unk374 >= 61) {
                                if (gStage == 4 && gSection == 0) {
                                    state->motion = 0x22;
                                    gMissState = 0;
                                } else {
                                    gMissState = 100;
                                }
                            }
                            if ((gStage != 4 || gSection != 0) && state->unk374 == 30) {
                                PlaySe(0x34);
                            }
                            break;
                    }
                }
            }
            break;
        case 0x23:
            if (state->action == 0x55) {
                sGame.drawX = Lerp(0, gView[0], state->unk378, 5);
                sGame.drawY = Lerp(0, -50, state->unk378, 5);
                sGame.drawZ = Lerp(0, gView[2], state->unk378, 5);
                if (++state->unk378 >= 5) {
                    StartRumble(gRumblePatterns, 5);
                    state->motion = 0x28;
                    state->unk37A = 0;
                    state->unk378 = 0;
                    state->unk376 = 0;
                    state->unk374 = 0;
                }
                D_800D86E0[0].coord.t[0] = sGame.drawX;
                D_800D86E0[0].coord.t[1] = sGame.y + sGame.drawY;
                D_800D86E0[0].coord.t[2] = sGame.drawZ;
                SetCoordRotation((SVECTOR *)D_8009EAB8, D_800D86E0);
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
                        state->motion = 0x24;
                    }
            break;
        case 0x24:
            ScatterEffect4((VECTOR *)D_8009EEC0);
            if (state->animResult == 0) {
                switch (state->action) {
                    case 0x28:
                    case 0x43:
                    case 0x57:
                        state->motion = 0x25;
                        break;
                    case 0x45:
                        StartRumble(gRumblePatterns, 3);
                        state->motion = 0x27;
                        break;
                }
            }
            break;
        case 0x25:
            if (state->animResult == 0) {
                state->unk376 = 0;
                state->unk374 = 0;
                state->motion = 0x21;
            }
            break;
        case 0x27:
            if (state->animResult == 0) {
                state->unk374++;
                ScatterEffect4((VECTOR *)D_8009EEC0);
                state->motion = 0x27;
                state->unk8 = 0xFF;
            }
            state->unk376++;
            if (state->unk374 >= 2) {
                if (AddBonusLevel(0) > 0) {
                    switch (state->action) {
                        case 0x45:
                            state->unk374 = 0;
                            state->motion = 0x21;
                            break;
                        case 0x47:
                            state->unk374 = 0;
                            state->unk376 = 0;
                            state->motion = 0x1F;
                            state->speed = 0;
                            state->unk374 = 0;
                            break;
                    }
                } else {
                    StartRumble(gRumblePatterns, 6);
                    state->motion = 0x21;
                    state->unk374 = 0;
                    if (state->unk376 == 30) {
                        PlaySe(0x34);
                    }
                    if (state->unk376 >= 61) {
                        gMissState = 100;
                    }
                }
            }
            break;
        case 0x28:
            switch (state->action) {
                case 0x51:
                    D_8009EAB8[1] = sGame.unk362 = D_800A7680[0].vy + 0x800;
                case 0x55:
                    D_800D86E0[0].coord.t[0] = sGame.drawX;
                    D_800D86E0[0].coord.t[1] = sGame.y + sGame.drawY;
                    D_800D86E0[0].coord.t[2] = sGame.drawZ;
                    SetCoordRotation((SVECTOR *)D_8009EAB8, D_800D86E0);
                    if (++state->unk378 < 10) {
                        /* MATCHING: x - (5 - r) keeps the -5 on the loaded word. */
                        gView[0] = gView[0] - (5 - rand() % 10);
                        gView[1] = gView[1] - (5 - rand() % 10);
                        gView[2] = gView[2] - (5 - rand() % 10);
                        GsSetRefView2((GsRVIEW2 *)gView);
                    }
                    if (state->unk378 >= 31) {
                        sGame.drawY += 2;
                    }
                    gMissState = -1;
                    if (state->unk378 >= 121) {
                        gMissState = 100;
                    }
                    break;
                default:
                    state->unk374--;
                    state->unk376++;
                    if (state->unk374 <= 0) {
                        state->unk374 = 1;
                    }
                    if (state->unk376 & 1) {
                        sGame.unk360 = -(state->unk374 * 0x5000 / 360);
                    } else {
                        sGame.unk360 = state->unk374 * 0x5000 / 360;
                    }
                    D_8009EAB8[0] = sGame.unk360;
                    D_800D86E0[0].coord.t[0] = sGame.drawX;
                    D_800D86E0[0].coord.t[1] = sGame.y + sGame.drawY;
                    D_800D86E0[0].coord.t[2] = sGame.drawZ;
                    SetCoordRotation((SVECTOR *)D_8009EAB8, D_800D86E0);
                    if (state->animResult == 0) {
                        sGame.unk360 = 0;
                        D_8009EAB8[0] = 0;
                        state->motion = 0x29;
                    }
                    break;
            }
            break;
        case 0x29:
            if (state->animResult == 0) {
                state->unk374 = 0;
                state->unk376 = 0;
                state->speed = -5;
                StartRumble(gRumblePatterns, 3);
                state->motion = 0x27;
            }
            break;
        case 0x2A:
            switch (state->action) {
                default:
                    if (state->animResult == 0 && ++state->unk374 >= 16) {
                        if (AddBonusLevel(0) != 0) {
                            state->motion = 0x2B;
                        } else {
                            if (state->unk374 == 30) {
                                PlaySe(0x34);
                            }
                            if (state->unk374 >= 61) {
                                gMissState = 100;
                            }
                        }
                    }
                    break;
                case 0x4B:
                    if (state->speed >= 0x15) {
                        ScatterEffect4((VECTOR *)D_8009EEC0);
                    }
                    state->speed -= 2;
                    if (state->speed <= 0) {
                        ResetSpeed();
                    }
                    if (state->animResult == 0 && state->speed == 0 && ++state->unk374 >= 16) {
                        if (AddBonusLevel(0) != 0) {
                            state->motion = 0x2B;
                        } else {
                            if (state->unk374 == 30) {
                                PlaySe(0x34);
                            }
                            if (state->unk374 >= 61) {
                                gMissState = 100;
                            }
                        }
                    }
                    break;
                case 0x4D:
                    if (state->animResult == 0 && ++state->unk374 >= 16) {
                        if (AddBonusLevel(0) != 0) {
                            state->unk376 = 0;
                            state->motion = 0x26;
                            state->unk374 = 0;
                        } else {
                            if (state->unk374 == 30) {
                                PlaySe(0x34);
                            }
                            if (state->unk374 >= 61) {
                                gMissState = 100;
                            }
                        }
                    }
                    break;
            }
            break;
        case 0x2B:
            if (state->animResult == 0) {
                state->unk376 = 0;
                state->motion = 0x1F;
            }
            break;
        case 0x2C:
            if (state->animResult == 0) {
                state->unk376 = 0;
                state->motion = 0x2D;
            }
            break;
        case 0x2D:
            sGame.drawX = rsin(D_800A7680[0].vy + 0x400) * 20 >> 12;
            sGame.drawZ = rcos(D_800A7680[0].vy + 0x400) * 20 >> 12;
            D_800D86E0[0].coord.t[2] = sGame.drawZ;
            D_800D86E0[0].coord.t[0] = sGame.drawX;
            D_800D86E0[0].coord.t[1] = sGame.y + sGame.drawY;
            SetCoordRotation((SVECTOR *)D_8009EAB8, D_800D86E0);
            sGame.drawZ = 0;
            sGame.drawX = 0;
        case 0x22:
            if (++state->unk376 == 1) {
                PlaySe(0x50);
            }
        case 0x7:
            if (state->animResult == 0) {
                goto reset;
            }
            break;
        case 0x2E:
            state->speed -= 2;
            if (state->speed <= 0) {
                ResetSpeed();
            }
            if (state->animResult == 0) {
                state->motion = 0x2A;
            }
            break;
        case 0x2F:
            if (state->animResult == 0) {
                r = LandPlayer(state);
                if (r != 0)
                    if (r >= 0)
                        if (r < 3) {
                            state->motion = 0x2A;
                        }
            }
            break;
        case 0x1F:
        case 0x26:
            if (++state->unk376 == 1) {
                PlaySe(0x50);
            }
            if (state->animResult == 0 && ++state->unk374 >= 16) {
                goto reset;
            }

            break;
        case 0x30:
            switch (state->action) {
                case 0x51:
                    sGame.unk362 = state->unk376 = Lerp(state->unk376, 0x6000, state->unk374, 30);
                    sGame.drawX = Lerp(0, gView[0], state->unk378, 15);
                    sGame.drawY = Lerp(0, -50, state->unk378, 15);
                    sGame.drawZ = Lerp(0, gView[2], state->unk378, 15);
                    state->unk374++;
                    if (++state->unk378 >= 14) {
                        StartRumble(gRumblePatterns, 5);
                        state->motion = 0x28;
                        state->unk37A = 0;
                        state->unk378 = 0;
                        state->unk376 = 0;
                        state->unk374 = 0;
                    }
                    break;
                case 0x4F:
                    if (++state->unk378 >= 41) {
                        state->unk378 = 60;
                        state->motion = 0x31;
                        state->unk374 = 0;
                    }
                    sGame.unk362 = state->unk376 = Lerp(state->unk376, 0x5800, state->unk378, 60);
                    break;
            }
            D_8009EAB8[1] = sGame.unk362;
            D_800D86E0[0].coord.t[0] = sGame.drawX;
            D_800D86E0[0].coord.t[1] = sGame.y + sGame.drawY;
            D_800D86E0[0].coord.t[2] = sGame.drawZ;
            SetCoordRotation((SVECTOR *)D_8009EAB8, D_800D86E0);
            break;
        case 0x31:
            if (state->animResult == 0) {
                StartRumble(gRumblePatterns, 6);
                state->unk374 = 0;
                state->motion = 0x21;
            }
            break;
        case 0x32:
            gMissState = -1;
            if (++state->unk374 >= 91) {
                gMissState = 100;
            }
            break;
        case 0x33:
        case 0x34:
        case 0x35:
            if ((u32)((D_8009EF48[0] & 0xF) - 2) < 2 && state->animResult == 0) {
                state->unk8 = 0xFF;
            }
            break;
    }
}

/* MATCHING: non-void with no return keeps two bnez delay slots nops. */
s32 UpdateRunSpeed(void) {
    s16 v;

    if (sGame.control != 0 && (u32)(sGame.motion - 8) >= 4) {
        if (sGame.action != 0x3A) {
            v = sGame.speed;
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
            sGame.speed = (s16)gSpeedFixed >> 4;
            if (sGame.speedBand == 2 && sGame.animResult == 2) {
                sGame.motion = 4;
            }
            if (sGame.speedBand == 1 && sGame.animResult == 2) {
                sGame.motion = 3;
            }
            if (sGame.speedBand == 0 && sGame.animResult == 2) {
                sGame.motion = 2;
            }
        } else {
            gSpeedFixed = 0x460;
            sGame.speed = 0x46;
        }
    }
}

/* MATCHING: non-void with no return keeps li 0x71 first in the call block. */
s32 UpdateSlideDash(void) {
    s32 btn;

    if (sGame.control != 0) {
        switch ((s8)sGame.slideState) {
            case 0:
                sGame.slideState = 0;
                sGame.unk38C = 0;
                if ((u32)(sGame.motion - 2) < 3 && sGame.y >= sGame.groundY && (gGamePadTrig & PADRdown)) {
                    btn = gPad;
                    if (btn & PADLdown) {
                        sGame.slideState = 1;
                        sGame.unk38C = 0;
                        sGame.action = 0x3B;
                    }
                    if (btn & PADLup) {
                        if ((u32)((u16)sGame.speed - 0x26) < 5) {
                            sGame.slideState = 2;
                            sGame.action = 0x39;
                            PlaySe(0x71);
                            sGame.unk38C = 0;
                        }
                    }
                }
                break;
            case 1:
                func_80028500();
                break;
            case 2:
                if (sGame.action == 0) {
                    sGame.slideState = 0;
                    sGame.unk38C = 0;
                }
                break;
        }
    }
}

/* MATCHING: as UpdateJumpMode3 for the switch; the volatile read keeps the unk348 and
 * unk350 loads below the stores to D_800956E0, D_800DF5A0[1] and D_800956E4. */
void UpdateJump(void) {
    s16 n;

    if (sGame.control != 0) {
        if ((gGamePadTrig & PADRright) &&
            (sGame.y >= sGame.groundY || (sGame.unk3B8 == 1 && sGame.unk3BC == 1)) &&
            sGame.jumpState == 0) {
            D_800956E0 = 0;
            D_800DF5A0[1] = sGame.y;
            D_800956E4 = sGame.y;
            D_800DF5A0[0] = *(volatile s32 *)&sGame.x;
            D_800DF5A0[2] = sGame.z;
            sGame.unk38F = 1;
            sGame.unk3BC = 0;
            sGame.jumpState = 1;
            sGame.jumpTime = 0;
            sGame.unk39C = 0;
            func_80018CA4(&sGame);
            sGame.motion = 8;
            sGame.jumpPower = 0x23;
            sGame.jumpStartY = sGame.y;
            ScatterEffect4((VECTOR *)&sGame.x);
            PlaySe(0x17);
        }
        if (sGame.jumpState == 1) {
            n = ++sGame.jumpTime;
            if (n < 3) {
                sGame.jumpPower = 0x19;
            } else if ((u16)(n - 4) < 3) {
                sGame.jumpPower = 0x12;
            } else if (n >= 9) {
                sGame.jumpPower = 0x14;
                sGame.jumpState = 2;
                sGame.motion = 9;
                sGame.unk3A0 = 2;
            } else if (gGamePad & PADRright) {
                sGame.jumpPower = 0x14;
            } else {
                sGame.jumpState = 2;
                switch (n) {
                    case 3:
                        sGame.motion = 0xB;
                        sGame.unk3A0 = 0;
                        break;
                    case 7:
                        sGame.motion = 0xA;
                        sGame.unk3A0 = 1;
                        break;
                    default:
                        sGame.motion = 9;
                        sGame.unk3A0 = 2;
                        break;
                }
            }
        }
        if (sGame.jumpState == 2) {
            sGame.jumpState = 0;
        }
    }
}

/* MATCHING: non-void with no return keeps the last branch's delay slot a nop. */
s32 SetPlayerView(void) {
    gRefView[0] = 0;
    gRefView[1] = rsin((gViewerAngle << 12) / 360) * 500 / 4096 + sGame.groundY;
    gRefView[2] = rcos((gViewerAngle << 12) / 360) * 600 / 4096;
    gRefView[4] = sGame.y;
    gRefView[0] = rsin(D_800A7680->vy) * 400 / 4096;
    gRefView[2] = rcos(D_800A7680->vy) * 400 / 4096;
    gRefView[3] = rsin(D_800A7680->vy - 0x800) * 900 / 4096;
    gRefView[5] = rcos(D_800A7680->vy - 0x800) * 900 / 4096;
    if (D_8009587E < sGame.speed) {
        D_8009587E++;
    }
    if (D_8009587E > sGame.speed) {
        D_8009587E--;
    }
}

/* MATCHING: the index-first integer sum gives retail's addu operand order. */
void UpdateStageLights(void) {
    u8 *c;

    c = (u8 *)((((u32)D_80095864 >> 12) & 3) * 4 + (s32)D_80095790);
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
        case -2:
            break;
        case -1:
            DrawHud();
            break;
        case 0:
            x = gBonusLevel;
            if (D_800958AC == 1) {
                if (x >= 2) {
                    gBonusLevel = 2;
                }
            } else if (x >= 3) {
                gBonusLevel = 3;
            }
            if (gBonusLevel <= 0) {
                gBonusLevel = 0;
            }
            if (D_8009EF4D[0] == 1) {
                gBonusLevel = 0;
            }
            if (gBonusLevel != 0 && D_8009EF4D[0] != 1 && (gStage != 4 || gSection != 0)) {
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
            sGame.control = 0;
            sGame.hittable = 0;
            sGame.invulnTimer = 500;
            sGame.slideState = 0;
            sGame.unk38C = 0;
            if (sGame.rideMode & 0xF) {
                sGame.action = 0;
                sGame.rideMode &= 0xF0;
                sGame.unk3D1 = 1;
                sGame.jumpPower = 0;
                sGame.jumpState = 0;
            }
            if (D_800CF080[196].unk0 == 1) {
                break;
            }
            if (D_800CF080[198].unk0 == 1) {
                break;
            }
            if (sGame.action == 0 || sGame.action == 0x3A || sGame.action == 0x3C) {
                if ((s16)gSpeedFixed >> 4 < 15) {
                    gSpeedFixed = 0xF0;
                }
                sGame.action = 0x23;
                gMissState = 1;
                D_800959B0 = 0;
                gMissTimer = 0;
                sGame.speed = (s16)gSpeedFixed >> 4;
                SetSoundFade(4);
                goto top;
            }
            break;
        case 1:
            gMissTimer++;
            DrawFrozenFrame();
            if (gMissTimer == 10) {
                for (i = 0; i < 200; i++) {
                    D_800CF080[i].unk0 = 2;
                }
                D_800958EC = 0;
                D_800957D6 = 0;
                *(Words3 *)D_800DF5C0 = *(Words3 *)D_8009EEC0;
            }
            if (gMissTimer >= 10) {
                sGame.speed = 0;
                gSpeedFixed = 0;
                *(Words3 *)&sGame.x = *(Words3 *)D_800DF5C0;
            }
            if (gMissTimer > 100) {
                t = gMissTimer - 100;
                if (t > 60) {
                    gMissState = 100;
                }
            }
            break;
        case 2:
            StopCdTrack();
            PlaySe(0x1B);
            gMissTimer = 0;
            gMissState = 3;
            break;
        case 3:
            t = ++gMissTimer;
            if (t > 60) {
                gMissState = 100;
            }
            break;
        case 100:
            ClipF = 0;
            if (gRetries == 0) {
                D_8009EF4A[0] = 0;
                gGameMode = GM_MENU_LOAD;
                gModeStep = 0xFFFF;
            } else {
                RestartStage();
                gRetries--;
            }
            break;
        case 110:
            t = ++gMissTimer;
            func_8003E360(t);
            if (t == 0x180) {
                gGameMode = GM_MENU_LOAD;
            }
            break;
    }
end:;
}

void DrawModeObject(void) {
    switch (sGame.rideMode & 0xF) {
        case 1:
            D_800D3860[0].unk18 = -0x88;
            D_800D3860[0].unk1A = 0xAA;
            D_800D3860[0].unk4 = 0;
            D_800D8370[0].unk58 = (s32)&D_800D86E0[16];
            D_800D3860[0].unk0 = -gWorldShift[0];
            D_800D3860[0].unk8 = -gWorldShift[2];
            DrawObjectModel((Rec78 *)&D_800D8370[D_800D3860[0].unk36], (Rec48 *)&D_800D3860[0]);
            break;
        case 4:
            D_800D3860[5].unk18 = -0x88;
            D_800D3860[5].unk1C = -0x88;
            D_800D3860[5].unk1A = 0xAA;
            D_800D3860[5].unk4 = 0;
            D_800D8370[5].unk58 = (s32)&D_800D86E0[16];
            D_800D3860[5].unk0 = -gWorldShift[0];
            D_800D3860[5].unk8 = -gWorldShift[2];
            DrawObjectModel((Rec78 *)&D_800D8370[D_800D3860[5].unk36], (Rec48 *)&D_800D3860[5]);
            break;
        case 2:
            switch (gStage) {
                case 3:
                    D_800D3860[3].unk4 = 0x3C;
                    D_800D8370[3].unk58 = (s32)&D_800D86E0[0];
                    D_800D3860[3].unk0 = -gWorldShift[0];
                    D_800D3860[3].unk8 = -gWorldShift[2];
                    D_800D3860[3].unk18 += ANGLE_DEG(sGame.speed / 2);
                    DrawObjectModel((Rec78 *)&D_800D8370[D_800D3860[3].unk36], (Rec48 *)&D_800D3860[3]);
                    if (gFrameCount % 20 == 0) {
                        PlaySe(0x81);
                    }
                    break;
                case 10:
                    D_800D3860[6].unk4 = 0x32;
                    D_800D8370[6].unk58 = (s32)&D_800D86E0[0];
                    D_800D3860[6].unk0 = -gWorldShift[0];
                    D_800D3860[6].unk8 = -gWorldShift[2];
                    D_800D3860[6].unk18 += ANGLE_DEG(sGame.speed / 2);
                    DrawObjectModel((Rec78 *)&D_800D8370[D_800D3860[6].unk36], (Rec48 *)&D_800D3860[6]);
                    if (gFrameCount % 20 == 0) {
                        PlaySe(0x81);
                    }
                    break;
            }
            break;
        case 3:
            switch (gStage) {
                case 0:
                    D_800D3860[4].unk4 = 0;
                    D_800D8370[4].unk58 = (s32)&D_800D86E0[1];
                    D_800D3860[4].unk0 = -gWorldShift[0];
                    D_800D3860[4].unk8 = -gWorldShift[2];
                    DrawObjectModel((Rec78 *)&D_800D8370[D_800D3860[4].unk36], (Rec48 *)&D_800D3860[4]);
                    break;
                case 12:
                    D_800D3860[2].unk4 = 0;
                    D_800D8370[2].unk58 = (s32)&D_800D86E0[1];
                    D_800D3860[2].unk0 = -gWorldShift[0];
                    D_800D3860[2].unk8 = -gWorldShift[2];
                    DrawObjectModel((Rec78 *)&D_800D8370[D_800D3860[2].unk36], (Rec48 *)&D_800D3860[2]);
                    break;
            }
            break;
    }
    if ((D_8009EF48[0] >> 4) == 5) {
        D_800D3860[1].unk4 = 0;
        D_800D3860[1].unk0 = -gWorldShift[0];
        D_800D3860[1].unk8 = -gWorldShift[2];
        DrawObjectModel((Rec78 *)&D_800D8370[D_800D3860[1].unk36], (Rec48 *)&D_800D3860[1]);
    }
}

/* MATCHING: non-void with no return makes the guards fill their delay slots with the lui. */
s32 AttachModeObject(void) {
    switch (sGame.rideMode & 0xF) {
        case 1:
            if ((u32)(sGame.motion - 0x33) >= 7) {
                sGame.motion = 0x33;
                D_800D83C8[0] = (s32)&D_800D86E0[16];
                D_800D3896[0] = 0;
            }
            break;
        case 4:
            if ((u32)(sGame.motion - 0x33) >= 7) {
                sGame.motion = 0x33;
                D_800D8620[0] = (s32)&D_800D86E0[16];
                D_800D39FE[0] = 5;
            }
            break;
        case 2:
            if ((u32)(sGame.motion - 0x33) >= 3) {
                sGame.motion = 0x33;
                switch (gStage) {
                    case 3:
                        D_800D8530[0] = (s32)&D_800D86E0[0];
                        D_800D396E[0] = 3;
                        break;
                    case 10:
                        D_800D8698[0] = (s32)&D_800D86E0[0];
                        D_800D3A46[0] = 6;
                        break;
                }
            }
            break;
        case 3:
            switch (gStage) {
                case 0:
                    D_800D85A8[0] = (s32)&D_800D86E0[1];
                    D_800D39B6[0] = 4;
                    break;
                case 12:
                    D_800D84B8[0] = (s32)&D_800D86E0[1];
                    D_800D3926[0] = 2;
                    break;
            }
            break;
    }
}

static __inline__ s16 findFreeSlot(SlotRec *slot) {
    u32 i;

    for (i = 0; i < D_800959B4 / sizeof(SlotRec); i++) {
        if (slot->unk36 == -1) {
            return i;
        }
        slot++;
    }
    return -1;
}

void SpawnModeObject(void) {
    SlotRec *slot;
    s16 idx;

    slot = (SlotRec *)D_800959B8;
    if (D_8009EF49[0] == 1) {
        idx = findFreeSlot(slot);
        if (idx != -1) {
            slot += idx;
            switch (gStage) {
                case 0:
                    D_800D85A8[0] = 0;
                    slot->unk36 = 4;
                    break;
                case 1:
                    D_800D83C8[0] = 0;
                    slot->unk36 = 0;
                    break;
                case 3:
                    D_800D8530[0] = 0;
                    slot->unk36 = 3;
                    break;
                case 10:
                    D_800D8698[0] = 0;
                    slot->unk36 = 6;
                    break;
                case 12:
                    D_800D84B8[0] = 0;
                    slot->unk36 = 2;
                    break;
                case 13:
                    D_800D8620[0] = 0;
                    slot->unk36 = 5;
                    break;
            }
            slot->unk34 = 0xBA;
            slot->unk42 = 0xFF;
            slot->unk43 = 0x69;
            slot->unk26 = 0;
            slot->unk24 = 0;
            D_800D3358[0] = 1;
        }
        D_8009EF49[0] = 0;
    }
}

/* MATCHING: PushPlayer's body as an inline helper; calling it with the s16 step
 * keeps the step's sign extension ahead of the rsin call. */
void SteerMode2(void) {
    s16 step;
    s32 v;
    s32 x;
    GameState *g;
    s32 hi;

    if (sGame.control != 0) {
        if (gPad & PADLleft) {
            ApplySteer(3, 16, 5);
            D_800959B2 -= 4;
        } else if (gPad & PADLright) {
            ApplySteer(2, 16, 5);
            D_800959B2 += 4;
        } else {
            sGame.action = 0x58;
            if (D_800959B2 != 0) {
                if (D_800959B2 < 0) {
                    D_800959B2 += 2;
                } else {
                    D_800959B2 -= 2;
                }
            }
        }
        switch (gStage) {
            case 3:
                sGame.y = sGame.groundY - 0x5A;
                break;
            case 10:
                sGame.y = sGame.groundY - 0x50;
                break;
        }
        D_800959B2 = D_800959B2 < -10 ? -10 : D_800959B2 > 10 ? 10 : D_800959B2;
        step = D_800959B2 / 10;
        pushBack(90, step);
        /* MATCHING: as in SteerMode3. */
        __asm__("");
        g = &sGame;
        x = g->yaw;
        v = D_800A7680->vy - 0x71;
        if (x >= v) {
            hi = D_800A7680->vy + 0x71;
            if (x <= hi) {
                v = x;
            } else {
                v = hi;
            }
        }
        g->yaw = v;
    }
}

/* MATCHING: the unsigned range test is the single subtract-and-compare retail does. */
void UpdateSpeedMode2(void) {
    s16 v;

    if (sGame.control != 0 && (u32)(sGame.motion - 0x33) < 3) {
        v = sGame.speed;
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
        D_8009EF20[0] = (s16)gSpeedFixed >> 4;
    }
}

void UpdateSpeedMode3(void) {
    s16 v;

    if (sGame.control != 0 && sGame.motion == 0x33) {
        v = sGame.speed;
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
        D_8009EF20[0] = (s16)gSpeedFixed >> 4;
    }
}

/* MATCHING: the switch lays the default arm out last, after its own base reload, as retail. */
void UpdateJumpMode3(void) {
    s16 n;

    if (sGame.control != 0) {
        if ((gGamePadTrig & PADRright) &&
            (sGame.y >= sGame.groundY || (sGame.unk3B8 == 1 && sGame.unk3BC == 1)) &&
            sGame.jumpState == 0) {
            sGame.unk38F = 1;
            sGame.unk3BC = 0;
            sGame.jumpState = 1;
            sGame.jumpTime = 0;
            sGame.unk39C = 0;
            func_80018CA4(&sGame);
            sGame.motion = 8;
            sGame.jumpPower = 0x23;
            sGame.jumpStartY = sGame.y;
            ScatterEffect4((VECTOR *)&sGame.x);
            PlaySe(0x17);
        }
        if (sGame.jumpState == 1) {
            n = ++sGame.jumpTime;
            if (n < 3) {
                sGame.jumpPower = 0x19;
            } else if ((u16)(n - 4) < 3) {
                sGame.jumpPower = 0x12;
            } else if (n >= 9) {
                sGame.jumpPower = 0x14;
                sGame.jumpState = 2;
                sGame.motion = 8;
                sGame.unk3A0 = 2;
            } else if (gGamePad & PADRright) {
                sGame.jumpPower = 0x14;
            } else {
                sGame.jumpState = 2;
                switch (n) {
                    case 3:
                        sGame.motion = 9;
                        sGame.unk3A0 = 0;
                        break;
                    case 7:
                        sGame.motion = 9;
                        sGame.unk3A0 = 1;
                        break;
                    default:
                        sGame.motion = 9;
                        sGame.unk3A0 = 2;
                        break;
                }
            }
        }
        if (sGame.jumpState == 2) {
            sGame.jumpState = 0;
        }
    }
}

void SteerMode3(void) {
    s16 step;
    s32 v;
    s32 x;
    GameState *g;
    s32 hi;

    if (sGame.control != 0) {
        if (gPad & PADLright) {
            ApplySteer(3, 16, 8);
            D_800959B2 -= 4;
        } else if (gPad & PADLleft) {
            ApplySteer(2, 16, 8);
            D_800959B2 += 4;
        } else if (sGame.y >= sGame.groundY) {
            sGame.action = 0x5E;
            if (D_800959B2 != 0) {
                if (D_800959B2 < 0) {
                    D_800959B2 += 2;
                } else {
                    D_800959B2 -= 2;
                }
            }
        }
        D_800959B2 = D_800959B2 < -7 ? -7 : D_800959B2 > 7 ? 7 : D_800959B2;
        step = D_800959B2 / 10;
        pushBack(90, step);
        /* MATCHING: the barrier keeps the yaw and unk380 loads below the unk350 store;
         * g keeps the sGame base in a register for the store at the join. */
        __asm__("");
        g = &sGame;
        x = g->yaw;
        v = D_800A7680->vy - 0x71;
        if (x >= v) {
            hi = D_800A7680->vy + 0x71;
            if (x <= hi) {
                v = x;
            } else {
                v = hi;
            }
        }
        g->yaw = v;
    }
}

s32 LandPlayer(GameState *g) {
    s32 ret;

    ret = 0;
    if (g->unk39C <= g->unk3CC && g->unk3B8 == 1) {
        if (g->y >= D_8009EF44[0]) {
            g->y = D_8009EF44[0];
            ret = 2;
            PlaySe(30);
        }
    } else if (g->y >= g->groundY) {
        g->y = g->groundY;
        ret = 1;
        PlaySe(30);
    }
    if (ret != 0) {
        StartRumble(gRumblePatterns, 6);
    }
    return ret;
}

s32 AddBonusLevel(s32 n) {
    gBonusLevel += n;
    if (D_800958AC == 1) {
        if (gBonusLevel >= 2) {
            gBonusLevel = 2;
        }
    } else if (gBonusLevel >= 3) {
        gBonusLevel = 3;
    }
    if (gBonusLevel <= 0) {
        gBonusLevel = 0;
    }
    if (D_8009EF4D[0] == 1) {
        gBonusLevel = 0;
    }
    return gBonusLevel;
}

/* MATCHING: the local pointer lets angle and the sGame base share $s0. */
void PushPlayer(s32 deg, s32 dist) {
    s32 angle;
    SVECTOR *rot;

    rot = D_800A7680;
    angle = ANGLE_DEG(deg);
    sGame.x -= rsin(rot->vy + angle) * dist >> FIX12_SHIFT;
    sGame.z -= rcos(rot->vy + angle) * dist >> FIX12_SHIFT;
}

/* MATCHING: non-void with no return keeps the first delay slot a nop. */
s32 UpdateSpeedBand(void) {
    s16 v;

    v = sGame.speed;
    if (v < 13) {
        sGame.speedBand = 0;
    } else if (v >= 26) {
        sGame.speedBand = 2;
    } else {
        sGame.speedBand = 1;
    }
}

void ResetPlayerAction(void) {
    sGame.motion = 2;
    sGame.action = 0;
    sGame.control = 1;
    sGame.slideState = 0;
    if (D_8009599C == 1) {
        sGame.hittable = 0;
        sGame.invulnTimer = 60;
        D_8009599C = 0;
    }
    if ((sGame.rideMode & 0xF) == 3) {
        sGame.motion = 0x33;
    }
}

/* MATCHING: `one` is the 1 retail keeps in $a0 for both compares. */
void CheckCanBonus(void) {
    s32 one;

    if (gCans != 0 && gCans % 10 == 0) {
        one = 1;
        if (D_800958AC == one) {
            if (gBonusLevel == 2) {
                goto skip;
            }
        } else if (gBonusLevel == 3) {
            goto skip;
        }
        if (D_8009EF4D[0] != one) {
            PlaySe(34);
        }
    skip:
        AddBonusLevel(1);
    }
}

void func_800284F8(void) {}

void func_80028500(void) {}

/* MATCHING: non-void with no return keeps the second bnez's delay slot a nop. */
s32 TrySlide(void) {
    if (sGame.control != 0 && (sGame.motion < 5 || sGame.motion >= 8) &&
        (sGame.unk38F != 1 || sGame.jumpPower < 6) && !(gPad & PADLup) && (gGamePadTrig & PADRdown)) {
        PlaySe(22);
        sGame.motion = 5;
    }
}

void EndJump(void) {
    sGame.jumpPower = 0;
    sGame.jumpState = 2;
}

void PolarOffset(s32 deg, s32 radius, VECTOR *out) {
    s32 angle;

    angle = ANGLE_DEG(deg);
    out->vx = rsin(angle) * radius >> FIX12_SHIFT;
    out->vz = rcos(angle) * radius >> FIX12_SHIFT;
}

void SetStageFog(void) {
    s32 i;

    i = ((u32)D_80095864 >> 8) & 3;
    SetFogNearFar(3000, 8000, 250);
    SetFarColor(gFogColors[i * 4], gFogColors[i * 4 + 1], gFogColors[i * 4 + 2]);
}

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

void ResetSpeed(void) {
    D_8009EF20[0] = 0;
    gSpeedFixed = 0x280;
}

void StopSpeed(void) {
    D_8009EF20[0] = 0;
    gSpeedFixed = 0;
}

void AttachMode5Object(void) {
    if ((D_8009EF48[0] >> 4) == 5) {
        D_800D8440[0] = (s32)D_800D8960;
        D_800D38DE[0] = 1;
    }
}

INCLUDE_RODATA("asm/nonmatchings/player", D_80010950);

INCLUDE_RODATA("asm/nonmatchings/player", D_80010AE0);

INCLUDE_RODATA("asm/nonmatchings/player", D_80010B10);
