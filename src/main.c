#include "common.h"

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

INCLUDE_ASM("asm/nonmatchings/main", func_80014D20);

INCLUDE_ASM("asm/nonmatchings/main", func_80014D6C);

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

INCLUDE_ASM("asm/nonmatchings/main", func_8001552C);

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
