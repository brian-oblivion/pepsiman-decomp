#include "common.h"
#include "code_a0bc.h"
#include "gte.h"

/** @brief A flat, unlit TMD triangle: a header, its colour and three
 *  vertex indices. */
typedef struct {
    u32 hdr;     /**< the TMD primitive header */
    CVECTOR rgb; /**< the colour, depth-cued per frame */
    u16 v0;      /**< first vertex index */
    u16 v1;      /**< second vertex index */
    u16 v2;      /**< third vertex index */
    u16 pad;     /**< padding */
} TmdF3;

/** @brief A flat, unlit TMD quad: a header, its colour and four vertex
 *  indices. */
typedef struct {
    u32 hdr;     /**< the TMD primitive header */
    CVECTOR rgb; /**< the colour, depth-cued per frame */
    u16 v0;      /**< first vertex index */
    u16 v1;      /**< second vertex index */
    u16 v2;      /**< third vertex index */
    u16 v3;      /**< fourth vertex index */
} TmdF4;

/** @brief A gouraud unlit TMD triangle: a header, three colours and
 *  three vertex indices. */
typedef struct {
    u8 olen;    /**< the TMD header: output length */
    u8 ilen;    /**< input length */
    u8 flag;    /**< flags */
    u8 mode;    /**< the primitive code, copied into the packet */
    CVECTOR c0; /**< first vertex colour */
    CVECTOR c1; /**< second vertex colour */
    CVECTOR c2; /**< third vertex colour */
    u16 v0;     /**< first vertex index */
    u16 v1;     /**< second vertex index */
    u16 v2;     /**< third vertex index */
    u16 pad;    /**< padding */
} TmdG3;

/** @brief A gouraud unlit TMD quad: a header, four colours and four
 *  vertex indices. */
typedef struct {
    u8 olen;    /**< the TMD header: output length */
    u8 ilen;    /**< input length */
    u8 flag;    /**< flags */
    u8 mode;    /**< the primitive code, copied into the packet */
    CVECTOR c0; /**< first vertex colour */
    CVECTOR c1; /**< second vertex colour */
    CVECTOR c2; /**< third vertex colour */
    CVECTOR c3; /**< fourth vertex colour */
    u16 v0;     /**< first vertex index */
    u16 v1;     /**< second vertex index */
    u16 v2;     /**< third vertex index */
    u16 v3;     /**< fourth vertex index */
} TmdG4;

/** @brief A lit gouraud-textured TMD triangle: a header, three UV words,
 *  then a normal index and a vertex index per corner. */
typedef struct {
    u32 hdr; /**< the TMD primitive header */
    u32 uv0; /**< u0, v0 and the CLUT */
    u32 uv1; /**< u1, v1 and the texture page */
    u32 uv2; /**< u2, v2 */
    u16 n0;  /**< first normal index */
    u16 v0;  /**< first vertex index */
    u16 n1;  /**< second normal index */
    u16 v1;  /**< second vertex index */
    u16 n2;  /**< third normal index */
    u16 v2;  /**< third vertex index */
} TmdGT3;

/** @brief A lit gouraud-textured TMD quad: a header, four UV words, then a
 *  normal index and a vertex index per corner. */
typedef struct {
    u32 hdr; /**< the TMD primitive header */
    u32 uv0; /**< u0, v0 and the CLUT */
    u32 uv1; /**< u1, v1 and the texture page */
    u32 uv2; /**< u2, v2 */
    u32 uv3; /**< u3, v3 */
    u16 n0;  /**< first normal index */
    u16 v0;  /**< first vertex index */
    u16 n1;  /**< second normal index */
    u16 v1;  /**< second vertex index */
    u16 n2;  /**< third normal index */
    u16 v2;  /**< third vertex index */
    u16 n3;  /**< fourth normal index */
    u16 v3;  /**< fourth vertex index */
} TmdGT4;

/** @brief One TMD texture coordinate word: u, v and the halfword packed
 *  above them (the CLUT, the texture page, or padding). */
typedef struct {
    u8 u;   /**< u */
    u8 v;   /**< v */
    u16 hi; /**< the CLUT, the texture page, or padding */
} TmdUV;

/** @brief A flat-textured unlit TMD triangle: a header, three UV words
 *  (the CLUT, the texture page, padding in their top halves), a colour and
 *  three vertex indices. */
typedef struct {
    u8 olen;     /**< the TMD header: output length */
    u8 ilen;     /**< input length */
    u8 flag;     /**< flags */
    u8 mode;     /**< the primitive code, copied into the packet */
    TmdUV uv0;   /**< u0, v0 and the CLUT */
    TmdUV uv1;   /**< u1, v1 and the texture page */
    TmdUV uv2;   /**< u2, v2 and padding */
    CVECTOR rgb; /**< the colour, depth-cued per frame */
    u16 v0;      /**< first vertex index */
    u16 v1;      /**< second vertex index */
    u16 v2;      /**< third vertex index */
    u16 pad1;    /**< padding */
} TmdFT3;

/** @brief An eight-byte table entry: two halfwords and four bytes. */
typedef struct {
    u16 unk0; /**< a byte and a bit packed together */
    u16 unk2; /**< a CLUT-style packed position */
    u8 unk4;  /**< not yet known */
    u8 unk5;  /**< not yet known */
    u8 unk6;  /**< not yet known */
    u8 unk7;  /**< not yet known */
} Sprite8;

/* MATCHING: this unit's view of the table code_1a098 copies as halfwords. */
extern Sprite8 D_800DD0A0[];

/** @brief A 2D sprite placement: a position, the pivot it is scaled and
 *  rotated about, a 4.12 scale per axis, an angle and a centring flag. */
typedef struct {
    s16 x;       /**< screen x of the top-left corner */
    s16 y;       /**< screen y of the top-left corner */
    s16 cx;      /**< pivot x */
    s16 cy;      /**< pivot y */
    s16 sx;      /**< x scale, 4.12 */
    s16 sy;      /**< y scale, 4.12 */
    s16 rot;     /**< rotation angle, 4096 to a turn */
    s16 centred; /**< nonzero: (x, y) is the sprite's centre */
} Sprite2D;

/** @brief The three flat lights. */
typedef struct {
    GsF_LIGHT l[3]; /**< lights 0 to 2 */
} FlatLights;

/* MATCHING: a struct lvalue keeps the base in one register; common.h
 * declares the table as words. */
#define sLights ((*(FlatLights *)D_800DD070).l)

/** @brief A primitive handler: one entry of the handler table. */
typedef PACKET *(*PrimFunc)();

/* The handler table, eight rows of eight handlers. */
extern PrimFunc D_800E48E8[8][8];

PACKET *func_8001B4BC(TmdF3 *prim, SVECTOR *vtx, POLY_F3 *pkt, s32 n, s32 shift, GsOT *ot);
PACKET *func_8001B9A4(TmdF4 *prim, SVECTOR *vtx, POLY_F4 *pkt, s32 n, s32 shift, GsOT *ot);
PACKET *func_8001C13C(TmdG3 *prim, SVECTOR *vtx, PACKET *packet, s32 n, s32 shift, GsOT *ot);
PACKET *func_8001C878();
PACKET *func_8001D39C(TmdFT3 *prim, SVECTOR *vtx, PACKET *packet, s32 n, s32 shift, GsOT *ot);
PACKET *func_8001DAD4();
PACKET *func_8001E558();
PACKET *func_8001EE30();
PACKET *func_8001FBBC(TmdGT3 *prim, SVECTOR *vtx, SVECTOR *nrm, PACKET *packet, s32 n, s32 shift,
                      GsOT *ot);
PACKET *func_8001FE5C(TmdGT4 *prim, SVECTOR *vtx, SVECTOR *nrm, volatile POLY_FT4 *pkt, s32 n,
                      s32 shift, GsOT *ot);
PACKET *func_800201BC(TmdGT3 *prim, SVECTOR *vtx, SVECTOR *nrm, PACKET *packet, s32 n, s32 shift,
                      GsOT *ot);
PACKET *func_80020520(TmdGT4 *prim, SVECTOR *vtx, SVECTOR *nrm, PACKET *packet, s32 n, s32 shift,
                      GsOT *ot);
PACKET *func_8002097C(TmdGT4 *prim, SVECTOR *vtx, SVECTOR *nrm, PACKET *packet, s32 n, s32 shift,
                      GsOT *ot);
PACKET *func_80020DD8(TmdF3 *prim, SVECTOR *vtx, POLY_F3 *pkt, s32 n, s32 shift, GsOT *ot);
PACKET *func_80020F24(TmdF4 *prim, SVECTOR *vtx, POLY_F4 *pkt, s32 n, s32 shift, GsOT *ot);
PACKET *func_800210B4(TmdG3 *prim, SVECTOR *vtx, PACKET *packet, s32 n, s32 shift, GsOT *ot);
PACKET *func_80021240(TmdG4 *prim, SVECTOR *vtx, PACKET *packet, s32 n, s32 shift, GsOT *ot);
PACKET *func_80021434(TmdFT3 *prim, SVECTOR *vtx, PACKET *packet, s32 n, s32 shift, GsOT *ot);

/* Handlers code_11dc4 defines; their parameter types are that unit's. */
/* MATCHING: a per-unit view; here they only fill the PrimFunc table. */
PACKET *func_800215C4();
PACKET *func_800217A8();
PACKET *func_80021958();
PACKET *func_80021B88();
PACKET *func_80021D3C();
PACKET *func_80021F80();
PACKET *func_80022150();
PACKET *func_8002230C();

void func_800198BC(u16 id, Sprite2D *s, CVECTOR *color, u16 otz, GsOT *ot) {
    Sprite8 *e;
    POLY_FT4 *p;
    u32 *tag;
    /* MATCHING: the corners declared before the sprite size, so the spilled
     * x2 takes the lowest stack slot. */
    s16 x0, x1, x2, x3;
    s16 y0, y1, y2, y3;
    u8 w;
    u8 h;
    s32 sn;
    s32 cs;

    e = &D_800DD0A0[id];
    p = (POLY_FT4 *)D_800E48D0;
    *(u32 *)&p->r0 =
        color != NULL ? 0x2C000000 | (color->cd << 16) | (color->b << 8) | color->g : 0x2D000000;
    if (color != NULL && (s8)color->r != -1) {
        *(u32 *)&p->r0 |= 0x02000000;
    }
    w = e->unk6;
    h = e->unk7;
    /* MATCHING: both corners assigned in each arm; CSE then cannot see
     * that x0 == x2 and y0 == y1, and cross-jumping merges the tails. */
    if (s->centred) {
        x0 = x2 = ((s->x - (w >> 1) - s->cx) * s->sx) >> 12;
        y0 = y1 = ((s->y - (h >> 1) - s->cy) * s->sy) >> 12;
    } else {
        x0 = x2 = ((s->x - s->cx) * s->sx) >> 12;
        y0 = y1 = ((s->y - s->cy) * s->sy) >> 12;
    }
    x1 = x3 = x0 + ((w * s->sx) >> 12);
    y2 = y3 = y0 + ((h * s->sy) >> 12);
    sn = rsin(s->rot);
    cs = rcos(s->rot);
    p->x0 = s->cx + ((x0 * cs - y0 * sn) >> 12);
    p->y0 = s->cy + ((x0 * sn + y0 * cs) >> 12);
    p->x1 = s->cx + ((x1 * cs - y1 * sn) >> 12);
    p->y1 = s->cy + ((x1 * sn + y1 * cs) >> 12);
    p->x2 = s->cx + ((x2 * cs - y2 * sn) >> 12);
    p->y2 = s->cy + ((x2 * sn + y2 * cs) >> 12);
    p->x3 = s->cx + ((x3 * cs - y3 * sn) >> 12);
    p->y3 = s->cy + ((x3 * sn + y3 * cs) >> 12);
    *(u32 *)&p->u0 = (e->unk5 << 8) | e->unk4 | (e->unk2 << 16);
    if (color != NULL && (s8)color->r != -1) {
        *(u32 *)&p->u1 = ((*(u32 *)&p->u0 + w - 1) & 0xFFFF) | ((e->unk0 | ((s8)color->r << 5)) << 16);
    } else {
        *(u32 *)&p->u1 = ((*(u32 *)&p->u0 + w - 1) & 0xFFFF) | (e->unk0 << 16);
    }
    *(u32 *)&p->u2 = ((e->unk5 + h - 1) << 8) | e->unk4;
    *(u32 *)&p->u3 = *(u32 *)&p->u2 + w - 1;
    tag = (u32 *)ot->org + otz;
    *(u32 *)p = (*tag & 0xFFFFFF) | 0x09000000;
    *tag = (u32)p & 0xFFFFFF;
    D_800E48D0 = (u8 *)(p + 1);
}

void func_80019CD8(u16 id, SVECTOR *pos, CVECTOR *color, s32 mode, u16 otz, GsOT *ot) {
    Sprite8 *e;
    /* MATCHING: volatile, so each chained vertex store reads its
     * first target back. */
    volatile POLY_FT4 *p;
    u32 *tag;
    u8 u0, u1, u2, u3;
    u8 v0, v1, v2, v3;

    e = &D_800DD0A0[id];
    p = (POLY_FT4 *)D_800E48D0;
    *(u32 *)&p->r0 =
        color != NULL ? 0x2C000000 | (color->cd << 16) | (color->b << 8) | color->g : 0x2D000000;
    if (color != NULL && (s8)color->r != -1) {
        *(u32 *)&p->r0 |= 0x02000000;
    }
    p->x0 = p->x2 = pos->vx;
    p->y0 = p->y1 = pos->vy;
    p->x1 = p->x3 = pos->vx + e->unk6 - 1;
    p->y2 = p->y3 = pos->vy + e->unk7 - 1;
    u0 = u2 = e->unk4;
    v0 = v1 = e->unk5;
    u1 = u3 = e->unk4 + e->unk6 - 1;
    v2 = v3 = e->unk5 + e->unk7 - 1;
    if (mode & 1) {
        u0 = u1;
        u3 = u2;
        u1 = u3;
        u2 = u0;
    }
    if (mode & 2) {
        v0 = v2;
        v3 = v1;
        v2 = v3;
        v1 = v0;
    }
    *(u32 *)&p->u0 = (v0 << 8) | u0 | (e->unk2 << 16);
    if (color != NULL && (s8)color->r != -1) {
        *(u32 *)&p->u1 = (v1 << 8) | u1 | ((e->unk0 | ((s8)color->r << 5)) << 16);
    } else {
        *(u32 *)&p->u1 = (v1 << 8) | u1 | (e->unk0 << 16);
    }
    *(u32 *)&p->u2 = (v2 << 8) | u2;
    *(u32 *)&p->u3 = (v3 << 8) | u3;
    tag = (u32 *)ot->org + otz;
    *(u32 *)p = (*tag & 0xFFFFFF) | 0x09000000;
    *tag = (u32)p & 0xFFFFFF;
    D_800E48D0 = (u8 *)(p + 1);
}

void func_80019F24(u16 id, Sprite2D *s, CVECTOR *color, u8 mode, u16 otz, GsOT *ot) {
    Sprite8 *e;
    POLY_FT4 *p;
    u32 *tag;
    s16 x0, x1, x2, x3;
    s16 y0, y1, y2, y3;
    u8 w;
    u8 h;
    s32 sn;
    s32 cs;

    e = &D_800DD0A0[id];
    p = (POLY_FT4 *)D_800E48D0;
    *(u32 *)&p->r0 =
        color != NULL ? 0x2C000000 | (color->cd << 16) | (color->b << 8) | color->g : 0x2D000000;
    if (color != NULL && (s8)color->r != -1) {
        *(u32 *)&p->r0 |= 0x02000000;
    }
    w = e->unk6;
    h = e->unk7;
    if (s->centred) {
        x0 = x2 = ((s->x - (w >> 1) - s->cx) * s->sx) >> 12;
        y0 = y1 = ((s->y - (h >> 1) - s->cy) * s->sy) >> 12;
    } else {
        x0 = x2 = ((s->x - s->cx) * s->sx) >> 12;
        y0 = y1 = ((s->y - s->cy) * s->sy) >> 12;
    }
    x1 = x3 = x0 + ((w * s->sx) >> 12);
    y2 = y3 = y0 + ((h * s->sy) >> 12);
    sn = rsin(s->rot);
    cs = rcos(s->rot);
    p->x0 = s->cx + ((x0 * cs - y0 * sn) >> 12);
    p->y0 = s->cy + ((x0 * sn + y0 * cs) >> 12);
    p->x1 = s->cx + ((x1 * cs - y1 * sn) >> 12);
    p->y1 = s->cy + ((x1 * sn + y1 * cs) >> 12);
    p->x2 = s->cx + ((x2 * cs - y2 * sn) >> 12);
    p->y2 = s->cy + ((x2 * sn + y2 * cs) >> 12);
    p->x3 = s->cx + ((x3 * cs - y3 * sn) >> 12);
    p->y3 = s->cy + ((x3 * sn + y3 * cs) >> 12);
    /* MATCHING: the s16 corners reused for the UVs, which packs them with
     * sll/sra 16. */
    x0 = x2 = e->unk4;
    y0 = y1 = e->unk5;
    x1 = x3 = e->unk4 + w - 1;
    y2 = y3 = e->unk5 + h - 1;
    if (mode & 1) {
        x0 = x1;
        x3 = x2;
        x1 = x3;
        x2 = x0;
    }
    if (mode & 2) {
        y0 = y2;
        y3 = y1;
        y2 = y3;
        y1 = y0;
    }
    *(u32 *)&p->u0 = (y0 << 8) | x0 | (e->unk2 << 16);
    if (color != NULL && (s8)color->r != -1) {
        *(u32 *)&p->u1 = (y1 << 8) | x1 | ((e->unk0 | ((s8)color->r << 5)) << 16);
    } else {
        *(u32 *)&p->u1 = (y1 << 8) | x1 | (e->unk0 << 16);
    }
    *(u32 *)&p->u2 = (y2 << 8) | x2;
    *(u32 *)&p->u3 = (y3 << 8) | x3;
    tag = (u32 *)ot->org + otz;
    *(u32 *)p = (*tag & 0xFFFFFF) | 0x09000000;
    *tag = (u32)p & 0xFFFFFF;
    D_800E48D0 = (u8 *)(p + 1);
}

void func_8001A3D4(u16 id, SVECTOR *size, CVECTOR *color, s32 shift, GsOT *ot) {
    SVECTOR sv[4];
    s32 v;
    s32 z;
    Sprite8 *e;
    POLY_FT4 *p;
    u32 *tag;

    e = &D_800DD0A0[id];
    p = (POLY_FT4 *)D_800E48D0;
    gte_SetRotMatrix(&D_800E4858);
    sv[0].vx = sv[2].vx = -(size->vx >> 1);
    sv[0].vy = sv[1].vy = -(size->vy >> 1);
    sv[1].vx = sv[3].vx = sv[0].vx + size->vx - 1;
    sv[2].vy = sv[3].vy = sv[0].vy + size->vy - 1;
    sv[0].vz = sv[1].vz = sv[2].vz = sv[3].vz = 0;
    gte_ldv3(&sv[0], &sv[1], &sv[2]);
    gte_rtpt();
    gte_stflg(&v);
    /* MATCHING: both flag tests mask the sign bit; CSE keeps the mask in a
     * register, so only this first one folds into a sign branch. */
    if (v & 0x80000000) {
        return;
    }
    gte_stsxy3_ft4(p);
    gte_ldv0(&sv[3]);
    gte_rtps();
    gte_stflg(&v);
    if (v & 0x80000000) {
        return;
    }
    gte_stsxy2(&p->x3);
    gte_avsz4();
    gte_stotz(&v);
    *(u32 *)&p->r0 =
        color != NULL ? 0x2C000000 | (color->cd << 16) | (color->b << 8) | color->g : 0x2D000000;
    if (color != NULL && (s8)color->r != -1) {
        *(u32 *)&p->r0 |= 0x02000000;
    }
    *(u32 *)&p->u0 = (e->unk5 << 8) | e->unk4 | (e->unk2 << 16);
    if (color != NULL && (s8)color->r != -1) {
        *(u32 *)&p->u1 =
            ((*(u32 *)&p->u0 + e->unk6 - 1) & 0xFFFF) | ((e->unk0 | ((s8)color->r << 5)) << 16);
    } else {
        *(u32 *)&p->u1 = ((*(u32 *)&p->u0 + e->unk6 - 1) & 0xFFFF) | (e->unk0 << 16);
    }
    *(u32 *)&p->u2 = ((e->unk5 + e->unk7 - 1) << 8) | e->unk4;
    /* MATCHING: the index read before the last UV store, as retail
     * schedules it. */
    z = v >> shift;
    *(u32 *)&p->u3 = *(u32 *)&p->u2 + e->unk6 - 1;
    tag = (u32 *)ot->org + z;
    *(u32 *)p = (*tag & 0xFFFFFF) | 0x09000000;
    *tag = (u32)p & 0xFFFFFF;
    D_800E48D0 = (u8 *)(p + 1);
}

void func_8001A69C(u16 id, SVECTOR *size, CVECTOR *color, s32 shift, GsOT *ot) {
    SVECTOR sv[4];
    s32 v;
    s32 z;
    Sprite8 *e;
    POLY_FT4 *p;
    u32 *tag;

    e = &D_800DD0A0[id];
    p = (POLY_FT4 *)D_800E48D0;
    gte_SetRotMatrix(&D_800E4858);
    sv[0].vx = sv[2].vx = -(size->vx >> 1);
    sv[0].vy = sv[1].vy = -size->vy + 1;
    sv[1].vx = sv[3].vx = sv[0].vx + size->vx - 1;
    sv[2].vy = sv[3].vy = 0;
    sv[0].vz = sv[1].vz = sv[2].vz = sv[3].vz = 0;
    gte_ldv3(&sv[0], &sv[1], &sv[2]);
    gte_rtpt();
    gte_stflg(&v);
    /* MATCHING: both flag tests mask the sign bit; CSE keeps the mask in a
     * register, so only this first one folds into a sign branch. */
    if (v & 0x80000000) {
        return;
    }
    gte_stsxy3_ft4(p);
    gte_ldv0(&sv[3]);
    gte_rtps();
    gte_stflg(&v);
    if (v & 0x80000000) {
        return;
    }
    gte_stsxy2(&p->x3);
    gte_avsz4();
    gte_stotz(&v);
    *(u32 *)&p->r0 =
        color != NULL ? 0x2C000000 | (color->cd << 16) | (color->b << 8) | color->g : 0x2D000000;
    if (color != NULL && (s8)color->r != -1) {
        *(u32 *)&p->r0 |= 0x02000000;
    }
    *(u32 *)&p->u0 = (e->unk5 << 8) | e->unk4 | (e->unk2 << 16);
    if (color != NULL && (s8)color->r != -1) {
        *(u32 *)&p->u1 =
            ((*(u32 *)&p->u0 + e->unk6 - 1) & 0xFFFF) | ((e->unk0 | ((s8)color->r << 5)) << 16);
    } else {
        *(u32 *)&p->u1 = ((*(u32 *)&p->u0 + e->unk6 - 1) & 0xFFFF) | (e->unk0 << 16);
    }
    *(u32 *)&p->u2 = ((e->unk5 + e->unk7 - 1) << 8) | e->unk4;
    /* MATCHING: the index read before the last UV store, as retail
     * schedules it. */
    z = v >> shift;
    *(u32 *)&p->u3 = *(u32 *)&p->u2 + e->unk6 - 1;
    tag = (u32 *)ot->org + z;
    *(u32 *)p = (*tag & 0xFFFFFF) | 0x09000000;
    *tag = (u32)p & 0xFFFFFF;
    D_800E48D0 = (u8 *)(p + 1);
}

void func_8001A950(u16 id, SVECTOR *size, CVECTOR *color, s16 angle, s32 shift, GsOT *ot) {
    SVECTOR sv[4];
    MATRIX m;
    s32 v;
    s32 z;
    Sprite8 *e;
    POLY_FT4 *p;
    u32 *tag;

    /* MATCHING: the entry before the matrix copy, so the matrix address is
     * built late and gets $s0. */
    e = &D_800DD0A0[id];
    m = D_800E4858;
    p = (POLY_FT4 *)D_800E48D0;
    RotMatrixZ(angle, &m);
    gte_SetRotMatrix(&m);
    sv[0].vx = sv[2].vx = -(size->vx >> 1);
    sv[0].vy = sv[1].vy = -(size->vy >> 1);
    sv[1].vx = sv[3].vx = sv[0].vx + size->vx - 1;
    sv[2].vy = sv[3].vy = sv[0].vy + size->vy - 1;
    sv[0].vz = sv[1].vz = sv[2].vz = sv[3].vz = 0;
    gte_ldv3(&sv[0], &sv[1], &sv[2]);
    gte_rtpt();
    gte_stflg(&v);
    /* MATCHING: both flag tests mask the sign bit; CSE keeps the mask in a
     * register, so only this first one folds into a sign branch. */
    if (v & 0x80000000) {
        return;
    }
    gte_stsxy3_ft4(p);
    gte_ldv0(&sv[3]);
    gte_rtps();
    gte_stflg(&v);
    if (v & 0x80000000) {
        return;
    }
    gte_stsxy2(&p->x3);
    gte_avsz4();
    gte_stotz(&v);
    *(u32 *)&p->r0 =
        color != NULL ? 0x2C000000 | (color->cd << 16) | (color->b << 8) | color->g : 0x2D000000;
    if (color != NULL && (s8)color->r != -1) {
        *(u32 *)&p->r0 |= 0x02000000;
    }
    *(u32 *)&p->u0 = (e->unk5 << 8) | e->unk4 | (e->unk2 << 16);
    if (color != NULL && (s8)color->r != -1) {
        *(u32 *)&p->u1 =
            ((*(u32 *)&p->u0 + e->unk6 - 1) & 0xFFFF) | ((e->unk0 | ((s8)color->r << 5)) << 16);
    } else {
        *(u32 *)&p->u1 = ((*(u32 *)&p->u0 + e->unk6 - 1) & 0xFFFF) | (e->unk0 << 16);
    }
    *(u32 *)&p->u2 = ((e->unk5 + e->unk7 - 1) << 8) | e->unk4;
    /* MATCHING: the index read before the last UV store, as retail
     * schedules it. */
    z = v >> shift;
    *(u32 *)&p->u3 = *(u32 *)&p->u2 + e->unk6 - 1;
    tag = (u32 *)ot->org + z;
    *(u32 *)p = (*tag & 0xFFFFFF) | 0x09000000;
    *tag = (u32)p & 0xFFFFFF;
    D_800E48D0 = (u8 *)(p + 1);
}

void func_8001ACB4(u16 id, SVECTOR *size, CVECTOR *color, s16 angle, s32 shift, GsOT *ot) {
    SVECTOR sv[4];
    MATRIX m;
    s32 v;
    s32 z;
    Sprite8 *e;
    POLY_FT4 *p;
    u32 *tag;

    /* MATCHING: the entry before the matrix copy, so the matrix address is
     * built late and gets $s0. */
    e = &D_800DD0A0[id];
    m = D_800E4858;
    p = (POLY_FT4 *)D_800E48D0;
    RotMatrixZ(angle, &m);
    gte_SetRotMatrix(&m);
    sv[0].vx = sv[2].vx = -(size->vx >> 1);
    sv[0].vy = sv[1].vy = -size->vy + 1;
    sv[1].vx = sv[3].vx = sv[0].vx + size->vx - 1;
    sv[2].vy = sv[3].vy = 0;
    sv[0].vz = sv[1].vz = sv[2].vz = sv[3].vz = 0;
    gte_ldv3(&sv[0], &sv[1], &sv[2]);
    gte_rtpt();
    gte_stflg(&v);
    /* MATCHING: both flag tests mask the sign bit; CSE keeps the mask in a
     * register, so only this first one folds into a sign branch. */
    if (v & 0x80000000) {
        return;
    }
    gte_stsxy3_ft4(p);
    gte_ldv0(&sv[3]);
    gte_rtps();
    gte_stflg(&v);
    if (v & 0x80000000) {
        return;
    }
    gte_stsxy2(&p->x3);
    gte_avsz4();
    gte_stotz(&v);
    *(u32 *)&p->r0 =
        color != NULL ? 0x2C000000 | (color->cd << 16) | (color->b << 8) | color->g : 0x2D000000;
    if (color != NULL && (s8)color->r != -1) {
        *(u32 *)&p->r0 |= 0x02000000;
    }
    *(u32 *)&p->u0 = (e->unk5 << 8) | e->unk4 | (e->unk2 << 16);
    if (color != NULL && (s8)color->r != -1) {
        *(u32 *)&p->u1 =
            ((*(u32 *)&p->u0 + e->unk6 - 1) & 0xFFFF) | ((e->unk0 | ((s8)color->r << 5)) << 16);
    } else {
        *(u32 *)&p->u1 = ((*(u32 *)&p->u0 + e->unk6 - 1) & 0xFFFF) | (e->unk0 << 16);
    }
    *(u32 *)&p->u2 = ((e->unk5 + e->unk7 - 1) << 8) | e->unk4;
    /* MATCHING: the index read before the last UV store, as retail
     * schedules it. */
    z = v >> shift;
    *(u32 *)&p->u3 = *(u32 *)&p->u2 + e->unk6 - 1;
    tag = (u32 *)ot->org + z;
    *(u32 *)p = (*tag & 0xFFFFFF) | 0x09000000;
    *tag = (u32)p & 0xFFFFFF;
    D_800E48D0 = (u8 *)(p + 1);
}

void func_8001B004(u16 id, SVECTOR *size, CVECTOR *color, s32 shift, GsOT *ot) {
    SVECTOR sv[4];
    CVECTOR c;
    s32 v;
    s32 dp;
    s32 z;
    Sprite8 *e;
    POLY_FT4 *p;
    u32 *tag;

    e = &D_800DD0A0[id];
    p = (POLY_FT4 *)D_800E48D0;
    gte_SetRotMatrix(&D_800E4858);
    sv[0].vx = sv[2].vx = -(size->vx >> 1);
    sv[0].vy = sv[1].vy = -size->vy + 1;
    sv[1].vx = sv[3].vx = sv[0].vx + size->vx - 1;
    sv[2].vy = sv[3].vy = 0;
    sv[0].vz = sv[1].vz = sv[2].vz = sv[3].vz = 0;
    gte_ldv3(&sv[0], &sv[1], &sv[2]);
    gte_rtpt();
    gte_stflg(&v);
    if (v & 0x80000000) {
        return;
    }
    gte_stsxy3_ft4(p);
    gte_ldv0(&sv[3]);
    gte_rtps();
    gte_stflg(&v);
    if (v & 0x80000000) {
        return;
    }
    gte_stsxy2(&p->x3);
    gte_avsz4();
    gte_stotz(&v);
    *(u32 *)&c = color != NULL ? (color->cd << 16) | (color->b << 8) | color->g : 0x808080;
    gte_stdp(&dp);
    gte_ldrgb(&c);
    gte_lddp(dp);
    gte_dpcs();
    gte_strgb(&p->r0);
    /* MATCHING: a volatile store, which stays out of the branch's delay
     * slot. */
    ((volatile POLY_FT4 *)p)->code = 0x2C;
    if (color != NULL && (s8)color->r != -1) {
        *(u32 *)&p->r0 |= 0x02000000;
    }
    *(u32 *)&p->u0 = (e->unk5 << 8) | e->unk4 | (e->unk2 << 16);
    if (color != NULL && (s8)color->r != -1) {
        *(u32 *)&p->u1 =
            ((*(u32 *)&p->u0 + e->unk6 - 1) & 0xFFFF) | ((e->unk0 | ((s8)color->r << 5)) << 16);
    } else {
        *(u32 *)&p->u1 = ((*(u32 *)&p->u0 + e->unk6 - 1) & 0xFFFF) | (e->unk0 << 16);
    }
    *(u32 *)&p->u2 = ((e->unk5 + e->unk7 - 1) << 8) | e->unk4;
    z = v >> shift;
    *(u32 *)&p->u3 = *(u32 *)&p->u2 + e->unk6 - 1;
    tag = (u32 *)ot->org + z;
    *(u32 *)p = (*tag & 0xFFFFFF) | 0x09000000;
    *tag = (u32)p & 0xFFFFFF;
    D_800E48D0 = (u8 *)(p + 1);
}

void func_8001B2F4(u16 id, u8 mode, s32 w, s32 h, s32 page, s32 u, s32 v, u16 clutX, s32 clutY) {
    D_800DD0A0[id].unk0 = (u8)page | (mode << 7);
    D_800DD0A0[id].unk2 = (clutY << 6) | (clutX >> 4);
    D_800DD0A0[id].unk4 = u;
    D_800DD0A0[id].unk5 = v;
    D_800DD0A0[id].unk6 = w;
    D_800DD0A0[id].unk7 = h;
}

void func_8001B354(u16 id, SVECTOR *pos, CVECTOR *color, s32 mode, GsOT *ot) {
    Sprite8 *e;
    u32 *p;
    u32 *q;
    u32 *tag;
    u32 c;

    e = &D_800DD0A0[id];
    p = (u32 *)D_800E48D0;
    /* MATCHING: a ternary, and the semi-transparency term through a local,
     * keep cc1 from regrouping the constants. */
    p[1] = color != NULL ? 0x64000000 | (color->cd << 16) | (color->b << 8) | color->g : 0x65000000;
    if (color != NULL && (s8)color->r != -1) {
        p[1] |= 0x02000000;
    }
    p[2] = (pos->vy << 16) | (u16)pos->vx;
    p[3] = (e->unk5 << 8) | e->unk4 | (e->unk2 << 16);
    p[4] = (e->unk7 << 16) | e->unk6;
    tag = (u32 *)ot->org + (u16)mode;
    p[0] = (*tag & 0xFFFFFF) | 0x04000000;
    q = p + 5;
    if (color != NULL && (s8)color->r != -1) {
        c = (s8)color->r << 5 | 0xE1000600;
        q[1] = e->unk0 | c;
    } else {
        q[1] = e->unk0 | 0xE1000600;
    }
    q[0] = ((u32)p & 0xFFFFFF) | 0x01000000;
    *tag = (u32)q & 0xFFFFFF;
    D_800E48D0 = (u8 *)(q + 2);
}

PACKET *func_8001B4BC(TmdF3 *prim, SVECTOR *vtx, POLY_F3 *pkt, s32 n, s32 shift, GsOT *ot) {
    SVECTOR m[3];
    s32 sz[3];
    s32 flg;
    s32 v;
    s32 dp;
    s32 i;
    s32 j;
    u32 *tag;

    for (i = 0; i < n; i++, prim++) {
        gte_ldv3(&vtx[prim->v0], &vtx[prim->v1], &vtx[prim->v2]);
        gte_rtpt();
        gte_stflg(&flg);
        gte_nclip();
        gte_stopz(&v);
        if (v <= 0) {
            continue;
        }
        gte_stsxy3_f3(pkt);
        gte_stsz3c(sz);
        if (sz[0] > sz[1]) {
            v = sz[0];
        } else {
            v = sz[1];
        }
        /* MATCHING: sz[2] first, so it is loaded before v. */
        if (sz[2] > v) {
            v = sz[2];
        }
        v >>= 2;
        gte_stdp(&dp);
        if (flg >= 0) {
            gte_ldrgb(&prim->rgb);
            gte_lddp(dp);
            gte_dpcs();
            gte_strgb(&pkt->r0);
            tag = (u32 *)ot->org + (v >> shift) + 1;
            *(u32 *)pkt = (*tag & 0xFFFFFF) | 0x04000000;
            *tag = (u32)pkt & 0xFFFFFF;
            pkt++;
        }
        if (v < 250) {
            m[0].vx = (vtx[prim->v0].vx + vtx[prim->v1].vx) >> 1;
            m[0].vy = (vtx[prim->v0].vy + vtx[prim->v1].vy) >> 1;
            m[0].vz = (vtx[prim->v0].vz + vtx[prim->v1].vz) >> 1;
            m[1].vx = (vtx[prim->v0].vx + vtx[prim->v2].vx) >> 1;
            m[1].vy = (vtx[prim->v0].vy + vtx[prim->v2].vy) >> 1;
            m[1].vz = (vtx[prim->v0].vz + vtx[prim->v2].vz) >> 1;
            m[2].vx = (vtx[prim->v1].vx + vtx[prim->v2].vx) >> 1;
            m[2].vy = (vtx[prim->v1].vy + vtx[prim->v2].vy) >> 1;
            m[2].vz = (vtx[prim->v1].vz + vtx[prim->v2].vz) >> 1;
            for (j = 0; j < 4; j++) {
                switch (j) {
                    case 0:
                        gte_ldv3(&vtx[prim->v0], &m[0], &m[1]);
                        break;
                    case 1:
                        gte_ldv3(&m[0], &m[2], &m[1]);
                        break;
                    case 2:
                        gte_ldv3(&m[0], &vtx[prim->v1], &m[2]);
                        break;
                    case 3:
                        gte_ldv3(&m[1], &m[2], &vtx[prim->v2]);
                        break;
                }
                gte_rtpt();
                gte_stflg(&v);
                if (v & 0x7F85E000) {
                    continue;
                }
                gte_stsxy3_f3(pkt);
                gte_avsz3();
                gte_stotz(&v);
                *(u32 *)&pkt->r0 = *(u32 *)&prim->rgb;
                tag = (u32 *)ot->org + (v >> shift);
                *(u32 *)pkt = (*tag & 0xFFFFFF) | 0x04000000;
                *tag = (u32)pkt & 0xFFFFFF;
                pkt++;
            }
        }
    }
    return (PACKET *)pkt;
}

PACKET *func_8001B9A4(TmdF4 *prim, SVECTOR *vtx, POLY_F4 *pkt, s32 n, s32 shift, GsOT *ot) {
    SVECTOR m[5];
    s32 sz[4];
    s32 flg;
    s32 v;
    s32 dp;
    s32 i;
    s32 j;
    u32 *tag;

    for (i = 0; i < n; i++, prim++) {
        gte_ldv3(&vtx[prim->v0], &vtx[prim->v1], &vtx[prim->v2]);
        gte_rtpt();
        gte_stflg(&flg);
        gte_nclip();
        gte_stopz(&v);
        if (v <= 0) {
            continue;
        }
        gte_stsxy3_f4(pkt);
        gte_ldv0(&vtx[prim->v3]);
        gte_rtps();
        gte_stflg(&v);
        gte_stsxy2(&pkt->x3);
        gte_stsz4c(sz);
        if (sz[0] > sz[1]) {
            v = sz[0];
        } else {
            v = sz[1];
        }
        if (sz[2] > v) {
            v = sz[2];
        } else if (sz[3] > v) {
            v = sz[3];
        }
        v >>= 2;
        gte_stdp(&dp);
        if (flg >= 0) {
            gte_ldrgb(&prim->rgb);
            gte_lddp(dp);
            gte_dpcs();
            gte_strgb(&pkt->r0);
            tag = (u32 *)ot->org + (v >> shift) + 1;
            *(u32 *)pkt = (*tag & 0xFFFFFF) | 0x05000000;
            *tag = (u32)pkt & 0xFFFFFF;
            pkt++;
        }
        tag = (u32 *)ot->org + (v >> shift);
        if (v < 250) {
            m[0].vx = (vtx[prim->v0].vx + vtx[prim->v1].vx) >> 1;
            m[0].vy = (vtx[prim->v0].vy + vtx[prim->v1].vy) >> 1;
            m[0].vz = (vtx[prim->v0].vz + vtx[prim->v1].vz) >> 1;
            m[1].vx = (vtx[prim->v0].vx + vtx[prim->v2].vx) >> 1;
            m[1].vy = (vtx[prim->v0].vy + vtx[prim->v2].vy) >> 1;
            m[1].vz = (vtx[prim->v0].vz + vtx[prim->v2].vz) >> 1;
            m[2].vx = (vtx[prim->v1].vx + vtx[prim->v3].vx) >> 1;
            m[2].vy = (vtx[prim->v1].vy + vtx[prim->v3].vy) >> 1;
            m[2].vz = (vtx[prim->v1].vz + vtx[prim->v3].vz) >> 1;
            m[3].vx = (vtx[prim->v2].vx + vtx[prim->v3].vx) >> 1;
            m[3].vy = (vtx[prim->v2].vy + vtx[prim->v3].vy) >> 1;
            m[3].vz = (vtx[prim->v2].vz + vtx[prim->v3].vz) >> 1;
            m[4].vx = (vtx[prim->v0].vx + vtx[prim->v1].vx + vtx[prim->v2].vx + vtx[prim->v3].vx) >> 2;
            m[4].vy = (vtx[prim->v0].vy + vtx[prim->v1].vy + vtx[prim->v2].vy + vtx[prim->v3].vy) >> 2;
            m[4].vz = (vtx[prim->v0].vz + vtx[prim->v1].vz + vtx[prim->v2].vz + vtx[prim->v3].vz) >> 2;
            for (j = 0; j < 4; j++) {
                switch (j) {
                    case 0:
                        gte_ldv3(&vtx[prim->v0], &m[0], &m[1]);
                        break;
                    case 1:
                        gte_ldv3(&m[0], &vtx[prim->v1], &m[4]);
                        break;
                    case 2:
                        gte_ldv3(&m[1], &m[4], &vtx[prim->v2]);
                        break;
                    case 3:
                        gte_ldv3(&m[4], &m[2], &m[3]);
                        break;
                }
                gte_rtpt();
                gte_stflg(&v);
                if (v & 0x7F85E000) {
                    continue;
                }
                gte_stsxy3_f4(pkt);
                switch (j) {
                    case 0:
                        gte_ldv0(&m[4]);
                        break;
                    case 1:
                        gte_ldv0(&m[2]);
                        break;
                    case 2:
                        gte_ldv0(&m[3]);
                        break;
                    case 3:
                        gte_ldv0(&vtx[prim->v3]);
                        break;
                }
                gte_rtps();
                gte_stflg(&v);
                if (v & 0x7F85E000) {
                    continue;
                }
                gte_stsxy2(&pkt->x3);
                *(u32 *)&pkt->r0 = *(u32 *)&prim->rgb;
                *(u32 *)pkt = (*tag & 0xFFFFFF) | 0x05000000;
                *tag = (u32)pkt & 0xFFFFFF;
                pkt++;
            }
        }
    }
    return (PACKET *)pkt;
}

PACKET *func_8001C13C(TmdG3 *prim, SVECTOR *vtx, PACKET *packet, s32 n, s32 shift, GsOT *ot) {
    POLY_G3 *pkt;
    SVECTOR m[3];
    s32 sz[3];
    s32 flg;
    s32 v;
    s32 dp;
    s32 i;
    s32 j;
    u32 *tag;
    u32 r01, g01, b01;
    u32 r02, g02, b02;
    u32 r12, g12, b12;

    /* MATCHING: a copy of the parameter, which puts the i = 0 store
     * before n's home-slot store in the prologue. */
    pkt = (POLY_G3 *)packet;
    for (i = 0; i < n; i++, prim++) {
        gte_ldv3(&vtx[prim->v0], &vtx[prim->v1], &vtx[prim->v2]);
        gte_rtpt();
        gte_stflg(&flg);
        gte_nclip();
        gte_stopz(&v);
        if (v <= 0) {
            continue;
        }
        gte_stsxy3_g3(pkt);
        gte_stsz3c(sz);
        if (sz[0] > sz[1]) {
            v = sz[0];
        } else {
            v = sz[1];
        }
        if (sz[2] > v) {
            v = sz[2];
        }
        v >>= 2;
        gte_stdp(&dp);
        if (flg >= 0) {
            gte_ldrgb3(&prim->c0, &prim->c1, &prim->c2);
            gte_lddp(dp);
            gte_dpct();
            gte_strgb3(&pkt->r0, &pkt->r1, &pkt->r2);
            pkt->code = prim->mode;
            tag = (u32 *)ot->org + (v >> shift) + 1;
            *(u32 *)pkt = (*tag & 0xFFFFFF) | 0x06000000;
            *tag = (u32)pkt & 0xFFFFFF;
            pkt++;
        }
        tag = (u32 *)ot->org + (v >> shift);
        if (v < 250) {
            m[0].vx = (vtx[prim->v0].vx + vtx[prim->v1].vx) >> 1;
            m[0].vy = (vtx[prim->v0].vy + vtx[prim->v1].vy) >> 1;
            m[0].vz = (vtx[prim->v0].vz + vtx[prim->v1].vz) >> 1;
            m[1].vx = (vtx[prim->v0].vx + vtx[prim->v2].vx) >> 1;
            m[1].vy = (vtx[prim->v0].vy + vtx[prim->v2].vy) >> 1;
            m[1].vz = (vtx[prim->v0].vz + vtx[prim->v2].vz) >> 1;
            m[2].vx = (vtx[prim->v1].vx + vtx[prim->v2].vx) >> 1;
            m[2].vy = (vtx[prim->v1].vy + vtx[prim->v2].vy) >> 1;
            m[2].vz = (vtx[prim->v1].vz + vtx[prim->v2].vz) >> 1;
            /* MATCHING: u32 halves give srl. */
            r01 = (u32)(prim->c0.r + prim->c1.r) >> 1;
            g01 = (u32)(prim->c0.g + prim->c1.g) >> 1;
            b01 = (u32)(prim->c0.b + prim->c1.b) >> 1;
            r02 = (u32)(prim->c0.r + prim->c2.r) >> 1;
            g02 = (u32)(prim->c0.g + prim->c2.g) >> 1;
            b02 = (u32)(prim->c0.b + prim->c2.b) >> 1;
            r12 = (u32)(prim->c1.r + prim->c2.r) >> 1;
            g12 = (u32)(prim->c1.g + prim->c2.g) >> 1;
            b12 = (u32)(prim->c1.b + prim->c2.b) >> 1;
            for (j = 0; j < 4; j++) {
                switch (j) {
                    case 0:
                        gte_ldv3(&vtx[prim->v0], &m[0], &m[1]);
                        break;
                    case 1:
                        gte_ldv3(&m[0], &m[2], &m[1]);
                        break;
                    case 2:
                        gte_ldv3(&m[0], &vtx[prim->v1], &m[2]);
                        break;
                    case 3:
                        gte_ldv3(&m[1], &m[2], &vtx[prim->v2]);
                        break;
                }
                gte_rtpt();
                gte_stflg(&v);
                if (v & 0x7F85E000) {
                    continue;
                }
                gte_stsxy3_g3(pkt);
                switch (j) {
                    case 0:
                        *(u32 *)&pkt->r0 =
                            (prim->mode << 24) | (prim->c0.b << 16) | (prim->c0.g << 8) | prim->c0.r;
                        *(u32 *)&pkt->r1 = (b01 << 16) | (g01 << 8) | r01;
                        *(u32 *)&pkt->r2 = (b02 << 16) | (g02 << 8) | r02;
                        break;
                    case 1:
                        *(u32 *)&pkt->r0 = (prim->mode << 24) | (b01 << 16) | (g01 << 8) | r01;
                        *(u32 *)&pkt->r1 = (b12 << 16) | (g12 << 8) | r12;
                        *(u32 *)&pkt->r2 = (b02 << 16) | (g02 << 8) | r02;
                        break;
                    case 2:
                        *(u32 *)&pkt->r0 = (prim->mode << 24) | (b01 << 16) | (g01 << 8) | r01;
                        *(u32 *)&pkt->r1 = (prim->c1.b << 16) | (prim->c1.g << 8) | prim->c1.r;
                        *(u32 *)&pkt->r2 = (b12 << 16) | (g12 << 8) | r12;
                        break;
                    case 3:
                        *(u32 *)&pkt->r0 = (prim->mode << 24) | (b02 << 16) | (g02 << 8) | r02;
                        *(u32 *)&pkt->r1 = (b12 << 16) | (g12 << 8) | r12;
                        *(u32 *)&pkt->r2 = (prim->c2.b << 16) | (prim->c2.g << 8) | prim->c2.r;
                        break;
                }
                *(u32 *)pkt = (*tag & 0xFFFFFF) | 0x06000000;
                *tag = (u32)pkt & 0xFFFFFF;
                pkt++;
            }
        }
    }
    return (PACKET *)pkt;
}

INCLUDE_ASM("asm/nonmatchings/code_a0bc", func_8001C878);

PACKET *func_8001D39C(TmdFT3 *prim, SVECTOR *vtx, PACKET *packet, s32 n, s32 shift, GsOT *ot) {
    POLY_FT3 *pkt;
    SVECTOR m[3];
    s32 sz[3];
    s32 flg;
    s32 v;
    s32 dp;
    s32 i;
    s32 j;
    u32 *tag;
    /* MATCHING: u01 and u02 s16 (permuter find), which reorders the
     * allocation so u02 is the one reload spills. */
    s16 u01;
    u32 v01;
    s16 u02;
    u32 v02;
    u32 u12, v12;

    /* MATCHING: the packet pointer is a copy of the parameter, so the
     * loop starts its reduced pointer from it, not from $a2. */
    pkt = (POLY_FT3 *)packet;
    for (i = 0; i < n; i++, prim++) {
        gte_ldv3(&vtx[prim->v0], &vtx[prim->v1], &vtx[prim->v2]);
        gte_rtpt();
        gte_stflg(&flg);
        gte_nclip();
        gte_stopz(&v);
        if (v <= 0) {
            continue;
        }
        gte_stsxy3_ft3(pkt);
        gte_stsz3c(sz);
        if (sz[0] > sz[1]) {
            v = sz[0];
        } else {
            v = sz[1];
        }
        if (sz[2] > v) {
            v = sz[2];
        }
        v >>= 2;
        gte_stdp(&dp);
        if (flg >= 0) {
            gte_ldrgb(&prim->rgb);
            gte_lddp(dp);
            gte_dpcs();
            gte_strgb(&pkt->r0);
            /* MATCHING: volatile code stores stay on the packet base. */
            ((volatile POLY_FT3 *)pkt)->code = prim->mode & 0xFE;
            *(u32 *)&pkt->u0 = *(u32 *)&prim->uv0;
            *(u32 *)&pkt->u1 = *(u32 *)&prim->uv1;
            *(u16 *)&pkt->u2 = *(u16 *)&prim->uv2;
            tag = (u32 *)ot->org + (v >> shift) + 1;
            *(u32 *)pkt = (*tag & 0xFFFFFF) | 0x07000000;
            *tag = (u32)pkt & 0xFFFFFF;
            pkt++;
        }
        tag = (u32 *)ot->org + (v >> shift);
        if (v < 250) {
            m[0].vx = (vtx[prim->v0].vx + vtx[prim->v1].vx) >> 1;
            m[0].vy = (vtx[prim->v0].vy + vtx[prim->v1].vy) >> 1;
            m[0].vz = (vtx[prim->v0].vz + vtx[prim->v1].vz) >> 1;
            m[1].vx = (vtx[prim->v0].vx + vtx[prim->v2].vx) >> 1;
            m[1].vy = (vtx[prim->v0].vy + vtx[prim->v2].vy) >> 1;
            m[1].vz = (vtx[prim->v0].vz + vtx[prim->v2].vz) >> 1;
            m[2].vx = (vtx[prim->v1].vx + vtx[prim->v2].vx) >> 1;
            m[2].vy = (vtx[prim->v1].vy + vtx[prim->v2].vy) >> 1;
            m[2].vz = (vtx[prim->v1].vz + vtx[prim->v2].vz) >> 1;
            u01 = (u32)(prim->uv0.u + prim->uv1.u) >> 1;
            v01 = (u32)(prim->uv0.v + prim->uv1.v) >> 1;
            v02 = (u32)(prim->uv0.v + prim->uv2.v) >> 1;
            u02 = (u32)(prim->uv0.u + prim->uv2.u) >> 1;
            v12 = (u32)(prim->uv1.v + prim->uv2.v) >> 1;
            u12 = (u32)(prim->uv1.u + prim->uv2.u) >> 1;
            for (j = 0; j < 4; j++) {
                switch (j) {
                    case 0:
                        gte_ldv3(&vtx[prim->v0], &m[0], &m[1]);
                        break;
                    case 1:
                        gte_ldv3(&m[0], &m[2], &m[1]);
                        break;
                    case 2:
                        gte_ldv3(&m[0], &vtx[prim->v1], &m[2]);
                        break;
                    case 3:
                        gte_ldv3(&m[1], &m[2], &vtx[prim->v2]);
                        break;
                }
                gte_rtpt();
                gte_stflg(&v);
                if (v & 0x7F85E000) {
                    continue;
                }
                gte_stsxy3_ft3(pkt);
                switch (j) {
                    case 0:
                        *(u32 *)&pkt->u0 = (prim->uv0.hi << 16) | (prim->uv0.v << 8) | prim->uv0.u;
                        *(u32 *)&pkt->u1 = (prim->uv1.hi << 16) | (v01 << 8) | u01;
                        *(u32 *)&pkt->u2 = (v02 << 8) | u02;
                        break;
                    case 1:
                        *(u32 *)&pkt->u0 = (prim->uv0.hi << 16) | (v01 << 8) | u01;
                        *(u32 *)&pkt->u1 = (prim->uv1.hi << 16) | (v12 << 8) | u12;
                        *(u32 *)&pkt->u2 = (v02 << 8) | u02;
                        break;
                    case 2:
                        *(u32 *)&pkt->u0 = (prim->uv0.hi << 16) | (v01 << 8) | u01;
                        *(u32 *)&pkt->u1 = (prim->uv1.hi << 16) | (prim->uv1.v << 8) | prim->uv1.u;
                        *(u32 *)&pkt->u2 = (v12 << 8) | u12;
                        break;
                    case 3:
                        *(u32 *)&pkt->u0 = (prim->uv0.hi << 16) | (v02 << 8) | u02;
                        *(u32 *)&pkt->u1 = (prim->uv1.hi << 16) | (v12 << 8) | u12;
                        *(u32 *)&pkt->u2 = (prim->uv2.v << 8) | prim->uv2.u;
                        break;
                }
                gte_ldrgb(&prim->rgb);
                gte_lddp(dp);
                gte_dpcs();
                gte_strgb(&pkt->r0);
                ((volatile POLY_FT3 *)pkt)->code = prim->mode & 0xFE;
                *(u32 *)pkt = (*tag & 0xFFFFFF) | 0x07000000;
                *tag = (u32)pkt & 0xFFFFFF;
                pkt++;
            }
        }
    }
    return (PACKET *)pkt;
}

INCLUDE_ASM("asm/nonmatchings/code_a0bc", func_8001DAD4);

INCLUDE_ASM("asm/nonmatchings/code_a0bc", func_8001E558);

INCLUDE_ASM("asm/nonmatchings/code_a0bc", func_8001EE30);

PACKET *func_8001FBBC(TmdGT3 *prim, SVECTOR *vtx, SVECTOR *nrm, PACKET *packet, s32 n, s32 shift,
                      GsOT *ot) {
    VECTOR mac;
    s32 v;
    /* MATCHING: volatile, so every packet field is read and written in
     * source order on the biv. */
    volatile POLY_FT3 *pkt;
    s32 page;
    s32 y;
    s32 i;
    u32 *tag;

    pkt = (POLY_FT3 *)packet;
    page = (s16)(D_800E474C ^ 1) << 4;
    for (i = 0; i < n; i++, prim++) {
        gte_ldv3(&vtx[prim->v0], &vtx[prim->v1], &vtx[prim->v2]);
        gte_rtpt();
        gte_stflg(&v);
        if (v & 0x7F85E000) {
            continue;
        }
        gte_nclip();
        gte_stopz(&v);
        if (v <= 0) {
            continue;
        }
        gte_avsz3();
        gte_stotz(&v);
        gte_stsxy3_ft3(pkt);
        pkt->code = 0x25;
        pkt->tpage = page + 0x161;
        gte_ldv0(&nrm[prim->n0]);
        gte_rtv0();
        gte_stlvnl(&mac);
        pkt->u0 = (mac.vx >> 8) - ((pkt->x0 >> 3) - 0x60);
        /* MATCHING: the v coordinate built in one local, so 0x78 stays on
         * the screen y term. */
        y = (pkt->y0 >> 3) + 0x78;
        y += mac.vy >> 8;
        pkt->v0 = y - page;
        gte_ldv0(&nrm[prim->n1]);
        gte_rtv0();
        gte_stlvnl(&mac);
        pkt->u1 = (mac.vx >> 8) - ((pkt->x1 >> 3) - 0x60);
        y = (pkt->y1 >> 3) + 0x78;
        y += mac.vy >> 8;
        pkt->v1 = y - page;
        gte_ldv0(&nrm[prim->n2]);
        gte_rtv0();
        gte_stlvnl(&mac);
        pkt->u2 = (mac.vx >> 8) - ((pkt->x2 >> 3) - 0x60);
        y = (pkt->y2 >> 3) + 0x78;
        y += mac.vy >> 8;
        pkt->v2 = y - page;
        tag = (u32 *)ot->org + (v >> shift);
        *(u32 *)pkt = (*tag & 0xFFFFFF) | 0x07000000;
        *tag = (u32)pkt & 0xFFFFFF;
        pkt++;
    }
    return (PACKET *)pkt;
}

/* MATCHING: the packet parameter volatile and used directly, so loop builds
 * no reduced pointer beside it; the unused pair puts v at sp+0x18. */
PACKET *func_8001FE5C(TmdGT4 *prim, SVECTOR *vtx, SVECTOR *nrm, volatile POLY_FT4 *pkt, s32 n,
                      s32 shift, GsOT *ot) {
    VECTOR mac;
    s32 unused[2];
    s32 v;
    s32 page;
    s32 y;
    s32 i;
    u32 *tag;

    page = (s16)(D_800E474C ^ 1) << 4;
    for (i = 0; i < n; i++, prim++) {
        gte_ldv3(&vtx[prim->v0], &vtx[prim->v1], &vtx[prim->v2]);
        gte_rtpt();
        gte_stflg(&v);
        if (v & 0x7F85E000) {
            continue;
        }
        gte_nclip();
        gte_stopz(&v);
        if (v <= 0) {
            continue;
        }
        gte_stsxy3_ft4(pkt);
        gte_ldv0(&vtx[prim->v3]);
        gte_rtps();
        gte_stflg(&v);
        if (v & 0x7F85E000) {
            continue;
        }
        gte_stsxy2(&pkt->x3);
        gte_avsz4();
        gte_stotz(&v);
        pkt->code = 0x2D;
        pkt->tpage = page + 0x161;
        gte_ldv0(&nrm[prim->n0]);
        gte_rtv0();
        gte_stlvnl(&mac);
        pkt->u0 = (mac.vx >> 8) - ((pkt->x0 >> 3) - 0x60);
        y = (pkt->y0 >> 3) + 0x78;
        y += mac.vy >> 8;
        pkt->v0 = y - page;
        gte_ldv0(&nrm[prim->n1]);
        gte_rtv0();
        gte_stlvnl(&mac);
        pkt->u1 = (mac.vx >> 8) - ((pkt->x1 >> 3) - 0x60);
        y = (pkt->y1 >> 3) + 0x78;
        y += mac.vy >> 8;
        pkt->v1 = y - page;
        gte_ldv0(&nrm[prim->n2]);
        gte_rtv0();
        gte_stlvnl(&mac);
        pkt->u2 = (mac.vx >> 8) - ((pkt->x2 >> 3) - 0x60);
        y = (pkt->y2 >> 3) + 0x78;
        y += mac.vy >> 8;
        pkt->v2 = y - page;
        gte_ldv0(&nrm[prim->n3]);
        gte_rtv0();
        gte_stlvnl(&mac);
        pkt->u3 = (mac.vx >> 8) - ((pkt->x3 >> 3) - 0x60);
        y = (pkt->y3 >> 3) + 0x78;
        y += mac.vy >> 8;
        pkt->v3 = y - page;
        tag = (u32 *)ot->org + (v >> shift);
        *(u32 *)pkt = (*tag & 0xFFFFFF) | 0x09000000;
        *tag = (u32)pkt & 0xFFFFFF;
        pkt++;
    }
    return (PACKET *)pkt;
}

PACKET *func_800201BC(TmdGT3 *prim, SVECTOR *vtx, SVECTOR *nrm, PACKET *packet, s32 n, s32 shift,
                      GsOT *ot) {
    /* MATCHING: a volatile copy of the parameter, so the triangle's fields
     * stay on it and only the second packet goes through loop's pointer. */
    volatile POLY_FT3 *pkt;
    VECTOR mac;
    CVECTOR c;
    s32 v;
    POLY_GT3 *g;
    s32 i;
    u32 *tag;

    pkt = (POLY_FT3 *)packet;
    for (i = 0; i < n; i++, prim++) {
        gte_ldv3(&vtx[prim->v0], &vtx[prim->v1], &vtx[prim->v2]);
        gte_rtpt();
        gte_stflg(&v);
        if (v & 0x7F85E000) {
            continue;
        }
        gte_nclip();
        gte_stopz(&v);
        if (v <= 0) {
            continue;
        }
        gte_avsz3();
        gte_stotz(&v);
        gte_stsxy3_ft3(pkt);
        pkt->r0 = sLights[0].r;
        pkt->g0 = sLights[0].g;
        pkt->b0 = sLights[0].b;
        pkt->code = 0x26;
        pkt->tpage = 0x7B;
        pkt->clut = 0x722D;
        gte_ldv0(&nrm[prim->n0]);
        gte_rtv0();
        gte_stlvnl(&mac);
        pkt->u0 = (mac.vx >> 7) + 0x20;
        pkt->v0 = (mac.vy >> 7) - 0x20;
        gte_ldv0(&nrm[prim->n1]);
        gte_rtv0();
        gte_stlvnl(&mac);
        pkt->u1 = (mac.vx >> 7) + 0x20;
        pkt->v1 = (mac.vy >> 7) - 0x20;
        gte_ldv0(&nrm[prim->n2]);
        gte_rtv0();
        gte_stlvnl(&mac);
        pkt->u2 = (mac.vx >> 7) + 0x20;
        pkt->v2 = (mac.vy >> 7) - 0x20;
        g = (POLY_GT3 *)(pkt + 1);
        tag = (u32 *)ot->org + (v >> shift);
        *(u32 *)pkt = (*tag & 0xFFFFFF) | 0x07000000;
        *tag = (u32)pkt & 0xFFFFFF;
        c.r = c.g = c.b = 0x80;
        gte_ldrgb(&c);
        gte_ldv3(&nrm[prim->n0], &nrm[prim->n1], &nrm[prim->n2]);
        gte_ncct();
        *(u32 *)&g->u0 = prim->uv0;
        *(u32 *)&g->u1 = prim->uv1;
        *(u16 *)&g->u2 = prim->uv2;
        *(u32 *)&g->x0 = *(u32 *)&pkt->x0;
        *(u32 *)&g->x1 = *(u32 *)&pkt->x1;
        *(u32 *)&g->x2 = *(u32 *)&pkt->x2;
        tag = (u32 *)ot->org + (v >> shift);
        *(u32 *)g = (*tag & 0xFFFFFF) | 0x09000000;
        *tag = (u32)g & 0xFFFFFF;
        gte_strgb3_gt3(g);
        /* MATCHING: a volatile store, which loop cannot rebase onto its
         * reduced pointer. */
        ((volatile POLY_GT3 *)g)->code = 0x34;
        pkt = (POLY_FT3 *)(g + 1);
    }
    return (PACKET *)pkt;
}

PACKET *func_80020520(TmdGT4 *prim, SVECTOR *vtx, SVECTOR *nrm, PACKET *packet, s32 n, s32 shift,
                      GsOT *ot) {
    volatile POLY_FT4 *pkt;
    VECTOR mac;
    CVECTOR c;
    s32 v;
    POLY_GT4 *g;
    s32 i;
    u32 *tag;

    pkt = (POLY_FT4 *)packet;
    for (i = 0; i < n; i++, prim++) {
        gte_ldv3(&vtx[prim->v0], &vtx[prim->v1], &vtx[prim->v2]);
        gte_rtpt();
        gte_stflg(&v);
        if (v & 0x7F85E000) {
            continue;
        }
        gte_nclip();
        gte_stopz(&v);
        if (v <= 0) {
            continue;
        }
        gte_stsxy3_ft4(pkt);
        gte_ldv0(&vtx[prim->v3]);
        gte_rtps();
        gte_stflg(&v);
        if (v & 0x7F85E000) {
            continue;
        }
        gte_stsxy2(&pkt->x3);
        gte_avsz4();
        gte_stotz(&v);
        pkt->r0 = sLights[0].r;
        pkt->g0 = sLights[0].g;
        pkt->b0 = sLights[0].b;
        pkt->code = 0x2E;
        pkt->tpage = 0x7B;
        pkt->clut = 0x722D;
        gte_ldv0(&nrm[prim->n0]);
        gte_rtv0();
        gte_stlvnl(&mac);
        pkt->u0 = (mac.vx >> 7) + 0x20;
        pkt->v0 = (mac.vy >> 7) - 0x20;
        gte_ldv0(&nrm[prim->n1]);
        gte_rtv0();
        gte_stlvnl(&mac);
        pkt->u1 = (mac.vx >> 7) + 0x20;
        pkt->v1 = (mac.vy >> 7) - 0x20;
        gte_ldv0(&nrm[prim->n2]);
        gte_rtv0();
        gte_stlvnl(&mac);
        pkt->u2 = (mac.vx >> 7) + 0x20;
        pkt->v2 = (mac.vy >> 7) - 0x20;
        gte_ldv0(&nrm[prim->n3]);
        gte_rtv0();
        gte_stlvnl(&mac);
        pkt->u3 = (mac.vx >> 7) + 0x20;
        pkt->v3 = (mac.vy >> 7) - 0x20;
        g = (POLY_GT4 *)(pkt + 1);
        tag = (u32 *)ot->org + (v >> shift);
        *(u32 *)pkt = (*tag & 0xFFFFFF) | 0x09000000;
        *tag = (u32)pkt & 0xFFFFFF;
        c.r = c.g = c.b = 0x80;
        gte_ldrgb(&c);
        gte_ldv3(&nrm[prim->n0], &nrm[prim->n1], &nrm[prim->n2]);
        gte_ncct();
        *(u32 *)&g->u0 = prim->uv0;
        *(u32 *)&g->u1 = prim->uv1;
        *(u16 *)&g->u2 = prim->uv2;
        *(u16 *)&g->u3 = prim->uv3;
        *(u32 *)&g->x0 = *(u32 *)&pkt->x0;
        *(u32 *)&g->x1 = *(u32 *)&pkt->x1;
        *(u32 *)&g->x2 = *(u32 *)&pkt->x2;
        *(u32 *)&g->x3 = *(u32 *)&pkt->x3;
        gte_strgb3_gt3(g);
        gte_ldv0(&nrm[prim->n3]);
        gte_nccs();
        tag = (u32 *)ot->org + (v >> shift);
        *(u32 *)g = (*tag & 0xFFFFFF) | 0x0C000000;
        *tag = (u32)g & 0xFFFFFF;
        gte_strgb(&g->r3);
        /* MATCHING: a volatile store, which loop cannot rebase onto its
         * reduced pointer. */
        ((volatile POLY_GT4 *)g)->code = 0x3C;
        pkt = (POLY_FT4 *)(g + 1);
    }
    return (PACKET *)pkt;
}

PACKET *func_8002097C(TmdGT4 *prim, SVECTOR *vtx, SVECTOR *nrm, PACKET *packet, s32 n, s32 shift,
                      GsOT *ot) {
    LINE_G3 *pkt;
    CVECTOR c;
    s32 v;
    s32 z;
    s32 i;
    u32 *tag;

    /* MATCHING: the packet pointer is a copy of the parameter, so the
     * loop starts its reduced pointer from it, not from $a3. */
    pkt = (LINE_G3 *)packet;

    for (i = 0; i < n; i++, prim++) {
        gte_ldv3(&vtx[prim->v0], &vtx[prim->v1], &vtx[prim->v3]);
        gte_rtpt();
        gte_stflg(&v);
        if (v & 0x7F85E000) {
            continue;
        }
        gte_nclip();
        gte_stopz(&v);
        if (v <= 0) {
            continue;
        }
        gte_stsxy3(&pkt[0].x0, &pkt[0].x1, &pkt[0].x2);
        *(u32 *)&pkt[1].x0 = *(u32 *)&pkt[0].x0;
        *(u32 *)&pkt[1].x2 = *(u32 *)&pkt[0].x2;
        gte_ldv0(&vtx[prim->v2]);
        gte_rtps();
        gte_stflg(&v);
        if (v & 0x7F85E000) {
            continue;
        }
        gte_stsxy2(&pkt[1].x1);
        gte_avsz4();
        gte_stotz(&v);
        c.r = 0x80;
        c.g = 0x80;
        c.b = 0x80;
        gte_ldrgb(&c);
        gte_ldv3(&nrm[prim->n0], &nrm[prim->n1], &nrm[prim->n3]);
        gte_ncct();
        gte_strgb3(&pkt[0].r0, &pkt[0].r1, &pkt[0].r2);
        *(u32 *)&pkt[1].r0 = *(u32 *)&pkt[0].r0;
        *(u32 *)&pkt[1].r2 = *(u32 *)&pkt[0].r2;
        gte_ldv0(&nrm[prim->n3]);
        gte_nccs();
        gte_strgb(&pkt[1].r1);
        /* MATCHING: volatile stores keep their own base; the order of the
         * index, read early, and the terminators is retail's schedule. */
        ((volatile LINE_G3 *)pkt)[0].code = 0x58;
        ((volatile LINE_G3 *)pkt)[1].code = 0x58;
        z = v >> shift;
        ((volatile LINE_G3 *)pkt)[0].pad = 0x55555555;
        ((volatile LINE_G3 *)pkt)[1].pad = 0x55555555;
        tag = (u32 *)ot->org + z;
        *(u32 *)pkt = (*tag & 0xFFFFFF) | 0x07000000;
        *tag = (u32)pkt & 0xFFFFFF;
        pkt++;
        tag = (u32 *)ot->org + (v >> shift);
        *(u32 *)pkt = (*tag & 0xFFFFFF) | 0x07000000;
        *tag = (u32)pkt & 0xFFFFFF;
        pkt++;
    }
    return (PACKET *)pkt;
}

void func_80020C14(void) {
    D_800E48E8[0][6] = func_80020DD8;
    D_800E48E8[0][7] = func_8001B4BC;
    D_800E48E8[1][6] = func_800210B4;
    D_800E48E8[1][7] = func_8001C13C;
    D_800E48E8[2][6] = func_80021434;
    D_800E48E8[2][7] = func_8001D39C;
    D_800E48E8[3][0] = func_80021F80;
    D_800E48E8[3][6] = func_800217A8;
    D_800E48E8[3][7] = func_8001E558;
    D_800E48E8[4][6] = func_80020F24;
    D_800E48E8[4][7] = func_8001B9A4;
    D_800E48E8[5][6] = func_80021240;
    D_800E48E8[5][7] = func_8001C878;
    D_800E48E8[6][6] = func_800215C4;
    D_800E48E8[6][7] = func_8001DAD4;
    D_800E48E8[7][0] = func_8002097C;
    D_800E48E8[7][6] = func_80021958;
    D_800E48E8[7][7] = func_8001EE30;
}

s32 func_80020CF8(s32 mode) {
    switch (mode) {
        case 0:
        case 1:
            D_800E48E8[3][0] = func_800201BC;
            D_800E48E8[7][0] = func_80020520;
            break;
        case 2:
            D_800E48E8[3][0] = func_80021F80;
            D_800E48E8[7][0] = func_8002097C;
            break;
        case 3:
            D_800E48E8[3][0] = func_8001FBBC;
            D_800E48E8[7][0] = func_8001FE5C;
            break;
        case 4:
            D_800E48E8[3][0] = func_80022150;
            D_800E48E8[7][0] = func_8002230C;
            break;
        case 5:
            D_800E48E8[3][0] = func_80021B88;
            D_800E48E8[7][0] = func_80021D3C;
            break;
    }
}

PACKET *func_80020DD8(TmdF3 *prim, SVECTOR *vtx, POLY_F3 *pkt, s32 n, s32 shift, GsOT *ot) {
    s32 v;
    s32 dp;
    s32 i;
    u32 *tag;

    for (i = 0; i < n; i++, prim++) {
        gte_ldv3(&vtx[prim->v0], &vtx[prim->v1], &vtx[prim->v2]);
        gte_rtpt();
        gte_stflg(&v);
        if (v & 0x7F85E000) {
            continue;
        }
        gte_nclip();
        gte_stopz(&v);
        if (v <= 0) {
            continue;
        }
        gte_stsxy3_f3(pkt);
        gte_avsz3();
        gte_stotz(&v);
        gte_stdp(&dp);
        gte_ldrgb(&prim->rgb);
        gte_lddp(dp);
        gte_dpcs();
        gte_strgb(&pkt->r0);
        tag = (u32 *)ot->org + (v >> shift);
        *(u32 *)pkt = (*tag & 0xFFFFFF) | 0x04000000;
        *tag = (u32)pkt & 0xFFFFFF;
        pkt++;
    }
    return (PACKET *)pkt;
}

PACKET *func_80020F24(TmdF4 *prim, SVECTOR *vtx, POLY_F4 *pkt, s32 n, s32 shift, GsOT *ot) {
    s32 v;
    s32 dp;
    s32 i;
    u32 *tag;

    for (i = 0; i < n; i++, prim++) {
        gte_ldv3(&vtx[prim->v0], &vtx[prim->v1], &vtx[prim->v2]);
        gte_rtpt();
        gte_stflg(&v);
        if (v & 0x7F85E000) {
            continue;
        }
        gte_nclip();
        gte_stopz(&v);
        if (v <= 0) {
            continue;
        }
        gte_stsxy3_f4(pkt);
        gte_ldv0(&vtx[prim->v3]);
        gte_rtps();
        gte_stflg(&v);
        if (v & 0x7F85E000) {
            continue;
        }
        gte_stsxy2(&pkt->x3);
        gte_avsz4();
        gte_stotz(&v);
        gte_stdp(&dp);
        gte_ldrgb(&prim->rgb);
        gte_lddp(dp);
        gte_dpcs();
        gte_strgb(&pkt->r0);
        tag = (u32 *)ot->org + (v >> shift);
        *(u32 *)pkt = (*tag & 0xFFFFFF) | 0x05000000;
        *tag = (u32)pkt & 0xFFFFFF;
        pkt++;
    }
    return (PACKET *)pkt;
}

PACKET *func_800210B4(TmdG3 *prim, SVECTOR *vtx, PACKET *packet, s32 n, s32 shift, GsOT *ot) {
    POLY_G3 *pkt;
    s32 v;
    s32 dp;
    s32 i;
    u32 *tag;

    /* MATCHING: the packet pointer is a copy of the parameter, so the
     * loop starts its reduced pointer from it, not from $a2. */
    pkt = (POLY_G3 *)packet;

    for (i = 0; i < n; i++, prim++) {
        gte_ldv3(&vtx[prim->v0], &vtx[prim->v1], &vtx[prim->v2]);
        gte_rtpt();
        gte_stflg(&v);
        if (v & 0x7F85E000) {
            continue;
        }
        gte_nclip();
        gte_stopz(&v);
        if (v <= 0) {
            continue;
        }
        gte_stsxy3_g3(pkt);
        gte_avsz3();
        gte_stotz(&v);
        gte_stdp(&dp);
        gte_ldrgb3(&prim->c0, &prim->c1, &prim->c2);
        gte_lddp(dp);
        gte_dpct();
        gte_strgb3(&pkt->r0, &pkt->r1, &pkt->r2);
        /* MATCHING: a volatile store, which loop cannot rebase onto its
         * reduced pointer; it still pays for reducing &pkt->r2. */
        ((volatile POLY_G3 *)pkt)->code = prim->mode;
        tag = (u32 *)ot->org + (v >> shift);
        *(u32 *)pkt = (*tag & 0xFFFFFF) | 0x06000000;
        *tag = (u32)pkt & 0xFFFFFF;
        pkt++;
    }
    return (PACKET *)pkt;
}

PACKET *func_80021240(TmdG4 *prim, SVECTOR *vtx, PACKET *packet, s32 n, s32 shift, GsOT *ot) {
    POLY_G4 *pkt;
    s32 v;
    s32 dp;
    s32 i;
    u32 *tag;

    /* MATCHING: the packet pointer is a copy of the parameter, so the
     * loop starts its reduced pointer from it, not from $a2. */
    pkt = (POLY_G4 *)packet;

    for (i = 0; i < n; i++, prim++) {
        gte_ldv3(&vtx[prim->v0], &vtx[prim->v1], &vtx[prim->v2]);
        gte_rtpt();
        gte_stflg(&v);
        if (v & 0x7F85E000) {
            continue;
        }
        gte_nclip();
        gte_stopz(&v);
        if (v <= 0) {
            continue;
        }
        gte_stsxy3_g4(pkt);
        gte_ldv0(&vtx[prim->v3]);
        gte_rtps();
        gte_stflg(&v);
        if (v & 0x7F85E000) {
            continue;
        }
        gte_stsxy2(&pkt->x3);
        gte_avsz4();
        gte_stotz(&v);
        gte_stdp(&dp);
        gte_ldrgb3(&prim->c0, &prim->c1, &prim->c2);
        gte_lddp(dp);
        gte_dpct();
        gte_strgb3(&pkt->r0, &pkt->r1, &pkt->r2);
        gte_ldrgb(&prim->c3);
        gte_lddp(dp);
        gte_dpcs();
        gte_strgb(&pkt->r3);
        /* MATCHING: a volatile store, which loop cannot rebase onto its
         * reduced pointer; it still pays for reducing &pkt->r3. */
        ((volatile POLY_G4 *)pkt)->code = prim->mode;
        tag = (u32 *)ot->org + (v >> shift);
        *(u32 *)pkt = (*tag & 0xFFFFFF) | 0x08000000;
        *tag = (u32)pkt & 0xFFFFFF;
        pkt++;
    }
    return (PACKET *)pkt;
}

PACKET *func_80021434(TmdFT3 *prim, SVECTOR *vtx, PACKET *packet, s32 n, s32 shift, GsOT *ot) {
    POLY_FT3 *pkt;
    s32 v;
    s32 dp;
    s32 i;
    u32 *tag;

    /* MATCHING: the packet pointer is a copy of the parameter, so the
     * loop starts its reduced pointer from it, not from $a2. */
    pkt = (POLY_FT3 *)packet;

    for (i = 0; i < n; i++, prim++) {
        gte_ldv3(&vtx[prim->v0], &vtx[prim->v1], &vtx[prim->v2]);
        gte_rtpt();
        gte_stflg(&v);
        if (v & 0x7F85E000) {
            continue;
        }
        gte_nclip();
        gte_stopz(&v);
        if (v <= 0) {
            continue;
        }
        gte_stsxy3_ft3(pkt);
        gte_avsz3();
        gte_stotz(&v);
        gte_stdp(&dp);
        gte_ldrgb(&prim->rgb);
        gte_lddp(dp);
        gte_dpcs();
        gte_strgb(&pkt->r0);
        /* MATCHING: a volatile store, which loop cannot rebase onto its
         * reduced pointer; it still pays for reducing &pkt->u2. */
        ((volatile POLY_FT3 *)pkt)->code = prim->mode & 0xFE;
        *(u32 *)&pkt->u0 = *(u32 *)&prim->uv0;
        *(u32 *)&pkt->u1 = *(u32 *)&prim->uv1;
        *(u16 *)&pkt->u2 = *(u16 *)&prim->uv2;
        tag = (u32 *)ot->org + (v >> shift);
        *(u32 *)pkt = (*tag & 0xFFFFFF) | 0x07000000;
        *tag = (u32)pkt & 0xFFFFFF;
        pkt++;
    }
    return (PACKET *)pkt;
}
