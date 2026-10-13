/* The code of world 3's free-running stages, 3-1 (ovl09) and 3-2 (ovl10):
 * the stages' own traps and hooks, run by stage_overlay and stage_flow. The
 * two overlays differ only in their data. The including unit defines
 * STAGE3_RODATA_SHAPES as its own INCLUDE_RODATA line for what is still
 * asm. */

#include "common.h"
#include "libgte.h"
#include "libgpu.h"
#include "libcd.h"
#include "libgs.h"
#include "libetc.h"
#include "object.h"
#include "draw.h"
#include "player.h"
#include "spad.h"
#include "collision.h"

/** @brief A trap as this stage's steps move it: its position, its placed
 *         copy and angles, a clock and a state. */
typedef struct {
    s32 x;           /**< position x */
    s32 y;           /**< position y */
    s32 z;           /**< position z */
    s32 homeX;       /**< placed x */
    s32 homeY;       /**< placed y */
    s32 homeZ;       /**< placed z */
    s16 rotX;        /**< rotation x */
    s16 rotY;        /**< rotation y, the heading it moves along */
    s16 rotZ;        /**< rotation z */
    s16 homeRotX;    /**< placed rotation x */
    s16 homeRotY;    /**< placed rotation y; some steps keep a heading in degrees here */
    s16 homeRotZ;    /**< placed rotation z */
    u16 timer;       /**< frames in the current state */
    s16 state;       /**< 0 while it moves */
    s32 block;       /**< the block it stands in */
    s32 lightAngle;  /**< from the floor lookup */
    s32 lightAngle2; /**< from the floor lookup */
    s16 switchNo;    /**< the switch that triggers it */
    s16 model;       /**< its ObjectModel entry */
    u8 unk38[8];     /**< not yet known */
    u8 touched;      /**< 1 when the player touched one of its hit points this frame */
    u8 unk41[7];     /**< not yet known */
} StageTrap;

/** @brief Three words, a position. */
typedef struct {
    s32 x; /**< x */
    s32 y; /**< y */
    s32 z; /**< z */
} Vec3;

/* MATCHING: object's view, with this unit's trap record. */
s32 SnapToFloor(StageTrap *t);
void PolarToXZ(s32 r, s32 deg, Vec3 *out);
void DrawObjectModel(ObjectModel *rec, Trap *r);

extern SwitchBox gSwitches[];

/** @brief The shape of each of the stage's own sprites, as one block. */
typedef struct {
    u8 shape[41]; /**< a shape per sprite id from SPR_STAGE on */
} SpriteShapes;

extern u8 sSpriteShapes[];
extern const SpriteShapes D_800F0074;

/* MATCHING: object's view, with this unit's trap record. */
void ResetObj34(StageTrap *t);

/** @brief The Trap table, copied whole. */
typedef struct {
    s32 w[200 * sizeof(Trap) / 4]; /**< the records */
} TrapTable;

/** @brief The SwitchBox table, copied whole. */
typedef struct {
    s32 w[200 * sizeof(SwitchBox) / 4]; /**< the records */
} SwitchTable;

/** @brief The StageSprite table (100 records of 0x3C bytes), copied whole. */
typedef struct {
    s32 w[0x3C * 100 / 4]; /**< the records */
} SpriteTable;

/** @brief The block header area, copied whole; bytes, so a copy of it
 *         tests the alignment at run time. */
typedef struct {
    u8 b[0x800]; /**< the area */
} BlockArea;

/** @brief The stage's object file as it is read to 0x8016D000: the tables
 *         a stage starts with. */
typedef struct {
    TrapTable traps;      /**< the Trap table */
    SwitchTable switches; /**< the SwitchBox table */
    SpriteTable sprites;  /**< the StageSprite table */
    BlockArea block;      /**< the block header area */
} StageObjects;

extern s32 gStageSprites[];
extern CdlLOC sStageFileLoc[];
extern s32 sStageFileSectors[];

/* MATCHING: main's views of the loaders. */
void SetBootReady(s32 arg);
void LoadTimPack(void *pack);
s32 LoadVabBank(s32 bank);
void LoadStageData(unsigned long *p);
void SetHitBanks(void *a, void *b);
void LoadSpritePack(void *dir, u16 id);
void ResetStageObjects(void);
void StartStageOverlay(void);
void func_800F15E4(void);
/* MATCHING: object's view, onto the model slots. */
void InitModel(ObjectModel *m, unsigned long *tmd, u8 n);

/** @brief The stage's start record: the start point, negated, and the
 *         start heading, negated. */
typedef struct {
    s16 x;   /**< the start x, negated */
    s16 y;   /**< the start y, negated */
    s16 z;   /**< the start z, negated */
    s16 yaw; /**< the start heading, negated */
} StartRec;

/* MATCHING: a per-unit view; stage stores it as an s32. */
extern StartRec *gStageStart;
extern s16 sCamOffsetX;
/* MATCHING: stage's sCamOffsetX and the sCamOffsetZ after it, reached as
 * one pair. */
#define sCamOffset (&sCamOffsetX)
extern s16 sCamHeight;
extern u16 sViewSpeed;

#define sTraps ((StageTrap *)gTraps)

/** @brief A button prompt: shown from a section on, for a while. */
typedef struct {
    s16 section; /**< the section that shows it */
    s16 limit;   /**< the frames it stays */
    s16 button;  /**< which button it asks for: left, right, cross or circle */
    s16 time;    /**< frames shown */
    s16 unk8;    /**< not yet known */
    s16 state;   /**< 0 waiting, 1 shown, 2 pressed, 100 missed */
} Prompt;

extern s32 D_800F2DD8;
extern Prompt D_800F2DDC[];
extern s32 D_800F2E6C;

void func_800F0F44(void);
void func_800F0F24(void);

/* MATCHING: player's views, with the offset as words. */
void PolarOffset(s32 deg, s32 radius, s32 *out);
void ScatterDust(VECTOR *pos);
s32 Lerp(s32 a, s32 b, u16 t, u16 n);

/* MATCHING: read signed, as the stage steps test them. */
#define sFellOff (*(s8 *)&gFellOff)
#define sPlayLive (*(s8 *)&gPlayLive)

/* MATCHING: the step's callees, which their units declare only where they
 * define them; this image links against them by address. */
s16 UpdateCamera(s16 x, s16 target, s16 y, s16 height);
void UpdatePlayer(void);
void UpdateStageLights(void);
void PushPlayerFromWalls(void);
s32 PairMarks(void);
void UpdateActiveEntries(void);
void UpdateActiveSprites(void);
s32 UpdatePlayerAction(PlayerState *state);
s32 DrawTodObjects(void);
/* MATCHING: more of the step's callees, declared only where defined. */
void DrawPlayerShadow(void);
void UnusedHook2(void);
void PopInNextCan(void);
void DrawStage(s16 x, s16 y, s16 z);
void DrawSky(u32 arg);
void UpdateEffects(s32 unused, s16 x, s16 y, s16 z);
/* MATCHING: object defines it with no parameters; retail's call passes the
 * ordering table it once took. */
void UpdatePickups(GsOT *ot);

/* MATCHING: an unused 32-byte local gives retail's frame. */
void func_800F00A0(void) {
    s32 unused[8];
    s32 v;

    if (sFellOff == 0) {
        gCourse.rot.vy = UpdateCamera(gPlayer.x, gPlayer.y, gPlayer.z, gFloorY[0]) + 0x800;
    }
    gPlayer.editorY = gPlayer.groundY;
    v = FindFloorNear(&gPlayer.x, gFloorY, gFloorIndex);
    if (v != -1) {
        gFloorIndex = v;
        gStageParams = *(s32 *)&gFloorY[4];
    }
    gPlayer.groundY = gFloorY[0];
    SetSpadStack();
    UpdatePlayer();
    UpdateStageLights();
    if (sFellOff == 0) {
        PushPlayerFromWalls();
    }
    TestBoxes();
    PairMarks();
    ResetCollision();
    if (gInEditor) {
        UpdateActiveEntries();
    } else {
        func_800F0F24();
    }
    UpdateActiveSprites();
    ReactToHits(&gPlayer);
    UpdatePickups(&gSubOt[gActiveBuff]);
    LeaveQuadTop();
    UpdatePlayerAction(&gPlayer);
    ResetSpadStack();
    DrawTodObjects();
    DrawPlayerShadow();
    UnusedHook2();
    PopInNextCan();
    if (sPlayLive == 1) {
        DrawStage(-gPlayer.x, 0, -gPlayer.z);
        DrawSky(gStageParams);
    }
    UpdateEffects((s32)&gSubOt[gActiveBuff], -gPlayer.x, 0, -gPlayer.z);
}

void func_800F02E8(StageTrap *t) {
    switch (t->state) {
        case 0:
            if (t->touched) {
                t->state++;
                t->timer = 0;
            }
            UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
            break;
        case 1:
            t->timer++;
            t->rotX = Lerp(0, -0x4E3, t->timer, 3);
            if (t->timer == 3) {
                t->homeRotY = (t->rotY - 0x800) * 360 / 4096;
                t->timer = 0;
                t->state++;
            }
            DrawObjectModel(&gObjectModels[t->model], (Trap *)t);
            break;
        case 2:
            gPlayer.motion[0] = 4;
            PolarOffset(t->homeRotY, 50, gOffsetVec);
            gPlayer.x += gOffsetVec[0];
            gPlayer.z += gOffsetVec[2];
            ScatterDust((VECTOR *)&gPlayer.x);
            if (++t->timer == 30) {
                t->timer = 0;
                t->homeRotY = t->rotY;
                t->state++;
            }
            DrawObjectModel(&gObjectModels[t->model], (Trap *)t);
            break;
    }
}

void func_800F04C4(StageTrap *t) {
    s32 v;

    switch (t->state) {
        case 0:
            if (t->touched) {
                t->homeX = 0;
                t->homeY = t->y;
                t->homeRotY = gPlayer.yaw;
                t->state++;
                t->timer = 0;
            }
            UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
            break;
        case 1:
            PolarToXZ(40, t->homeRotY * 360 / 4096, (Vec3 *)gOffsetVec);
            t->x -= gOffsetVec[0] >> 12;
            t->z -= gOffsetVec[2] >> 12;
            t->rotX -= 0x71;
            t->timer = (t->timer + 10) % 360;
            v = rsin(t->timer * 4096 / 360) * 200 >> 12;
            /* MATCHING: abs as one insn keeps v block-local. */
            v = v >= 0 ? v : -v;
            t->y = t->homeY - v;
            if (++t->homeX == 50) {
                t->state++;
            }
            if (gFrameCount % 3 != 0) {
                DrawObjectModel(&gObjectModels[t->model], (Trap *)t);
            }
            break;
    }
}

void func_800F06F4(StageTrap *t, s8 dir) {
    s32 a;

    switch (t->state) {
        case 0:
            if (DistanceSquared((VECTOR *)&gPlayer.x, (VECTOR *)t) < 500 * 500) {
                t->timer = 0;
                t->state++;
                t->homeRotY = t->rotY;
            }
            DrawObjectModel(&gObjectModels[t->model], (Trap *)t);
            break;
        case 1:
            t->timer++;
            a = Lerp(0, 0x200, t->timer, 10);
            if (dir != 0) {
                if (dir == 1) {
                    t->rotY = t->homeRotY - a;
                }
            } else {
                t->rotY = t->homeRotY + a;
            }
            DrawObjectModel(&gObjectModels[t->model], (Trap *)t);
            if (t->timer == 10) {
                t->timer = 0;
                t->state++;
                t->model++;
                switch (dir) {
                    case 1:
                        t->homeRotY += 0x200;
                        break;
                    case 0:
                        t->homeRotY -= 0x200;
                        break;
                }
            }
            break;
        case 2:
            t->timer++;
            a = Lerp(0, 0x200, t->timer, 10);
            if (dir != 0) {
                if (dir == 1) {
                    t->rotY = t->homeRotY - a;
                }
            } else {
                t->rotY = t->homeRotY + a;
            }
            if (t->timer == 10) {
                t->timer = 0;
                t->state++;
                if (dir != 0) {
                    if (dir == 1) {
                        t->rotY = 0;
                    }
                } else {
                    t->rotY = 0x800;
                }
            }
        case 3:
            UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
            break;
    }
}

/* MATCHING: non-void with no return keeps the last delay slot a nop. */
s32 func_800F0988(void) {
    SVECTOR pos;
    s32 t;

    switch (D_800F2DDC[D_800F2DD8].state) {
        case 0:
            if (D_800F2DDC[D_800F2DD8].section == gSection) {
                D_800F2DDC[D_800F2DD8].state = 1;
            }
            break;
        case 1:
            switch (D_800F2DDC[D_800F2DD8].button) {
                case 0:
                    if (gPadTrig & PADLleft) {
                        D_800F2DDC[D_800F2DD8].state = 2;
                    }
                    t = 8;
                    if (D_800F2DDC[D_800F2DD8].time < 9) {
                        t = D_800F2DDC[D_800F2DD8].time;
                    }
                    pos.vx = 0xC4 - t * 30;
                    pos.vy = -20;
                    DrawSprite(0x161, &pos, NULL, 0, &gSubOt[gActiveBuff]);
                    break;
                case 1:
                    if (gPadTrig & PADLright) {
                        D_800F2DDC[D_800F2DD8].state = 2;
                    }
                    t = 8;
                    if (D_800F2DDC[D_800F2DD8].time < 9) {
                        t = D_800F2DDC[D_800F2DD8].time;
                    }
                    pos.vx = t * 30 - 0x11C;
                    pos.vy = -20;
                    DrawSprite(0x162, &pos, NULL, 0, &gSubOt[gActiveBuff]);
                    break;
                case 2:
                    if (gPadTrig & PADRdown) {
                        D_800F2DDC[D_800F2DD8].state = 2;
                    }
                    t = 8;
                    if (D_800F2DDC[D_800F2DD8].time < 9) {
                        t = D_800F2DDC[D_800F2DD8].time;
                    }
                    pos.vx = -0x2C;
                    pos.vy = 0xDC - t * 30;
                    DrawSprite(0x163, &pos, NULL, 0, &gSubOt[gActiveBuff]);
                    break;
                case 3:
                    if (gPadTrig & PADRright) {
                        D_800F2DDC[D_800F2DD8].state = 2;
                    }
                    t = 8;
                    if (D_800F2DDC[D_800F2DD8].time < 9) {
                        t = D_800F2DDC[D_800F2DD8].time;
                    }
                    pos.vx = -0x2C;
                    pos.vy = t * 30 - 0x104;
                    DrawSprite(0x133, &pos, NULL, 0, &gSubOt[gActiveBuff]);
                    break;
            }
            if (++D_800F2DDC[D_800F2DD8].time > D_800F2DDC[D_800F2DD8].limit) {
                D_800F2DDC[D_800F2DD8].state = 100;
            }
            break;
        case 2:
            D_800F2DD8++;
            break;
        case 100:
            D_800F2DD8++;
            break;
    }
}

void func_800F0CF4(StageTrap *t, u16 speed) {
    if (t->state == 0) {
        SnapToFloor(t);
        PolarToXZ(speed, t->rotY * 360 / 4096, (Vec3 *)gOffsetVec);
        t->x += gOffsetVec[0] >> 12;
        t->z += gOffsetVec[2] >> 12;
        if (++t->timer == 200) {
            t->timer = 0;
            t->state++;
        }
        DrawObjectModel(&gObjectModels[t->model], (Trap *)t);
    }
}

void func_800F0DF0(StageTrap *t, u16 speed) {
    if (t->state == 0) {
        SnapToFloor(t);
        PolarToXZ(speed, t->rotY * 360 / 4096, (Vec3 *)gOffsetVec);
        t->x += gOffsetVec[0] >> 12;
        t->z += gOffsetVec[2] >> 12;
        if (++t->timer == 200) {
            t->timer = 0;
            t->state++;
        }
        UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
    }
}

void func_800F0EEC(void) {
    ClearCanGroups();
    gCanGroupOn[1] = 1;
    gCanGroupOn[2] = 1;
    ShowRevealedCans();
}

void func_800F0F24(void) {
    func_800F0F44();
}

void func_800F0F44(void) {
    s32 unused[4];
}

void func_800F0F54(void) {
    s32 unused[4];
}

void func_800F0F64(void) {
    s32 unused[4];
}

void func_800F0F74(void) {
    s32 unused[4];
}

void func_800F0F84(void) {
    s32 unused[4];

    if (gSwitches[8].state == SWITCH_ON) {
        func_800F0DF0(&sTraps[11], 30);
    }
}

void func_800F0FBC(void) {
    s32 unused[4];
}

void func_800F0FCC(void) {
    s32 unused[4];
}

void func_800F0FDC(void) {
    s32 unused[4];
}

void func_800F0FEC(StageTrap *t) {
    if (t->state == 0) {
        UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
    }
}

void func_800F1038(void) {
    s32 i;

    for (i = 0; i < D_800F2E6C; i++) {
        D_800F2DDC[i].time = 0;
        D_800F2DDC[i].unk8 = 0;
        D_800F2DDC[i].state = 0;
    }
    D_800F2DD8 = 0;
}

s32 func_800F1080(void) {
    StageObjects *f;
    /* MATCHING: an s16 result keeps gSubStep's increment off its register;
     * an unused 8-byte local gives retail's frame. */
    s16 ret = 0;
    s32 unused[2];

    switch (gSubStep) {
        case 0:
            SetBootReady(0);
            gCdLoadLoc[0] = (s32)&sStageFileLoc[0];
            gCdLoadDest[0] = 0x8019D000;
            gCdLoadCount = 1;
            gCdLoadState = CD_LOAD_SEEK;
            gCdLoadSectors[0] = sStageFileSectors[1];
            gSubStep++;
            break;
        case 1:
            if (gCdLoadState == CD_LOAD_DONE) {
                LoadTimList((unsigned long *)0x8019D000);
                gVabLoadStep = 0;
                gSubStep++;
            }
            break;
        case 2:
            if (LoadVabBank(4) == 1) {
                gSubStep++;
            }
            break;
        case 3:
            gCdLoadLoc[0] = (s32)&sStageFileLoc[1];
            gCdLoadDest[0] = 0x8019D000;
            gCdLoadCount = 1;
            gCdLoadState = CD_LOAD_SEEK;
            gCdLoadSectors[0] = sStageFileSectors[2];
            gSubStep++;
            break;
        case 4:
            if (gCdLoadState == CD_LOAD_DONE) {
                LoadTimPack((void *)0x8019D000);
                gSubStep++;
            }
            break;
        case 5:
            gCdLoadLoc[0] = (s32)&sStageFileLoc[2];
            gCdLoadDest[0] = 0x8019D000;
            gCdLoadLoc[1] = (s32)&sStageFileLoc[3];
            gCdLoadDest[1] = 0x80155000;
            gCdLoadCount = 2;
            gCdLoadState = CD_LOAD_SEEK;
            gCdLoadSectors[0] = sStageFileSectors[3];
            gCdLoadSectors[1] = sStageFileSectors[4];
            gSubStep++;
            break;
        case 6:
            if (gCdLoadState == CD_LOAD_DONE) {
                LoadStageData((unsigned long *)0x8019D000);
                gSubStep++;
            }
            break;
        case 7:
            gCdLoadLoc[0] = (s32)&sStageFileLoc[4];
            gCdLoadDest[0] = 0x8016D000;
            gCdLoadCount = 1;
            gCdLoadState = CD_LOAD_SEEK;
            gCdLoadSectors[0] = sStageFileSectors[5];
            gSubStep++;
            break;
        case 8:
            if (gCdLoadState == CD_LOAD_DONE) {
                LoadSpritePack((void *)0x8016D000, SPR_STAGE);
                gSubStep++;
            }
            break;
        case 9:
            func_800F15E4();
            gSubStep++;
            break;
        case 10:
            gCdLoadLoc[0] = (s32)&sStageFileLoc[5];
            gCdLoadDest[0] = 0x8016D000;
            gCdLoadCount = 1;
            gCdLoadState = CD_LOAD_SEEK;
            gCdLoadSectors[0] = sStageFileSectors[6];
            gSubStep++;
            break;
        case 11:
            if (gCdLoadState == CD_LOAD_DONE) {
                SetHitBanks((void *)0x8016D000, (void *)0x80171744);
                gSubStep++;
            }
            break;
        case 12:
            gCdLoadLoc[0] = (s32)&sStageFileLoc[6];
            gCdLoadDest[0] = 0x8016D000;
            gCdLoadCount = 1;
            gCdLoadState = CD_LOAD_SEEK;
            gCdLoadSectors[0] = sStageFileSectors[7];
            gSubStep++;
            break;
        case 13:
            if (gCdLoadState == CD_LOAD_DONE) {
                f = (StageObjects *)0x8016D000;
                *(TrapTable *)gTraps = f->traps;
                *(SwitchTable *)gSwitches = f->switches;
                *(SpriteTable *)gStageSprites = f->sprites;
                *(BlockArea *)0x801FD000 = f->block;
                SetCurrentBlock((BlockHeader *)0x801FD000);
                gSubStep++;
            }
            break;
        case 14:
            ret = 1;
            ResetStageObjects();
            StartStageOverlay();
            SetBootReady(1);
            gSubStep++;
            break;
    }
    FntPrint("CDEXEC %d", gSubStep);
    return ret;
}

void func_800F15E4(void) {
    gSectionCount = 0;
    InitModel(&gObjectModels[0], (unsigned long *)0x80155000, 0);
    InitModel(&gObjectModels[1], (unsigned long *)0x80155000, 1);
    InitModel(&gObjectModels[2], (unsigned long *)0x80155000, 2);
    InitModel(&gObjectModels[3], (unsigned long *)0x80155000, 3);
    InitModel(&gObjectModels[4], (unsigned long *)0x80155000, 4);
    InitModel(&gObjectModels[5], (unsigned long *)0x80155000, 5);
    InitModel(&gObjectModels[6], (unsigned long *)0x80155000, 6);
    InitModel(&gObjectModels[7], (unsigned long *)0x80155000, 7);
    InitModel(&gObjectModels[8], (unsigned long *)0x80155000, 8);
    InitModel(&gObjectModels[9], (unsigned long *)0x80155000, 9);
    InitModel(&gObjectModels[10], (unsigned long *)0x80155000, 10);
    InitModel(&gObjectModels[11], (unsigned long *)0x80155000, 11);
    InitModel(&gObjectModels[12], (unsigned long *)0x80155000, 12);
    InitModel(&gObjectModels[13], (unsigned long *)0x80155000, 13);
    InitModel(&gObjectModels[14], (unsigned long *)0x80155000, 14);
    InitModel(&gObjectModels[15], (unsigned long *)0x80155000, 15);
    InitModel(&gObjectModels[16], (unsigned long *)0x80155000, 16);
    InitModel(&gObjectModels[17], (unsigned long *)0x80155000, 17);
    InitModel(&gObjectModels[18], (unsigned long *)0x80155000, 18);
    InitModel(&gObjectModels[19], (unsigned long *)0x80155000, 19);
    InitModel(&gObjectModels[20], (unsigned long *)0x80155000, 20);
    InitModel(&gObjectModels[21], (unsigned long *)0x80155000, 21);
    InitModel(&gObjectModels[22], (unsigned long *)0x80155000, 22);
    InitModel(&gObjectModels[23], (unsigned long *)0x80155000, 23);
    InitModel(&gObjectModels[24], (unsigned long *)0x80155000, 24);
    InitModel(&gObjectModels[25], (unsigned long *)0x80155000, 25);
    InitModel(&gObjectModels[26], (unsigned long *)0x80155000, 26);
    InitModel(&gObjectModels[27], (unsigned long *)0x80155000, 27);
    InitModel(&gObjectModels[28], (unsigned long *)0x80155000, 28);
    InitModel(&gObjectModels[29], (unsigned long *)0x80155000, 29);
    InitModel(&gObjectModels[30], (unsigned long *)0x80155000, 30);
    InitModel(&gObjectModels[31], (unsigned long *)0x80155000, 31);
    InitModel(&gObjectModels[32], (unsigned long *)0x80155000, 32);
    InitModel(&gObjectModels[33], (unsigned long *)0x80155000, 33);
    InitModel(&gObjectModels[34], (unsigned long *)0x80155000, 34);
    InitModel(&gObjectModels[35], (unsigned long *)0x80155000, 35);
    InitModel(&gObjectModels[36], (unsigned long *)0x80155000, 36);
    InitModel(&gObjectModels[37], (unsigned long *)0x80155000, 37);
}

void func_800F1908(void) {
    s16 yaw;
    u8 one;
    StartRec *st;

    /* MATCHING: stored as the s16 it is read as; the start pointer and the 1
     * in locals, set after gFellOff, order the first stores. */
    *(s16 *)&gCameraYaw = -0x201;
    sCamHeight = -150;
    sViewSpeed = 100;
    gViewerAngle = 0xD8;
    gFellOff = 0;
    st = gStageStart;
    one = 1;
    gPlayLive = one;
    yaw = -st->yaw;
    ((SVECTOR *)sCamOffset)->vx = 0;
    gView[1] = 0;
    ((SVECTOR *)sCamOffset)->vy = 0;
    gPlayer.motion[0] = MOT_RUN_SLOW;
    gPlayer.control = 1;
    gPlayer.hittable = 1;
    gCourse.rot.vy = yaw;
    gCourse.rot.vz = yaw;
    gPlayer.startYaw = gPlayer.yaw = yaw;
    gFloorIndex = 0;
    gCourse.x = gPlayer.x = -st->x;
    gSteerYaw = yaw;
    gCourse.y = gPlayer.y = -st->y;
    gSteerYawHalf = yaw;
    gCourse.z = gPlayer.z = -st->z;
    gPlayer.drawX = 0;
    gPlayer.drawY = 0;
    gPlayer.drawZ = 0;
    gBodyRot[0] = 0;
    gBodyRot[1] = 0;
    gBodyRot[2] = 0;
    gView[1] = (s16)(gCourse.y - 150);
    func_800F0EEC();
}

/* MATCHING: ret set before the table copy and again after the call, so it
 * stays a register; set once it folds to a constant. */
u16 func_800F1A48(s16 id, StageTrap *src, StageTrap *dst) {
    u16 ret = 1;
    u8 models[38] = {0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17, 18,
                     19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37};

    if (id < 38) {
        if (id >= 0) {
            dst->homeX = src->x;
            dst->homeY = src->y;
            dst->homeZ = src->z;
            dst->homeRotX = src->rotX;
            dst->homeRotY = src->rotY;
            dst->homeRotZ = src->rotZ;
            ResetObj34(dst);
            dst->model = models[id];
            ret = 1;
        }
    }
    return ret;
}

void func_800F1BA4(void) {
    SpriteShapes shapes;

    shapes = D_800F0074;
    *(SpriteShapes *)sSpriteShapes = shapes;
}

STAGE3_RODATA_SHAPES
