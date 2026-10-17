#include "common.h"
#include "ram.h"
#include "libetc.h"
#include "memory.h"
#include "libapi.h"
#include "libgte.h"
#include "sys/file.h"
#include "libgpu.h"
#include "libgs.h"
#include "object.h"
#include "model.h"
#include "player.h"
#include "draw.h"
#include "sound.h"

/** @brief An object whose current position and halfword triple are reset
 *         from a stored copy. */
typedef struct {
    s32 x;           /**< position x; reset from homeX */
    s32 y;           /**< position y; reset from homeY */
    s32 z;           /**< position z; reset from homeZ */
    s32 homeX;       /**< where it was placed */
    s32 homeY;       /**< where it was placed */
    s32 homeZ;       /**< where it was placed */
    u16 rotX;        /**< rotation; reset from homeRotX */
    u16 rotY;        /**< rotation; reset from homeRotY */
    u16 rotZ;        /**< rotation; reset from homeRotZ */
    u16 homeRotX;    /**< its placed rotation */
    u16 homeRotY;    /**< its placed rotation */
    u16 homeRotZ;    /**< its placed rotation */
    u8 unk24[4];     /**< not yet known */
    s32 block;       /**< the block it was placed in; passed to the lookup and updated from it */
    s32 lightAngle;  /**< zeroed on a reset; an angle from the light lookup */
    s32 lightAngle2; /**< zeroed on a reset; a second angle from the light lookup */
} Obj34;

/** @brief An object with a word at 0x28 that a lookup updates. */
typedef struct {
    u8 unk0[0x28]; /**< not yet known */
    s32 block;     /**< passed to the lookup and updated from it */
} Obj2C;

/** @brief The 0x30-byte argument block of a position query: collision's
 *         Sphere. */
typedef struct {
    Vec3 pos;       /**< the position asked about */
    u8 unkC[8];     /**< not set by the range-50 caller */
    s32 radius;     /**< the query's radius, summed with the other's */
    u8 kind;        /**< the hit kind it reports: a HitReaction row */
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

/** @brief A 16-byte directory entry of a loaded file. */
typedef struct {
    s32 offset;   /**< byte offset of the entry from the directory */
    u8 unk4[0xA]; /**< not yet known */
    u16 count;    /**< entry count; read from the first entry only */
} DirEnt16;

/** @brief A stage sprite (gStageSprites, 100 of them) (editor has the same
 *         record, with fewer fields known). */
typedef struct {
    u16 id;         /**< its sprite; -1 when free; one on when a HIT ACT sprite is hit */
    u8 unk2[2];     /**< not yet known */
    s32 x;          /**< x */
    s32 y;          /**< y */
    s32 z;          /**< z */
    s32 driftCount; /**< zeroed when hit: StepDrift's count */
    s32 driftBaseY; /**< the player's y when hit: StepDrift's base */
    s32 driftPhase; /**< zeroed when hit: StepDrift's phase */
    u16 width;      /**< its drawn width */
    u16 height;     /**< its drawn height; the query is centred half of it lower */
    s32 radius;     /**< a third of its magnitude is the query's range */
    u8 queried;     /**< set to 1 before the query */
    u8 hit;         /**< bit 0 of the query's result */
    u8 flags;       /**< SpriteFlag bits and a SpriteAct */
    u8 unk27[5];    /**< not yet known */
    s16 switchNo;   /**< the switch that shows it */
    u8 unk2E[0xE];  /**< not yet known */
} StageSprite;

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

/** @brief The Trap table, copied whole. */
typedef struct {
    s32 w[sizeof(Trap) * 200 / 4]; /**< the records */
} Recs48Copy;

/** @brief The SwitchBox table, copied whole. */
typedef struct {
    s32 w[sizeof(SwitchBox) * 200 / 4]; /**< the records */
} Recs5CCopy;

/** @brief The StageSprite table, copied whole. */
typedef struct {
    s32 w[sizeof(StageSprite) * 100 / 4]; /**< the records */
} Recs3CCopy;

/** @brief The block header area, copied whole; bytes, so a copy of it
 *         tests the alignment at run time. */
typedef struct {
    u8 b[0x800]; /**< the area */
} BlockCopy;

/** @brief A saved copy of the record tables and the block header. */
typedef struct {
    Recs48Copy recs48; /**< the Trap table */
    Recs5CCopy recs5C; /**< the SwitchBox table */
    Recs3CCopy recs3C; /**< the StageSprite table */
    BlockCopy block;   /**< the block header area */
} SavedTables;

/** @brief The editor's state block (editor's EditorState), as far as the
 *         full reset writes it. */
typedef struct {
    u8 magic;      /**< EDITOR_MAGIC after a reset, the mark a saved slot is checked for */
    u8 unk1;       /**< 1 after a reset */
    u8 stage;      /**< gStage, on a reset */
    u8 kind;       /**< not written here */
    s32 grid;      /**< the move step; 50 after a reset */
    s32 height;    /**< the cursor's height above the ground */
    s32 camHeight; /**< the camera's height offset */
    u8 unk10[2];   /**< not yet known */
    s16 values[18]; /**< switchNo to spriteFlip, as one array; [14], spriteSize, is 600 after a reset */
    u8 unk36[0x32];  /**< not yet known */
    u8 trapType;     /**< "TRAP SEL" TYPENO. */
    u8 trapAct;      /**< "TRAP SEL" ACT NO. */
    s16 spriteAngle; /**< the sprite's angle */
    u8 spriteWalk;   /**< "SPRITE SEL" WALK. */
    u8 spriteSe;     /**< "SPRITE SEL" SE. */
} EditorReset;

extern s16 sFloorHit[]; /**< FindFloorNear's result: a height, then a normal */
/* MATCHING: copied whole as Quad16 here; draw reads its fields. */
extern Quad16 gSpriteDefs[];  /**< a table of eight-byte entries */
extern SwitchBox gSwitches[]; /**< 200 SwitchBox records */
extern u8 gCanState[];        /**< each can: 0 hidden, 1 to pop in, 2 shown */
/* MATCHING: a per-unit view; stage reads the same path as Edge. */
extern PathPt *gSectionEdges;       /**< the current path */
extern StageSprite gStageSprites[]; /**< 100 StageSprite records */
extern s32 gCurBlock;               /**< zeroed by the full reset */
extern u8 gEditorPicking;           /**< zeroed by the full reset */
extern u8 gEditorInfo;              /**< zeroed by the full reset */

/* The unpacked header of the last TIM loaded; common.h declares a word. */
#define sTim ((TimInfo *)gTimInfo)
/* The Trap table; common.h declares it as words. */
#define sTraps ((Trap *)gTraps)
/* MATCHING: the progress block, declared per unit; editor views the same
 * bytes as other records. */
extern u8 gEditorState[]; /**< the editor's state; this unit reads its EditorReset view */
#define sEditorReset (*(EditorReset *)gEditorState)

/* MATCHING: a per-unit view; collision calls the same 0x30-byte block Body. */
s32 CollidePlayer(Query30 *q);
/* MATCHING: all s32 where the callee has s16: retail neither re-extends
 * the result nor extends the a and b it passes. */
s32 Lerp(s32 a, s32 b, u16 t, u16 n);
void RefreshAllZones(void);
s32 FindFloor(Trap *r);
s32 ClearObjectHits(void);
/* MATCHING: a per-unit view of the squared-distance helper; it takes two
 * VECTOR pointers. */
s32 DistanceSquared(void *a, void *b);
/* MATCHING: stage defines x..n as s16; this unit's calls pass them
 * unextended, so its prototype takes s32. */
s32 SpawnEffect(s32 kind, s32 x, s32 y, s32 z, s32 delay);
void TransformByCoord(GsCOORDINATE2 *coord, SVECTOR *pos, VECTOR *out);
void DrawObjectModel(ObjectModel *rec, Trap *r);
void DrawObjectSprite(u16 id, StageSprite *r);
s32 StepDrift(Drifter *d);
void PolarToXZ(s32 r, s32 deg, Vec3 *out);
/* MATCHING: editor's resets, declared per unit: StageSprite is local to each
 * unit until a shared header holds it. */
void BuildEmptyBlock(u8 *buf);
void ClearSwitches(SwitchBox *recs);
void ResetTraps(Trap *recs);
void ClearStageSprites(StageSprite *recs);

/** @brief Sets `p->pos` to the world position of its local position. */
/* MATCHING: the unused pair puts flag at sp+0x70 and the frame at 0x88. */
void UpdatePlacedPos(Placed2C *p) {
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
void LocalToWorld(LocalPos *lp, VECTOR *out) {
    MATRIX world;
    MATRIX local;
    SVECTOR v;
    VECTOR t;
    s32 unused[2];
    long flag;

    v.vx = lp->x;
    v.vy = lp->y;
    v.vz = lp->z;
    GsGetLws(&gTodCoords[lp->coord], &local, &world);
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
s32 QueryFloor(FloorHit *out, s32 zone, VECTOR *pos, FloorHdr *hdr) {
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
s32 QueryWall(s32 zone, VECTOR *pos, FloorHdr *hdr) {
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

/* MATCHING: stage defines x and y as s16; this unit passes them
 * unextended. */
void StepFloorIndex(s32 *index, s32 x, s32 y);

s32 CheckForcedThrowBack(void);

/** @brief Probes 50 units ahead of the player, 45 degrees either side of
 *         the heading; each probe that hits pushes the player 25 units
 *         back from that side. Then runs the mode check. */
void PushPlayerFromWalls(void) {
    VECTOR pos;
    s32 idx;

    pos.vx = gPlayer.x + (rsin(gCourse.rot.vy + 0x200) * 50 >> 12);
    pos.vy = gPlayer.y;
    pos.vz = gPlayer.z + (rcos(gCourse.rot.vy + 0x200) * 50 >> 12);
    idx = gFloorIndex;
    StepFloorIndex(&idx, pos.vx, pos.vz);
    if (QueryWall(idx, &pos, (FloorHdr *)gFloorData)) {
        gPlayer.x -= rsin(gCourse.rot.vy + 0x400) * 25 >> 12;
        gPlayer.z -= rcos(gCourse.rot.vy + 0x400) * 25 >> 12;
    }
    pos.vx = gPlayer.x + (rsin(gCourse.rot.vy - 0x200) * 50 >> 12);
    pos.vy = gPlayer.y;
    pos.vz = gPlayer.z + (rcos(gCourse.rot.vy - 0x200) * 50 >> 12);
    idx = gFloorIndex;
    StepFloorIndex(&idx, pos.vx, pos.vz);
    if (QueryWall(idx, &pos, (FloorHdr *)gFloorData)) {
        gPlayer.x -= rsin(gCourse.rot.vy - 0x400) * 25 >> 12;
        gPlayer.z -= rcos(gCourse.rot.vy - 0x400) * 25 >> 12;
    }
    CheckForcedThrowBack();
}

/** @brief Throws the player back (ACT_THROWN_BACK_START) and takes a
 *         bonus level, when the stage parameter's bits 4-5 are 1 and the
 *         player is not already thrown back.
 *  @return nothing; the value is undefined. */
s32 CheckForcedThrowBack(void) {
    /* MATCHING: non-void with no return keeps the second branch's delay
     * slot a nop. */
    if ((((u32)gStageParams >> 4) & 3) == 1 && gPlayer.action != ACT_THROWN_BACK) {
        gPlayer.action = ACT_THROWN_BACK_START;
        AddBonusLevel(-1);
    }
}

/** @brief The magnitude of `x`. */
/* MATCHING: this ternary is cc1's one-insn abs (abssi2), a bgez whose slot is a nop. */
static __inline__ s32 absInt(s32 x) {
    return x >= 0 ? x : -x;
}

/** @brief Places and sorts the model of `rec` for record `r`: position
 *         plus the world offset, rotation from the record's angles, light
 *         and scale; sorted far (GsSortObject4J) when x or z is beyond 700,
 *         and the offset taken off again afterwards. */
void DrawObjectModel(ObjectModel *rec, Trap *r) {
    GsCOORD2PARAM param;
    MATRIX mat;
    Model70 *m;
    Obj34 *o;

    m = (Model70 *)rec;
    o = (Obj34 *)r;
    m->coord.coord.t[0] = o->x;
    m->coord.coord.t[1] = o->y;
    m->coord.coord.t[2] = o->z;
    m->rot.vx = o->rotX;
    m->rot.vy = o->rotY;
    m->rot.vz = o->rotZ;
    m->coord.coord.t[0] += gWorldShift[0];
    m->coord.coord.t[2] += gWorldShift[2];
    SetCoordRotation(&m->rot, &m->coord);
    RotMatrixX(o->lightAngle, &m->coord.coord);
    RotMatrixZ(o->lightAngle2, &m->coord.coord);
    m->obj.attribute = 0;
    m->obj.coord2->flg = 0;
    m->obj.coord2->param = &param;
    GsGetLw(m->obj.coord2, &mat);
    GsSetLightMatrix(&mat);
    GsGetLs(m->obj.coord2, &mat);
    param.scale.vx = m->scale.vx;
    param.scale.vy = m->scale.vy;
    param.scale.vz = m->scale.vz;
    ScaleMatrix(&mat, &param.scale);
    GsSetLsMatrix(&mat);
    if (gGameMode == GM_HIT_TOOL) {
        GsSortObject4(&m->obj, &gOt[gActiveBuff], 2, getScratchAddr(0));
    } else if (absInt(m->coord.coord.t[0]) > 700 || absInt(m->coord.coord.t[2]) > 700) {
        GsSortObject4J(&m->obj, &gOt[gActiveBuff], 2, getScratchAddr(0));
    } else {
        GsSortObject4(&m->obj, &gSubOt[gActiveBuff], 2, getScratchAddr(0));
    }
    m->coord.coord.t[0] -= gWorldShift[0];
    m->coord.coord.t[2] -= gWorldShift[2];
}

/* The result of a second-buffer record's test: a flag, then a word. */
extern s8 sQuadHit;      /**< 1 on a hit */
extern s32 sQuadHitWord; /**< the word reported with a hit */
/* collision's tests of a 0x4C-byte record; types not yet known. */
/* MATCHING: s32 (its body leaves a value); void moves a register. */
s32 GetQuadHeightUnderPlayer(void *rec, s8 *out);

/** @brief Updates the entry `rec` and places and tests its records: the
 *         first-buffer ones collect a hit bit into `r->touched`, and, when
 *         the game head allows, the second-buffer ones report a hit to it.
 *  @return nothing; the value is undefined. */
/* MATCHING: non-void with no return keeps two delay slots nops (the
 * unk2 test and the second loop's back branch). */
s32 UpdateObjectEntry(ObjectModel *rec, Trap *r) {
    s16 i;
    s16 start;
    s16 n;
    Placed2C *p;
    u8 *q;

    DrawObjectModel(rec, r);
    start = rec->pointFirst;
    n = rec->pointCount;
    r->touched = 0;
    if (n != -1) {
        for (i = start; i < start + n; i++) {
            p = &((Placed2C *)gLivePoints)[i];
            UpdatePlacedPos(p);
            r->touched |= CollidePlayer((Query30 *)p) & 1;
        }
    }
    if (gPlayer.hittable != 0) {
        start = rec->quadFirst;
        n = rec->quadCount;
        if (n != -1) {
            for (i = start; i < start + n; i++) {
                q = gLiveQuads + i * 0x4C;
                PlaceCollisionQuad(q);
                GetQuadHeightUnderPlayer(q, &sQuadHit);
                if (sQuadHit == 1) {
                    gPlayer.onRideQuad = sQuadHit;
                    gPlayer.rideQuadY = sQuadHitWord;
                }
            }
        }
    }
}

/** @brief Sets the four corners of a box `w` wide and `d` deep around the
 *         position `c`, at its height, relative to it. */
void SetBoxAround(s16 (*v)[3], VECTOR *c, s16 w, s16 d) {
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
s32 TestBoxes(void) {
    MATRIX world;
    MATRIX local;
    SVECTOR sv;
    VECTOR t;
    s32 unused[2];
    GsCOORDINATE2 coord;
    long flag;
    u16 i;
    s16 j;
    SwitchBox *b;
    s16(*v)[3];
    s16 px;
    s16 pz;

    if (gMissState > MISS_NONE) {
        return;
    }
    i = 0;
    if (gInEditor == 0) {
        for (; i < 200; i++) {
            b = &gSwitches[(s16)i];
            v = gSwitches[(s16)i].v;
            if (b->state != SWITCH_ARMED) {
                continue;
            }
            px = gPlayer.x - b->x;
            pz = gPlayer.z - b->z;
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
            b->state = SWITCH_ON;
        }
    } else {
        for (; i < 200; i++) {
            if (gSwitches[(s16)i].state != SWITCH_ARMED) {
                continue;
            }
            GsInitCoordinate2(WORLD, &coord);
            coord.coord.t[0] = gSwitches[(s16)i].x;
            coord.coord.t[1] = gSwitches[(s16)i].y;
            coord.coord.t[2] = gSwitches[(s16)i].z;
            gSwitches[(s16)i].coord = &coord;
            GsGetLws(gSwitches[(s16)i].coord, &local, &world);
            GsSetLsMatrix(&local);
            for (j = 0; j < 4; j++) {
                sv.vx = gSwitches[(s16)i].v[j][0];
                sv.vy = gSwitches[(s16)i].v[j][1];
                sv.vz = gSwitches[(s16)i].v[j][2];
                RotTrans(&sv, &t, &flag);
                gSwitches[(s16)i].w[j][0] = t.vx + gWorldShift[0];
                gSwitches[(s16)i].w[j][1] = t.vy;
                gSwitches[(s16)i].w[j][2] = t.vz + gWorldShift[2];
            }
            GsSetLsMatrix(&world);
            GetQuadHeightUnderPlayer(gSwitches[(s16)i].v, &sQuadHit);
            if (sQuadHit == 1) {
                gSwitches[(s16)i].state = SWITCH_ON;
            }
        }
    }
}

/** @brief Moves `u` to the next or previous path segment once it has
 *         passed the next point or not yet reached its own.
 *  @return the new segment */
/* MATCHING: the unused pair gives the leaf its 8-byte frame. */
s32 UpdatePathSegment(PathUser *u) {
    s32 unused[2];
    s32 i;
    s32 d;
    PathPt *next;
    PathPt *pt;

    /* MATCHING: two point locals and a sum local; one reused point pointer
     * moves the parameter out of $a0. Integer sums put the index first. */
    i = u->seg;
    next = (PathPt *)(i * 8 + (u32)gSectionEdges) + 1;
    d = next->dx * (u->x - next->x) + next->dz * (u->z - next->z);
    if (d >= 0) {
        i++;
    }
    pt = (PathPt *)(i * 8 + (u32)gSectionEdges);
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
s16 GetPathDirection(PathUser *u) {
    s32 unused[2];
    s32 i;
    s32 d;
    PathPt *next;
    PathPt *pt;
    PathPt *seg;

    i = u->seg;
    next = (PathPt *)(i * 8 + (u32)gSectionEdges) + 1;
    d = next->dx * (u->x - next->x) + next->dz * (u->z - next->z);
    if (d >= 0) {
        i++;
    }
    pt = (PathPt *)(i * 8 + (u32)gSectionEdges);
    d = -pt->dx * (u->x - pt->x) + -pt->dz * (u->z - pt->z);
    if (d >= 0) {
        i--;
    }
    seg = (PathPt *)(i * 8 + (u32)gSectionEdges);
    return ratan2(seg[1].x - seg->x, seg[1].z - seg->z);
}

void DrawAxes(void);

/** @brief A trap viewer frame: fixed view, no fog, steps the selected
 *         ObjectModel entry with the pad, places and draws it on a turning
 *         record, draws the axes and prints the entry number. */
void UpdateTrapViewer(void) {
    s32 flags;

    gWorldShift[0] = 0;
    gWorldShift[2] = 0;
    gRefView[0] = 100;
    gRefView[1] = -200;
    gRefView[2] = 1000;
    gRefView[3] = 0;
    gRefView[4] = 0;
    gRefView[5] = 0;
    flags = gPadTrig;
    gClearB = 0;
    gClearG = 0;
    gClearR = 0;
    if (flags & PADR2) {
        gSelectedSection++;
    }
    if (flags & PADL2) {
        gSelectedSection--;
    }
    gSelectedSection = gSelectedSection < 0                   ? 0
                       : gSelectedSection > gSectionCount - 1 ? gSectionCount - 1
                                                              : gSelectedSection;
    ResetCollision();
    gTraps[0] = 0;
    gTraps[1] = 0;
    gTraps[2] = 0;
    ((s16 *)gTraps)[12] = 0;
    ((s16 *)gTraps)[13] = gFrameCount % 360 * 4096 / 360;
    ((s16 *)gTraps)[14] = 0;
    UpdateObjectEntry(&gObjectModels[gSelectedSection], (Trap *)gTraps);
    ReactToHits(&gPlayer);
    LeaveQuadTop();
    DrawAxes();
    FntPrint(gTrapNoFmt, gSelectedSection, gSectionCount);
}

/** @brief A Trap record seen as an Obj34 followed by its indices. */
typedef struct {
    Obj34 o;        /**< position, stored copy and angles */
    s16 switchNo;   /**< the switch that triggers it */
    s16 model;      /**< the ObjectModel entry it belongs to */
    u8 unk38[0x10]; /**< not yet known */
} TrapPlaced;

/* MATCHING: per-unit views of editor's placement record, count and filler. */
extern Obj34 gPlaceQueue; /**< the record the placement starts from */
extern u16 gPlaceCount;   /**< how many records the placement filled */
u16 PlaceStageTrap(s16 arg0, s32 arg1, s32 arg2);

/** @brief The trap viewer's step: state 0 sets the fixed view, state 1
 *         fills the Trap table from a turning start record, state 2 steps
 *         the selected entry with the pad and places every record
 *         relative to record 0; every state prints the entry number. */
void UpdateTrapLayoutViewer(void) {
    s32 unused[18];
    s32 flags;
    s16 old;
    u16 i;
    TrapPlaced *recs;
    TrapPlaced *r;
    ObjectModel *tbl;

    switch (gModeStep) {
        case 0:
            gWorldShift[0] = 0;
            gWorldShift[2] = 0;
            gRefView[0] = 100;
            gRefView[1] = -200;
            gRefView[2] = 1000;
            gRefView[3] = 0;
            gRefView[4] = 0;
            gRefView[5] = 0;
            gClearB = 0;
            gClearG = 0;
            gClearR = 0;
            gModeStep = 1;
            break;
        case 1:
            gPlaceQueue.x = 0;
            gPlaceQueue.y = 0;
            gPlaceQueue.z = 0;
            gPlaceQueue.rotX = 0;
            gPlaceQueue.rotY = gFrameCount % 360 * 4096 / 360;
            gPlaceQueue.rotZ = 0;
            gPlaceCount = PlaceStageTrap(gSelectedSection, (s32)&gPlaceQueue, (s32)gTraps);
            gModeStep++;
            break;
        case 2:
            flags = gPadTrig;
            old = gSelectedSection;
            if (flags & PADR2) {
                gSelectedSection++;
            }
            if (flags & PADL2) {
                gSelectedSection--;
            }
            gSelectedSection = gSelectedSection < 0                   ? 0
                               : gSelectedSection > gSectionCount - 1 ? gSectionCount - 1
                                                                      : gSelectedSection;
            if (gSelectedSection != old) {
                gModeStep = 1;
            }
            ResetCollision();
            ((TrapPlaced *)gTraps)->o.rotY = gFrameCount % 360 * 4096 / 360;
            UpdateObjectEntry(&gObjectModels[((TrapPlaced *)gTraps)->model], (Trap *)gTraps);
            for (i = 1; i < gPlaceCount; i++) {
                /* MATCHING: both bases assigned in the loop, hoisted after the guard. */
                recs = (TrapPlaced *)gTraps;
                r = (TrapPlaced *)(i * 72 + (u32)recs);
                gLocalPos.vx = r->o.homeX;
                gLocalPos.vy = r->o.homeY;
                gLocalPos.vz = r->o.homeZ;
                TransformByCoord((GsCOORDINATE2 *)gObjectModels[recs->model].coord, &gLocalPos,
                                 &gWorldPos);
                r->o.x = gWorldPos.vx;
                r->o.y = gWorldPos.vy;
                r->o.z = gWorldPos.vz;
                r->o.rotY = recs->o.rotY + r->o.homeRotY;
                tbl = gObjectModels;
                UpdateObjectEntry((ObjectModel *)(r->model * 120 + (u32)tbl), (Trap *)r);
            }
            ReactToHits(&gPlayer);
            LeaveQuadTop();
            DrawAxes();
            break;
    }
    FntPrint(gTrapNoFmt, gSelectedSection, gSectionCount);
}

/** @brief Sorts three white lines 400 units long through the origin, one
 *         along each axis, into the current ordering table. */
void DrawAxes(void) {
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
    ProjectToScreen(&world, &screen);
    line.x0 = screen.vx;
    line.y0 = screen.vy;
    world.vx = 0;
    world.vy = 200;
    world.vz = 0;
    ProjectToScreen(&world, &screen);
    line.x1 = screen.vx;
    line.y1 = screen.vy;
    GsSortLine(&line, &gSubOt[gActiveBuff], 50);
    world.vx = -200;
    world.vy = 0;
    world.vz = 0;
    ProjectToScreen(&world, &screen);
    line.x0 = screen.vx;
    line.y0 = screen.vy;
    world.vx = 200;
    world.vy = 0;
    world.vz = 0;
    ProjectToScreen(&world, &screen);
    line.x1 = screen.vx;
    line.y1 = screen.vy;
    GsSortLine(&line, &gSubOt[gActiveBuff], 50);
    world.vx = 0;
    world.vy = 0;
    world.vz = -200;
    ProjectToScreen(&world, &screen);
    line.x0 = screen.vx;
    line.y0 = screen.vy;
    world.vx = 0;
    world.vy = 0;
    world.vz = 200;
    ProjectToScreen(&world, &screen);
    line.x1 = screen.vx;
    line.y1 = screen.vy;
    GsSortLine(&line, &gSubOt[gActiveBuff], world.vz >> 2);
}

/** @brief Loads every image of the directory `dir` and registers each as a
 *         texture, numbered from `id` on. */
void LoadSpritePack(DirEnt16 *dir, u16 id) {
    DirEnt16 *e;
    u16 i;
    u16 n;
    s32 tp;
    TimInfo *t;

    e = dir;
    n = dir->count;
    for (i = 0; i < n; i++) {
        LoadTim((u8 *)dir + e->offset);
        DrawSync(0);
        tp = GetTPage(0, 0, sTim->pixRect.x, sTim->pixRect.y);
        t = sTim;
        /* MATCHING: x mod 64 spelled out; % narrows to a halfword. */
        DefineSprite(id, 0, (u8)t->width, (u8)t->pixRect.h, (u8)tp,
                     (u8)((t->pixRect.x - t->pixRect.x / 64 * 64) * t->perUnit),
                     (u8)(t->pixRect.y % 256), t->clutRect.x, (u16)t->clutRect.y);
        id++;
        e++;
    }
}

extern u8 sSpriteShapes[]; /**< a sprite shape per id, from SPR_STAGE on */

/** @brief Draws the sprite of record `r` with id `id` at the record's
 *         position plus the world offset: the next id when the record is
 *         in state 1 within 600 units (and the mode is not 0x20), a size
 *         from its width and height by the id's shape, mirrored by flag
 *         bit 6, sorted near within 700 units and far beyond. */
void DrawObjectSprite(u16 id, StageSprite *r) {
    GsCOORDINATE2 coord;
    MATRIX mat;
    SVECTOR size;
    u16 k;

    GsInitCoordinate2(WORLD, &coord);
    k = id;
    coord.coord.t[0] = r->x + gWorldShift[0];
    coord.coord.t[1] = r->y;
    coord.coord.t[2] = r->z + gWorldShift[2];
    coord.flg = 0;
    GsGetLs(&coord, &mat);
    GsSetLsMatrix(&mat);
    if (gGameMode != GM_EDITOR && (r->flags & SPRITE_ACT_MASK) == SPRITE_ACT_NEAR &&
        absInt(r->x + gWorldShift[0]) < 600 && absInt(r->z + gWorldShift[2]) < 600) {
        k = id + 1;
    }
    switch (sSpriteShapes[k - SPR_STAGE]) {
        case 0:
            size.vx = (s16)r->width / 2 + (s16)r->width / 3;
            size.vy = (s16)r->height / 2 + (s16)r->height / 4;
            break;
        case 1:
            size.vx = (s16)r->width / 2 + (s16)r->width / 4;
            size.vy = r->height;
            break;
        case 2:
            size.vx = r->width + (s16)r->width / 3;
            size.vy = (s16)r->height / 8 * 4;
            break;
        case 3:
            size.vx = r->width;
            size.vy = r->height;
            break;
    }
    if (r->flags & SPRITE_MIRROR) {
        size.vx *= -1;
    }
    if (absInt(r->x + gWorldShift[0]) > 700 || absInt(r->z + gWorldShift[2]) > 700) {
        DrawStandingBillboardFog(k, &size, NULL, 2, &gOt[gActiveBuff]);
    } else {
        DrawStandingBillboard(k, &size, NULL, 2, &gSubOt[gActiveBuff]);
    }
}

/** @brief Updates the ObjectModel entry of every live Trap record whose SwitchBox
 *         is SWITCH_ON. */
void UpdateActiveEntries(void) {
    u32 i;

    for (i = 0; i < 200; i++) {
        if (sTraps[i].model != -1 && gSwitches[sTraps[i].switchNo].state == SWITCH_ON) {
            UpdateObjectEntry(&gObjectModels[sTraps[i].model], &sTraps[i]);
        }
    }
}

/** @brief Updates `r` (id `id`) and queries around it; a hit in state 2
 *         of a flagged record moves it to state 3. Inlined here and by the
 *         out-of-line copy below. */
static __inline__ void queryRec(s16 id, StageSprite *r) {
    Query30 q;
    s32 v;

    DrawObjectSprite(id, r);
    q.pos.x = r->x;
    q.pos.y = r->y - (s16)r->height / 2;
    q.pos.z = r->z;
    r->queried = 1;
    q.kind = 1;
    q.radius = (r->radius < 0 ? -r->radius : r->radius) / 3;
    v = CollidePlayer(&q) & 1;
    r->hit = v;
    if (v) {
        if (r->flags & SPRITE_HITTABLE) {
            if ((r->flags & SPRITE_ACT_MASK) == SPRITE_ACT_HIT) {
                r->driftCount = 0;
                r->id++;
                r->flags = (r->flags & SPRITE_MIRROR) | SPRITE_ACT_DRIFT;
                r->driftBaseY = gPlayer.y;
                r->driftPhase = 0;
            }
        }
    }
}

/** @brief Steps every live StageSprite record whose switch is SWITCH_ON:
 *         a flagged one is updated and queried, a state-3 one drifts and is
 *         updated on a drift step, any other one is updated. */
/* MATCHING: the record pointer is taken inside the body. */
void UpdateActiveSprites(void) {
    u32 i;
    StageSprite *r;
    s8 k;

    for (i = 0; i < 100; i++) {
        r = &gStageSprites[i];
        if ((s16)r->id != -1) {
            k = gSwitches[r->switchNo].state;
            if (k == SWITCH_ON) {
                if (r->flags & SPRITE_HITTABLE) {
                    queryRec(r->id, r);
                } else if ((r->flags & SPRITE_ACT_MASK) != SPRITE_ACT_DRIFT ||
                           (s8)StepDrift((Drifter *)r) == 1) {
                    DrawObjectSprite(r->id, r);
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
s32 StepDrift(Drifter *d) {
    s32 b;
    u8 ret;
    s32 y;

    if (d->count < 15) {
        d->count++;
        ret = d->count % 3 != 0;
        PolarToXZ(20, gCourse.rot.vy * 360 / 4096, (Vec3 *)gOffsetVec);
        d->x -= ((Vec3 *)gOffsetVec)->x >> 12;
        d->z -= ((Vec3 *)gOffsetVec)->z >> 12;
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
s32 LoadStageSprites(void) {
    DirEnt16 *e;
    s32 *p;

    /* MATCHING: non-void with no return orders the loop preheader; the
     * pointer is assigned inside the store so the index loads first. */
    e = (DirEnt16 *)(RAM_WORK_A + 0x10708);
    gLoopI = 0x33;
    gLoopJ = e->count;
    for (; gLoopI < gLoopJ + 0x33; gLoopI++) {
        gAnimations[gLoopI] = (p = (s32 *)((u8 *)(RAM_WORK_A + 0x10708) + e->offset)) + 1;
        e++;
        gPlayer.motionLen[gLoopI] = *gAnimations[gLoopI];
        gAnimations[gLoopI]++;
        gPlayer.motionStart[gLoopI] = gAnimations[gLoopI][1];
    }
}

/** @brief Looks up the height under `p`; on success sets its current and
 *         stored y and two angles from the result.
 *  @return the lookup's result, -1 when it failed */
s32 SnapToFloor(Obj34 *p) {
    s32 v;

    v = FindFloorNear(p, sFloorHit, p->block);
    if (v != -1) {
        p->block = v;
        p->y = p->homeY = sFloorHit[0];
        p->lightAngle = ratan2(-sFloorHit[3], sFloorHit[2]);
        p->lightAngle2 = ratan2(-sFloorHit[1], sFloorHit[2]);
    }
    return v;
}

/** @brief Sets x and z of `out` to the point `r` away at `deg` degrees. */
void PolarToXZ(s32 r, s32 deg, Vec3 *out) {
    s32 a;

    /* MATCHING: deg is s32, narrowed here; StepDrift passes it unextended. */
    a = (s16)deg * 4096 / 360;
    out->x = rsin(a) * r;
    out->z = rcos(a) * r;
}

/** @brief Sets up `m` to draw object `n` of the TMD file at `tmd`, with an
 *         identity transform, and counts it. */
void InitModel(Model70 *m, unsigned long *tmd, u8 n) {
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
    gSectionCount++;
}

/** @brief Restores the Trap, SwitchBox and StageSprite tables and the block header
 *         from their saved copy, and points the current-block globals at
 *         the restored header. */
/* MATCHING: word-typed table copies; the byte-typed header copy keeps the
 * runtime alignment test. */
void RestoreStageObjects(void) {
    SavedTables *s;

    s = (SavedTables *)(RAM_WORK_A + 0x6778);
    *(Recs48Copy *)sTraps = s->recs48;
    *(Recs5CCopy *)gSwitches = s->recs5C;
    *(Recs3CCopy *)gStageSprites = s->recs3C;
    *(BlockCopy *)RAM_CUR_BLOCK = s->block;
    SetCurrentBlock((BlockHeader *)RAM_CUR_BLOCK);
}

/** @brief Updates `p->block` from a lookup unless the lookup fails (-1). */
void RefreshZone(Obj2C *p) {
    s32 v;

    v = FindFloorNear(p, sFloorHit, p->block);
    if (v != -1) {
        p->block = v;
    }
}

/** @brief Clears every used SwitchBox record, then refreshes the Trap table
 *         and clears its records. */
void ResetStageObjects(void) {
    u32 i;

    for (i = 0; i < 200; i++) {
        if (gSwitches[i].state != SWITCH_FREE) {
            gSwitches[i].state = SWITCH_ARMED;
        }
    }
    RefreshAllZones();
    ClearObjectHits();
}

/** @brief Refreshes the block of every live trap. */
void RefreshAllZones(void) {
    u32 i;

    /* MATCHING: indexed; a walking Trap pointer is biased to block. */
    for (i = 0; i < 200; i++) {
        if (sTraps[i].model != -1) {
            sTraps[i].block = FindFloor(&sTraps[i]);
        }
    }
}

/** @brief Sets the four corners of a box `w` wide and `d` deep, centred on
 *         the origin at height 0. */
void SetBoxCentred(Box4 *b, s16 w, s16 d) {
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

/** @brief Clears byte 0 of the first `count` records of a SwitchBox table. */
void ClearMarks(SwitchBox *recs, u16 count) {
    u16 i;

    for (i = 0; i < count; i++) {
        recs->state = SWITCH_ARMED;
        recs++;
    }
}

/** @brief For each odd-numbered switch that is SWITCH_ON, turns it and
 *         the switch before it SWITCH_OFF.
 *  @return nothing; the value is undefined. */
s32 PairMarks(void) {
    u16 i;

    /* MATCHING: non-void with no return keeps the loop's delay slot a nop. */
    for (i = 1; i < 200; i += 2) {
        if (gSwitches[(s16)i].state == SWITCH_ON) {
            gSwitches[(s16)i].state = SWITCH_OFF;
            gSwitches[(s16)i - 1].state = SWITCH_OFF;
        }
    }
}

/** @brief Clears touched and timer of all 200 Trap records and all but bit 0
 *         of marks.
 *  @return nothing; the value is undefined. */
s32 ClearObjectHits(void) {
    u16 i;
    Trap *r;

    /* MATCHING: non-void with no return keeps the loop's delay slot a nop. */
    for (i = 0; i < 200; i++) {
        r = &sTraps[(s16)i];
        r->touched = 0;
        r->timer = 0;
        r->marks &= 1;
    }
}

/** @brief Latches bit 1 of `r->marks` once the record comes within `range`
 *         of a fixed object.
 *  @return 1 when the bit was set by this call, else 0. */
s32 LatchProximity(Trap *r, s32 range) {
    s32 ret;
    s32 sq;

    ret = 0;
    if (!((r->marks >> 1) & 1)) {
        sq = range * range;
        if (DistanceSquared(&gPlayer.x, r) < sq) {
            r->marks |= 2;
            ret = 1;
        }
    }
    return ret;
}

/** @brief Places `b` at its stored position in the coordinate system of
 *         `a`'s ObjectModel entry, takes `a`'s angles, adds `a`'s heading to its
 *         own, and updates `b`'s ObjectModel entry from it. */
/* MATCHING: the Traps' first 0x34 bytes read through the Obj34 view. */
void AttachToParent(Trap *a, Trap *b) {
    Obj34 *pa;
    Obj34 *pb;

    pa = (Obj34 *)a;
    pb = (Obj34 *)b;
    gLocalPos.vx = pb->homeX;
    gLocalPos.vy = pb->homeY;
    gLocalPos.vz = pb->homeZ;
    TransformByCoord((GsCOORDINATE2 *)gObjectModels[a->model].coord, &gLocalPos, &gWorldPos);
    pb->x = gWorldPos.vx;
    pb->y = gWorldPos.vy;
    pb->z = gWorldPos.vz;
    pb->lightAngle = pa->lightAngle;
    pb->lightAngle2 = pa->lightAngle2;
    pb->rotY = pa->rotY + pb->homeRotY;
    DrawObjectModel(&gObjectModels[b->model], b);
}

/** @brief Resets an object's current values from its stored copy. */
void ResetObj34(Obj34 *p) {
    p->lightAngle = 0;
    p->lightAngle2 = 0;
    p->x = p->homeX;
    p->y = p->homeY;
    p->z = p->homeZ;
    p->rotX = p->homeRotX;
    p->rotY = p->homeRotY;
    p->rotZ = p->homeRotZ;
}

/** @brief Copies entry `i` of the eight-byte table to `out`. */
void GetQuad16(u16 i, Quad16 *out) {
    Quad16 *src;

    /* MATCHING: the base in its own local, then advanced by i; indexing
     * gives the sum the index register. */
    src = gSpriteDefs;
    src += i;
    *out = *src;
}

/** @brief Runs record `r`'s step, then queries its surroundings; on a hit
 *  while flagged in state 2, moves it to state 3 and resets its motion. */
/* MATCHING: the fixed object's y read as a VECTOR member; a plain word
 * read lets the scheduler hoist the r->driftPhase store above it. */
void StepSprite(s16 id, StageSprite *r) {
    queryRec(id, r);
}

/** @brief Interpolates three angles by `t`/`n` into `out`: out[1] is `r`
 *         times the sine of 0..180 degrees, out[0] -60..60 degrees and
 *         out[2] 0..20 degrees, in 4096ths of a turn. */
void InterpSwing(s16 r, u16 t, u16 n, s32 *out) {
    out[1] = rsin((s16)Lerp(0, 180, t, n) * 4096 / 360) * r;
    out[0] = (s16)Lerp(-60, 60, t, n) * 4096 / 360;
    out[2] = (s16)Lerp(0, 20, t, n) * 4096 / 360;
}

/** @brief Interpolates from 0 towards `b` by `t`/`n`, into out[1]. */
void InterpFromZero(s16 b, u16 t, u16 n, s32 *out) {
    out[1] = Lerp(0, b, t, n);
}

/** @brief Interpolates from `a` towards `b` by `t`/`n`, into out[1]. */
void InterpRange(s16 a, s16 b, u16 t, u16 n, s32 *out) {
    out[1] = Lerp(a, b, t, n);
}

/** @brief Interpolates by `t`/`n` into out[1]: from 0 towards twice `b`
 *         in the first 80 percent of `n`, else from four times `b` towards
 *         `b`. */
void InterpOvershoot(s16 b, u16 t, u16 n, s32 *out) {
    if ((double)(t * 4096 / 100) < (double)(n * 4096 / 100) * 0.8) {
        out[1] = Lerp(0, b * 2, t, n);
    } else {
        out[1] = Lerp(b * 4, b, t, n);
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
    gSectionCount++;
}

/** @brief Sets up all 80 ObjectModel records as objects of the TMD file at a
 *         fixed address, record i drawing object i, and counts them. */
void InitEntryModels(void) {
    u32 i;

    gSectionCount = 0;
    for (i = 0; i < 80; i++) {
        setupModel((Model70 *)&gObjectModels[i], (unsigned long *)RAM_OBJECT_MODELS, i);
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

/* MATCHING: a per-unit view; stage types the same list as Run. */
extern Zone4 *gStageRuns; /**< the zones */

s8 ProbePosition(Vec3 *pos);

/** @brief Draws the markers of the current zone: each block entry marked 2
 *         whose flag is set and not yet collected gets a sprite, near or
 *         far by its distance, and when the player is over it it is
 *         collected (its effect, a sound, bit 15 of its flag index). */
void UpdatePickups(void) {
    GsCOORDINATE2 coord;
    MATRIX mat;
    SVECTOR size;
    Vec3 pos;
    s32 j;
    u32 i;
    u32 k;
    u16 n;
    CanPoint *e;
    s16 m;

    GsInitCoordinate2(WORLD, &coord);
    for (j = 0; j < gStageRuns[gSection].count; j++) {
        if (gStageRunObjs[gStageRuns[gSection].start + j] < gCanSpanCount) {
            for (i = 0;
                 i < ((Group8 *)gCanSpans)[gStageRunObjs[gStageRuns[gSection].start + j]].count; i++) {
                k = ((Group8 *)gCanSpans)[gStageRunObjs[gStageRuns[gSection].start + j]].first + i;
                if ((s8)gCanState[k] != 2) {
                    continue;
                }
                e = (CanPoint *)(k * 8 + (u32)gCanPoints);
                n = e->group;
                if ((s8)gCanGroupOn[e->group] == 0 || e->group == 0 || (e->group & CAN_COLLECTED)) {
                    continue;
                }
                coord.coord.t[0] = -gPlayer.x + e->x;
                coord.coord.t[1] = e->y;
                coord.coord.t[2] = -gPlayer.z + (s16)e->z;
                coord.flg = 0;
                GsGetLs(&coord, &mat);
                GsSetLsMatrix(&mat);
                size.vy = 75;
                size.vx = 75;
                if (absInt(coord.coord.t[0]) > 700 || absInt(coord.coord.t[2]) > 700) {
                    DrawStandingBillboardFog(SPR_CAN, &size, NULL, 2, &gOt[gActiveBuff]);
                } else {
                    DrawStandingBillboard(SPR_CAN, &size, NULL, 2, &gSubOt[gActiveBuff]);
                }
                pos.x = coord.coord.t[0] - gWorldShift[0];
                pos.y = coord.coord.t[1] - 50;
                pos.z = coord.coord.t[2] - gWorldShift[2];
                if (ProbePosition(&pos)) {
                    m = n;
                    OnCanCollected(m);
                    switch (m) {
                        case 0:
                            break;
                        case 1:
                        default:
                            PlaySe(SE_CAN);
                            SpawnEffect(EFFECT_CAN_COLLECT, ((CanPoint *)(k * 8 + (u32)gCanPoints))->x,
                                        ((CanPoint *)(k * 8 + (u32)gCanPoints))->y - 50,
                                        (s16)((CanPoint *)(k * 8 + (u32)gCanPoints))->z, 0);
                            break;
                        case 2:
                            PlaySe(SE_CAN);
                            SpawnEffect(EFFECT_FIVE_CANS, ((CanPoint *)(k * 8 + (u32)gCanPoints))->x,
                                        ((CanPoint *)(k * 8 + (u32)gCanPoints))->y - 50,
                                        (s16)((CanPoint *)(k * 8 + (u32)gCanPoints))->z, 0);
                            break;
                    }
                    ((CanPoint *)(k * 8 + (u32)gCanPoints))->group |= CAN_COLLECTED;
                }
            }
        }
    }
}

/** @brief Points the current-block globals at the block `hdr` heads. */
void SetCurrentBlock(BlockHeader *hdr) {
    gCanSpans = (u8 *)(hdr + 1);
    gCanSpanCount = hdr->spanCount;
    /* MATCHING: offset read through a byte pointer, not hdr->offset: the
     * member access lets the load rise above the gCanSpanCount store. */
    gCanPoints = (u8 *)hdr + *(s32 *)((u8 *)hdr + 4);
    gCans = 0;
}

/** @brief Runs the position query for `pos` with a range of 50. */
s8 ProbePosition(Vec3 *pos) {
    Query30 q;

    q.pos.x = pos->x;
    q.pos.y = pos->y;
    q.pos.z = pos->z;
    q.kind = 0;
    q.radius = 50;
    return CollidePlayer(&q);
}

/** @brief Hides every can group.
 *  @return nothing; the value is undefined. */
s32 ClearCanGroups(void) {
    u32 i;

    /* MATCHING: non-void with no return; it keeps $v0 live at the exit, so
     * the loop's delay slot stays a nop. */
    for (i = 0; i < 128; i++) {
        gCanGroupOn[i] = 0;
    }
}

/** @brief Shows (marks 2) every can whose group is revealed.
 *  @return nothing; the value is undefined. */
s32 ShowRevealedCans(void) {
    u32 i;

    /* MATCHING: non-void with no return keeps the loop's delay slot a nop. */
    for (i = 0; i < 200; i++) {
        if ((s8)gCanGroupOn[((CanPoint *)gCanPoints)[i].group] == 1) {
            gCanState[i] = 2;
        }
    }
}

/** @brief Reveals can group n: flags it, then marks its cans to pop in.
 *  @param n the group
 *  @return nothing; the value is undefined. */
s32 RevealCanGroup(s16 n) {
    u32 i;

    /* MATCHING: non-void with no return keeps the loop's delay slot a nop. */
    gCanGroupOn[n] = 1;
    for (i = 0; i < 200; i++) {
        if (((CanPoint *)gCanPoints)[i].group == n) {
            gCanState[i] = 1;
        }
    }
}

/** @brief On even frames, pops in the first can marked 1: effect 4 at it,
 *         then marks it shown. */
void PopInNextCan(void) {
    u32 i;
    CanPoint *e;

    if (gFrameCount & 1) {
        return;
    }
    for (i = 0; i < 200; i++) {
        if ((s8)gCanState[i] == 1) {
            /* MATCHING: an integer sum puts the scaled index first. */
            e = (CanPoint *)(i * 8 + (u32)gCanPoints);
            SpawnEffect(EFFECT_DUST, e->x, e->y - 50, (s16)e->z, 0);
            gCanState[i] = 2;
            return;
        }
    }
}

/** @brief Resets the game state: clears the 0x30000-byte block at `recs`
 *         and frees its 200 Trap records, clears and resets the block
 *         header, the SwitchBox, Trap and StageSprite tables, and the progress
 *         block. */
void ResetGameState(Trap *recs) {
    u32 i;

    bzero((u8 *)recs, 0x30000);
    for (i = 0; i < 200; i++) {
        recs->switchNo = -1;
        recs->model = -1;
        recs++;
    }
    bzero((u8 *)RAM_CUR_BLOCK, 0x800);
    BuildEmptyBlock((u8 *)RAM_CUR_BLOCK);
    bzero((u8 *)gSwitches, sizeof(SwitchBox) * 200);
    ClearSwitches(gSwitches);
    bzero((u8 *)sTraps, sizeof(Trap) * 200);
    ResetTraps(sTraps);
    bzero((u8 *)gStageSprites, sizeof(StageSprite) * 100);
    ClearStageSprites(gStageSprites);
    sEditorReset.magic = EDITOR_MAGIC;
    sEditorReset.unk1 = 1;
    sEditorReset.trapType = 0;
    sEditorReset.trapAct = 0;
    sEditorReset.grid = 50;
    sEditorReset.height = 0;
    sEditorReset.camHeight = 0;
    sEditorReset.values[8] = 0;
    sEditorReset.values[9] = 0;
    sEditorReset.values[10] = 0;
    sEditorReset.values[0] = 0;
    sEditorReset.values[1] = 0;
    sEditorReset.values[2] = 0;
    sEditorReset.values[3] = 0;
    sEditorReset.values[4] = 0;
    sEditorReset.values[6] = 0;
    sEditorReset.values[5] = 0;
    sEditorReset.values[7] = 0;
    sEditorReset.values[11] = 0;
    sEditorReset.values[12] = 0;
    sEditorReset.values[13] = 0;
    sEditorReset.values[14] = 600;
    sEditorReset.values[15] = 0;
    sEditorReset.values[16] = 0;
    sEditorReset.values[17] = 0;
    sEditorReset.spriteAngle = 0;
    sEditorReset.spriteWalk = 0;
    sEditorReset.spriteSe = 0;
    gCurBlock = 0;
    gEditorPicking = 0;
    gEditorInfo = 0;
    sEditorReset.stage = gStage;
}
