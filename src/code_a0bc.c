#include "common.h"
#include "code_a0bc.h"

/** @brief An eight-byte table entry: two halfwords and four bytes. */
typedef struct {
    u16 unk0; /**< a byte and a bit packed together */
    u16 unk2; /**< a CLUT-style packed position */
    u8 unk4;  /**< not yet known */
    u8 unk5;  /**< not yet known */
    u8 unk6;  /**< not yet known */
    u8 unk7;  /**< not yet known */
} Sprite8;

/* MATCHING: this unit's view of the table code_1a098 copies as halfwords. */
extern Sprite8 D_800DD0A0[];

INCLUDE_ASM("asm/nonmatchings/code_a0bc", func_800198BC);

INCLUDE_ASM("asm/nonmatchings/code_a0bc", func_80019CD8);

INCLUDE_ASM("asm/nonmatchings/code_a0bc", func_80019F24);

INCLUDE_ASM("asm/nonmatchings/code_a0bc", func_8001A3D4);

INCLUDE_ASM("asm/nonmatchings/code_a0bc", func_8001A69C);

INCLUDE_ASM("asm/nonmatchings/code_a0bc", func_8001A950);

INCLUDE_ASM("asm/nonmatchings/code_a0bc", func_8001ACB4);

INCLUDE_ASM("asm/nonmatchings/code_a0bc", func_8001B004);

void func_8001B2F4(u16 idx, u8 a1, u8 a2, u8 a3, s32 a4, u8 a5, u8 a6, u16 a7, s32 a8) {
    D_800DD0A0[idx].unk0 = (u8)a4 | (a1 << 7);
    D_800DD0A0[idx].unk2 = (a8 << 6) | (a7 >> 4);
    D_800DD0A0[idx].unk4 = a5;
    D_800DD0A0[idx].unk5 = a6;
    D_800DD0A0[idx].unk6 = a2;
    D_800DD0A0[idx].unk7 = a3;
}

INCLUDE_ASM("asm/nonmatchings/code_a0bc", func_8001B354);

INCLUDE_ASM("asm/nonmatchings/code_a0bc", func_8001B4BC);

INCLUDE_ASM("asm/nonmatchings/code_a0bc", func_8001B9A4);

INCLUDE_ASM("asm/nonmatchings/code_a0bc", func_8001C13C);

INCLUDE_ASM("asm/nonmatchings/code_a0bc", func_8001C878);

INCLUDE_ASM("asm/nonmatchings/code_a0bc", func_8001D39C);

INCLUDE_ASM("asm/nonmatchings/code_a0bc", func_8001DAD4);

INCLUDE_ASM("asm/nonmatchings/code_a0bc", func_8001E558);

INCLUDE_ASM("asm/nonmatchings/code_a0bc", func_8001EE30);

INCLUDE_ASM("asm/nonmatchings/code_a0bc", func_8001FBBC);

INCLUDE_ASM("asm/nonmatchings/code_a0bc", func_8001FE5C);

INCLUDE_ASM("asm/nonmatchings/code_a0bc", func_800201BC);

INCLUDE_ASM("asm/nonmatchings/code_a0bc", func_80020520);

INCLUDE_ASM("asm/nonmatchings/code_a0bc", func_8002097C);

INCLUDE_ASM("asm/nonmatchings/code_a0bc", func_80020C14);

INCLUDE_ASM("asm/nonmatchings/code_a0bc", func_80020CF8);

INCLUDE_ASM("asm/nonmatchings/code_a0bc", func_80020DD8);

INCLUDE_ASM("asm/nonmatchings/code_a0bc", func_80020F24);

INCLUDE_ASM("asm/nonmatchings/code_a0bc", func_800210B4);

INCLUDE_ASM("asm/nonmatchings/code_a0bc", func_80021240);

INCLUDE_ASM("asm/nonmatchings/code_a0bc", func_80021434);
