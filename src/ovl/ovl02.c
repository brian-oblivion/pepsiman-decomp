#include "common.h"
#include "ram.h"
#include "libgte.h"
#include "libgpu.h"
#include "libgs.h"
#include "libetc.h"
#include "libcd.h"
#include "sound.h"
#include "libsnd.h"
#include "libmcrd.h"
#include "libpad.h"
#include "rand.h"
#include "model.h"
#include "draw.h"
#include "spad.h"
#include "world_intro.h"

/* MATCHING: non-static inline, emitted at the file's end in this order. */
inline void DrawTitlePlayerShadow(void);
inline void DrawTitleGroundShadow(void);
inline void DrawMenuPlayer(void);
inline void DrawScreenFade(void);
inline void DrawMenuCursor(SVECTOR *pos, s32 kind);
inline void StepMenuMotion(PlayerState *obj);
inline void StartFadeIn(void);
inline void StartFadeOut(void);
inline void OrbitMenuCamera(SVECTOR *rot);

/** @brief The three flat lights of the scene. */
typedef struct {
    GsF_LIGHT l[3]; /**< one per light index */
} LightTable;

/* MATCHING: a struct lvalue keeps the table's base in a register. */
#define sLights (*(LightTable *)gFlatLights)

/** @brief Two words, copied as a pair (a unit-local view). */
typedef struct {
    s32 a; /**< first word */
    s32 b; /**< second word */
} Pair8;

/** @brief The game's save as the title reads it from the memory card. */
typedef struct {
    Pair8 times[4];     /**< gBestTimes */
    Pair8 cans[4];      /**< gBestCans */
    u8 pad40[8];        /**< not read here */
    Pair8 continueCans; /**< the continue point's act cans */
    u8 stereo;          /**< gStereo */
    u8 rumble;          /**< gRumbleEnabled */
    s8 sortMode;        /**< gTmdSortMode */
    s8 buttonConfig;    /**< gButtonConfig */
    s8 continueStage;   /**< the stage to continue from; -1 for none */
    u8 gameCleared;     /**< gGameCleared */
    u8 allTargets;      /**< gAllTargetsBeaten */
    u8 unlocks;         /**< gUnlocks */
    s32 checksum;       /**< the complement of the byte sum of the 0x5C bytes, read as 0 */
} SaveData;

/** @brief A memory card file's header: the first 0x200 bytes of the file,
 *         which the BIOS's card screen reads. */
typedef struct {
    char magic[2];   /**< "SC" */
    u8 iconFlag;     /**< 0x13: an icon of three frames */
    u8 blocks;       /**< the file's size in 8 KB blocks */
    char title[64];  /**< the title, in Shift-JIS */
    u8 pad44[28];    /**< zero */
    u16 clut[16];    /**< the icon's palette */
    u8 icon[3][128]; /**< the icon's frames, 16x16 at 4 bits a pixel */
} CardFileHeader;

/* The overlay's data, in address order. */

extern const char sSaveFileName[]; /* in the rodata, below */

#define DISC_FILE(x) sFiles1##x
#include "disc_files.inc"
#define DISC_FILE(x) sFiles2##x
#include "disc_files.inc"

/* The save file's header, the first 0x200 bytes the card holds. */
CardFileHeader sCardFileHeader = {
    {'S', 'C'},
    0x13,
    1,
    /* "PEPSIMAN" in katakana, Shift-JIS */
    "\x83\x79\x83\x76\x83\x56\x83\x7D\x83\x93",
    {0},
    {0x39CE, 0x5674, 0x2929, 0x4C00, 0x54E7, 0x8000, 0x1C00, 0x3000, 0x673B, 0x0004, 0x0012, 0x000A,
     0x043A, 0x18DD, 0x2116, 0x7FFF},
    /* clang-format off: one line per row of 16 pixels */
    {
        {
            0x55, 0x55, 0x95, 0xB5, 0xBB, 0x5B, 0x55, 0x55, 0x55, 0x55, 0xB9, 0xAA, 0xCA,
            0xAA, 0x59, 0x55, 0x55, 0xB5, 0xAA, 0xAA, 0xCA, 0xCC, 0xBC, 0x55, 0x55, 0xBB,
            0xAB, 0xCA, 0xCC, 0xCC, 0xCC, 0x59, 0x95, 0xBB, 0xAB, 0xAC, 0xCC, 0xDD, 0xCD,
            0x5C, 0xB9, 0xBB, 0xAA, 0xCA, 0xDC, 0xDD, 0xDD, 0xBC, 0x29, 0x02, 0x10, 0xE0,
            0xDE, 0xDD, 0xDD, 0x0D, 0x02, 0x00, 0x11, 0x88, 0xF8, 0xF8, 0xF8, 0x8F, 0x02,
            0x00, 0x11, 0x81, 0xF8, 0xFF, 0xFF, 0x1F, 0x22, 0x76, 0x37, 0x44, 0x11, 0xFF,
            0xFF, 0x4F, 0x76, 0x77, 0x77, 0x37, 0x33, 0x44, 0x41, 0x54, 0x65, 0x77, 0x77,
            0x33, 0x33, 0x33, 0x33, 0x53, 0x65, 0x77, 0x77, 0x37, 0x37, 0x33, 0x33, 0x56,
            0x55, 0x76, 0x77, 0x77, 0x33, 0x33, 0x73, 0x55, 0x55, 0x65, 0x77, 0x77, 0x37,
            0x37, 0x56, 0x55, 0x55, 0x55, 0x65, 0x76, 0x66, 0x56, 0x55, 0x55,
        },
        {
            0x55, 0x55, 0x95, 0xB5, 0xBB, 0x5B, 0x55, 0x55, 0x55, 0x55, 0xB9, 0xAA, 0xCA,
            0xAA, 0x59, 0x55, 0x55, 0xB5, 0xAA, 0xAA, 0xCA, 0xCC, 0xBC, 0x55, 0x55, 0xBB,
            0xAB, 0xCA, 0xCC, 0xCC, 0xCC, 0x59, 0xB5, 0xBB, 0xAB, 0xCA, 0xCC, 0xDD, 0xCD,
            0x5C, 0xB9, 0xBB, 0xAA, 0xAE, 0xCD, 0xDD, 0xDD, 0xBC, 0xB5, 0x0B, 0x10, 0x88,
            0x88, 0xE8, 0xDD, 0xAC, 0x22, 0x00, 0x11, 0x81, 0xF8, 0xFF, 0x8F, 0x18, 0x02,
            0x00, 0x11, 0x11, 0xF1, 0xFF, 0xFF, 0x8F, 0x02, 0x20, 0x42, 0x44, 0x44, 0x14,
            0xF8, 0x1F, 0x26, 0x67, 0x77, 0x37, 0x33, 0x33, 0x43, 0x24, 0x75, 0x77, 0x77,
            0x33, 0x33, 0x33, 0x33, 0x63, 0x65, 0x77, 0x77, 0x37, 0x37, 0x33, 0x33, 0x56,
            0x55, 0x76, 0x77, 0x77, 0x33, 0x33, 0x73, 0x55, 0x55, 0x65, 0x77, 0x77, 0x37,
            0x37, 0x56, 0x55, 0x55, 0x55, 0x65, 0x76, 0x66, 0x56, 0x55, 0x55,
        },
        {
            0x55, 0x55, 0x95, 0xB5, 0xBB, 0x5B, 0x55, 0x55, 0x55, 0x55, 0xB9, 0xAA, 0xCA,
            0xAA, 0x59, 0x55, 0x55, 0xB5, 0xAA, 0xAA, 0xCA, 0xCC, 0xBC, 0x55, 0x55, 0xBB,
            0xAB, 0xCA, 0xCC, 0xCC, 0xCC, 0x59, 0x95, 0xBB, 0xAA, 0xAC, 0xCC, 0xDD, 0xDD,
            0x5C, 0xB9, 0xBB, 0xAA, 0xCA, 0xDC, 0xDD, 0xDD, 0xBC, 0xB5, 0xAB, 0xAA, 0xEE,
            0x88, 0xFF, 0x8F, 0xA8, 0x02, 0x02, 0x10, 0x81, 0xF8, 0xFF, 0xFF, 0x1F, 0x02,
            0x00, 0x10, 0x88, 0xF8, 0xFF, 0xFF, 0x8F, 0x06, 0x00, 0x11, 0x11, 0x11, 0x44,
            0x34, 0x14, 0x76, 0x27, 0x42, 0x74, 0x33, 0x33, 0x33, 0x73, 0x65, 0x77, 0x77,
            0x73, 0x33, 0x33, 0x33, 0x53, 0x65, 0x77, 0x77, 0x37, 0x37, 0x33, 0x33, 0x56,
            0x55, 0x76, 0x77, 0x77, 0x33, 0x33, 0x73, 0x55, 0x55, 0x65, 0x77, 0x77, 0x37,
            0x37, 0x56, 0x55, 0x55, 0x55, 0x65, 0x76, 0x76, 0x56, 0x55, 0x55,
        },
    },
    /* clang-format on */
};

/* Each button config's icons for the two actions the options screen shows
 * (and a third), as sprite offsets from 0x69. */
s8 sButtonIcons[4][4] = {{1, 2, 0, 0}, {2, 1, 0, 0}, {1, 0, 2, 0}, {0, 1, 2, 0}};

/* The copy of the disc's file table the menu loads from. */
#define DISC_FILE(x) sFiles3##x
#include "disc_files.inc"

/* The title model's rotation after it lands, one per frame. */
SVECTOR sWobbleRots[30] = {
    /*   0 */ {0, -716, 0, 0},
    /*   1 */ {-3, -716, 0, 0},
    /*   2 */ {-6, -716, 0, 0},
    /*   3 */ {-11, -728, 0, 0},
    /*   4 */ {-14, -728, 0, 0},
    /*   5 */ {-18, -728, 0, 0},
    /*   6 */ {-22, -739, 0, 0},
    /*   7 */ {-11, -739, 0, 0},
    /*   8 */ {0, -739, 0, 0},
    /*   9 */ {11, -762, 0, 0},
    /*  10 */ {11, -762, 0, 0},
    /*  11 */ {5, -807, 0, 0},
    /*  12 */ {0, -853, 0, 0},
    /*  13 */ {0, -898, 0, 0},
    /*  14 */ {-11, -944, 0, 0},
    /*  15 */ {-22, -989, 0, 0},
    /*  16 */ {-22, -1012, 0, 0},
    /*  17 */ {-11, -1035, 0, 0},
    /*  18 */ {-5, -1046, 0, 0},
    /*  19 */ {0, -1058, 0, 0},
    /*  20 */ {11, -1058, 0, 0},
    /*  21 */ {22, -1069, 0, 0},
    /*  22 */ {17, -1069, 0, 0},
    /*  23 */ {0, -1080, 0, 0},
    /*  24 */ {0, -1069, 0, 0},
    /*  25 */ {0, -1069, 0, 0},
    /*  26 */ {0, -1069, 0, 0},
    /*  27 */ {0, -1069, 0, 0},
    /*  28 */ {0, -1069, 0, 0},
    /*  29 */ {0, -1069, 0, 0},
};

#define DISC_FILE(x) sFiles4##x
#include "disc_files.inc"

s32 sOpeningTime = 0;           /* frames the opening has run */
u32 sOpeningSpare = 0xEDF014FE; /* never read */
s32 sCardStep = 0;              /* the memory card load's step */
s32 sCardStepSpare = 0;         /* never read */
SVECTOR sMenuModelRot = {0};    /* the menu model's rotation */
s32 sCardMessage = 0;           /* the sprite of the card error shown */
s32 sOptionsCursor = 0;         /* the options menu's line */
s32 sSaveChecksum = 0;          /* the checksum the save holds */
s32 sMenuTime = 0;              /* frames since the menu began sliding */
s32 sMenuCancelled = 0;         /* 1 when the stage select was backed out of */
SaveData *sSave = NULL;         /* the save, in the card buffer */
s32 sSaveSum = 0;               /* the checksum computed */
s32 sPadHasRumble = 0;          /* 1 when the pad can vibrate */
s32 sMenuScene = 0;             /* which model the menu draws */
s32 sBootCardWait = 0;          /* frames the boot's card read waits */
s8 sMenuItems[8] = {0};         /* the main menu's entries */
s8 sMenuItemCount = 0;          /* how many */
s32 sCardMessageTime = 0;       /* frames the card error has shown */
long sCardResult = 0;           /* the last MemCardSync result */
s32 sMenuCursor = 0;            /* the main menu's entry */
s32 sTitleStarted = 0;          /* 1 once START was pressed on the title */
s32 sWobbleTime = 0;            /* the title model's frame */
s32 sTitleCamTime = 0;          /* the title camera's clock */
s32 sTitleExitTime = 0;         /* frames since START on the title */
s32 sFadeTime = 0;              /* the screen fade's frame */
s32 sFadeMode = 0;              /* the fade: 2 fading in, then 1; 3 fading out, then 0 */

void StepOpening(void) {
    SVECTOR pos;
    SColor color;

    switch (gSubStep) {
        case 50:
            DefineSprite(0x1E, 0, 0x80, 0xF0, 0x15, 0, 0, 0, 0x1F3);
            DefineSprite(0x1F, 0, 0x80, 0xF0, 0x15, 0x80, 0, 0, 0x1F3);
            DefineSprite(0x20, 0, 0x40, 0xF0, 0x16, 0, 0, 0, 0x1F3);
            sLights.l[0].r = 0x80;
            sLights.l[0].g = 0x80;
            sLights.l[0].b = 0x80;
            sLights.l[1].r = 0x80;
            sLights.l[1].g = 0x80;
            sLights.l[1].b = 0x80;
            sLights.l[2].r = 0x80;
            sLights.l[2].g = 0x80;
            sLights.l[2].b = 0x80;
            sLights.l[0].vx = 0;
            sLights.l[0].vy = 100;
            sLights.l[0].vz = 100;
            GsSetFlatLight(0, &sLights.l[0]);
            sLights.l[1].vx = 0x56;
            sLights.l[1].vy = 100;
            sLights.l[1].vz = -50;
            GsSetFlatLight(1, &sLights.l[1]);
            sLights.l[2].vx = -0x56;
            sLights.l[2].vy = 100;
            sLights.l[2].vz = -50;
            GsSetFlatLight(2, &sLights.l[2]);
            gPlayer.action = 0x61;
            gPlayer.motion[0] = 3;
            gPlayer.z = -600;
            gPlayer.speed = 0;
            gPlayer.x = 0;
            gPlayer.y = 100;
            gPlayer.yaw = 0x800;
            sView.vpx = 500;
            sView.vpy = -100;
            sView.vpz = 0;
            sView.vrx = 0;
            sView.vry = 0;
            sView.vrz = 0;
            sOpeningTime = 0;
            gSubStep = 100;
            StartFadeIn();
            break;
        case 100:
            DrawScreenFade();
            color.r = -1;
            color.g = color.b = color.cd = 0x80;
            pos.vx = -160;
            pos.vy = -120;
            DrawSprite(0x1E, &pos, (CVECTOR *)&color, 1, gCurOt);
            pos.vx += 0x80;
            DrawSprite(0x1F, &pos, (CVECTOR *)&color, 1, gCurOt);
            pos.vx += 0x80;
            DrawSprite(0x20, &pos, (CVECTOR *)&color, 1, gCurOt);
            if (sFadeMode == 1) {
                sOpeningTime++;
                gPlayer.z += 20;
                if (sOpeningTime >= 60) {
                    StartFadeOut();
                }
            }
            if (sFadeMode == 0) {
                gModeStep = 5;
            }
            if (gPadTrig & PADstart) {
                gModeStep = 5;
            }
            break;
    }
}

void DrawCansAndTime(SVECTOR *pos, s32 count, s32 frames) {
    SColor color;
    s32 x;
    s32 n;
    s32 m;
    s32 d;
    s32 k;
    s32 t;
    s32 sec;
    /* MATCHING: two dead divisions keep the /10 and /100 constants in
     * prologue registers; the /100 one only after the first draw. */
    u32 dead;

    dead = (s16)count / 10;
    color.r = BLEND_ADD;
    color.g = color.b = color.cd = 0x80;
    x = pos->vx;
    pos->vx -= 8;
    DrawSprite(0x2E, pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
    pos->vx -= 8;
    DrawSprite(0x2E, pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
    pos->vx -= 8;
    DrawSprite(0x2F, pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
    pos->vx -= 8;
    DrawSprite(0x39, pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
    n = count;
    dead = (u32)frames / 100;
    count /= 10;
    pos->vx -= 8;
    DrawSprite(n % 10 + 0x2E, pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
    if (count != 0) {
        pos->vx -= 8;
        DrawSprite(count % 10 + 0x2E, pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
    }
    if (n / 100 != 0) {
        pos->vx -= 8;
        DrawSprite(n / 100 % 10 + 0x2E, pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
    }
    n = frames * 100 / 30;
    d = n;
    d /= 10;
    pos->vx = x - 8;
    pos->vy += 8;
    DrawSprite(n % 10 + 0x2E, pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
    pos->vx -= 8;
    DrawSprite(d % 10 + 0x2E, pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
    pos->vx -= 8;
    DrawSprite(0x38, pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
    sec = frames / 30;
    m = sec / 60;
    n = sec % 60;
    k = 1;
    d = n;
    d /= 10;
    pos->vx -= 8;
    DrawSprite(n % 10 + 0x2E, pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
    pos->vx -= 8;
    DrawSprite(d % 10 + 0x2E, pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
    n = m;
    pos->vx -= 8;
    DrawSprite(0x38, pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
    for (; k < 11; k *= 10) {
        t = n / k;
        if (t != 0 || (k == 1 && n == 0)) {
            pos->vx -= 8;
            DrawSprite(t % 10 + 0x2E, pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
        }
    }
}

/* MATCHING: per-unit views, as in main.c. */
extern Pair8 gBestCans[];
extern Pair8 gBestTimes[];
extern Pair8 D_80095978;
extern u8 gRumbleEnabled;
extern s8 D_800957A0;
extern u8 gGameCleared;

void StepCardLoad(void) {
    s32 bad;
    u32 i;

    switch (gSubStep) {
        case 50:
            sBootCardWait = 0;
            gSubStep = 51;
            break;
        case 51:
            if (IsScreenWipeActive() == 0 && sBootCardWait == 0) {
                sBootCardWait = 1;
            }
            if (sBootCardWait != 0) {
                sBootCardWait++;
            }
            if (sBootCardWait == 8) {
                MemCardSync(0, 0, &sCardResult);
                switch (sCardResult) {
                    case 0:
                    case 3:
                        bad = 0;
                        break;
                    default:
                        bad = 1;
                        break;
                }
                if (bad == 0) {
                    MemCardReadFile(0, (char *)sSaveFileName, (unsigned long *)RAM_WORK_A, 0x200, 0x80);
                    MemCardSync(0, 0, &sCardResult);
                    if (sCardResult == 0) {
                        sSave = (SaveData *)RAM_WORK_A;
                        sSaveChecksum = sSave->checksum;
                        sSave->checksum = 0;
                        sSaveSum = 0;
                        for (i = 0; i < 0x5C; i++) {
                            sSaveSum += ((u8 *)sSave)[i];
                        }
                        sSaveSum = ~sSaveSum;
                        if (sSaveSum == sSaveChecksum) {
                            gBestTimes[0].a = sSave->times[0].a;
                            gBestTimes[1].a = sSave->times[1].a;
                            gBestTimes[2].a = sSave->times[2].a;
                            gBestTimes[3].a = sSave->times[3].a;
                            gBestTimes[0].b = sSave->times[0].b;
                            gBestTimes[1].b = sSave->times[1].b;
                            gBestTimes[2].b = sSave->times[2].b;
                            gBestTimes[3].b = sSave->times[3].b;
                            gBestCans[0].a = sSave->cans[0].a;
                            gBestCans[1].a = sSave->cans[1].a;
                            gBestCans[2].a = sSave->cans[2].a;
                            gBestCans[3].a = sSave->cans[3].a;
                            gBestCans[0].b = sSave->cans[0].b;
                            gBestCans[1].b = sSave->cans[1].b;
                            gBestCans[2].b = sSave->cans[2].b;
                            gBestCans[3].b = sSave->cans[3].b;
                            D_80095978.a = sSave->continueCans.a;
                            D_80095978.b = sSave->continueCans.b;
                            gStereo = sSave->stereo;
                            gRumbleEnabled = sSave->rumble;
                            gTmdSortMode = sSave->sortMode;
                            gButtonConfig = sSave->buttonConfig;
                            D_800957A0 = sSave->continueStage;
                            gGameCleared = sSave->gameCleared;
                            gAllTargetsBeaten = sSave->allTargets;
                            gUnlocks = sSave->unlocks;
                            if (gStereo != 0) {
                                SsSetStereo();
                            } else {
                                SsSetMono();
                            }
                            switch (gTmdSortMode) {
                                case 0:
                                case 2:
                                case 3:
                                    LoadTimList((unsigned long *)(*(s32 *)(RAM_STAGE_LOAD + 4) +
                                                                  RAM_STAGE_LOAD));
                                    break;
                                case 1:
                                    LoadTimList((unsigned long *)(*(s32 *)(RAM_STAGE_LOAD + 8) +
                                                                  RAM_STAGE_LOAD));
                                    break;
                            }
                        }
                    }
                }
                gModeStep = 40;
                gSubStep = 50;
            }
            break;
    }
}

void DefineMenuSprites(void) {
    s32 i;

    for (i = 0; i < 7; i++) {
        DefineSprite(i + 0x1E, 0, 0xA0, 0x10, 6, 0, (u8)(i * 16), 0, 0x1E0);
    }
    for (i = 0; i < 7; i++) {
        DefineSprite(i + 0x26, 0, 0x60, 0x10, 6, 0xA0, (u8)(i * 16), 0, 0x1E0);
    }
    DefineSprite(0x25, 0, 0x80, 0x10, 6, 0, 0x70, 0, 0x1E0);
    DefineSprite(0x2D, 0, 0x20, 0x10, 6, 0x80, 0x70, 0, 0x1E0);
    for (i = 0; i < 12; i++) {
        DefineSprite(i + 0x2E, 0, 8, 8, 6, (u8)(i * 8 + 0xA0), 0x70, 0, 0x1E0);
    }
    DefineSprite(0x3A, 0, 0x50, 0x10, 6, 0, 0x80, 0, 0x1E0);
    for (i = 0; i < 3; i++) {
        DefineSprite(i + 0x3B, 0, 0x80, 0x10, 6, 0, (u8)(i * 16 + 0x90), 0, 0x1E0);
    }
    DefineSprite(0x3E, 0, 0x30, 0x10, 6, 0x50, 0x80, 0, 0x1E0);
    DefineSprite(0x3F, 0, 0x40, 0x10, 6, 0, 0xC0, 0, 0x1E0);
    DefineSprite(0x40, 0, 0x60, 0x10, 6, 0x40, 0xC0, 0, 0x1E0);
    DefineSprite(0x41, 0, 0x90, 0x10, 6, 0, 0xD0, 0, 0x1E0);
    DefineSprite(0x42, 0, 0x50, 0x10, 6, 0, 0xE0, 0, 0x1E0);
    DefineSprite(0x43, 0, 0x50, 0x10, 6, 0x50, 0xE0, 0, 0x1E0);
    DefineSprite(0x44, 0, 0x50, 0x10, 6, 0, 0xF0, 0, 0x1E0);
    DefineSprite(0x45, 0, 0x50, 0x10, 6, 0x50, 0xF0, 0, 0x1E0);
    for (i = 0; i < 8; i++) {
        DefineSprite(i + 0x46, 0, 0x60, 0x10, 6, 0xA0, (u8)(i * 16 + 0x80), 0, 0x1E0);
    }
    DefineSprite(0x4E, 0, 0x10, 0x10, 6, 0x80, 0x80, 0, 0x1E0);
    DefineSprite(0x4F, 0, 0x10, 0x10, 6, 0x90, 0x80, 0, 0x1E0);
    DefineSprite(0x50, 0, 0x10, 0x10, 6, 0x80, 0x90, 0, 0x1E0);
    DefineSprite(0x51, 0, 0x10, 0x10, 6, 0x80, 0xA0, 0, 0x1E0);
    DefineSprite(0x52, 0, 0x10, 0x10, 6, 0x80, 0xB0, 0, 0x1E0);
    DefineSprite(0x53, 0, 0xA0, 0x20, 7, 0, 0x80, 0x30, 0x1E0);
    DefineSprite(0x54, 0, 0x80, 0x20, 7, 0, 0xA0, 0x30, 0x1E0);
    DefineSprite(0x55, 0, 0x90, 0x20, 7, 0, 0xC0, 0x30, 0x1E0);
    DefineSprite(0x56, 0, 0x88, 0x20, 7, 0, 0xE0, 0x30, 0x1E0);
    for (i = 0; i < 18; i++) {
        DefineSprite(i + 0x57, 0, 0xFF, 0xE, 5, 0, (u8)(i * 14), 0x10, 0x1E0);
    }
    DefineSprite(0x69, 0, 0x10, 0x10, 7, 0, 0, 0x20, 0x1E0);
    DefineSprite(0x6A, 0, 0x10, 0x10, 7, 0x10, 0, 0x20, 0x1E0);
    DefineSprite(0x6B, 0, 0x10, 0x10, 7, 0x20, 0, 0x20, 0x1E0);
}

extern GsCOORDINATE2 sAct3Coords[];
extern GsDOBJ2 sAct3Objs[];
extern VECTOR D_8009EA78[];
extern SVECTOR D_8009EB58[];

void DrawMenuObject(s32 i) {
    MATRIX m;

    sAct3Coords[i].coord.t[0] = D_8009EA78[i].vx;
    sAct3Coords[i].coord.t[1] = D_8009EA78[i].vy;
    sAct3Coords[i].coord.t[2] = D_8009EA78[i].vz;
    SetCoordRotation(&D_8009EB58[i], &sAct3Coords[i]);
    GsGetLs(sAct3Objs[i].coord2, &m);
    GsSetLsMatrix(&m);
    GsGetLw(sAct3Objs[i].coord2, &m);
    GsSetLightMatrix(&m);
    SetTmdSortMode(0);
    GsSortObject4J(&sAct3Objs[i], &gSubOt[gActiveBuff], 2, getScratchAddr(0));
}

/* The save file's name on the memory card. */
const char sSaveFileName[] = "BISLPS-01762";

extern s32 D_80095984;
extern s32 D_80095998;
extern Pair8 gActTimes;
extern Pair8 gActCans;
extern Pair8 D_800958C0;
extern u16 gRumbleTime;
/* MATCHING: the small and large strengths read as one array; sbss.c has two
 * scalars. */
extern u16 gRumbleSmall[];

void StepMainMenu(void) {
    SVECTOR pos;
    SVECTOR p2;
    SVECTOR unused;
    SColor col;
    SColor norm;
    SColor hi;
    u32 i;
    s32 t;
    s32 d;

    switch (gSubStep) {
        case 50:
            sMenuScene = 0;
            DefineMenuSprites();
            GsInitCoordinate2(&sAct3Coords[0], NULL);
            GsInitCoordinate2(&sAct3Coords[1], NULL);
            GsInitCoordinate2(&sAct3Coords[2], NULL);
            GsMapModelingData((unsigned long *)(*(s32 *)(RAM_STAGE_LOAD + 0x18) + (RAM_STAGE_LOAD + 4)));
            GsMapModelingData((unsigned long *)(*(s32 *)(RAM_STAGE_LOAD + 0x1C) + (RAM_STAGE_LOAD + 4)));
            GsMapModelingData((unsigned long *)(*(s32 *)(RAM_STAGE_LOAD + 0x20) + (RAM_STAGE_LOAD + 4)));
            GsLinkObject4(*(s32 *)(RAM_STAGE_LOAD + 0x18) + (RAM_STAGE_LOAD + 0xC), &sAct3Objs[0], 0);
            GsLinkObject4(*(s32 *)(RAM_STAGE_LOAD + 0x1C) + (RAM_STAGE_LOAD + 0xC), &sAct3Objs[1], 0);
            GsLinkObject4(*(s32 *)(RAM_STAGE_LOAD + 0x20) + (RAM_STAGE_LOAD + 0xC), &sAct3Objs[2], 0);
            sAct3Objs[0].coord2 = &sAct3Coords[0];
            sAct3Objs[0].attribute = 0;
            sAct3Objs[1].coord2 = &sAct3Coords[1];
            sAct3Objs[1].attribute = 0;
            sAct3Objs[2].coord2 = &sAct3Coords[2];
            sAct3Objs[2].attribute = 0;
            D_8009EA78[0].vx = D_8009EA78[1].vx = D_8009EA78[2].vx = 0;
            D_8009EA78[0].vy = D_8009EA78[1].vy = D_8009EA78[2].vy = 0;
            D_8009EA78[0].vz = D_8009EA78[1].vz = D_8009EA78[2].vz = 0;
            D_8009EB58[0].vx = D_8009EB58[1].vx = D_8009EB58[2].vx = 0;
            D_8009EB58[0].vy = D_8009EB58[1].vy = D_8009EB58[2].vy = 0;
            D_8009EB58[0].vz = D_8009EB58[1].vz = D_8009EB58[2].vz = 0;
            sLights.l[0].r = 0x80;
            sLights.l[0].g = 0x80;
            sLights.l[0].b = 0x80;
            sLights.l[1].r = 0x80;
            sLights.l[1].g = 0x80;
            sLights.l[1].b = 0x80;
            sLights.l[2].r = 0x80;
            sLights.l[2].g = 0x80;
            sLights.l[2].b = 0x80;
            sLights.l[0].vx = 0;
            sLights.l[0].vy = 100;
            sLights.l[0].vz = 100;
            GsSetFlatLight(0, &sLights.l[0]);
            sLights.l[1].vx = 0x56;
            sLights.l[1].vy = 100;
            sLights.l[1].vz = -50;
            GsSetFlatLight(1, &sLights.l[1]);
            sLights.l[2].vx = -0x56;
            sLights.l[2].vy = 100;
            sLights.l[2].vz = -50;
            GsSetFlatLight(2, &sLights.l[2]);
            gPlayer.action = 0x61;
            gPlayer.motion[0] = 3;
            gPlayer.x = 0;
            gPlayer.y = 0;
            gPlayer.z = 0;
            gPlayer.yaw = 0;
            sMenuModelRot.vx = 0;
            sMenuModelRot.vy = 0;
            sMenuModelRot.vz = 0;
            switch (gStageTableSet) {
                case -1:
                case 0:
                case 1:
                    gSubStep = 100;
                    break;
                case 2:
                case 3:
                    sMenuScene = 2;
                    gSubStep = 1000;
                    break;
            }
            break;
        case 100:
            sMenuScene = 0;
            sMenuItemCount = 0;
            sMenuItems[sMenuItemCount] = 0;
            sMenuItemCount++;
            if (D_800957A0 != -1) {
                sMenuItems[sMenuItemCount] = 1;
                sMenuItemCount++;
            }
            /* MATCHING: retail tests the flags with lb, an s8 view. */
            if (*(s8 *)&gGameCleared != 0) {
                sMenuItems[sMenuItemCount] = 2;
                sMenuItemCount++;
            }
            if (*(s8 *)&gAllTargetsBeaten != 0) {
                sMenuItems[sMenuItemCount] = 3;
                sMenuItemCount++;
            }
            sMenuItems[sMenuItemCount] = 4;
            sMenuItemCount++;
            sMenuItems[sMenuItemCount] = 5;
            sMenuItemCount++;
            sMenuItems[sMenuItemCount] = 6;
            sMenuItemCount++;
            sMenuCursor = 0;
            sMenuTime = 0;
            gSubStep = 101;
            break;
        case 101:
            col.r = -1;
            col.g = col.b = col.cd = 0x80;
            sMenuTime++;
            for (i = 0; (s32)i < sMenuItemCount; i++) {
                t = sMenuTime - i * 2;
                if (t > 8) {
                    t = 8;
                }
                pos.vx = t * 32 - 0x150;
                /* MATCHING: the cast keeps i * 24 - 12 one induction value;
                 * without it the -12 moves onto the count. */
                pos.vy = (s16)(i * 24 - 12) - sMenuItemCount * 12;
                DrawSprite(sMenuItems[i] + 0x1E, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            }
            if (sMenuTime == sMenuItemCount * 2 + 8) {
                gSubStep = 102;
            }
            break;
        case 102:
            norm.r = -1;
            norm.g = norm.b = norm.cd = 0x80;
            hi.r = -1;
            hi.g = hi.b = 0x80;
            hi.cd = 0;
            for (i = 0; (s32)i < sMenuItemCount; i++) {
                if (i == sMenuCursor) {
                    col = hi;
                } else {
                    col = norm;
                }
                /* MATCHING: t = 8 as in the slides; its hoisted set halves later hoisting. */
                t = 8;
                pos.vx = t * 32 - 0x150;
                pos.vy = (s16)(i * 24 - 12) - sMenuItemCount * 12;
                DrawSprite(sMenuItems[i] + 0x1E, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            }
            p2.vx = -0x50;
            p2.vy = sMenuCursor * 24 - sMenuItemCount * 12 - 0x15;
            DrawMenuCursor(&p2, 1);
            if (gPadTrig & PADLup) {
                PlaySe(0xB);
                if (sMenuCursor == 0) {
                    sMenuCursor = sMenuItemCount - 1;
                } else {
                    sMenuCursor--;
                }
            } else if (gPadTrig & PADLdown) {
                PlaySe(0xB);
                if (sMenuCursor == sMenuItemCount - 1) {
                    sMenuCursor = 0;
                } else {
                    sMenuCursor++;
                }
            } else if (gPadTrig & PADRright) {
                PlaySe(0xC);
                switch (sMenuItems[sMenuCursor]) {
                    case 0:
                    case 1:
                    case 2:
                    case 3:
                    case 4:
                        sMenuTime = 0;
                        gSubStep = 103;
                        break;
                    case 5:
                        sCardStep = 0;
                        gSubStep = 5000;
                        break;
                    case 6:
                        sCardStep = 0;
                        gSubStep = 6000;
                        break;
                }
            }
            break;
        case 103:
            col.r = -1;
            col.g = col.b = col.cd = 0x80;
            sMenuTime++;
            for (i = 0; (s32)i < sMenuItemCount; i++) {
                t = sMenuTime - i * 2;
                if (t < 0) {
                    t = 0;
                }
                pos.vx = t * 32 - 0x50;
                pos.vy = (s16)(i * 24 - 12) - sMenuItemCount * 12;
                DrawSprite(sMenuItems[i] + 0x1E, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            }
            if (sMenuTime == sMenuItemCount * 2 + 12) {
                switch (sMenuItems[sMenuCursor]) {
                    case 0:
                        gStageTableSet = 0;
                        gRetries = 3;
                        gStage = 0;
                        gGameMode = 0x2C;
                        break;
                    case 1:
                        gRetries = 3;
                        if (D_800957A0 >= 100) {
                            gStageTableSet = 1;
                            gStage = D_800957A0 - 100;
                        } else {
                            gStageTableSet = 0;
                            gStage = D_800957A0;
                        }
                        gActTimes.a = D_800958C0.a;
                        gActTimes.b = D_800958C0.b;
                        gActCans.a = D_80095978.a;
                        gActCans.b = D_80095978.b;
                        switch ((u8)(gStage % 3)) {
                            case 0:
                                BeginLoadScreenModel();
                                break;
                            case 1:
                                BeginLoadScreen();
                                break;
                            case 2:
                                BeginAct3LoadScreen();
                                break;
                        }
                        break;
                    case 2:
                        gSubStep = 1000;
                        break;
                    case 3:
                        gStageTableSet = 1;
                        gRetries = 3;
                        gStage = 0;
                        gGameMode = 0x2C;
                        break;
                    case 4:
                        sOptionsCursor = 0;
                        gSubStep = 2000;
                        break;
                }
            }
            break;
        case 1000:
            sMenuScene = 2;
            sMenuTime = 0;
            sMenuCancelled = 0;
            gSubStep++;
            break;
        case 1001:
            col.r = -1;
            col.g = col.b = col.cd = 0x80;
            t = ++sMenuTime;
            if (t > 8) {
                t = 8;
            }
            pos.vx = t * 32 - 0x150;
            pos.vy = -0x58;
            DrawSprite(0x20, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            t = sMenuTime - 2;
            if (t > 8) {
                t = 8;
            }
            pos.vx = t * 32 - 0x130;
            pos.vy = -0x38;
            DrawSprite(D_80095998 + 0x26, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            p2.vy = pos.vy;
            /* MATCHING: the bob in a block-local with an (s16) cast; a shared
             * local takes the division's register, no cast folds the + 8. */
            {
                s32 b = rsin(gFrameCount * 0xA000 / 360) / 512;
                p2.vx = pos.vx - (s16)((b >= 0 ? b : -b) + 8);
            }
            DrawSprite(0x4E, &p2, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            {
                s32 b = rsin(gFrameCount * 0xA000 / 360) / 512;
                p2.vx = pos.vx + (s16)((b >= 0 ? b : -b) + 0x58);
            }
            DrawSprite(0x4F, &p2, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            t = sMenuTime - 4;
            if (t > 8) {
                t = 8;
            }
            pos.vx = t * 32 - 0x180;
            pos.vy = -0x18;
            DrawSprite(0x25, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            t = sMenuTime - 6;
            if (t > 8) {
                t = 8;
            }
            pos.vx = t * 32 - 0x180;
            pos.vy = 8;
            DrawSprite(0x2A, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            p2.vx = pos.vx + 0x100;
            p2.vy = pos.vy;
            pos.vx += 0x90;
            DrawSprite(0x2D, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            DrawCansAndTime(&p2, gBestCans[D_80095998].a, gBestTimes[D_80095998].a);
            t = sMenuTime - 8;
            if (t > 8) {
                t = 8;
            }
            pos.vx = t * 32 - 0x180;
            pos.vy = 0x28;
            DrawSprite(0x2B, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            p2.vx = pos.vx + 0x100;
            p2.vy = pos.vy;
            pos.vx += 0x90;
            DrawSprite(0x2D, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            DrawCansAndTime(&p2, gBestCans[D_80095998].b, gBestTimes[D_80095998].b);
            t = sMenuTime - 10;
            if (t > 8) {
                t = 8;
            }
            pos.vx = t * 32 - 0x180;
            pos.vy = 0x48;
            DrawSprite(0x2C, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            if (sMenuTime == 20) {
                gSubStep = 1002;
            }
            break;
        case 1002:
            col.r = -1;
            col.g = col.b = col.cd = 0x80;
            norm.r = -1;
            norm.g = norm.b = norm.cd = 0x80;
            hi.r = -1;
            hi.g = hi.b = 0x80;
            hi.cd = 0;
            pos.vx = -0x50;
            pos.vy = -0x58;
            DrawSprite(0x20, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            pos.vx = -0x30;
            pos.vy = -0x38;
            DrawSprite(D_80095998 + 0x26, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            p2.vy = pos.vy;
            {
                s32 b = rsin(gFrameCount * 0xA000 / 360) / 512;
                p2.vx = pos.vx - (s16)((b >= 0 ? b : -b) + 8);
            }
            DrawSprite(0x4E, &p2, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            {
                s32 b = rsin(gFrameCount * 0xA000 / 360) / 512;
                p2.vx = pos.vx + (s16)((b >= 0 ? b : -b) + 0x58);
            }
            DrawSprite(0x4F, &p2, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            if (sMenuTime < 16) {
                sMenuTime++;
            }
            if (D_80095984 == 0) {
                col = hi;
            } else {
                col = norm;
            }
            t = sMenuTime;
            if (t > 8) {
                t = 8;
            }
            pos.vx = t * 32 - 0x180;
            pos.vy = -0x18;
            DrawSprite(0x25, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            if (D_80095984 == 1) {
                col = hi;
            } else {
                col = norm;
            }
            t = sMenuTime - 2;
            if (t > 8) {
                t = 8;
            }
            pos.vx = t * 32 - 0x180;
            pos.vy = 8;
            DrawSprite(0x2A, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            col.r = -1;
            col.g = col.b = col.cd = 0x80;
            p2.vx = pos.vx + 0x100;
            p2.vy = pos.vy;
            pos.vx += 0x90;
            DrawSprite(0x2D, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            DrawCansAndTime(&p2, gBestCans[D_80095998].a, gBestTimes[D_80095998].a);
            if (D_80095984 == 2) {
                col = hi;
            } else {
                col = norm;
            }
            t = sMenuTime - 4;
            if (t > 8) {
                t = 8;
            }
            pos.vx = t * 32 - 0x180;
            pos.vy = 0x28;
            DrawSprite(0x2B, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            col.r = -1;
            col.g = col.b = col.cd = 0x80;
            p2.vx = pos.vx + 0x100;
            p2.vy = pos.vy;
            pos.vx += 0x90;
            DrawSprite(0x2D, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            DrawCansAndTime(&p2, gBestCans[D_80095998].b, gBestTimes[D_80095998].b);
            if (D_80095984 == 3) {
                col = hi;
            } else {
                col = norm;
            }
            t = sMenuTime - 6;
            if (t > 8) {
                t = 8;
            }
            pos.vx = t * 32 - 0x180;
            pos.vy = 0x48;
            DrawSprite(0x2C, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            if (sMenuTime >= 16) {
                p2.vx = -0x8E;
                p2.vy = D_80095984 * 32 - 0x21;
                DrawMenuCursor(&p2, 2);
            }
            if (gPadTrig & PADLleft) {
                PlaySe(0xB);
                sMenuTime = 0;
                if (D_80095998 == 0) {
                    D_80095998 = 3;
                } else {
                    D_80095998--;
                }
            } else if (gPadTrig & PADLright) {
                PlaySe(0xB);
                sMenuTime = 0;
                if (D_80095998 == 3) {
                    D_80095998 = 0;
                } else {
                    D_80095998++;
                }
            } else if (gPadTrig & PADLup) {
                if (sMenuTime >= 16) {
                    PlaySe(0xB);
                    if (D_80095984 == 0) {
                        D_80095984 = 3;
                    } else {
                        D_80095984--;
                    }
                }
            } else if (gPadTrig & PADLdown) {
                if (sMenuTime >= 16) {
                    PlaySe(0xB);
                    if (D_80095984 == 3) {
                        D_80095984 = 0;
                    } else {
                        D_80095984++;
                    }
                }
            } else if (gPadTrig & PADRdown) {
                if (sMenuTime >= 16) {
                    PlaySe(0xE);
                    sMenuCancelled = 1;
                    sMenuTime = 0;
                    gSubStep = 1003;
                }
            } else if (gPadTrig & PADRright) {
                if (sMenuTime >= 16) {
                    PlaySe(0xC);
                    sMenuTime = 0;
                    gSubStep = 1003;
                }
            }
            break;
        case 1003:
            col.r = -1;
            col.g = col.b = col.cd = 0x80;
            t = ++sMenuTime;
            if (t < 0) {
                t = 0;
            }
            pos.vx = t * 32 - 0x50;
            pos.vy = -0x58;
            DrawSprite(0x20, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            t = sMenuTime - 2;
            if (t < 0) {
                t = 0;
            }
            pos.vx = t * 32 - 0x30;
            pos.vy = -0x38;
            DrawSprite(D_80095998 + 0x26, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            p2.vy = pos.vy;
            {
                s32 b = rsin(gFrameCount * 0xA000 / 360) / 512;
                p2.vx = pos.vx - (s16)((b >= 0 ? b : -b) + 8);
            }
            DrawSprite(0x4E, &p2, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            {
                s32 b = rsin(gFrameCount * 0xA000 / 360) / 512;
                p2.vx = pos.vx + (s16)((b >= 0 ? b : -b) + 0x58);
            }
            DrawSprite(0x4F, &p2, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            t = sMenuTime - 4;
            if (t < 0) {
                t = 0;
            }
            pos.vx = t * 32 - 0x80;
            pos.vy = -0x18;
            DrawSprite(0x25, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            t = sMenuTime - 6;
            if (t < 0) {
                t = 0;
            }
            pos.vx = t * 32 - 0x80;
            pos.vy = 8;
            DrawSprite(0x2A, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            p2.vx = pos.vx + 0x100;
            p2.vy = pos.vy;
            pos.vx += 0x90;
            DrawSprite(0x2D, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            DrawCansAndTime(&p2, gBestCans[D_80095998].a, gBestTimes[D_80095998].a);
            t = sMenuTime - 8;
            if (t < 0) {
                t = 0;
            }
            pos.vx = t * 32 - 0x80;
            pos.vy = 0x28;
            DrawSprite(0x2B, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            p2.vx = pos.vx + 0x100;
            p2.vy = pos.vy;
            pos.vx += 0x90;
            DrawSprite(0x2D, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            DrawCansAndTime(&p2, gBestCans[D_80095998].b, gBestTimes[D_80095998].b);
            t = sMenuTime - 10;
            if (t < 0) {
                t = 0;
            }
            pos.vx = t * 32 - 0x80;
            pos.vy = 0x48;
            DrawSprite(0x2C, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            if (sMenuTime == 24) {
                if (sMenuCancelled == 1) {
                    gSubStep = 100;
                } else {
                    gRetries = 99;
                    switch (D_80095998) {
                        case 0:
                            gStage = 0;
                            break;
                        case 1:
                            gStage = 3;
                            break;
                        case 2:
                            gStage = 9;
                            break;
                        case 3:
                            gStage = 12;
                            break;
                    }
                    switch (D_80095984) {
                        case 0:
                            gStageTableSet = 2;
                            gGameMode = 0x26;
                            break;
                        case 1:
                            gStageTableSet = 3;
                            BeginLoadScreenModel();
                            break;
                        case 2:
                            gStageTableSet = 3;
                            gStage++;
                            BeginLoadScreen();
                            break;
                        case 3:
                            gStageTableSet = 3;
                            gStage += 2;
                            BeginAct3LoadScreen();
                            break;
                    }
                }
            }
            break;
        case 2000:
            sMenuScene = 1;
            sMenuTime = 0;
            gSubStep = 2001;
            break;
        case 2001:
            col.r = -1;
            col.g = col.b = col.cd = 0x80;
            t = ++sMenuTime;
            if (t > 8) {
                t = 8;
            }
            pos.vx = t * 32 - 0x150;
            pos.vy = -0x58;
            DrawSprite(0x22, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            t = sMenuTime - 2;
            if (t > 8) {
                t = 8;
            }
            pos.vx = t * 32 - 0x180;
            pos.vy = -0x38;
            DrawSprite(0x3A, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            pos.vx += 0x98;
            /* MATCHING: one call per arm, which cross-jumping merges. */
            if (gStereo != 0) {
                DrawSprite(0x46, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            } else {
                DrawSprite(0x47, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            }
            if (PadInfoMode(0, 2, 0) != 0) {
                t = sMenuTime - 4;
                if (t > 8) {
                    t = 8;
                }
                pos.vx = t * 32 - 0x180;
                pos.vy = -0x18;
                DrawSprite(0x3B, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
                pos.vx += 0x98;
                if (gRumbleEnabled != 0) {
                    DrawSprite(0x48, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
                } else {
                    DrawSprite(0x49, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
                }
            }
            if (*(s8 *)&gUnlocks != 1) {
                t = sMenuTime - 6;
                if (t > 8) {
                    t = 8;
                }
                pos.vx = t * 32 - 0x180;
                pos.vy = 8;
                DrawSprite(0x3C, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
                pos.vx += 0x98;
                switch (gTmdSortMode) {
                    case 0:
                        DrawSprite(0x4A, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
                        break;
                    case 1:
                        DrawSprite(0x4B, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
                        break;
                    case 2:
                        DrawSprite(0x4C, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
                        break;
                    case 3:
                        DrawSprite(0x4D, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
                        break;
                }
            }
            t = sMenuTime - 8;
            if (t > 8) {
                t = 8;
            }
            pos.vx = t * 32 - 0x140;
            pos.vy = 0x28;
            DrawSprite(0x3D, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            t = sMenuTime - 10;
            if (t > 8) {
                t = 8;
            }
            pos.vx = t * 32 - 0x118;
            pos.vy = 0x48;
            DrawSprite(0x3E, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            if (sMenuTime == 20) {
                gSubStep = 2002;
            }
            break;
        case 2002:
            col.r = -1;
            col.g = col.b = col.cd = 0x80;
            norm.r = -1;
            norm.g = norm.b = norm.cd = 0x80;
            hi.r = -1;
            hi.g = hi.b = 0x80;
            hi.cd = 0;
            pos.vx = -0x50;
            pos.vy = -0x58;
            DrawSprite(0x22, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            t = 8;
            if (sOptionsCursor == 0) {
                col = hi;
            } else {
                col = norm;
            }
            pos.vx = t * 32 - 0x180;
            pos.vy = -0x38;
            DrawSprite(0x3A, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            pos.vx += 0x98;
            if (gStereo != 0) {
                DrawSprite(0x46, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            } else {
                DrawSprite(0x47, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            }
            if (sOptionsCursor == 0) {
                p2.vy = pos.vy;
                {
                    s32 b = rsin(gFrameCount * 0xA000 / 360) / 512;
                    p2.vx = pos.vx - (b >= 0 ? b : -b);
                }
                DrawSprite(0x4E, &p2, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
                {
                    s32 b = rsin(gFrameCount * 0xA000 / 360) / 512;
                    p2.vx = pos.vx + (s16)((b >= 0 ? b : -b) + 0x50);
                }
                DrawSprite(0x4F, &p2, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            }
            if ((sPadHasRumble = PadInfoMode(0, 2, 0) != 0)) {
                if (sOptionsCursor == 1) {
                    col = hi;
                } else {
                    col = norm;
                }
                pos.vx = t * 32 - 0x180;
                pos.vy = -0x18;
                DrawSprite(0x3B, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
                pos.vx += 0x98;
                if (gRumbleEnabled != 0) {
                    DrawSprite(0x48, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
                } else {
                    DrawSprite(0x49, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
                }
                if (sOptionsCursor == 1) {
                    p2.vy = pos.vy;
                    {
                        s32 b = rsin(gFrameCount * 0xA000 / 360) / 512;
                        p2.vx = pos.vx - (b >= 0 ? b : -b);
                    }
                    DrawSprite(0x4E, &p2, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
                    {
                        s32 b = rsin(gFrameCount * 0xA000 / 360) / 512;
                        p2.vx = pos.vx + (s16)((b >= 0 ? b : -b) + 0x50);
                    }
                    DrawSprite(0x4F, &p2, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
                }
            } else if (sOptionsCursor == 1) {
                sOptionsCursor = 0;
            }
            if (*(s8 *)&gUnlocks != 1) {
                if (sOptionsCursor == 2) {
                    col = hi;
                } else {
                    col = norm;
                }
                pos.vx = t * 32 - 0x180;
                pos.vy = 8;
                DrawSprite(0x3C, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
                pos.vx += 0x98;
                switch (gTmdSortMode) {
                    case 0:
                        DrawSprite(0x4A, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
                        break;
                    case 1:
                        DrawSprite(0x4B, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
                        break;
                    case 2:
                        DrawSprite(0x4C, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
                        break;
                    case 3:
                        DrawSprite(0x4D, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
                        break;
                }
                if (sOptionsCursor == 2) {
                    sMenuScene = 3;
                    p2.vy = pos.vy;
                    {
                        s32 b = rsin(gFrameCount * 0xA000 / 360) / 512;
                        p2.vx = pos.vx - (b >= 0 ? b : -b);
                    }
                    DrawSprite(0x4E, &p2, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
                    {
                        s32 b = rsin(gFrameCount * 0xA000 / 360) / 512;
                        p2.vx = pos.vx + (s16)((b >= 0 ? b : -b) + 0x50);
                    }
                    DrawSprite(0x4F, &p2, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
                } else {
                    sMenuScene = 1;
                }
            }
            if (sOptionsCursor == 3) {
                col = hi;
            } else {
                col = norm;
            }
            pos.vx = t * 32 - 0x140;
            pos.vy = 0x28;
            DrawSprite(0x3D, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            if (sOptionsCursor == 4) {
                col = hi;
            } else {
                col = norm;
            }
            pos.vx = t * 32 - 0x118;
            pos.vy = 0x48;
            DrawSprite(0x3E, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            d = sOptionsCursor;
            p2.vx = d >= 3 ? -0x40 : -0x8E;
            p2.vy = d * 32 - 0x41;
            /* MATCHING: keeps the arguments below the stores. */
            __asm__("");
            DrawMenuCursor(&p2, 2);
            if (gPadTrig & PADLup) {
                PlaySe(0xB);
                do {
                    i = 0;
                    if (sOptionsCursor == 0) {
                        sOptionsCursor = 4;
                    } else {
                        sOptionsCursor--;
                    }
                    if (sOptionsCursor == 1 && sPadHasRumble == 0) {
                        i = 1;
                    }
                    if (sOptionsCursor == 2 && *(s8 *)&gUnlocks == 1) {
                        i = 1;
                    }
                } while (i != 0);
            } else if (gPadTrig & PADLdown) {
                PlaySe(0xB);
                do {
                    i = 0;
                    if (sOptionsCursor == 4) {
                        sOptionsCursor = 0;
                    } else {
                        sOptionsCursor++;
                    }
                    if (sOptionsCursor == 1 && sPadHasRumble == 0) {
                        i = 1;
                    }
                    if (sOptionsCursor == 2 && *(s8 *)&gUnlocks == 1) {
                        i = 1;
                    }
                } while (i != 0);
            } else if (gPadTrig & PADLleft) {
                PlaySe(0xB);
                switch (sOptionsCursor) {
                    case 0:
                        gStereo ^= 1;
                        if (gStereo != 0) {
                            SsSetStereo();
                        } else {
                            SsSetMono();
                        }
                        break;
                    case 1:
                        gRumbleEnabled ^= 1;
                        if (gRumbleEnabled != 0) {
                            gRumbleTime = 15;
                            gRumbleSmall[0] = 0xFF;
                            gRumbleSmall[1] = 0xFF;
                        }
                        break;
                    case 2:
                        do {
                            if (gTmdSortMode == 0) {
                                gTmdSortMode = 3;
                            } else {
                                gTmdSortMode--;
                            }
                        } while (!((*(s8 *)&gUnlocks >> gTmdSortMode) & 1));
                        switch (gTmdSortMode) {
                            case 0:
                            case 2:
                            case 3:
                                LoadTimList(
                                    (unsigned long *)(*(s32 *)(RAM_STAGE_LOAD + 4) + RAM_STAGE_LOAD));
                                break;
                            case 1:
                                LoadTimList(
                                    (unsigned long *)(*(s32 *)(RAM_STAGE_LOAD + 8) + RAM_STAGE_LOAD));
                                break;
                        }
                        break;
                }
            } else if (gPadTrig & PADLright) {
                PlaySe(0xB);
                switch (sOptionsCursor) {
                    case 0:
                        gStereo ^= 1;
                        if (gStereo != 0) {
                            SsSetStereo();
                        } else {
                            SsSetMono();
                        }
                        break;
                    case 1:
                        gRumbleEnabled ^= 1;
                        if (gRumbleEnabled != 0) {
                            gRumbleTime = 15;
                            gRumbleSmall[0] = 0xFF;
                            gRumbleSmall[1] = 0xFF;
                        }
                        break;
                    case 2:
                        do {
                            if (gTmdSortMode == 3) {
                                gTmdSortMode = 0;
                            } else {
                                gTmdSortMode++;
                            }
                        } while (!((*(s8 *)&gUnlocks >> gTmdSortMode) & 1));
                        switch (gTmdSortMode) {
                            case 0:
                            case 2:
                            case 3:
                                LoadTimList(
                                    (unsigned long *)(*(s32 *)(RAM_STAGE_LOAD + 4) + RAM_STAGE_LOAD));
                                break;
                            case 1:
                                LoadTimList(
                                    (unsigned long *)(*(s32 *)(RAM_STAGE_LOAD + 8) + RAM_STAGE_LOAD));
                                break;
                        }
                        break;
                }
            } else if (gPadTrig & PADRdown) {
                PlaySe(0xE);
                sMenuTime = 0;
                if (sOptionsCursor != 4) {
                    sOptionsCursor = 4;
                }
                gSubStep = 2003;
            } else if (gPadTrig & PADRright) {
                switch (sOptionsCursor) {
                    case 3:
                        PlaySe(0xC);
                        sMenuTime = 0;
                        gSubStep = 2003;
                        break;
                    case 4:
                        PlaySe(0xE);
                        sMenuTime = 0;
                        gSubStep = 2003;
                        break;
                }
            }
            break;
        case 2003:
            col.r = -1;
            col.g = col.b = col.cd = 0x80;
            t = ++sMenuTime;
            if (t < 0) {
                t = 0;
            }
            pos.vx = t * 32 - 0x50;
            pos.vy = -0x58;
            DrawSprite(0x22, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            t = sMenuTime - 2;
            if (t < 0) {
                t = 0;
            }
            pos.vx = t * 32 - 0x80;
            pos.vy = -0x38;
            DrawSprite(0x3A, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            pos.vx += 0x98;
            if (gStereo != 0) {
                DrawSprite(0x46, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            } else {
                DrawSprite(0x47, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            }
            if (PadInfoMode(0, 2, 0) != 0) {
                t = sMenuTime - 4;
                if (t < 0) {
                    t = 0;
                }
                pos.vx = t * 32 - 0x80;
                pos.vy = -0x18;
                DrawSprite(0x3B, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
                pos.vx += 0x98;
                if (gRumbleEnabled != 0) {
                    DrawSprite(0x48, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
                } else {
                    DrawSprite(0x49, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
                }
            }
            if (*(s8 *)&gUnlocks != 1) {
                t = sMenuTime - 6;
                if (t < 0) {
                    t = 0;
                }
                pos.vx = t * 32 - 0x80;
                pos.vy = 8;
                DrawSprite(0x3C, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
                pos.vx += 0x98;
                switch (gTmdSortMode) {
                    case 0:
                        DrawSprite(0x4A, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
                        break;
                    case 1:
                        DrawSprite(0x4B, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
                        break;
                    case 2:
                        DrawSprite(0x4C, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
                        break;
                    case 3:
                        DrawSprite(0x4D, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
                        break;
                }
            }
            t = sMenuTime - 8;
            if (t < 0) {
                t = 0;
            }
            pos.vx = t * 32 - 0x40;
            pos.vy = 0x28;
            DrawSprite(0x3D, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            t = sMenuTime - 10;
            if (t < 0) {
                t = 0;
            }
            pos.vx = t * 32 - 0x18;
            pos.vy = 0x48;
            DrawSprite(0x3E, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            if (sMenuTime == 24) {
                if (sOptionsCursor == 4) {
                    gSubStep = 100;
                } else {
                    gSubStep = 3000;
                }
            }
            break;
        case 3000:
            sMenuTime = 0;
            gSubStep++;
            break;
        case 3001:
            col.r = -1;
            col.g = col.b = col.cd = 0x80;
            t = ++sMenuTime;
            if (t > 8) {
                t = 8;
            }
            pos.vx = t * 32 - 0x140;
            pos.vy = -0x58;
            DrawSprite(0x3D, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            t = sMenuTime - 2;
            if (t > 8) {
                t = 8;
            }
            pos.vx = t * 32 - 0x128;
            pos.vy = -0x38;
            DrawSprite(gButtonConfig + 0x42, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            t = sMenuTime - 4;
            if (t > 8) {
                t = 8;
            }
            pos.vx = t * 32 - 0x160;
            pos.vy = -0x18;
            DrawSprite(0x3F, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            pos.vx += 0xB0;
            DrawSprite(sButtonIcons[gButtonConfig][0] + 0x69, &pos, NULL, 0, &gSubOt[gActiveBuff]);
            t = sMenuTime - 6;
            if (t > 8) {
                t = 8;
            }
            pos.vx = t * 32 - 0x160;
            pos.vy = 8;
            DrawSprite(0x40, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            pos.vx += 0xB0;
            DrawSprite(sButtonIcons[gButtonConfig][1] + 0x69, &pos, NULL, 0, &gSubOt[gActiveBuff]);
            if (sMenuTime == 18) {
                gSubStep = 3002;
            }
            break;
        case 3002:
            col.r = -1;
            col.g = col.b = col.cd = 0x80;
            t = 8;
            pos.vx = t * 32 - 0x140;
            pos.vy = -0x58;
            DrawSprite(0x3D, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            pos.vx = t * 32 - 0x128;
            pos.vy = -0x38;
            DrawSprite(gButtonConfig + 0x42, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            p2.vy = pos.vy;
            {
                s32 b = rsin(gFrameCount * 0xA000 / 360) / 512;
                p2.vx = pos.vx - (s16)((b >= 0 ? b : -b) + 8);
            }
            DrawSprite(0x4E, &p2, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            {
                s32 b = rsin(gFrameCount * 0xA000 / 360) / 512;
                p2.vx = pos.vx + (s16)((b >= 0 ? b : -b) + 0x48);
            }
            DrawSprite(0x4F, &p2, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            pos.vx = t * 32 - 0x160;
            pos.vy = -0x18;
            DrawSprite(0x3F, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            pos.vx += 0xB0;
            DrawSprite(sButtonIcons[gButtonConfig][0] + 0x69, &pos, NULL, 0, &gSubOt[gActiveBuff]);
            pos.vx = t * 32 - 0x160;
            pos.vy = t;
            DrawSprite(0x40, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            pos.vx += 0xB0;
            DrawSprite(sButtonIcons[gButtonConfig][1] + 0x69, &pos, NULL, 0, &gSubOt[gActiveBuff]);
            if (gPadTrig & PADLleft) {
                PlaySe(0xB);
                if (gButtonConfig == 0) {
                    gButtonConfig = 3;
                } else {
                    gButtonConfig--;
                }
            } else if (gPadTrig & PADLright) {
                PlaySe(0xB);
                if (gButtonConfig == 3) {
                    gButtonConfig = 0;
                } else {
                    gButtonConfig++;
                }
            } else if (gPadTrig & PADRright) {
                PlaySe(0xE);
                sMenuTime = 0;
                gSubStep = 3003;
            } else if (gPadTrig & PADRdown) {
                PlaySe(0xE);
                sMenuTime = 0;
                gSubStep = 3003;
            }
            break;
        case 3003:
            col.r = -1;
            col.g = col.b = col.cd = 0x80;
            t = ++sMenuTime;
            if (t < 0) {
                t = 0;
            }
            pos.vx = t * 32 - 0x40;
            pos.vy = -0x58;
            DrawSprite(0x3D, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            t = sMenuTime - 2;
            if (t < 0) {
                t = 0;
            }
            pos.vx = t * 32 - 0x28;
            pos.vy = -0x38;
            DrawSprite(gButtonConfig + 0x42, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            t = sMenuTime - 4;
            if (t < 0) {
                t = 0;
            }
            pos.vx = t * 32 - 0x60;
            pos.vy = -0x18;
            DrawSprite(0x3F, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            pos.vx += 0xB0;
            DrawSprite(sButtonIcons[gButtonConfig][0] + 0x69, &pos, NULL, 0, &gSubOt[gActiveBuff]);
            t = sMenuTime - 6;
            if (t < 0) {
                t = 0;
            }
            pos.vx = t * 32 - 0x60;
            pos.vy = 8;
            DrawSprite(0x40, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            pos.vx += 0xB0;
            DrawSprite(sButtonIcons[gButtonConfig][1] + 0x69, &pos, NULL, 0, &gSubOt[gActiveBuff]);
            if (sMenuTime == 22) {
                gSubStep = 2000;
            }
            break;
        case 5000:
            col.r = -1;
            col.g = col.b = col.cd = 0x80;
            for (i = 0; (s32)i < sMenuItemCount; i++) {
                t = 8;
                pos.vx = t * 32 - 0x150;
                pos.vy = (s16)(i * 24 - 12) - sMenuItemCount * 12;
                DrawSprite(sMenuItems[i] + 0x1E, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            }
            p2.vx = -0x88;
            p2.vy = 0x4F;
            DrawMenuCursor(&p2, 3);
            pos.vx = -0x80;
            pos.vy = 0x58;
            switch (sCardStep) {
                case 0:
                    DrawSprite(0x5A, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
                    pos.vx = 0x41;
                    pos.vy = 0x57;
                    DrawSprite(0x69, &pos, NULL, 0, &gSubOt[gActiveBuff]);
                    pos.vx = 0x6F;
                    pos.vy = 0x57;
                    DrawSprite(0x6A, &pos, NULL, 0, &gSubOt[gActiveBuff]);
                    if (gPadTrig & PADRdown) {
                        PlaySe(0xE);
                        gSubStep = 102;
                    } else if (gPadTrig & PADRright) {
                        PlaySe(0xC);
                        sCardStep = 10;
                    }
                    break;
                case 5:
                    DrawSprite(0x57, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
                    MemCardExist(0);
                    sCardStep = 6;
                    break;
                case 6:
                    DrawSprite(0x57, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
                    if (MemCardSync(1, NULL, &sCardResult) == 1) {
                        switch (sCardResult) {
                            case 0:
                                sCardStep = 12;
                                break;
                            case 1:
                                sCardMessage = 0x59;
                                sCardMessageTime = 0;
                                sCardStep = 1000;
                                break;
                            case 2:
                                sCardMessage = 0x68;
                                sCardMessageTime = 0;
                                sCardStep = 1000;
                                break;
                            case 3:
                                sCardStep = 10;
                                break;
                        }
                    }
                    break;
                case 10:
                    DrawSprite(0x57, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
                    MemCardAccept(0);
                    sCardStep = 11;
                    break;
                case 11:
                    DrawSprite(0x57, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
                    if (MemCardSync(1, NULL, &sCardResult) == 1) {
                        switch (sCardResult) {
                            case 0:
                            case 3:
                                sCardStep = 12;
                                break;
                            case 1:
                                sCardMessage = 0x59;
                                sCardMessageTime = 0;
                                sCardStep = 1000;
                                break;
                            case 2:
                                sCardMessage = 0x68;
                                sCardMessageTime = 0;
                                sCardStep = 1000;
                                break;
                            case 4:
                                sCardMessage = 0x58;
                                sCardMessageTime = 0;
                                sCardStep = 1000;
                                break;
                        }
                    }
                    break;
                case 12:
                    MemCardReadFile(0, (char *)sSaveFileName, (unsigned long *)RAM_WORK_A, 0x200, 0x80);
                    sCardStep = 13;
                    break;
                case 13:
                    DrawSprite(0x5D, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
                    if (MemCardSync(1, NULL, &sCardResult) == 1) {
                        switch (sCardResult) {
                            case 0:
                                sCardStep = 20;
                                break;
                            case 1:
                                sCardMessage = 0x59;
                                sCardMessageTime = 0;
                                sCardStep = 1000;
                                break;
                            case 2:
                                sCardMessage = 0x68;
                                sCardMessageTime = 0;
                                sCardStep = 1000;
                                break;
                            case 3:
                                sCardMessage = 0x68;
                                sCardMessageTime = 0;
                                sCardStep = 1000;
                                break;
                            case 4:
                                break;
                            case 5:
                                sCardMessage = 0x58;
                                sCardMessageTime = 0;
                                sCardStep = 1000;
                                break;
                        }
                    }
                    break;
                case 20:
                    sSave = (SaveData *)RAM_WORK_A;
                    sSaveChecksum = sSave->checksum;
                    sSave->checksum = 0;
                    sSaveSum = 0;
                    for (i = 0; i < 0x5C; i++) {
                        sSaveSum += ((u8 *)sSave)[i];
                    }
                    sSaveSum = ~sSaveSum;
                    if (sSaveSum != sSaveChecksum) {
                        sCardMessage = 0x67;
                        sCardMessageTime = 0;
                        sCardStep = 1000;
                        break;
                    }
                    gBestTimes[0].a = sSave->times[0].a;
                    gBestTimes[1].a = sSave->times[1].a;
                    gBestTimes[2].a = sSave->times[2].a;
                    gBestTimes[3].a = sSave->times[3].a;
                    gBestTimes[0].b = sSave->times[0].b;
                    gBestTimes[1].b = sSave->times[1].b;
                    gBestTimes[2].b = sSave->times[2].b;
                    gBestTimes[3].b = sSave->times[3].b;
                    gBestCans[0].a = sSave->cans[0].a;
                    gBestCans[1].a = sSave->cans[1].a;
                    gBestCans[2].a = sSave->cans[2].a;
                    gBestCans[3].a = sSave->cans[3].a;
                    gBestCans[0].b = sSave->cans[0].b;
                    gBestCans[1].b = sSave->cans[1].b;
                    gBestCans[2].b = sSave->cans[2].b;
                    gBestCans[3].b = sSave->cans[3].b;
                    D_80095978.a = sSave->continueCans.a;
                    D_80095978.b = sSave->continueCans.b;
                    gStereo = sSave->stereo;
                    gRumbleEnabled = sSave->rumble;
                    gTmdSortMode = sSave->sortMode;
                    gButtonConfig = sSave->buttonConfig;
                    D_800957A0 = sSave->continueStage;
                    gGameCleared = sSave->gameCleared;
                    gAllTargetsBeaten = sSave->allTargets;
                    gUnlocks = sSave->unlocks;
                    if (gStereo != 0) {
                        SsSetStereo();
                    } else {
                        SsSetMono();
                    }
                    switch (gTmdSortMode) {
                        case 0:
                        case 2:
                        case 3:
                            LoadTimList((unsigned long *)(*(s32 *)(RAM_STAGE_LOAD + 4) + RAM_STAGE_LOAD));
                            break;
                        case 1:
                            LoadTimList((unsigned long *)(*(s32 *)(RAM_STAGE_LOAD + 8) + RAM_STAGE_LOAD));
                            break;
                    }
                    sCardMessage = 0x66;
                    sCardMessageTime = 0;
                    sCardStep = 2000;
                    break;
                case 1000:
                    col.g = 0x80;
                    col.b = col.cd = 0;
                    DrawSprite(sCardMessage, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
                    if (++sCardMessageTime == 30) {
                        gSubStep = 102;
                    }
                    break;
                case 2000:
                    DrawSprite(sCardMessage, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
                    if (++sCardMessageTime == 30) {
                        gSubStep = 100;
                    }
                    break;
            }
            break;
        case 6000:
            col.r = -1;
            col.g = col.b = col.cd = 0x80;
            for (i = 0; (s32)i < sMenuItemCount; i++) {
                t = 8;
                pos.vx = t * 32 - 0x150;
                pos.vy = (s16)(i * 24 - 12) - sMenuItemCount * 12;
                DrawSprite(sMenuItems[i] + 0x1E, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
            }
            p2.vx = -0x88;
            p2.vy = 0x4F;
            DrawMenuCursor(&p2, 3);
            pos.vx = -0x80;
            pos.vy = 0x58;
            switch (sCardStep) {
                case 0:
                    DrawSprite(0x5B, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
                    pos.vx = 0x41;
                    pos.vy = 0x57;
                    DrawSprite(0x69, &pos, NULL, 0, &gSubOt[gActiveBuff]);
                    pos.vx = 0x6F;
                    pos.vy = 0x57;
                    DrawSprite(0x6A, &pos, NULL, 0, &gSubOt[gActiveBuff]);
                    if (gPadTrig & PADRdown) {
                        PlaySe(0xE);
                        gSubStep = 102;
                    } else if (gPadTrig & PADRright) {
                        PlaySe(0xC);
                        sCardMessageTime = 0;
                        sCardStep = 10;
                    }
                    break;
                case 5:
                    DrawSprite(0x57, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
                    if (++sCardMessageTime == 4) {
                        MemCardExist(0);
                        sCardStep = 6;
                    }
                    break;
                case 6:
                    DrawSprite(0x57, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
                    if (MemCardSync(1, NULL, &sCardResult) == 1) {
                        switch (sCardResult) {
                            case 0:
                                switch (MemCardCreateFile(0, (char *)sSaveFileName, 1)) {
                                    case 0:
                                        sCardStep = 12;
                                        break;
                                    case 6:
                                        sCardStep = 13;
                                        break;
                                    case 1:
                                        sCardMessage = 0x59;
                                        sCardMessageTime = 0;
                                        sCardStep = 1000;
                                        break;
                                    case 2:
                                        sCardMessage = 0x68;
                                        sCardMessageTime = 0;
                                        sCardStep = 1000;
                                        break;
                                    case 7:
                                        sCardMessage = 0x65;
                                        sCardMessageTime = 0;
                                        sCardStep = 1000;
                                        break;
                                    case 3:
                                    case 5:
                                        break;
                                    case 4:
                                        sCardMessageTime = 0;
                                        sCardStep = 500;
                                        break;
                                }
                                break;
                            case 3:
                                sCardStep = 10;
                                break;
                            case 1:
                                sCardMessage = 0x59;
                                sCardMessageTime = 0;
                                sCardStep = 1000;
                                break;
                            case 2:
                                sCardMessage = 0x68;
                                sCardMessageTime = 0;
                                sCardStep = 1000;
                                break;
                        }
                    }
                    break;
                case 10:
                    DrawSprite(0x57, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
                    MemCardAccept(0);
                    sCardStep = 11;
                    break;
                case 11:
                    DrawSprite(0x57, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
                    if (MemCardSync(1, NULL, &sCardResult) == 1) {
                        switch (sCardResult) {
                            case 0:
                            case 3:
                                switch (MemCardCreateFile(0, (char *)sSaveFileName, 1)) {
                                    case 0:
                                        sCardStep = 12;
                                        break;
                                    case 6:
                                        sCardStep = 13;
                                        break;
                                    case 1:
                                        sCardMessage = 0x59;
                                        sCardMessageTime = 0;
                                        sCardStep = 1000;
                                        break;
                                    case 2:
                                        sCardMessage = 0x68;
                                        sCardMessageTime = 0;
                                        sCardStep = 1000;
                                        break;
                                    case 7:
                                        sCardMessage = 0x65;
                                        sCardMessageTime = 0;
                                        sCardStep = 1000;
                                        break;
                                    case 3:
                                    case 5:
                                        break;
                                    case 4:
                                        sCardMessageTime = 0;
                                        sCardStep = 500;
                                        break;
                                }
                                break;
                            case 1:
                                sCardMessage = 0x59;
                                sCardMessageTime = 0;
                                sCardStep = 1000;
                                break;
                            case 2:
                                sCardMessage = 0x68;
                                sCardMessageTime = 0;
                                sCardStep = 1000;
                                break;
                            case 4:
                                sCardMessageTime = 0;
                                sCardStep = 500;
                                break;
                        }
                    }
                    break;
                case 12:
                    for (i = 0; (s32)i < 0x200; i++) {
                        ((u8 *)RAM_WORK_A)[i] = ((u8 *)&sCardFileHeader)[i];
                    }
                    sSave = (SaveData *)(RAM_WORK_A + 0x200);
                    sSave->times[0].a = gBestTimes[0].a;
                    sSave->times[1].a = gBestTimes[1].a;
                    sSave->times[2].a = gBestTimes[2].a;
                    sSave->times[3].a = gBestTimes[3].a;
                    sSave->times[0].b = gBestTimes[0].b;
                    sSave->times[1].b = gBestTimes[1].b;
                    sSave->times[2].b = gBestTimes[2].b;
                    sSave->times[3].b = gBestTimes[3].b;
                    sSave->cans[0].a = gBestCans[0].a;
                    sSave->cans[1].a = gBestCans[1].a;
                    sSave->cans[2].a = gBestCans[2].a;
                    sSave->cans[3].a = gBestCans[3].a;
                    sSave->cans[0].b = gBestCans[0].b;
                    sSave->cans[1].b = gBestCans[1].b;
                    sSave->cans[2].b = gBestCans[2].b;
                    sSave->cans[3].b = gBestCans[3].b;
                    sSave->continueCans.a = D_80095978.a;
                    sSave->continueCans.b = D_80095978.b;
                    sSave->stereo = gStereo;
                    sSave->rumble = gRumbleEnabled;
                    sSave->sortMode = gTmdSortMode;
                    sSave->buttonConfig = gButtonConfig;
                    sSave->continueStage = D_800957A0;
                    sSave->gameCleared = gGameCleared;
                    sSave->allTargets = gAllTargetsBeaten;
                    sSave->unlocks = gUnlocks;
                    sSave->checksum = 0;
                    sSaveSum = 0;
                    for (i = 0; i < 0x5C; i++) {
                        sSaveSum += ((u8 *)sSave)[i];
                    }
                    sSaveSum = ~sSaveSum;
                    sSave->checksum = sSaveSum;
                    MemCardWriteFile(0, (char *)sSaveFileName, (unsigned long *)RAM_WORK_A, 0, 0x280);
                    sCardStep = 15;
                    break;
                case 13:
                    DrawSprite(0x5C, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
                    pos.vx = 0x41;
                    pos.vy = 0x57;
                    DrawSprite(0x69, &pos, NULL, 0, &gSubOt[gActiveBuff]);
                    pos.vx = 0x6F;
                    pos.vy = 0x57;
                    DrawSprite(0x6A, &pos, NULL, 0, &gSubOt[gActiveBuff]);
                    if (gPadTrig & PADRdown) {
                        PlaySe(0xE);
                        gSubStep = 102;
                    } else if (gPadTrig & PADRright) {
                        PlaySe(0xC);
                        sSave = (SaveData *)RAM_WORK_A;
                        sSave->times[0].a = gBestTimes[0].a;
                        sSave->times[1].a = gBestTimes[1].a;
                        sSave->times[2].a = gBestTimes[2].a;
                        sSave->times[3].a = gBestTimes[3].a;
                        sSave->times[0].b = gBestTimes[0].b;
                        sSave->times[1].b = gBestTimes[1].b;
                        sSave->times[2].b = gBestTimes[2].b;
                        sSave->times[3].b = gBestTimes[3].b;
                        sSave->cans[0].a = gBestCans[0].a;
                        sSave->cans[1].a = gBestCans[1].a;
                        sSave->cans[2].a = gBestCans[2].a;
                        sSave->cans[3].a = gBestCans[3].a;
                        sSave->cans[0].b = gBestCans[0].b;
                        sSave->cans[1].b = gBestCans[1].b;
                        sSave->cans[2].b = gBestCans[2].b;
                        sSave->cans[3].b = gBestCans[3].b;
                        sSave->continueCans.a = D_80095978.a;
                        sSave->continueCans.b = D_80095978.b;
                        sSave->stereo = gStereo;
                        sSave->rumble = gRumbleEnabled;
                        sSave->sortMode = gTmdSortMode;
                        sSave->buttonConfig = gButtonConfig;
                        sSave->continueStage = D_800957A0;
                        sSave->gameCleared = gGameCleared;
                        sSave->allTargets = gAllTargetsBeaten;
                        sSave->unlocks = gUnlocks;
                        sSave->checksum = 0;
                        sSaveSum = 0;
                        for (i = 0; i < 0x5C; i++) {
                            sSaveSum += ((u8 *)sSave)[i];
                        }
                        sSaveSum = ~sSaveSum;
                        sSave->checksum = sSaveSum;
                        MemCardWriteFile(0, (char *)sSaveFileName, (unsigned long *)RAM_WORK_A,
                                         0x200, 0x80);
                        sCardStep = 15;
                    }
                    break;
                case 15:
                    DrawSprite(0x5E, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
                    if (MemCardSync(1, NULL, &sCardResult) == 1) {
                        switch (sCardResult) {
                            case 0:
                                sCardMessage = 0x66;
                                sCardMessageTime = 0;
                                sCardStep = 2000;
                                break;
                            case 1:
                                sCardMessage = 0x59;
                                sCardMessageTime = 0;
                                sCardStep = 1000;
                                break;
                            case 2:
                                sCardMessage = 0x68;
                                sCardMessageTime = 0;
                                sCardStep = 1000;
                                break;
                            case 3:
                                sCardMessage = 0x68;
                                sCardMessageTime = 0;
                                sCardStep = 1000;
                                break;
                            case 4:
                                break;
                            case 5:
                                sCardMessage = 0x58;
                                sCardMessageTime = 0;
                                sCardStep = 1000;
                                break;
                        }
                    }
                    break;
                case 500:
                    col.g = 0x80;
                    col.b = col.cd = 0;
                    DrawSprite(0x61, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
                    if (++sCardMessageTime == 30) {
                        sCardStep = 501;
                    }
                    break;
                case 501:
                    DrawSprite(0x62, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
                    pos.vx = 0x41;
                    pos.vy = 0x57;
                    DrawSprite(0x69, &pos, NULL, 0, &gSubOt[gActiveBuff]);
                    pos.vx = 0x6F;
                    pos.vy = 0x57;
                    DrawSprite(0x6A, &pos, NULL, 0, &gSubOt[gActiveBuff]);
                    if (gPadTrig & PADRdown) {
                        PlaySe(0xE);
                        gSubStep = 102;
                    } else if (gPadTrig & PADRright) {
                        PlaySe(0xC);
                        sCardMessageTime = 0;
                        sCardStep = 502;
                    }
                    break;
                case 502:
                    DrawSprite(0x63, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
                    if (++sCardMessageTime == 8) {
                        sCardStep = 503;
                    }
                    break;
                case 503:
                    DrawSprite(0x63, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
                    switch (MemCardFormat(0)) {
                        case 0:
                            sCardMessageTime = 0;
                            sCardStep = 10;
                            break;
                        case 1:
                            sCardMessage = 0x59;
                            sCardMessageTime = 0;
                            sCardStep = 1000;
                            break;
                        case 2:
                            sCardMessage = 0x68;
                            sCardMessageTime = 0;
                            sCardStep = 1000;
                            break;
                    }
                    break;
                case 504:
                    DrawSprite(0x66, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
                    sCardMessageTime++;
                    break;
                case 1000:
                    col.g = 0x80;
                    col.b = col.cd = 0;
                    DrawSprite(sCardMessage, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
                    if (++sCardMessageTime == 30) {
                        gSubStep = 102;
                    }
                    break;
                case 2000:
                    DrawSprite(sCardMessage, &pos, (CVECTOR *)&col, 0, &gSubOt[gActiveBuff]);
                    if (++sCardMessageTime == 30) {
                        gSubStep = 100;
                    }
                    break;
            }
            break;
    }
    sMenuModelRot.vy += 11;
    OrbitMenuCamera(&sMenuModelRot);
    StepStepper(&gPlayer);
    switch (sMenuScene) {
        case 0:
            DrawMenuObject(2);
            break;
        case 1:
            DrawMenuObject(1);
            break;
        case 2:
            DrawMenuObject(0);
            break;
        case 3:
            DrawStepper(&gPlayer);
            break;
    }
}

void StepCardLoad(void);
void StepMainMenu(void);
void StepTitle(void);
s32 StepGameOver(void);

void StepMenus(void) {
    switch (gModeStep) {
        case 0:
            gSubStep = 0;
            gCdLoadLoc[0] = (s32)&sFiles3OvlLoc[2];
            gCdLoadDest[0] = RAM_STAGE_LOAD;
            gCdLoadCount = 1;
            gCdLoadState = CD_LOAD_SEEK;
            gModeStep = 1;
            gCdLoadSectors[0] = sFiles3OvlSectors[2];
            break;
        case 1:
            if (gCdLoadState == CD_LOAD_DONE) {
                gPlayer.rideMode = 0;
                gPlayer.flattened = 0;
                LoadTimList((unsigned long *)(*(s32 *)RAM_STAGE_LOAD + RAM_STAGE_LOAD));
                gSubStep = 50;
                if (gStageTableSet == -1) {
                    LoadTimList((unsigned long *)(*(s32 *)(RAM_STAGE_LOAD + 4) + RAM_STAGE_LOAD));
                    gModeStep = 2;
                } else {
                    gModeStep = 40;
                }
            }
            break;
        case 2:
            DrawFrozenFrame();
            SetTmdSortMode(gTmdSortMode);
            StepOpening();
            break;
        case 5:
            PlayMovie("\\MOVIE0.STR;1", NULL, 0x31F, 0xF0, 1);
            gModeStep = 9;
            gSubStep = 50;
            break;
        case 6:
            PlayMovie("\\OP2_OK.STR;1", NULL, 0xAE, 0xF0, 1);
            gModeStep = 9;
            gSubStep = 50;
            break;
        case 7:
            PlayMovie("\\OP2_15.STR;1", NULL, 0x15E, 0xF0, 1);
            gModeStep = 9;
            gSubStep = 50;
            break;
        case 9:
            LoadTimList((unsigned long *)(*(s32 *)RAM_STAGE_LOAD + RAM_STAGE_LOAD));
            gModeStep = 10;
            gSubStep = 50;
            break;
        case 10:
            DrawFrozenFrame();
            SetTmdSortMode(gTmdSortMode);
            StepTitle();
            break;
        case 30:
            DrawFrozenFrame();
            DrawScreenWipe();
            StepCardLoad();
            break;
        case 40:
            DrawFrozenFrame();
            SetTmdSortMode(gTmdSortMode);
            StepMainMenu();
            break;
        case 50:
            gSubStep = 0;
            gCdLoadLoc[0] = (s32)&sFiles3OvlLoc[3];
            gCdLoadDest[0] = RAM_STAGE_LOAD;
            gCdLoadCount = 1;
            gCdLoadState = CD_LOAD_SEEK;
            gModeStep = 51;
            gCdLoadSectors[0] = sFiles3OvlSectors[3];
            break;
        case 51:
            if (gCdLoadState == CD_LOAD_DONE) {
                LoadTimList((unsigned long *)(*(s32 *)RAM_STAGE_LOAD + RAM_STAGE_LOAD));
                if (gTmdSortMode == 1) {
                    LoadTimList((unsigned long *)(*(s32 *)(RAM_STAGE_LOAD + 0xC) + RAM_STAGE_LOAD));
                } else {
                    LoadTimList((unsigned long *)(*(s32 *)(RAM_STAGE_LOAD + 8) + RAM_STAGE_LOAD));
                }
                gSubStep = 0;
                gModeStep = 60;
            }
            break;
        case 60:
            StepGameOver();
            break;
    }
}

inline void DrawTitlePlayerShadow(void) {
    GsCOORDINATE2 c;
    MATRIX m;
    SVECTOR size;
    SVECTOR unused;
    SColor color;

    GsInitCoordinate2(WORLD, &c);
    c.coord.t[1] = 0;
    c.coord.t[0] = gPlayer.x;
    c.coord.t[2] = gPlayer.z;
    GsGetLs(&c, &m);
    GsSetLsMatrix(&m);
    size.vx = 100;
    size.vy = size.vx / 2;
    color.r = BLEND_SUB;
    color.g = color.b = color.cd = 0x40;
    DrawBillboard(0x12D, &size, (CVECTOR *)&color, 2, &gSubOt[gActiveBuff]);
}

inline void DrawTitleGroundShadow(void) {
    GsCOORDINATE2 c;
    MATRIX m;
    SVECTOR size;
    SVECTOR unused;
    SColor color;

    GsInitCoordinate2(WORLD, &c);
    c.coord.t[0] = 0x2B5C;
    c.coord.t[1] = 0;
    c.coord.t[2] = 0;
    GsGetLs(&c, &m);
    GsSetLsMatrix(&m);
    size.vx = 800;
    size.vy = 50;
    color.r = BLEND_SUB;
    color.g = color.b = color.cd = 0x60;
    DrawBillboard(0x12D, &size, (CVECTOR *)&color, 2, &gSubOt[gActiveBuff]);
}

void StepTitle(void) {
    SVECTOR pos;
    SColor color;
    MATRIX m;
    POLY_F4 *p;
    DR_TPAGE *t;
    s32 *dir;

    switch (gSubStep) {
        case 50:
            DefineSprite(0x1E, 1, 0xFF, 0x60, 0x17, 0, 0, 0, 0x1E1);
            DefineSprite(0x1F, 0, 0xFF, 0x10, 0x17, 0, 0x80, 0x40, 0x1E0);
            DefineSprite(0x20, 0, 0xFF, 0x10, 0x17, 0, 0x90, 0x40, 0x1E0);
            DefineSprite(0x21, 0, 0xFF, 0x20, 0x17, 0, 0xA0, 0x40, 0x1E0);
            DefineSprite(0x22, 0, 0xFF, 0x20, 0x17, 0, 0xC0, 0x40, 0x1E0);
            for (gLoopI = 0; gLoopI < 2; gLoopI++) {
                /* MATCHING: the entry assigned inside the store keeps the +1 off
                 * the directory's address. */
                gAnimations[gLoopI + 51] =
                    (dir = (s32 *)(((s32 *)(RAM_STAGE_LOAD + 0x10))[gLoopI] + RAM_STAGE_LOAD)) + 1;
                gPlayer.motionLen[gLoopI + 51] = *gAnimations[gLoopI + 51];
                gAnimations[gLoopI + 51]++;
                gPlayer.motionStart[gLoopI + 51] = gAnimations[gLoopI + 51][1];
            }
            GsSetProjection(250);
            SetFogNearFar(2000, 6000, 250);
            SetFarColor(0, 0, 0);
            GsMapModelingData((unsigned long *)(*(s32 *)(RAM_STAGE_LOAD + 0xC) + (RAM_STAGE_LOAD + 4)));
            GsLinkObject4(*(s32 *)(RAM_STAGE_LOAD + 0xC) + (RAM_STAGE_LOAD + 0xC), &sAct3Objs[0], 0);
            sAct3Objs[0].coord2 = &sAct3Coords[0];
            sAct3Objs[0].attribute = 0x200;
            D_8009EB58[0].vx = 0;
            D_8009EB58[0].vy = 0;
            D_8009EB58[0].vz = 0;
            sLights.l[0].r = 0x80;
            sLights.l[0].g = 0x80;
            sLights.l[0].b = 0x80;
            sLights.l[1].r = 0x80;
            sLights.l[1].g = 0x80;
            sLights.l[1].b = 0x80;
            sLights.l[2].r = 0x80;
            sLights.l[2].g = 0x80;
            sLights.l[2].b = 0x80;
            sLights.l[0].vx = 0;
            sLights.l[0].vy = 100;
            sLights.l[0].vz = 100;
            GsSetFlatLight(0, &sLights.l[0]);
            sLights.l[1].vx = 0x56;
            sLights.l[1].vy = 100;
            sLights.l[1].vz = -50;
            GsSetFlatLight(1, &sLights.l[1]);
            sLights.l[2].vx = -0x56;
            sLights.l[2].vy = 100;
            sLights.l[2].vz = -50;
            GsSetFlatLight(2, &sLights.l[2]);
            sView.vpx = 0;
            sView.vpy = -150;
            sView.vpz = 0;
            sView.vrx = -1000;
            sView.vry = -150;
            sView.vrz = 0;
            gPlayer.x = -10000;
            gPlayer.yaw = -0x400;
            gPlayer.action = 0x61;
            gPlayer.y = 0;
            gPlayer.z = 0;
            gPlayer.motion[0] = 3;
            PlayCdTrack(7, 0);
            gSubStep = 100;
            break;
        case 100:
            p = (POLY_F4 *)GsOUT_PACKET_P;
            setPolyF4(p);
            setSemiTrans(p, 1);
            gLoopI = ((gPlayer.x >= 0 ? gPlayer.x : -gPlayer.x) - 1000) / 20;
            if (gLoopI < 0) {
                gLoopI = 0;
            }
            if (gLoopI >= 256) {
                gLoopI = 255;
            }
            setXY4(p, -160, -120, 160, -120, -160, 120, 160, 120);
            p->r0 = p->g0 = p->b0 = gLoopI;
            addPrim(gOt[gActiveBuff].org, p);
            p++;
            GsOUT_PACKET_P = (PACKET *)p;
            t = (DR_TPAGE *)GsOUT_PACKET_P;
            setlen(t, 1);
            t->code[0] = 0xE1000640;
            addPrim(gOt[gActiveBuff].org, t);
            t++;
            GsOUT_PACKET_P = (PACKET *)t;
            gPlayer.x += 50;
            if (gPlayer.x >= -250) {
                sTitleCamTime = 0;
                gSubStep = 110;
            }
            break;
        case 110:
            gPlayer.x += 50;
            sView.vpx = gPlayer.x + rsin((-sTitleCamTime * 12 + 90) * 4096 / 360) * 250 / 4096;
            sView.vpy = -150;
            sView.vpz = rcos((-sTitleCamTime * 12 + 90) * 4096 / 360) * 150 / 4096;
            sView.vry = -150;
            sView.vrz = 0;
            sView.vrx = gPlayer.x;
            if (++sTitleCamTime == 15) {
                sTitleCamTime = 250;
                gSubStep = 120;
            }
            break;
        case 120:
            sView.vpy = -150;
            sView.vpz = 0;
            sView.vry = -150;
            sView.vrz = 0;
            gPlayer.x += 50;
            sView.vpx = gPlayer.x - sTitleCamTime;
            sView.vrx = gPlayer.x;
            if ((sTitleCamTime += 5) == 500) {
                /* MATCHING: no code; moves sTitleCamTime to global alloc. */
                sTitleCamTime = 500;
                sTitleStarted = 0;
                gSubStep = 130;
            }
            break;
        case 130:
            gPlayer.x += 50;
            if (sTitleStarted == 0) {
                if ((sTitleCamTime -= 5) == 300) {
                    sTitleStarted = 1;
                }
            } else {
                if ((sTitleCamTime += 5) == 500) {
                    sTitleStarted = 0;
                }
            }
            sView.vpy = -150;
            sView.vpz = 0;
            sView.vry = -150;
            sView.vrz = 0;
            sView.vpx = gPlayer.x - sTitleCamTime;
            sView.vrx = gPlayer.x;
            if (gPlayer.x == 11000) {
                sTitleCamTime = 0;
                PlaySe(0x23);
                gPlayer.action = 0x60;
                gPlayer.motion[0] = 0x33;
                gSubStep = 140;
            }
            break;
        case 140:
            if (sTitleCamTime < 5) {
                sView.vpx = gPlayer.x - 500 + sTitleCamTime * 50;
                sView.vrx = gPlayer.x;
                sView.vpy = sView.vry = rand() % 50 - 175;
                sView.vpz = sView.vrz = rand() % 50 - 25;
            } else if (sTitleCamTime < 10) {
                sView.vpx = gPlayer.x - 250 - (sTitleCamTime - 5) * 50;
                sView.vrx = gPlayer.x;
                sView.vpy = sView.vry = rand() % 50 - 175;
                sView.vpz = sView.vrz = rand() % 50 - 25;
            } else {
                sView.vpy = sView.vry = -150;
                sView.vpz = sView.vrz = 0;
                sView.vpx = gPlayer.x - 500;
                sView.vrx = gPlayer.x;
            }
            if (++sTitleCamTime == 31) {
                gSubStep = 1000;
            }
            break;
        case 1000:
            gPlayer.x = 11000;
            gPlayer.yaw = -0x400;
            gPlayer.action = 0x61;
            gPlayer.y = 0;
            gPlayer.z = 0;
            gPlayer.motion[0] = 0x34;
            sView.vpx = 0x2904;
            sView.vrx = 11000;
            sView.vpy = sView.vry = -150;
            sView.vpz = sView.vrz = 0;
            sTitleCamTime = 0;
            sTitleExitTime = 0;
            sTitleStarted = 0;
            gSubStep = 1001;
            PlaySe(0x50);
            break;
        case 1001:
            if (gPadTrig & PADstart) {
                if (sTitleStarted == 0) {
                    MemCardAccept(0);
                    PlaySe(0x22);
                    gPlayer.yaw = 0x400;
                    gPlayer.action = 0x60;
                    gPlayer.motion[0] = 0xF;
                    sTitleExitTime = 1;
                    sTitleStarted = 1;
                }
            }
            if (sTitleExitTime != 0) {
                if (++sTitleExitTime == 36) {
                    StopCdTrack();
                    gSubStep = 50;
                    gModeStep = 30;
                    StartScreenWipe();
                }
            } else {
                if (++sTitleCamTime == 450) {
                    StopCdTrack();
                    gSubStep = 50;
                    gModeStep = 2;
                }
            }
            break;
    }
    GsSetRefView2(&sView);
    if (gPlayer.motion[0] == 3 && !(gFrameCount & 7)) {
        PlaySe(0x13);
    }
    if (gSubStep >= 1000) {
        if (sTitleStarted == 0 || (gFrameCount & 1)) {
            color.r = 1;
            color.g = color.b = color.cd = 0x80;
            pos.vx = -128;
            pos.vy = 16;
            DrawSprite(0x1F, &pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
            color.r = 2;
            color.g = color.b = color.cd = 0x80;
            pos.vx = -128;
            pos.vy = 16;
            DrawSprite(0x20, &pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
        }
        color.r = -1;
        color.g = color.b = color.cd = 0x80;
        pos.vx = -128;
        pos.vy = 0x48;
        DrawSprite(0x21, &pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
        color.r = 2;
        color.g = color.b = color.cd = 0x80;
        pos.vx = -128;
        pos.vy = 0x48;
        DrawSprite(0x22, &pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
    }
    StepStepper(&gPlayer);
    DrawStepper(&gPlayer);
    DrawTitlePlayerShadow();
    sAct3Coords[0].coord.t[0] = 11000;
    sAct3Coords[0].coord.t[1] = -234;
    sAct3Coords[0].coord.t[2] = 0;
    m = GsIDMATRIX;
    m.t[0] = sAct3Coords[0].coord.t[0];
    m.t[1] = sAct3Coords[0].coord.t[1];
    m.t[2] = sAct3Coords[0].coord.t[2];
    RotMatrixYXZ(&D_8009EB58[0], &m);
    sAct3Coords[0].coord = m;
    sAct3Coords[0].flg = 0;
    GsGetLs(sAct3Objs[0].coord2, &m);
    GsSetLsMatrix(&m);
    pos.vx = 0x1FE;
    pos.vy = 0xC0;
    color.r = -1;
    color.g = color.b = color.cd = 0x80;
    DrawBillboard(0x1E, &pos, (CVECTOR *)&color, 2, &gOt[gActiveBuff]);
    sAct3Coords[0].coord.t[0] = 0;
    sAct3Coords[0].coord.t[1] = 0;
    sAct3Coords[0].coord.t[2] = 0;
    m = GsIDMATRIX;
    m.t[0] = sAct3Coords[0].coord.t[0];
    m.t[1] = sAct3Coords[0].coord.t[1];
    m.t[2] = sAct3Coords[0].coord.t[2];
    RotMatrixYXZ(&D_8009EB58[0], &m);
    sAct3Coords[0].coord = m;
    sAct3Coords[0].flg = 0;
    GsGetLs(sAct3Objs[0].coord2, &m);
    GsSetLsMatrix(&m);
    GsGetLw(sAct3Objs[0].coord2, &m);
    GsSetLightMatrix(&m);
    if (gPlayer.x != -10000) {
        GsSortObject4J(&sAct3Objs[0], &gOt[gActiveBuff], 2, getScratchAddr(0));
    }
    DrawTitleGroundShadow();
    if ((gPadTrig & PADstart) && gSubStep < 1000) {
        gSubStep = 1000;
    }
}

s32 StepGameOver(void) {
    SVECTOR pos;
    SColor color;
    MATRIX m;

    switch (gSubStep) {
        case 0:
            DefineSprite(0x1E, 1, 0xFF, 0x20, 5, 0, 0, 0, 0x1E0);
            DefineSprite(0x1F, 1, 0xFF, 0x20, 5, 0, 0x20, 0, 0x1E0);
            GsInitCoordinate2(&sAct3Coords[0], NULL);
            GsMapModelingData((unsigned long *)(*(s32 *)(RAM_STAGE_LOAD + 4) + (RAM_STAGE_LOAD + 4)));
            GsLinkObject4(*(s32 *)(RAM_STAGE_LOAD + 4) + (RAM_STAGE_LOAD + 0xC), &sAct3Objs[0], 0);
            sAct3Objs[0].coord2 = &sAct3Coords[0];
            D_8009EA78[0].vx = 0;
            sAct3Objs[0].attribute = 0;
            D_8009EA78[0].vy = -1000;
            D_8009EA78[0].vz = 0;
            D_8009EB58[0].vx = 0;
            D_8009EB58[0].vy = -0x2CC;
            D_8009EB58[0].vz = 0;
            sView.vrx = 0;
            sView.vry = -240;
            sView.vrz = 0;
            sView.vpx = 0x226;
            sView.vpy = -240;
            sView.vpz = 0x226;
            sLights.l[0].r = 0x80;
            sLights.l[0].g = 0x80;
            sLights.l[0].b = 0x80;
            sLights.l[1].r = 0x80;
            sLights.l[1].g = 0x80;
            sLights.l[1].b = 0x80;
            sLights.l[2].r = 0x80;
            sLights.l[2].g = 0x80;
            sLights.l[2].b = 0x80;
            sLights.l[0].vx = 0;
            sLights.l[0].vy = 100;
            sLights.l[0].vz = 100;
            GsSetFlatLight(0, &sLights.l[0]);
            sLights.l[1].vx = 0x56;
            sLights.l[1].vy = 100;
            sLights.l[1].vz = -50;
            GsSetFlatLight(1, &sLights.l[1]);
            sLights.l[2].vx = -0x56;
            sLights.l[2].vy = 100;
            sLights.l[2].vz = -50;
            GsSetFlatLight(2, &sLights.l[2]);
            sWobbleTime = 0;
            CdControlF(CdlSeekL, (u_char *)&sFiles3LastLoc[0]);
            CdSync(0, NULL);
            PlayXa(5);
            gSubStep++;
            break;
        case 1:
            if (++sWobbleTime < 10) {
                D_8009EA78[0].vy += 100;
            } else if (sWobbleTime < 35) {
                D_8009EA78[0].vy = 0;
                D_8009EB58[0].vx = sWobbleRots[sWobbleTime - 10].vx;
                D_8009EB58[0].vy = sWobbleRots[sWobbleTime - 10].vy;
                D_8009EB58[0].vz = sWobbleRots[sWobbleTime - 10].vz;
            }
            if ((u32)(sWobbleTime - 31) < 63) {
                color.r = 1;
                color.g = color.b = color.cd = (sWobbleTime - 30) * 2;
                pos.vx = -128;
                pos.vy = -16;
                DrawSprite(0x1E, &pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
                color.r = 2;
                pos.vx = -128;
                pos.vy = -16;
                color.g = color.b = color.cd = (sWobbleTime - 30) * 2;
                DrawSprite(0x1F, &pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
            }
            if (sWobbleTime >= 94) {
                color.r = 1;
                color.g = color.b = color.cd = 0x80;
                pos.vx = -128;
                pos.vy = -16;
                DrawSprite(0x1E, &pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
                color.r = 2;
                color.g = color.b = color.cd = 0x80;
                pos.vx = -128;
                pos.vy = -16;
                DrawSprite(0x1F, &pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
            }
            sAct3Coords[0].coord.t[0] = D_8009EA78[0].vx;
            sAct3Coords[0].coord.t[1] = D_8009EA78[0].vy;
            sAct3Coords[0].coord.t[2] = D_8009EA78[0].vz;
            m = GsIDMATRIX;
            m.t[0] = sAct3Coords[0].coord.t[0];
            m.t[1] = sAct3Coords[0].coord.t[1];
            m.t[2] = sAct3Coords[0].coord.t[2];
            RotMatrixYXZ(&D_8009EB58[0], &m);
            sAct3Coords[0].coord = m;
            sAct3Coords[0].flg = 0;
            GsGetLs(sAct3Objs[0].coord2, &m);
            GsSetLsMatrix(&m);
            GsGetLw(sAct3Objs[0].coord2, &m);
            GsSetLightMatrix(&m);
            SetTmdSortMode(0);
            GsSortObject4J(&sAct3Objs[0], &gSubOt[gActiveBuff], 2, getScratchAddr(0));
            sAct3Coords[0].flg = 0;
            sAct3Coords[0].coord.m[1][0] *= -1;
            sAct3Coords[0].coord.m[1][1] *= -1;
            sAct3Coords[0].coord.m[1][2] *= -1;
            sAct3Coords[0].coord.t[1] = -sAct3Coords[0].coord.t[1];
            GsGetLs(sAct3Objs[0].coord2, &m);
            GsSetLsMatrix(&m);
            SetTmdSortMode(5);
            GsSortObject4J(&sAct3Objs[0], &gSubOt[gActiveBuff], 2, getScratchAddr(0));
            if (sWobbleTime == 150) {
                if (gStageTableSet == 0) {
                    D_800957A0 = gStage;
                    D_800958C0.a = gActTimes.a;
                    D_800958C0.b = gActTimes.b;
                    D_80095978.a = gActCans.a;
                    D_80095978.b = gActCans.b;
                } else if (gStageTableSet == 1) {
                    D_800957A0 = gStage + 100;
                    D_800958C0.a = gActTimes.a;
                    D_800958C0.b = gActTimes.b;
                    D_80095978.a = gActCans.a;
                    D_80095978.b = gActCans.b;
                }
                StopXa();
                gModeStep = 0;
                gSubStep = 50;
            }
            break;
    }
}

inline void DrawMenuPlayer(void) {
    MATRIX m;
    SVECTOR rot;
    GsDOBJ2 *o;
    s32 i;

    SetTmdSortMode(1);
    o = ((DrawList *)gTodObjTable)->objs;
    o->coord2->coord.t[0] = gPlayer.x;
    o->coord2->coord.t[1] = gPlayer.y;
    o->coord2->coord.t[2] = gPlayer.z;
    rot.vx = 0;
    rot.vz = 0;
    rot.vy = gPlayer.yaw;
    SetCoordRotation(&rot, o->coord2);
    for (i = 0; i < ((DrawList *)gTodObjTable)->count; i++, o++) {
        o->coord2->flg = 0;
        if (o->id != -1 && o->tmd != NULL) {
            GsGetLs(o->coord2, &m);
            GsSetLsMatrix(&m);
            GsGetLw(o->coord2, &m);
            GsSetLightMatrix(&m);
            SetSpadStack();
            GsSortObject4J(o, &gSubOt[gActiveBuff], 2, getScratchAddr(0));
            ResetSpadStack();
        }
        /* MATCHING: as in world_intro.inc's model draw, three barriers rank
         * the object pointer above the counter (retail's s1 and s2). */
        __asm__("");
        __asm__("");
        __asm__("");
    }
}

inline void DrawScreenFade(void) {
    GsOT *ot;
    POLY_F4 *p;
    DR_TPAGE *t;

    ot = &gSubOt[gActiveBuff];
    switch (sFadeMode) {
        case 1:
            break;
        case 0:
            p = (POLY_F4 *)GsOUT_PACKET_P;
            setPolyF4(p);
            p->r0 = p->g0 = p->b0 = 0;
            p->x0 = p->x2 = -160;
            p->x1 = p->x3 = 160;
            p->y0 = p->y1 = -120;
            p->y2 = p->y3 = 120;
            addPrim(ot->org, p);
            p++;
            GsOUT_PACKET_P = (PACKET *)p;
            break;
        case 2:
            p = (POLY_F4 *)GsOUT_PACKET_P;
            setPolyF4(p);
            setSemiTrans(p, 1);
            p->x0 = p->x2 = -160;
            p->x1 = p->x3 = 160;
            p->y0 = p->y1 = -120;
            p->y2 = p->y3 = 120;
            p->r0 = p->g0 = p->b0 = ~(sFadeTime * 8);
            addPrim(ot->org, p);
            p++;
            t = (DR_TPAGE *)p;
            setlen(t, 1);
            t->code[0] = 0xE1000640;
            addPrim(ot->org, t);
            t++;
            GsOUT_PACKET_P = (PACKET *)t;
            sFadeTime++;
            if (sFadeTime >= 32) {
                sFadeMode = 1;
            }
            break;
        case 3:
            p = (POLY_F4 *)GsOUT_PACKET_P;
            setPolyF4(p);
            setSemiTrans(p, 1);
            p->x0 = p->x2 = -160;
            p->x1 = p->x3 = 160;
            p->y0 = p->y1 = -120;
            p->y2 = p->y3 = 120;
            p->r0 = p->g0 = p->b0 = sFadeTime * 8;
            addPrim(ot->org, p);
            p++;
            t = (DR_TPAGE *)p;
            setlen(t, 1);
            t->code[0] = 0xE1000640;
            addPrim(ot->org, t);
            t++;
            GsOUT_PACKET_P = (PACKET *)t;
            sFadeTime++;
            if (sFadeTime >= 32) {
                sFadeMode = 0;
            }
            break;
    }
}

inline void DrawMenuCursor(SVECTOR *pos, s32 kind) {
    switch (kind) {
        case 1:
            DrawSprite(0x53, pos, NULL, 0, &gSubOt[gActiveBuff]);
            break;
        case 2:
            DrawSprite(0x54, pos, NULL, 0, &gSubOt[gActiveBuff]);
            break;
        case 3:
            DrawSprite(0x55, pos, NULL, 0, &gSubOt[gActiveBuff]);
            pos->vx += 0x90;
            DrawSprite(0x56, pos, NULL, 0, &gSubOt[gActiveBuff]);
            break;
    }
}

inline void StepMenuMotion(PlayerState *obj) {
    obj->motionResult[0] = StepAnimation(obj, 0, obj->animFilter);
    switch (obj->action) {
        case 0x60:
            break;
        case 0x61:
            if (obj->motionResult[0] == 0) {
                obj->curMotion[0] = 0xFF;
            }
            break;
    }
}

inline void StartFadeIn(void) {
    sFadeTime = 0;
    sFadeMode = 2;
}

inline void StartFadeOut(void) {
    sFadeTime = 0;
    sFadeMode = 3;
}

inline void OrbitMenuCamera(SVECTOR *rot) {
    MATRIX m;
    SVECTOR v;
    VECTOR out;

    v.vx = 300;
    v.vy = 0;
    v.vz = 0;
    m = GsIDMATRIX;
    RotMatrix(rot, &m);
    SetRotMatrix(&m);
    SetTransMatrix(&m);
    RotTrans(&v, &out, 0);
    sView.vrx = 0;
    sView.vry = -100;
    sView.vrz = 0;
    sView.vpx = out.vx;
    sView.vpy = out.vy - 100;
    sView.vpz = out.vz;
    GsSetRefView2(&sView);
}
