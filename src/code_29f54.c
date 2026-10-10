#include "common.h"
#include "libgte.h"
#include "libgpu.h"
#include "libetc.h"
#include "libgs.h"
#include "code_a0bc.h"
#include "code_7d74.h"
#include "spad.h"

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
    s16 unk0;  /**< animation frame; cleared when the record is taken */
    s16 unk2;  /**< frames to wait before the animation starts */
    s16 unk4;  /**< per-frame step of the rising sprite's y offset */
    s16 unk6;  /**< the rising sprite's end distance */
    s16 unk8;  /**< per-frame step subtracted from the y offset */
    s16 unkA;  /**< per-frame turn of a spinning piece, in degrees */
    s16 unkC;  /**< x offset added to the drawing position */
    s16 unkE;  /**< y offset added to the drawing position */
    s16 unk10; /**< z offset added to the drawing position */
    u8 unk12;  /**< not yet known */
    u8 next;   /**< free-list link: the next record's index */
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

/** @brief A run of entries in a draw list: where it starts and how long. */
typedef struct {
    s16 start; /**< the first entry's index */
    s16 count; /**< the number of entries */
} Run;

extern Run *D_80095934;
extern Run *D_80095938;
extern s8 *D_8009593C;
extern s8 *D_80095940;

extern DR_STP D_800DFA90;
extern DR_ENV D_800DFA10;
extern DR_STP D_800DFAA0;
extern DR_ENV D_800DFA50;

void func_80039754(s32 clip) {
    DRAWENV env;
    /* MATCHING: retail's frame has 8 more bytes above the environment. */
    s32 unused[2];

    if (clip) {
        GetDrawEnv(&env);
        env.clip.x = 0;
        env.clip.y = D_800E474C * 240 + 32;
        env.clip.w = 319;
        env.clip.h = 175;
        SetDrawStp(&D_800DFA90, 1);
        addPrim(D_800A7318[D_80095750].org + 0xFFF, &D_800DFA90);
        SetDrawEnv(&D_800DFA10, &env);
        addPrim(D_800A7318[D_80095750].org + 0xFFF, &D_800DFA10);
        env.clip.x = 0;
        env.clip.y = D_800E474C * 240;
        env.clip.w = 319;
        env.clip.h = 239;
        SetDrawStp(&D_800DFAA0, 1);
        addPrim(D_800ACEA8[D_80095750].org, &D_800DFAA0);
        SetDrawEnv(&D_800DFA50, &env);
        addPrim(D_800ACEA8[D_80095750].org, &D_800DFA50);
    }
}

INCLUDE_ASM("asm/nonmatchings/code_29f54", func_800399A8);

INCLUDE_ASM("asm/nonmatchings/code_29f54", func_80039C3C);

void func_8003A008(s16 x, s16 y, s16 z) {
    s32 i;
    s32 k;
    SVECTOR rot;
    MATRIX ls;

    D_800A72B8.coord.t[0] = x;
    D_800A72B8.coord.t[1] = y;
    D_800A72B8.coord.t[2] = z;
    rot.vx = rot.vy = rot.vz = 0;
    func_80018AE0(&rot, &D_800A72B8);
    GsGetLs(&D_800A72B8, &ls);
    GsSetLsMatrix(&ls);
    SetSpadStack();
    for (i = 0; i < D_80095934[D_8009578C].count; i++) {
        k = D_8009593C[D_80095934[D_8009578C].start + i];
        if ((u32)k < D_80095794) {
            GsSortObject4J(&D_800AC868[k], D_80095884, 2, (u_long *)0x1F800000);
        }
    }
    for (i = 0; i < D_80095938[D_8009578C].count; i++) {
        k = D_80095940[D_80095938[D_8009578C].start + i];
        if ((u32)k < D_80095798 && k != -1) {
            GsSortObject4J(&D_800ACB88[k], D_80095884, 2, (u_long *)0x1F800000);
        }
    }
    ResetSpadStack();
}

void func_8003A20C(s32 pos, s32 unused, s32 z, s32 range) {
    s32 i;
    s32 k;
    s32 d;
    s32 r;
    SVECTOR rot;
    MATRIX ls;

    for (i = -1; i < 2; i++) {
        k = (((pos + 10000000) / 5000 + i) & 1) + 1;
        d = pos - i * 5000;
        if (d >= 0) {
            if (d >= 5000) {
                continue;
            }
            k = 0;
        }
        if (d < -range) {
            if (d < -(range + 5000)) {
                continue;
            }
            k = 3;
        }
        if (pos >= 0) {
            r = pos % 5000;
        } else {
            r = (pos + 10000000) % 5000;
        }
        D_800A72B8.coord.t[0] = r + (i - 1) * 5000;
        D_800A72B8.coord.t[1] = 0;
        D_800A72B8.coord.t[2] = z;
        rot.vx = rot.vy = rot.vz = 0;
        func_80018AE0(&rot, &D_800A72B8);
        GsGetLs(&D_800A72B8, &ls);
        GsSetLsMatrix(&ls);
        GsSortObject4J(&D_800AC868[k], D_80095884, 2, (u_long *)0x1F800000);
        GsSortObject4J(&D_800ACB88[k], D_80095884, 2, (u_long *)0x1F800000);
    }
}

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

void func_8003BDF4(void) {
    POLY_F4 *p;
    s32 i;

    for (i = 0; i < D_80095968; i++) {
        if (D_800A7278[i] != 0xFF) {
            p = (POLY_F4 *)D_800E48D0;
            setPolyF4(p);
            p->x0 = p->x2 = -160;
            p->x1 = p->x3 = -152;
            p->y0 = p->y1 = (D_800A7278[i] >> 1) - 120;
            if (i != 0) {
                p->y3 = (D_800A7278[i - 1] >> 1) - 120;
            } else {
                p->y3 = -120;
            }
            p->y2 = p->y3;
            p->r0 = D_800AC848[i];
            p->g0 = D_800A7888[i];
            p->b0 = D_800A76E8[i];
            addPrim(D_80095884->org, p);
            p++;
            D_800E48D0 = (u8 *)p;
        }
    }
    p = (POLY_F4 *)D_800E48D0;
    setPolyF4(p);
    p->x0 = p->x2 = -160;
    p->x1 = p->x3 = -144;
    p->y0 = p->y1 = 8;
    p->y2 = p->y3 = 9;
    setRGB0(p, 0xFF, 0xFF, 0xFF);
    D_80095968 = 0;
    addPrim(D_80095884->org, p);
    p++;
    D_800E48D0 = (u8 *)p;
}

void func_8003C014(void) {
    RECT rect;
    SVECTOR pos;
    CVECTOR color;

    func_8001B2F4(0x1FE, 2, 0xA0, 0xF0, 10, 0, 0, 0, 0);
    func_8001B2F4(0x1FF, 2, 0xA0, 0xF0, 12, 0x20, 0, 0, 0);
    rect.x = 0;
    rect.y = D_800E474C * 240;
    rect.w = 320;
    rect.h = 240;
    MoveImage(&rect, 640, 0);
    color.r = 0;
    color.g = color.b = color.cd = 0x80;
    pos.vx = -160;
    pos.vy = -120;
    func_8001B354(0x1FE, &pos, &color, 0, &D_800ACEA8[D_80095750]);
    pos.vx = 0;
    pos.vy = -120;
    func_8001B354(0x1FF, &pos, &color, 0, &D_800ACEA8[D_80095750]);
}

void func_8003C17C(u8 level) {
    RECT rect;
    SVECTOR pos;
    CVECTOR color;

    func_8001B2F4(0x1FE, 2, 0xA0, 0xF0, 10, 0, 0, 0, 0);
    func_8001B2F4(0x1FF, 2, 0xA0, 0xF0, 12, 0x20, 0, 0, 0);
    rect.x = 0;
    rect.y = D_800E474C * 240;
    rect.w = 320;
    rect.h = 240;
    MoveImage(&rect, 640, 0);
    /* MATCHING: an s8 store gives li -1; a u_char one gives li 0xFF. */
    *(s8 *)&color.r = -1;
    color.g = color.b = color.cd = level;
    pos.vx = -160;
    pos.vy = -120;
    func_8001B354(0x1FE, &pos, &color, 0xFFF, &D_800A7318[D_80095750]);
    pos.vx = 0;
    pos.vy = -120;
    func_8001B354(0x1FF, &pos, &color, 0xFFF, &D_800A7318[D_80095750]);
}

void func_8003C2E8(void) {
    POLY_FT4 *p;
    s32 n;

    if (D_80095A94 != 0) {
        if (D_80095A78 >= 24) {
            D_80095A94 = 0;
            return;
        }
        n = D_80095A78 / 3;
        p = (POLY_FT4 *)D_800E48D0;
        setPolyFT4(p);
        p->r0 = p->g0 = p->b0 = 0x80;
        p->x0 = p->x2 = -160;
        p->x1 = p->x3 = 160;
        p->y0 = p->y1 = -120;
        p->y2 = p->y3 = 120;
        p->u0 = p->u2 = (n % 2) << 7;
        p->v0 = p->v1 = (n / 2 % 2) * 96;
        p->clut = getClut(720, n + 448);
        p->u1 = p->u3 = p->u0 + 0x7F;
        p->v2 = p->v3 = p->v0 + 0x5F;
        p->tpage = getTPage(0, 0, n / 4 * 64 + 704, 256);
        addPrim(D_800ACEA8[D_80095750].org, p);
        D_80095A78++;
        p++;
        D_800E48D0 = (u8 *)p;
    }
}

INCLUDE_ASM("asm/nonmatchings/code_29f54", func_8003C494);

INCLUDE_ASM("asm/nonmatchings/code_29f54", func_8003C8D0);

INCLUDE_ASM("asm/nonmatchings/code_29f54", func_8003CC94);

INCLUDE_ASM("asm/nonmatchings/code_29f54", func_8003D960);

/** @brief Sixteen signed bytes, copied together as one unaligned block. */
typedef struct {
    s8 b[4][4]; /**< four rows of four */
} Bytes16;

extern Bytes16 D_80011EB0;
extern s8 D_80095908;
extern s32 D_800957B4;

void func_8003DE34(void) {
    Bytes16 tbl;
    s32 k;
    u32 a;
    u32 b;

    /* MATCHING: the row index in a local, indexed per use; a row pointer
     * moves the address add above the first andi. */
    tbl = D_80011EB0;
    a = D_80095970;
    b = D_80095964;
    k = D_80095908;
    D_800957EC = a;
    D_800957B4 = b;
    D_800957EC = a & ~0xE0;
    D_800957B4 = b & ~0xE0;
    D_800957EC |= ((a & 0x20) >> 5) << (tbl.b[k][0] + 5);
    D_800957EC |= ((a & 0x40) >> 6) << (tbl.b[k][1] + 5);
    D_800957EC |= ((a & 0x80) >> 7) << (tbl.b[k][2] + 5);
    D_800957B4 |= ((b & 0x20) >> 5) << (tbl.b[k][0] + 5);
    D_800957B4 |= ((b & 0x40) >> 6) << (tbl.b[k][1] + 5);
    D_800957B4 |= ((b & 0x80) >> 7) << (tbl.b[k][2] + 5);
}

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

s32 func_8003E550(Slot *p, GsOT *ot, s16 x, s16 y, s16 z) {
    SVECTOR size;
    CVECTOR color;

    if (p->unk2 == 0) {
        setLs(x + p->unkC, y + p->unkE, z + p->unk10);
        /* MATCHING: retail shifts the scaled sine with srl. */
        size.vx = (u32)(rcos((p->unk0 * 3 << 13) / 360) * 25) >> 9;
        size.vy = (u32)(rsin((p->unk0 << 15) / 360) * 25) >> 8;
        color.r = 1;
        color.g = color.b = color.cd = 0x80;
        func_8001A69C(0x11F, &size, &color, 2, ot);
        return ++p->unk0 == 15;
    }
    p->unk2--;
    return 0;
}

s32 func_8003E6F8(Slot *p, GsOT *ot, s16 x, s16 y, s16 z) {
    s16 c[2];
    POLY_F4 *f;
    DR_TPAGE *t;

    c[0] = p->unkC + (p->unk10 >> 1) + 2;
    c[1] = p->unkE + 4;
    f = (POLY_F4 *)D_800E48D0;
    setPolyF4(f);
    setSemiTrans(f, 1);
    f->x0 = p->unkC;
    f->x1 = p->unkC + p->unk10;
    f->x2 = f->x0 + 4;
    f->x3 = f->x1 + 4;
    f->y0 = f->y1 = p->unkE;
    f->y2 = f->y3 = p->unkE + 8;
    f->x0 = c[0] + ((c[0] - f->x0) * ((p->unk0 << 9) + 0x1000) >> 12);
    f->x1 = c[0] + ((c[0] - f->x1) * ((p->unk0 << 9) + 0x1000) >> 12);
    f->x2 = c[0] + ((c[0] - f->x2) * ((p->unk0 << 9) + 0x1000) >> 12);
    f->x3 = c[0] + ((c[0] - f->x3) * ((p->unk0 << 9) + 0x1000) >> 12);
    f->y0 = c[1] + ((c[1] - f->y0) * ((p->unk0 << 10) + 0x1000) >> 12);
    f->y1 = c[1] + ((c[1] - f->y1) * ((p->unk0 << 10) + 0x1000) >> 12);
    f->y2 = c[1] + ((c[1] - f->y2) * ((p->unk0 << 10) + 0x1000) >> 12);
    f->y3 = c[1] + ((c[1] - f->y3) * ((p->unk0 << 10) + 0x1000) >> 12);
    f->r0 = f->g0 = f->b0 = ~(p->unk0 << 5);
    addPrim(ot->org, f);
    f++;
    D_800E48D0 = (u8 *)f;
    t = (DR_TPAGE *)D_800E48D0;
    setDrawTPage(t, 1, 1, getTPage(0, 1, 0, 0));
    addPrim(ot->org, t);
    t++;
    D_800E48D0 = (u8 *)t;
    return ++p->unk0 == 8;
}

/* MATCHING: code_a0bc takes a Sprite2D *, a type local to that unit; this
 * unit passes the same eight halfwords as an array. */
void func_800198BC(u16 id, s16 *quad, CVECTOR *color, u16 otz, GsOT *ot);

s32 func_8003EA04(Slot *p, GsOT *ot, s16 x, s16 y, s16 z) {
    s16 q[8];
    CVECTOR color;
    s32 unused[2];

    if (p->unk2 == 0) {
        if (p->unk0 == 0) {
            setLs(x, y, z);
            RotTransPers((SVECTOR *)&p->unkC, (long *)q, NULL, NULL);
            p->unkC = q[0];
            p->unkE = q[1];
        }
        q[0] = q[2] = p->unkC + ((-136 - p->unkC) * p->unk0 >> 3);
        q[1] = q[3] = p->unkE + ((-88 - p->unkE) * p->unk0 >> 3);
        q[4] = q[5] = 0x1000 - (p->unk0 << 8);
        q[6] = p->unk0 << 8;
        q[7] = 1;
        color.r = 0;
        color.g = color.b = color.cd = ~(p->unk0 << 4);
        func_800198BC(0xFA, q, &color, 0, &D_800ACEA8[D_80095750]);
        if (++p->unk0 == 8) {
            D_800958E8++;
            func_80028448();
            return 1;
        }
        return 0;
    }
    p->unk2--;
    return 0;
}

s32 func_8003EC04(Slot *p, GsOT *ot, s16 x, s16 y, s16 z) {
    s16 q[8];
    CVECTOR color;
    s32 unused[2];
    s32 i;

    if (p->unk2 == 0) {
        if (p->unk0 == 0) {
            setLs(x, y, z);
            RotTransPers((SVECTOR *)&p->unkC, (long *)q, NULL, NULL);
            p->unkC = q[0];
            p->unkE = q[1];
        }
        for (i = 0; i < 5; i++) {
            q[0] = p->unkC + ((-136 - p->unkC) * p->unk0 >> 3);
            q[1] = p->unkE + ((-88 - p->unkE) * p->unk0 >> 3);
            q[0] = q[2] = q[0] + rsin(((p->unk0 * 20 + i * 72) << 12) / 360) * (64 - p->unk0 * 8) / 4096;
            q[1] = q[3] = q[1] + rcos(((p->unk0 * 20 + i * 72) << 12) / 360) * (64 - p->unk0 * 8) / 4096;
            q[4] = q[5] = 0x1000 - (p->unk0 << 8);
            q[6] = -(((p->unk0 * 20 + i * 72 + 90) << 12) / 360);
            q[7] = 1;
            color.r = 0;
            color.g = color.b = color.cd = ~(p->unk0 << 4);
            func_800198BC(0xFA, q, &color, 0, &D_800ACEA8[D_80095750]);
        }
        if (++p->unk0 == 8) {
            D_800958E8 += 5;
            func_80028448();
            return 1;
        }
        return 0;
    }
    p->unk2--;
    return 0;
}

/** @brief A CVECTOR whose first byte is signed (-1 means "no tint"). */
typedef struct {
    s8 r;  /**< red, or -1 */
    u8 g;  /**< green */
    u8 b;  /**< blue */
    u8 cd; /**< code byte */
} SColor;

s32 func_8003EF40(Slot *p) {
    SVECTOR pos;
    SColor color;

    if (p->unk2 == 0) {
        if (p->unk0 < 16) {
            pos.vx = -72 - (16 - p->unk0) * 15;
            pos.vy = -12;
            color.r = -1;
            color.g = color.b = color.cd = 0x80;
            func_8001B354(0x136, &pos, (CVECTOR *)&color, 0, &D_800ACEA8[D_80095750]);
        } else if (p->unk0 < 32) {
            pos.vx = -72;
            pos.vy = -12;
            color.r = -1;
            color.g = 0x80;
            color.b = color.cd = (8 - (p->unk0 - 16) % 8) * 16;
            func_8001B354(0x136, &pos, (CVECTOR *)&color, 0, &D_800ACEA8[D_80095750]);
        } else {
            pos.vx = (p->unk0 - 32) * 15 - 72;
            pos.vy = -12;
            color.r = -1;
            color.g = 0x80;
            color.b = color.cd = 0;
            func_8001B354(0x136, &pos, (CVECTOR *)&color, 0, &D_800ACEA8[D_80095750]);
        }
        return ++p->unk0 == 48;
    }
    p->unk2--;
    return 0;
}

INCLUDE_ASM("asm/nonmatchings/code_29f54", func_8003F100);

s32 func_8003F488(Slot *p, GsOT *ot, s16 x, s16 y, s16 z) {
    SVECTOR size;
    CVECTOR color;

    if (p->unk2 == 0) {
        setLs(x + p->unkC, y + p->unkE, z + p->unk10);
        size.vx = p->unk0 * 10 + 100;
        size.vy = p->unk0 * 10 + 100;
        color.r = 1;
        color.g = color.b = color.cd = 0x80 - (p->unk0 << 2);
        func_8001A3D4(0x12D, &size, &color, 2, ot);
        if (p->unk6 != 0) {
            p->unk6--;
        }
        p->unkC += rsin(p->unk4) * p->unk6 / 4096;
        p->unk10 += rcos(p->unk4) * p->unk6 / 4096;
        p->unkE -= p->unk8;
        return ++p->unk0 == 32;
    }
    p->unk2--;
    return 0;
}

/** @brief One step of a slot's animation; returns nonzero when it ends. */
typedef s32 (*SlotStep)(Slot *p, GsOT *ot, s16 x, s16 y, s16 z);

extern SlotStep D_8007A7C4[];

void func_8003F664(s32 unused, s16 x, s16 y, s16 z) {
    u8 i;
    u8 prev;
    u8 next;
    s32 d;
    s32 done;
    Slot *slot;

    prev = 0xFF;
    if ((s8)D_80095AA8 < 0) {
        return;
    }
    i = D_80095AA8;
    do {
        slot = &sSlots[i];
        d = x + slot->unkC;
        next = slot->next;
        if (d * (d * 2) > 0x77A0F) {
            done = D_8007A7C4[slot->unk12](slot, &D_800A7318[D_80095750], x, y, z);
        } else {
            done = D_8007A7C4[slot->unk12](slot, &D_800ACEA8[D_80095750], x, y, z);
        }
        if (done) {
            if (prev == 0xFF) {
                D_80095AA8 = sSlots[i].next;
            } else {
                sSlots[prev].next = sSlots[i].next;
            }
            sSlots[i].next = D_80095AA9;
            D_80095AA9 = i;
        } else {
            prev = i;
        }
        if ((s8)next < 0) {
            break;
        }
        i = next;
    } while (1);
}

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

s32 func_8003FE5C(Slot *p, GsOT *ot, s16 x, s16 y, s16 z) {
    SVECTOR size;
    CVECTOR color;

    if (p->unk2 == 0) {
        setLs(x + p->unkC, y + p->unkE, z + p->unk10);
        size.vx = p->unk0 * 10 + 100;
        size.vy = p->unk0 * 10 + 100;
        color.r = 2;
        color.g = color.b = color.cd = 0x80 - (p->unk0 << 4);
        func_8001A3D4(0x12D, &size, &color, 2, ot);
        return ++p->unk0 == 8;
    }
    p->unk2--;
    return 0;
}

s32 func_8003FFAC(Slot *p, GsOT *ot, s16 x, s16 y, s16 z) {
    SVECTOR size;
    CVECTOR color;

    if (p->unk2 == 0) {
        setLs(x + p->unkC, y + p->unkE + p->unk4 * p->unk0, z + p->unk10);
        size.vy = size.vx = 100;
        color.r = 0;
        color.g = color.b = color.cd = 0x80;
        func_8001A3D4(0x134, &size, &color, 2, ot);
        return ++p->unk0 * p->unk4 >= p->unk6;
    }
    p->unk2--;
    return 0;
}
