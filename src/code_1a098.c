#include "common.h"
#include "memory.h"
#include "libapi.h"
#include "libgte.h"
#include "sys/file.h"
#include "libgpu.h"
#include "libgs.h"
#include "code_1a098.h"

/** @brief An object whose current position and halfword triple are reset
 *         from a stored copy. */
typedef struct {
    s32 unk0;    /**< current; reset from unkC */
    s32 unk4;    /**< current; reset from unk10 */
    s32 unk8;    /**< current; reset from unk14 */
    s32 unkC;    /**< stored */
    s32 unk10;   /**< stored */
    s32 unk14;   /**< stored */
    u16 unk18;   /**< current; reset from unk1E */
    u16 unk1A;   /**< current; reset from unk20 */
    u16 unk1C;   /**< current; reset from unk22 */
    u16 unk1E;   /**< stored */
    u16 unk20;   /**< stored */
    u16 unk22;   /**< stored */
    u8 unk24[4]; /**< not yet known */
    s32 unk28;   /**< passed to the lookup and updated from it */
    s32 unk2C;   /**< zeroed on a reset; an angle from the lookup */
    s32 unk30;   /**< zeroed on a reset; an angle from the lookup */
} Obj34;

/** @brief An object with a word at 0x28 that a lookup updates. */
typedef struct {
    u8 unk0[0x28]; /**< not yet known */
    s32 unk28;     /**< passed to the lookup and updated from it */
} Obj2C;

/** @brief Three words, a position. */
typedef struct {
    s32 x; /**< x */
    s32 y; /**< y */
    s32 z; /**< z */
} Vec3;

/** @brief The 0x30-byte argument block of a position query. */
typedef struct {
    Vec3 pos;       /**< the position asked about */
    u8 unkC[8];     /**< not set by the range-50 caller */
    s32 unk14;      /**< 50 from the range-50 caller (a range?) */
    u8 unk18;       /**< 0 from the range-50 caller */
    u8 unk19[0x17]; /**< not set by the range-50 caller */
} Query30;

/** @brief Eight bytes of four halfwords, copied whole. */
typedef struct {
    s16 v[4]; /**< not yet known */
} Quad16;

/** @brief Four halfword triples after a word: the corners of a box. */
typedef struct {
    u8 unk0[4];  /**< not yet known */
    s16 v[4][3]; /**< x, y, z of each corner */
} Box4;

/** @brief A 0x2C-byte record placed relative to a coordinate system. */
typedef struct {
    VECTOR pos;           /**< world position, written from the local one */
    u8 unk10[0xC];        /**< not yet known */
    s32 lx;               /**< local x */
    s32 ly;               /**< local y */
    s32 lz;               /**< local z */
    GsCOORDINATE2 *coord; /**< the coordinate system lx..lz are in */
} Placed2C;

/** @brief A local position and the index of its coordinate system. */
typedef struct {
    s32 x;    /**< local x */
    s32 y;    /**< local y */
    s32 z;    /**< local z */
    s8 coord; /**< index into the coordinate-system table */
} LocalPos;

/** @brief A point of a path with the direction of its segment. */
typedef struct {
    s16 x;  /**< x of the point */
    s16 z;  /**< z of the point */
    s16 dx; /**< x of the direction */
    s16 dz; /**< z of the direction */
} PathPt;

/** @brief Something that follows a path; only x, z and the segment are
 *         known. */
typedef struct {
    s16 x;         /**< x */
    u8 unk2[6];    /**< not yet known */
    s16 z;         /**< z */
    u8 unkA[0x32]; /**< not yet known */
    s32 seg;       /**< the current path segment */
} PathUser;

/** @brief A model object with its own coordinate system and transform. */
typedef struct {
    GsDOBJ2 obj;         /**< the object handler */
    GsCOORDINATE2 coord; /**< the object's coordinate system */
    SVECTOR rot;         /**< rotation */
    SVECTOR scale;       /**< scale, 0x1000 = 1 */
} Model70;

/** @brief The head of the game state, as far as this unit reaches. */
typedef struct {
    u8 unk0[5];      /**< not yet known */
    u8 unk5;         /**< a mode byte: 0x42 and 0x43 seen */
    u8 unk6[0x1A];   /**< not yet known */
    s32 unk20[100];  /**< first word of each loaded entry */
    s32 unk1B0[100]; /**< third word of each loaded entry */
} GameHead;

/** @brief A 16-byte directory entry of a loaded file. */
typedef struct {
    s32 offset;   /**< byte offset of the entry from the directory */
    u8 unk4[0xA]; /**< not yet known */
    u16 count;    /**< entry count; read from the first entry only */
} DirEnt16;

/** @brief A 0x3C-byte record of a 100-entry table (code_1dc24 has the same
 *         record, with fewer fields known). */
typedef struct {
    u16 unk0;       /**< a counter; bumped on a state change */
    u8 unk2[2];     /**< not yet known */
    s32 unk4;       /**< x */
    s32 unk8;       /**< y */
    s32 unkC;       /**< z */
    s32 unk10;      /**< zeroed on a state change */
    s32 unk14;      /**< set from a fixed object's y on a state change */
    s32 unk18;      /**< zeroed on a state change */
    u8 unk1C[2];    /**< not yet known */
    u16 unk1E;      /**< a height; the query is centred half of it lower */
    s32 unk20;      /**< a third of its magnitude is the query's range */
    u8 unk24;       /**< set to 1 before the query */
    u8 unk25;       /**< bit 0 of the query's result */
    u8 unk26;       /**< flags: bit 7, bit 6, and a state in bits 0..5 */
    u8 unk27[0x15]; /**< not yet known */
} Rec3C;

extern u8 D_800A74D0[];  /**< 128 byte flags; cleared together */
extern s16 D_80096738[]; /**< filled by the lookup: a height, then a direction */
/* MATCHING: copied whole as Quad16 here; code_a0bc reads its fields. */
extern Quad16 D_800DD0A0[]; /**< a table of eight-byte entries */
extern Rec5C D_800CF080[];  /**< 200 Rec5C records */
extern u8 D_800A7550[];     /**< 200 byte marks, one per block entry */
extern PathPt *D_800958A0;  /**< the current path */

/* MATCHING: a struct lvalue keeps the base in a register. */
#define sGameHead (*(GameHead *)D_8009EB78)
/* The Rec48 table; common.h declares it as words. */
#define sRecs48 ((Rec48 *)D_800A9008)

s32 func_80028AE4(Query30 *q);
/* MATCHING: all s32 where the callee has s16: retail neither re-extends
 * the result nor extends the a and b it passes. */
s32 func_80018D04(s32 a, s32 b, u16 t, u16 n);
s32 func_80028260(s32 n);
void func_8002C4D8(void);
s32 func_800183B0(Rec48 *r);
s32 func_8002C650(void);
/* MATCHING: a per-unit view of the squared-distance helper; it takes two
 * VECTOR pointers. */
s32 func_800297A4(void *a, void *b);
/* MATCHING: code_29f54 defines x..n as s16; this unit's calls pass them
 * unextended, so its prototype takes s32. */
s32 func_8003F834(s32 id, s32 x, s32 y, s32 z, s32 n);
void func_80023194(GsCOORDINATE2 *coord, SVECTOR *pos, VECTOR *out);
void func_8002A5B0(Rec78 *rec, Rec48 *r);
void func_8002B8F8(u16 id, Rec3C *r);

extern SVECTOR D_800957E4; /**< a local position to transform to world */
extern VECTOR D_8009F268;  /**< the world position of that local one */

/** @brief Sets `p->pos` to the world position of its local position. */
/* MATCHING: the unused pair puts flag at sp+0x70 and the frame at 0x88. */
void func_80029898(Placed2C *p) {
    MATRIX world;
    MATRIX local;
    SVECTOR v;
    VECTOR t;
    s32 unused[2];
    long flag;

    v.vx = p->lx;
    v.vy = p->ly;
    v.vz = p->lz;
    GsGetLws(p->coord, &local, &world);
    GsSetLsMatrix(&local);
    RotTrans(&v, &t, &flag);
    p->pos.vx = t.vx;
    p->pos.vy = t.vy;
    p->pos.vz = t.vz;
    GsSetLsMatrix(&world);
}

/** @brief Sets `out` to the world position of the local position `lp`. */
/* MATCHING: the unused pair puts flag at sp+0x70 and the frame at 0x88. */
void func_80029930(LocalPos *lp, VECTOR *out) {
    MATRIX world;
    MATRIX local;
    SVECTOR v;
    VECTOR t;
    s32 unused[2];
    long flag;

    v.vx = lp->x;
    v.vy = lp->y;
    v.vz = lp->z;
    GsGetLws(&D_800D86E0[lp->coord], &local, &world);
    GsSetLsMatrix(&local);
    RotTrans(&v, &t, &flag);
    out->vx = t.vx;
    out->vy = t.vy;
    out->vz = t.vz;
}

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_800299D8);

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_80029E74);

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_8002A328);

/** @brief Switches the game mode byte to 0x42 and resets, when a status
 *         field is 1 and the mode is not already 0x43.
 *  @return nothing; the value is undefined. */
s32 func_8002A558(void) {
    /* MATCHING: non-void with no return keeps the second branch's delay
     * slot a nop. */
    if ((((u32)D_80095864 >> 4) & 3) == 1 && sGameHead.unk5 != 0x43) {
        sGameHead.unk5 = 0x42;
        func_80028260(-1);
    }
}

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_8002A5B0);

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_8002A7D8);

/** @brief Sets the four corners of a box `w` wide and `d` deep around the
 *         position `c`, at its height, relative to it. */
void func_8002A98C(s16 (*v)[3], VECTOR *c, s16 w, s16 d) {
    s16 x;
    s16 z;

    x = w / 2;
    v[0][0] = x - c->vx;
    v[1][0] = x - c->vx;
    v[2][0] = -x - c->vx;
    v[3][0] = -x - c->vx;
    z = d / 2;
    v[0][2] = z - c->vz;
    v[1][2] = -z - c->vz;
    v[2][2] = z - c->vz;
    v[3][2] = -z - c->vz;
    v[0][1] = c->vy;
    v[1][1] = c->vy;
    v[2][1] = c->vy;
    v[3][1] = c->vy;
}

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_8002AA58);

/** @brief Moves `u` to the next or previous path segment once it has
 *         passed the next point or not yet reached its own.
 *  @return the new segment */
/* MATCHING: the unused pair gives the leaf its 8-byte frame. */
s32 func_8002AEB8(PathUser *u) {
    s32 unused[2];
    s32 i;
    s32 d;
    PathPt *next;
    PathPt *pt;

    /* MATCHING: two point locals and a sum local; one reused point pointer
     * moves the parameter out of $a0. Integer sums put the index first. */
    i = u->seg;
    next = (PathPt *)(i * 8 + (u32)D_800958A0) + 1;
    d = next->dx * (u->x - next->x) + next->dz * (u->z - next->z);
    if (d >= 0) {
        i++;
    }
    pt = (PathPt *)(i * 8 + (u32)D_800958A0);
    d = -pt->dx * (u->x - pt->x) + -pt->dz * (u->z - pt->z);
    if (d >= 0) {
        i--;
    }
    u->seg = i;
    return i;
}

/** @brief Finds the path segment `u` is on, as the segment update above
 *         does, without storing it.
 *  @return the direction of that segment, from ratan2 */
/* MATCHING: the unused pair gives the frame its 0x20 bytes; a third point
 * local for the last segment, where reusing either colours $a0 differently. */
s16 func_8002AF6C(PathUser *u) {
    s32 unused[2];
    s32 i;
    s32 d;
    PathPt *next;
    PathPt *pt;
    PathPt *seg;

    i = u->seg;
    next = (PathPt *)(i * 8 + (u32)D_800958A0) + 1;
    d = next->dx * (u->x - next->x) + next->dz * (u->z - next->z);
    if (d >= 0) {
        i++;
    }
    pt = (PathPt *)(i * 8 + (u32)D_800958A0);
    d = -pt->dx * (u->x - pt->x) + -pt->dz * (u->z - pt->z);
    if (d >= 0) {
        i--;
    }
    seg = (PathPt *)(i * 8 + (u32)D_800958A0);
    return ratan2(seg[1].x - seg->x, seg[1].z - seg->z);
}

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_8002B04C);

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_8002B220);

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_8002B5FC);

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_8002B7C8);

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_8002B8F8);

/** @brief Updates the Rec78 entry of every live Rec48 record whose Rec5C
 *         record is marked 1. */
void func_8002BC4C(void) {
    u32 i;

    for (i = 0; i < 200; i++) {
        if (sRecs48[i].unk36 != -1 && D_800CF080[sRecs48[i].unk34].unk0 == 1) {
            func_8002A7D8(&D_800D8D20[sRecs48[i].unk36], &sRecs48[i]);
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_8002BD00);

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_8002BEC0);

/** @brief Registers the entries of the directory loaded at a fixed address
 *         from slot 0x33 on.
 *  @return nothing; the value is undefined. */
s32 func_8002C044(void) {
    DirEnt16 *e;
    s32 *p;

    /* MATCHING: non-void with no return orders the loop preheader; the
     * pointer is assigned inside the store so the index loads first. */
    e = (DirEnt16 *)0x8017D708;
    D_800958CC = 0x33;
    D_800958D0 = e->count;
    for (; D_800958CC < D_800958D0 + 0x33; D_800958CC++) {
        D_800D81B0[D_800958CC] = (p = (s32 *)((u8 *)0x8017D708 + e->offset)) + 1;
        e++;
        sGameHead.unk20[D_800958CC] = *D_800D81B0[D_800958CC];
        D_800D81B0[D_800958CC]++;
        sGameHead.unk1B0[D_800958CC] = D_800D81B0[D_800958CC][1];
    }
}

/** @brief Looks up the height under `p`; on success sets its current and
 *         stored y and two angles from the result.
 *  @return the lookup's result, -1 when it failed */
s32 func_8002C0EC(Obj34 *p) {
    s32 v;

    v = func_80018D70(p, D_80096738, p->unk28);
    if (v != -1) {
        p->unk28 = v;
        p->unk4 = p->unk10 = D_80096738[0];
        p->unk2C = ratan2(-D_80096738[3], D_80096738[2]);
        p->unk30 = ratan2(-D_80096738[1], D_80096738[2]);
    }
    return v;
}

/** @brief Sets x and z of `out` to the point `r` away at `deg` degrees. */
void func_8002C188(s32 r, s16 deg, Vec3 *out) {
    s32 a;

    a = deg * 4096 / 360;
    out->x = rsin(a) * r;
    out->z = rcos(a) * r;
}

/** @brief Sets up `m` to draw object `n` of the TMD file at `tmd`, with an
 *         identity transform, and counts it. */
void func_8002C20C(Model70 *m, unsigned long *tmd, u8 n) {
    GsInitCoordinate2(WORLD, &m->coord);
    m->obj.coord2 = &m->coord;
    /* MATCHING: the parameter is advanced in two steps; offsets from one
     * copy give the object pointer and the TMD pointer swapped registers. */
    tmd++;
    GsMapModelingData(tmd);
    tmd += 2;
    GsLinkObject4((unsigned long)tmd, &m->obj, n);
    m->obj.attribute = 0x200;
    m->scale.vx = 0x1000;
    m->scale.vy = 0x1000;
    m->scale.vz = 0x1000;
    m->rot.vx = 0;
    m->rot.vy = 0;
    m->rot.vz = 0;
    D_8009588E++;
}

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_8002C2B4);

/** @brief Updates `p->unk28` from a lookup unless the lookup fails (-1). */
void func_8002C438(Obj2C *p) {
    s32 v;

    v = func_80018D70(p, D_80096738, p->unk28);
    if (v != -1) {
        p->unk28 = v;
    }
}

/** @brief Clears every used Rec5C record, then refreshes the Rec48 table
 *         and clears its records. */
void func_8002C47C(void) {
    u32 i;

    for (i = 0; i < 200; i++) {
        if (D_800CF080[i].unk0 != -1) {
            D_800CF080[i].unk0 = 0;
        }
    }
    func_8002C4D8();
    func_8002C650();
}

/** @brief Refreshes unk28 of every live record of the 200-entry table. */
void func_8002C4D8(void) {
    u32 i;

    /* MATCHING: indexed; a walking Rec48 pointer is biased to unk28. */
    for (i = 0; i < 200; i++) {
        if (sRecs48[i].unk36 != -1) {
            sRecs48[i].unk28 = func_800183B0(&sRecs48[i]);
        }
    }
}

/** @brief Sets the four corners of a box `w` wide and `d` deep, centred on
 *         the origin at height 0. */
void func_8002C540(Box4 *b, s16 w, s16 d) {
    s16 x;
    s16 z;

    x = w / 2;
    b->v[0][0] = x;
    b->v[1][0] = x;
    b->v[2][0] = -x;
    b->v[3][0] = -x;
    z = d / 2;
    b->v[0][2] = z;
    b->v[1][2] = -z;
    b->v[2][2] = z;
    b->v[3][2] = -z;
    b->v[0][1] = 0;
    b->v[1][1] = 0;
    b->v[2][1] = 0;
    b->v[3][1] = 0;
}

/** @brief Clears byte 0 of the first `count` records of a Rec5C table. */
void func_8002C5A4(Rec5C *recs, u16 count) {
    u16 i;

    for (i = 0; i < count; i++) {
        recs->unk0 = 0;
        recs++;
    }
}

/** @brief For each odd-numbered Rec5C record whose byte 0 is 1, sets byte 0
 *         of it and of the record before it to 2.
 *  @return nothing; the value is undefined. */
s32 func_8002C5D0(void) {
    u16 i;

    /* MATCHING: non-void with no return keeps the loop's delay slot a nop. */
    for (i = 1; i < 200; i += 2) {
        if (D_800CF080[(s16)i].unk0 == 1) {
            D_800CF080[(s16)i].unk0 = 2;
            D_800CF080[(s16)i - 1].unk0 = 2;
        }
    }
}

/** @brief Clears unk40 and unk24 of all 200 Rec48 records and all but bit 0
 *         of unk41.
 *  @return nothing; the value is undefined. */
s32 func_8002C650(void) {
    u16 i;
    Rec48 *r;

    /* MATCHING: non-void with no return keeps the loop's delay slot a nop. */
    for (i = 0; i < 200; i++) {
        r = &sRecs48[(s16)i];
        r->unk40 = 0;
        r->unk24 = 0;
        r->unk41 &= 1;
    }
}

/** @brief Latches bit 1 of `r->unk41` once the record comes within `range`
 *         of a fixed object.
 *  @return 1 when the bit was set by this call, else 0. */
s32 func_8002C6A4(Rec48 *r, s32 range) {
    s32 ret;
    s32 sq;

    ret = 0;
    if (!((r->unk41 >> 1) & 1)) {
        sq = range * range;
        if (func_800297A4(D_8009EEC0, r) < sq) {
            r->unk41 |= 2;
            ret = 1;
        }
    }
    return ret;
}

/** @brief Places `b` at its stored position in the coordinate system of
 *         `a`'s Rec78 entry, takes `a`'s angles, adds `a`'s heading to its
 *         own, and updates `b`'s Rec78 entry from it. */
/* MATCHING: the Rec48s' first 0x34 bytes read through the Obj34 view. */
void func_8002C724(Rec48 *a, Rec48 *b) {
    Obj34 *pa;
    Obj34 *pb;

    pa = (Obj34 *)a;
    pb = (Obj34 *)b;
    D_800957E4.vx = pb->unkC;
    D_800957E4.vy = pb->unk10;
    D_800957E4.vz = pb->unk14;
    func_80023194((GsCOORDINATE2 *)D_800D8D20[a->unk36].unk10, &D_800957E4, &D_8009F268);
    pb->unk0 = D_8009F268.vx;
    pb->unk4 = D_8009F268.vy;
    pb->unk8 = D_8009F268.vz;
    pb->unk2C = pa->unk2C;
    pb->unk30 = pa->unk30;
    pb->unk1A = pa->unk1A + pb->unk20;
    func_8002A5B0(&D_800D8D20[b->unk36], b);
}

/** @brief Resets an object's current values from its stored copy. */
void func_8002C820(Obj34 *p) {
    p->unk2C = 0;
    p->unk30 = 0;
    p->unk0 = p->unkC;
    p->unk4 = p->unk10;
    p->unk8 = p->unk14;
    p->unk18 = p->unk1E;
    p->unk1A = p->unk20;
    p->unk1C = p->unk22;
}

/** @brief Copies entry `i` of the eight-byte table to `out`. */
void func_8002C85C(u16 i, Quad16 *out) {
    Quad16 *src;

    /* MATCHING: the base in its own local, then advanced by i; indexing
     * gives the sum the index register. */
    src = D_800DD0A0;
    src += i;
    *out = *src;
}

/** @brief Runs record `r`'s step, then queries its surroundings; on a hit
 *  while flagged in state 2, moves it to state 3 and resets its motion. */
/* MATCHING: the fixed object's y read as a VECTOR member; a plain word
 * read lets the scheduler hoist the r->unk18 store above it. */
void func_8002C894(s16 id, Rec3C *r) {
    Query30 q;
    s32 v;

    func_8002B8F8(id, r);
    q.pos.x = r->unk4;
    q.pos.y = r->unk8 - (s16)r->unk1E / 2;
    q.pos.z = r->unkC;
    r->unk24 = 1;
    q.unk18 = 1;
    q.unk14 = (r->unk20 < 0 ? -r->unk20 : r->unk20) / 3;
    v = func_80028AE4(&q) & 1;
    r->unk25 = v;
    if (v) {
        if (r->unk26 & 0x80) {
            if ((r->unk26 & 0x3F) == 2) {
                r->unk10 = 0;
                r->unk0++;
                r->unk26 = (r->unk26 & 0x40) | 3;
                r->unk14 = ((VECTOR *)D_8009EEC0)->vy;
                r->unk18 = 0;
            }
        }
    }
}

/** @brief Interpolates three angles by `t`/`n` into `out`: out[1] is `r`
 *         times the sine of 0..180 degrees, out[0] -60..60 degrees and
 *         out[2] 0..20 degrees, in 4096ths of a turn. */
void func_8002C994(s16 r, u16 t, u16 n, s32 *out) {
    out[1] = rsin((s16)func_80018D04(0, 180, t, n) * 4096 / 360) * r;
    out[0] = (s16)func_80018D04(-60, 60, t, n) * 4096 / 360;
    out[2] = (s16)func_80018D04(0, 20, t, n) * 4096 / 360;
}

/** @brief Interpolates from 0 towards `b` by `t`/`n`, into out[1]. */
void func_8002CAA4(s16 b, u16 t, u16 n, s32 *out) {
    out[1] = func_80018D04(0, b, t, n);
}

/** @brief Interpolates from `a` towards `b` by `t`/`n`, into out[1]. */
void func_8002CAE4(s16 a, s16 b, u16 t, u16 n, s32 *out) {
    out[1] = func_80018D04(a, b, t, n);
}

/** @brief Interpolates by `t`/`n` into out[1]: from 0 towards twice `b`
 *         in the first 80 percent of `n`, else from four times `b` towards
 *         `b`. */
void func_8002CB24(s16 b, u16 t, u16 n, s32 *out) {
    if ((double)(t * 4096 / 100) < (double)(n * 4096 / 100) * 0.8) {
        out[1] = func_80018D04(0, b * 2, t, n);
    } else {
        out[1] = func_80018D04(b * 4, b, t, n);
    }
}

/** @brief Sets up `m` to draw object `n` of the TMD file at `tmd`, with an
 *         identity transform, and counts it. */
/* MATCHING: an inline copy of the setup above; inlined, the u8 parameter is
 * copied before its andi, where a (u8) argument truncates in place. */
static __inline__ void setupModel(Model70 *m, unsigned long *tmd, u8 n) {
    GsInitCoordinate2(WORLD, &m->coord);
    m->obj.coord2 = &m->coord;
    GsMapModelingData(tmd + 1);
    GsLinkObject4((unsigned long)(tmd + 3), &m->obj, n);
    m->obj.attribute = 0x200;
    m->scale.vx = 0x1000;
    m->scale.vy = 0x1000;
    m->scale.vz = 0x1000;
    m->rot.vx = 0;
    m->rot.vy = 0;
    m->rot.vz = 0;
    D_8009588E++;
}

/** @brief Sets up all 80 Rec78 records as objects of the TMD file at a
 *         fixed address, record i drawing object i, and counts them. */
void func_8002CC24(void) {
    u32 i;

    D_8009588E = 0;
    for (i = 0; i < 80; i++) {
        setupModel((Model70 *)&D_800D8D20[i], (unsigned long *)0x80155000, i);
    }
}

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_8002CCEC);

/** @brief Points the current-block globals at the block `hdr` heads. */
void func_8002D0C4(BlockHeader *hdr) {
    D_800959C0 = (u8 *)hdr + 8;
    D_800959C8 = hdr->unk0;
    /* MATCHING: offset read through a byte pointer, not hdr->offset: the
     * member access lets the load rise above the D_800959C8 store. */
    D_800959C4 = (u8 *)hdr + *(s32 *)((u8 *)hdr + 4);
    D_800958E8 = 0;
}

/** @brief Runs the position query for `pos` with a range of 50. */
s8 func_8002D0F0(Vec3 *pos) {
    Query30 q;

    q.pos.x = pos->x;
    q.pos.y = pos->y;
    q.pos.z = pos->z;
    q.unk18 = 0;
    q.unk14 = 50;
    return func_80028AE4(&q);
}

/** @brief Clears a 128-byte table of flags.
 *  @return nothing; the value is undefined. */
s32 func_8002D140(void) {
    u32 i;

    /* MATCHING: non-void with no return; it keeps $v0 live at the exit, so
     * the loop's delay slot stays a nop. */
    for (i = 0; i < 128; i++) {
        D_800A74D0[i] = 0;
    }
}

/** @brief Marks with 2 every block entry whose flag is 1.
 *  @return nothing; the value is undefined. */
s32 func_8002D16C(void) {
    u32 i;

    /* MATCHING: non-void with no return keeps the loop's delay slot a nop. */
    for (i = 0; i < 200; i++) {
        if ((s8)D_800A74D0[((Ent8 *)D_800959C4)[i].unk6] == 1) {
            D_800A7550[i] = 2;
        }
    }
}

/** @brief Flags n, then marks with 1 every block entry whose unk6 is n.
 *  @param n the flag to set and the value to look for
 *  @return nothing; the value is undefined. */
s32 func_8002D1CC(s16 n) {
    u32 i;

    /* MATCHING: non-void with no return keeps the loop's delay slot a nop. */
    D_800A74D0[n] = 1;
    for (i = 0; i < 200; i++) {
        if (((Ent8 *)D_800959C4)[i].unk6 == n) {
            D_800A7550[i] = 1;
        }
    }
}

/** @brief On even frames, launches effect 4 at the first block entry marked
 *         1 and marks it 2. */
void func_8002D230(void) {
    u32 i;
    Ent8 *e;

    if (D_8009585C & 1) {
        return;
    }
    for (i = 0; i < 200; i++) {
        if ((s8)D_800A7550[i] == 1) {
            /* MATCHING: an integer sum puts the scaled index first. */
            e = (Ent8 *)(i * 8 + (u32)D_800959C4);
            func_8003F834(4, e->unk0, e->unk2 - 50, (s16)e->unk4, 0);
            D_800A7550[i] = 2;
            return;
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_8002D2C0);
