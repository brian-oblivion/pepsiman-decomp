#include "common.h"
#include "ram.h"
#include "sbss.h"
#include "libetc.h"
#include "libgte.h"
#include "libgpu.h"
#include "libgs.h"
#include "libcd.h"
#include "draw.h"
#include "model.h"
#include "player.h"
#include "object.h"

/* The stage overlays' entry points. Each stage's overlay has its code at its
 * own addresses, and the name here is the overlay's name for the function
 * plus the stage: PlaceTrap1_2 is ovl04's PlaceTrap. STAGE_3_1 and
 * STAGE_3_2 load the same code (stage3.inc), so theirs carry the world
 * alone. */
u16 PlaceTrap1_1(s16 id, s32 a, s32 b);
u16 PlaceTrap1_2(s16 id, s32 a, s32 b);
u16 PlaceTrap2_1(s16 id, s32 a, s32 b);
u16 PlaceTrap2_2(s16 id, s32 a, s32 b);
u16 PlaceTrap3(s16 id, s32 a, s32 b);
u16 PlaceTrap4_1(s16 id, s32 a, s32 b);
u16 PlaceTrap4_2(s16 id, s32 a, s32 b);
u16 PlaceTrap5_1(s16 id, s32 a, s32 b);
u16 PlaceTrap5_2(s16 id, s32 a, s32 b);

s32 LoadStageFiles1_1(void);
s32 LoadStageFiles1_2(void);
s32 LoadChaseStage1_3(void);
s32 LoadStageFiles2_1(void);
s32 LoadStageFiles2_2(void);
s32 LoadChaseStage2_3(void);
s32 LoadStageFiles3(void);
s32 LoadChaseStage3_3(void);
s32 LoadStageFiles4_1(void);
s32 LoadStageFiles4_2(void);
s32 LoadChaseStage4_3(void);
s32 LoadStageFiles5_1(void);
s32 LoadStageFiles5_2(void);
s32 LoadChaseStage5_3(void);

void ShowStartCanGroups1_1(void);

/** @brief A position as three words. */
typedef struct {
    s32 vx; /**< x */
    s32 vy; /**< y */
    s32 vz; /**< z */
} Pos3;

/** @brief object's Trap with the halfword this unit clears. */
typedef struct {
    u8 unk0[0x26];  /**< not yet known */
    s16 state;      /**< cleared when a stage starts */
    u8 unk28[0x20]; /**< not yet known */
} TrapClear;

/* The Trap table; common.h declares it as words. */
#define sTraps ((TrapClear *)gTraps)

/* MATCHING: stored as u8 here; player tests it as s8 (lb). */
extern u8 gSkipLandRumble; /**< 1 after a start or restart: the next landing does not rumble */

/* The disc file table's 25th copy (disc_files.inc), which no code here
 * reads. */
#define DISC_FILE(x) sFiles25##x
#include "disc_files.inc"

/** 1 for a stage whose start looks up the floor, by stage. */
s8 sStageFindsFloor[16] = {1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 0};

/* Three positions per stage, x y z, that the stage overlays' scripted
 * falls move the player to (all 0 for a stage with none): the first when
 * gMissState is -1 (StepMissSequence), the other two in turn when it is
 * -2. Only the overlays read them. */
Vec3 gMissTargets[] = {
    {-5700, -9080, 18500},  /* 0 */
    {17332, -9080, 27535},  /* 1 */
    {0, 0, 0},              /* 2 */
    {-12481, -300, 18310},  /* 3 */
    {0, 0, 0},              /* 4 */
    {0, 0, 0},              /* 5 */
    {0, 0, 0},              /* 6 */
    {0, 0, 0},              /* 7 */
    {0, 0, 0},              /* 8 */
    {0, 0, 0},              /* 9 */
    {20467, -117, 13079},   /* 10 */
    {0, 0, 0},              /* 11 */
    {6471, 0, -10182},      /* 12 */
    {18200, -2000, -26512}, /* 13 */
    {0, 0, 0},              /* 14 */
};

Vec3 gGoalWalkTargets[] = {
    {26696, -9080, -5923},  /* 0 */
    {6385, 0, -23383},      /* 1 */
    {0, 0, 0},              /* 2 */
    {-13000, -300, -24971}, /* 3 */
    {0, 0, 0},              /* 4 */
    {0, 0, 0},              /* 5 */
    {0, 0, 0},              /* 6 */
    {0, 0, 0},              /* 7 */
    {0, 0, 0},              /* 8 */
    {-5799, 0, 3725},       /* 9 */
    {10417, 610, -21361},   /* 10 */
    {0, 0, 0},              /* 11 */
    {19644, 0, 19346},      /* 12 */
    {27178, -2000, 16214},  /* 13 */
    {0, 0, 0},              /* 14 */
};

Vec3 gGoalSlideTargets[] = {
    {26695, -9080, -6120},  /* 0 */
    {6123, 0, -23383},      /* 1 */
    {0, 0, 0},              /* 2 */
    {-13409, -300, -25402}, /* 3 */
    {0, 0, 0},              /* 4 */
    {0, 0, 0},              /* 5 */
    {0, 0, 0},              /* 6 */
    {0, 0, 0},              /* 7 */
    {0, 0, 0},              /* 8 */
    {-5637, 0, 3858},       /* 9 */
    {10794, 610, -21361},   /* 10 */
    {0, 0, 0},              /* 11 */
    {19646, 0, 19607},      /* 12 */
    {27178, -2000, 16480},  /* 13 */
    {0, 0, 0},              /* 14 */
};

/** Each stage's time limit in seconds, by where it starts: [0] from the
 *  beginning, [1] and [2] from a saved start (SavedStart.valid). */
s32 gTimeLimits[][3] = {
    {115, 82, 40},  /* 0 */
    {98, 60, 45},   /* 1 */
    {60, 60, 60},   /* 2 */
    {120, 100, 70}, /* 3 */
    {125, 75, 35},  /* 4 */
    {60, 60, 60},   /* 5 */
    {60, 60, 60},   /* 6 */
    {60, 60, 60},   /* 7 */
    {60, 60, 60},   /* 8 */
    {160, 125, 75}, /* 9 */
    {150, 110, 85}, /* 10 */
    {60, 60, 60},   /* 11 */
    {115, 75, 30},  /* 12 */
    {140, 85, 23},  /* 13 */
    {60, 60, 60},   /* 14 */
};

/** The time limits when gStageTableSet is 1. */
s32 gTimeLimitsAlt[][3] = {
    {100, 65, 30},  /* 0 */
    {85, 50, 40},   /* 1 */
    {60, 60, 60},   /* 2 */
    {105, 80, 60},  /* 3 */
    {110, 65, 25},  /* 4 */
    {60, 60, 60},   /* 5 */
    {60, 60, 60},   /* 6 */
    {60, 60, 60},   /* 7 */
    {60, 60, 60},   /* 8 */
    {140, 110, 65}, /* 9 */
    {130, 95, 75},  /* 10 */
    {60, 60, 60},   /* 11 */
    {100, 60, 23},  /* 12 */
    {125, 80, 21},  /* 13 */
    {60, 60, 60},   /* 14 */
};

/** @brief The record the start heading is read from. */
typedef struct {
    s16 x, y, z; /**< the start position, negated */
    s16 yaw;     /**< the start heading, negated */
} StartRec;

/* MATCHING: a per-unit view; stage stores it as an s32. */
extern StartRec *gStageStart;

/* MATCHING: object hands it a Trap; here it gets a position. */
s32 FindFloor(Pos3 *pos);
void RestoreStageObjects(void);
void ResetStageObjects(void);
void RestoreToolBuffer(void);
void PlayStageMusic(u8 idx);
void ResetStage(void);
void StartStageOverlay(void);
void OnStageRestart(u8 stage);

/** @brief The saved state a stage can resume from. */
typedef struct {
    s32 valid;      /**< non-zero when there is one */
    Pos3 pos;       /**< the position */
    s32 section;    /**< gSection */
    s32 floorIndex; /**< gFloorIndex */
    u16 cameraYaw;  /**< gCameraYaw */
    u16 cans;       /**< gCans */
    u8 pad1C[4];    /**< not yet known */
    s32 viewY;      /**< gView[1], the camera height */
} SavedStart;

/* MATCHING: a struct lvalue keeps the base in a register. */
#define sSaved (*(SavedStart *)gSavedStart)

extern u16 sPauseFrames; /**< frames shown paused */

/* The overlays' frame steps, named as the entry points above. */
void StepStageFrame3(void);
void StepChaseFrame4_3(void);
void StepChaseFrame5_3(void);
void StepChaseFrame3_3(void);
void StepChaseFrame2_3(void);
void StepChaseFrame1_3(void);
void StepStageFrame5_2(void);
void StepStageFrame4_1(void);
void StepStageFrame5_1(void);
void StepStageFrame4_2(void);
void StepStageFrame1_2(void);
void StepStageFrame2_2(void);
void StepStageFrame1_1(void);
void StepStageFrame2_1(void);
void DrawPausedFrame(void);

/* MATCHING: main.c defines it on a RumbleStep; the table is a u16 array
 * here, as in player. */
void StartRumble(u16 *table, s32 index);

void InitEffects(void) {
    s32 i;

    for (i = 0; i < 128; i++) {
        gEffects[i].next = i + 1;
    }
    gEffectFree = 0;
    gEffectActive = 0xFF;
}

/* MATCHING: kind is only stored with sb, yet s32; as u8 it takes the first
 * temporary and shifts every other argument register. */
s32 SpawnEffectWithParams(s32 kind, s16 x, s16 y, s16 z, s16 delay, s16 param0, s16 param1,
                          s16 param2, s16 param3) {
    u8 old;

    if ((s8)gEffectFree < 0) {
        return -1;
    }
    old = gEffectActive;
    gEffectActive = gEffectFree;
    gEffectFree = gEffects[gEffectActive].next;
    gEffects[gEffectActive].next = old;
    gEffects[gEffectActive].kind = kind;
    gEffects[gEffectActive].x = x;
    gEffects[gEffectActive].y = y;
    gEffects[gEffectActive].z = z;
    gEffects[gEffectActive].param0 = param0;
    gEffects[gEffectActive].param1 = param1;
    gEffects[gEffectActive].param2 = param2;
    gEffects[gEffectActive].param3 = param3;
    gEffects[gEffectActive].frame = 0;
    gEffects[gEffectActive].delay = delay;
    return 0;
}

void UpdateStagePlay(u8 stage) {
    RECT r;

    if (gStage % 3 != 2) {
        UpdateActResults();
        DrawScreenWipe();
    }
    switch (gModeStep) {
        case 0:
            gSubStep = 0;
            gModeStep++;
            break;
        case 1:
            if (InitStageOverlay(gStage) == 1) {
                gModeStep++;
            }
            break;
        case 2:
        case 3:
            sSaved.valid = 0;
            gRetries = 50;
            sSaved.section = gSection;
            sSaved.pos.vx = gCourse.x;
            sSaved.pos.vy = gCourse.y;
            sSaved.pos.vz = gCourse.z;
            sSaved.floorIndex = gFloorIndex;
            ResetStage();
            gSkipLandRumble = 1;
            gModeStep = 4;
            break;
        case 4:
            if (gPauseState == 0) {
                SetTmdSortMode(gTmdSortMode);
                UpdatePlayState();
                UpdateRumble();
                switch (stage) {
                    case STAGE_1_1:
                        StepStageFrame1_1();
                        break;
                    case STAGE_1_2:
                        StepStageFrame1_2();
                        break;
                    case STAGE_1_3:
                        StepChaseFrame1_3();
                        break;
                    case STAGE_2_1:
                        StepStageFrame2_1();
                        break;
                    case STAGE_2_2:
                        StepStageFrame2_2();
                        break;
                    case STAGE_2_3:
                        StepChaseFrame2_3();
                        break;
                    case STAGE_3_1:
                        StepStageFrame3();
                        break;
                    case STAGE_3_2:
                        /* MATCHING: keeps cross-jumping from merging this arm with case 6. */
                        StepStageFrame3();
                        __asm__("");
                        break;
                    case STAGE_3_3:
                        StepChaseFrame3_3();
                        break;
                    case STAGE_4_1:
                        StepStageFrame4_1();
                        break;
                    case STAGE_4_2:
                        StepStageFrame4_2();
                        break;
                    case STAGE_4_3:
                        StepChaseFrame4_3();
                        break;
                    case STAGE_5_1:
                        StepStageFrame5_1();
                        break;
                    case STAGE_5_2:
                        StepStageFrame5_2();
                        break;
                    case STAGE_5_3:
                        StepChaseFrame5_3();
                        break;
                }
            } else {
                SetPadVibration(0, 0, 0);
                switch (gPauseState) {
                    case 1:
                        r.x = 0;
                        r.y = gActiveBuff * 240;
                        r.w = 320;
                        r.h = 240;
                        MoveImage(&r, 0x280, 0);
                        sPauseFrames = 0;
                        gPauseState++;
                    case 2:
                        sPauseFrames++;
                        DrawPausedFrame();
                        break;
                }
            }
            break;
    }
    gSkipLandRumble = 0;
    if (gPadTrig & PADstart) {
        if ((u16)(gMissState - MISS_DYING) < 2) { /* MISS_DYING or MISS_NONE */
            if (gPauseState == 0) {
                SetCdVolume(30);
                gPauseState = 1;
            } else {
                if (gPad & PADselect) {
                    StartRumble(gRumblePatterns, RUMBLE_NONE);
                    gGameMode = GM_MENU_LOAD;
                    gModeStep = 0;
                    StopCdTrack();
                    gPlayer.flattened = 0;
                    gPlayer.rideMode = RIDE_NONE;
                }
                SetCdVolume(80);
                gPauseState = 0;
            }
        }
    }
}

void DrawPausedFrame(void) {
    SVECTOR pos;
    CVECTOR color;

    pos.vx = -0x40;
    pos.vy = -0x10;
    color.r = BLEND_ADD;
    color.g = color.b = color.cd = 0x80;
    DrawSprite(SPR_PAUSE, &pos, &color, 0, &gSubOt[gActiveBuff]);
    DefineSprite(SPR_SCREEN_LEFT, 2, 0xA0, 0xF0, 10, 0, 0, 0, 0);
    DefineSprite(SPR_SCREEN_RIGHT, 2, 0xA0, 0xF0, 12, 0x20, 0, 0, 0);
    color.g = color.b = color.cd = 0x40;
    pos.vx = -0xA0;
    pos.vy = -0x78;
    DrawSprite(SPR_SCREEN_LEFT, &pos, &color, 0, &gSubOt[gActiveBuff]);
    pos.vx = 0;
    pos.vy = -0x78;
    DrawSprite(SPR_SCREEN_RIGHT, &pos, &color, 0, &gSubOt[gActiveBuff]);
}

/* MATCHING: an unused 8-byte local gives retail's 0x20 frame; the start
 * position is copied field by field, not as a struct. */
void RestartStage(u8 stage) {
    s32 unused[2];
    s16 yaw;
    u32 i;

    gSkipLandRumble = 1;
    if (stage % 3 == 2) {
        gModeStep = 2;
        return;
    }
    gPlayer.invulnTimer = 0;
    gPlayer.hittable = 1;
    gBonusLevel = 3;
    gPlayer.x = gCourse.x;
    gPlayer.y = gCourse.y;
    gPlayer.z = gCourse.z;
    gMissState = MISS_NONE;
    gPlayer.falling = 0;
    if (sStageFindsFloor[gStage] == 1) {
        gFloorIndex = FindFloor((Pos3 *)&gPlayer.x);
    } else {
        gFloorIndex = 0;
    }
    gSection = 0;
    yaw = -gStageStart->yaw;
    gView[1] = (s16)(gCourse.y - 500);
    gCourse.rot.vy = yaw;
    gPlayer.startYaw = gPlayer.yaw = yaw;
    gSteerYaw = yaw;
    gSteerYawHalf = yaw;
    if (gInEditor != 1) {
        RestoreStageObjects();
    } else {
        RestoreToolBuffer();
    }
    ResetStageObjects();
    OnStageRestart(gStage);
    gPlayer.motion[0] = MOT_RUN_SLOW;
    gPlayer.control = 1;
    gFellOff = 0;
    gPlayLive = 1;
    for (i = 0; i < 200; i++) {
        sTraps[i].state = 0;
    }
    ResetStage();
    StartStageOverlay();
    PlayStageMusic(gStage);
}

/* MATCHING: the table indices go into locals before the final if, so both
 * are loaded once ahead of the branch. */
void ResetStage(void) {
    RECT r;
    s32 mode;
    s32 saved;

    gSkipLandRumble = 1;
    StopCdTrack();
    if (gGameMode != GM_LOAD_SCREEN_MODEL) {
        r.x = 0;
        r.y = PSDIDX * 240;
        r.w = 320;
        r.h = 240;
        MoveImage(&r, 0x280, 0);
    }
    InitEffects();
    gLoading = 0;
    gVSyncMode = 2;
    ResetStageOverlay();
    if (gStageTableSet == 1) {
        gBonusLevel = 2;
    } else {
        gBonusLevel = 3;
    }
    gMissState = MISS_NONE;
    SetCurrentBlock((BlockHeader *)RAM_CUR_BLOCK);
    gWorldIntroStep = 0;
    gPlayer.motion[0] = MOT_RUN;
    gPlayer.falling = 0;
    gPlayer.twoTracks = 0;
    gPlayer.animFilter = 0;
    gPlayer.action = ACT_RUN;
    gPlayer.curMotion[0] = MOT_OFF;
    StepAnimation(&gPlayer, 0, 0);
    gPlayer.speed = 50;
    gPlayer.slideState = SLIDE_NONE;
    gPlayer.slideStateTime = 0;
    gPlayer.flattened = 0;
    gPlayer.drawX = gPlayer.drawY = gPlayer.drawZ = 0;
    gPlayer.bodyRotX = gPlayer.bodyYaw = gPlayer.unk364 = 0;
    gPlayer.dropRide = 0;
    gPlayer.showShadow = 1;
    gPlayer.invulnTimer = 0;
    gPlayer.hittable = 1;
    gHitRecover = 0;
    gPlayer.altModel = gPlayer.bonusLocked = 0;
    gPlayer.fallSpeed = 0;
    gView[6] = 0;
    gPlayer.control = 1;
    gFellOff = 0;
    gPlayLive = 1;
    gSection = 0;
    gPauseState = 0;
    D_800957A4 = 0;
    ClipF = 0;
    gFogFade = 0;
    ResetSpeed();
    if (sSaved.valid != 0) {
        gSection = sSaved.section;
        gPlayer.x = sSaved.pos.vx;
        gPlayer.y = sSaved.pos.vy;
        gPlayer.z = sSaved.pos.vz;
        gFloorIndex = sSaved.floorIndex;
        gCameraYaw = sSaved.cameraYaw;
        gCans = sSaved.cans;
        gView[1] = sSaved.viewY;
    } else if (gStage == STAGE_2_2) {
        gPlayer.action = ACT_BLOWN_AWAY;
        gPlayer.motion[0] = MOT_THROWN_BACK;
        gPlayer.jumpPower = 0;
        gPlayer.speed = 0;
        gSpeedFixed = 0;
        gPlayer.actTimer = gPlayer.actTimer2 = gPlayer.actTimer3 = gPlayer.actTimer4 = 0;
        gPlayer.control = 0;
        gPlayer.y = gCourse.y - 1000;
    }
    mode = gStage;
    saved = sSaved.valid;
    if (gStageTableSet == 1) {
        gTimeLeft = gTimeLimitsAlt[mode][saved] * 30;
    } else {
        gTimeLeft = gTimeLimits[mode][saved] * 30;
    }
    gPlayer.rideMode = RIDE_NONE;
    EndJump();
    StartRumble(gRumblePatterns, RUMBLE_NONE);
}

/* MATCHING: ret has no default; out-of-range numbers return whatever $a1
 * held. */
s16 InitStageOverlay(u8 stage) {
    s32 ret;

    switch (stage) {
        case STAGE_1_1:
            ret = LoadStageFiles1_1();
            break;
        case STAGE_1_2:
            ret = LoadStageFiles1_2();
            break;
        case STAGE_1_3:
            ret = LoadChaseStage1_3();
            break;
        case STAGE_2_1:
            ret = LoadStageFiles2_1();
            break;
        case STAGE_2_2:
            ret = LoadStageFiles2_2();
            break;
        case STAGE_2_3:
            ret = LoadChaseStage2_3();
            break;
        case STAGE_3_1:
            ret = LoadStageFiles3();
            break;
        case STAGE_3_2:
            ret = LoadStageFiles3();
            break;
        case STAGE_3_3:
            ret = LoadChaseStage3_3();
            break;
        case STAGE_4_1:
            ret = LoadStageFiles4_1();
            break;
        case STAGE_4_2:
            ret = LoadStageFiles4_2();
            break;
        case STAGE_4_3:
            ret = LoadChaseStage4_3();
            break;
        case STAGE_5_1:
            ret = LoadStageFiles5_1();
            break;
        case STAGE_5_2:
            ret = LoadStageFiles5_2();
            break;
        case STAGE_5_3:
            ret = LoadChaseStage5_3();
            break;
    }
    return ret;
}

/* MATCHING: a and b go through to every callee untouched; they keep
 * $a1 and $a2 live, which puts ret in $a3. */
u16 PlaceStageTrap(s16 id, s32 a, s32 b) {
    u16 ret = 1;

    switch (gStage) {
        case STAGE_1_1:
            ret = PlaceTrap1_1(id, a, b);
            break;
        case STAGE_1_2:
            ret = PlaceTrap1_2(id, a, b);
            break;
        case STAGE_2_1:
            ret = PlaceTrap2_1(id, a, b);
            break;
        case STAGE_2_2:
            ret = PlaceTrap2_2(id, a, b);
            break;
        case STAGE_3_1:
            ret = PlaceTrap3(id, a, b);
            break;
        case STAGE_3_2:
            ret = PlaceTrap3(id, a, b);
            break;
        case STAGE_4_1:
            ret = PlaceTrap4_1(id, a, b);
            break;
        case STAGE_4_2:
            ret = PlaceTrap4_2(id, a, b);
            break;
        case STAGE_5_1:
            ret = PlaceTrap5_1(id, a, b);
            break;
        case STAGE_5_2:
            ret = PlaceTrap5_2(id, a, b);
            break;
    }
    return ret;
}

void FadeInFog(void) {
    POLY_F4 *p;
    s32 i;

    if (gFogFade != 100) {
        if (gFogFade == 0) {
            p = (POLY_F4 *)GsOUT_PACKET_P;
            setPolyF4(p);
            p->r0 = p->g0 = p->b0 = 0xFF;
            setXY4(p, -160, -120, 160, -120, -160, 120, 160, 120);
            addPrim(gSubOt[gActiveBuff].org, p);
            p++;
            GsOUT_PACKET_P = (u8 *)p;
        }
        i = ((u32)gStageParams >> 8) & 3;
        SetFogNearFar(gFogFade * 3000 * 2048 / 0x10000, gFogFade * 8000 * 2048 / 0x10000, 250);
        SetFarColor(gFogColors[i * 4] * (gFogFade << 11) / 0x10000,
                    gFogColors[i * 4 + 1] * (gFogFade << 11) / 0x10000,
                    gFogColors[i * 4 + 2] * (gFogFade << 11) / 0x10000);
        gFogFade++;
        if (gFogFade == 32) {
            gFogFade = 100;
        }
    }
}

/** @brief Runs the stage overlay's restart hook; only STAGE_1_1 has one. */
void OnStageRestart(u8 stage) {
    s32 mode = stage;

    if (mode != 1) {
        if (mode < 2) {
            if (mode == 0) {
                ShowStartCanGroups1_1();
            }
        }
    }
}
