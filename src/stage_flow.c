#include "common.h"
#include "libgte.h"
#include "libgpu.h"
#include "libgs.h"
#include "draw.h"
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

/** @brief The head of the game state as this unit writes it. */
typedef struct {
    u8 unk0;                  /**< set to 1 when a stage starts */
    u8 pad1;                  /**< not yet known */
    u8 unk2;                  /**< set to 1 when a stage starts */
    u8 unk3;                  /**< cleared on a reset */
    u8 unk4;                  /**< cleared on a reset */
    u8 unk5;                  /**< cleared on a reset; 0x57 in mode 4 */
    u8 unk6;                  /**< 2 when a stage starts, 3 on a reset */
    u8 pad7;                  /**< not yet known */
    u8 unk8;                  /**< 0xFF on a reset */
    u8 pad9[0x348 - 0x9];     /**< not yet known */
    Pos3 unk348;              /**< the start position */
    s32 unk354;               /**< cleared on a reset */
    s32 unk358;               /**< cleared on a reset */
    s32 unk35C;               /**< cleared on a reset */
    s16 unk360;               /**< cleared on a reset */
    s16 unk362;               /**< cleared on a reset */
    s16 unk364;               /**< cleared on a reset */
    u8 pad366[0x374 - 0x366]; /**< not yet known */
    s16 unk374;               /**< cleared in mode 4 */
    s16 unk376;               /**< cleared in mode 4 */
    s16 unk378;               /**< cleared in mode 4 */
    s16 unk37A;               /**< cleared in mode 4 */
    s32 unk37C;               /**< the start heading */
    s32 unk380;               /**< the start heading */
    u8 pad384[0x38C - 0x384]; /**< not yet known */
    s16 unk38C;               /**< cleared on a reset */
    u8 unk38E;                /**< cleared on a reset */
    u8 pad38F[0x398 - 0x38F]; /**< not yet known */
    s16 unk398;               /**< cleared in mode 4 */
    u8 pad39A[0x3A6 - 0x39A]; /**< not yet known */
    s16 unk3A6;               /**< cleared on a reset */
    s16 unk3A8;               /**< 50 on a reset, 0 in mode 4 */
    u8 pad3AA[0x3C8 - 0x3AA]; /**< not yet known */
    s16 unk3C8;               /**< cleared when a stage starts */
    s16 unk3CA;               /**< cleared when a stage starts */
    u8 pad3CC[0x3D0 - 0x3CC]; /**< not yet known */
    u8 unk3D0;                /**< cleared when leaving through the menu */
    u8 unk3D1;                /**< cleared on a reset */
    u8 unk3D2;                /**< cleared on a reset */
    u8 unk3D3;                /**< 1 on a reset */
    u8 unk3D4;                /**< cleared on a reset */
    u8 unk3D5;                /**< cleared on a reset */
} StageState;

/* MATCHING: a struct lvalue keeps the base in a register. */
#define sStage (*(StageState *)gPlayer)

/** @brief object's Rec48 with the halfword this unit clears. */
typedef struct {
    u8 unk0[0x26];  /**< not yet known */
    s16 unk26;      /**< cleared when a stage starts */
    u8 unk28[0x20]; /**< not yet known */
} Rec48Clear;

/* The Rec48 table; common.h declares it as words. */
#define sRecs48 ((Rec48Clear *)D_800A9008)

/** @brief A rotation followed by a position. */
typedef struct {
    SVECTOR rot; /**< the rotation */
    Pos3 pos;    /**< the position */
} Placement;

/* MATCHING: player reads this as an SVECTOR array; the position copy
 * needs the whole record here. */
extern Placement D_800A7680;

/* MATCHING: stored as u8 here; player tests it as s8 (lb). */
extern u8 D_80095900;

extern s8 D_8007AD58[];

/** @brief The record the start heading is read from. */
typedef struct {
    u8 unk0[6]; /**< not yet known */
    s16 unk6;   /**< the start heading, negated */
} StartRec;

/* MATCHING: a per-unit view; stage stores it as an s32. */
extern StartRec *D_80095840;

/* MATCHING: object hands it a Rec48; here it gets a position. */
s32 FindFloor(Pos3 *pos);
void RestoreStageObjects(void);
void ResetStageObjects(void);
void RestoreToolBuffer(void);
void PlayStageMusic(u8 idx);
void ResetStage(void);
void StartStageOverlay(void);
void func_80041118(u8 stage);

/** @brief The saved state a stage can resume from. */
typedef struct {
    s32 valid;   /**< non-zero when there is one */
    Pos3 pos;    /**< the position */
    s32 unk10;   /**< restored into a global word */
    s32 unk14;   /**< restored into a second global word */
    u16 unk18;   /**< restored into a global halfword */
    u16 unk1A;   /**< restored into a second global halfword */
    u8 pad1C[4]; /**< not yet known */
    s32 unk20;   /**< restored into a word of a global table */
} SavedStart;

/* MATCHING: a struct lvalue keeps the base in a register. */
#define sSaved (*(SavedStart *)D_800D86B8)

extern s32 D_800957A4;
extern u16 D_800958EA;

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

/* MATCHING: main.c types the object as its Stepper; here it is the game
 * state's head. */
u8 StepAnimation(StageState *obj, u16 index, u8 arg);
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
            sSaved.unk10 = gSection;
            sSaved.pos.vx = D_800A7680.pos.vx;
            sSaved.pos.vy = D_800A7680.pos.vy;
            sSaved.pos.vz = D_800A7680.pos.vz;
            sSaved.unk14 = D_800957F4;
            ResetStage();
            D_80095900 = 1;
            gModeStep = 4;
            break;
        case 4:
            if (gPauseState == 0) {
                SetTmdSortMode(D_80095758);
                UpdatePlayState();
                UpdateRumble();
                switch (stage) {
                    case 0:
                        func_800F809C();
                        break;
                    case 1:
                        func_800F7A9C();
                        break;
                    case 2:
                        func_800F4794();
                        break;
                    case 3:
                        func_800F8264();
                        break;
                    case 4:
                        func_800F7F40();
                        break;
                    case 5:
                        func_800F4758();
                        break;
                    case 6:
                        func_800F00A0();
                        break;
                    case 7:
                        /* MATCHING: keeps cross-jumping from merging this arm with case 6. */
                        func_800F00A0();
                        __asm__("");
                        break;
                    case 8:
                        func_800F42E8();
                        break;
                    case 9:
                        func_800F72B8();
                        break;
                    case 10:
                        func_800F7844();
                        break;
                    case 11:
                        func_800F1654();
                        break;
                    case 12:
                        func_800F7724();
                        break;
                    case 13:
                        func_800F5E80();
                        break;
                    case 14:
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
                        D_800958EA = 0;
                        gPauseState++;
                    case 2:
                        D_800958EA++;
                        DrawPausedFrame();
                        break;
                }
            }
            break;
    }
    D_80095900 = 0;
    if (gPadTrig & 0x800) {
        if ((u16)(gMissState + 1) < 2) {
            if (gPauseState == 0) {
                SetCdVolume(30);
                gPauseState = 1;
            } else {
                if (gPad & 0x100) {
                    StartRumble(gRumblePatterns, 0);
                    gGameMode = 6;
                    gModeStep = 0;
                    StopCdTrack();
                    sStage.unk3D2 = 0;
                    sStage.unk3D0 = 0;
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
    color.r = 1;
    color.g = color.b = color.cd = 0x80;
    DrawSprite(0x15F, &pos, &color, 0, &gSubOt[gActiveBuff]);
    DefineSprite(0x1FE, 2, 0xA0, 0xF0, 10, 0, 0, 0, 0);
    DefineSprite(0x1FF, 2, 0xA0, 0xF0, 12, 0x20, 0, 0, 0);
    color.g = color.b = color.cd = 0x40;
    pos.vx = -0xA0;
    pos.vy = -0x78;
    DrawSprite(0x1FE, &pos, &color, 0, &gSubOt[gActiveBuff]);
    pos.vx = 0;
    pos.vy = -0x78;
    DrawSprite(0x1FF, &pos, &color, 0, &gSubOt[gActiveBuff]);
}

/* MATCHING: an unused 8-byte local gives retail's 0x20 frame; the start
 * position is copied field by field, not as a struct. */
void RestartStage(u8 stage) {
    s32 unused[2];
    s16 yaw;
    u32 i;

    D_80095900 = 1;
    if (stage % 3 == 2) {
        gModeStep = 2;
        return;
    }
    sStage.unk3C8 = 0;
    sStage.unk2 = 1;
    gBonusLevel = 3;
    sStage.unk348.vx = D_800A7680.pos.vx;
    sStage.unk348.vy = D_800A7680.pos.vy;
    sStage.unk348.vz = D_800A7680.pos.vz;
    gMissState = 0;
    sStage.unk3CA = 0;
    if (D_8007AD58[gStage] == 1) {
        D_800957F4 = FindFloor(&sStage.unk348);
    } else {
        D_800957F4 = 0;
    }
    gSection = 0;
    yaw = -D_80095840->unk6;
    D_8009676C[0] = (s16)(D_800A7680.pos.vy - 500);
    D_800A7680.rot.vy = yaw;
    sStage.unk37C = sStage.unk380 = yaw;
    D_800957BC = yaw;
    D_800957B0 = yaw;
    if (gInEditor != 1) {
        RestoreStageObjects();
    } else {
        RestoreToolBuffer();
    }
    ResetStageObjects();
    func_80041118(gStage);
    sStage.unk6 = 2;
    sStage.unk0 = 1;
    D_800958EC = 0;
    D_800957D6 = 1;
    for (i = 0; i < 200; i++) {
        sRecs48[i].unk26 = 0;
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

    D_80095900 = 1;
    StopCdTrack();
    if (gGameMode != 0x29) {
        r.x = 0;
        r.y = D_800E474C * 240;
        r.w = 320;
        r.h = 240;
        MoveImage(&r, 0x280, 0);
    }
    InitEffects();
    gLoading = 0;
    gVSyncMode = 2;
    ResetStageOverlay();
    if (D_800958AC == 1) {
        gBonusLevel = 2;
    } else {
        gBonusLevel = 3;
    }
    gMissState = 0;
    SetCurrentBlock((BlockHeader *)0x801FD000);
    D_80095768 = 0;
    sStage.unk6 = 3;
    sStage.unk3CA = 0;
    sStage.unk3 = 0;
    sStage.unk4 = 0;
    sStage.unk5 = 0;
    sStage.unk8 = 0xFF;
    StepAnimation(&sStage, 0, 0);
    sStage.unk3A8 = 50;
    sStage.unk38E = 0;
    sStage.unk38C = 0;
    sStage.unk3D2 = 0;
    sStage.unk354 = sStage.unk358 = sStage.unk35C = 0;
    sStage.unk360 = sStage.unk362 = sStage.unk364 = 0;
    sStage.unk3D1 = 0;
    sStage.unk3D3 = 1;
    sStage.unk3C8 = 0;
    sStage.unk2 = 1;
    D_8009599C = 0;
    sStage.unk3D4 = sStage.unk3D5 = 0;
    sStage.unk3A6 = 0;
    gView[6] = 0;
    sStage.unk0 = 1;
    D_800958EC = 0;
    D_800957D6 = 1;
    gSection = 0;
    gPauseState = 0;
    D_800957A4 = 0;
    ClipF = 0;
    gFogFade = 0;
    ResetSpeed();
    if (sSaved.valid != 0) {
        gSection = sSaved.unk10;
        sStage.unk348.vx = sSaved.pos.vx;
        sStage.unk348.vy = sSaved.pos.vy;
        sStage.unk348.vz = sSaved.pos.vz;
        D_800957F4 = sSaved.unk14;
        gCameraYaw = sSaved.unk18;
        gCans = sSaved.unk1A;
        gView[1] = sSaved.unk20;
    } else if (gStage == 4) {
        sStage.unk5 = 0x57;
        sStage.unk6 = 0x23;
        sStage.unk398 = 0;
        sStage.unk3A8 = 0;
        gSpeedFixed = 0;
        sStage.unk374 = sStage.unk376 = sStage.unk378 = sStage.unk37A = 0;
        sStage.unk0 = 0;
        sStage.unk348.vy = D_800A7680.pos.vy - 1000;
    }
    mode = gStage;
    saved = sSaved.valid;
    if (D_800958AC == 1) {
        gTimeLeft = gTimeLimitsAlt[mode][saved] * 30;
    } else {
        gTimeLeft = gTimeLimits[mode][saved] * 30;
    }
    D_8009EF48[0] = 0;
    EndJump();
    StartRumble(gRumblePatterns, 0);
}

/* MATCHING: ret has no default; out-of-range numbers return whatever $a1
 * held. */
s16 InitStageOverlay(u8 stage) {
    s32 ret;

    switch (stage) {
        case 0:
            ret = func_800FA6C8();
            break;
        case 1:
            ret = func_800FA804();
            break;
        case 2:
            ret = func_800F6CE4();
            break;
        case 3:
            ret = func_800FA3EC();
            break;
        case 4:
            ret = func_800FA1C0();
            break;
        case 5:
            ret = func_800F6AC8();
            break;
        case 6:
            ret = func_800F1080();
            break;
        case 7:
            ret = func_800F1080();
            break;
        case 8:
            ret = func_800F64B0();
            break;
        case 9:
            ret = func_800F9618();
            break;
        case 10:
            ret = func_800F98E4();
            break;
        case 11:
            ret = func_800F475C();
            break;
        case 12:
            ret = func_800F9BC0();
            break;
        case 13:
            ret = func_800F8D34();
            break;
        case 14:
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
        case 0:
            ret = func_800FAD08(id, a, b);
            break;
        case 1:
            ret = func_800FAD04(id, a, b);
            break;
        case 3:
            ret = func_800FA894(id, a, b);
            break;
        case 4:
            ret = func_800FA580(id, a, b);
            break;
        case 6:
            ret = func_800F1A48(id, a, b);
            break;
        case 7:
            ret = func_800F1A48(id, a, b);
            break;
        case 9:
            ret = func_800F9ABC(id, a, b);
            break;
        case 10:
            ret = func_800F9CA4(id, a, b);
            break;
        case 12:
            ret = func_800FA060(id, a, b);
            break;
        case 13:
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
        i = ((u32)D_80095864 >> 8) & 3;
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

void func_80041118(u8 stage) {
    s32 mode = stage;

    if (mode != 1) {
        if (mode < 2) {
            if (mode == 0) {
                func_800FA5D0();
            }
        }
    }
}
