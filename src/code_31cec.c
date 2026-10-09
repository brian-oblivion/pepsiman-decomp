#include "common.h"
#include "libsnd.h"
#include "libcd.h"
#include "libetc.h"
#include "libpress.h"

extern s16 D_800E0570[];
extern char D_800E0588[];
extern s32 D_80095AB4;
extern u8 D_80095AEE;

void func_800FA610(s16 arg0);
void func_80041EE0(CdlLOC *loc);

void func_800414EC(s16 arg0) {
    s32 mode = D_80095830;

    if (mode != 1) {
        if (mode < 2) {
            if (mode == 0) {
                func_800FA610(arg0);
            }
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/code_31cec", func_80041534);

INCLUDE_ASM("asm/nonmatchings/code_31cec", func_80041964);

INCLUDE_ASM("asm/nonmatchings/code_31cec", func_80041A6C);

INCLUDE_ASM("asm/nonmatchings/code_31cec", func_80041BAC);

INCLUDE_ASM("asm/nonmatchings/code_31cec", func_80041C7C);

void func_80041D18(CdlLOC *loc, void (*callback)()) {
    DecDCTReset(0);
    D_80095AB4 = 0;
    DecDCToutCallback(callback);
    StSetRing((u_long *)0x80185000, 32);
    StSetStream(1, 1, -1, 0, 0);
    func_80041EE0(loc);
}

INCLUDE_ASM("asm/nonmatchings/code_31cec", func_80041D88);

INCLUDE_ASM("asm/nonmatchings/code_31cec", func_80041E20);

void func_80041EE0(CdlLOC *loc) {
    do {
        while (CdControl(CdlSetloc, (u_char *)loc, 0) == 0) {
        }
    } while (CdRead2(CdlModeStream | CdlModeSpeed | CdlModeRT) == 0);
}

INCLUDE_ASM("asm/nonmatchings/code_31cec", func_80041F28);

INCLUDE_ASM("asm/nonmatchings/code_31cec", func_80042058);

INCLUDE_ASM("asm/nonmatchings/code_31cec", func_80042150);

INCLUDE_ASM("asm/nonmatchings/code_31cec", func_80042208);

INCLUDE_ASM("asm/nonmatchings/code_31cec", func_80042538);

INCLUDE_ASM("asm/nonmatchings/code_31cec", func_800426A4);

void func_800428B0(void) {
    SsInit();
    SsSetTableSize(D_800E0588, 9, 1);
    SsSetTickMode(1);
}

void func_800428EC(void) {
    SsSetMVol(0, 0);
    SsSetRVol(0, 0);
    SsUtSetReverbType(SS_REV_TYPE_STUDIO_B);
    VSync(0);
    SsSetMVol(127, 127);
    SsSetRVol(120, 120);
    SsUtReverbOn();
    SsUtSetReverbDepth(48, 48);
}

extern s16 D_80095AF0;

void func_80042958(u8 value) {
    D_80095AF0 = value;
}

void func_80042968(u8 vol) {
    CdlATV atv;

    if (D_80095AEE) {
        atv.val2 = vol;
        atv.val0 = vol;
        atv.val3 = 0;
        atv.val1 = 0;
    } else {
        atv.val2 = vol;
        atv.val0 = vol;
        atv.val3 = vol;
        atv.val1 = vol;
    }
    CdMix(&atv);
}

void func_800429B4(void) {
    SsSeqStop(D_800E0570[0]);
    SsSeqClose(D_800E0570[0]);
}

INCLUDE_ASM("asm/nonmatchings/code_31cec", func_800429EC);

INCLUDE_ASM("asm/nonmatchings/code_31cec", func_80042A88);

INCLUDE_ASM("asm/nonmatchings/code_31cec", func_80042B80);

void func_80042C14(void) {
    CdlATV vol;

    vol.val3 = 0;
    vol.val2 = 0;
    vol.val1 = 0;
    vol.val0 = 0;
    CdMix(&vol);
    CdControlF(CdlPause, 0);
}

void func_80042C50(void) {}
