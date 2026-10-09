#include "common.h"
#include "rand.h"
#include "libgte.h"
#include "libgpu.h"
#include "libgs.h"
#include "libcd.h"
#include "code_a0bc.h"

/** @brief A 16-byte entry of a pack's directory; the first entry's count is
 *         the number of entries. */
typedef struct {
    s32 offset;   /**< byte offset of the entry's data from the pack start */
    u8 unk4[0xA]; /**< not yet known */
    u16 count;    /**< number of entries (read from the first one) */
} PackEntry;

/** @brief A slot of a SlotList, found by its key. */
typedef struct {
    s32 unk0; /**< only the sign bit set when the slot is initialised */
    s32 unk4; /**< the slot's 0x50-byte object */
    s32 unk8; /**< cleared when the slot is initialised */
    s32 unkC; /**< the key; -1 when free */
} Slot;

/** @brief A fixed-capacity list of Slots. */
typedef struct {
    Slot *slots;  /**< the slot array */
    s32 count;    /**< slots in use */
    s32 capacity; /**< slots in the array */
} SlotList;

/** @brief A TIM image's header, unpacked: where its CLUT and pixels go. */
typedef struct {
    s16 mode;      /**< pixel mode, the low 3 bits of the TIM flags */
    s16 hasClut;   /**< 1 when the TIM carries a CLUT */
    u32 *clut;     /**< the CLUT's pixel data */
    RECT clutRect; /**< where the CLUT goes in VRAM */
    u32 *pixel;    /**< the image's pixel data */
    RECT pixRect;  /**< where the image goes in VRAM */
    s16 unk1C;     /**< the image width scaled by the pixel mode */
    s16 unk1E;     /**< 4, 2 or not set, by pixel mode */
} TimInfo;

/* MATCHING: reading the TimInfo through the global each time, not a
 * pointer local, picks retail's registers. */
#define sTim ((TimInfo *)D_800956D4)

/** @brief A position as three words. */
typedef struct {
    s32 x; /**< x */
    s32 y; /**< y */
    s32 z; /**< z */
} Vec3i;

extern u8 D_800958C9;
extern s32 D_800958B4;
s32 func_800299D8(s32 *out, s32 index, Vec3i *pos, s32 data);
extern CdlLOC D_80095728;
extern MATRIX D_800E4858;

/** @brief The three flat lights. */
typedef struct {
    GsF_LIGHT l[3]; /**< lights 0 to 2 */
} FlatLights;

/* MATCHING: a struct lvalue keeps the base in one register; common.h
 * declares the table as words. */
#define sLights ((*(FlatLights *)D_800DD070).l)

s32 func_80017774(void *data);
s8 func_80017640(u16 *tim);
s32 func_800175AC(u8 com);
u8 *func_80018DF0(u8 *data, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s8 arg5);

void func_80017574(void) {
    if (D_800958C9 != 0) {
        CdControlF(CdlPause, 0);
        D_800958C9 = 0;
    }
}

s32 func_800175AC(u8 com) {
    CdIntToPos(D_80095720, &D_80095728);
    while (CdControl(com, (u_char *)&D_80095728, 0) == 0) {
    }
    D_80095714 = D_80095720;
    D_8009571C++;
    return 0;
}

void func_80017614(u8 mode) {
    if (mode == 4) {
        func_800175AC(CdlPlay);
    }
}

s8 func_80017640(u16 *tim) {
    u16 *p;
    u16 *start;
    s32 len;
    s32 c;

    p = tim;
    if (*p != 0x10) {
        return -1;
    }
    p += 2;
    sTim->mode = *p & 7;
    /* MATCHING: the masked flag through a local keeps cc1's store-flag
     * code from turning the test into a shift. */
    c = *p & 8;
    sTim->hasClut = c != 0;
    if (sTim->hasClut) {
        p += 2;
        start = p;
        len = *p;
        p += 2;
        sTim->clutRect.x = *p++;
        sTim->clutRect.y = *p++;
        sTim->clutRect.w = *p++;
        sTim->clutRect.h = *p++;
        sTim->clut = (u32 *)p;
        p = (u16 *)((u8 *)start + len);
    } else {
        p += 2;
    }
    p += 2;
    sTim->pixRect.x = *p++;
    sTim->pixRect.y = *p++;
    sTim->pixRect.w = *p++;
    sTim->pixRect.h = *p++;
    sTim->pixel = (u32 *)p;
    switch (sTim->mode) {
        case 0:
            sTim->unk1E = 4;
            /* MATCHING: a multiply, not a shift, loads the width with lh. */
            sTim->unk1C = sTim->pixRect.w * 4;
            break;
        case 1:
            sTim->unk1E = 2;
            sTim->unk1C = sTim->pixRect.w * 2;
            break;
        default:
            sTim->unk1C = sTim->pixRect.w;
            break;
    }
    return 0;
}

s32 func_80017774(void *data) {
    RECT rect;
    RECT clut;
    TimInfo *info;
    u16 w;
    u16 h;

    if (func_80017640(data) == -1) {
        return -1;
    }
    w = sTim->pixRect.w;
    h = sTim->pixRect.h;
    rect.x = sTim->pixRect.x;
    rect.y = sTim->pixRect.y;
    rect.w = sTim->pixRect.w;
    rect.h = sTim->pixRect.h;
    LoadImage(&rect, sTim->pixel);
    info = sTim;
    if (info->hasClut == 1) {
        clut.x = info->clutRect.x;
        clut.y = info->clutRect.y;
        clut.w = info->clutRect.w;
        clut.h = info->clutRect.h;
        LoadImage(&clut, info->clut);
    }
    return (s16)w * (s16)h * sTim->unk1E + (s32)sTim->pixel;
}

/* MATCHING: s32 with no value returned keeps the return register live,
 * so cc1 leaves the two forward branch delay slots empty. */
s32 func_80017880(s16 px, s16 py, s16 cx, s16 cy, u16 *tim) {
    RECT rect;
    RECT clut;
    TimInfo *info;
    TimInfo *clutInfo;

    if (func_80017640(tim) == -1) {
        return;
    }
    info = (TimInfo *)D_800956D4;
    if (px != -1) {
        rect.x = px;
        rect.y = py;
        rect.w = info->pixRect.w;
        rect.h = info->pixRect.h;
        LoadImage(&rect, info->pixel);
    }
    if (cx != -1) {
        clutInfo = (TimInfo *)D_800956D4;
        if (clutInfo->hasClut == 1) {
            clut.x = cx;
            clut.y = cy;
            clut.w = clutInfo->clutRect.w;
            clut.h = clutInfo->clutRect.h;
            LoadImage(&clut, clutInfo->clut);
        }
    }
}

void func_8001797C(PackEntry *pack) {
    PackEntry *e;
    u16 i;
    u16 n;

    e = pack;
    n = pack->count;
    for (i = 0; i < n; i++) {
        func_80017774((u8 *)pack + e->offset);
        e++;
        DrawSync(0);
    }
}

/* MATCHING: the red channel through its own local moves the colour's copy
 * first in the prologue, as retail. */
void func_800179F8(u16 col, s16 x0, s16 y0, s16 x1, s16 y1, s16 x2, s16 y2, s16 x3, s16 y3, u16 pri) {
    POLY_F4 poly;
    POLY_F4 *p;
    s32 r;

    p = &poly;
    SetPolyF4(p);
    SetSemiTrans(p, 1);
    r = col & 0x1F;
    p->r0 = r << 3;
    p->g0 = ((col >> 5) & 0x1F) << 3;
    p->b0 = ((col >> 10) & 0x1F) << 3;
    p->x0 = x0;
    p->y0 = y0;
    p->x1 = x1;
    p->y1 = y1;
    p->x2 = x3;
    p->y2 = y3;
    p->x3 = x2;
    p->y3 = y2;
    GsSortPoly(p, &D_800ACEA8[D_80095750], pri);
}

s32 func_80017B18(void) {
    return rand();
}

INCLUDE_ASM("asm/nonmatchings/code_7d74", func_80017B38);

INCLUDE_ASM("asm/nonmatchings/code_7d74", func_80017DD4);

INCLUDE_ASM("asm/nonmatchings/code_7d74", func_80017F0C);

void func_80018094(void) {
    GsFOGPARAM fog;

    sLights[0].vx = 0;
    sLights[0].vy = 100;
    sLights[0].vz = 100;
    GsSetFlatLight(0, &sLights[0]);
    sLights[1].vx = 86;
    sLights[1].vy = 100;
    sLights[1].vz = -50;
    GsSetFlatLight(1, &sLights[1]);
    sLights[2].vx = -86;
    sLights[2].vy = 100;
    sLights[2].vz = -50;
    sLights[0].r = 0xD0;
    sLights[0].g = 0xD0;
    sLights[0].b = 0xD0;
    sLights[1].r = 0xD0;
    sLights[1].g = 0xD0;
    sLights[1].b = 0xD0;
    sLights[2].r = 0xD0;
    sLights[2].g = 0xD0;
    sLights[2].b = 0xD0;
    GsSetFlatLight(2, &sLights[2]);
    GsSetAmbient(0x400, 0x400, 0x400);
    GsSetLightMode(0);
    fog.dqa = -0x3200;
    fog.dqb = 0x1400000;
    fog.rfc = D_8009575C;
    fog.gfc = D_80095754;
    fog.bfc = D_8009574C;
    GsSetFogParam(&fog);
}

INCLUDE_ASM("asm/nonmatchings/code_7d74", func_8001819C);

s32 func_800183B0(Vec3i *pos) {
    Vec3i p;
    s32 out[4];
    u32 i;
    s32 found;
    u8 *blob;
    u8 *e;
    s32 verts;
    s16 *v;
    s32 dx;
    s32 dz;
    s32 d;

    p.x = pos->x;
    p.y = pos->y;
    p.z = pos->z;
    found = -1;
    for (i = 0; i < D_80095794; i++) {
        blob = (u8 *)D_800958B4;
        /* MATCHING: integer sums, offset first, and the vertex base in
         * its own statement give retail's operand and load order. */
        e = (u8 *)(*(s32 *)(blob + i * 8 + 8) + (s32)blob);
        verts = *(s32 *)(blob + 4) + (s32)blob;
        v = (s16 *)(*(u16 *)(e + 4) * 8 + verts);
        dx = v[0] - p.x;
        dz = v[2] - p.z;
        d = dx * dx + dz * dz;
        if ((d < 0 ? -d : d) > 100000000) {
            continue;
        }
        if (func_800299D8(out, i, &p, D_800958B4) != 0x7FFF) {
            found = i;
            break;
        }
    }
    return found;
}

INCLUDE_ASM("asm/nonmatchings/code_7d74", func_800184BC);

void func_80018AE0(SVECTOR *rot, GsCOORDINATE2 *coord) {
    MATRIX m;
    SVECTOR r;

    m = D_800E4858;
    m.t[0] = coord->coord.t[0];
    m.t[1] = coord->coord.t[1];
    m.t[2] = coord->coord.t[2];
    r = *rot;
    RotMatrixYXZ(&r, &m);
    coord->coord = m;
    coord->flg = 0;
}

/** @brief The head of the game state as this function sees it. */
typedef struct {
    u8 unk0[6]; /**< not yet known */
    u8 unk6;    /**< takes the level value when flag 0x20 is set */
    u8 unk7;    /**< not yet known */
    u8 unk8;    /**< set to 0xFF when flag 0x20 is set */
} LevelHead;

extern s8 D_800956D0; /**< a level, kept within 0..120 */
extern u8 D_800956D1; /**< set to 1 when the level is applied */

void func_80018BD8(void) {
    s32 flags = D_80095970;

    if (flags & 0x10) {
        D_800956D0++;
    }
    if (flags & 0x80) {
        D_800956D0--;
    }
    if (D_800956D0 < 0) {
        D_800956D0 = 0;
    }
    if (D_800956D0 >= 0x79) {
        D_800956D0 = 0x78;
    }
    if (flags & 0x20) {
        (*(LevelHead *)D_8009EB78).unk8 = 0xFF;
        D_800956D1 = 1;
        (*(LevelHead *)D_8009EB78).unk6 = D_800956D0;
    }
}

void func_80018CA4(void) {}

void func_80018CAC(void) {}

void func_80018CB4(void) {
    GsSetProjection(250);
    D_800DB2A0[0] = 0;
    D_800DB2A0[1] = 0;
    D_800DB2A0[2] = 4000;
    D_800DB2A0[3] = 0;
    D_800DB2A0[4] = 0;
    D_800DB2A0[5] = 0;
    D_800DB2A0[6] = 0;
    D_800DB2A0[7] = 0;
    GsSetRefView2((GsRVIEW2 *)D_800DB2A0);
}

s16 func_80018D04(s16 from, s16 to, u16 step, u16 steps) {
    s32 v;

    if (step == steps) {
        return to;
    }
    /* MATCHING: one local carried through compound steps keeps every
     * stage of the arithmetic in one register. */
    v = (to - from) << 16;
    v /= steps;
    v *= step;
    v /= 0x10000;
    return from + v;
}

INCLUDE_ASM("asm/nonmatchings/code_7d74", func_80018D70);

void func_80018DE8(void) {}

INCLUDE_ASM("asm/nonmatchings/code_7d74", func_80018DF0);

u8 *func_800195CC(u32 time, u8 *data, s32 arg2, s32 arg3, s32 arg4, s32 arg5, s8 arg6) {
    u32 i;
    u32 n;

    n = *(u16 *)(data + 2);
    if (time < *(u32 *)(data + 4)) {
        return data;
    }
    data += 8;
    for (i = 0; i < n; i++) {
        data = func_80018DF0(data, arg2, arg3, arg4, arg5, arg6);
    }
    return data;
}

void func_80019684(SlotList *list, Slot *slots, u8 *objs, s32 data, s32 n) {
    s32 i;

    list->count = 0;
    list->slots = slots;
    list->capacity = n;
    for (i = 0; i < n; i++) {
        slots->unk0 = 0x80000000;
        slots->unkC = -1;
        slots->unk4 = (s32)objs;
        *(s32 *)(objs + 0x44) = data;
        slots->unk8 = 0;
        slots++;
        objs += 0x50;
        data += 0x28;
    }
}

Slot *func_800196E4(SlotList *list, s32 key) {
    Slot *s;
    s32 i;

    s = list->slots;
    for (i = 0; i < list->count; i++) {
        if (key == s->unkC) {
            break;
        }
        s++;
    }
    if (i == list->count) {
        return NULL;
    }
    return s;
}

Slot *func_80019730(SlotList *list, s32 key) {
    Slot *s;
    s32 i;

    s = list->slots;
    for (i = 0; i < list->count; i++) {
        if (s->unkC == -1) {
            break;
        }
        s++;
    }
    if (i < list->count) {
        s->unkC = key;
        s->unk0 = 0;
        GsInitCoordinate2(NULL, (GsCOORDINATE2 *)s->unk4);
        s->unk8 = 0;
        return s;
    }
    if (i < list->capacity) {
        s = &list->slots[list->count];
        list->count++;
        s->unkC = key;
        s->unk0 = 0;
        GsInitCoordinate2(NULL, (GsCOORDINATE2 *)s->unk4);
        s->unk8 = 0;
        return s;
    }
    return NULL;
}

Slot *func_800197E4(SlotList *list) {
    Slot *s;
    s32 i;

    s = list->slots;
    for (i = 0; i < list->count; i++) {
        if (s->unkC == -1) {
            break;
        }
        s++;
    }
    if (i < list->count) {
        s->unkC = -1;
        if (i == list->count - 1) {
            do {
                list->count--;
                s--;
            } while (s->unkC == -1);
        }
        return s;
    }
    return NULL;
}

s32 *func_80019874(s32 *table, s32 *keys, s32 key) {
    s32 n;

    table++;
    n = *table++;
    while (n > 0) {
        if (key == *keys) {
            break;
        }
        table += 7;
        n--;
        keys++;
    }
    if (n == 0) {
        return NULL;
    }
    return table;
}

INCLUDE_RODATA("asm/nonmatchings/code_7d74", D_80010454);
