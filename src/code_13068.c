#include "common.h"
#include "libgte.h"
#include "libgpu.h"
#include "libgs.h"

extern u16 D_800957D2;
extern s8 D_8009599C;
extern u8 *D_800958FC;

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
    u8 pad7[0x348 - 0x7];
    s32 unk348; /**< pushed back along the sine of an angle */
    s32 unk34C; /**< raised to a cap: unk3C0, or a global one when unk3B8 is 1 */
    s32 unk350; /**< pushed back along the cosine of an angle */
    u8 pad354[0x38E - 0x354];
    u8 unk38E;  /**< cleared on a reset */
    u8 unk38F;  /**< 1 also gates a check on unk398 */
    s16 unk390; /**< set to 2 together with clearing unk398 */
    u8 pad392[0x398 - 0x392];
    s16 unk398; /**< cleared together with setting unk390 */
    u8 pad39A[0x39C - 0x39A];
    s32 unk39C; /**< with unk3CC, picks which cap unk34C gets */
    u8 pad3A0[0x3A8 - 0x3A0];
    s16 unk3A8; /**< a count that unk3AC grades in three bands */
    u8 pad3AA[0x3AC - 0x3AA];
    s16 unk3AC; /**< 0 below 13 in unk3A8, 1 below 26, else 2 */
    u8 pad3AE[0x3B8 - 0x3AE];
    s32 unk3B8; /**< 1 selects the global cap for unk34C */
    u8 pad3BC[0x3C0 - 0x3BC];
    s32 unk3C0; /**< the usual cap for unk34C */
    u8 pad3C4[0x3C8 - 0x3C4];
    s16 unk3C8; /**< set to 60 on a reset when the flag byte is 1 */
    u8 pad3CA[0x3CC - 0x3CA];
    s32 unk3CC; /**< with unk39C, picks which cap unk34C gets */
    u8 unk3D0;  /**< low nibble read on a reset */
} GameState;

/* MATCHING: a struct lvalue keeps the base in a register; array offsets fold into %lo. */
#define sGame (*(GameState *)D_8009EB78)

/* MATCHING: retail reaches these through a split lui/%lo pair, so each is
 * an array of unknown size here. */
extern s32 D_800D8440[];
extern u8 D_800D8960[];
extern s16 D_800D38DE[];
extern s8 D_8009EF4D[];

extern s32 D_800958A8;
extern s32 D_800958AC;
extern SVECTOR D_800A7680[];
extern s32 D_80095964;
extern s32 D_800957EC;
/* MATCHING: cc1 splits this load (its lui sits in a branch delay slot, away
 * from the lw), so it is an array here, though both halves use one register. */
extern s32 D_8009EF44[];

void func_80015450(u16 *table, s32 index);
void func_80042538(s32 id);

INCLUDE_ASM("asm/nonmatchings/code_13068", func_80022868);

INCLUDE_ASM("asm/nonmatchings/code_13068", func_800229A8);

INCLUDE_ASM("asm/nonmatchings/code_13068", func_80022A74);

void func_80022F58(void) {}

void func_80022F60(void) {}

INCLUDE_ASM("asm/nonmatchings/code_13068", func_80022F68);

INCLUDE_ASM("asm/nonmatchings/code_13068", func_80023020);

void func_800230D8(void) {}

INCLUDE_ASM("asm/nonmatchings/code_13068", func_800230E0);

/* MATCHING: the unused pair puts flag at sp+0x70 and the frame at 0x88. */
void func_80023194(GsCOORDINATE2 *coord, SVECTOR *pos, VECTOR *out) {
    MATRIX world;
    MATRIX local;
    SVECTOR v;
    VECTOR t;
    s32 unused[2];
    long flag;

    v.vx = pos->vx;
    v.vy = pos->vy;
    v.vz = pos->vz;
    GsGetLws(coord, &local, &world);
    GsSetLsMatrix(&local);
    RotTrans(&v, &t, &flag);
    out->vx = t.vx;
    out->vy = t.vy;
    out->vz = t.vz;
    GsSetLsMatrix(&world);
}

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

s32 func_800281B8(GameState *g) {
    s32 ret;

    ret = 0;
    if (g->unk39C <= g->unk3CC && g->unk3B8 == 1) {
        if (g->unk34C >= D_8009EF44[0]) {
            g->unk34C = D_8009EF44[0];
            ret = 2;
            func_80042538(30);
        }
    } else if (g->unk34C >= g->unk3C0) {
        g->unk34C = g->unk3C0;
        ret = 1;
        func_80042538(30);
    }
    if (ret != 0) {
        func_80015450(D_800734AC, 6);
    }
    return ret;
}

s32 func_80028260(s32 n) {
    D_800958A8 += n;
    if (D_800958AC == 1) {
        if (D_800958A8 >= 2) {
            D_800958A8 = 2;
        }
    } else if (D_800958A8 >= 3) {
        D_800958A8 = 3;
    }
    if (D_800958A8 <= 0) {
        D_800958A8 = 0;
    }
    if (D_8009EF4D[0] == 1) {
        D_800958A8 = 0;
    }
    return D_800958A8;
}

/* MATCHING: the local pointer lets angle and the sGame base share $s0. */
void func_800282F0(s32 deg, s32 dist) {
    s32 angle;
    SVECTOR *rot;

    rot = D_800A7680;
    angle = ANGLE_DEG(deg);
    sGame.unk348 -= rsin(rot->vy + angle) * dist >> FIX12_SHIFT;
    sGame.unk350 -= rcos(rot->vy + angle) * dist >> FIX12_SHIFT;
}

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

/* MATCHING: `one` is the 1 retail keeps in $a0 for both compares. */
void func_80028448(void) {
    s32 one;

    if (D_800958E8 != 0 && D_800958E8 % 10 == 0) {
        one = 1;
        if (D_800958AC == one) {
            if (D_800958A8 == 2) {
                goto skip;
            }
        } else if (D_800958A8 == 3) {
            goto skip;
        }
        if (D_8009EF4D[0] != one) {
            func_80042538(34);
        }
    skip:
        func_80028260(1);
    }
}

void func_800284F8(void) {}

void func_80028500(void) {}

/* MATCHING: non-void with no return keeps the second bnez's delay slot a nop. */
s32 func_80028508(void) {
    if (sGame.unk0 != 0 && (sGame.unk6 < 5 || sGame.unk6 >= 8) &&
        (sGame.unk38F != 1 || sGame.unk398 < 6) && !(D_80095964 & 0x1000) && (D_800957EC & 0x40)) {
        func_80042538(22);
        sGame.unk6 = 5;
    }
}

void func_800285B0(void) {
    sGame.unk398 = 0;
    sGame.unk390 = 2;
}

void func_800285C8(s32 deg, s32 radius, VECTOR *out) {
    s32 angle;

    angle = ANGLE_DEG(deg);
    out->vx = rsin(angle) * radius >> FIX12_SHIFT;
    out->vz = rcos(angle) * radius >> FIX12_SHIFT;
}

void func_80028650(void) {
    s32 i;

    i = ((u32)D_80095864 >> 8) & 3;
    SetFogNearFar(3000, 8000, 250);
    SetFarColor(D_800958FC[i * 4], D_800958FC[i * 4 + 1], D_800958FC[i * 4 + 2]);
}

INCLUDE_ASM("asm/nonmatchings/code_13068", func_800286B0);

void func_800287C0(void) {
    D_8009EF20[0] = 0;
    D_800957D2 = 0x280;
}

void func_800287DC(void) {
    D_8009EF20[0] = 0;
    D_800957D2 = 0;
}

void func_800287F4(void) {
    if ((D_8009EF48[0] >> 4) == 5) {
        D_800D8440[0] = (s32)D_800D8960;
        D_800D38DE[0] = 1;
    }
}

INCLUDE_RODATA("asm/nonmatchings/code_13068", D_80010950);

INCLUDE_RODATA("asm/nonmatchings/code_13068", D_80010AE0);

INCLUDE_RODATA("asm/nonmatchings/code_13068", D_80010B10);
