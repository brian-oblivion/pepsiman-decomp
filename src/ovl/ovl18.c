#include "common.h"
#include "libgte.h"
#include "libgpu.h"
#include "libgs.h"
#include "model.h"
#include "draw.h"

extern GsCOORDINATE2 D_800F1E90;
extern GsDOBJ2 D_800F2A50;
extern VECTOR D_800F2A60;
extern SVECTOR D_800F2A70;

INCLUDE_ASM("asm/ovl/ovl18/nonmatchings/ovl18", func_800F00C0);

INCLUDE_ASM("asm/ovl/ovl18/nonmatchings/ovl18", func_800F01B8);

INCLUDE_RODATA("asm/ovl/ovl18/nonmatchings/ovl18", D_800F0000);

INCLUDE_ASM("asm/ovl/ovl18/nonmatchings/ovl18", func_800F03BC);

INCLUDE_ASM("asm/ovl/ovl18/nonmatchings/ovl18", func_800F11D4);

void func_800F1318(void) {
    MATRIX m;

    D_800F1E90.coord.t[0] = D_800F2A60.vx;
    D_800F1E90.coord.t[1] = D_800F2A60.vy;
    D_800F1E90.coord.t[2] = D_800F2A60.vz;
    SetCoordRotation(&D_800F2A70, &D_800F1E90);
    GsGetLs(D_800F2A50.coord2, &m);
    GsSetLsMatrix(&m);
    GsGetLw(D_800F2A50.coord2, &m);
    GsSetLightMatrix(&m);
    GsSortObject4(&D_800F2A50, &gSubOt[gActiveBuff], 2, (u_long *)0x1F800000);
}
