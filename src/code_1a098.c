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
    u8 unk24[8]; /**< not yet known */
    s32 unk2C;   /**< zeroed on a reset */
    s32 unk30;   /**< zeroed on a reset */
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

/** @brief The head of the game state; only byte 5 is used here. */
typedef struct {
    u8 unk0[5]; /**< not yet known */
    u8 unk5;    /**< a mode byte: 0x42 and 0x43 seen */
} GameHead;

extern u8 D_800A74D0[];            /**< 128 byte flags; cleared together */
extern u8 D_80096738[];            /**< passed to the lookup */
extern Quad16 D_800DD0A0[];        /**< a table of eight-byte entries */
extern Rec5C D_800CF080[];         /**< 200 Rec5C records */
extern u8 D_800A7550[];            /**< 200 byte marks, one per block entry */
extern GsCOORDINATE2 D_800D86E0[]; /**< coordinate systems */

/* MATCHING: a struct lvalue keeps the base in a register. */
#define sGameHead (*(GameHead *)D_8009EB78)
/* The Rec48 table; common.h declares it as words. */
#define sRecs48 ((Rec48 *)D_800A9008)

s32 func_80018D70(void *pos, void *arg, s32 cur);
s32 func_80028AE4(Query30 *q);
/* MATCHING: s32, though the callee returns a sign-extended s16: retail
 * stores the result with no re-extension. */
s32 func_80018D04(s16 a, s16 b, u16 t, u16 n);
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

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_8002A98C);

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_8002AA58);

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_8002AEB8);

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_8002AF6C);

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_8002B04C);

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_8002B220);

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_8002B5FC);

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_8002B7C8);

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_8002B8F8);

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_8002BC4C);

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_8002BD00);

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_8002BEC0);

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_8002C044);

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_8002C0EC);

/** @brief Sets x and z of `out` to the point `r` away at `deg` degrees. */
void func_8002C188(s32 r, s16 deg, Vec3 *out) {
    s32 a;

    a = deg * 4096 / 360;
    out->x = rsin(a) * r;
    out->z = rcos(a) * r;
}

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_8002C20C);

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

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_8002C724);

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

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_8002C894);

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_8002C994);

/** @brief Interpolates from 0 towards `b` by `t`/`n`, into out[1]. */
void func_8002CAA4(s16 b, u16 t, u16 n, s32 *out) {
    out[1] = func_80018D04(0, b, t, n);
}

/** @brief Interpolates from `a` towards `b` by `t`/`n`, into out[1]. */
void func_8002CAE4(s16 a, s16 b, u16 t, u16 n, s32 *out) {
    out[1] = func_80018D04(a, b, t, n);
}

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_8002CB24);

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_8002CC24);

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
