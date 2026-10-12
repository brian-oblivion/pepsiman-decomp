#include "common.h"
#include "libetc.h"
#include "libgte.h"
#include "libgpu.h"
#include "libgs.h"
#include "draw.h"
#include "model.h"
#include "player.h"
#include "object.h"

u16 func_800FAD08(s16 id, s32 a, s32 b);
u16 func_800FAD04(s16 id, s32 a, s32 b);
u16 func_800FA894(s16 id, s32 a, s32 b);
u16 func_800FA580(s16 id, s32 a, s32 b);
u16 func_800F1A48(s16 id, s32 a, s32 b);
u16 func_800F9ABC(s16 id, s32 a, s32 b);
u16 func_800F9CA4(s16 id, s32 a, s32 b);
u16 func_800FA060(s16 id, s32 a, s32 b);
u16 func_800F922C(s16 id, s32 a, s32 b);

s32 func_800FA6C8(void);
s32 func_800FA804(void);
s32 func_800F6CE4(void);
s32 func_800FA3EC(void);
s32 func_800FA1C0(void);
s32 func_800F6AC8(void);
s32 func_800F1080(void);
s32 func_800F64B0(void);
s32 func_800F9618(void);
s32 func_800F98E4(void);
s32 func_800F475C(void);
s32 func_800F9BC0(void);
s32 func_800F8D34(void);
s32 func_800F3800(void);

void func_800FA5D0(void);

/** @brief A position as three words. */
typedef struct {
    s32 vx; /**< x */
    s32 vy; /**< y */
    s32 vz; /**< z */
} Pos3;

/** @brief object's Trap with the halfword this unit clears. */
typedef struct {
    u8 unk0[0x26];  /**< not yet known */
    s16 unk26;      /**< cleared when a stage starts */
    u8 unk28[0x20]; /**< not yet known */
} TrapClear;

/* The Trap table; common.h declares it as words. */
#define sTraps ((TrapClear *)gTraps)

/* MATCHING: stored as u8 here; player tests it as s8 (lb). */
extern u8 gSkipLandRumble; /**< 1 after a start or restart: the next landing does not rumble */

extern s8 sStageFindsFloor[]; /**< 1 for a stage whose start looks up the floor */

/** @brief The record the start heading is read from. */
typedef struct {
    u8 unk0[6]; /**< not yet known */
    s16 unk6;   /**< the start heading, negated */
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

extern s32 D_800957A4;
extern u16 sPauseFrames; /**< frames shown paused */

void func_800F00A0(void);
void func_800F1654(void);
void func_800F17B0(void);
void func_800F42E8(void);
void func_800F4758(void);
void func_800F4794(void);
void func_800F5E80(void);
void func_800F72B8(void);
void func_800F7724(void);
void func_800F7844(void);
void func_800F7A9C(void);
void func_800F7F40(void);
void func_800F809C(void);
void func_800F8264(void);
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
                        func_800F809C();
                        break;
                    case STAGE_1_2:
                        func_800F7A9C();
                        break;
                    case STAGE_1_3:
                        func_800F4794();
                        break;
                    case STAGE_2_1:
                        func_800F8264();
                        break;
                    case STAGE_2_2:
                        func_800F7F40();
                        break;
                    case STAGE_2_3:
                        func_800F4758();
                        break;
                    case STAGE_3_1:
                        func_800F00A0();
                        break;
                    case STAGE_3_2:
                        /* MATCHING: keeps cross-jumping from merging this arm with case 6. */
                        func_800F00A0();
                        __asm__("");
                        break;
                    case STAGE_3_3:
                        func_800F42E8();
                        break;
                    case STAGE_4_1:
                        func_800F72B8();
                        break;
                    case STAGE_4_2:
                        func_800F7844();
                        break;
                    case STAGE_4_3:
                        func_800F1654();
                        break;
                    case STAGE_5_1:
                        func_800F7724();
                        break;
                    case STAGE_5_2:
                        func_800F5E80();
                        break;
                    case STAGE_5_3:
                        func_800F17B0();
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
    yaw = -gStageStart->unk6;
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
        sTraps[i].unk26 = 0;
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
    SetCurrentBlock((BlockHeader *)0x801FD000);
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
    gPlayer.unk38C = 0;
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
            ret = func_800FA6C8();
            break;
        case STAGE_1_2:
            ret = func_800FA804();
            break;
        case STAGE_1_3:
            ret = func_800F6CE4();
            break;
        case STAGE_2_1:
            ret = func_800FA3EC();
            break;
        case STAGE_2_2:
            ret = func_800FA1C0();
            break;
        case STAGE_2_3:
            ret = func_800F6AC8();
            break;
        case STAGE_3_1:
            ret = func_800F1080();
            break;
        case STAGE_3_2:
            ret = func_800F1080();
            break;
        case STAGE_3_3:
            ret = func_800F64B0();
            break;
        case STAGE_4_1:
            ret = func_800F9618();
            break;
        case STAGE_4_2:
            ret = func_800F98E4();
            break;
        case STAGE_4_3:
            ret = func_800F475C();
            break;
        case STAGE_5_1:
            ret = func_800F9BC0();
            break;
        case STAGE_5_2:
            ret = func_800F8D34();
            break;
        case STAGE_5_3:
            ret = func_800F3800();
            break;
    }
    return ret;
}

/* MATCHING: a and b go through to every callee untouched; they keep
 * $a1 and $a2 live, which puts ret in $a3. */
u16 CallStageHook(s16 id, s32 a, s32 b) {
    u16 ret = 1;

    switch (gStage) {
        case STAGE_1_1:
            ret = func_800FAD08(id, a, b);
            break;
        case STAGE_1_2:
            ret = func_800FAD04(id, a, b);
            break;
        case STAGE_2_1:
            ret = func_800FA894(id, a, b);
            break;
        case STAGE_2_2:
            ret = func_800FA580(id, a, b);
            break;
        case STAGE_3_1:
            ret = func_800F1A48(id, a, b);
            break;
        case STAGE_3_2:
            ret = func_800F1A48(id, a, b);
            break;
        case STAGE_4_1:
            ret = func_800F9ABC(id, a, b);
            break;
        case STAGE_4_2:
            ret = func_800F9CA4(id, a, b);
            break;
        case STAGE_5_1:
            ret = func_800FA060(id, a, b);
            break;
        case STAGE_5_2:
            ret = func_800F922C(id, a, b);
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
                func_800FA5D0();
            }
        }
    }
}
