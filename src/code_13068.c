#include "common.h"

extern u16 D_800957D2;
extern s8 D_8009599C;

/** @brief The game-wide state record, as far as this unit reads it. The
 *         shared header declares it as a byte array; the rest of the layout
 *         is still unknown. */
typedef struct {
    u8 unk0; /**< set to 1 on a reset; not yet known */
    u8 pad1;
    u8 unk2; /**< cleared on a reset when the flag byte is 1 */
    u8 pad3[2];
    u8 unk5; /**< cleared on a reset */
    u8 unk6; /**< 2 on a reset, 0x33 when unk3D0's low nibble is 3 */
    u8 pad7[0x38E - 0x7];
    u8 unk38E; /**< cleared on a reset */
    u8 pad38F;
    s16 unk390; /**< set to 2 together with clearing unk398 */
    u8 pad392[0x398 - 0x392];
    s16 unk398; /**< cleared together with setting unk390 */
    u8 pad39A[0x3A8 - 0x39A];
    s16 unk3A8; /**< a count that unk3AC grades in three bands */
    u8 pad3AA[0x3AC - 0x3AA];
    s16 unk3AC; /**< 0 below 13 in unk3A8, 1 below 26, else 2 */
    u8 pad3AE[0x3C8 - 0x3AE];
    s16 unk3C8; /**< set to 60 on a reset when the flag byte is 1 */
    u8 pad3CA[0x3D0 - 0x3CA];
    u8 unk3D0; /**< low nibble read on a reset */
} GameState;

/* MATCHING: a struct lvalue keeps the base in a register; array offsets fold into %lo. */
#define sGame (*(GameState *)D_8009EB78)

/* MATCHING: retail reaches these through a split lui/%lo pair, so each is
 * an array of unknown size here. */
extern s32 D_800D8440[];
extern u8 D_800D8960[];
extern s16 D_800D38DE[];

INCLUDE_ASM("asm/nonmatchings/code_13068", func_80022868);

INCLUDE_ASM("asm/nonmatchings/code_13068", func_800229A8);

INCLUDE_ASM("asm/nonmatchings/code_13068", func_80022A74);

void func_80022F58(void) {}

void func_80022F60(void) {}

INCLUDE_ASM("asm/nonmatchings/code_13068", func_80022F68);

INCLUDE_ASM("asm/nonmatchings/code_13068", func_80023020);

void func_800230D8(void) {}

INCLUDE_ASM("asm/nonmatchings/code_13068", func_800230E0);

INCLUDE_ASM("asm/nonmatchings/code_13068", func_80023194);

INCLUDE_ASM("asm/nonmatchings/code_13068", func_80023228);

INCLUDE_ASM("asm/nonmatchings/code_13068", func_80023764);

INCLUDE_ASM("asm/nonmatchings/code_13068", func_80023834);

INCLUDE_ASM("asm/nonmatchings/code_13068", func_80023B20);

INCLUDE_ASM("asm/nonmatchings/code_13068", func_80023BFC);

INCLUDE_ASM("asm/nonmatchings/code_13068", func_80023D68);

INCLUDE_ASM("asm/nonmatchings/code_13068", func_80023F80);

INCLUDE_ASM("asm/nonmatchings/code_13068", func_80024450);

INCLUDE_ASM("asm/nonmatchings/code_13068", func_80026548);

INCLUDE_ASM("asm/nonmatchings/code_13068", func_8002670C);

INCLUDE_ASM("asm/nonmatchings/code_13068", func_80026848);

INCLUDE_ASM("asm/nonmatchings/code_13068", func_80026A60);

INCLUDE_ASM("asm/nonmatchings/code_13068", func_80026C70);

INCLUDE_ASM("asm/nonmatchings/code_13068", func_80026D9C);

INCLUDE_ASM("asm/nonmatchings/code_13068", func_800272D4);

INCLUDE_ASM("asm/nonmatchings/code_13068", func_80027714);

INCLUDE_ASM("asm/nonmatchings/code_13068", func_800278B0);

INCLUDE_ASM("asm/nonmatchings/code_13068", func_80027A00);

INCLUDE_ASM("asm/nonmatchings/code_13068", func_80027BEC);

INCLUDE_ASM("asm/nonmatchings/code_13068", func_80027D04);

INCLUDE_ASM("asm/nonmatchings/code_13068", func_80027E14);

INCLUDE_ASM("asm/nonmatchings/code_13068", func_80028008);

INCLUDE_ASM("asm/nonmatchings/code_13068", func_800281B8);

INCLUDE_ASM("asm/nonmatchings/code_13068", func_80028260);

INCLUDE_ASM("asm/nonmatchings/code_13068", func_800282F0);

/* MATCHING: non-void with no return keeps the first delay slot a nop. */
s32 func_800283A0(void) {
    s16 v;

    v = sGame.unk3A8;
    if (v < 13) {
        sGame.unk3AC = 0;
    } else if (v >= 26) {
        sGame.unk3AC = 2;
    } else {
        sGame.unk3AC = 1;
    }
}

void func_800283E4(void) {
    sGame.unk6 = 2;
    sGame.unk5 = 0;
    sGame.unk0 = 1;
    sGame.unk38E = 0;
    if (D_8009599C == 1) {
        sGame.unk2 = 0;
        sGame.unk3C8 = 60;
        D_8009599C = 0;
    }
    if ((sGame.unk3D0 & 0xF) == 3) {
        sGame.unk6 = 0x33;
    }
}

INCLUDE_ASM("asm/nonmatchings/code_13068", func_80028448);

void func_800284F8(void) {}

void func_80028500(void) {}

INCLUDE_ASM("asm/nonmatchings/code_13068", func_80028508);

void func_800285B0(void) {
    sGame.unk398 = 0;
    sGame.unk390 = 2;
}

INCLUDE_ASM("asm/nonmatchings/code_13068", func_800285C8);

INCLUDE_ASM("asm/nonmatchings/code_13068", func_80028650);

INCLUDE_ASM("asm/nonmatchings/code_13068", func_800286B0);

INCLUDE_ASM("asm/nonmatchings/code_13068", func_800287C0);

INCLUDE_ASM("asm/nonmatchings/code_13068", func_800287DC);

INCLUDE_ASM("asm/nonmatchings/code_13068", func_800287F4);

INCLUDE_RODATA("asm/nonmatchings/code_13068", D_80010950);

INCLUDE_RODATA("asm/nonmatchings/code_13068", D_80010AE0);

INCLUDE_RODATA("asm/nonmatchings/code_13068", D_80010B10);
