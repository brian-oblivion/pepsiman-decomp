#include "common.h"
#include "libgte.h"
#include "libgpu.h"
#include "libgs.h"

typedef struct {
    u8 b[8];
} Bytes8;

extern s32 D_80095A78;
extern s32 D_80095A7C;
extern s32 D_80095A80;
extern s32 D_80095A90;
extern s32 D_80095A94;
extern s32 D_80095A98;
extern s8 D_80095A88[8];
extern s8 D_80095AA0[8];
extern u8 D_800956B0[];
extern u8 D_800956B8[];

void func_80042538(s32 arg0);

INCLUDE_ASM("asm/nonmatchings/code_29f54", func_80039754);

INCLUDE_ASM("asm/nonmatchings/code_29f54", func_800399A8);

INCLUDE_ASM("asm/nonmatchings/code_29f54", func_80039C3C);

INCLUDE_ASM("asm/nonmatchings/code_29f54", func_8003A008);

INCLUDE_ASM("asm/nonmatchings/code_29f54", func_8003A20C);

INCLUDE_ASM("asm/nonmatchings/code_29f54", func_8003A3F4);

INCLUDE_ASM("asm/nonmatchings/code_29f54", func_8003A4B4);

INCLUDE_ASM("asm/nonmatchings/code_29f54", func_8003A84C);

INCLUDE_ASM("asm/nonmatchings/code_29f54", func_8003AFAC);

INCLUDE_ASM("asm/nonmatchings/code_29f54", func_8003B780);

INCLUDE_ASM("asm/nonmatchings/code_29f54", func_8003B9B4);

INCLUDE_ASM("asm/nonmatchings/code_29f54", func_8003BDF4);

INCLUDE_ASM("asm/nonmatchings/code_29f54", func_8003C014);

INCLUDE_ASM("asm/nonmatchings/code_29f54", func_8003C17C);

INCLUDE_ASM("asm/nonmatchings/code_29f54", func_8003C2E8);

INCLUDE_ASM("asm/nonmatchings/code_29f54", func_8003C494);

INCLUDE_ASM("asm/nonmatchings/code_29f54", func_8003C8D0);

INCLUDE_ASM("asm/nonmatchings/code_29f54", func_8003CC94);

INCLUDE_ASM("asm/nonmatchings/code_29f54", func_8003D960);

INCLUDE_ASM("asm/nonmatchings/code_29f54", func_8003DE34);

INCLUDE_ASM("asm/nonmatchings/code_29f54", func_8003DFB8);

INCLUDE_ASM("asm/nonmatchings/code_29f54", func_8003E07C);

INCLUDE_ASM("asm/nonmatchings/code_29f54", func_8003E13C);

void func_8003E1FC(s16 *out, s16 x0, s16 y0, s16 x1, s16 y1) {
    s32 dx;
    s32 dy;
    s32 t;
    s32 d;

    dx = x1 - x0;
    dy = y1 - y0;
    t = -(x0 * dx + y0 * dy);
    d = dx * dx + dy * dy;
    out[0] = t * dx / d;
    out[1] = t * dy / d;
}

INCLUDE_ASM("asm/nonmatchings/code_29f54", func_8003E29C);

INCLUDE_ASM("asm/nonmatchings/code_29f54", func_8003E360);

void func_8003E40C(void) {
    D_80095A78 = 0;
    D_80095A94 = 1;
    func_80042538(0x19);
}

s32 func_8003E438(void) {
    return D_80095A94;
}

void func_8003E444(void) {
    s32 i;
    Bytes8 buf;

    buf = *(Bytes8 *)D_800956B0;
    D_80095A98 = 0;
    D_80095A90 = 0;
    D_80095A80 = 1;
    D_80095A7C = 0;
    for (i = 0; i < 8; i++) {
        D_80095AA0[i] = buf.b[i];
        D_80095A88[i] = 0;
    }
}

void func_8003E4C4(void) {
    s32 i;
    Bytes8 buf;

    buf = *(Bytes8 *)D_800956B8;
    D_80095A98 = 0;
    D_80095A90 = 0;
    D_80095A80 = 1;
    D_80095A7C = 0;
    for (i = 0; i < 8; i++) {
        D_80095AA0[i] = buf.b[i];
        D_80095A88[i] = 0;
    }
}

s32 func_8003E544(void) {
    return D_80095A80;
}

INCLUDE_ASM("asm/nonmatchings/code_29f54", func_8003E550);

INCLUDE_ASM("asm/nonmatchings/code_29f54", func_8003E6F8);

INCLUDE_ASM("asm/nonmatchings/code_29f54", func_8003EA04);

INCLUDE_ASM("asm/nonmatchings/code_29f54", func_8003EC04);

INCLUDE_ASM("asm/nonmatchings/code_29f54", func_8003EF40);

INCLUDE_ASM("asm/nonmatchings/code_29f54", func_8003F100);

INCLUDE_ASM("asm/nonmatchings/code_29f54", func_8003F488);

INCLUDE_ASM("asm/nonmatchings/code_29f54", func_8003F664);

INCLUDE_ASM("asm/nonmatchings/code_29f54", func_8003F834);

void func_8003F8D4(s16 x, s16 y, s16 z) {
    GsCOORDINATE2 coord;
    MATRIX ls;

    GsInitCoordinate2(WORLD, &coord);
    coord.coord.t[0] = x;
    coord.coord.t[1] = y;
    coord.coord.t[2] = z;
    coord.flg = 0;
    GsGetLs(&coord, &ls);
    GsSetLsMatrix(&ls);
}

INCLUDE_ASM("asm/nonmatchings/code_29f54", func_8003F960);

INCLUDE_ASM("asm/nonmatchings/code_29f54", func_8003FA88);

INCLUDE_ASM("asm/nonmatchings/code_29f54", func_8003FBE0);

INCLUDE_ASM("asm/nonmatchings/code_29f54", func_8003FD0C);

INCLUDE_ASM("asm/nonmatchings/code_29f54", func_8003FE5C);

INCLUDE_ASM("asm/nonmatchings/code_29f54", func_8003FFAC);
