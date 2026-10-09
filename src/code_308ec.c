#include "common.h"

extern u8 D_80095AA8;
extern u8 D_80095AA9;

void func_800400EC(void) {
    s32 i;

    for (i = 0; i < 128; i++) {
        D_800DFAB0[i].unk13 = i + 1;
    }
    D_80095AA9 = 0;
    D_80095AA8 = 0xFF;
}

INCLUDE_ASM("asm/nonmatchings/code_308ec", func_80040130);

INCLUDE_ASM("asm/nonmatchings/code_308ec", func_800401F0);

INCLUDE_ASM("asm/nonmatchings/code_308ec", func_80040628);

INCLUDE_ASM("asm/nonmatchings/code_308ec", func_8004079C);

INCLUDE_ASM("asm/nonmatchings/code_308ec", func_80040998);

INCLUDE_ASM("asm/nonmatchings/code_308ec", func_80040CD0);

INCLUDE_ASM("asm/nonmatchings/code_308ec", func_80040E04);

INCLUDE_ASM("asm/nonmatchings/code_308ec", func_80040F14);

INCLUDE_ASM("asm/nonmatchings/code_308ec", func_80041118);
