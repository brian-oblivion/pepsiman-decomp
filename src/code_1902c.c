#include "common.h"
#include "libgte.h"
#include "libgpu.h"
#include "libgs.h"
#include "code_a0bc.h"
#include "code_1a098.h"
#include "code_13068.h"
#include "code_7d74.h"
#include "code_1902c.h"

/* MATCHING: retail reaches these through a split lui/%lo pair, so each is
 * an array of unknown size here. */
extern s32 D_8009EF30[];

/** @brief A 72-byte record in the slot table the free-slot search walks;
 *         only the free marker is known. */
typedef struct {
    u8 pad0[0x36];
    s16 unk36; /**< -1 when the record is free */
    u8 pad38[0x48 - 0x38];
} Slot48;

/** @brief A position with a radius further in, as the collision test
 *         reads it. */
typedef struct {
    s32 x;     /**< position */
    s32 y;     /**< position */
    s32 z;     /**< position */
    s16 unkC;  /**< copied to a hit record */
    s16 unkE;  /**< copied to a hit record */
    s16 unk10; /**< copied to a hit record */
    u8 pad12[2];
    s32 radius; /**< summed with the other body's radius */
    u8 unk18;   /**< copied to a hit record */
    u8 unk19;   /**< copied to a hit record */
    u8 pad1A[0x30 - 0x1A];
} Body;

/** @brief A 10-byte hit record, one per player point group. */
typedef struct {
    s8 unk0;  /**< the hit flag; 0 when free */
    u8 unk1;  /**< the nearest point of the group */
    u8 unk2;  /**< from the query */
    u8 unk3;  /**< from the query */
    s16 unk4; /**< from the query */
    s16 unk6; /**< from the query */
    s16 unk8; /**< from the query */
} Hit10;

/* MATCHING: common.h declares the table as bytes; this unit reads its records. */
#define sHits ((Hit10 *)D_8009F0B0)

/** @brief A 0x4C-byte record: four local corners, their world positions
 *         and the coordinate system that places them. */
typedef struct {
    struct {
        s16 vx;
        s16 vy;
        s16 vz;
    } local[4]; /**< corners in the record's own space */

    struct {
        s32 vx;
        s32 vy;
        s32 vz;
    } world[4]; /**< the corners placed in the world */

    GsCOORDINATE2 *coord; /**< places the corners */
} Rec4C;

/** @brief A point of three words, as the placed-point table holds them. */
typedef struct {
    s32 x; /**< position */
    s32 y; /**< position */
    s32 z; /**< position */
} Point12;

extern Point12 D_8009F0D0[];

/** @brief A local position and the index of its coordinate system. */
typedef struct {
    s32 x;    /**< local x */
    s32 y;    /**< local y */
    s32 z;    /**< local z */
    s8 coord; /**< index into the coordinate-system table */
} PointSpec;

/** @brief The 25 local points placed each frame. */
typedef struct {
    PointSpec p[25]; /**< in the order of the placed-point table */
} PointSpec25;

/** @brief Three bytes copied to the stack and never read. */
typedef struct {
    s8 b[3]; /**< 7, 13, 19 */
} Bytes3;

extern PointSpec25 D_80010950;
extern Bytes3 D_800954F0[];

/** @brief Three halfwords, a corner or an edge of a quad. */
typedef struct {
    s16 x; /**< x */
    s16 y; /**< y */
    s16 z; /**< z */
} Short3;

/** @brief A quad's four corners and four edge slots. */
typedef struct {
    Short3 p[8]; /**< corners 0..3, edges 4..7 */
} Quad8;

/** @brief A debug line record, filled and never drawn. */
typedef struct {
    s32 unk0; /**< zeroed */
    s16 x0;   /**< screen x of corner 2 */
    s16 y0;   /**< screen y of corner 2 */
    s16 x1;   /**< screen x of corner 1 */
    s16 y1;   /**< screen y of corner 1 */
    u8 unkC;  /**< 0xFF */
    u8 unkD;  /**< zeroed */
    u8 unkE;  /**< zeroed */
} DebugLine;

/** @brief The player's position, as this unit reads the game state. */
typedef struct {
    u8 pad0[0x348];
    s32 x; /**< position */
    s32 y; /**< position */
    s32 z; /**< position */
} GamePos;

/* MATCHING: a struct lvalue keeps the game state's base in a register. */
#define sGamePos (*(GamePos *)D_8009EB78)

/* MATCHING: code_13068 defines func_800230E0 void; these calls saw an s32 one. */
#define sProject ((s32 (*)(VECTOR *, SVECTOR *))func_800230E0)

extern Quad8 D_80010AE0;
extern Point12 D_80010B10;

void func_80028984(void);
s32 func_8002971C(Body *a, Body *b);
u8 func_80028DBC(u16 start, VECTOR *pos);
void func_8002985C(void);
void func_8002988C(void);

s16 func_8002882C(void) {
    Slot48 *slot = (Slot48 *)D_800959B8;
    u32 i;

    for (i = 0; i < D_800959B4 / sizeof(Slot48); i++) {
        if (slot->unk36 == -1) {
            return i;
        }
        slot++;
    }
    return -1;
}

/** @brief Places the four corners of the record `rec` in the world: each
 *         is rotated by its coordinate system and offset by the world
 *         origin shift in x and z. */
void func_80028888(void *rec) {
    Rec4C *r = rec;
    MATRIX ls;
    MATRIX lw;
    SVECTOR v;
    VECTOR out;
    s32 unused[2]; /* MATCHING: retail leaves 8 bytes between out and flag */
    long flag;
    s16 i;

    GsGetLws(r->coord, &lw, &ls);
    GsSetLsMatrix(&lw);
    for (i = 0; i < 4; i++) {
        v.vx = r->local[i].vx;
        v.vy = r->local[i].vy;
        v.vz = r->local[i].vz;
        RotTrans(&v, &out, &flag);
        r->world[i].vx = out.vx + D_800A7308[0];
        r->world[i].vy = out.vy;
        r->world[i].vz = out.vz + D_800A7308[2];
    }
    GsSetLsMatrix(&ls);
}

/** @brief Places the 25 local points in the world, each in its own
 *         coordinate system, and stores them in the placed-point table. */
/* MATCHING: the two copies are initialisers in retail; see the report. */
void func_80028984(void) {
    s32 unused0[6];
    MATRIX ls;
    MATRIX lw;
    SVECTOR v;
    VECTOR out;
    s32 unused[2];
    Bytes3 b;
    PointSpec25 spec;
    long flag;
    s16 i;

    b = *D_800954F0;
    spec = D_80010950;
    for (i = 0; i < 25; i++) {
        v.vx = spec.p[i].x;
        v.vy = spec.p[i].y;
        v.vz = spec.p[i].z;
        GsGetLws(&D_800D86E0[spec.p[i].coord], &lw, &ls);
        GsSetLsMatrix(&lw);
        RotTrans(&v, &out, &flag);
        D_8009F0D0[i].x = out.vx;
        D_8009F0D0[i].y = out.vy;
        D_8009F0D0[i].z = out.vz;
    }
}

/** @brief Tests the body `q`, shifted by the world origin, against the
 *         player's three bodies, whose radii depend on the stage mode;
 *         a body's first hit records the query in its hit record.
 *  @return 1 if any of the three was hit, else 0 */
/* MATCHING: the early return splits the epilogue from the result's
 * extension, as retail; `h` is an s32 copy of the s8 hit. */
s32 func_80028AE4(Body *q) {
    s32 unused0[4];
    Body a;
    Body b;
    s32 r[3];
    s32 unused1[10];
    s16 i;
    s8 hit;
    s32 h;
    s32 v;

    hit = 0;
    switch (D_8009EF48[0] & 0xF) {
        case 0:
            r[0] = 20;
            v = D_8009EB7E[0];
            r[1] = 35;
            r[2] = 35;
            if (v < 5) {
            } else if (v < 8) {
                r[0] = 0;
            } else if (v < 12) {
                r[2] = 0;
            }
            break;
        case 3:
        default:
            r[0] = 20;
            r[1] = 35;
            r[2] = 35;
            break;
        case 2:
            r[0] = 20;
            r[1] = 35;
            r[2] = 70;
            break;
        case 1:
        case 4:
            r[0] = 20;
            r[1] = 20;
            r[2] = 0;
            break;
    }
    b.x = q->x + D_800A7308[0];
    b.y = q->y;
    b.z = q->z + D_800A7308[2];
    b.radius = q->radius;
    b.unk18 = q->unk18;
    b.unkC = q->unkC;
    b.unkE = q->unkE;
    b.unk10 = q->unk10;
    if ((b.x >= 0 ? b.x : -b.x) > 1000 || (b.z >= 0 ? b.z : -b.z) > 1000) {
        return hit;
    }
    for (i = 0; i < 3; i++) {
        a.x = D_8009F0D0[i].x;
        a.y = D_8009F0D0[i].y;
        a.z = D_8009F0D0[i].z;
        a.radius = r[i];
        hit |= func_8002971C(&a, &b);
        h = hit;
        if (h == 1 && sHits[i].unk0 == 0) {
            sHits[i].unk1 = func_80028DBC(i * 6 + 7, (VECTOR *)&b);
            sHits[i].unk0 = h;
            sHits[i].unk2 = q->unk18;
            sHits[i].unk3 = q->unk19;
            sHits[i].unk4 = q->unkC;
            sHits[i].unk6 = q->unkE;
            sHits[i].unk8 = q->unk10;
        }
    }
    return hit;
}

/** @brief Finds which of six placed points from `start` is nearest
 *         `pos`.
 *  @return the point's offset from `start`, 0..5 */
/* MATCHING: `i++, k++` in the step; `dist[k++]` in the body gives the two
 * increments the other way round. */
u8 func_80028DBC(u16 start, VECTOR *pos) {
    VECTOR a;
    VECTOR b;
    s32 dist[6];
    s16 i;
    u8 k;
    s32 min;

    k = 0;
    for (i = start; i < start + 6; i++, k++) {
        a.vx = D_8009F0D0[i].x;
        a.vy = D_8009F0D0[i].y;
        a.vz = D_8009F0D0[i].z;
        b.vx = pos->vx;
        b.vy = pos->vy;
        b.vz = pos->vz;
        dist[k] = func_800297A4(&a, &b);
    }
    min = dist[0];
    k = 0;
    for (i = 1; i < 6; i++) {
        if (dist[i] < min) {
            min = dist[i];
            k = i;
        }
    }
    return k;
}

/** @brief Tests whether the player stands over the quad `rec` and finds
 *         the height of its plane there.
 *
 *  The player's x and z must lie inside all four edges (corners 0, 1, 3, 2
 *  in turn); the plane through corners 0, 1 and 2 then gives the height.
 *  With the debug flag set, the quad's outline is drawn.
 *  @param out a hit byte, then the height as a word at offset 4 */
/* MATCHING: the volatile store keeps the normal's z, as retail. */
void func_80028F0C(void *rec, s8 *out) {
    Quad8 q;
    Point12 org;
    VECTOR p;
    s32 num;
    VECTOR v;
    Short3 scr[4];
    s32 w[8];
    VECTOR n;
    s32 unused[2];
    DebugLine dbg;
    u16 k;

    q = D_80010AE0;
    org = D_80010B10;
    out[0] = 0;
    *(s32 *)(out + 4) = 0;
    if ((((Rec4C *)rec)->world[0].vx >= 0 ? ((Rec4C *)rec)->world[0].vx
                                          : -((Rec4C *)rec)->world[0].vx) > 2000) {
        return;
    }
    if ((((Rec4C *)rec)->world[0].vz >= 0 ? ((Rec4C *)rec)->world[0].vz
                                          : -((Rec4C *)rec)->world[0].vz) > 2000) {
        return;
    }
    for (k = 0; k < 4; k++) {
        q.p[k].x = ((Rec4C *)rec)->world[k].vx;
        q.p[k].y = ((Rec4C *)rec)->world[k].vy;
        q.p[k].z = ((Rec4C *)rec)->world[k].vz;
    }
    org.x = ((Rec4C *)rec)->coord->coord.t[0] + D_800A7308[0];
    org.y = ((Rec4C *)rec)->coord->coord.t[1];
    org.z = ((Rec4C *)rec)->coord->coord.t[2] + D_800A7308[2];
    if (D_800958F8 == 1) {
        dbg.unk0 = 0;
        dbg.unkC = 0xFF;
        dbg.unkD = 0;
        dbg.unkE = 0;
        v.vx = q.p[1].x + org.x;
        v.vy = q.p[1].y + org.y;
        v.vz = q.p[1].z + org.z;
        sProject(&v, (SVECTOR *)&scr[0]);
        dbg.x1 = scr[0].x;
        dbg.y1 = scr[0].y;
        v.vx = q.p[2].x + org.x;
        v.vy = q.p[2].y + org.y;
        v.vz = q.p[2].z + org.z;
        sProject(&v, (SVECTOR *)&scr[0]);
        dbg.x0 = scr[0].x;
        dbg.y0 = scr[0].y;
        if (D_800958F8 == 1) {
            v.vx = org.x + q.p[0].x;
            v.vy = q.p[0].y;
            v.vz = org.z + q.p[0].z;
            sProject(&v, (SVECTOR *)&scr[0]);
            v.vx = org.x + q.p[1].x;
            v.vy = q.p[1].y;
            v.vz = org.z + q.p[1].z;
            sProject(&v, (SVECTOR *)&scr[1]);
            v.vx = org.x + q.p[3].x;
            v.vy = q.p[3].y;
            v.vz = org.z + q.p[3].z;
            sProject(&v, (SVECTOR *)&scr[2]);
            v.vx = org.x + q.p[2].x;
            v.vy = q.p[2].y;
            v.vz = org.z + q.p[2].z;
            sProject(&v, (SVECTOR *)&scr[3]);
            func_800179F8(0x3E0, scr[0].x, scr[0].y, scr[1].x, scr[1].y, scr[2].x, scr[2].y,
                          scr[3].x, scr[3].y, 0);
        }
    }
    w[1] = -1;
    w[0] = 0;
    p.vx = sGamePos.x + D_800A7308[0];
    p.vy = sGamePos.y;
    p.vz = sGamePos.z + D_800A7308[2];
    w[2] = 0;
    q.p[4].x = 0;
    q.p[4].z = 0;
    q.p[5].x = q.p[1].x - q.p[0].x;
    q.p[5].z = q.p[1].z - q.p[0].z;
    n.vx = q.p[5].z;
    n.vy = 0;
    n.vz = -q.p[5].x;
    w[3] = n.vx * (p.vx - (org.x + q.p[0].x)) + n.vz * (p.vz - (org.z + q.p[0].z));
    if (w[3] < 0) {
        return;
    }
    q.p[5].x = 0;
    q.p[5].z = 0;
    q.p[7].x = q.p[3].x - q.p[1].x;
    q.p[7].z = q.p[3].z - q.p[1].z;
    n.vx = q.p[7].z;
    n.vy = 0;
    n.vz = -q.p[7].x;
    w[4] = n.vx * (p.vx - (org.x + q.p[1].x)) + n.vz * (p.vz - (org.z + q.p[1].z));
    if (w[4] < 0) {
        return;
    }
    q.p[7].x = 0;
    q.p[7].z = 0;
    q.p[6].x = q.p[2].x - q.p[3].x;
    q.p[6].z = q.p[2].z - q.p[3].z;
    n.vx = q.p[6].z;
    n.vy = 0;
    n.vz = -q.p[6].x;
    w[5] = n.vx * (p.vx - (org.x + q.p[3].x)) + n.vz * (p.vz - (org.z + q.p[3].z));
    if (w[5] < 0) {
        return;
    }
    q.p[6].x = 0;
    q.p[6].z = 0;
    q.p[4].x = q.p[0].x - q.p[2].x;
    q.p[4].z = q.p[0].z - q.p[2].z;
    n.vx = q.p[4].z;
    n.vy = 0;
    n.vz = -q.p[4].x;
    w[6] = n.vx * (p.vx - (org.x + q.p[2].x)) + n.vz * (p.vz - (org.z + q.p[2].z));
    if (w[6] < 0) {
        return;
    }

    w[0] = q.p[1].x - q.p[0].x;
    w[1] = q.p[1].y - q.p[0].y;
    w[2] = q.p[1].z - q.p[0].z;
    w[3] = q.p[2].x - q.p[0].x;
    w[4] = q.p[2].y - q.p[0].y;
    w[5] = q.p[2].z - q.p[0].z;
    p.vx = p.vx - (org.x + q.p[0].x);
    p.vy = 0;
    p.vz = p.vz - (org.z + q.p[0].z);
    n.vx = w[1] * w[5] - w[2] * w[4];
    n.vy = w[2] * w[3] - w[0] * w[5];
    num = w[0] * w[4] - w[1] * w[3];
    *(volatile s32 *)&n.vz = num;
    n.vz = -n.vx * p.vx + -num * p.vz - q.p[0].y;
    if (n.vy == 0) {
        n.vy = 1;
    }
    n.pad = n.vz / n.vy + q.p[0].y;
    out[0] = 1;
    *(s32 *)(out + 4) = n.pad;
}

void func_8002964C(VECTOR *pos, u16 scale) {
    SVECTOR size;
    CVECTOR color;
    GsCOORDINATE2 coord;
    MATRIX ls;

    GsInitCoordinate2(WORLD, &coord);
    coord.coord.t[0] = pos->vx;
    coord.coord.t[1] = pos->vy;
    coord.coord.t[2] = pos->vz;
    GsGetLs(&coord, &ls);
    GsSetLsMatrix(&ls);
    size.vx = size.vy = scale * 2;
    color.r = 0;
    color.g = color.b = color.cd = 0x80;
    func_8001A3D4(0x15D, &size, &color, 2, &D_800ACEA8[D_80095750]);
}

s32 func_8002971C(Body *a, Body *b) {
    VECTOR d;
    s32 dist;

    d.vx = (b->x - a->x) * (b->x - a->x);
    d.vy = (b->y - a->y) * (b->y - a->y);
    d.vz = (b->z - a->z) * (b->z - a->z);
    dist = d.vx + d.vy + d.vz;
    d.vx = (b->radius + a->radius) * (b->radius + a->radius);
    return dist < d.vx;
}

s32 func_800297A4(VECTOR *a, VECTOR *b) {
    VECTOR d;

    d.vx = (b->vx - a->vx) * (b->vx - a->vx);
    d.vy = (b->vy - a->vy) * (b->vy - a->vy);
    d.vz = (b->vz - a->vz) * (b->vz - a->vz);
    return d.vx + d.vy + d.vz;
}

void func_8002980C(void) {
    D_8009EB78[1] = 0;
    func_8002985C();
    func_8002988C();
}

void func_80029838(void) {
    s32 *p = (s32 *)D_8009EB78;

    if (p[0x3B8 / 4] == 0) {
        p[0x3BC / 4] = 0;
    }
}

void func_8002985C(void) {
    D_8009F0B0[0] = 0;
    D_8009F0B0[10] = 0;
    D_8009F0B0[20] = 0;
    func_80028984();
}

void func_8002988C(void) {
    D_8009EF30[0] = 0;
}

INCLUDE_RODATA("asm/nonmatchings/code_1902c", D_80010B34);

INCLUDE_RODATA("asm/nonmatchings/code_1902c", D_80010B4C);

INCLUDE_RODATA("asm/nonmatchings/code_1902c", D_80010B58);

INCLUDE_RODATA("asm/nonmatchings/code_1902c", D_80010B64);
