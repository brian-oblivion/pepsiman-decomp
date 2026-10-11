#include "common.h"
#include "libgte.h"
#include "libgpu.h"
#include "libetc.h"
#include "libpad.h"
#include "libgs.h"
#include "libapi.h"
#include "libcd.h"
#include "libsnd.h"
#include "sound.h"
#include "libmcrd.h"
#include "memory.h"
#include "draw.h"
#include "model.h"
#include "object.h"
#include "spad.h"

INCLUDE_RODATA("asm/nonmatchings/main", sStageMusicTracks);

/** @brief Two words, cleared or copied as a pair (a unit-local view). */
typedef struct {
    s32 a; /**< first word */
    s32 b; /**< second word */
} Pair8;

/* MATCHING: per-unit views; stage reads the same pairs as words. */
extern Pair8 gBestCans[];
extern Pair8 gBestTimes[];
extern Pair8 gTargetTimes[];
extern u8 gRumbleEnabled;
extern s8 D_800957A0;
extern u8 D_800958A5;
extern s32 D_80095984;
extern s32 D_80095998;
extern u16 gRumbleSmall;
extern u16 gRumbleLarge;
extern char sNewline[]; /**< "\n" */

/* MATCHING: arity-ok, retail passes an argument the empty definition ignores. */
void func_80018DE8(s32 arg);
s8 LoadBootData(void);
s32 LoadSoundData(void);
void InitSystem(void);
void ResetClearColor(void);
s8 UpdateCdLoad(void);
void EndFrame(void);
void UpdateLoadScreenModel(void);
void UpdateLoadScreen(void);
void UpdateAct3LoadScreen(void);
void UpdateTrapLayoutViewer(void);
void func_800F026C(void);
void func_800F6670(void);
void func_800F03B4(void);
void func_800F03BC(void);
void func_800F0414(void);
void func_800F0480(void);

/**
 * @brief The game's entry, called from crt0: initialises the system and the
 *        globals, then steps the top-level game state once
 *        per frame, forever.
 */
int main(void) {
    u8 world;
    Pair8 *dst;
    s32 a;
    s32 b;

    InitSystem();
    InitTmdSortFuncs();
    func_80018DE8(1);
    InitView();
    InitLights();
    SetFogNearFar(3000, 8000, 250);
    SetFarColor(0x40, 0x60, 0xFF);
    gRumbleEnabled = 1;
    gStereo = 1;
    gUnlocks = 1;
    D_800957A0 = -1;
    ClipF = 0;
    gTmdSortMode = 0;
    gButtonConfig = 0;
    D_800957D5 = 0;
    D_800958A5 = 0;
    gStageTableSet = -1;
    D_80095984 = 0;
    D_80095998 = 0;
    for (gLoopI = 0; gLoopI < 4; gLoopI++) {
        gBestCans[gLoopI].a = 0;
        gBestCans[gLoopI].b = 0;
        dst = &gBestTimes[gLoopI];
        a = gTargetTimes[gLoopI].a;
        b = gTargetTimes[gLoopI].b;
        dst->a = a;
        dst->b = b;
    }
    gViewerMotion = 0;
    gModeStep = 0;
    gGameMode = GM_BOOT;
    gLoading = 0;
    gRumbleLarge = 0;
    gRumbleSmall = 0;
    gRumbleEnabled = 0;
    gCpuMarkCount = 0;
    gInEditor = 0;
    gCdTrackPlaying = 0;
    for (;;) {
        FntPrint(sNewline);
        if (gLoading == 1) {
            ResetClearColor();
        }
        switch (gGameMode) {
            case GM_BOOT:
                gClearB = 0;
                gClearG = 0;
                gClearR = 0;
                gSubStep = 0;
                gGameMode = GM_BOOT_LOAD;
                gVSyncMode = 2;
                break;
            case GM_BOOT_LOAD:
                if (LoadBootData() == 1) {
                    gSubStep = 0;
                    gGameMode = GM_SOUND_LOAD;
                }
                break;
            case GM_SOUND_LOAD:
                if (LoadSoundData() == 1) {
                    gGameMode = GM_MENU_LOAD;
                }
                break;
            case GM_OVERLAY0_LOAD:
                gLoading = 1;
                gGameMode = GM_OVERLAY0_WAIT;
                LoadOverlay(0);
                break;
            case GM_OVERLAY0_WAIT:
                if (gCdLoadState == CD_LOAD_DONE) {
                    gGameMode = GM_OVERLAY0;
                    gModeStep = 0;
                    gLoading = 0;
                }
                break;
            case GM_OVERLAY0:
                func_800F026C();
                break;
            case GM_MENU_LOAD:
                gLoading = 1;
                gInEditor = 0;
                gGameMode = GM_MENU_WAIT;
                LoadOverlay(2);
                break;
            case GM_MENU_WAIT:
                if (gCdLoadState == CD_LOAD_DONE) {
                    gGameMode = GM_MENU;
                    if (gModeStep == 0xFFFF) {
                        gModeStep = 50;
                    } else {
                        gModeStep = 0;
                    }
                    gLoading = 0;
                }
                break;
            case GM_MENU:
                func_800F6670();
                UpdateRumble();
                break;
            case GM_WORLD_INTRO_LOAD:
                world = gStage / 3;
                gClearB = 0;
                gClearG = 0;
                gClearR = 0;
                gLoading = 1;
                gGameMode = GM_WORLD_INTRO_WAIT;
                LoadOverlay(world + 18);
                break;
            case GM_WORLD_INTRO_WAIT:
                if (gCdLoadState == CD_LOAD_DONE) {
                    gGameMode = GM_WORLD_INTRO;
                }
                gModeStep = 0;
                D_80095768 = 0;
                break;
            case GM_WORLD_INTRO:
                switch ((u8)(gStage / 3)) {
                    case 0:
                        func_800F03BC();
                        break;
                    case 1:
                        func_800F03B4();
                        break;
                    case 2:
                        func_800F0480();
                        break;
                    case 3:
                        /* MATCHING: keeps cross-jumping off case 0's call. */
                        func_800F03BC();
                        __asm__("");
                        break;
                    case 4:
                        func_800F0414();
                        break;
                }
                break;
            case GM_STAGE_LOAD:
                gClearB = 0;
                gClearG = 0;
                gClearR = 0;
                gInEditor = 0;
                gLoading = 1;
                gGameMode = GM_STAGE_WAIT;
                LoadStageOverlay(gStage);
                break;
            case GM_STAGE_WAIT:
                if (gCdLoadState == CD_LOAD_DONE) {
                    gGameMode = GM_STAGE;
                }
                gModeStep = 0;
                break;
            case GM_STAGE:
                UpdateStagePlay(gStage);
                break;
            case GM_GRID_TEST_INIT:
                gGameMode = GM_GRID_TEST;
                InitGridTest();
                gClearB = 0;
                gClearG = 0;
                gClearR = 0;
                gVSyncMode = 2;
                break;
            case GM_GRID_TEST:
                UpdateGridTest();
                break;
            case GM_MOTION_VIEWER_INIT:
                gVSyncMode = 2;
                gGameMode = GM_MOTION_VIEWER;
                gModeStep = 0;
                break;
            case GM_MOTION_VIEWER:
                UpdateMotionViewer();
                break;
            case GM_TRAP_VIEWER_LOAD:
                gGameMode = GM_TRAP_VIEWER;
                gClearB = 0;
                gClearG = 0;
                gClearR = 0;
                gLoading = 1;
                gGameMode = GM_TRAP_VIEWER_WAIT;
                gSubStep = 0;
                gSelectedSection = 0;
                LoadStageOverlay(gStage);
                gVSyncMode = 0;
                break;
            case GM_TRAP_VIEWER_WAIT:
                if (gCdLoadState == CD_LOAD_DONE) {
                    gGameMode = GM_TRAP_VIEWER_INIT;
                }
                break;
            case GM_TRAP_VIEWER_INIT:
                if (InitStageOverlay(gStage) == 1) {
                    gGameMode = GM_TRAP_VIEWER;
                    gLoading = 0;
                }
                break;
            case GM_TRAP_VIEWER:
                UpdateTrapViewer();
                break;
            case GM_TRAP_LAYOUT_LOAD:
                gClearB = 0;
                gClearG = 0;
                gClearR = 0;
                gLoading = 1;
                gGameMode = GM_TRAP_LAYOUT_WAIT;
                gSubStep = 0;
                gSelectedSection = 0;
                LoadStageOverlay(gStage);
                gVSyncMode = 0;
                break;
            case GM_TRAP_LAYOUT_WAIT:
                if (gCdLoadState == CD_LOAD_DONE) {
                    gGameMode = GM_TRAP_LAYOUT_INIT;
                }
                break;
            case GM_TRAP_LAYOUT_INIT:
                if (InitStageOverlay(gStage) == 1) {
                    gGameMode = GM_TRAP_LAYOUT;
                    gLoading = 0;
                    gModeStep = 0;
                }
                break;
            case GM_TRAP_LAYOUT:
                UpdateTrapLayoutViewer();
                break;
            case GM_EDITOR_LOAD:
                gInEditor = 1;
                gClearB = 0;
                gClearG = 0;
                gClearR = 0;
                gLoading = 1;
                gGameMode = GM_EDITOR_WAIT;
                gSubStep = 0;
                gSelectedSection = 0;
                LoadStageOverlay(gStage);
                gVSyncMode = 2;
                break;
            case GM_EDITOR_WAIT:
                if (gCdLoadState == CD_LOAD_DONE) {
                    gGameMode = GM_EDITOR_INIT;
                }
                break;
            case GM_EDITOR_INIT:
                if (InitStageOverlay(gStage) == 1) {
                    gGameMode = GM_EDITOR;
                    gModeStep = 0;
                    gLoading = 0;
                }
                break;
            case GM_EDITOR_RELOAD:
                if (gCdLoadState == CD_LOAD_DONE) {
                    gGameMode = GM_EDITOR;
                    gModeStep = 0;
                    gLoading = 0;
                }
                break;
            case GM_EDITOR:
                UpdateEditor();
                break;
            case GM_HIT_TOOL_LOAD:
                gInEditor = 1;
                gClearB = 0;
                gClearG = 0;
                gClearR = 0;
                gLoading = 1;
                gGameMode = GM_HIT_TOOL_WAIT;
                gSubStep = 0;
                gSelectedSection = 0;
                LoadStageOverlay(gStage);
                gVSyncMode = 2;
                break;
            case GM_HIT_TOOL_WAIT:
                if (gCdLoadState == CD_LOAD_DONE) {
                    gGameMode = GM_HIT_TOOL_INIT;
                }
                break;
            case GM_HIT_TOOL_INIT:
                if (InitStageOverlay(gStage) == 1) {
                    gGameMode = GM_HIT_TOOL;
                    gModeStep = 0;
                    gLoading = 0;
                }
                break;
            case GM_HIT_TOOL_RELOAD:
                if (gCdLoadState == CD_LOAD_DONE) {
                    gGameMode = GM_HIT_TOOL;
                    gModeStep = 0;
                    gLoading = 0;
                }
                break;
            case GM_HIT_TOOL:
                UpdateTool();
                break;
            case GM_LOAD_SCREEN_MODEL:
                UpdateLoadScreenModel();
                break;
            case GM_LOAD_SCREEN:
                UpdateLoadScreen();
                break;
            case GM_LOAD_SCREEN_ACT3:
                UpdateAct3LoadScreen();
                break;
            case GM_MOVIE:
                switch ((u8)(gStage / 3)) {
                    case 0:
                        PlayMovie("\\MOVIE1.STR;1", NULL, 0xBC, 0xF0, 1);
                        gGameMode = GM_WORLD_INTRO_LOAD;
                        break;
                    case 1:
                        PlayMovie("\\MOVIE2.STR;1", NULL, 0x166, 0xF0, 1);
                        gGameMode = GM_WORLD_INTRO_LOAD;
                        break;
                    case 2:
                        PlayMovie("\\MOVIE3.STR;1", NULL, 0x12A, 0xF0, 1);
                        gGameMode = GM_WORLD_INTRO_LOAD;
                        break;
                    case 3:
                        PlayMovie("\\MOVIE3.STR;1", NULL, 0x12A, 0xF0, 1);
                        gGameMode = GM_WORLD_INTRO_LOAD;
                        break;
                    case 4:
                        PlayMovie("\\MOVIE4.STR;1", NULL, 0x184, 0xF0, 1);
                        gGameMode = GM_WORLD_INTRO_LOAD;
                        break;
                    case 5:
                        PlayMovie("\\MOVIE5.STR;1", NULL, 0x661, 0xF0, 1);
                        if (gStageTableSet == 0) {
                            D_800958A5 = 1;
                            gUnlocks |= 2;
                        } else if (gStageTableSet == 1) {
                            gUnlocks |= 8;
                        }
                        D_800957A0 = -1;
                        gGameMode = GM_MENU_LOAD;
                        gModeStep = 0;
                        break;
                }
                break;
        }
        FntFlush(-1);
        UpdateCdLoad();
        UpdateSoundFade();
        EndFrame();
    }
}

extern s32 sBootFileLoc[];
extern s32 sBootFileSectors[];
void func_80013CDC(void);

/* MATCHING: model types the pack as its own PackEntry. */
void LoadTimPack(void *pack);

s8 LoadBootData(void) {
    /* MATCHING: a byte, so the later constant 1 is not copied from it. */
    s8 ret;

    ret = 0;
    switch (gSubStep) {
        case 0:
            gCdLoadLoc[0] = (s32)&sBootFileLoc[0];
            gCdLoadDest[0] = 0x8014D000;
            gCdLoadLoc[1] = (s32)&sBootFileLoc[1];
            gCdLoadDest[1] = 0x8018D000;
            gCdLoadLoc[2] = (s32)&sBootFileLoc[2];
            gCdLoadDest[2] = 0x80123000;
            gCdLoadLoc[3] = (s32)&sBootFileLoc[3];
            gCdLoadDest[3] = 0x8016D000;
            gCdLoadLoc[4] = (s32)&sBootFileLoc[4];
            gCdLoadDest[4] = 0x80101000;
            gCdLoadCount = 5;
            gCdLoadSectors[0] = sBootFileSectors[0];
            gCdLoadSectors[1] = sBootFileSectors[1];
            gCdLoadSectors[2] = sBootFileSectors[2];
            gCdLoadSectors[3] = sBootFileSectors[3];
            gCdLoadSectors[4] = sBootFileSectors[4];
            gCdLoadState = CD_LOAD_SEEK;
            gSubStep = 1;
            break;
        case 1:
            if (gCdLoadState == CD_LOAD_DONE) {
                func_80018DE8(0);
                gSubStep++;
            }
            break;
        case 2:
            ret = 1;
            func_80013CDC();
            LoadTimPack((void *)0x8018D000);
            LoadTimList((unsigned long *)(*(s32 *)0x8016D000 + 0x8016D000));
            DefineHudSprites();
            func_80018DE8(1);
            break;
    }
    return ret;
}

/** @brief A model object with its own coordinate system (a unit-local
 *         copy of object's view). */
typedef struct {
    GsDOBJ2 obj;         /**< the object handler */
    GsCOORDINATE2 coord; /**< the object's coordinate system */
    SVECTOR rot;         /**< rotation */
    SVECTOR scale;       /**< scale, 0x1000 = 1 */
} Model70;

/** @brief A 16-byte directory entry of a loaded file. */
typedef struct {
    s32 offset;   /**< byte offset of the entry from the directory */
    u8 unk4[0xA]; /**< not yet known */
    u16 count;    /**< entry count; read from the first entry only */
} DirEnt16;

/* MATCHING: per-unit view; model.c reads it as a word array. */
extern Model70 D_800963A0;

/** @brief A 0x78-byte model slot: a Model70 and 8 bytes not yet known. */
typedef struct {
    Model70 m;   /**< the model */
    u8 unk70[8]; /**< not yet known */
} ModelSlot;

/* MATCHING: player has its own view of these slots. */
extern ModelSlot gModeObjectModels[];
extern u8 gTodObjs[];        /**< the 20 GsDOBJ2s of gTodObjTable */
extern u8 gTodCoordParams[]; /**< a GsCOORD2PARAM for each of gTodCoords */

/* MATCHING: object defines it on its own Model70 view. */
void InitModel(Model70 *m, unsigned long *tmd, u8 n);
/* MATCHING: model types the list, slots and data as its own records. */
void InitTodObjTable(s32 *list, u8 *slots, GsCOORDINATE2 *objs, u8 *data, s32 n);

void func_80013CDC(void) {
    /* MATCHING: one pointer reused for the pack, its second word and the
     * directory, so all three share a saved register. */
    s32 *p;
    s32 *q;

    p = (s32 *)0x8014D000;
    gModelTmd = (s32)((unsigned long *)((u8 *)p + *p) + 1);
    GsMapModelingData((unsigned long *)gModelTmd);
    InitModel(&D_800963A0, (unsigned long *)((u8 *)p + *p), 16);
    p = (s32 *)0x8014D010;
    InitModel(&gModeObjectModels[0].m, (unsigned long *)(*p + 0x8014D000), 0);
    InitModel(&gModeObjectModels[1].m, (unsigned long *)(*p + 0x8014D000), 1);
    InitModel(&gModeObjectModels[2].m, (unsigned long *)(*p + 0x8014D000), 2);
    InitModel(&gModeObjectModels[3].m, (unsigned long *)(*p + 0x8014D000), 3);
    InitModel(&gModeObjectModels[4].m, (unsigned long *)(*p + 0x8014D000), 4);
    InitModel(&gModeObjectModels[5].m, (unsigned long *)(*p + 0x8014D000), 5);
    InitModel(&gModeObjectModels[6].m, (unsigned long *)(*p + 0x8014D000), 6);
    InitTodObjTable(gTodObjTable, gTodObjs, gTodCoords, gTodCoordParams, 20);
    p = (s32 *)0x80123000;
    gLoopI = 0;
    gLoopJ = ((DirEnt16 *)p)->count;
    for (; gLoopI < gLoopJ; gLoopI++) {
        /* MATCHING: assigned inside the store, as in LoadStageSprites. */
        gAnimations[gLoopI] = (q = (s32 *)((u8 *)0x80123000 + *p)) + 1;
        p += 4;
        gPlayer.motionLen[gLoopI] = *gAnimations[gLoopI];
        gAnimations[gLoopI]++;
        gPlayer.motionStart[gLoopI] = gAnimations[gLoopI][1];
    }
    GsInitCoordinate2(WORLD, &gTodCoords[0]);
    gPlayer.action = 0;
    gPlayer.motion[0] = 0;
    /* MATCHING: retail stores 8 before 7. */
    gPlayer.curMotion[0] = 0xFF;
    gPlayer.motion[1] = 0;
    gPlayer.curMotion[1] = 0xFF;
    StepAnimation(&gPlayer, 0, 0);
}

/* Sony's (libgs, carved as asm): GsInitGraph by its arguments. */
void func_80056774(s32 w, s32 h, s32 intmode, s32 dith, s32 vram);

extern GsOT_TAG gOtTags[2][0x1000];
extern GsOT_TAG gSubOtTags[2][0x1000];
extern PACKET gGpuWork[2][67200];

void InitGraphics(void) {
    s32 i;

    func_80056774(320, 240, 4, 1, 0);
    GsDefDispBuff(0, 0, 0, 240);
    for (i = 0; i < 2; i++) {
        gSubOt[i].length = 12;
        gSubOt[i].org = gSubOtTags[i];
        gSubOt[i].point = 0;
        gSubOt[i].offset = 0;
        gOt[i].length = 12;
        gOt[i].org = gOtTags[i];
        /* MATCHING: retail stores this field twice. */
        gSubOt[i].point = 300;
        gOt[i].offset = 0;
    }
    gActiveBuff = GsGetActiveBuff();
    GsSetWorkBase(gGpuWork[gActiveBuff]);
    GsClearOt(0, 300, &gOt[gActiveBuff]);
    GsClearOt(0, 0, &gSubOt[gActiveBuff]);
    gCurOt = &gOt[gActiveBuff];
}

/* Sony's (libgs, carved as asm): GsSortClear by its arguments. */
void func_80056B8C(u8 r, u8 g, u8 b, GsOT *ot);

extern DR_STP sDrawStpOn;
extern DR_STP sDrawStpOff;

void EndFrame(void) {
    GsRVIEW2 view;

    switch (gLoading) {
        case 0:
            if (gGameMode != GM_STAGE) {
                GsSetRefView2((GsRVIEW2 *)gRefView);
            }
            break;
        case 1:
            view.vpx = 0;
            view.vpy = 0;
            view.vpz = 400;
            view.rz = 0;
            view.vrx = 0;
            view.vry = 0;
            view.vrz = 0;
            GsSetRefView2(&view);
            break;
    }
    gVSyncTime = VSync(gVSyncMode);
    DrawSync(0);
    GsSwapDispBuff();
    func_80056B8C(gClearR, gClearG, gClearB, &gOt[gActiveBuff]);
    SetDrawStp(&sDrawStpOn, 1);
    AddPrim(&gOt[gActiveBuff].org[0xFFF], &sDrawStpOn);
    SetLetterbox(ClipF);
    GsDrawOt(&gOt[gActiveBuff]);
    gActiveBuff = GsGetActiveBuff();
    GsSetWorkBase(gGpuWork[gActiveBuff]);
    GsClearOt(0, 300, &gOt[gActiveBuff]);
    GsClearOt(0, 0, &gSubOt[gActiveBuff]);
    GsSortOt(&gSubOt[gActiveBuff], &gOt[gActiveBuff]);
    gCurOt = &gOt[gActiveBuff];
    SetDrawStp(&sDrawStpOff, 0);
    AddPrim(gSubOt[gActiveBuff].org, &sDrawStpOff);
    ReadPad(0);
    RemapButtons();
    if (gPauseState == 0) {
        gFrameCount++;
    }
}

/* MATCHING: declared at most 8 bytes, so each base is one `la` register. */
extern s32 gPadPrev[2];
extern s32 gPadPressed[2];
extern s32 gPadHeld[2];

extern s8 gRepeatSpeed;
extern u8 gRepeatUp;
extern u8 gRepeatDown;
extern u8 gRepeatSides[2];
extern u8 gRepeatButtons[3];

void PollPads(void);

/**
 * @brief Auto-repeat for one held button: once @p cnt has counted 30
 *        frames (scaled by the speed byte), @p bit repeats into the
 *        pressed-flags word every few frames. A release clears @p cnt.
 */
/* MATCHING: a bare block; `do { } while (0)` changes the branch layout. */
#define REPEAT(bit, cnt)                                                               \
    {                                                                                  \
        if (gPad & (bit)) {                                                            \
            if ((cnt) >= 30 / gRepeatSpeed && gFrameCount % (4 / gRepeatSpeed) == 0) { \
                gPadTrig |= (bit);                                                     \
            } else if ((cnt) < 30 / gRepeatSpeed) {                                    \
                (cnt)++;                                                               \
            }                                                                          \
        } else {                                                                       \
            (cnt) = 0;                                                                 \
        }                                                                              \
    }

void ReadPad(s32 port) {
    gRepeatSpeed = 4;
    PollPads();
    if (gPadType == 0x41) {
        gStickRawX = gStickRawY = 0x80;
    }
    /* MATCHING: this order builds gPadPressed's address first, as retail;
     * the stores still come out gPad first. */
    gPadTrig = gPadPressed[(u16)port];
    gPad = gPadHeld[(u16)port];
    REPEAT(PADLup, gRepeatUp)
    REPEAT(PADLdown, gRepeatDown)
    REPEAT(PADLleft, gRepeatSides[0])
    REPEAT(PADLright, gRepeatSides[1])
    if (gRepeatButtonsOn == 1) {
        REPEAT(PADRleft, gRepeatButtons[0])
        REPEAT(PADRdown, gRepeatButtons[1])
        REPEAT(PADRright, gRepeatButtons[2])
    }
}

void InitGraphics(void);
void WaitFrames(s16 frames);

void AdjustScreenPosition(void) {
    s32 x;
    s32 y;
    s32 pad;

    x = 0;
    y = 0;
    WaitFrames(20);
    for (;;) {
        pad = gPadHeld[0];
        if (pad & PADLright) {
            x += 8;
        }
        if (pad & PADLleft) {
            x -= 8;
        }
        if (pad & PADLup) {
            y -= 8;
        }
        if (pad & PADLdown) {
            y += 8;
        }
        if (pad & PADstart) {
            break;
        }
        x = x < 0 ? 0 : x > 0x2C0 ? 0x2C0 : x;
        y = y < 0 ? 0 : y > 0x110 ? 0x110 : y;
        GsDefDispBuff(x, y, x, y);
        ReadPad(0);
        DrawSync(0);
        VSync(0);
        GsSwapDispBuff();
    }
    InitGraphics();
    GsInit3D();
    InitView();
    InitLights();
    WaitFrames(5);
}

extern char sTab[];           /**< "\t" */
extern char sWhiteLineText[]; /**< "~c888\n" */
extern char sHexAddrFmt[];    /**< "%x\n" */
extern char sHexByteFmt[];    /**< " %2x" */
extern s32 sDumpOffset;

void DumpMemory(u8 *base) {
    u8 *p;
    s32 row;
    s32 col;
    s16 i;
    s16 j;

    sDumpOffset = 0;
    FntPrint(sWhiteLineText);
    p = base + sDumpOffset;
    FntPrint(sHexAddrFmt, p);
    for (i = 0; i < 1; i++) {
        FntPrint(sTab);
    }
    for (row = 0; row < 16; row++) {
        for (col = 0; col < 8; col++) {
            FntPrint(sHexByteFmt, *p);
            p++;
        }
        FntPrint(sNewline);
        for (j = 0; j < 1; j++) {
            FntPrint(sTab);
        }
    }
}

void InitPads(void);
void ClearVram(void);
void InitDebugFont(void);

extern u8 D_80095820;

void InitSystem(void) {
    ResetCallback();
    bzero((u8 *)0x80101000, 0xFC800);
    InitCARD(1);
    StartCARD();
    MemCardInit(1);
    MemCardStart();
    ChangeClearPAD(0);
    _bu_init();
    InitPads();
    InitSound();
    CdInit();
    SsStart();
    gClearB = 0;
    gClearG = 0;
    gClearR = 0;
    InitGraphics();
    GsInit3D();
    ClearVram();
    D_80095820 = 0;
    InitDebugFont();
}

void WaitFrames(s16 frames) {
    s16 i;

    for (i = 0; i < frames; i++) {
        ReadPad(0);
        VSync(0);
    }
}

void PrintTabs(s16 count) {
    s16 i;

    for (i = 0; i < count; i++) {
        FntPrint(sTab);
    }
}

/** @brief 15 bytes copied as one block, for a local copy of a table. */
typedef struct {
    u8 b[15]; /**< the bytes */
} Bytes15;

extern u8 sStageMusicTracks[];

void PlayCdTrack(s32 track, s32 loop);

void PlayStageMusic(u8 idx) {
    Bytes15 tbl;

    tbl = *(Bytes15 *)sStageMusicTracks;
    if (gSoundFade != 0) {
        gSoundFade = 0;
    }
    PlayCdTrack(tbl.b[idx], 1);
}

void ResetClearColor(void) {
    /* MATCHING: retail reserves an 8-byte frame it never touches. */
    s32 unused[2];

    gVSyncMode = 2;
    gClearB = 0;
    gClearG = 0;
    gClearR = 0;
}

void ClearVram(void) {
    RECT rect;

    rect.x = 0;
    rect.y = 0;
    rect.w = 0x400;
    rect.h = 0x200;
    ClearImage(&rect, 0, 0, 0);
    DrawSync(0);
}

void InitDebugFont(void) {
    FntLoad(0x3C0, 0);
    FntOpen(-0x9A, -0x74, 0x140, 0x100, 0, 0x400);
}

/**
 * @brief One step of a vibration pattern: vibrate for @c duration frames,
 *        then after @c wait frames repeat the step or go on to @c next.
 */
typedef struct {
    u16 wait;     /**< frames from this step's start to the next one */
    u16 duration; /**< frames the motors run */
    u8 large;     /**< the large motor's strength */
    u8 small;     /**< the small motor's setting */
    u8 repeat;    /**< times the step runs */
    u8 pad7;      /**< padding */
    u16 next;     /**< the step that follows; 0 ends the pattern */
} RumbleStep;

extern RumbleStep *gRumblePattern;
extern u16 gRumbleIndex;
extern u16 gRumbleWait;
extern u16 gRumbleTime;
/* MATCHING: signed, so the zero test in UpdateRumble is its own `lh`. */
extern s16 gRumbleNext;
extern u16 gRumbleRepeat;

/**
 * @brief Starts step @p index of pattern @p tbl, unpacking it into the
 *        rumble globals. StartRumble's body; inlined with the pattern
 *        already current as @p tbl, its store of the pattern drops out.
 */
static __inline__ void setRumble(RumbleStep *tbl, u16 index) {
    u8 *rec;

    gRumblePattern = tbl;
    tbl += index;
    gRumbleIndex = index;
    gRumbleWait = tbl->wait;
    gRumbleTime = tbl->duration;
    /* MATCHING: byte-pointer reads keep each load below the prior store. */
    rec = (u8 *)tbl;
    gRumbleSmall = rec[5];
    gRumbleLarge = rec[4];
    gRumbleNext = *(u16 *)(rec + 8);
    gRumbleRepeat = rec[6];
}

/**
 * @brief setRumble's twin for repeating the current step: the same
 *        unpacking, with the repeat count @p count stepped down instead of
 *        reloaded. Inlined, so its parameter conversions survive.
 */
static __inline__ void repeatRumble(RumbleStep *tbl, u16 index, u8 count) {
    u8 *rec;

    gRumblePattern = tbl;
    tbl += index;
    gRumbleIndex = index;
    gRumbleWait = tbl->wait;
    gRumbleTime = tbl->duration;
    rec = (u8 *)tbl;
    gRumbleSmall = rec[5];
    gRumbleLarge = rec[4];
    gRumbleNext = *(u16 *)(rec + 8);
    gRumbleRepeat = count - 1;
}

/* MATCHING: s32 with a bare return, so the `bgtz` keeps its `nop` slot. */
s32 UpdateRumble(void) {
    if ((s16)gRumbleTime > 0 && gRumbleEnabled == 1) {
        SetPadVibration(0, gRumbleSmall, gRumbleLarge);
        gRumbleTime--;
        return;
    }
    gRumbleTime = 0;
    gRumbleSmall = 0;
    gRumbleLarge = 0;
    SetPadVibration(0, 0, 0);
    if ((s16)gRumbleRepeat > 0) {
        if ((s16)--gRumbleWait <= 0) {
            if ((s16)gRumbleRepeat < 2) {
                gRumbleRepeat = 0;
                if (gRumbleNext != 0) {
                    setRumble(gRumblePattern, gRumbleNext);
                }
            } else {
                repeatRumble(gRumblePattern, gRumbleIndex, gRumbleRepeat);
            }
        }
    } else {
        gRumbleWait = 0;
    }
}

extern u8 sPadBufOffsets[];
extern u8 gPadBuf[];
extern s32 gStickY;
extern s32 gStickX;
/* MATCHING: editor declares it too; both reach it the same way. */
extern s32 gStickTrigX;

extern s32 sPad2Prev;
extern s32 sPad2Held;
extern s32 sPad2Pressed;

u32 UpdatePadActuators(void);

/**
 * @brief Per-frame pad read: polls both pads, keeps port 0's id byte,
 *        latches each port's buttons (active high) and the
 *        newly pressed ones, and for an analog pad (id 0x73) turns the
 *        stick's extremes into direction bits and keeps the raw axes.
 */
void PollPads(void) {
    u8 id;
    s32 btn;
    s32 m;
    s32 b;

    UpdatePadActuators();
    id = gPadBuf[1];
    gPadType = id;
    /* MATCHING: built up in one local over three statements; as one
     * expression, combine drops port 1's `andi 0xFFFF`. */
    btn = gPadBuf[sPadBufOffsets[0] + 2] << 8;
    btn |= gPadBuf[sPadBufOffsets[0] + 3];
    btn ^= 0xFFFF;
    if (id == 0) {
        btn = 0;
    }
    m = gPadPrev[0] ^ 0xFFFF;
    /* MATCHING: a new local, not `btn = (u16)btn;`: it takes $v1 in
     * local-alloc, so the sPad2Prev load lands in $a0 and is hoisted. */
    b = (u16)btn;
    gPadHeld[0] = gPadPrev[0] = b;
    gPadPressed[0] = b & m;
    m = sPad2Prev ^ 0xFFFF;
    btn = gPadBuf[sPadBufOffsets[1] + 0x24] << 8;
    btn |= gPadBuf[sPadBufOffsets[1] + 0x25];
    btn ^= 0xFFFF;
    sPad2Held = sPad2Prev = (u16)btn;
    btn = (u16)btn & m;
    sPad2Pressed = btn;
    if (id == 0x73) {
        m = gStickY ^ 0xFFFF;
        if (gPadBuf[7] < 10) {
            gStickY = 0x1000;
        } else if (gPadBuf[7] >= 0xF6) {
            gStickY = 0x4000;
        } else {
            gStickY = 0;
        }
        gStickTrigY = gStickY & m;
        m = gStickX ^ 0xFFFF;
        if (gPadBuf[6] < 10) {
            gStickX = 0x8000;
        } else if (gPadBuf[6] >= 0xF6) {
            gStickX = 0x2000;
        } else {
            gStickX = 0;
        }
        gStickTrigX = gStickX & m;
        gStickRawX = gPadBuf[6];
        gStickRawY = gPadBuf[7];
    }
}

extern u8 *sPadBufPtr;
extern u8 *sPadActPtr;
/* MATCHING: a known small size, so its address is one `la` register. */
extern u8 sPadActAlign[6];

u32 UpdatePadActuators(void) {
    u8 *buf;
    u8 *act;
    s32 k;
    s32 state;
    s32 i;

    buf = sPadBufPtr;
    act = sPadActPtr;
    for (k = 0; k < 2; k++, act += 0x10) {
        state = PadGetState(k * 16);
        if (state == 1) {
            act[0] = 0;
        }
        if (act[0] == 0) {
            PadSetAct(k * 16, act + 2, 2);
            if (state == 2 || (state == 6 && PadSetActAlign(k * 16, sPadActAlign))) {
                act[0] = 1;
            }
        }
        if (buf[k * 0x22] != 0) {
            for (i = 2; i < 8; i++) {
                buf[k * 0x22 + i] = 0xFF;
            }
        } else if ((buf[k * 0x22 + 1] & 0xF0) == 0x70) {
            for (i = 4; i < 8; i++) {
                if ((u32)(buf[k * 0x22 + i] - 0x69) < 0x2F) {
                    buf[k * 0x22 + i] = 0x80;
                }
            }
        }
    }
    return ~((buf[0x24] << 24) | (buf[0x25] << 16) | (buf[2] << 8) | buf[3]);
}

void SetPadVibration(s32 offset, u8 a, u8 b) {
    sPadActPtr[offset + 2] = a;
    sPadActPtr[offset + 3] = b;
}

void StartPads(u8 *a, u8 *b);

extern u8 sPadActBuf[];

void InitPads(void) {
    s16 i;

    StartPads(gPadBuf, sPadActBuf);
    for (i = 0; i < 6; i++) {
        gPadPrev[i] = 0;
        gPadPressed[i] = 0;
        gPadHeld[i] = 0;
    }
}

void SetPadMainMode(s32 mode) {
    mode &= 1;
    gLoopI = 0;
    do {
        VSync(0);
        if (PadGetState(0) == 6) {
            PadSetMainMode(0, mode, 0);
            return;
        }
        gLoopI++;
    } while (gLoopI < 10);
}

void StartRumble(RumbleStep *tbl, u16 index) {
    u8 *rec;

    gRumblePattern = tbl;
    tbl += index;
    gRumbleIndex = index;
    gRumbleWait = tbl->wait;
    gRumbleTime = tbl->duration;
    /* MATCHING: byte-pointer reads keep each load below the prior store. */
    rec = (u8 *)tbl;
    gRumbleSmall = rec[5];
    gRumbleLarge = rec[4];
    gRumbleNext = *(u16 *)(rec + 8);
    gRumbleRepeat = rec[6];
}

void ResetRumble(void) {
    RumbleStep *hdr;

    hdr = (RumbleStep *)gRumblePatterns;
    gRumblePattern = hdr;
    gRumbleIndex = 0;
    gRumbleWait = hdr->wait;
    gRumbleTime = hdr->duration;
    gRumbleSmall = hdr->small;
    gRumbleLarge = hdr->large;
    gRumbleNext = hdr->next;
    gRumbleRepeat = hdr->repeat;
}

void StartPads(u8 *a, u8 *b) {
    s32 i;

    sPadBufPtr = a;
    sPadActPtr = b;
    for (i = 0; i < 32; i++) {
        sPadActPtr[i] = 0;
    }
    PadInitDirect(sPadBufPtr, sPadBufPtr + 0x22);
    PadStartCom();
}

INCLUDE_RODATA("asm/nonmatchings/main", sCantOpenFmt);

extern s16 gCdLoadIndex;
extern s32 gCdLoadReadSectors;
extern u8 gCdLoadRetries;

s8 UpdateCdLoad(void) {
    s32 r;

    switch (gCdLoadState) {
        case CD_LOAD_IDLE:
            break;
        case CD_LOAD_SEEK:
            CdControlF(CdlSetloc, (u_char *)gCdLoadLoc[gCdLoadIndex]);
            gCdLoadState++;
            break;
        case CD_LOAD_SEEK_WAIT:
            r = CdSync(1, NULL);
            if (r == CdlComplete) {
                gCdLoadState++;
            } else if (r == CdlDiskError) {
                gCdLoadState = CD_LOAD_SEEK;
                gCdLoadRetries++;
            }
            break;
        case CD_LOAD_READ:
            gCdLoadReadSectors = gCdLoadSectors[gCdLoadIndex] + 1;
            if (CdRead(gCdLoadReadSectors, (u_long *)gCdLoadDest[gCdLoadIndex], CdlModeSpeed) == 0) {
                gCdLoadRetries++;
            } else {
                gCdLoadState++;
            }
            break;
        case CD_LOAD_READ_WAIT:
            r = CdReadSync(1, NULL);
            if (r == 0) {
                gCdLoadState++;
            } else if (r == -1) {
                gCdLoadState = CD_LOAD_SEEK;
                gCdLoadRetries++;
            }
            break;
        case CD_LOAD_NEXT:
            if (++gCdLoadIndex >= gCdLoadCount) {
                gCdLoadState = CD_LOAD_DONE;
                gCdLoadIndex = 0;
            } else {
                gCdLoadState = CD_LOAD_SEEK;
            }
            break;
    }
    return gCdLoadState;
}

extern char sCantOpenFmt[]; /**< "Can't open %s\n" */

void LoadFile(char *name, void *buf) {
    s32 fd;

    fd = open(name, 1);
    if (fd == -1) {
        for (;;) {
            GsSwapDispBuff();
            FntPrint(sCantOpenFmt, name);
            FntFlush(-1);
        }
    }
    read(fd, buf, 0x100000);
    close(fd);
}

extern PlayerState gLoadScreenModel;

void BeginLoadScreenModel(void) {
    SVECTOR pos;
    RECT rect;

    DefineSprite(0x1FE, 2, 0xA0, 0xF0, 10, 0, 0, 0, 0);
    DefineSprite(0x1FF, 2, 0xA0, 0xF0, 12, 0x20, 0, 0, 0);
    rect.x = 0;
    rect.y = PSDIDX * 240;
    rect.w = 320;
    rect.h = 240;
    MoveImage(&rect, 640, 0);
    pos.vx = -160;
    pos.vy = -120;
    DrawSprite(0x1FE, &pos, NULL, 0, &gSubOt[gActiveBuff]);
    pos.vx = 0;
    pos.vy = -120;
    DrawSprite(0x1FF, &pos, NULL, 0, &gSubOt[gActiveBuff]);
    gLoadScreenModel = gPlayer;
    gLoadScreenModel.yaw = 0x800;
    gLoadScreenModel.motion[0] = 3;
    gLoadScreenModel.x = 0;
    gLoadScreenModel.y = 0;
    gLoadScreenModel.z = 0;
    gLoadScreenModel.action = 0x61;
    gLoadScreenModel.speed = 0;
    gLoadScreenModel.control = 0;
    gRefView[0] = rsin(0x800) * 400 / 4096;
    gRefView[1] = -150;
    gRefView[2] = rcos(0x800) * 400 / 4096;
    gRefView[3] = 0;
    gRefView[4] = -150;
    gRefView[5] = 0;
    gSubStep = 0;
    gGameMode = GM_LOAD_SCREEN_MODEL;
}

void BeginLoadScreen(void) {
    SVECTOR pos;
    RECT rect;

    DefineSprite(0x1FE, 2, 0xA0, 0xF0, 10, 0, 0, 0, 0);
    DefineSprite(0x1FF, 2, 0xA0, 0xF0, 12, 0x20, 0, 0, 0);
    rect.x = 0;
    rect.y = PSDIDX * 240;
    rect.w = 320;
    rect.h = 240;
    MoveImage(&rect, 640, 0);
    pos.vx = -160;
    pos.vy = -120;
    DrawSprite(0x1FE, &pos, NULL, 0, &gSubOt[gActiveBuff]);
    pos.vx = 0;
    pos.vy = -120;
    DrawSprite(0x1FF, &pos, NULL, 0, &gSubOt[gActiveBuff]);
    gSubStep = 0;
    gGameMode = GM_LOAD_SCREEN;
}

void BeginAct3LoadScreen(void) {
    SVECTOR pos;
    RECT rect;

    DefineSprite(0x1FE, 2, 0xA0, 0xF0, 10, 0, 0, 0, 0);
    DefineSprite(0x1FF, 2, 0xA0, 0xF0, 12, 0x20, 0, 0, 0);
    rect.x = 0;
    rect.y = PSDIDX * 240;
    rect.w = 320;
    rect.h = 240;
    MoveImage(&rect, 640, 0);
    pos.vx = -160;
    pos.vy = -120;
    DrawSprite(0x1FE, &pos, NULL, 0, &gSubOt[gActiveBuff]);
    pos.vx = 0;
    pos.vy = -120;
    DrawSprite(0x1FF, &pos, NULL, 0, &gSubOt[gActiveBuff]);
    gSubStep = 0;
    gGameMode = GM_LOAD_SCREEN_ACT3;
}

/* The act-3 stages' files (2, 5, 8, 11, 14), for LoadAct3Stage: each
 * file's disc position, then its size in sectors, one pair per stage. */
extern CdlLOC sStage02FileLoc[];
extern s32 sStage02FileSectors[];
extern CdlLOC sStage05FileLoc[];
extern s32 sStage05FileSectors[];
extern CdlLOC sStage08FileLoc[];
extern s32 sStage08FileSectors[];
extern CdlLOC sStage11FileLoc[];
extern s32 sStage11FileSectors[];
extern CdlLOC sStage14FileLoc[];
extern s32 sStage14FileSectors[];

/* MATCHING: sound defines it with a u16 bank; this caller passes it
 * unextended, so its view is s32. */
s32 LoadVabBank(s32 bank);
void LoadAct3Data(unsigned long *p);
void ResetStage(void);
void func_800F6C60(void);
void func_800F6A44(void);
void func_800F642C(void);
void func_800F46D8(void);
void func_800F377C(void);

void LoadAct3Stage(void) {
    s32 bank;

    switch (gSubStep) {
        case 0:
            gVabLoadStep = 0;
            gSubStep++;
            break;
        case 1:
            switch (gStage) {
                case 2:
                    bank = 2;
                    break;
                case 5:
                    bank = 3;
                    break;
                case 8:
                    bank = 4;
                    break;
                case 11:
                    bank = 5;
                    break;
                case 14:
                    bank = 6;
                    break;
            }
            if (LoadVabBank(bank) == 1) {
                gSubStep++;
            }
            break;
        case 2:
            LoadStageOverlay(gStage);
            gSubStep++;
            break;
        case 3:
            if (gCdLoadState == CD_LOAD_DONE) {
                gSubStep++;
            }
            break;
        case 4:
            func_80018DE8(0);
            switch (gStage) {
                case 2:
                    gCdLoadLoc[0] = (s32)&sStage02FileLoc[0];
                    gCdLoadSectors[0] = sStage02FileSectors[0];
                    break;
                case 5:
                    gCdLoadLoc[0] = (s32)&sStage05FileLoc[0];
                    gCdLoadSectors[0] = sStage05FileSectors[0];
                    break;
                case 8:
                    gCdLoadLoc[0] = (s32)&sStage08FileLoc[0];
                    gCdLoadSectors[0] = sStage08FileSectors[0];
                    break;
                case 11:
                    gCdLoadLoc[0] = (s32)&sStage11FileLoc[0];
                    gCdLoadSectors[0] = sStage11FileSectors[0];
                    break;
                case 14:
                    gCdLoadLoc[0] = (s32)&sStage14FileLoc[0];
                    gCdLoadSectors[0] = sStage14FileSectors[0];
                    break;
            }
            gCdLoadDest[0] = 0x8019D000;
            gCdLoadCount = 1;
            gCdLoadState = CD_LOAD_SEEK;
            gSubStep++;
            break;
        case 5:
            if (gCdLoadState == CD_LOAD_DONE) {
                LoadTimList((unsigned long *)0x8019D000);
                gSubStep++;
            }
            break;
        case 6:
            switch (gStage) {
                case 2:
                    gCdLoadLoc[0] = (s32)&sStage02FileLoc[1];
                    gCdLoadSectors[0] = sStage02FileSectors[1];
                    break;
                case 5:
                    gCdLoadLoc[0] = (s32)&sStage05FileLoc[1];
                    gCdLoadSectors[0] = sStage05FileSectors[1];
                    break;
                case 8:
                    gCdLoadLoc[0] = (s32)&sStage08FileLoc[1];
                    gCdLoadSectors[0] = sStage08FileSectors[1];
                    break;
                case 11:
                    gCdLoadLoc[0] = (s32)&sStage11FileLoc[1];
                    gCdLoadSectors[0] = sStage11FileSectors[1];
                    break;
                case 14:
                    gCdLoadLoc[0] = (s32)&sStage14FileLoc[1];
                    gCdLoadSectors[0] = sStage14FileSectors[1];
                    break;
            }
            gCdLoadDest[0] = 0x8019D000;
            gCdLoadCount = 1;
            gCdLoadState = CD_LOAD_SEEK;
            gSubStep++;
            break;
        case 7:
            if (gCdLoadState == CD_LOAD_DONE) {
                LoadAct3Data((unsigned long *)0x8019D000);
                switch (gStage) {
                    case 2:
                        func_800F6C60();
                        break;
                    case 5:
                        func_800F6A44();
                        break;
                    case 8:
                        func_800F642C();
                        break;
                    case 11:
                        func_800F46D8();
                        break;
                    case 14:
                        func_800F377C();
                        break;
                }
                ResetStage();
                gSubStep = 100;
            }
            break;
    }
}

/* The other stages' files, for LoadStage: disc positions, and sizes in
 * sectors counted from an extra leading word, one pair per stage. */
extern CdlLOC sStage00FileLoc[];
extern s32 sStage00FileSectors[];
extern CdlLOC sStage01FileLoc[];
extern s32 sStage01FileSectors[];
extern CdlLOC sStage03FileLoc[];
extern s32 sStage03FileSectors[];
extern CdlLOC sStage04FileLoc[];
extern s32 sStage04FileSectors[];
extern CdlLOC sStage06FileLoc[];
extern s32 sStage06FileSectors[];
extern CdlLOC sStage07FileLoc[];
extern s32 sStage07FileSectors[];
extern CdlLOC sStage09FileLoc[];
extern s32 sStage09FileSectors[];
extern CdlLOC sStage10FileLoc[];
extern s32 sStage10FileSectors[];
extern CdlLOC sStage12FileLoc[];
extern s32 sStage12FileSectors[];
extern CdlLOC sStage13FileLoc[];
extern s32 sStage13FileSectors[];

/* MATCHING: stage defines it and has no header to declare it. */
void LoadStageData(unsigned long *p);
/* MATCHING: a per-unit view; DirEnt16 is this unit's view of the directory. */
void LoadSpritePack(DirEnt16 *dir, u16 id);
/* MATCHING: editor types both as its own BankFile. */
void SetHitBanks(void *a, void *b);
void RestoreStageObjects(void);
s32 LoadStageSprites(void);
void ResetStageObjects(void);
void StartStageOverlay(void);

void LoadStage(void) {
    switch (gSubStep) {
        case 0:
            gVabLoadStep = 0;
            gSubStep++;
        case 1:
            if (LoadVabBank(GetStageVabBank(gStage)) == 1) {
                gSubStep++;
            }
            break;
        case 2:
            LoadStageOverlay(gStage);
            gSubStep++;
            break;
        case 3:
            if (gCdLoadState == CD_LOAD_DONE) {
                gSubStep++;
            }
            break;
        case 4:
            func_80018DE8(0);
            switch (gStage) {
                case 0:
                    gCdLoadLoc[0] = (s32)&sStage00FileLoc[0];
                    gCdLoadSectors[0] = sStage00FileSectors[1];
                    break;
                case 1:
                    gCdLoadLoc[0] = (s32)&sStage01FileLoc[0];
                    gCdLoadSectors[0] = sStage01FileSectors[1];
                    break;
                case 3:
                    gCdLoadLoc[0] = (s32)&sStage03FileLoc[0];
                    gCdLoadSectors[0] = sStage03FileSectors[1];
                    break;
                case 4:
                    gCdLoadLoc[0] = (s32)&sStage04FileLoc[0];
                    gCdLoadSectors[0] = sStage04FileSectors[1];
                    break;
                case 6:
                    gCdLoadLoc[0] = (s32)&sStage06FileLoc[0];
                    gCdLoadSectors[0] = sStage06FileSectors[1];
                    break;
                case 7:
                    gCdLoadLoc[0] = (s32)&sStage07FileLoc[0];
                    gCdLoadSectors[0] = sStage07FileSectors[1];
                    break;
                case 9:
                    gCdLoadLoc[0] = (s32)&sStage09FileLoc[0];
                    gCdLoadSectors[0] = sStage09FileSectors[1];
                    break;
                case 10:
                    gCdLoadLoc[0] = (s32)&sStage10FileLoc[0];
                    gCdLoadSectors[0] = sStage10FileSectors[1];
                    break;
                case 12:
                    gCdLoadLoc[0] = (s32)&sStage12FileLoc[0];
                    gCdLoadSectors[0] = sStage12FileSectors[1];
                    break;
                case 13:
                    gCdLoadLoc[0] = (s32)&sStage13FileLoc[0];
                    gCdLoadSectors[0] = sStage13FileSectors[1];
                    break;
            }
            gCdLoadDest[0] = 0x8019D000;
            gCdLoadCount = 1;
            gCdLoadState = CD_LOAD_SEEK;
            gSubStep++;
            break;
        case 5:
            if (gCdLoadState == CD_LOAD_DONE) {
                LoadTimList((unsigned long *)0x8019D000);
                gSubStep++;
            }
            break;
        case 6:
            switch (gStage) {
                case 0:
                    gCdLoadLoc[0] = (s32)&sStage00FileLoc[1];
                    gCdLoadSectors[0] = sStage00FileSectors[2];
                    break;
                case 1:
                    gCdLoadLoc[0] = (s32)&sStage01FileLoc[1];
                    gCdLoadSectors[0] = sStage01FileSectors[2];
                    break;
                case 3:
                    gCdLoadLoc[0] = (s32)&sStage03FileLoc[1];
                    gCdLoadSectors[0] = sStage03FileSectors[2];
                    break;
                case 4:
                    gCdLoadLoc[0] = (s32)&sStage04FileLoc[1];
                    gCdLoadSectors[0] = sStage04FileSectors[2];
                    break;
                case 6:
                    gCdLoadLoc[0] = (s32)&sStage06FileLoc[1];
                    gCdLoadSectors[0] = sStage06FileSectors[2];
                    break;
                case 7:
                    gCdLoadLoc[0] = (s32)&sStage07FileLoc[1];
                    gCdLoadSectors[0] = sStage07FileSectors[2];
                    break;
                case 9:
                    gCdLoadLoc[0] = (s32)&sStage09FileLoc[1];
                    gCdLoadSectors[0] = sStage09FileSectors[2];
                    break;
                case 10:
                    gCdLoadLoc[0] = (s32)&sStage10FileLoc[1];
                    gCdLoadSectors[0] = sStage10FileSectors[2];
                    break;
                case 12:
                    gCdLoadLoc[0] = (s32)&sStage12FileLoc[1];
                    gCdLoadSectors[0] = sStage12FileSectors[2];
                    break;
                case 13:
                    gCdLoadLoc[0] = (s32)&sStage13FileLoc[1];
                    gCdLoadSectors[0] = sStage13FileSectors[2];
                    break;
            }
            gCdLoadDest[0] = 0x8019D000;
            gCdLoadCount = 1;
            gCdLoadState = CD_LOAD_SEEK;
            gSubStep++;
            break;
        case 7:
            if (gCdLoadState == CD_LOAD_DONE) {
                LoadTimPack((void *)0x8019D000);
                gSubStep++;
            }
            break;
        case 8:
            switch (gStage) {
                case 0:
                    gCdLoadLoc[0] = (s32)&sStage00FileLoc[2];
                    gCdLoadLoc[1] = (s32)&sStage00FileLoc[3];
                    gCdLoadSectors[0] = sStage00FileSectors[3];
                    gCdLoadSectors[1] = sStage00FileSectors[4];
                    break;
                case 1:
                    gCdLoadLoc[0] = (s32)&sStage01FileLoc[2];
                    gCdLoadLoc[1] = (s32)&sStage01FileLoc[3];
                    gCdLoadSectors[0] = sStage01FileSectors[3];
                    gCdLoadSectors[1] = sStage01FileSectors[4];
                    break;
                case 3:
                    gCdLoadLoc[0] = (s32)&sStage03FileLoc[2];
                    gCdLoadLoc[1] = (s32)&sStage03FileLoc[3];
                    gCdLoadSectors[0] = sStage03FileSectors[3];
                    gCdLoadSectors[1] = sStage03FileSectors[4];
                    break;
                case 4:
                    gCdLoadLoc[0] = (s32)&sStage04FileLoc[2];
                    gCdLoadLoc[1] = (s32)&sStage04FileLoc[3];
                    gCdLoadSectors[0] = sStage04FileSectors[3];
                    gCdLoadSectors[1] = sStage04FileSectors[4];
                    break;
                case 6:
                    gCdLoadLoc[0] = (s32)&sStage06FileLoc[2];
                    gCdLoadLoc[1] = (s32)&sStage06FileLoc[3];
                    gCdLoadSectors[0] = sStage06FileSectors[3];
                    gCdLoadSectors[1] = sStage06FileSectors[4];
                    break;
                case 7:
                    gCdLoadLoc[0] = (s32)&sStage07FileLoc[2];
                    gCdLoadLoc[1] = (s32)&sStage07FileLoc[3];
                    gCdLoadSectors[0] = sStage07FileSectors[3];
                    gCdLoadSectors[1] = sStage07FileSectors[4];
                    break;
                case 9:
                    gCdLoadLoc[0] = (s32)&sStage09FileLoc[2];
                    gCdLoadLoc[1] = (s32)&sStage09FileLoc[3];
                    gCdLoadSectors[0] = sStage09FileSectors[3];
                    gCdLoadSectors[1] = sStage09FileSectors[4];
                    break;
                case 10:
                    gCdLoadLoc[0] = (s32)&sStage10FileLoc[2];
                    gCdLoadLoc[1] = (s32)&sStage10FileLoc[3];
                    gCdLoadSectors[0] = sStage10FileSectors[3];
                    gCdLoadSectors[1] = sStage10FileSectors[4];
                    break;
                case 12:
                    gCdLoadLoc[0] = (s32)&sStage12FileLoc[2];
                    gCdLoadLoc[1] = (s32)&sStage12FileLoc[3];
                    gCdLoadSectors[0] = sStage12FileSectors[3];
                    gCdLoadSectors[1] = sStage12FileSectors[4];
                    break;
                case 13:
                    gCdLoadLoc[0] = (s32)&sStage13FileLoc[2];
                    gCdLoadLoc[1] = (s32)&sStage13FileLoc[3];
                    gCdLoadSectors[0] = sStage13FileSectors[3];
                    gCdLoadSectors[1] = sStage13FileSectors[4];
                    break;
            }
            gCdLoadDest[0] = 0x8019D000;
            gCdLoadDest[1] = 0x80155000;
            gCdLoadCount = 2;
            gCdLoadState = CD_LOAD_SEEK;
            gSubStep++;
            break;
        case 9:
            if (gCdLoadState == CD_LOAD_DONE) {
                LoadStageData((unsigned long *)0x8019D000);
                gSubStep++;
            }
            break;
        case 10:
            switch (gStage) {
                case 0:
                    gCdLoadLoc[0] = (s32)&sStage00FileLoc[4];
                    gCdLoadSectors[0] = sStage00FileSectors[5];
                    break;
                case 1:
                    gCdLoadLoc[0] = (s32)&sStage01FileLoc[4];
                    gCdLoadSectors[0] = sStage01FileSectors[5];
                    break;
                case 3:
                    gCdLoadLoc[0] = (s32)&sStage03FileLoc[4];
                    gCdLoadSectors[0] = sStage03FileSectors[5];
                    break;
                case 4:
                    gCdLoadLoc[0] = (s32)&sStage04FileLoc[4];
                    gCdLoadSectors[0] = sStage04FileSectors[5];
                    break;
                case 6:
                    gCdLoadLoc[0] = (s32)&sStage06FileLoc[4];
                    gCdLoadSectors[0] = sStage06FileSectors[5];
                    break;
                case 7:
                    gCdLoadLoc[0] = (s32)&sStage07FileLoc[4];
                    gCdLoadSectors[0] = sStage07FileSectors[5];
                    break;
                case 9:
                    gCdLoadLoc[0] = (s32)&sStage09FileLoc[4];
                    gCdLoadSectors[0] = sStage09FileSectors[5];
                    break;
                case 10:
                    gCdLoadLoc[0] = (s32)&sStage10FileLoc[4];
                    gCdLoadSectors[0] = sStage10FileSectors[5];
                    break;
                case 12:
                    gCdLoadLoc[0] = (s32)&sStage12FileLoc[4];
                    gCdLoadSectors[0] = sStage12FileSectors[5];
                    break;
                case 13:
                    gCdLoadLoc[0] = (s32)&sStage13FileLoc[4];
                    gCdLoadSectors[0] = sStage13FileSectors[5];
                    break;
            }
            gCdLoadDest[0] = 0x8016D000;
            gCdLoadCount = 1;
            gCdLoadState = CD_LOAD_SEEK;
            gSubStep++;
            break;
        case 11:
            if (gCdLoadState == CD_LOAD_DONE) {
                LoadSpritePack((DirEnt16 *)0x8016D000, 30);
                gSubStep++;
            }
            break;
        case 12:
            InitEntryModels();
            gSubStep++;
            break;
        case 13:
            switch (gStage) {
                case 0:
                    gCdLoadLoc[0] = (s32)&sStage00FileLoc[5];
                    gCdLoadSectors[0] = sStage00FileSectors[6];
                    break;
                case 1:
                    gCdLoadLoc[0] = (s32)&sStage01FileLoc[5];
                    gCdLoadSectors[0] = sStage01FileSectors[6];
                    break;
                case 3:
                    gCdLoadLoc[0] = (s32)&sStage03FileLoc[5];
                    gCdLoadSectors[0] = sStage03FileSectors[6];
                    break;
                case 4:
                    gCdLoadLoc[0] = (s32)&sStage04FileLoc[5];
                    gCdLoadSectors[0] = sStage04FileSectors[6];
                    break;
                case 6:
                    gCdLoadLoc[0] = (s32)&sStage06FileLoc[5];
                    gCdLoadSectors[0] = sStage06FileSectors[6];
                    break;
                case 7:
                    gCdLoadLoc[0] = (s32)&sStage07FileLoc[5];
                    gCdLoadSectors[0] = sStage07FileSectors[6];
                    break;
                case 9:
                    gCdLoadLoc[0] = (s32)&sStage09FileLoc[5];
                    gCdLoadSectors[0] = sStage09FileSectors[6];
                    break;
                case 10:
                    gCdLoadLoc[0] = (s32)&sStage10FileLoc[5];
                    gCdLoadSectors[0] = sStage10FileSectors[6];
                    break;
                case 12:
                    gCdLoadLoc[0] = (s32)&sStage12FileLoc[5];
                    gCdLoadSectors[0] = sStage12FileSectors[6];
                    break;
                case 13:
                    gCdLoadLoc[0] = (s32)&sStage13FileLoc[5];
                    gCdLoadSectors[0] = sStage13FileSectors[6];
                    break;
            }
            gCdLoadDest[0] = 0x8016D000;
            gCdLoadCount = 1;
            gCdLoadState = CD_LOAD_SEEK;
            gSubStep++;
            break;
        case 14:
            if (gCdLoadState == CD_LOAD_DONE) {
                SetHitBanks((void *)0x8016D000, (void *)0x80171744);
                RestoreStageObjects();
                LoadStageSprites();
                gSubStep++;
            }
            break;
        case 15:
            if ((u8)(gStage % 3) == 1) {
                switch (gStage) {
                    case 1:
                        gCdLoadLoc[0] = (s32)&sStage01FileLoc[6];
                        gCdLoadSectors[0] = sStage01FileSectors[7];
                        break;
                    case 4:
                        gCdLoadLoc[0] = (s32)&sStage04FileLoc[6];
                        gCdLoadSectors[0] = sStage04FileSectors[7];
                        break;
                    case 7:
                        gCdLoadLoc[0] = (s32)&sStage07FileLoc[6];
                        gCdLoadSectors[0] = sStage07FileSectors[7];
                        break;
                    case 10:
                        gCdLoadLoc[0] = (s32)&sStage10FileLoc[6];
                        gCdLoadSectors[0] = sStage10FileSectors[7];
                        break;
                    case 13:
                        gCdLoadLoc[0] = (s32)&sStage13FileLoc[6];
                        gCdLoadSectors[0] = sStage13FileSectors[7];
                        break;
                }
                gCdLoadDest[0] = 0x8018D000;
            }
            gCdLoadCount = 1;
            gCdLoadState = CD_LOAD_SEEK;
            gSubStep++;
            break;
        case 16:
            if (gCdLoadState == CD_LOAD_DONE) {
                ResetStageObjects();
                StartStageOverlay();
                func_80018DE8(1);
                gSavedStart[0] = 0;
                ResetStage();
                gSubStep = 100;
            }
            break;
    }
}

/** @brief A list of models drawn together (a unit-local view). */
typedef struct {
    GsDOBJ2 *objs; /**< the models */
    s32 count;     /**< how many */
} ModelSet;

/* MATCHING: a struct lvalue over the shared array declaration. */
#define sModels (*(ModelSet *)gTodObjTable)


void LoadStage(void);

/** @brief One step of @p obj: runs the stepper and marks a finished 0x61
 *         channel. Inlined here and by the out-of-line step below. */
static __inline__ void stepObj(PlayerState *obj) {
    /* MATCHING: signed, so the zero test is `sll 24`, not `andi 0xFF`. */
    s8 result;

    result = StepAnimation(obj, 0, obj->unk4);
    obj->motionResult[0] = result;
    switch (obj->action) {
        case 0x60:
            break;
        case 0x61:
            if (result == 0) {
                obj->curMotion[0] = 0xFF;
            }
            break;
    }
}

/** @brief Draws @p obj's models at its position and rotation. Inlined
 *         here and by the out-of-line draw below. */
static __inline__ void drawObj(PlayerState *obj) {
    MATRIX m;
    SVECTOR rot;
    GsDOBJ2 *o;
    s32 i;

    SetTmdSortMode(gTmdSortMode);
    o = sModels.objs;
    o->coord2->coord.t[0] = obj->x;
    o->coord2->coord.t[1] = obj->y;
    o->coord2->coord.t[2] = obj->z;
    rot.vx = 0;
    rot.vy = obj->yaw;
    rot.vz = 0;
    SetCoordRotation(&rot, o->coord2);
    for (i = 0; i < sModels.count; o++, i++) {
        o->coord2->flg = 0;
        if (o->id != -1 && o->tmd != NULL) {
            GsGetLs(o->coord2, &m);
            GsSetLsMatrix(&m);
            GsGetLw(o->coord2, &m);
            GsSetLightMatrix(&m);
            /* MATCHING: retail runs this sort on the scratchpad stack. */
            SetSpadStack();
            GsSortObject4J(o, &gSubOt[gActiveBuff], 2, (u_long *)0x1F800000);
            ResetSpadStack();
        }
    }
}

void UpdateLoadScreenModel(void) {
    /* MATCHING: retail reserves 8 bytes below the inlined draw's locals. */
    s32 unused[2];

    gRefView[0] = rsin(0x800) * 400 / 4096;
    gRefView[1] = -150;
    gRefView[2] = rcos(0x800) * 400 / 4096;
    gRefView[3] = 0;
    gRefView[4] = -150;
    gRefView[5] = 0;
    GsSetRefView2((GsRVIEW2 *)gRefView);
    stepObj(&gLoadScreenModel);
    drawObj(&gLoadScreenModel);
    LoadStage();
    if (gSubStep == 100) {
        SetFogNearFar(0, 0, 250);
        SetFarColor(0, 0, 0);
        gModeStep = 4;
        gElapsed = 0;
        gSubStep = 0;
        gGameMode = GM_STAGE;
        PlayStageMusic(gStage);
    }
}

void UpdateLoadScreen(void) {
    SVECTOR pos;

    pos.vx = 0x70;
    pos.vy = 0x48;
    DrawSprite(gFrameCount % 10 + 0xFB, &pos, NULL, 0, &gSubOt[gActiveBuff]);
    pos.vx = -0xA0;
    pos.vy = -0x78;
    DrawSprite(0x1FE, &pos, NULL, 0, &gSubOt[gActiveBuff]);
    pos.vx = 0;
    pos.vy = -0x78;
    DrawSprite(0x1FF, &pos, NULL, 0, &gSubOt[gActiveBuff]);
    LoadStage();
    if (gSubStep == 100) {
        gModeStep = 4;
        gElapsed = 0;
        gSubStep = 0;
        gGameMode = GM_STAGE;
        PlayStageMusic(gStage);
    }
}

void LoadAct3Stage(void);

void UpdateAct3LoadScreen(void) {
    SVECTOR pos;

    pos.vx = 0x70;
    pos.vy = 0x48;
    DrawSprite(gFrameCount % 10 + 0xFB, &pos, NULL, 0, &gSubOt[gActiveBuff]);
    pos.vx = -0xA0;
    pos.vy = -0x78;
    DrawSprite(0x1FE, &pos, NULL, 0, &gSubOt[gActiveBuff]);
    pos.vx = 0;
    pos.vy = -0x78;
    DrawSprite(0x1FF, &pos, NULL, 0, &gSubOt[gActiveBuff]);
    LoadAct3Stage();
    if (gSubStep == 100) {
        gModeStep = 4;
        gSubStep = 0;
        gGameMode = GM_STAGE;
    }
}

void DrawStepper(PlayerState *obj) {
    drawObj(obj);
}

void StepStepper(PlayerState *obj) {
    stepObj(obj);
}

extern CdlLOC gCdToc[];
extern s32 gCdTrackEnd;
extern s32 gCdTrackLoop;
/* MATCHING: a known small size, so the byte store is `$gp`-relative. */
extern u8 sCdModeParam[4];

void PlayCdTrack(s32 track, s32 loop) {
    CdlATV atv;

    if (gCdTrackPlaying == 1) {
        StopCdTrack();
    }
    if (gStereo) {
        atv.val2 = 0x50;
        atv.val0 = 0x50;
        atv.val3 = 0;
        atv.val1 = 0;
    } else {
        atv.val2 = 0x50;
        atv.val0 = 0x50;
        atv.val3 = 0x50;
        atv.val1 = 0x50;
    }
    CdMix(&atv);
    gCdTrackLoop = loop;
    CdGetToc(gCdToc);
    gCdTrackStart = CdPosToInt(&gCdToc[track]);
    gCdTrackEnd = CdPosToInt(&gCdToc[track + 1]) - 20;
    gCdTrackPlays = 0;
    gCdTrackPos = gCdTrackStart;
    SsSetSerialAttr(0, 0, 1);
    SsSetSerialVol(0, 0x72, 0x72);
    sCdModeParam[0] = 3;
    CdControlB(CdlSetmode, sCdModeParam, 0);
    VSync(3);
    CdReadyCallback((CdlCB)OnCdTrackReady);
    SendCdTrackCommand(CdlPlay);
    gCdTrackPlaying = 1;
}

INCLUDE_RODATA("asm/nonmatchings/main", gMotionFmt);
