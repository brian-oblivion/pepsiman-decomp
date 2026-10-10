#include "common.h"
#include "libgte.h"
#include "libgpu.h"
#include "libgs.h"
#include "code_a0bc.h"

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
    s32 x; /**< position */
    s32 y; /**< position */
    s32 z; /**< position */
    u8 padC[8];
    s32 radius; /**< summed with the other body's radius */
} Body;

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

/** @brief A point of three words, as the six-point table holds them. */
typedef struct {
    s32 x; /**< position */
    s32 y; /**< position */
    s32 z; /**< position */
} Point12;

extern Point12 D_8009F0D0[];

void func_80028984(void);
s32 func_800297A4(VECTOR *a, VECTOR *b);
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

INCLUDE_ASM("asm/nonmatchings/code_1902c", func_80028984);

INCLUDE_ASM("asm/nonmatchings/code_1902c", func_80028AE4);

/** @brief Finds which of the six table points from `start` is nearest
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

INCLUDE_ASM("asm/nonmatchings/code_1902c", func_80028F0C);

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
