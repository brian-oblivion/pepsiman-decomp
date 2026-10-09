#include "common.h"
#include "libgte.h"
#include "libgpu.h"
#include "libetc.h"
#include "libgs.h"
#include "code_a0bc.h"

/** @brief Eight bytes, copied together as one unaligned block. */
typedef struct {
    u8 b[8]; /**< the bytes */
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

/** @brief The 0x14-byte records of common.h's NumberedSlot table, as this
 *         unit writes them. */
typedef struct {
    s16 unk0;   /**< cleared when the record is taken */
    s16 unk2;   /**< not yet known */
    u8 unk4[8]; /**< not yet known */
    s16 unkC;   /**< not yet known */
    s16 unkE;   /**< not yet known */
    s16 unk10;  /**< not yet known */
    u8 unk12;   /**< not yet known */
    u8 next;    /**< free-list link: the next record's index */
} Slot;

#define sSlots ((Slot *)D_800DFAB0)

/** @brief An 8-byte boundary record: a point and the direction its sign
 *         test projects onto. */
typedef struct {
    s16 x;  /**< the point's x */
    s16 y;  /**< the point's y */
    s16 dx; /**< the direction's x */
    s16 dy; /**< the direction's y */
} Edge;

extern Edge *D_800958D4;

extern GsDOBJ2 D_800AC868[];
extern u32 D_80095798;
extern GsDOBJ2 D_800ACB88[];
extern GsCOORDINATE2 D_800A72B8;

extern s32 D_80095968;
extern s32 D_800A7278[];
extern u8 D_800AC848[];
extern u8 D_800A7888[];
extern u8 D_800A76E8[];

INCLUDE_ASM("asm/nonmatchings/code_29f54", func_80039754);

INCLUDE_ASM("asm/nonmatchings/code_29f54", func_800399A8);

INCLUDE_ASM("asm/nonmatchings/code_29f54", func_80039C3C);

INCLUDE_ASM("asm/nonmatchings/code_29f54", func_8003A008);

INCLUDE_ASM("asm/nonmatchings/code_29f54", func_8003A20C);

void func_8003A3F4(s32 *index, s16 x, s16 y) {
    s32 i;
    s32 j;
    s32 d;
    Edge *e;
    Edge *f;

    /* MATCHING: integer address sums put the scaled index first; pointer
     * arithmetic puts the base first and changes the register choice. */
    i = *index;
    e = (Edge *)(i * 8 + (s32)D_800958D4) + 1;
    d = e->dx * (x - e->x) + e->dy * (y - e->y);
    if (d >= 0) {
        *index = i + 1;
    }
    j = *index;
    f = (Edge *)(j * 8 + (s32)D_800958D4);
    d = -f->dx * (x - f->x) + -f->dy * (y - f->y);
    if (d >= 0) {
        *index = j - 1;
    }
}

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

void func_8003DFB8(unsigned long *tmd) {
    u32 i;
    GsDOBJ2 *obj;

    tmd++;
    GsMapModelingData(tmd);
    tmd++;
    D_80095794 = *tmd;
    tmd++;
    for (i = 0; i < D_80095794; i++) {
        GsLinkObject4((unsigned long)tmd, &D_800AC868[i], i);
    }
    obj = D_800AC868;
    for (i = 0; i < D_80095794; i++) {
        obj->coord2 = &D_800A72B8;
        obj->attribute = 0x200;
        obj++;
    }
}

void func_8003E07C(unsigned long *tmd) {
    u32 i;
    GsDOBJ2 *obj;

    tmd++;
    GsMapModelingData(tmd);
    tmd++;
    D_80095798 = *tmd;
    tmd++;
    for (i = 0; i < D_80095798; i++) {
        GsLinkObject4((unsigned long)tmd, &D_800ACB88[i], i);
    }
    obj = D_800ACB88;
    for (i = 0; i < D_80095798; i++) {
        obj->coord2 = &D_800A72B8;
        obj->attribute = 0;
        obj++;
    }
}

void func_8003E13C(unsigned long *p) {
    RECT rect;
    GsIMAGE tim;

    while (*p == 0x10) {
        GsGetTimInfo(p + 1, &tim);
        rect.x = tim.cx;
        rect.y = tim.cy;
        rect.w = tim.cw;
        rect.h = tim.ch;
        LoadImage(&rect, (u_long *)tim.clut);
        rect.x = tim.px;
        rect.y = tim.py;
        rect.w = tim.pw;
        rect.h = tim.ph;
        LoadImage(&rect, (u_long *)tim.pixel);
        p += 2;
        p += *p >> 2;
        p += *p >> 2;
    }
}

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

void func_8003E29C(s32 a, s32 b, s32 c) {
    if (D_80095968 < 16) {
        D_800A7278[D_80095968] = VSync(1);
        D_800AC848[D_80095968] = a;
        D_800A7888[D_80095968] = b;
        D_800A76E8[D_80095968] = c;
        D_80095968++;
    }
}

void func_8003E360(s32 level) {
    s32 i;
    s32 v;
    s32 base;

    struct {
        SVECTOR a;
        SVECTOR b;
        CVECTOR c;
    } st;

    if (level > 0x80) {
        level = 0x80;
    }
    for (i = 8; i >= 0; i--) {
        base = i * 3 + 0x40;
        v = base - level;
        if (v < 0) {
            v = 0;
        }
        st.a.vy = st.a.pad = st.a.vx = st.a.vz = 0;
        st.b.vx = st.b.vy = v * 32 + 0x1000;
        st.b.vz = (v << 12) / 360 * 3;
        st.b.pad = 1;
        st.c.r = 1;
        st.c.g = st.c.b = st.c.cd = 0x10;
    }
}

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

s32 func_8003F834(s32 arg0, s16 arg1, s16 arg2, s16 arg3, s16 arg4) {
    u8 prev;
    Slot *slot;

    if ((s8)D_80095AA9 < 0) {
        return -1;
    }
    prev = D_80095AA8;
    D_80095AA8 = D_80095AA9;
    D_80095AA9 = sSlots[D_80095AA8].next;
    sSlots[D_80095AA8].next = prev;
    sSlots[D_80095AA8].unk12 = arg0;
    slot = &sSlots[D_80095AA8];
    slot->unkC = arg1;
    slot->unkE = arg2;
    slot->unk10 = arg3;
    slot->unk0 = 0;
    slot->unk2 = arg4;
    return 0;
}

/* MATCHING: inlined, so each call rebuilds the two struct addresses. */
/**
 * @brief Sets the GS local-screen matrix to a pure translation (x, y, z);
 *        the sprite steps below expand it in place.
 * @param x translation x
 * @param y translation y
 * @param z translation z
 */
static __inline__ void setLs(s16 x, s16 y, s16 z) {
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

s32 func_8003F960(Slot *p, GsOT *ot, s16 x, s16 y, s16 z) {
    SVECTOR size;
    CVECTOR color;

    if (p->unk2 == 0) {
        setLs(x + p->unkC, y + p->unkE, z + p->unk10);
        size.vy = size.vx = 0x96;
        color.r = 1;
        color.g = color.b = color.cd = 0x80;
        func_8001A3D4((u16)(p->unk0 + 0x11A), &size, &color, 2, ot);
        return ++p->unk0 == 8;
    }
    p->unk2--;
    return 0;
}

s32 func_8003FA88(Slot *p, GsOT *ot, s16 x, s16 y, s16 z) {
    SVECTOR size;
    CVECTOR color;

    if (p->unk2 == 0) {
        setLs(x + p->unkC, y + p->unkE, z + p->unk10);
        size.vx = p->unk0 + 0x14;
        size.vy = p->unk0 + 0x14;
        color.r = 1;
        color.g = color.b = color.cd = 0x80 - (p->unk0 << 2);
        func_8001A3D4(0x11F, &size, &color, 2, ot);
        p->unkE -= 4;
        p->unk10 += p->unk0 >> 2;
        return ++p->unk0 == 0x20;
    }
    p->unk2--;
    return 0;
}

s32 func_8003FBE0(Slot *p, GsOT *ot, s16 x, s16 y, s16 z) {
    SVECTOR size;
    CVECTOR color;

    if (p->unk2 == 0) {
        setLs(x + p->unkC, y + p->unkE, z + p->unk10);
        size.vy = size.vx = 0x32;
        color.r = 1;
        color.g = color.b = color.cd = 0x80 - (p->unk0 << 4);
        func_8001A3D4(0x11A, &size, &color, 2, ot);
        return ++p->unk0 == 8;
    }
    p->unk2--;
    return 0;
}

s32 func_8003FD0C(Slot *p, GsOT *ot, s16 x, s16 y, s16 z) {
    SVECTOR size;
    CVECTOR color;

    if (p->unk2 == 0) {
        setLs(x + p->unkC, y + p->unkE, z + p->unk10);
        size.vx = p->unk0 * 10 + 100;
        size.vy = p->unk0 * 10 + 100;
        color.r = 1;
        color.g = color.b = color.cd = 0x80 - (p->unk0 << 4);
        func_8001A3D4(0x12D, &size, &color, 2, ot);
        return ++p->unk0 == 8;
    }
    p->unk2--;
    return 0;
}

INCLUDE_ASM("asm/nonmatchings/code_29f54", func_8003FE5C);

INCLUDE_ASM("asm/nonmatchings/code_29f54", func_8003FFAC);
