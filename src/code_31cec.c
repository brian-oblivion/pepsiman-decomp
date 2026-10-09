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

#ifdef NON_MATCHING
/* Off by one register choice: the mode byte is declared as an array in
 * common.h, but this function reads it as a scalar. */
void func_800414EC(s16 arg0) {
    s32 mode = D_80095830[0];

    if (mode != 1) {
        if (mode < 2) {
            if (mode == 0) {
                func_800FA610(arg0);
            }
        }
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/code_31cec", func_800414EC);
#endif

INCLUDE_ASM("asm/nonmatchings/code_31cec", func_80041534);

INCLUDE_ASM("asm/nonmatchings/code_31cec", func_80041964);

INCLUDE_ASM("asm/nonmatchings/code_31cec", func_80041A6C);

INCLUDE_ASM("asm/nonmatchings/code_31cec", func_80041BAC);

INCLUDE_ASM("asm/nonmatchings/code_31cec", func_80041C7C);

INCLUDE_ASM("asm/nonmatchings/code_31cec", func_80041D18);

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

INCLUDE_ASM("asm/nonmatchings/code_31cec", func_800428EC);

extern s16 D_80095AF0;

void func_80042958(u8 value) {
    D_80095AF0 = value;
}

INCLUDE_ASM("asm/nonmatchings/code_31cec", func_80042968);

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
