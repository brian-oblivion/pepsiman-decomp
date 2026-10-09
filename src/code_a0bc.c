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

/** @brief A primitive handler: one entry of the handler table. */
typedef void (*PrimFunc)();

/* The handler table, eight rows of eight handlers. */
extern PrimFunc D_800E48E8[8][8];

void func_8001B4BC();
void func_8001B9A4();
void func_8001C13C();
void func_8001C878();
void func_8001D39C();
void func_8001DAD4();
void func_8001E558();
void func_8001EE30();
void func_8001FBBC();
void func_8001FE5C();
void func_800201BC();
void func_80020520();
void func_8002097C();
void func_80020DD8();
void func_80020F24();
void func_800210B4();
void func_80021240();
void func_80021434();

/* Handlers code_11dc4 defines. */
void func_800215C4();
void func_800217A8();
void func_80021958();
void func_80021B88();
void func_80021D3C();
void func_80021F80();
void func_80022150();
void func_8002230C();

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

void func_80020C14(void) {
    D_800E48E8[0][6] = func_80020DD8;
    D_800E48E8[0][7] = func_8001B4BC;
    D_800E48E8[1][6] = func_800210B4;
    D_800E48E8[1][7] = func_8001C13C;
    D_800E48E8[2][6] = func_80021434;
    D_800E48E8[2][7] = func_8001D39C;
    D_800E48E8[3][0] = func_80021F80;
    D_800E48E8[3][6] = func_800217A8;
    D_800E48E8[3][7] = func_8001E558;
    D_800E48E8[4][6] = func_80020F24;
    D_800E48E8[4][7] = func_8001B9A4;
    D_800E48E8[5][6] = func_80021240;
    D_800E48E8[5][7] = func_8001C878;
    D_800E48E8[6][6] = func_800215C4;
    D_800E48E8[6][7] = func_8001DAD4;
    D_800E48E8[7][0] = func_8002097C;
    D_800E48E8[7][6] = func_80021958;
    D_800E48E8[7][7] = func_8001EE30;
}

INCLUDE_ASM("asm/nonmatchings/code_a0bc", func_80020CF8);

INCLUDE_ASM("asm/nonmatchings/code_a0bc", func_80020DD8);

INCLUDE_ASM("asm/nonmatchings/code_a0bc", func_80020F24);

INCLUDE_ASM("asm/nonmatchings/code_a0bc", func_800210B4);

INCLUDE_ASM("asm/nonmatchings/code_a0bc", func_80021240);

INCLUDE_ASM("asm/nonmatchings/code_a0bc", func_80021434);
