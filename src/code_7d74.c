#include "common.h"
#include "rand.h"
#include "libgte.h"
#include "libgpu.h"
#include "libgs.h"
#include "libcd.h"
#include "code_7d74.h"
#include "code_a0bc.h"
#include "code_1a098.h"

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

/* MATCHING: reading the TimInfo through the global each time, not a
 * pointer local, picks retail's registers. */
#define sTim ((TimInfo *)D_800956D4)

/** @brief A position as three words. */
typedef struct {
    s32 x; /**< x */
    s32 y; /**< y */
    s32 z; /**< z */
} Vec3i;

extern s8 D_800956D1;     /**< set to 1 when the level is applied */
extern s16 D_80095918;    /**< cleared when the viewer starts */
extern char D_80010404[]; /**< the motion-number format */
/* MATCHING: a per-unit view; code_1a098 defines it on its own floor
 * types, and this unit passes D_800958B4's word as the data. */
s32 func_800299D8(void *out, s32 index, Vec3i *pos, s32 data);
/* MATCHING: this unit passes the coordinates unextended, so it saw s32
 * parameters (code_29f54 defines them as s16). */
void func_8003A3F4(s32 *index, s32 x, s32 y);
extern CdlLOC D_80095728;

/** @brief The three flat lights. */
typedef struct {
    GsF_LIGHT l[3]; /**< lights 0 to 2 */
} FlatLights;

/* MATCHING: a struct lvalue keeps the base in one register; common.h
 * declares the table as words. */
#define sLights ((*(FlatLights *)D_800DD070).l)

/** @brief The head of the game state, seen as a two-channel animation
 *         player. */
typedef struct {
    u8 unk0[6];      /**< not yet known */
    u8 want[2];      /**< the sequence each channel is asked to play */
    u8 cur[2];       /**< the sequence each channel is playing */
    s16 time[2];     /**< each channel's clock */
    u8 unkE[0xA];    /**< not yet known */
    u8 *data[2];     /**< each channel's position in its sequence */
    s32 len[100];    /**< each sequence's length */
    s32 start[100];  /**< each sequence's start time */
    u8 unk340[8];    /**< not yet known */
    s32 unk348;      /**< cleared when the viewer starts */
    s32 unk34C;      /**< cleared when the viewer starts */
    s32 unk350;      /**< cleared when the viewer starts */
    u8 unk354[0x2C]; /**< not yet known */
    s32 unk380;      /**< the model's starting y rotation */
    s32 unk384;      /**< the model's starting z rotation */
} Player;

/* MATCHING: a struct lvalue keeps the base in a register. */
#define sPlayer (*(Player *)D_8009EB78)

/* MATCHING: each caller declares its own view of the state it passes;
 * this is the defining unit's. */
u8 func_80017F0C(Player *obj, u16 index, s8 arg);

void func_80017DD4(void);
void func_8001819C(void);
void func_80018BD8(void);
s8 func_80017640(u16 *tim);
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

void func_80017B38(void) {
    /* MATCHING: an unused local gives retail's 0x20-byte frame. */
    s32 unused[2];

    switch (D_80095760) {
        case 0:
            D_8009575C = 0x10;
            D_80095754 = 0x20;
            D_8009574C = 0x80;
            D_800DB2A0[0] = 0;
            D_800DB2A0[1] = 0;
            D_800DB2A0[2] = -1000;
            D_800DB2A0[3] = 0;
            D_800DB2A0[4] = 0;
            D_800DB2A0[5] = 0;
            D_80095914 = -90;
            sPlayer.unk380 = 0;
            sPlayer.unk384 = 0;
            sPlayer.unk348 = 0;
            sPlayer.unk34C = 0;
            sPlayer.unk350 = 0;
            D_800A7308[0] = 0;
            D_80095918 = 0;
            D_800A7308[2] = 0;
            D_800D86E0[0].coord.t[0] = 0;
            D_800D86E0[0].coord.t[1] = 0;
            D_800D86E0[0].coord.t[2] = 0;
            D_8009EAB8[0] = 0;
            D_8009EAB8[1] = sPlayer.unk380;
            D_8009EAB8[2] = sPlayer.unk384;
            func_80018AE0((SVECTOR *)D_8009EAB8, D_800D86E0);
            sPlayer.want[0] = 0;
            func_80017F0C(&sPlayer, 0, 0);
            D_80095760++;
            break;
        case 1:
            if (D_800956D1 == 1) {
                D_800956D1 = func_80017F0C(&sPlayer, 0, 0);
            } else {
                D_8009EB7E[0] = 0;
            }
            func_8001819C();
            func_80018BD8();
            func_80017DD4();
            if (D_80095964 & 0x2000) {
                D_800D86E0[0].coord.t[0]++;
            }
            if (D_80095964 & 0x8000) {
                D_800D86E0[0].coord.t[0]--;
            }
            if (D_80095964 & 0x4000) {
                D_800D86E0[0].coord.t[1]++;
            }
            if (D_80095964 & 0x1000) {
                D_800D86E0[0].coord.t[1]--;
            }
            if (D_80095964 & 1) {
                D_800D86E0[0].coord.t[2]--;
            }
            if (D_80095964 & 2) {
                D_800D86E0[0].coord.t[2]++;
            }
            break;
    }
    FntPrint(D_80010404, D_800956D0);
}

void func_80017DD4(void) {
    D_800DB2A0[0] = rcos((D_80095914 << 12) / 360) * 400 / 4096;
    D_800DB2A0[1] = -100;
    D_800DB2A0[2] = rsin((D_80095914 << 12) / 360) * 400 / 4096;
    D_800DB2A0[6] = 0;
    D_800DB2A0[3] = 0;
    D_800DB2A0[4] = -100;
    D_800DB2A0[5] = 0;
    if (D_80095964 & 4) {
        D_80095914 += 2;
    }
    if (D_80095964 & 8) {
        D_80095914 -= 2;
    }
}

extern u8 D_800760EC[];

#ifdef NON_MATCHING
u8 func_80017F0C(Player *obj, u16 index, s8 arg) {
    u8 ret;
    s32 seq;

    ret = 1;
    if (obj->want[0] == 1) {
        return 0;
    }
    if (obj->want[index] != obj->cur[index]) {
        obj->time[index] = obj->start[obj->want[index]];
        obj->data[index] = (u8 *)D_800D81B0[obj->want[index]];
        obj->cur[index] = obj->want[index];
    }
    seq = obj->cur[index];
    if (obj->time[index] >= obj->len[seq] + obj->start[seq]) {
        if (seq < 2) {
            return 0;
        }
        if (seq < 5) {
            ret = 2;
            obj->time[index] = obj->start[seq];
            obj->data[index] = (u8 *)D_800D81B0[obj->cur[index]];
        } else if ((s16)seq == 26) {
            ret = 2;
            obj->time[index] = obj->start[seq];
            obj->data[index] = (u8 *)D_800D81B0[obj->cur[index]];
        } else {
            return 0;
        }
    } else {
        obj->time[index]++;
    }
    obj->data[index] = func_800195CC(obj->time[index], obj->data[index], (s32)D_800D8360,
                                     (s32)D_800760EC, D_80095904, 0, arg);
    return ret;
}
#else
INCLUDE_ASM("asm/nonmatchings/code_7d74", func_80017F0C);
#endif

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

s32 func_80018D70(void *pos, void *arg, s32 cur) {
    Vec3i p;

    p.x = ((Vec3i *)pos)->x;
    p.y = ((Vec3i *)pos)->y;
    p.z = ((Vec3i *)pos)->z;
    func_8003A3F4(&cur, p.x, p.z);
    if (func_800299D8(arg, cur, &p, D_800958B4) == 0x7FFF) {
        return -1;
    }
    return cur;
}

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
