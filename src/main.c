#include "common.h"
#include "libgte.h"
#include "libgpu.h"
#include "libpad.h"

INCLUDE_RODATA("asm/nonmatchings/main", D_80010000);

INCLUDE_ASM("asm/nonmatchings/main", main);

INCLUDE_ASM("asm/nonmatchings/main", func_80013B38);

INCLUDE_ASM("asm/nonmatchings/main", func_80013CDC);

INCLUDE_ASM("asm/nonmatchings/main", func_80013EE4);

INCLUDE_ASM("asm/nonmatchings/main", func_80014044);

INCLUDE_ASM("asm/nonmatchings/main", func_800142EC);

INCLUDE_ASM("asm/nonmatchings/main", func_800148B0);

INCLUDE_ASM("asm/nonmatchings/main", func_800149D0);

INCLUDE_ASM("asm/nonmatchings/main", func_80014AC8);

INCLUDE_ASM("asm/nonmatchings/main", func_80014B8C);

INCLUDE_ASM("asm/nonmatchings/main", func_80014BF0);

INCLUDE_ASM("asm/nonmatchings/main", func_80014C58);

extern s32 D_800957A8;
extern u8 D_8009574C;
extern u8 D_80095754;
extern u8 D_8009575C;

void func_80014CF0(void) {
    /* MATCHING: retail reserves an 8-byte frame it never touches. */
    s32 unused[2];

    D_800957A8 = 2;
    D_8009574C = 0;
    D_80095754 = 0;
    D_8009575C = 0;
}

void func_80014D20(void) {
    RECT rect;

    rect.x = 0;
    rect.y = 0;
    rect.w = 0x400;
    rect.h = 0x200;
    ClearImage(&rect, 0, 0, 0);
    DrawSync(0);
}

void func_80014D6C(void) {
    FntLoad(0x3C0, 0);
    FntOpen(-0x9A, -0x74, 0x140, 0x100, 0, 0x400);
}

INCLUDE_ASM("asm/nonmatchings/main", func_80014DB0);

INCLUDE_ASM("asm/nonmatchings/main", func_80014FA8);

INCLUDE_ASM("asm/nonmatchings/main", func_80015180);

extern u8 *D_80095704;

void func_80015328(s32 offset, u8 a, u8 b) {
    D_80095704[offset + 2] = a;
    D_80095704[offset + 3] = b;
}

INCLUDE_ASM("asm/nonmatchings/main", func_8001534C);

INCLUDE_ASM("asm/nonmatchings/main", func_800153CC);

INCLUDE_ASM("asm/nonmatchings/main", func_80015450);

INCLUDE_ASM("asm/nonmatchings/main", func_800154C4);

extern u8 *D_80095700;

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

INCLUDE_ASM("asm/nonmatchings/main", func_80015584);

INCLUDE_ASM("asm/nonmatchings/main", func_80015754);

INCLUDE_ASM("asm/nonmatchings/main", func_800157DC);

INCLUDE_ASM("asm/nonmatchings/main", func_80015A28);

INCLUDE_ASM("asm/nonmatchings/main", func_80015B78);

INCLUDE_ASM("asm/nonmatchings/main", func_80015CC8);

INCLUDE_ASM("asm/nonmatchings/main", func_800160E8);

INCLUDE_ASM("asm/nonmatchings/main", func_80016D14);

INCLUDE_ASM("asm/nonmatchings/main", func_80016FC0);

INCLUDE_ASM("asm/nonmatchings/main", func_80017124);

INCLUDE_ASM("asm/nonmatchings/main", func_80017270);

INCLUDE_ASM("asm/nonmatchings/main", func_800173E8);

INCLUDE_ASM("asm/nonmatchings/main", func_80017440);

INCLUDE_RODATA("asm/nonmatchings/main", D_80010404);
