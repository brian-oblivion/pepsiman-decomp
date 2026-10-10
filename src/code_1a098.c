#include "common.h"
#include "memory.h"
#include "libapi.h"
#include "libgte.h"
#include "sys/file.h"
#include "libgpu.h"
#include "libgs.h"
#include "code_1a098.h"
#include "code_7d74.h"
#include "code_13068.h"
#include "code_a0bc.h"

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
    u8 unk0[2];      /**< not yet known */
    u8 unk2;         /**< nonzero to place the second-buffer records */
    u8 unk3[2];      /**< not yet known */
    u8 unk5;         /**< a mode byte: 0x42 and 0x43 seen */
    u8 unk6[0x1A];   /**< not yet known */
    s32 unk20[100];  /**< first word of each loaded entry */
    s32 unk1B0[100]; /**< third word of each loaded entry */
    u8 unk340[8];    /**< not yet known */
    s32 unk348;      /**< x of the player (a guess) */
    s32 unk34C;      /**< y */
    s32 unk350;      /**< z */
    u8 unk354[0x64]; /**< not yet known */
    s32 unk3B8;      /**< 1 once a placed record reports a hit */
    u8 unk3BC[0x10]; /**< not yet known */
    s32 unk3CC;      /**< the word reported with that hit */
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
    u16 unk0;      /**< a counter; bumped on a state change */
    u8 unk2[2];    /**< not yet known */
    s32 unk4;      /**< x */
    s32 unk8;      /**< y */
    s32 unkC;      /**< z */
    s32 unk10;     /**< zeroed on a state change */
    s32 unk14;     /**< set from a fixed object's y on a state change */
    s32 unk18;     /**< zeroed on a state change */
    u16 unk1C;     /**< a width */
    u16 unk1E;     /**< a height; the query is centred half of it lower */
    s32 unk20;     /**< a third of its magnitude is the query's range */
    u8 unk24;      /**< set to 1 before the query */
    u8 unk25;      /**< bit 0 of the query's result */
    u8 unk26;      /**< flags: bit 7, bit 6, and a state in bits 0..5 */
    u8 unk27[5];   /**< not yet known */
    s16 unk2C;     /**< index of the Rec5C record it belongs to */
    u8 unk2E[0xE]; /**< not yet known */
} Rec3C;

/** @brief Something that bobs up and down while it drifts. */
typedef struct {
    u8 unk0[4]; /**< not yet known */
    s32 x;      /**< x */
    s32 y;      /**< y */
    s32 z;      /**< z */
    s32 count;  /**< steps taken; the drift stops at 15 */
    s32 baseY;  /**< y the bob is measured from */
    s32 phase;  /**< bob phase in degrees */
} Drifter;

/** @brief The Rec48 table, copied whole. */
typedef struct {
    s32 w[sizeof(Rec48) * 200 / 4]; /**< the records */
} Recs48Copy;

/** @brief The Rec5C table, copied whole. */
typedef struct {
    s32 w[sizeof(Rec5C) * 200 / 4]; /**< the records */
} Recs5CCopy;

/** @brief The Rec3C table, copied whole. */
typedef struct {
    s32 w[sizeof(Rec3C) * 100 / 4]; /**< the records */
} Recs3CCopy;

/** @brief The block header area, copied whole; bytes, so a copy of it
 *         tests the alignment at run time. */
typedef struct {
    u8 b[0x800]; /**< the area */
} BlockCopy;

/** @brief A saved copy of the record tables and the block header. */
typedef struct {
    Recs48Copy recs48; /**< the Rec48 table */
    Recs5CCopy recs5C; /**< the Rec5C table */
    Recs3CCopy recs3C; /**< the Rec3C table */
    BlockCopy block;   /**< the block header area */
} SavedTables;

/** @brief The game-progress block, as far as the full reset writes it. */
typedef struct {
    u8 unk0;        /**< 0x38 after a reset */
    u8 unk1;        /**< 1 after a reset */
    u8 unk2;        /**< copied from a global byte on a reset */
    u8 unk3;        /**< not yet known */
    s32 unk4;       /**< 50 after a reset */
    s32 unk8;       /**< zeroed on a reset */
    s32 unkC;       /**< zeroed on a reset */
    u8 unk10[2];    /**< not yet known */
    s16 unk12[18];  /**< zeroed on a reset, but [14] (0x2E) set to 600 */
    u8 unk36[0x32]; /**< not yet known */
    u8 unk68;       /**< zeroed on a reset */
    u8 unk69;       /**< zeroed on a reset */
    s16 unk6A;      /**< zeroed on a reset */
    u8 unk6C;       /**< zeroed on a reset */
    u8 unk6D;       /**< zeroed on a reset */
} Progress6E;

extern s16 D_80096738[]; /**< filled by the lookup: a height, then a direction */
/* MATCHING: copied whole as Quad16 here; code_a0bc reads its fields. */
extern Quad16 D_800DD0A0[]; /**< a table of eight-byte entries */
extern Rec5C D_800CF080[];  /**< 200 Rec5C records */
extern u8 D_800A7550[];     /**< 200 byte marks, one per block entry */
extern PathPt *D_800958A0;  /**< the current path */
extern Rec3C D_800A7898[];  /**< 100 Rec3C records */
extern s32 D_80095824;      /**< zeroed by the full reset */
extern u8 D_800958D8;       /**< zeroed by the full reset */
extern u8 D_800959D8;       /**< zeroed by the full reset */

/* The unpacked header of the last TIM loaded; common.h declares a word. */
#define sTim ((TimInfo *)D_800956D4)
/* MATCHING: a struct lvalue keeps the base in a register. */
#define sGameHead (*(GameHead *)D_8009EB78)
/* The Rec48 table; common.h declares it as words. */
#define sRecs48 ((Rec48 *)D_800A9008)
/* The progress block; code_1dc24 views the same bytes as other records. */
extern u8 D_80095B28[];
#define sProgress (*(Progress6E *)D_80095B28)

s32 func_80028AE4(Query30 *q);
/* MATCHING: all s32 where the callee has s16: retail neither re-extends
 * the result nor extends the a and b it passes. */
s32 func_80018D04(s32 a, s32 b, u16 t, u16 n);
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
s32 func_8002BEC0(Drifter *d);
void func_8002C188(s32 r, s32 deg, Vec3 *out);
/* MATCHING: code_1dc24's resets, declared per unit: Rec3C is local to each
 * unit until a shared header holds it. */
void func_800337E4(u8 *buf);
void func_80033854(Rec5C *recs);
void func_800338A0(Rec48 *recs);
void func_8003390C(Rec3C *recs);

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

/** @brief A vertex of the floor data. */
typedef struct {
    s16 x;   /**< x */
    s16 y;   /**< height */
    s16 z;   /**< z */
    s16 pad; /**< unused */
} FloorVtx;

/** @brief A face of the floor data: a triangle when v[3] is 0xFFFF. */
typedef struct {
    s32 flag; /**< zero for a face that is skipped; reported on a hit */
    u16 v[4]; /**< vertex indices */
} FloorFace;

/** @brief A zone's run of faces. */
typedef struct {
    s32 offset; /**< byte offset of the faces from the header */
    u32 count;  /**< how many */
} FloorZone;

/** @brief The head of the floor data. */
typedef struct {
    s32 unk0;          /**< not yet known */
    s32 vtx;           /**< byte offset of the vertices from the header */
    FloorZone zone[1]; /**< the zones */
} FloorHdr;

/** @brief What a floor query reports. */
typedef struct {
    s16 y;    /**< the floor height */
    s16 nx;   /**< the floor normal */
    s16 ny;   /**< the floor normal */
    s16 nz;   /**< the floor normal */
    s32 flag; /**< the face's flag */
} FloorHit;

/* MATCHING: integer sums put the scaled index first. */
#define VTX(i) ((FloorVtx *)((i) * 8 + (s32)v))

/** @brief Finds the face of zone `zone` of the floor data `hdr` under `pos`
 *         (x and z) and reports its height there, its normal and its flag.
 *  @return the height, 0 for a vertical face, 0x7FFF when no face is
 *          under `pos` */
/* MATCHING: every vertex read spelled out; pointer locals move registers. */
s32 func_800299D8(FloorHit *out, s32 zone, VECTOR *pos, FloorHdr *hdr) {
    VECTOR a;
    VECTOR b;
    VECTOR n;
    FloorVtx *v;
    FloorFace *f;
    u32 k;
    s32 d;

    f = (FloorFace *)(((FloorZone *)(zone * 8 + (s32)hdr))[1].offset + (s32)hdr);
    v = (FloorVtx *)(hdr->vtx + (s32)hdr);
    for (k = 0; k < ((FloorZone *)(zone * 8 + (s32)hdr))[1].count; k++, f++) {
        if (f->flag == 0) {
            continue;
        }
        if (f->v[3] == 0xFFFF) {
            if ((pos->vx - VTX(f->v[0])->x) * (VTX(f->v[1])->z - VTX(f->v[0])->z) +
                    (pos->vz - VTX(f->v[0])->z) * (VTX(f->v[0])->x - VTX(f->v[1])->x) <
                0) {
                continue;
            }
            if ((pos->vx - VTX(f->v[1])->x) * (VTX(f->v[2])->z - VTX(f->v[1])->z) +
                    (pos->vz - VTX(f->v[1])->z) * (VTX(f->v[1])->x - VTX(f->v[2])->x) <
                0) {
                continue;
            }
            d = (pos->vx - VTX(f->v[2])->x) * (VTX(f->v[0])->z - VTX(f->v[2])->z) +
                (pos->vz - VTX(f->v[2])->z) * (VTX(f->v[2])->x - VTX(f->v[0])->x);
        } else {
            if ((pos->vx - VTX(f->v[0])->x) * (VTX(f->v[1])->z - VTX(f->v[0])->z) +
                    (pos->vz - VTX(f->v[0])->z) * (VTX(f->v[0])->x - VTX(f->v[1])->x) <
                0) {
                continue;
            }
            if ((pos->vx - VTX(f->v[1])->x) * (VTX(f->v[3])->z - VTX(f->v[1])->z) +
                    (pos->vz - VTX(f->v[1])->z) * (VTX(f->v[1])->x - VTX(f->v[3])->x) <
                0) {
                continue;
            }
            if ((pos->vx - VTX(f->v[2])->x) * (VTX(f->v[0])->z - VTX(f->v[2])->z) +
                    (pos->vz - VTX(f->v[2])->z) * (VTX(f->v[2])->x - VTX(f->v[0])->x) <
                0) {
                continue;
            }
            d = (pos->vx - VTX(f->v[3])->x) * (VTX(f->v[2])->z - VTX(f->v[3])->z) +
                (pos->vz - VTX(f->v[3])->z) * (VTX(f->v[3])->x - VTX(f->v[2])->x);
        }
        if (d < 0) {
            continue;
        }
        a.vx = VTX(f->v[2])->x - VTX(f->v[0])->x;
        a.vy = VTX(f->v[2])->y - VTX(f->v[0])->y;
        a.vz = VTX(f->v[2])->z - VTX(f->v[0])->z;
        b.vx = VTX(f->v[1])->x - VTX(f->v[0])->x;
        b.vy = VTX(f->v[1])->y - VTX(f->v[0])->y;
        b.vz = VTX(f->v[1])->z - VTX(f->v[0])->z;
        OuterProduct0(&a, &b, &n);
        a.vx = pos->vx - VTX(f->v[0])->x;
        a.vz = pos->vz - VTX(f->v[0])->z;
        if (n.vy == 0) {
            return 0;
        }
        d = n.vx * a.vx + n.vz * a.vz;
        out->y = VTX(f->v[0])->y + ((n.vy >> 1) - d) / n.vy;
        n.vx >>= 6;
        n.vy >>= 6;
        n.vz >>= 6;
        VectorNormal(&n, &a);
        out->nx = a.vx;
        out->ny = a.vy;
        out->nz = a.vz;
        out->flag = f->flag;
        return out->y;
    }
    return 0x7FFF;
}

/** @brief Finds the face of zone `zone` of the wall data `hdr` whose centre
 *         is nearest `pos` and tells which side of it `pos` is on.
 *  @return -1 behind the nearest face, 0 in front of it or with no face */
s32 func_80029E74(s32 zone, VECTOR *pos, FloorHdr *hdr) {
    VECTOR a;
    VECTOR b;
    VECTOR n;
    FloorVtx *v;
    FloorFace *f;
    FloorFace *best;
    u32 k;
    s32 d;
    s32 min;

    min = 1000000;
    f = (FloorFace *)(((FloorZone *)(zone * 8 + (s32)hdr))[1].offset + (s32)hdr);
    v = (FloorVtx *)(hdr->vtx + (s32)hdr);
    for (k = 0; k < ((FloorZone *)(zone * 8 + (s32)hdr))[1].count; k++, f++) {
        if (f->flag != 0) {
            continue;
        }
        if (f->v[3] == 0xFFFF) {
            a.vx = (VTX(f->v[0])->x + VTX(f->v[1])->x + VTX(f->v[2])->x) / 3;
            a.vy = (VTX(f->v[0])->y + VTX(f->v[1])->y + VTX(f->v[2])->y) / 3;
            a.vz = (VTX(f->v[0])->z + VTX(f->v[1])->z + VTX(f->v[2])->z) / 3;
        } else {
            a.vx = (VTX(f->v[0])->x + VTX(f->v[1])->x + VTX(f->v[2])->x + VTX(f->v[3])->x) >> 2;
            a.vy = (VTX(f->v[0])->y + VTX(f->v[1])->y + VTX(f->v[2])->y + VTX(f->v[3])->y) >> 2;
            a.vz = (VTX(f->v[0])->z + VTX(f->v[1])->z + VTX(f->v[2])->z + VTX(f->v[3])->z) >> 2;
        }
        d = (a.vx - pos->vx) * (a.vx - pos->vx) + (a.vy - pos->vy) * (a.vy - pos->vy) +
            (a.vz - pos->vz) * (a.vz - pos->vz);
        if (d <= min) {
            min = d;
            best = f;
        }
    }
    if (min == 1000000) {
        return 0;
    }
    f = best;
    a.vx = VTX(f->v[2])->x - VTX(f->v[0])->x;
    a.vy = VTX(f->v[2])->y - VTX(f->v[0])->y;
    a.vz = VTX(f->v[2])->z - VTX(f->v[0])->z;
    b.vx = VTX(f->v[1])->x - VTX(f->v[0])->x;
    b.vy = VTX(f->v[1])->y - VTX(f->v[0])->y;
    b.vz = VTX(f->v[1])->z - VTX(f->v[0])->z;
    OuterProduct0(&a, &b, &n);
    n.vx >>= 6;
    n.vy >>= 6;
    n.vz >>= 6;
    VectorNormal(&n, &a);
    if (a.vx * (pos->vx - VTX(f->v[0])->x) + a.vy * (pos->vy - VTX(f->v[0])->y) +
            a.vz * (pos->vz - VTX(f->v[0])->z) <
        0) {
        return -1;
    }
    return 0;
}

#undef VTX

/* MATCHING: code_29f54 defines x and y as s16; this unit passes them
 * unextended. */
void func_8003A3F4(s32 *index, s32 x, s32 y);

s32 func_8002A558(void);

/** @brief Probes 50 units ahead of the player, 45 degrees either side of
 *         the heading; each probe that hits pushes the player 25 units
 *         back from that side. Then runs the mode check. */
void func_8002A328(void) {
    VECTOR pos;
    s32 idx;

    pos.vx = sGameHead.unk348 + (rsin(D_800A7680[0].vy + 0x200) * 50 >> 12);
    pos.vy = sGameHead.unk34C;
    pos.vz = sGameHead.unk350 + (rcos(D_800A7680[0].vy + 0x200) * 50 >> 12);
    idx = D_800957F4;
    func_8003A3F4(&idx, pos.vx, pos.vz);
    if (func_80029E74(idx, &pos, (FloorHdr *)D_800958B4)) {
        sGameHead.unk348 -= rsin(D_800A7680[0].vy + 0x400) * 25 >> 12;
        sGameHead.unk350 -= rcos(D_800A7680[0].vy + 0x400) * 25 >> 12;
    }
    pos.vx = sGameHead.unk348 + (rsin(D_800A7680[0].vy - 0x200) * 50 >> 12);
    pos.vy = sGameHead.unk34C;
    pos.vz = sGameHead.unk350 + (rcos(D_800A7680[0].vy - 0x200) * 50 >> 12);
    idx = D_800957F4;
    func_8003A3F4(&idx, pos.vx, pos.vz);
    if (func_80029E74(idx, &pos, (FloorHdr *)D_800958B4)) {
        sGameHead.unk348 -= rsin(D_800A7680[0].vy - 0x400) * 25 >> 12;
        sGameHead.unk350 -= rcos(D_800A7680[0].vy - 0x400) * 25 >> 12;
    }
    func_8002A558();
}

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

/* The result of a second-buffer record's test: a flag, then a word. */
extern s8 D_80095898;  /**< 1 on a hit */
extern s32 D_8009589C; /**< the word reported with a hit */
/* code_1902c's tests of a 0x4C-byte record; types not yet known. */
/* MATCHING: s32 (its body leaves a value); void moves a register. */
s32 func_80028F0C(void *rec, s8 *out);

/** @brief Updates the entry `rec` and places and tests its records: the
 *         first-buffer ones collect a hit bit into `r->unk40`, and, when
 *         the game head allows, the second-buffer ones report a hit to it.
 *  @return nothing; the value is undefined. */
/* MATCHING: non-void with no return keeps two delay slots nops (the
 * unk2 test and the second loop's back branch). */
s32 func_8002A7D8(Rec78 *rec, Rec48 *r) {
    s16 i;
    s16 start;
    s16 n;
    Placed2C *p;
    u8 *q;

    func_8002A5B0(rec, r);
    start = rec->unk72;
    n = rec->unk74;
    r->unk40 = 0;
    if (n != -1) {
        for (i = start; i < start + n; i++) {
            p = &((Placed2C *)D_800D3CA8)[i];
            func_80029898(p);
            r->unk40 |= func_80028AE4((Query30 *)p) & 1;
        }
    }
    if (sGameHead.unk2 != 0) {
        start = rec->unk6E;
        n = rec->unk70;
        if (n != -1) {
            for (i = start; i < start + n; i++) {
                q = D_800DB2C0 + i * 0x4C;
                func_80028888(q);
                func_80028F0C(q, &D_80095898);
                if (D_80095898 == 1) {
                    sGameHead.unk3B8 = D_80095898;
                    sGameHead.unk3CC = D_8009589C;
                }
            }
        }
    }
}

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

/** @brief Tests every box not yet hit. With the mode byte clear, a box is
 *         hit when the player stands inside its corners (halved); with it
 *         set, its corners are placed in the world around it and the
 *         record test decides.
 *  @return nothing; the value is undefined. */
/* MATCHING: non-void with no return keeps two delay slots nops. */
s32 func_8002AA58(void) {
    MATRIX world;
    MATRIX local;
    SVECTOR sv;
    VECTOR t;
    s32 unused[2];
    GsCOORDINATE2 coord;
    long flag;
    u16 i;
    s16 j;
    Rec5C *b;
    s16(*v)[3];
    s16 px;
    s16 pz;

    if (D_80095858 > 0) {
        return;
    }
    i = 0;
    if (D_800958F8 == 0) {
        for (; i < 200; i++) {
            b = &D_800CF080[(s16)i];
            v = D_800CF080[(s16)i].v;
            if (b->unk0 != 0) {
                continue;
            }
            px = sGameHead.unk348 - b->x;
            pz = sGameHead.unk350 - b->z;
            if ((px - v[0][0] / 2) * ((v[1][2] - v[0][2]) / 2) +
                    (pz - v[0][2] / 2) * -((v[1][0] - v[0][0]) / 2) <
                0) {
                continue;
            }
            if ((px - v[1][0] / 2) * ((v[3][2] - v[1][2]) / 2) +
                    (pz - v[1][2] / 2) * -((v[3][0] - v[1][0]) / 2) <
                0) {
                continue;
            }
            if ((px - v[2][0] / 2) * ((v[0][2] - v[2][2]) / 2) +
                    (pz - v[2][2] / 2) * -((v[0][0] - v[2][0]) / 2) <
                0) {
                continue;
            }
            if ((px - v[3][0] / 2) * ((v[2][2] - v[3][2]) / 2) +
                    ((pz - v[3][2]) / 2) * -((v[2][0] - v[3][0]) / 2) <
                0) {
                continue;
            }
            b->unk0 = 1;
        }
    } else {
        for (; i < 200; i++) {
            if (D_800CF080[(s16)i].unk0 != 0) {
                continue;
            }
            GsInitCoordinate2(WORLD, &coord);
            coord.coord.t[0] = D_800CF080[(s16)i].x;
            coord.coord.t[1] = D_800CF080[(s16)i].y;
            coord.coord.t[2] = D_800CF080[(s16)i].z;
            D_800CF080[(s16)i].coord = &coord;
            GsGetLws(D_800CF080[(s16)i].coord, &local, &world);
            GsSetLsMatrix(&local);
            for (j = 0; j < 4; j++) {
                sv.vx = D_800CF080[(s16)i].v[j][0];
                sv.vy = D_800CF080[(s16)i].v[j][1];
                sv.vz = D_800CF080[(s16)i].v[j][2];
                RotTrans(&sv, &t, &flag);
                D_800CF080[(s16)i].w[j][0] = t.vx + D_800A7308[0];
                D_800CF080[(s16)i].w[j][1] = t.vy;
                D_800CF080[(s16)i].w[j][2] = t.vz + D_800A7308[2];
            }
            GsSetLsMatrix(&world);
            func_80028F0C(D_800CF080[(s16)i].v, &D_80095898);
            if (D_80095898 == 1) {
                D_800CF080[(s16)i].unk0 = 1;
            }
        }
    }
}

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

extern s16 D_800958E2;    /**< the selected Rec78 entry */
extern char D_80010B34[]; /**< "TRAP NO  (%2d / %2d)" */
void func_8002B5FC(void);

/** @brief A trap viewer frame: fixed view, no fog, steps the selected
 *         Rec78 entry with the pad, places and draws it on a turning
 *         record, draws the axes and prints the entry number. */
void func_8002B04C(void) {
    s32 flags;

    D_800A7308[0] = 0;
    D_800A7308[2] = 0;
    D_800DB2A0[0] = 100;
    D_800DB2A0[1] = -200;
    D_800DB2A0[2] = 1000;
    D_800DB2A0[3] = 0;
    D_800DB2A0[4] = 0;
    D_800DB2A0[5] = 0;
    flags = D_80095970;
    D_8009574C = 0;
    D_80095754 = 0;
    D_8009575C = 0;
    if (flags & 2) {
        D_800958E2++;
    }
    if (flags & 1) {
        D_800958E2--;
    }
    D_800958E2 = D_800958E2 < 0 ? 0 : D_800958E2 > D_8009588E - 1 ? D_8009588E - 1 : D_800958E2;
    func_8002980C();
    D_800A9008[0] = 0;
    D_800A9008[1] = 0;
    D_800A9008[2] = 0;
    ((s16 *)D_800A9008)[12] = 0;
    ((s16 *)D_800A9008)[13] = D_8009585C % 360 * 4096 / 360;
    ((s16 *)D_800A9008)[14] = 0;
    func_8002A7D8(&D_800D8D20[D_800958E2], (Rec48 *)D_800A9008);
    func_80023F80(D_8009EB78);
    func_80029838();
    func_8002B5FC();
    FntPrint(D_80010B34, D_800958E2, D_8009588E);
}

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_8002B220);

/** @brief Sorts three white lines 400 units long through the origin, one
 *         along each axis, into the current ordering table. */
void func_8002B5FC(void) {
    VECTOR world;
    SVECTOR screen;
    GsLINE line;

    line.attribute = 0;
    line.r = 0xFF;
    line.g = 0xFF;
    line.b = 0xFF;
    world.vx = 0;
    world.vy = -200;
    world.vz = 0;
    func_800230E0(&world, &screen);
    line.x0 = screen.vx;
    line.y0 = screen.vy;
    world.vx = 0;
    world.vy = 200;
    world.vz = 0;
    func_800230E0(&world, &screen);
    line.x1 = screen.vx;
    line.y1 = screen.vy;
    GsSortLine(&line, &D_800ACEA8[D_80095750], 50);
    world.vx = -200;
    world.vy = 0;
    world.vz = 0;
    func_800230E0(&world, &screen);
    line.x0 = screen.vx;
    line.y0 = screen.vy;
    world.vx = 200;
    world.vy = 0;
    world.vz = 0;
    func_800230E0(&world, &screen);
    line.x1 = screen.vx;
    line.y1 = screen.vy;
    GsSortLine(&line, &D_800ACEA8[D_80095750], 50);
    world.vx = 0;
    world.vy = 0;
    world.vz = -200;
    func_800230E0(&world, &screen);
    line.x0 = screen.vx;
    line.y0 = screen.vy;
    world.vx = 0;
    world.vy = 0;
    world.vz = 200;
    func_800230E0(&world, &screen);
    line.x1 = screen.vx;
    line.y1 = screen.vy;
    GsSortLine(&line, &D_800ACEA8[D_80095750], world.vz >> 2);
}

/** @brief Loads every image of the directory `dir` and registers each as a
 *         texture, numbered from `id` on. */
void func_8002B7C8(DirEnt16 *dir, u16 id) {
    DirEnt16 *e;
    u16 i;
    u16 n;
    s32 tp;
    TimInfo *t;

    e = dir;
    n = dir->count;
    for (i = 0; i < n; i++) {
        func_80017774((u8 *)dir + e->offset);
        DrawSync(0);
        tp = GetTPage(0, 0, sTim->pixRect.x, sTim->pixRect.y);
        t = sTim;
        /* MATCHING: x mod 64 spelled out; % narrows to a halfword. */
        func_8001B2F4(id, 0, (u8)t->unk1C, (u8)t->pixRect.h, (u8)tp,
                      (u8)((t->pixRect.x - t->pixRect.x / 64 * 64) * t->unk1E),
                      (u8)(t->pixRect.y % 256), t->clutRect.x, (u16)t->clutRect.y);
        id++;
        e++;
    }
}

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

/** @brief Updates `r` (id `id`) and queries around it; a hit in state 2
 *         of a flagged record moves it to state 3. Inlined here and by the
 *         out-of-line copy below. */
static __inline__ void queryRec(s16 id, Rec3C *r) {
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

/** @brief Steps every live Rec3C record whose Rec5C record is marked 1:
 *         a flagged one is updated and queried, a state-3 one drifts and is
 *         updated on a drift step, any other one is updated. */
/* MATCHING: the record pointer is taken inside the body. */
void func_8002BD00(void) {
    u32 i;
    Rec3C *r;
    s8 k;

    for (i = 0; i < 100; i++) {
        r = &D_800A7898[i];
        if ((s16)r->unk0 != -1) {
            k = D_800CF080[r->unk2C].unk0;
            if (k == 1) {
                if (r->unk26 & 0x80) {
                    queryRec(r->unk0, r);
                } else if ((r->unk26 & 0x3F) != 3 || (s8)func_8002BEC0((Drifter *)r) == 1) {
                    func_8002B8F8(r->unk0, r);
                }
            }
        }
    }
}

/** @brief Takes one of the first 15 drift steps of `d`: moves it 20 units
 *         along the current heading and bobs it.
 *  @return 1 on two steps of every three, 0 on the third and once done */
/* MATCHING: a u8 result copies the test into its register; a base local
 * keeps baseY's load above the abs. */
s32 func_8002BEC0(Drifter *d) {
    s32 b;
    u8 ret;
    s32 y;

    if (d->count < 15) {
        d->count++;
        ret = d->count % 3 != 0;
        func_8002C188(20, D_800A7680[0].vy * 360 / 4096, (Vec3 *)D_800D39C8);
        d->x -= ((Vec3 *)D_800D39C8)->x >> 12;
        d->z -= ((Vec3 *)D_800D39C8)->z >> 12;
        d->phase = (d->phase + 10) % 360;
        b = rsin(d->phase * 4096 / 360) * 200 >> 12;
        y = d->baseY;
        if (b < 0) {
            b = -b;
        }
        d->y = y - b;
    } else {
        ret = 0;
    }
    return ret;
}

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
void func_8002C188(s32 r, s32 deg, Vec3 *out) {
    s32 a;

    /* MATCHING: deg is s32, narrowed here; func_8002BEC0 passes it unextended. */
    a = (s16)deg * 4096 / 360;
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

/** @brief Restores the Rec48, Rec5C and Rec3C tables and the block header
 *         from their saved copy, and points the current-block globals at
 *         the restored header. */
/* MATCHING: word-typed table copies; the byte-typed header copy keeps the
 * runtime alignment test. */
void func_8002C2B4(void) {
    SavedTables *s;

    s = (SavedTables *)0x80173778;
    *(Recs48Copy *)sRecs48 = s->recs48;
    *(Recs5CCopy *)D_800CF080 = s->recs5C;
    *(Recs3CCopy *)D_800A7898 = s->recs3C;
    *(BlockCopy *)0x801FD000 = s->block;
    func_8002D0C4((BlockHeader *)0x801FD000);
}

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
    queryRec(id, r);
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

/** @brief A zone's run of group indices. */
typedef struct {
    s16 start; /**< first index into the group list */
    s16 count; /**< how many */
} Zone4;

/** @brief A run of block entries. */
typedef struct {
    u32 first; /**< first entry */
    u32 count; /**< how many */
} Group8;

extern Zone4 *D_80095934; /**< the zones */
extern s8 *D_8009593C;    /**< the group list the zones index */
void func_800414EC(s16 n);

/** @brief The magnitude of `x`. */
/* MATCHING: this ternary is cc1's abs, a bgez with its delay slot a nop. */
static __inline__ s32 absInt(s32 x) {
    return x >= 0 ? x : -x;
}

s8 func_8002D0F0(Vec3 *pos);
void func_8001B004(u16 id, SVECTOR *size, CVECTOR *color, s32 shift, GsOT *ot);

/** @brief Draws the markers of the current zone: each block entry marked 2
 *         whose flag is set and not yet collected gets a sprite, near or
 *         far by its distance, and when the player is over it it is
 *         collected (its effect, a sound, bit 15 of its flag index). */
void func_8002CCEC(void) {
    GsCOORDINATE2 coord;
    MATRIX mat;
    SVECTOR size;
    Vec3 pos;
    s32 j;
    u32 i;
    u32 k;
    u16 n;
    Ent8 *e;
    s16 m;

    GsInitCoordinate2(WORLD, &coord);
    for (j = 0; j < D_80095934[D_8009578C].count; j++) {
        if (D_8009593C[D_80095934[D_8009578C].start + j] < D_800959C8) {
            for (i = 0;
                 i < ((Group8 *)D_800959C0)[D_8009593C[D_80095934[D_8009578C].start + j]].count; i++) {
                k = ((Group8 *)D_800959C0)[D_8009593C[D_80095934[D_8009578C].start + j]].first + i;
                if ((s8)D_800A7550[k] != 2) {
                    continue;
                }
                e = (Ent8 *)(k * 8 + (u32)D_800959C4);
                n = e->unk6;
                if ((s8)D_800A74D0[e->unk6] == 0 || e->unk6 == 0 || (e->unk6 & 0x8000)) {
                    continue;
                }
                coord.coord.t[0] = -sGameHead.unk348 + e->unk0;
                coord.coord.t[1] = e->unk2;
                coord.coord.t[2] = -sGameHead.unk350 + (s16)e->unk4;
                coord.flg = 0;
                GsGetLs(&coord, &mat);
                GsSetLsMatrix(&mat);
                size.vy = 75;
                size.vx = 75;
                if (absInt(coord.coord.t[0]) > 700 || absInt(coord.coord.t[2]) > 700) {
                    func_8001B004(0xFA, &size, NULL, 2, &D_800A7318[D_80095750]);
                } else {
                    func_8001A69C(0xFA, &size, NULL, 2, &D_800ACEA8[D_80095750]);
                }
                pos.x = coord.coord.t[0] - D_800A7308[0];
                pos.y = coord.coord.t[1] - 50;
                pos.z = coord.coord.t[2] - D_800A7308[2];
                if (func_8002D0F0(&pos)) {
                    m = n;
                    func_800414EC(m);
                    switch (m) {
                        case 0:
                            break;
                        case 1:
                        default:
                            func_80042538(0x35);
                            func_8003F834(7, ((Ent8 *)(k * 8 + (u32)D_800959C4))->unk0,
                                          ((Ent8 *)(k * 8 + (u32)D_800959C4))->unk2 - 50,
                                          (s16)((Ent8 *)(k * 8 + (u32)D_800959C4))->unk4, 0);
                            break;
                        case 2:
                            func_80042538(0x35);
                            func_8003F834(8, ((Ent8 *)(k * 8 + (u32)D_800959C4))->unk0,
                                          ((Ent8 *)(k * 8 + (u32)D_800959C4))->unk2 - 50,
                                          (s16)((Ent8 *)(k * 8 + (u32)D_800959C4))->unk4, 0);
                            break;
                    }
                    ((Ent8 *)(k * 8 + (u32)D_800959C4))->unk6 |= 0x8000;
                }
            }
        }
    }
}

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

/** @brief Resets the game state: clears the 0x30000-byte block at `recs`
 *         and frees its 200 Rec48 records, clears and resets the block
 *         header, the Rec5C, Rec48 and Rec3C tables, and the progress
 *         block. */
void func_8002D2C0(Rec48 *recs) {
    u32 i;

    bzero((u8 *)recs, 0x30000);
    for (i = 0; i < 200; i++) {
        recs->unk34 = -1;
        recs->unk36 = -1;
        recs++;
    }
    bzero((u8 *)0x801FD000, 0x800);
    func_800337E4((u8 *)0x801FD000);
    bzero((u8 *)D_800CF080, sizeof(Rec5C) * 200);
    func_80033854(D_800CF080);
    bzero((u8 *)sRecs48, sizeof(Rec48) * 200);
    func_800338A0(sRecs48);
    bzero((u8 *)D_800A7898, sizeof(Rec3C) * 100);
    func_8003390C(D_800A7898);
    sProgress.unk0 = 0x38;
    sProgress.unk1 = 1;
    sProgress.unk68 = 0;
    sProgress.unk69 = 0;
    sProgress.unk4 = 50;
    sProgress.unk8 = 0;
    sProgress.unkC = 0;
    sProgress.unk12[8] = 0;
    sProgress.unk12[9] = 0;
    sProgress.unk12[10] = 0;
    sProgress.unk12[0] = 0;
    sProgress.unk12[1] = 0;
    sProgress.unk12[2] = 0;
    sProgress.unk12[3] = 0;
    sProgress.unk12[4] = 0;
    sProgress.unk12[6] = 0;
    sProgress.unk12[5] = 0;
    sProgress.unk12[7] = 0;
    sProgress.unk12[11] = 0;
    sProgress.unk12[12] = 0;
    sProgress.unk12[13] = 0;
    sProgress.unk12[14] = 600;
    sProgress.unk12[15] = 0;
    sProgress.unk12[16] = 0;
    sProgress.unk12[17] = 0;
    sProgress.unk6A = 0;
    sProgress.unk6C = 0;
    sProgress.unk6D = 0;
    D_80095824 = 0;
    D_800958D8 = 0;
    D_800959D8 = 0;
    sProgress.unk2 = D_80095830;
}
