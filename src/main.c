#include "common.h"
#include "libgte.h"
#include "libgpu.h"
#include "libetc.h"
#include "libpad.h"
#include "libgs.h"
#include "libapi.h"
#include "libcd.h"
#include "libsnd.h"
#include "code_31cec.h"
#include "libmcrd.h"
#include "memory.h"
#include "code_a0bc.h"
#include "code_7d74.h"
#include "code_1a098.h"
#include "spad.h"

INCLUDE_RODATA("asm/nonmatchings/main", D_80010000);

/** @brief Two words, cleared or copied as a pair (a unit-local view). */
typedef struct {
    s32 a; /**< first word */
    s32 b; /**< second word */
} Pair8;

/* MATCHING: per-unit views; code_29f54 reads the same pairs as words. */
extern Pair8 D_80096A58[];
extern Pair8 D_800D8340[];
extern Pair8 D_80010454[];
extern u8 D_800957DA;
extern s8 D_800957A0;
extern u8 D_800958A5;
extern s32 D_80095984;
extern s32 D_80095998;
extern u16 D_80095764;
extern u16 D_80095766;
extern char D_800954C8[];

/* MATCHING: arity-ok, retail passes an argument the empty definition ignores. */
void func_80018DE8(s32 arg);
s8 func_80013B38(void);
s32 LoadSoundData(void);
void InitSystem(void);
void ResetClearColor(void);
s8 UpdateCdLoad(void);
void EndFrame(void);
void func_80016D14(void);
void func_80016FC0(void);
void func_80017124(void);
void func_8002B220(void);
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
    func_80020C14();
    func_80018DE8(1);
    func_80018CB4();
    func_80018094();
    SetFogNearFar(3000, 8000, 250);
    SetFarColor(0x40, 0x60, 0xFF);
    D_800957DA = 1;
    gStereo = 1;
    D_800958C8 = 1;
    D_800957A0 = -1;
    ClipF = 0;
    D_80095758 = 0;
    D_80095908 = 0;
    D_800957D5 = 0;
    D_800958A5 = 0;
    D_800958AC = -1;
    D_80095984 = 0;
    D_80095998 = 0;
    for (D_800958CC = 0; D_800958CC < 4; D_800958CC++) {
        D_80096A58[D_800958CC].a = 0;
        D_80096A58[D_800958CC].b = 0;
        dst = &D_800D8340[D_800958CC];
        a = D_80010454[D_800958CC].a;
        b = D_80010454[D_800958CC].b;
        dst->a = a;
        dst->b = b;
    }
    D_800956D0 = 0;
    gModeStep = 0;
    gGameMode = 0;
    gLoading = 0;
    D_80095766 = 0;
    D_80095764 = 0;
    D_800957DA = 0;
    D_80095968 = 0;
    D_800958F8 = 0;
    gCdTrackPlaying = 0;
    for (;;) {
        FntPrint(D_800954C8);
        if (gLoading == 1) {
            ResetClearColor();
        }
        switch (gGameMode) {
            case 0:
                gClearB = 0;
                gClearG = 0;
                gClearR = 0;
                D_800958A6 = 0;
                gGameMode = 1;
                gVSyncMode = 2;
                break;
            case 1:
                if (func_80013B38() == 1) {
                    D_800958A6 = 0;
                    gGameMode = 2;
                }
                break;
            case 2:
                if (LoadSoundData() == 1) {
                    gGameMode = 6;
                }
                break;
            case 3:
                gLoading = 1;
                gGameMode = 4;
                LoadOverlay(0);
                break;
            case 4:
                if (gCdLoadState == 6) {
                    gGameMode = 5;
                    gModeStep = 0;
                    gLoading = 0;
                }
                break;
            case 5:
                func_800F026C();
                break;
            case 6:
                gLoading = 1;
                D_800958F8 = 0;
                gGameMode = 7;
                LoadOverlay(2);
                break;
            case 7:
                if (gCdLoadState == 6) {
                    gGameMode = 8;
                    if (gModeStep == 0xFFFF) {
                        gModeStep = 50;
                    } else {
                        gModeStep = 0;
                    }
                    gLoading = 0;
                }
                break;
            case 8:
                func_800F6670();
                func_80014DB0();
                break;
            case 38:
                world = gStage / 3;
                gClearB = 0;
                gClearG = 0;
                gClearR = 0;
                gLoading = 1;
                gGameMode = 39;
                LoadOverlay(world + 18);
                break;
            case 39:
                if (gCdLoadState == 6) {
                    gGameMode = 40;
                }
                gModeStep = 0;
                D_80095768 = 0;
                break;
            case 40:
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
            case 12:
                gClearB = 0;
                gClearG = 0;
                gClearR = 0;
                D_800958F8 = 0;
                gLoading = 1;
                gGameMode = 13;
                LoadStageOverlay(gStage);
                break;
            case 13:
                if (gCdLoadState == 6) {
                    gGameMode = 14;
                }
                gModeStep = 0;
                break;
            case 14:
                UpdateStagePlay(gStage);
                break;
            case 15:
                gGameMode = 16;
                func_800229A8();
                gClearB = 0;
                gClearG = 0;
                gClearR = 0;
                gVSyncMode = 2;
                break;
            case 16:
                func_80022A74();
                break;
            case 17:
                gVSyncMode = 2;
                gGameMode = 19;
                gModeStep = 0;
                break;
            case 19:
                func_80017B38();
                break;
            case 20:
                gGameMode = 23;
                gClearB = 0;
                gClearG = 0;
                gClearR = 0;
                gLoading = 1;
                gGameMode = 21;
                D_800958A6 = 0;
                D_800958E2 = 0;
                LoadStageOverlay(gStage);
                gVSyncMode = 0;
                break;
            case 21:
                if (gCdLoadState == 6) {
                    gGameMode = 22;
                }
                break;
            case 22:
                if (InitStageOverlay(gStage) == 1) {
                    gGameMode = 23;
                    gLoading = 0;
                }
                break;
            case 23:
                func_8002B04C();
                break;
            case 24:
                gClearB = 0;
                gClearG = 0;
                gClearR = 0;
                gLoading = 1;
                gGameMode = 25;
                D_800958A6 = 0;
                D_800958E2 = 0;
                LoadStageOverlay(gStage);
                gVSyncMode = 0;
                break;
            case 25:
                if (gCdLoadState == 6) {
                    gGameMode = 26;
                }
                break;
            case 26:
                if (InitStageOverlay(gStage) == 1) {
                    gGameMode = 27;
                    gLoading = 0;
                    gModeStep = 0;
                }
                break;
            case 27:
                func_8002B220();
                break;
            case 28:
                D_800958F8 = 1;
                gClearB = 0;
                gClearG = 0;
                gClearR = 0;
                gLoading = 1;
                gGameMode = 29;
                D_800958A6 = 0;
                D_800958E2 = 0;
                LoadStageOverlay(gStage);
                gVSyncMode = 2;
                break;
            case 29:
                if (gCdLoadState == 6) {
                    gGameMode = 30;
                }
                break;
            case 30:
                if (InitStageOverlay(gStage) == 1) {
                    gGameMode = 32;
                    gModeStep = 0;
                    gLoading = 0;
                }
                break;
            case 31:
                if (gCdLoadState == 6) {
                    gGameMode = 32;
                    gModeStep = 0;
                    gLoading = 0;
                }
                break;
            case 32:
                func_8002D424();
                break;
            case 33:
                D_800958F8 = 1;
                gClearB = 0;
                gClearG = 0;
                gClearR = 0;
                gLoading = 1;
                gGameMode = 34;
                D_800958A6 = 0;
                D_800958E2 = 0;
                LoadStageOverlay(gStage);
                gVSyncMode = 2;
                break;
            case 34:
                if (gCdLoadState == 6) {
                    gGameMode = 35;
                }
                break;
            case 35:
                if (InitStageOverlay(gStage) == 1) {
                    gGameMode = 37;
                    gModeStep = 0;
                    gLoading = 0;
                }
                break;
            case 36:
                if (gCdLoadState == 6) {
                    gGameMode = 37;
                    gModeStep = 0;
                    gLoading = 0;
                }
                break;
            case 37:
                func_80034070();
                break;
            case 41:
                func_80016D14();
                break;
            case 42:
                func_80016FC0();
                break;
            case 43:
                func_80017124();
                break;
            case 44:
                switch ((u8)(gStage / 3)) {
                    case 0:
                        PlayMovie("\\MOVIE1.STR;1", NULL, 0xBC, 0xF0, 1);
                        gGameMode = 38;
                        break;
                    case 1:
                        PlayMovie("\\MOVIE2.STR;1", NULL, 0x166, 0xF0, 1);
                        gGameMode = 38;
                        break;
                    case 2:
                        PlayMovie("\\MOVIE3.STR;1", NULL, 0x12A, 0xF0, 1);
                        gGameMode = 38;
                        break;
                    case 3:
                        PlayMovie("\\MOVIE3.STR;1", NULL, 0x12A, 0xF0, 1);
                        gGameMode = 38;
                        break;
                    case 4:
                        PlayMovie("\\MOVIE4.STR;1", NULL, 0x184, 0xF0, 1);
                        gGameMode = 38;
                        break;
                    case 5:
                        PlayMovie("\\MOVIE5.STR;1", NULL, 0x661, 0xF0, 1);
                        if (D_800958AC == 0) {
                            D_800958A5 = 1;
                            D_800958C8 |= 2;
                        } else if (D_800958AC == 1) {
                            D_800958C8 |= 8;
                        }
                        D_800957A0 = -1;
                        gGameMode = 6;
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

extern s32 D_80072484[];
extern s32 D_800724C4[];
void func_80013CDC(void);

/* MATCHING: code_7d74 types the pack as its own PackEntry. */
void func_8001797C(void *pack);

s8 func_80013B38(void) {
    /* MATCHING: a byte, so the later constant 1 is not copied from it. */
    s8 ret;

    ret = 0;
    switch (D_800958A6) {
        case 0:
            gCdLoadLoc[0] = (s32)&D_80072484[0];
            gCdLoadDest[0] = 0x8014D000;
            gCdLoadLoc[1] = (s32)&D_80072484[1];
            gCdLoadDest[1] = 0x8018D000;
            gCdLoadLoc[2] = (s32)&D_80072484[2];
            gCdLoadDest[2] = 0x80123000;
            gCdLoadLoc[3] = (s32)&D_80072484[3];
            gCdLoadDest[3] = 0x8016D000;
            gCdLoadLoc[4] = (s32)&D_80072484[4];
            gCdLoadDest[4] = 0x80101000;
            gCdLoadCount = 5;
            gCdLoadSectors[0] = D_800724C4[0];
            gCdLoadSectors[1] = D_800724C4[1];
            gCdLoadSectors[2] = D_800724C4[2];
            gCdLoadSectors[3] = D_800724C4[3];
            gCdLoadSectors[4] = D_800724C4[4];
            gCdLoadState = 1;
            D_800958A6 = 1;
            break;
        case 1:
            if (gCdLoadState == 6) {
                func_80018DE8(0);
                D_800958A6++;
            }
            break;
        case 2:
            ret = 1;
            func_80013CDC();
            func_8001797C((void *)0x8018D000);
            func_8003E13C((unsigned long *)(*(s32 *)0x8016D000 + 0x8016D000));
            func_8003A84C();
            func_80018DE8(1);
            break;
    }
    return ret;
}

/**
 * @brief A per-channel stepping state (the name is a guess): its stepper
 *        advances channel N through tables at offsets 0x20 and 0x1B0.
 *        Only the bytes this unit touches are named.
 */
typedef struct {
    u8 unk0[4];      /**< not yet known */
    u8 unk4;         /**< passed on as the stepper's third argument */
    u8 unk5;         /**< a kind: 0x60 and 0x61 are tested; cleared at load */
    u8 unk6;         /**< cleared at load */
    u8 unk7;         /**< cleared at load */
    u8 unk8;         /**< set to 0xFF when a 0x61 step returns zero */
    u8 unk9;         /**< set to 0xFF at load */
    u8 unkA[0x16];   /**< not yet known */
    s32 unk20[100];  /**< first word of each loaded entry */
    s32 unk1B0[100]; /**< third word of each loaded entry */
    u8 unk340;       /**< the stepper's last result */
    u8 unk341[7];    /**< not yet known */
    s32 unk348;      /**< a position, x: the model's translation */
    s32 unk34C;      /**< a position, y */
    s32 unk350;      /**< a position, z */
    u8 unk354[0x2C]; /**< not yet known */
    u16 unk380;      /**< a rotation about y */
} Stepper;

/* The stepper's state lives at the head of the game state. */
#define sStepper (*(Stepper *)D_8009EB78)

/** @brief A model object with its own coordinate system (a unit-local
 *         copy of code_1a098's view). */
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

/* MATCHING: per-unit view; code_7d74.c reads it as a word array. */
extern Model70 D_800963A0;

/** @brief A 0x78-byte model slot: a Model70 and 8 bytes not yet known. */
typedef struct {
    Model70 m;   /**< the model */
    u8 unk70[8]; /**< not yet known */
} ModelSlot;

/* MATCHING: code_13068 has its own view of these slots. */
extern ModelSlot D_800D8370[];
extern u8 D_8009EF50[];
extern u8 D_80096418[];

/* MATCHING: code_1a098 defines it on its own Model70 view. */
void func_8002C20C(Model70 *m, unsigned long *tmd, u8 n);
/* MATCHING: code_7d74 types the list, slots and data as its own records. */
void InitTodObjTable(s32 *list, u8 *slots, GsCOORDINATE2 *objs, u8 *data, s32 n);
/* MATCHING: code_308ec passes the game state's head in its own view. */
u8 func_80017F0C(Stepper *obj, u16 index, u8 arg);

void func_80013CDC(void) {
    /* MATCHING: one pointer reused for the pack, its second word and the
     * directory, so all three share a saved register. */
    s32 *p;
    s32 *q;

    p = (s32 *)0x8014D000;
    D_80095904 = (s32)((unsigned long *)((u8 *)p + *p) + 1);
    GsMapModelingData((unsigned long *)D_80095904);
    func_8002C20C(&D_800963A0, (unsigned long *)((u8 *)p + *p), 16);
    p = (s32 *)0x8014D010;
    func_8002C20C(&D_800D8370[0].m, (unsigned long *)(*p + 0x8014D000), 0);
    func_8002C20C(&D_800D8370[1].m, (unsigned long *)(*p + 0x8014D000), 1);
    func_8002C20C(&D_800D8370[2].m, (unsigned long *)(*p + 0x8014D000), 2);
    func_8002C20C(&D_800D8370[3].m, (unsigned long *)(*p + 0x8014D000), 3);
    func_8002C20C(&D_800D8370[4].m, (unsigned long *)(*p + 0x8014D000), 4);
    func_8002C20C(&D_800D8370[5].m, (unsigned long *)(*p + 0x8014D000), 5);
    func_8002C20C(&D_800D8370[6].m, (unsigned long *)(*p + 0x8014D000), 6);
    InitTodObjTable(gTodObjTable, D_8009EF50, D_800D86E0, D_80096418, 20);
    p = (s32 *)0x80123000;
    D_800958CC = 0;
    D_800958D0 = ((DirEnt16 *)p)->count;
    for (; D_800958CC < D_800958D0; D_800958CC++) {
        /* MATCHING: assigned inside the store, as in func_8002C044. */
        D_800D81B0[D_800958CC] = (q = (s32 *)((u8 *)0x80123000 + *p)) + 1;
        p += 4;
        sStepper.unk20[D_800958CC] = *D_800D81B0[D_800958CC];
        D_800D81B0[D_800958CC]++;
        sStepper.unk1B0[D_800958CC] = D_800D81B0[D_800958CC][1];
    }
    GsInitCoordinate2(WORLD, &D_800D86E0[0]);
    sStepper.unk5 = 0;
    sStepper.unk6 = 0;
    /* MATCHING: retail stores 8 before 7. */
    sStepper.unk8 = 0xFF;
    sStepper.unk7 = 0;
    sStepper.unk9 = 0xFF;
    func_80017F0C(&sStepper, 0, 0);
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

extern DR_STP D_800CEBD0;
extern DR_STP D_800CEBC0;

void EndFrame(void) {
    GsRVIEW2 view;

    switch (gLoading) {
        case 0:
            if (gGameMode != 14) {
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
    SetDrawStp(&D_800CEBD0, 1);
    AddPrim(&gOt[gActiveBuff].org[0xFFF], &D_800CEBD0);
    func_80039754(ClipF);
    GsDrawOt(&gOt[gActiveBuff]);
    gActiveBuff = GsGetActiveBuff();
    GsSetWorkBase(gGpuWork[gActiveBuff]);
    GsClearOt(0, 300, &gOt[gActiveBuff]);
    GsClearOt(0, 0, &gSubOt[gActiveBuff]);
    GsSortOt(&gSubOt[gActiveBuff], &gOt[gActiveBuff]);
    gCurOt = &gOt[gActiveBuff];
    SetDrawStp(&D_800CEBC0, 0);
    AddPrim(gSubOt[gActiveBuff].org, &D_800CEBC0);
    ReadPad(0);
    func_8003DE34();
    if (gPauseState == 0) {
        gFrameCount++;
    }
}

/* MATCHING: declared at most 8 bytes, so each base is one `la` register. */
extern s32 gPadPrev[2];
extern s32 gPadPressed[2];
extern s32 gPadHeld[2];

extern s8 D_8009575D;
extern u8 D_800956EA;
extern u8 D_800956DB;
extern u8 D_800956E8[2];
extern u8 D_800956D8[3];

void PollPads(void);

/**
 * @brief Auto-repeat for one held button: once @p cnt has counted 30
 *        frames (scaled by the speed byte), @p bit repeats into the
 *        pressed-flags word every few frames. A release clears @p cnt.
 */
/* MATCHING: a bare block; `do { } while (0)` changes the branch layout. */
#define REPEAT(bit, cnt)                                                           \
    {                                                                              \
        if (gPad & (bit)) {                                                        \
            if ((cnt) >= 30 / D_8009575D && gFrameCount % (4 / D_8009575D) == 0) { \
                gPadTrig |= (bit);                                                 \
            } else if ((cnt) < 30 / D_8009575D) {                                  \
                (cnt)++;                                                           \
            }                                                                      \
        } else {                                                                   \
            (cnt) = 0;                                                             \
        }                                                                          \
    }

void ReadPad(s32 port) {
    D_8009575D = 4;
    PollPads();
    if (gPadType == 0x41) {
        D_800957C0 = D_800957C8 = 0x80;
    }
    /* MATCHING: this order builds gPadPressed's address first, as retail;
     * the stores still come out gPad first. */
    gPadTrig = gPadPressed[(u16)port];
    gPad = gPadHeld[(u16)port];
    REPEAT(0x1000, D_800956EA)
    REPEAT(0x4000, D_800956DB)
    REPEAT(0x8000, D_800956E8[0])
    REPEAT(0x2000, D_800956E8[1])
    if (D_800956F7 == 1) {
        REPEAT(0x80, D_800956D8[0])
        REPEAT(0x40, D_800956D8[1])
        REPEAT(0x20, D_800956D8[2])
    }
}

void InitGraphics(void);
void WaitFrames(s16 frames);

void func_800148B0(void) {
    s32 x;
    s32 y;
    s32 pad;

    x = 0;
    y = 0;
    WaitFrames(20);
    for (;;) {
        pad = gPadHeld[0];
        if (pad & 0x2000) {
            x += 8;
        }
        if (pad & 0x8000) {
            x -= 8;
        }
        if (pad & 0x1000) {
            y -= 8;
        }
        if (pad & 0x4000) {
            y += 8;
        }
        if (pad & 0x800) {
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
    func_80018CB4();
    func_80018094();
    WaitFrames(5);
}

extern char D_800954CC[];
extern char D_800954D0[];
extern char D_800954D8[];
extern char D_800954DC[];
extern s32 D_80095788;

void func_800149D0(u8 *base) {
    u8 *p;
    s32 row;
    s32 col;
    s16 i;
    s16 j;

    D_80095788 = 0;
    FntPrint(D_800954D0);
    p = base + D_80095788;
    FntPrint(D_800954D8, p);
    for (i = 0; i < 1; i++) {
        FntPrint(D_800954CC);
    }
    for (row = 0; row < 16; row++) {
        for (col = 0; col < 8; col++) {
            FntPrint(D_800954DC, *p);
            p++;
        }
        FntPrint(D_800954C8);
        for (j = 0; j < 1; j++) {
            FntPrint(D_800954CC);
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
        FntPrint(D_800954CC);
    }
}

/** @brief 15 bytes copied as one block, for a local copy of a table. */
typedef struct {
    u8 b[15]; /**< the bytes */
} Bytes15;

extern u8 D_80010000[];

void PlayCdTrack(s32 track, s32 loop);

void PlayStageMusic(u8 idx) {
    Bytes15 tbl;

    tbl = *(Bytes15 *)D_80010000;
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
 * @brief A 10-byte record of a table in initialized data (the table
 *        starts with one), unpacked field by field into small globals.
 *        Meanings not yet known.
 */
typedef struct {
    u16 unk0; /**< not yet known */
    u16 unk2; /**< not yet known */
    u8 unk4;  /**< not yet known */
    u8 unk5;  /**< not yet known */
    u8 unk6;  /**< not yet known */
    u8 unk7;  /**< not yet known */
    u16 unk8; /**< not yet known */
} TableHeader;

extern TableHeader *D_80095930;
extern u16 D_8009576C;
extern u16 D_8009586A;
extern u16 D_800958E6;
/* MATCHING: signed, so the zero test in func_80014DB0 is its own `lh`. */
extern s16 D_800957D8;
extern u16 D_800957E0;

/**
 * @brief Selects record @p index of @p tbl and unpacks it into the record
 *        globals. The same body as the out-of-line selector below; inlined
 *        with the table head as @p tbl, its store of the head drops out.
 */
static __inline__ void setRecord(TableHeader *tbl, u16 index) {
    u8 *rec;

    D_80095930 = tbl;
    tbl += index;
    D_8009576C = index;
    D_8009586A = tbl->unk0;
    D_800958E6 = tbl->unk2;
    /* MATCHING: byte-pointer reads keep each load below the prior store. */
    rec = (u8 *)tbl;
    D_80095764 = rec[5];
    D_80095766 = rec[4];
    D_800957D8 = *(u16 *)(rec + 8);
    D_800957E0 = rec[6];
}

/**
 * @brief setRecord's twin for repeating the current record: the same
 *        unpacking, with the repeat count @p count stepped down instead of
 *        reloaded. Inlined, so its parameter conversions survive.
 */
static __inline__ void repeatRecord(TableHeader *tbl, u16 index, u8 count) {
    u8 *rec;

    D_80095930 = tbl;
    tbl += index;
    D_8009576C = index;
    D_8009586A = tbl->unk0;
    D_800958E6 = tbl->unk2;
    rec = (u8 *)tbl;
    D_80095764 = rec[5];
    D_80095766 = rec[4];
    D_800957D8 = *(u16 *)(rec + 8);
    D_800957E0 = count - 1;
}

/* MATCHING: s32 with a bare return, so the `bgtz` keeps its `nop` slot. */
s32 func_80014DB0(void) {
    if ((s16)D_800958E6 > 0 && D_800957DA == 1) {
        SetPadVibration(0, D_80095764, D_80095766);
        D_800958E6--;
        return;
    }
    D_800958E6 = 0;
    D_80095764 = 0;
    D_80095766 = 0;
    SetPadVibration(0, 0, 0);
    if ((s16)D_800957E0 > 0) {
        if ((s16)--D_8009586A <= 0) {
            if ((s16)D_800957E0 < 2) {
                D_800957E0 = 0;
                if (D_800957D8 != 0) {
                    setRecord(D_80095930, D_800957D8);
                }
            } else {
                repeatRecord(D_80095930, D_8009576C, D_800957E0);
            }
        }
    } else {
        D_8009586A = 0;
    }
}

extern u8 D_80072F44[];
extern u8 gPadBuf[];
extern s32 D_80095928;
extern s32 D_80095920;
/* MATCHING: code_1dc24 declares it too; both reach it the same way. */
extern s32 D_80095950;

extern s32 D_80095854;
extern s32 D_8009584C;
extern s32 D_80095874;

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
    btn = gPadBuf[D_80072F44[0] + 2] << 8;
    btn |= gPadBuf[D_80072F44[0] + 3];
    btn ^= 0xFFFF;
    if (id == 0) {
        btn = 0;
    }
    m = gPadPrev[0] ^ 0xFFFF;
    /* MATCHING: a new local, not `btn = (u16)btn;`: it takes $v1 in
     * local-alloc, so the D_80095854 load lands in $a0 and is hoisted. */
    b = (u16)btn;
    gPadHeld[0] = gPadPrev[0] = b;
    gPadPressed[0] = b & m;
    m = D_80095854 ^ 0xFFFF;
    btn = gPadBuf[D_80072F44[1] + 0x24] << 8;
    btn |= gPadBuf[D_80072F44[1] + 0x25];
    btn ^= 0xFFFF;
    D_8009584C = D_80095854 = (u16)btn;
    btn = (u16)btn & m;
    D_80095874 = btn;
    if (id == 0x73) {
        m = D_80095928 ^ 0xFFFF;
        if (gPadBuf[7] < 10) {
            D_80095928 = 0x1000;
        } else if (gPadBuf[7] >= 0xF6) {
            D_80095928 = 0x4000;
        } else {
            D_80095928 = 0;
        }
        D_80095958 = D_80095928 & m;
        m = D_80095920 ^ 0xFFFF;
        if (gPadBuf[6] < 10) {
            D_80095920 = 0x8000;
        } else if (gPadBuf[6] >= 0xF6) {
            D_80095920 = 0x2000;
        } else {
            D_80095920 = 0;
        }
        D_80095950 = D_80095920 & m;
        D_800957C0 = gPadBuf[6];
        D_800957C8 = gPadBuf[7];
    }
}

extern u8 *D_80095700;
extern u8 *D_80095704;
/* MATCHING: a known small size, so its address is one `la` register. */
extern u8 D_800954E4[6];

u32 UpdatePadActuators(void) {
    u8 *buf;
    u8 *act;
    s32 k;
    s32 state;
    s32 i;

    buf = D_80095700;
    act = D_80095704;
    for (k = 0; k < 2; k++, act += 0x10) {
        state = PadGetState(k * 16);
        if (state == 1) {
            act[0] = 0;
        }
        if (act[0] == 0) {
            PadSetAct(k * 16, act + 2, 2);
            if (state == 2 || (state == 6 && PadSetActAlign(k * 16, D_800954E4))) {
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
    D_80095704[offset + 2] = a;
    D_80095704[offset + 3] = b;
}

void func_8001552C(u8 *a, u8 *b);

extern u8 D_80095BE8[];

void InitPads(void) {
    s16 i;

    func_8001552C(gPadBuf, D_80095BE8);
    for (i = 0; i < 6; i++) {
        gPadPrev[i] = 0;
        gPadPressed[i] = 0;
        gPadHeld[i] = 0;
    }
}

void SetPadMainMode(s32 mode) {
    mode &= 1;
    D_800958CC = 0;
    do {
        VSync(0);
        if (PadGetState(0) == 6) {
            PadSetMainMode(0, mode, 0);
            return;
        }
        D_800958CC++;
    } while (D_800958CC < 10);
}

void func_80015450(TableHeader *tbl, u16 index) {
    u8 *rec;

    D_80095930 = tbl;
    tbl += index;
    D_8009576C = index;
    D_8009586A = tbl->unk0;
    D_800958E6 = tbl->unk2;
    /* MATCHING: byte-pointer reads keep each load below the prior store. */
    rec = (u8 *)tbl;
    D_80095764 = rec[5];
    D_80095766 = rec[4];
    D_800957D8 = *(u16 *)(rec + 8);
    D_800957E0 = rec[6];
}

void func_800154C4(void) {
    TableHeader *hdr;

    hdr = (TableHeader *)D_800734AC;
    D_80095930 = hdr;
    D_8009576C = 0;
    D_8009586A = hdr->unk0;
    D_800958E6 = hdr->unk2;
    D_80095764 = hdr->unk5;
    D_80095766 = hdr->unk4;
    D_800957D8 = hdr->unk8;
    D_800957E0 = hdr->unk6;
}

void func_8001552C(u8 *a, u8 *b) {
    s32 i;

    D_80095700 = a;
    D_80095704 = b;
    for (i = 0; i < 32; i++) {
        D_80095704[i] = 0;
    }
    PadInitDirect(D_80095700, D_80095700 + 0x22);
    PadStartCom();
}

INCLUDE_RODATA("asm/nonmatchings/main", D_80010148);

extern s16 gCdLoadIndex;
extern s32 gCdLoadReadSectors;
extern u8 gCdLoadRetries;

s8 UpdateCdLoad(void) {
    s32 r;

    switch (gCdLoadState) {
        case 0:
            break;
        case 1:
            CdControlF(CdlSetloc, (u_char *)gCdLoadLoc[gCdLoadIndex]);
            gCdLoadState++;
            break;
        case 2:
            r = CdSync(1, NULL);
            if (r == 2) {
                gCdLoadState++;
            } else if (r == 5) {
                gCdLoadState = 1;
                gCdLoadRetries++;
            }
            break;
        case 3:
            gCdLoadReadSectors = gCdLoadSectors[gCdLoadIndex] + 1;
            if (CdRead(gCdLoadReadSectors, (u_long *)gCdLoadDest[gCdLoadIndex], 0x80) == 0) {
                gCdLoadRetries++;
            } else {
                gCdLoadState++;
            }
            break;
        case 4:
            r = CdReadSync(1, NULL);
            if (r == 0) {
                gCdLoadState++;
            } else if (r == -1) {
                gCdLoadState = 1;
                gCdLoadRetries++;
            }
            break;
        case 5:
            if (++gCdLoadIndex >= gCdLoadCount) {
                gCdLoadState = 6;
                gCdLoadIndex = 0;
            } else {
                gCdLoadState = 1;
            }
            break;
    }
    return gCdLoadState;
}

extern char D_80010148[];

void LoadFile(char *name, void *buf) {
    s32 fd;

    fd = open(name, 1);
    if (fd == -1) {
        for (;;) {
            GsSwapDispBuff();
            FntPrint(D_80010148, name);
            FntFlush(-1);
        }
    }
    read(fd, buf, 0x100000);
    close(fd);
}

/**
 * @brief A 0x3D8-byte copy of the game state's head, reset for a demo or
 *        replay. Only the members this unit writes are named.
 */
typedef struct {
    u8 unk0;         /**< cleared */
    u8 unk1[4];      /**< not yet known */
    u8 unk5;         /**< the stepper kind; set to 0x61 */
    u8 unk6;         /**< set to 3 */
    u8 unk7[0x341];  /**< not yet known */
    s32 unk348;      /**< a position, x */
    s32 unk34C;      /**< a position, y */
    s32 unk350;      /**< a position, z */
    u8 unk354[0x2C]; /**< not yet known */
    s32 unk380;      /**< a rotation about y, as a word */
    u8 unk384[0x24]; /**< not yet known */
    s16 unk3A8;      /**< cleared */
    u8 unk3AA[0x2E]; /**< not yet known */
} StateCopy;

extern StateCopy D_80095C08;

void func_800157DC(void) {
    SVECTOR pos;
    RECT rect;

    func_8001B2F4(0x1FE, 2, 0xA0, 0xF0, 10, 0, 0, 0, 0);
    func_8001B2F4(0x1FF, 2, 0xA0, 0xF0, 12, 0x20, 0, 0, 0);
    rect.x = 0;
    rect.y = D_800E474C * 240;
    rect.w = 320;
    rect.h = 240;
    MoveImage(&rect, 640, 0);
    pos.vx = -160;
    pos.vy = -120;
    func_8001B354(0x1FE, &pos, NULL, 0, &gSubOt[gActiveBuff]);
    pos.vx = 0;
    pos.vy = -120;
    func_8001B354(0x1FF, &pos, NULL, 0, &gSubOt[gActiveBuff]);
    D_80095C08 = *(StateCopy *)D_8009EB78;
    D_80095C08.unk380 = 0x800;
    D_80095C08.unk6 = 3;
    D_80095C08.unk348 = 0;
    D_80095C08.unk34C = 0;
    D_80095C08.unk350 = 0;
    D_80095C08.unk5 = 0x61;
    D_80095C08.unk3A8 = 0;
    D_80095C08.unk0 = 0;
    gRefView[0] = rsin(0x800) * 400 / 4096;
    gRefView[1] = -150;
    gRefView[2] = rcos(0x800) * 400 / 4096;
    gRefView[3] = 0;
    gRefView[4] = -150;
    gRefView[5] = 0;
    D_800958A6 = 0;
    gGameMode = 0x29;
}

void func_80015A28(void) {
    SVECTOR pos;
    RECT rect;

    func_8001B2F4(0x1FE, 2, 0xA0, 0xF0, 10, 0, 0, 0, 0);
    func_8001B2F4(0x1FF, 2, 0xA0, 0xF0, 12, 0x20, 0, 0, 0);
    rect.x = 0;
    rect.y = D_800E474C * 240;
    rect.w = 320;
    rect.h = 240;
    MoveImage(&rect, 640, 0);
    pos.vx = -160;
    pos.vy = -120;
    func_8001B354(0x1FE, &pos, NULL, 0, &gSubOt[gActiveBuff]);
    pos.vx = 0;
    pos.vy = -120;
    func_8001B354(0x1FF, &pos, NULL, 0, &gSubOt[gActiveBuff]);
    D_800958A6 = 0;
    gGameMode = 0x2A;
}

void func_80015B78(void) {
    SVECTOR pos;
    RECT rect;

    func_8001B2F4(0x1FE, 2, 0xA0, 0xF0, 10, 0, 0, 0, 0);
    func_8001B2F4(0x1FF, 2, 0xA0, 0xF0, 12, 0x20, 0, 0, 0);
    rect.x = 0;
    rect.y = D_800E474C * 240;
    rect.w = 320;
    rect.h = 240;
    MoveImage(&rect, 640, 0);
    pos.vx = -160;
    pos.vy = -120;
    func_8001B354(0x1FE, &pos, NULL, 0, &gSubOt[gActiveBuff]);
    pos.vx = 0;
    pos.vy = -120;
    func_8001B354(0x1FF, &pos, NULL, 0, &gSubOt[gActiveBuff]);
    D_800958A6 = 0;
    gGameMode = 0x2B;
}

/* Each stage's disc directory (the names are guesses): file positions,
 * then their sizes in sectors, one pair of arrays per stage. */
extern CdlLOC D_80073D00[];
extern s32 D_80073D24[];
extern CdlLOC D_80073DD8[];
extern s32 D_80073DFC[];
extern CdlLOC D_80073EB0[];
extern s32 D_80073ED4[];
extern CdlLOC D_80073F88[];
extern s32 D_80073FAC[];
extern CdlLOC D_80074060[];
extern s32 D_80074084[];

/* MATCHING: code_31cec defines it with a u16 bank; this caller passes it
 * unextended, so its view is s32. */
s32 LoadVabBank(s32 bank);
void func_80039C3C(unsigned long *p);
void ResetStage(void);
void func_800F6C60(void);
void func_800F6A44(void);
void func_800F642C(void);
void func_800F46D8(void);
void func_800F377C(void);

void func_80015CC8(void) {
    s32 bank;

    switch (D_800958A6) {
        case 0:
            gVabLoadStep = 0;
            D_800958A6++;
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
                D_800958A6++;
            }
            break;
        case 2:
            LoadStageOverlay(gStage);
            D_800958A6++;
            break;
        case 3:
            if (gCdLoadState == 6) {
                D_800958A6++;
            }
            break;
        case 4:
            func_80018DE8(0);
            switch (gStage) {
                case 2:
                    gCdLoadLoc[0] = (s32)&D_80073D00[0];
                    gCdLoadSectors[0] = D_80073D24[0];
                    break;
                case 5:
                    gCdLoadLoc[0] = (s32)&D_80073DD8[0];
                    gCdLoadSectors[0] = D_80073DFC[0];
                    break;
                case 8:
                    gCdLoadLoc[0] = (s32)&D_80073EB0[0];
                    gCdLoadSectors[0] = D_80073ED4[0];
                    break;
                case 11:
                    gCdLoadLoc[0] = (s32)&D_80073F88[0];
                    gCdLoadSectors[0] = D_80073FAC[0];
                    break;
                case 14:
                    gCdLoadLoc[0] = (s32)&D_80074060[0];
                    gCdLoadSectors[0] = D_80074084[0];
                    break;
            }
            gCdLoadDest[0] = 0x8019D000;
            gCdLoadCount = 1;
            gCdLoadState = 1;
            D_800958A6++;
            break;
        case 5:
            if (gCdLoadState == 6) {
                func_8003E13C((unsigned long *)0x8019D000);
                D_800958A6++;
            }
            break;
        case 6:
            switch (gStage) {
                case 2:
                    gCdLoadLoc[0] = (s32)&D_80073D00[1];
                    gCdLoadSectors[0] = D_80073D24[1];
                    break;
                case 5:
                    gCdLoadLoc[0] = (s32)&D_80073DD8[1];
                    gCdLoadSectors[0] = D_80073DFC[1];
                    break;
                case 8:
                    gCdLoadLoc[0] = (s32)&D_80073EB0[1];
                    gCdLoadSectors[0] = D_80073ED4[1];
                    break;
                case 11:
                    gCdLoadLoc[0] = (s32)&D_80073F88[1];
                    gCdLoadSectors[0] = D_80073FAC[1];
                    break;
                case 14:
                    gCdLoadLoc[0] = (s32)&D_80074060[1];
                    gCdLoadSectors[0] = D_80074084[1];
                    break;
            }
            gCdLoadDest[0] = 0x8019D000;
            gCdLoadCount = 1;
            gCdLoadState = 1;
            D_800958A6++;
            break;
        case 7:
            if (gCdLoadState == 6) {
                func_80039C3C((unsigned long *)0x8019D000);
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
                D_800958A6 = 100;
            }
            break;
    }
}

/* The other stages' directories (see the stage loader above): file positions, and
 * sizes in sectors counted from an extra leading word, one pair per stage. */
extern CdlLOC D_80073C70[];
extern s32 D_80073C90[];
extern CdlLOC D_80073CB8[];
extern s32 D_80073CD8[];
extern CdlLOC D_80073D48[];
extern s32 D_80073D68[];
extern CdlLOC D_80073D90[];
extern s32 D_80073DB0[];
extern CdlLOC D_80073E20[];
extern s32 D_80073E40[];
extern CdlLOC D_80073E68[];
extern s32 D_80073E88[];
extern CdlLOC D_80073EF8[];
extern s32 D_80073F18[];
extern CdlLOC D_80073F40[];
extern s32 D_80073F60[];
extern CdlLOC D_80073FD0[];
extern s32 D_80073FF0[];
extern CdlLOC D_80074018[];
extern s32 D_80074038[];

/* MATCHING: code_29f54 defines it and has no header to declare it. */
void func_800399A8(unsigned long *p);
/* MATCHING: a per-unit view; DirEnt16 is this unit's view of the directory. */
void func_8002B7C8(DirEnt16 *dir, u16 id);
/* MATCHING: code_1dc24 types both as its own BankFile. */
void func_800373C8(void *a, void *b);
void func_8002C2B4(void);
s32 func_8002C044(void);
void func_8002C47C(void);
void StartStageOverlay(void);

void func_800160E8(void) {
    switch (D_800958A6) {
        case 0:
            gVabLoadStep = 0;
            D_800958A6++;
        case 1:
            if (LoadVabBank(GetStageVabBank(gStage)) == 1) {
                D_800958A6++;
            }
            break;
        case 2:
            LoadStageOverlay(gStage);
            D_800958A6++;
            break;
        case 3:
            if (gCdLoadState == 6) {
                D_800958A6++;
            }
            break;
        case 4:
            func_80018DE8(0);
            switch (gStage) {
                case 0:
                    gCdLoadLoc[0] = (s32)&D_80073C70[0];
                    gCdLoadSectors[0] = D_80073C90[1];
                    break;
                case 1:
                    gCdLoadLoc[0] = (s32)&D_80073CB8[0];
                    gCdLoadSectors[0] = D_80073CD8[1];
                    break;
                case 3:
                    gCdLoadLoc[0] = (s32)&D_80073D48[0];
                    gCdLoadSectors[0] = D_80073D68[1];
                    break;
                case 4:
                    gCdLoadLoc[0] = (s32)&D_80073D90[0];
                    gCdLoadSectors[0] = D_80073DB0[1];
                    break;
                case 6:
                    gCdLoadLoc[0] = (s32)&D_80073E20[0];
                    gCdLoadSectors[0] = D_80073E40[1];
                    break;
                case 7:
                    gCdLoadLoc[0] = (s32)&D_80073E68[0];
                    gCdLoadSectors[0] = D_80073E88[1];
                    break;
                case 9:
                    gCdLoadLoc[0] = (s32)&D_80073EF8[0];
                    gCdLoadSectors[0] = D_80073F18[1];
                    break;
                case 10:
                    gCdLoadLoc[0] = (s32)&D_80073F40[0];
                    gCdLoadSectors[0] = D_80073F60[1];
                    break;
                case 12:
                    gCdLoadLoc[0] = (s32)&D_80073FD0[0];
                    gCdLoadSectors[0] = D_80073FF0[1];
                    break;
                case 13:
                    gCdLoadLoc[0] = (s32)&D_80074018[0];
                    gCdLoadSectors[0] = D_80074038[1];
                    break;
            }
            gCdLoadDest[0] = 0x8019D000;
            gCdLoadCount = 1;
            gCdLoadState = 1;
            D_800958A6++;
            break;
        case 5:
            if (gCdLoadState == 6) {
                func_8003E13C((unsigned long *)0x8019D000);
                D_800958A6++;
            }
            break;
        case 6:
            switch (gStage) {
                case 0:
                    gCdLoadLoc[0] = (s32)&D_80073C70[1];
                    gCdLoadSectors[0] = D_80073C90[2];
                    break;
                case 1:
                    gCdLoadLoc[0] = (s32)&D_80073CB8[1];
                    gCdLoadSectors[0] = D_80073CD8[2];
                    break;
                case 3:
                    gCdLoadLoc[0] = (s32)&D_80073D48[1];
                    gCdLoadSectors[0] = D_80073D68[2];
                    break;
                case 4:
                    gCdLoadLoc[0] = (s32)&D_80073D90[1];
                    gCdLoadSectors[0] = D_80073DB0[2];
                    break;
                case 6:
                    gCdLoadLoc[0] = (s32)&D_80073E20[1];
                    gCdLoadSectors[0] = D_80073E40[2];
                    break;
                case 7:
                    gCdLoadLoc[0] = (s32)&D_80073E68[1];
                    gCdLoadSectors[0] = D_80073E88[2];
                    break;
                case 9:
                    gCdLoadLoc[0] = (s32)&D_80073EF8[1];
                    gCdLoadSectors[0] = D_80073F18[2];
                    break;
                case 10:
                    gCdLoadLoc[0] = (s32)&D_80073F40[1];
                    gCdLoadSectors[0] = D_80073F60[2];
                    break;
                case 12:
                    gCdLoadLoc[0] = (s32)&D_80073FD0[1];
                    gCdLoadSectors[0] = D_80073FF0[2];
                    break;
                case 13:
                    gCdLoadLoc[0] = (s32)&D_80074018[1];
                    gCdLoadSectors[0] = D_80074038[2];
                    break;
            }
            gCdLoadDest[0] = 0x8019D000;
            gCdLoadCount = 1;
            gCdLoadState = 1;
            D_800958A6++;
            break;
        case 7:
            if (gCdLoadState == 6) {
                func_8001797C((void *)0x8019D000);
                D_800958A6++;
            }
            break;
        case 8:
            switch (gStage) {
                case 0:
                    gCdLoadLoc[0] = (s32)&D_80073C70[2];
                    gCdLoadLoc[1] = (s32)&D_80073C70[3];
                    gCdLoadSectors[0] = D_80073C90[3];
                    gCdLoadSectors[1] = D_80073C90[4];
                    break;
                case 1:
                    gCdLoadLoc[0] = (s32)&D_80073CB8[2];
                    gCdLoadLoc[1] = (s32)&D_80073CB8[3];
                    gCdLoadSectors[0] = D_80073CD8[3];
                    gCdLoadSectors[1] = D_80073CD8[4];
                    break;
                case 3:
                    gCdLoadLoc[0] = (s32)&D_80073D48[2];
                    gCdLoadLoc[1] = (s32)&D_80073D48[3];
                    gCdLoadSectors[0] = D_80073D68[3];
                    gCdLoadSectors[1] = D_80073D68[4];
                    break;
                case 4:
                    gCdLoadLoc[0] = (s32)&D_80073D90[2];
                    gCdLoadLoc[1] = (s32)&D_80073D90[3];
                    gCdLoadSectors[0] = D_80073DB0[3];
                    gCdLoadSectors[1] = D_80073DB0[4];
                    break;
                case 6:
                    gCdLoadLoc[0] = (s32)&D_80073E20[2];
                    gCdLoadLoc[1] = (s32)&D_80073E20[3];
                    gCdLoadSectors[0] = D_80073E40[3];
                    gCdLoadSectors[1] = D_80073E40[4];
                    break;
                case 7:
                    gCdLoadLoc[0] = (s32)&D_80073E68[2];
                    gCdLoadLoc[1] = (s32)&D_80073E68[3];
                    gCdLoadSectors[0] = D_80073E88[3];
                    gCdLoadSectors[1] = D_80073E88[4];
                    break;
                case 9:
                    gCdLoadLoc[0] = (s32)&D_80073EF8[2];
                    gCdLoadLoc[1] = (s32)&D_80073EF8[3];
                    gCdLoadSectors[0] = D_80073F18[3];
                    gCdLoadSectors[1] = D_80073F18[4];
                    break;
                case 10:
                    gCdLoadLoc[0] = (s32)&D_80073F40[2];
                    gCdLoadLoc[1] = (s32)&D_80073F40[3];
                    gCdLoadSectors[0] = D_80073F60[3];
                    gCdLoadSectors[1] = D_80073F60[4];
                    break;
                case 12:
                    gCdLoadLoc[0] = (s32)&D_80073FD0[2];
                    gCdLoadLoc[1] = (s32)&D_80073FD0[3];
                    gCdLoadSectors[0] = D_80073FF0[3];
                    gCdLoadSectors[1] = D_80073FF0[4];
                    break;
                case 13:
                    gCdLoadLoc[0] = (s32)&D_80074018[2];
                    gCdLoadLoc[1] = (s32)&D_80074018[3];
                    gCdLoadSectors[0] = D_80074038[3];
                    gCdLoadSectors[1] = D_80074038[4];
                    break;
            }
            gCdLoadDest[0] = 0x8019D000;
            gCdLoadDest[1] = 0x80155000;
            gCdLoadCount = 2;
            gCdLoadState = 1;
            D_800958A6++;
            break;
        case 9:
            if (gCdLoadState == 6) {
                func_800399A8((unsigned long *)0x8019D000);
                D_800958A6++;
            }
            break;
        case 10:
            switch (gStage) {
                case 0:
                    gCdLoadLoc[0] = (s32)&D_80073C70[4];
                    gCdLoadSectors[0] = D_80073C90[5];
                    break;
                case 1:
                    gCdLoadLoc[0] = (s32)&D_80073CB8[4];
                    gCdLoadSectors[0] = D_80073CD8[5];
                    break;
                case 3:
                    gCdLoadLoc[0] = (s32)&D_80073D48[4];
                    gCdLoadSectors[0] = D_80073D68[5];
                    break;
                case 4:
                    gCdLoadLoc[0] = (s32)&D_80073D90[4];
                    gCdLoadSectors[0] = D_80073DB0[5];
                    break;
                case 6:
                    gCdLoadLoc[0] = (s32)&D_80073E20[4];
                    gCdLoadSectors[0] = D_80073E40[5];
                    break;
                case 7:
                    gCdLoadLoc[0] = (s32)&D_80073E68[4];
                    gCdLoadSectors[0] = D_80073E88[5];
                    break;
                case 9:
                    gCdLoadLoc[0] = (s32)&D_80073EF8[4];
                    gCdLoadSectors[0] = D_80073F18[5];
                    break;
                case 10:
                    gCdLoadLoc[0] = (s32)&D_80073F40[4];
                    gCdLoadSectors[0] = D_80073F60[5];
                    break;
                case 12:
                    gCdLoadLoc[0] = (s32)&D_80073FD0[4];
                    gCdLoadSectors[0] = D_80073FF0[5];
                    break;
                case 13:
                    gCdLoadLoc[0] = (s32)&D_80074018[4];
                    gCdLoadSectors[0] = D_80074038[5];
                    break;
            }
            gCdLoadDest[0] = 0x8016D000;
            gCdLoadCount = 1;
            gCdLoadState = 1;
            D_800958A6++;
            break;
        case 11:
            if (gCdLoadState == 6) {
                func_8002B7C8((DirEnt16 *)0x8016D000, 30);
                D_800958A6++;
            }
            break;
        case 12:
            func_8002CC24();
            D_800958A6++;
            break;
        case 13:
            switch (gStage) {
                case 0:
                    gCdLoadLoc[0] = (s32)&D_80073C70[5];
                    gCdLoadSectors[0] = D_80073C90[6];
                    break;
                case 1:
                    gCdLoadLoc[0] = (s32)&D_80073CB8[5];
                    gCdLoadSectors[0] = D_80073CD8[6];
                    break;
                case 3:
                    gCdLoadLoc[0] = (s32)&D_80073D48[5];
                    gCdLoadSectors[0] = D_80073D68[6];
                    break;
                case 4:
                    gCdLoadLoc[0] = (s32)&D_80073D90[5];
                    gCdLoadSectors[0] = D_80073DB0[6];
                    break;
                case 6:
                    gCdLoadLoc[0] = (s32)&D_80073E20[5];
                    gCdLoadSectors[0] = D_80073E40[6];
                    break;
                case 7:
                    gCdLoadLoc[0] = (s32)&D_80073E68[5];
                    gCdLoadSectors[0] = D_80073E88[6];
                    break;
                case 9:
                    gCdLoadLoc[0] = (s32)&D_80073EF8[5];
                    gCdLoadSectors[0] = D_80073F18[6];
                    break;
                case 10:
                    gCdLoadLoc[0] = (s32)&D_80073F40[5];
                    gCdLoadSectors[0] = D_80073F60[6];
                    break;
                case 12:
                    gCdLoadLoc[0] = (s32)&D_80073FD0[5];
                    gCdLoadSectors[0] = D_80073FF0[6];
                    break;
                case 13:
                    gCdLoadLoc[0] = (s32)&D_80074018[5];
                    gCdLoadSectors[0] = D_80074038[6];
                    break;
            }
            gCdLoadDest[0] = 0x8016D000;
            gCdLoadCount = 1;
            gCdLoadState = 1;
            D_800958A6++;
            break;
        case 14:
            if (gCdLoadState == 6) {
                func_800373C8((void *)0x8016D000, (void *)0x80171744);
                func_8002C2B4();
                func_8002C044();
                D_800958A6++;
            }
            break;
        case 15:
            if ((u8)(gStage % 3) == 1) {
                switch (gStage) {
                    case 1:
                        gCdLoadLoc[0] = (s32)&D_80073CB8[6];
                        gCdLoadSectors[0] = D_80073CD8[7];
                        break;
                    case 4:
                        gCdLoadLoc[0] = (s32)&D_80073D90[6];
                        gCdLoadSectors[0] = D_80073DB0[7];
                        break;
                    case 7:
                        gCdLoadLoc[0] = (s32)&D_80073E68[6];
                        gCdLoadSectors[0] = D_80073E88[7];
                        break;
                    case 10:
                        gCdLoadLoc[0] = (s32)&D_80073F40[6];
                        gCdLoadSectors[0] = D_80073F60[7];
                        break;
                    case 13:
                        gCdLoadLoc[0] = (s32)&D_80074018[6];
                        gCdLoadSectors[0] = D_80074038[7];
                        break;
                }
                gCdLoadDest[0] = 0x8018D000;
            }
            gCdLoadCount = 1;
            gCdLoadState = 1;
            D_800958A6++;
            break;
        case 16:
            if (gCdLoadState == 6) {
                func_8002C47C();
                StartStageOverlay();
                func_80018DE8(1);
                D_800D86B8[0] = 0;
                ResetStage();
                D_800958A6 = 100;
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


void func_800160E8(void);

/** @brief One step of @p obj: runs the stepper and marks a finished 0x61
 *         channel. Inlined here and by the out-of-line step below. */
static __inline__ void stepObj(Stepper *obj) {
    /* MATCHING: signed, so the zero test is `sll 24`, not `andi 0xFF`. */
    s8 result;

    result = func_80017F0C(obj, 0, obj->unk4);
    obj->unk340 = result;
    switch (obj->unk5) {
        case 0x60:
            break;
        case 0x61:
            if (result == 0) {
                obj->unk8 = 0xFF;
            }
            break;
    }
}

/** @brief Draws @p obj's models at its position and rotation. Inlined
 *         here and by the out-of-line draw below. */
static __inline__ void drawObj(Stepper *obj) {
    MATRIX m;
    SVECTOR rot;
    GsDOBJ2 *o;
    s32 i;

    func_80020CF8(D_80095758);
    o = sModels.objs;
    o->coord2->coord.t[0] = obj->unk348;
    o->coord2->coord.t[1] = obj->unk34C;
    o->coord2->coord.t[2] = obj->unk350;
    rot.vx = 0;
    rot.vy = obj->unk380;
    rot.vz = 0;
    func_80018AE0(&rot, o->coord2);
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

void func_80016D14(void) {
    /* MATCHING: retail reserves 8 bytes below the inlined draw's locals. */
    s32 unused[2];

    gRefView[0] = rsin(0x800) * 400 / 4096;
    gRefView[1] = -150;
    gRefView[2] = rcos(0x800) * 400 / 4096;
    gRefView[3] = 0;
    gRefView[4] = -150;
    gRefView[5] = 0;
    GsSetRefView2((GsRVIEW2 *)gRefView);
    stepObj((Stepper *)&D_80095C08);
    drawObj((Stepper *)&D_80095C08);
    func_800160E8();
    if (D_800958A6 == 100) {
        SetFogNearFar(0, 0, 250);
        SetFarColor(0, 0, 0);
        gModeStep = 4;
        D_80095980 = 0;
        D_800958A6 = 0;
        gGameMode = 14;
        PlayStageMusic(gStage);
    }
}

void func_80016FC0(void) {
    SVECTOR pos;

    pos.vx = 0x70;
    pos.vy = 0x48;
    func_8001B354(gFrameCount % 10 + 0xFB, &pos, NULL, 0, &gSubOt[gActiveBuff]);
    pos.vx = -0xA0;
    pos.vy = -0x78;
    func_8001B354(0x1FE, &pos, NULL, 0, &gSubOt[gActiveBuff]);
    pos.vx = 0;
    pos.vy = -0x78;
    func_8001B354(0x1FF, &pos, NULL, 0, &gSubOt[gActiveBuff]);
    func_800160E8();
    if (D_800958A6 == 100) {
        gModeStep = 4;
        D_80095980 = 0;
        D_800958A6 = 0;
        gGameMode = 14;
        PlayStageMusic(gStage);
    }
}

void func_80015CC8(void);

void func_80017124(void) {
    SVECTOR pos;

    pos.vx = 0x70;
    pos.vy = 0x48;
    func_8001B354(gFrameCount % 10 + 0xFB, &pos, NULL, 0, &gSubOt[gActiveBuff]);
    pos.vx = -0xA0;
    pos.vy = -0x78;
    func_8001B354(0x1FE, &pos, NULL, 0, &gSubOt[gActiveBuff]);
    pos.vx = 0;
    pos.vy = -0x78;
    func_8001B354(0x1FF, &pos, NULL, 0, &gSubOt[gActiveBuff]);
    func_80015CC8();
    if (D_800958A6 == 100) {
        gModeStep = 4;
        D_800958A6 = 0;
        gGameMode = 14;
    }
}

void func_80017270(Stepper *obj) {
    drawObj(obj);
}

void func_800173E8(Stepper *obj) {
    stepObj(obj);
}

extern CdlLOC gCdToc[];
extern s32 gCdTrackEnd;
extern s32 gCdTrackLoop;
/* MATCHING: a known small size, so the byte store is `$gp`-relative. */
extern u8 D_80095AC0[4];

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
    D_80095AC0[0] = 3;
    CdControlB(CdlSetmode, D_80095AC0, 0);
    VSync(3);
    CdReadyCallback((CdlCB)OnCdTrackReady);
    SendCdTrackCommand(CdlPlay);
    gCdTrackPlaying = 1;
}

INCLUDE_RODATA("asm/nonmatchings/main", D_80010404);
