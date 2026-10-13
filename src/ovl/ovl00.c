/* The developer menu, the overlay main loads for GM_OVERLAY0 and runs each
 * frame: a list of the game's modes and test modes, the stage to start, the
 * vibration test's readings, and the sound test it hands over to. */

#include "common.h"
#include "libgte.h"
#include "libgpu.h"
#include "libgs.h"
#include "libetc.h"
#include "object.h"
#include "model.h"

/* MATCHING: views of routines their units declare only where defined. */
s32 LoadVabBank(s32 bank);
void PlayBgm(u16 song, s32 unused);
void StopBgm(s32 unused);
void StopFinishedSe(void);
void StartRumble(u16 *table, s32 index);
void ResetClearColor(void);

/* MATCHING: read signed here; their units keep them unsigned. */
extern s16 gRumbleSmall[];
extern s16 gRumbleTime;
extern s16 gRumbleRepeat;
extern s16 gRumbleWait;
extern s16 gRumbleNext;
extern s16 gBgmVab;
extern u16 gVabBank;
extern u8 gRumbleEnabled;

/* The vibration test's pattern, an index into gRumblePatterns. */
extern s16 D_800958B8[];

extern s32 gGridLastAngle;
extern s32 gGridLastValue;
extern s32 gGridLastX;
extern s32 gGridLastY;
extern s32 D_80095814;
extern s32 D_80095818;

/* The sound test's choices: the VAB bank, the sound effect, the BGM. */
extern u16 D_800F1CF8;
extern u16 D_800F1CFA;
extern u16 D_800F1CFC;

/* The highlighted line's colour, and the others'. */
#define PRINT_LINE(n)       \
    if (gMenuLine == (n)) { \
        FntPrint("~c888");  \
    } else {                \
        FntPrint("~c008");  \
    }

s32 func_800F1060(void);
void func_800F09C0(void);

void func_800F026C(void) {
    /* MATCHING: an unused 8-byte local gives retail's frame. */
    s32 unused[2];

    switch (gModeStep) {
        case 0:
            PlayBgm(2, 0);
            gRefView[0] = 0;
            gRefView[2] = 400;
            gRefView[1] = 0;
            gRefView[3] = 0;
            gRefView[4] = 0;
            gRefView[5] = 0;
            gEditMax = 15;
            gMenuLineCount = 11;
            gEditValue = 0;
            gMenuLine = 0;
            gVSyncMode = 2;
            gClearB = 0;
            gClearG = 0;
            gClearR = 0;
            gRumbleSmall[1] = 0;
            gRumbleSmall[0] = 0;
            D_800958B8[0] = 0;
            gPlayer.hittable = 1;
            gVabLoadStep = 0;
            gModeStep++;
            break;
        case 1:
            if (LoadVabBank(1) == 1) {
                gModeStep++;
            }
            break;
        case 2:
            if (gMenuLine == 4) {
                if (gPadTrig & PADLright) {
                    D_800958B8[0]++;
                }
                if (gPadTrig & PADLleft) {
                    D_800958B8[0]--;
                }
                D_800958B8[0] = D_800958B8[0] < 0 ? 0 : D_800958B8[0] >= 32 ? 31 : D_800958B8[0];
                if (gPadTrig & PADRup) {
                    StartRumble(gRumblePatterns, D_800958B8[0]);
                }
            } else {
                gStage = gEditValue;
            }
            if (func_800F1060() == 0) {
                PRINT_LINE(0)
                FntPrint("\n0.GAME MODE");
                FntPrint("~c888\t\t  << STAGE %d-", (u8)(gStage / 3) + 1);
                switch ((u8)(gStage % 3)) {
                    case 0:
                        FntPrint("1");
                        break;
                    case 1:
                        FntPrint("2");
                        break;
                    case 2:
                        FntPrint("BOSS");
                        break;
                }
                FntPrint(" >>");
                PRINT_LINE(1)
                FntPrint("\n1.SOUND TEST MODE");
                PRINT_LINE(2)
                FntPrint("\n2.MOVIE TEST MODE");
                PRINT_LINE(3)
                FntPrint("\n3.MOTION TEST MODE");
                PRINT_LINE(4)
                FntPrint("\n4.VIBRATION TEST MODE");
                PRINT_LINE(5)
                FntPrint("\n5.TRAP TEST MODE");
                PRINT_LINE(6)
                FntPrint("\n6.TRAP TEST MODE 2ND");
                PRINT_LINE(7)
                FntPrint("\n7.TOOL0 MODE");
                PRINT_LINE(8)
                FntPrint("\n8.TOOL1 MODE");
                PRINT_LINE(9)
                FntPrint("\n9.YAMA0 MODE");
                PRINT_LINE(10)
                FntPrint("\n10.DEMO MODE");
                FntPrint("\n\n~c888DUAL SHOCK (PUSH SELECT) : ");
                if (gPadTrig & PADselect) {
                    gRumbleEnabled ^= 1;
                }
                switch (gRumbleEnabled) {
                    case 0:
                        FntPrint("OFF");
                        break;
                    case 1:
                        FntPrint("ON");
                        break;
                }
            } else {
                switch (gMenuLine) {
                    case 0:
                        gModeStep = 50;
                        gSubStep = 0;
                        break;
                    case 4:
                        break;
                    case 9:
                        gGameMode = 6;
                        break;
                    case 10:
                        gGameMode = 0x26;
                        break;
                }
            }
            if (gMenuLine == 4) {
                FntPrint("\n\n~c888");
                FntPrint("\nDEVICE ID->%x", gPadType);
                FntPrint("\nSHOCKTABLE NO->%d", D_800958B8[0]);
                FntPrint("\nSHOCK0->%3d SHOCK1->%3d", gRumbleSmall[0], gRumbleSmall[1]);
                FntPrint("\nFRAME->%3d REP->%3d WAIT->%3d", gRumbleTime, gRumbleRepeat, gRumbleWait);
                FntPrint("\nNEXT TABLE NO->%d", gRumbleNext);
            }
            ResetClearColor();
            UpdateRumble();
            break;
        case 50:
            StartRumble(gRumblePatterns, 0);
            SetSoundFade(1);
            gModeStep++;
            break;
        case 51:
            gGameMode = GM_STAGE_LOAD;
            break;
        case 100:
            func_800F09C0();
            break;
    }
}

void func_800F09C0(void) {
    u16 prev;

    switch (gWorldIntroStep) {
        case 0:
            SetSoundFade(1);
            gEditMax = 15;
            gMenuLineCount = 5;
            D_800F1CFC = 0;
            D_800F1CF8 = 0;
            D_800F1CFA = 70;
            gEditValue = 0;
            gMenuLine = 0;
            gVabLoadStep = 0;
            gWorldIntroStep++;
            break;
        case 1:
            if (LoadVabBank(D_800F1CF8) == 1) {
                gWorldIntroStep++;
            }
            break;
        case 2:
            prev = gMenuLine;
            switch (gMenuLine) {
                case 0:
                    D_800F1CF8 = gEditValue;
                    break;
                case 1:
                    D_800F1CFC = gEditValue;
                    break;
                case 2:
                    D_800F1CFA = gEditValue;
                    break;
            }
            FntPrint("\n\nVABID NO 0:%d 1:%d", gBgmVab, (&gBgmVab)[1]);
            FntPrint("\nVAB1  NO %d", gVabBank);
            if (func_800F1060() == 0) {
                PRINT_LINE(0)
                FntPrint("\n\nVAB NO %d", D_800F1CF8);
                PRINT_LINE(1)
                FntPrint("\nBGM %d", D_800F1CFC);
                PRINT_LINE(2)
                FntPrint("\nSE %d", D_800F1CFA + 1);
                PRINT_LINE(3)
                FntPrint("\nFADEOUT");
                PRINT_LINE(4)
                FntPrint("\nEXIT");
            } else {
                switch (gMenuLine) {
                    case 0:
                        gWorldIntroStep = 1;
                        gVabLoadStep = 0;
                        break;
                    case 1:
                        PlayBgm(D_800F1CFC, 0);
                        break;
                    case 2:
                        PlaySe(D_800F1CFA + 11);
                        break;
                    case 3:
                        SetSoundFade(3);
                        break;
                    case 4:
                        gModeStep = 0;
                        StopBgm(0);
                        StopFinishedSe();
                        break;
                }
            }
            if (gMenuLine != prev) {
                switch (gMenuLine) {
                    case 0:
                        gEditMax = 15;
                        gEditValue = D_800F1CF8;
                        break;
                    case 1:
                        gEditMax = 11;
                        gEditValue = D_800F1CFC;
                        break;
                    case 2:
                        gEditMax = 0x87;
                        gEditValue = D_800F1CFA;
                        break;
                }
            }
            ResetClearColor();
            FntPrint("\nD0->%d %x", gGridLastAngle, gGridLastAngle);
            FntPrint("\nD1->%d %x", gGridLastValue, gGridLastValue);
            FntPrint("\nD2->%d %x", gGridLastX, gGridLastX);
            FntPrint("\nD3->%d %x", gGridLastY, gGridLastY);
            FntPrint("\nD4->%d %x", D_80095814, D_80095814);
            FntPrint("\nD5->%d %x", D_80095818, D_80095818);
            break;
    }
}

/* Turns the player model with the pad and moves it and the view: the
 * motion test's controls. */
/* MATCHING: an unused 40-byte local gives retail's frame. */
s32 func_800F0E68(void) {
    s32 unused[10];

    if (gPad & PADRup) {
        gBodyRot[1] -= 0x38;
    }
    if (gPad & PADRleft) {
        gBodyRot[1] += 0x38;
    }
    if (gPad & PADR1) {
        gBodyRot[0] += 0x38;
    }
    if (gPad & PADR2) {
        gBodyRot[0] -= 0x38;
    }
    if (gPad & PADLleft) {
        gTodCoords[0].coord.t[0] += 20;
    }
    if (gPad & PADLright) {
        gTodCoords[0].coord.t[0] -= 20;
    }
    if (gPad & PADRright) {
        gTodCoords[0].coord.t[1] += 20;
    }
    if (gPad & PADRdown) {
        gTodCoords[0].coord.t[1] -= 20;
    }
    if (gPad & PADLup) {
        gTodCoords[0].coord.t[2] -= 100;
    }
    if (gPad & PADLdown) {
        gTodCoords[0].coord.t[2] += 100;
    }
    if (gPad & PADL1) {
        gRefView[1] += 50;
    }
    if (gPad & PADL2) {
        gRefView[1] -= 50;
    }
    SetCoordRotation((SVECTOR *)gBodyRot, &gTodCoords[0]);
    return 1;
}

void func_800F1218(void);
void func_800F11E8(void);
s32 func_800F1158(void);

/* Steps the edited value or the highlighted line from the pad and wraps
 * them; returns 1 when a choice is made. */
s32 func_800F1060(void) {
    if (gPadTrig & (PADRleft | PADRright)) {
        if (gModeStep != 100) {
            func_800F1218();
            SetSoundFade(1);
        }
        return 1;
    }
    if (gPadType == PAD_TYPE_DIGITAL || gPadType == PAD_TYPE_ANALOG) {
        /* MATCHING: each branch makes its own calls; cross-jumping merges
         * the tails into retail's shared ones. */
        if (gPadTrig & PADLright) {
            gEditValue++;
            func_800F11E8();
            func_800F1158();
            return 0;
        }
        if (gPadTrig & PADLleft) {
            gEditValue--;
            func_800F11E8();
            func_800F1158();
            return 0;
        }
        if (gPadTrig & PADLdown) {
            gMenuLine++;
            func_800F11E8();
            func_800F1158();
            return 0;
        }
        if (gPadTrig & PADLup) {
            gMenuLine--;
            func_800F11E8();
            func_800F1158();
            return 0;
        }
    }
    return 0;
}

/* This image's copy of editor's WrapMenuValues. */
s32 func_800F1158(void) {
    /* MATCHING: retail reads the edited value signed (lh); common.h's
     * declaration is u16. */
    if ((s16)gEditValue < 0) {
        gEditValue = gEditMax - 1;
    }
    if ((s16)gEditValue >= gEditMax) {
        gEditValue = 0;
    }
    if (gMenuLine < 0) {
        gMenuLine = gMenuLineCount - 1;
    }
    if (gMenuLine >= gMenuLineCount) {
        gMenuLine = 0;
    }
    return 0;
}

/* The cursor's click, outside the sound test. */
void func_800F11E8(void) {
    if (gModeStep != 100) {
        PlaySe(0xB);
    }
}

/* The choice's sound. */
void func_800F1218(void) {
    PlaySe(SE_POWER_UP);
}
