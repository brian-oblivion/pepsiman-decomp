#include "common.h"
#include "draw.h"
#include "gte.h"
#include "prim.h"

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

/** @brief A flat-textured unlit TMD quad: a header, four UV words (the
 *  CLUT, the texture page, padding in their top halves), a colour and
 *  four vertex indices. */
typedef struct {
    u8 olen;     /**< the TMD header: output length */
    u8 ilen;     /**< input length */
    u8 flag;     /**< flags */
    u8 mode;     /**< the primitive code, copied into the packet */
    TmdUV uv0;   /**< u0, v0 and the CLUT */
    TmdUV uv1;   /**< u1, v1 and the texture page */
    TmdUV uv2;   /**< u2, v2 and padding */
    TmdUV uv3;   /**< u3, v3 and padding */
    CVECTOR rgb; /**< the colour, depth-cued per frame */
    u16 v0;      /**< first vertex index */
    u16 v1;      /**< second vertex index */
    u16 v2;      /**< third vertex index */
    u16 v3;      /**< fourth vertex index */
} TmdFT4;

/** @brief A gouraud-textured unlit TMD triangle: a header, three UV words
 *  (the CLUT, the texture page, padding in their top halves), three
 *  colours and three vertex indices. */
typedef struct {
    u8 olen;    /**< the TMD header: output length */
    u8 ilen;    /**< input length */
    u8 flag;    /**< flags */
    u8 mode;    /**< the primitive code, copied into the packet */
    TmdUV uv0;  /**< u0, v0 and the CLUT */
    TmdUV uv1;  /**< u1, v1 and the texture page */
    TmdUV uv2;  /**< u2, v2 and padding */
    CVECTOR c0; /**< first vertex colour */
    CVECTOR c1; /**< second vertex colour */
    CVECTOR c2; /**< third vertex colour */
    u16 v0;     /**< first vertex index */
    u16 v1;     /**< second vertex index */
    u16 v2;     /**< third vertex index */
    u16 pad;    /**< padding */
} TmdGT3U;

/** @brief A gouraud-textured unlit TMD quad: a header, four UV words (the
 *  CLUT, the texture page, padding in their top halves), four colours and
 *  four vertex indices. */
typedef struct {
    u8 olen;    /**< the TMD header: output length */
    u8 ilen;    /**< input length */
    u8 flag;    /**< flags */
    u8 mode;    /**< the primitive code, copied into the packet */
    TmdUV uv0;  /**< u0, v0 and the CLUT */
    TmdUV uv1;  /**< u1, v1 and the texture page */
    TmdUV uv2;  /**< u2, v2 and padding */
    TmdUV uv3;  /**< u3, v3 and padding */
    CVECTOR c0; /**< first vertex colour */
    CVECTOR c1; /**< second vertex colour */
    CVECTOR c2; /**< third vertex colour */
    CVECTOR c3; /**< fourth vertex colour */
    u16 v0;     /**< first vertex index */
    u16 v1;     /**< second vertex index */
    u16 v2;     /**< third vertex index */
    u16 v3;     /**< fourth vertex index */
} TmdGT4U;

/** @brief The SortTmdDiv* functions split a polygon into four when its
 *         deepest vertex's Z, over 4, is nearer than this. */
#define SUBDIVIDE_Z 250

/** @brief A sprite's place in VRAM, set by DefineSprite. */
typedef struct {
    u16 tpage; /**< texture page, with the semi-transparency bit at bit 7 */
    u16 clut;  /**< the CLUT's packed VRAM position */
    u8 u;      /**< left edge in the page */
    u8 v;      /**< top edge in the page */
    u8 w;      /**< width */
    u8 h;      /**< height */
} SpriteDef;

/* MATCHING: this unit's view of the table object copies as halfwords. */
extern SpriteDef gSpriteDefs[];

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
#define sLights ((*(FlatLights *)gFlatLights).l)

/** @brief A primitive handler: one entry of the handler table. */
typedef PACKET *(*PrimFunc)();

/* The handler table, eight rows of eight handlers. */
extern PrimFunc gTmdSortFuncs[8][8];

PACKET *SortTmdDivF3(TmdF3 *prim, SVECTOR *vtx, POLY_F3 *pkt, s32 n, s32 shift, GsOT *ot);
PACKET *SortTmdDivF4(TmdF4 *prim, SVECTOR *vtx, POLY_F4 *pkt, s32 n, s32 shift, GsOT *ot);
PACKET *SortTmdDivG3(TmdG3 *prim, SVECTOR *vtx, PACKET *packet, s32 n, s32 shift, GsOT *ot);
PACKET *SortTmdDivG4(TmdG4 *prim, SVECTOR *vtx, PACKET *packet, s32 n, s32 shift, GsOT *ot);
PACKET *SortTmdDivFT3(TmdFT3 *prim, SVECTOR *vtx, PACKET *packet, s32 n, s32 shift, GsOT *ot);
PACKET *SortTmdDivFT4(TmdFT4 *prim, SVECTOR *vtx, PACKET *packet, s32 n, s32 shift, GsOT *ot);
PACKET *SortTmdDivGT3(TmdGT3U *prim, SVECTOR *vtx, PACKET *packet, s32 n, s32 shift, GsOT *ot);
PACKET *SortTmdDivGT4(TmdGT4U *prim, SVECTOR *vtx, PACKET *packet, s32 n, s32 shift, GsOT *ot);
PACKET *SortTmdEnvMapGT3(TmdGT3 *prim, SVECTOR *vtx, SVECTOR *nrm, PACKET *packet, s32 n, s32 shift,
                         GsOT *ot);
PACKET *SortTmdEnvMapGT4(TmdGT4 *prim, SVECTOR *vtx, SVECTOR *nrm, volatile POLY_FT4 *pkt, s32 n,
                         s32 shift, GsOT *ot);
PACKET *SortTmdLitEnvMapGT3(TmdGT3 *prim, SVECTOR *vtx, SVECTOR *nrm, PACKET *packet, s32 n,
                            s32 shift, GsOT *ot);
PACKET *SortTmdLitEnvMapGT4(TmdGT4 *prim, SVECTOR *vtx, SVECTOR *nrm, PACKET *packet, s32 n,
                            s32 shift, GsOT *ot);
PACKET *SortTmdWireGT4(TmdGT4 *prim, SVECTOR *vtx, SVECTOR *nrm, PACKET *packet, s32 n, s32 shift,
                       GsOT *ot);
PACKET *SortTmdFastF3(TmdF3 *prim, SVECTOR *vtx, POLY_F3 *pkt, s32 n, s32 shift, GsOT *ot);
PACKET *SortTmdFastF4(TmdF4 *prim, SVECTOR *vtx, POLY_F4 *pkt, s32 n, s32 shift, GsOT *ot);
PACKET *SortTmdFastG3(TmdG3 *prim, SVECTOR *vtx, PACKET *packet, s32 n, s32 shift, GsOT *ot);
PACKET *SortTmdFastG4(TmdG4 *prim, SVECTOR *vtx, PACKET *packet, s32 n, s32 shift, GsOT *ot);
PACKET *SortTmdFastFT3(TmdFT3 *prim, SVECTOR *vtx, PACKET *packet, s32 n, s32 shift, GsOT *ot);

/* Handlers draw_tmd defines; their parameter types are that unit's. */
/* MATCHING: a per-unit view; here they only fill the PrimFunc table. */
PACKET *SortTmdFastFT4();
PACKET *SortTmdFastGT3();
PACKET *SortTmdFastGT4();
PACKET *SortTmdLitSemiGT3();
PACKET *SortTmdLitSemiGT4();
PACKET *SortTmdWireGT3();
PACKET *SortTmdLitGT3();
PACKET *SortTmdLitGT4();

void DrawSpriteTransformed(u16 id, Sprite2D *s, CVECTOR *color, u16 otz, GsOT *ot) {
    SpriteDef *e;
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

    e = &gSpriteDefs[id];
    p = (POLY_FT4 *)GsOUT_PACKET_P;
    *(u32 *)&p->r0 = color != NULL ? GPU_COM_TF4 << 24 | (color->cd << 16) | (color->b << 8) | color->g
                                   : GPU_COM_NTF4 << 24;
    if (color != NULL && (s8)color->r != -1) {
        *(u32 *)&p->r0 |= PRIM_SEMI_TRANS << 24;
    }
    w = e->w;
    h = e->h;
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
    *(u32 *)&p->u0 = (e->v << 8) | e->u | (e->clut << 16);
    if (color != NULL && (s8)color->r != -1) {
        *(u32 *)&p->u1 = ((*(u32 *)&p->u0 + w - 1) & 0xFFFF) | ((e->tpage | ((s8)color->r << 5)) << 16);
    } else {
        *(u32 *)&p->u1 = ((*(u32 *)&p->u0 + w - 1) & 0xFFFF) | (e->tpage << 16);
    }
    *(u32 *)&p->u2 = ((e->v + h - 1) << 8) | e->u;
    *(u32 *)&p->u3 = *(u32 *)&p->u2 + w - 1;
    tag = (u32 *)ot->org + otz;
    linkPrim(tag, p, POLY_FT4);
    GsOUT_PACKET_P = (u8 *)(p + 1);
}

void DrawSpriteFlip(u16 id, SVECTOR *pos, CVECTOR *color, s32 mode, u16 otz, GsOT *ot) {
    SpriteDef *e;
    /* MATCHING: volatile, so each chained vertex store reads its
     * first target back. */
    volatile POLY_FT4 *p;
    u32 *tag;
    u8 u0, u1, u2, u3;
    u8 v0, v1, v2, v3;

    e = &gSpriteDefs[id];
    p = (POLY_FT4 *)GsOUT_PACKET_P;
    *(u32 *)&p->r0 = color != NULL ? GPU_COM_TF4 << 24 | (color->cd << 16) | (color->b << 8) | color->g
                                   : GPU_COM_NTF4 << 24;
    if (color != NULL && (s8)color->r != -1) {
        *(u32 *)&p->r0 |= PRIM_SEMI_TRANS << 24;
    }
    p->x0 = p->x2 = pos->vx;
    p->y0 = p->y1 = pos->vy;
    p->x1 = p->x3 = pos->vx + e->w - 1;
    p->y2 = p->y3 = pos->vy + e->h - 1;
    u0 = u2 = e->u;
    v0 = v1 = e->v;
    u1 = u3 = e->u + e->w - 1;
    v2 = v3 = e->v + e->h - 1;
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
    *(u32 *)&p->u0 = (v0 << 8) | u0 | (e->clut << 16);
    if (color != NULL && (s8)color->r != -1) {
        *(u32 *)&p->u1 = (v1 << 8) | u1 | ((e->tpage | ((s8)color->r << 5)) << 16);
    } else {
        *(u32 *)&p->u1 = (v1 << 8) | u1 | (e->tpage << 16);
    }
    *(u32 *)&p->u2 = (v2 << 8) | u2;
    *(u32 *)&p->u3 = (v3 << 8) | u3;
    tag = (u32 *)ot->org + otz;
    linkPrim(tag, p, POLY_FT4);
    GsOUT_PACKET_P = (u8 *)(p + 1);
}

void DrawSpriteTransformedFlip(u16 id, Sprite2D *s, CVECTOR *color, u8 mode, u16 otz, GsOT *ot) {
    SpriteDef *e;
    POLY_FT4 *p;
    u32 *tag;
    s16 x0, x1, x2, x3;
    s16 y0, y1, y2, y3;
    u8 w;
    u8 h;
    s32 sn;
    s32 cs;

    e = &gSpriteDefs[id];
    p = (POLY_FT4 *)GsOUT_PACKET_P;
    *(u32 *)&p->r0 = color != NULL ? GPU_COM_TF4 << 24 | (color->cd << 16) | (color->b << 8) | color->g
                                   : GPU_COM_NTF4 << 24;
    if (color != NULL && (s8)color->r != -1) {
        *(u32 *)&p->r0 |= PRIM_SEMI_TRANS << 24;
    }
    w = e->w;
    h = e->h;
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
    x0 = x2 = e->u;
    y0 = y1 = e->v;
    x1 = x3 = e->u + w - 1;
    y2 = y3 = e->v + h - 1;
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
    *(u32 *)&p->u0 = (y0 << 8) | x0 | (e->clut << 16);
    if (color != NULL && (s8)color->r != -1) {
        *(u32 *)&p->u1 = (y1 << 8) | x1 | ((e->tpage | ((s8)color->r << 5)) << 16);
    } else {
        *(u32 *)&p->u1 = (y1 << 8) | x1 | (e->tpage << 16);
    }
    *(u32 *)&p->u2 = (y2 << 8) | x2;
    *(u32 *)&p->u3 = (y3 << 8) | x3;
    tag = (u32 *)ot->org + otz;
    linkPrim(tag, p, POLY_FT4);
    GsOUT_PACKET_P = (u8 *)(p + 1);
}

void DrawBillboard(u16 id, SVECTOR *size, CVECTOR *color, s32 shift, GsOT *ot) {
    SVECTOR sv[4];
    s32 v;
    s32 z;
    SpriteDef *e;
    POLY_FT4 *p;
    u32 *tag;

    e = &gSpriteDefs[id];
    p = (POLY_FT4 *)GsOUT_PACKET_P;
    gte_SetRotMatrix(&GsIDMATRIX);
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
    if (v & GTE_FLAG_ERROR) {
        return;
    }
    gte_stsxy3_ft4(p);
    gte_ldv0(&sv[3]);
    gte_rtps();
    gte_stflg(&v);
    if (v & GTE_FLAG_ERROR) {
        return;
    }
    gte_stsxy2(&p->x3);
    gte_avsz4();
    gte_stotz(&v);
    *(u32 *)&p->r0 = color != NULL ? GPU_COM_TF4 << 24 | (color->cd << 16) | (color->b << 8) | color->g
                                   : GPU_COM_NTF4 << 24;
    if (color != NULL && (s8)color->r != -1) {
        *(u32 *)&p->r0 |= PRIM_SEMI_TRANS << 24;
    }
    *(u32 *)&p->u0 = (e->v << 8) | e->u | (e->clut << 16);
    if (color != NULL && (s8)color->r != -1) {
        *(u32 *)&p->u1 =
            ((*(u32 *)&p->u0 + e->w - 1) & 0xFFFF) | ((e->tpage | ((s8)color->r << 5)) << 16);
    } else {
        *(u32 *)&p->u1 = ((*(u32 *)&p->u0 + e->w - 1) & 0xFFFF) | (e->tpage << 16);
    }
    *(u32 *)&p->u2 = ((e->v + e->h - 1) << 8) | e->u;
    /* MATCHING: the index read before the last UV store, as retail
     * schedules it. */
    z = v >> shift;
    *(u32 *)&p->u3 = *(u32 *)&p->u2 + e->w - 1;
    tag = (u32 *)ot->org + z;
    linkPrim(tag, p, POLY_FT4);
    GsOUT_PACKET_P = (u8 *)(p + 1);
}

void DrawStandingBillboard(u16 id, SVECTOR *size, CVECTOR *color, s32 shift, GsOT *ot) {
    SVECTOR sv[4];
    s32 v;
    s32 z;
    SpriteDef *e;
    POLY_FT4 *p;
    u32 *tag;

    e = &gSpriteDefs[id];
    p = (POLY_FT4 *)GsOUT_PACKET_P;
    gte_SetRotMatrix(&GsIDMATRIX);
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
    if (v & GTE_FLAG_ERROR) {
        return;
    }
    gte_stsxy3_ft4(p);
    gte_ldv0(&sv[3]);
    gte_rtps();
    gte_stflg(&v);
    if (v & GTE_FLAG_ERROR) {
        return;
    }
    gte_stsxy2(&p->x3);
    gte_avsz4();
    gte_stotz(&v);
    *(u32 *)&p->r0 = color != NULL ? GPU_COM_TF4 << 24 | (color->cd << 16) | (color->b << 8) | color->g
                                   : GPU_COM_NTF4 << 24;
    if (color != NULL && (s8)color->r != -1) {
        *(u32 *)&p->r0 |= PRIM_SEMI_TRANS << 24;
    }
    *(u32 *)&p->u0 = (e->v << 8) | e->u | (e->clut << 16);
    if (color != NULL && (s8)color->r != -1) {
        *(u32 *)&p->u1 =
            ((*(u32 *)&p->u0 + e->w - 1) & 0xFFFF) | ((e->tpage | ((s8)color->r << 5)) << 16);
    } else {
        *(u32 *)&p->u1 = ((*(u32 *)&p->u0 + e->w - 1) & 0xFFFF) | (e->tpage << 16);
    }
    *(u32 *)&p->u2 = ((e->v + e->h - 1) << 8) | e->u;
    /* MATCHING: the index read before the last UV store, as retail
     * schedules it. */
    z = v >> shift;
    *(u32 *)&p->u3 = *(u32 *)&p->u2 + e->w - 1;
    tag = (u32 *)ot->org + z;
    linkPrim(tag, p, POLY_FT4);
    GsOUT_PACKET_P = (u8 *)(p + 1);
}

void DrawBillboardRotated(u16 id, SVECTOR *size, CVECTOR *color, s16 angle, s32 shift, GsOT *ot) {
    SVECTOR sv[4];
    MATRIX m;
    s32 v;
    s32 z;
    SpriteDef *e;
    POLY_FT4 *p;
    u32 *tag;

    /* MATCHING: the entry before the matrix copy, so the matrix address is
     * built late and gets $s0. */
    e = &gSpriteDefs[id];
    m = GsIDMATRIX;
    p = (POLY_FT4 *)GsOUT_PACKET_P;
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
    if (v & GTE_FLAG_ERROR) {
        return;
    }
    gte_stsxy3_ft4(p);
    gte_ldv0(&sv[3]);
    gte_rtps();
    gte_stflg(&v);
    if (v & GTE_FLAG_ERROR) {
        return;
    }
    gte_stsxy2(&p->x3);
    gte_avsz4();
    gte_stotz(&v);
    *(u32 *)&p->r0 = color != NULL ? GPU_COM_TF4 << 24 | (color->cd << 16) | (color->b << 8) | color->g
                                   : GPU_COM_NTF4 << 24;
    if (color != NULL && (s8)color->r != -1) {
        *(u32 *)&p->r0 |= PRIM_SEMI_TRANS << 24;
    }
    *(u32 *)&p->u0 = (e->v << 8) | e->u | (e->clut << 16);
    if (color != NULL && (s8)color->r != -1) {
        *(u32 *)&p->u1 =
            ((*(u32 *)&p->u0 + e->w - 1) & 0xFFFF) | ((e->tpage | ((s8)color->r << 5)) << 16);
    } else {
        *(u32 *)&p->u1 = ((*(u32 *)&p->u0 + e->w - 1) & 0xFFFF) | (e->tpage << 16);
    }
    *(u32 *)&p->u2 = ((e->v + e->h - 1) << 8) | e->u;
    /* MATCHING: the index read before the last UV store, as retail
     * schedules it. */
    z = v >> shift;
    *(u32 *)&p->u3 = *(u32 *)&p->u2 + e->w - 1;
    tag = (u32 *)ot->org + z;
    linkPrim(tag, p, POLY_FT4);
    GsOUT_PACKET_P = (u8 *)(p + 1);
}

void DrawStandingBillboardRotated(u16 id, SVECTOR *size, CVECTOR *color, s16 angle, s32 shift, GsOT *ot) {
    SVECTOR sv[4];
    MATRIX m;
    s32 v;
    s32 z;
    SpriteDef *e;
    POLY_FT4 *p;
    u32 *tag;

    /* MATCHING: the entry before the matrix copy, so the matrix address is
     * built late and gets $s0. */
    e = &gSpriteDefs[id];
    m = GsIDMATRIX;
    p = (POLY_FT4 *)GsOUT_PACKET_P;
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
    if (v & GTE_FLAG_ERROR) {
        return;
    }
    gte_stsxy3_ft4(p);
    gte_ldv0(&sv[3]);
    gte_rtps();
    gte_stflg(&v);
    if (v & GTE_FLAG_ERROR) {
        return;
    }
    gte_stsxy2(&p->x3);
    gte_avsz4();
    gte_stotz(&v);
    *(u32 *)&p->r0 = color != NULL ? GPU_COM_TF4 << 24 | (color->cd << 16) | (color->b << 8) | color->g
                                   : GPU_COM_NTF4 << 24;
    if (color != NULL && (s8)color->r != -1) {
        *(u32 *)&p->r0 |= PRIM_SEMI_TRANS << 24;
    }
    *(u32 *)&p->u0 = (e->v << 8) | e->u | (e->clut << 16);
    if (color != NULL && (s8)color->r != -1) {
        *(u32 *)&p->u1 =
            ((*(u32 *)&p->u0 + e->w - 1) & 0xFFFF) | ((e->tpage | ((s8)color->r << 5)) << 16);
    } else {
        *(u32 *)&p->u1 = ((*(u32 *)&p->u0 + e->w - 1) & 0xFFFF) | (e->tpage << 16);
    }
    *(u32 *)&p->u2 = ((e->v + e->h - 1) << 8) | e->u;
    /* MATCHING: the index read before the last UV store, as retail
     * schedules it. */
    z = v >> shift;
    *(u32 *)&p->u3 = *(u32 *)&p->u2 + e->w - 1;
    tag = (u32 *)ot->org + z;
    linkPrim(tag, p, POLY_FT4);
    GsOUT_PACKET_P = (u8 *)(p + 1);
}

void DrawStandingBillboardFog(u16 id, SVECTOR *size, CVECTOR *color, s32 shift, GsOT *ot) {
    SVECTOR sv[4];
    CVECTOR c;
    s32 v;
    s32 dp;
    s32 z;
    SpriteDef *e;
    POLY_FT4 *p;
    u32 *tag;

    e = &gSpriteDefs[id];
    p = (POLY_FT4 *)GsOUT_PACKET_P;
    gte_SetRotMatrix(&GsIDMATRIX);
    sv[0].vx = sv[2].vx = -(size->vx >> 1);
    sv[0].vy = sv[1].vy = -size->vy + 1;
    sv[1].vx = sv[3].vx = sv[0].vx + size->vx - 1;
    sv[2].vy = sv[3].vy = 0;
    sv[0].vz = sv[1].vz = sv[2].vz = sv[3].vz = 0;
    gte_ldv3(&sv[0], &sv[1], &sv[2]);
    gte_rtpt();
    gte_stflg(&v);
    if (v & GTE_FLAG_ERROR) {
        return;
    }
    gte_stsxy3_ft4(p);
    gte_ldv0(&sv[3]);
    gte_rtps();
    gte_stflg(&v);
    if (v & GTE_FLAG_ERROR) {
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
        *(u32 *)&p->r0 |= PRIM_SEMI_TRANS << 24;
    }
    *(u32 *)&p->u0 = (e->v << 8) | e->u | (e->clut << 16);
    if (color != NULL && (s8)color->r != -1) {
        *(u32 *)&p->u1 =
            ((*(u32 *)&p->u0 + e->w - 1) & 0xFFFF) | ((e->tpage | ((s8)color->r << 5)) << 16);
    } else {
        *(u32 *)&p->u1 = ((*(u32 *)&p->u0 + e->w - 1) & 0xFFFF) | (e->tpage << 16);
    }
    *(u32 *)&p->u2 = ((e->v + e->h - 1) << 8) | e->u;
    z = v >> shift;
    *(u32 *)&p->u3 = *(u32 *)&p->u2 + e->w - 1;
    tag = (u32 *)ot->org + z;
    linkPrim(tag, p, POLY_FT4);
    GsOUT_PACKET_P = (u8 *)(p + 1);
}

void DefineSprite(u16 id, u8 mode, s32 w, s32 h, s32 page, s32 u, s32 v, u16 clutX, s32 clutY) {
    gSpriteDefs[id].tpage = (u8)page | (mode << 7);
    gSpriteDefs[id].clut = (clutY << 6) | (clutX >> 4);
    gSpriteDefs[id].u = u;
    gSpriteDefs[id].v = v;
    gSpriteDefs[id].w = w;
    gSpriteDefs[id].h = h;
}

void DrawSprite(u16 id, SVECTOR *pos, CVECTOR *color, s32 mode, GsOT *ot) {
    SpriteDef *e;
    u32 *p;
    u32 *q;
    u32 *tag;
    u32 c;

    e = &gSpriteDefs[id];
    p = (u32 *)GsOUT_PACKET_P;
    /* MATCHING: a ternary, and the semi-transparency term through a local,
     * keep cc1 from regrouping the constants. */
    p[1] = color != NULL ? PRIM_SPRT << 24 | (color->cd << 16) | (color->b << 8) | color->g
                         : (PRIM_SPRT | PRIM_RAW) << 24;
    if (color != NULL && (s8)color->r != -1) {
        p[1] |= PRIM_SEMI_TRANS << 24;
    }
    p[2] = (pos->vy << 16) | (u16)pos->vx;
    p[3] = (e->v << 8) | e->u | (e->clut << 16);
    p[4] = (e->h << 16) | e->w;
    tag = (u32 *)ot->org + (u16)mode;
    p[0] = (*tag & 0xFFFFFF) | (sizeof(SPRT) / 4 - 1) << 24;
    q = p + 5;
    if (color != NULL && (s8)color->r != -1) {
        c = (s8)color->r << 5 | PRIM_TPAGE_WORD;
        q[1] = e->tpage | c;
    } else {
        q[1] = e->tpage | PRIM_TPAGE_WORD;
    }
    q[0] = ((u32)p & 0xFFFFFF) | (sizeof(DR_TPAGE) / 4 - 1) << 24;
    *tag = (u32)q & 0xFFFFFF;
    GsOUT_PACKET_P = (u8 *)(q + 2);
}

PACKET *SortTmdDivF3(TmdF3 *prim, SVECTOR *vtx, POLY_F3 *pkt, s32 n, s32 shift, GsOT *ot) {
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
            linkPrim(tag, pkt, POLY_F3);
            pkt++;
        }
        if (v < SUBDIVIDE_Z) {
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
                if (v & GTE_FLAG_REJECT) {
                    continue;
                }
                gte_stsxy3_f3(pkt);
                gte_avsz3();
                gte_stotz(&v);
                *(u32 *)&pkt->r0 = *(u32 *)&prim->rgb;
                tag = (u32 *)ot->org + (v >> shift);
                linkPrim(tag, pkt, POLY_F3);
                pkt++;
            }
        }
    }
    return (PACKET *)pkt;
}

PACKET *SortTmdDivF4(TmdF4 *prim, SVECTOR *vtx, POLY_F4 *pkt, s32 n, s32 shift, GsOT *ot) {
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
            linkPrim(tag, pkt, POLY_F4);
            pkt++;
        }
        tag = (u32 *)ot->org + (v >> shift);
        if (v < SUBDIVIDE_Z) {
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
                if (v & GTE_FLAG_REJECT) {
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
                if (v & GTE_FLAG_REJECT) {
                    continue;
                }
                gte_stsxy2(&pkt->x3);
                *(u32 *)&pkt->r0 = *(u32 *)&prim->rgb;
                linkPrim(tag, pkt, POLY_F4);
                pkt++;
            }
        }
    }
    return (PACKET *)pkt;
}

PACKET *SortTmdDivG3(TmdG3 *prim, SVECTOR *vtx, PACKET *packet, s32 n, s32 shift, GsOT *ot) {
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
            linkPrim(tag, pkt, POLY_G3);
            pkt++;
        }
        tag = (u32 *)ot->org + (v >> shift);
        if (v < SUBDIVIDE_Z) {
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
                if (v & GTE_FLAG_REJECT) {
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
                linkPrim(tag, pkt, POLY_G3);
                pkt++;
            }
        }
    }
    return (PACKET *)pkt;
}

PACKET *SortTmdDivG4(TmdG4 *prim, SVECTOR *vtx, PACKET *packet, s32 n, s32 shift, GsOT *ot) {
    POLY_G4 *pkt;
    SVECTOR m[5];
    s32 sz[4];
    s32 flg;
    s32 v;
    s32 dp;
    s32 i;
    s32 j;
    u32 *tag;
    u32 r01, g01, b01;
    u32 r02, g02, b02;
    u32 r13, g13, b13;
    u32 r23, g23, b23;
    u32 rc, gc, bc;

    /* MATCHING: a copy of the parameter, as in SortTmdDivFT3. */
    pkt = (POLY_G4 *)packet;
    for (i = 0; i < n; i++, prim++) {
        gte_ldv3(&vtx[prim->v0], &vtx[prim->v1], &vtx[prim->v2]);
        gte_rtpt();
        gte_stflg(&flg);
        gte_nclip();
        gte_stopz(&v);
        if (v <= 0) {
            continue;
        }
        gte_stsxy3_g4(pkt);
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
            gte_ldrgb3(&prim->c0, &prim->c1, &prim->c2);
            gte_lddp(dp);
            gte_dpct();
            gte_strgb3(&pkt->r0, &pkt->r1, &pkt->r2);
            gte_ldrgb(&prim->c3);
            gte_lddp(dp);
            gte_dpcs();
            gte_strgb(&pkt->r3);
            pkt->code = prim->mode;
            tag = (u32 *)ot->org + (v >> shift) + 1;
            linkPrim(tag, pkt, POLY_G4);
            pkt++;
        }
        tag = (u32 *)ot->org + (v >> shift);
        if (v < SUBDIVIDE_Z) {
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
            /* MATCHING: u32 halves give srl. */
            r01 = (u32)(prim->c0.r + prim->c1.r) >> 1;
            g01 = (u32)(prim->c0.g + prim->c1.g) >> 1;
            b01 = (u32)(prim->c0.b + prim->c1.b) >> 1;
            r02 = (u32)(prim->c0.r + prim->c2.r) >> 1;
            g02 = (u32)(prim->c0.g + prim->c2.g) >> 1;
            b02 = (u32)(prim->c0.b + prim->c2.b) >> 1;
            r13 = (u32)(prim->c1.r + prim->c3.r) >> 1;
            g13 = (u32)(prim->c1.g + prim->c3.g) >> 1;
            b13 = (u32)(prim->c1.b + prim->c3.b) >> 1;
            r23 = (u32)(prim->c2.r + prim->c3.r) >> 1;
            g23 = (u32)(prim->c2.g + prim->c3.g) >> 1;
            b23 = (u32)(prim->c2.b + prim->c3.b) >> 1;
            rc = (u32)(prim->c0.r + prim->c1.r + prim->c2.r + prim->c3.r) >> 2;
            gc = (u32)(prim->c0.g + prim->c1.g + prim->c2.g + prim->c3.g) >> 2;
            bc = (u32)(prim->c0.b + prim->c1.b + prim->c2.b + prim->c3.b) >> 2;
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
                if (v & GTE_FLAG_REJECT) {
                    continue;
                }
                gte_stsxy3_g4(pkt);
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
                if (v & GTE_FLAG_REJECT) {
                    continue;
                }
                gte_stsxy2(&pkt->x3);
                switch (j) {
                    case 0:
                        *(u32 *)&pkt->r0 =
                            (prim->mode << 24) | (prim->c0.b << 16) | (prim->c0.g << 8) | prim->c0.r;
                        *(u32 *)&pkt->r1 = (b01 << 16) | (g01 << 8) | r01;
                        *(u32 *)&pkt->r2 = (b02 << 16) | (g02 << 8) | r02;
                        *(u32 *)&pkt->r3 = (bc << 16) | (gc << 8) | rc;
                        break;
                    case 1:
                        *(u32 *)&pkt->r0 = (prim->mode << 24) | (b01 << 16) | (g01 << 8) | r01;
                        *(u32 *)&pkt->r1 = (prim->c1.b << 16) | (prim->c1.g << 8) | prim->c1.r;
                        *(u32 *)&pkt->r2 = (bc << 16) | (gc << 8) | rc;
                        *(u32 *)&pkt->r3 = (b13 << 16) | (g13 << 8) | r13;
                        break;
                    case 2:
                        *(u32 *)&pkt->r0 = (prim->mode << 24) | (b02 << 16) | (g02 << 8) | r02;
                        *(u32 *)&pkt->r1 = (bc << 16) | (gc << 8) | rc;
                        *(u32 *)&pkt->r2 = (prim->c2.b << 16) | (prim->c2.g << 8) | prim->c2.r;
                        *(u32 *)&pkt->r3 = (b23 << 16) | (g23 << 8) | r23;
                        break;
                    case 3:
                        *(u32 *)&pkt->r0 = (prim->mode << 24) | (bc << 16) | (gc << 8) | rc;
                        *(u32 *)&pkt->r1 = (b13 << 16) | (g13 << 8) | r13;
                        *(u32 *)&pkt->r2 = (b23 << 16) | (g23 << 8) | r23;
                        *(u32 *)&pkt->r3 = (prim->c3.b << 16) | (prim->c3.g << 8) | prim->c3.r;
                        break;
                }
                linkPrim(tag, pkt, POLY_G4);
                pkt++;
            }
        }
    }
    return (PACKET *)pkt;
}

PACKET *SortTmdDivFT3(TmdFT3 *prim, SVECTOR *vtx, PACKET *packet, s32 n, s32 shift, GsOT *ot) {
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
            ((volatile POLY_FT3 *)pkt)->code = prim->mode & ~PRIM_RAW;
            *(u32 *)&pkt->u0 = *(u32 *)&prim->uv0;
            *(u32 *)&pkt->u1 = *(u32 *)&prim->uv1;
            *(u16 *)&pkt->u2 = *(u16 *)&prim->uv2;
            tag = (u32 *)ot->org + (v >> shift) + 1;
            linkPrim(tag, pkt, POLY_FT3);
            pkt++;
        }
        tag = (u32 *)ot->org + (v >> shift);
        if (v < SUBDIVIDE_Z) {
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
                if (v & GTE_FLAG_REJECT) {
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
                ((volatile POLY_FT3 *)pkt)->code = prim->mode & ~PRIM_RAW;
                linkPrim(tag, pkt, POLY_FT3);
                pkt++;
            }
        }
    }
    return (PACKET *)pkt;
}

PACKET *SortTmdDivFT4(TmdFT4 *prim, SVECTOR *vtx, PACKET *packet, s32 n, s32 shift, GsOT *ot) {
    POLY_FT4 *pkt;
    SVECTOR m[5];
    s32 sz[4];
    s32 flg;
    s32 v;
    s32 dp;
    s32 i;
    s32 j;
    u32 *tag;
    /* MATCHING: u8 midpoints; the averages of two bytes need no mask. */
    u8 u01, v01, u02, v02, u13, v13, u23, v23, uc, vc;

    /* MATCHING: a copy of the parameter, as in SortTmdDivFT3. */
    pkt = (POLY_FT4 *)packet;
    for (i = 0; i < n; i++, prim++) {
        gte_ldv3(&vtx[prim->v0], &vtx[prim->v1], &vtx[prim->v2]);
        gte_rtpt();
        gte_stflg(&flg);
        gte_nclip();
        gte_stopz(&v);
        if (v <= 0) {
            continue;
        }
        gte_stsxy3_ft4(pkt);
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
            ((volatile POLY_FT4 *)pkt)->code = prim->mode & ~PRIM_RAW;
            *(u32 *)&pkt->u0 = *(u32 *)&prim->uv0;
            *(u32 *)&pkt->u1 = *(u32 *)&prim->uv1;
            *(u16 *)&pkt->u2 = *(u16 *)&prim->uv2;
            *(u16 *)&pkt->u3 = *(u16 *)&prim->uv3;
            tag = (u32 *)ot->org + (v >> shift) + 1;
            linkPrim(tag, pkt, POLY_FT4);
            pkt++;
        }
        tag = (u32 *)ot->org + (v >> shift);
        if (v < SUBDIVIDE_Z) {
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
            /* MATCHING: every u before every v. */
            u01 = (u32)(prim->uv0.u + prim->uv1.u) >> 1;
            u02 = (u32)(prim->uv0.u + prim->uv2.u) >> 1;
            u13 = (u32)(prim->uv1.u + prim->uv3.u) >> 1;
            u23 = (u32)(prim->uv2.u + prim->uv3.u) >> 1;
            uc = (u32)(prim->uv0.u + prim->uv1.u + prim->uv2.u + prim->uv3.u) >> 2;
            v01 = (u32)(prim->uv0.v + prim->uv1.v) >> 1;
            v02 = (u32)(prim->uv0.v + prim->uv2.v) >> 1;
            v13 = (u32)(prim->uv1.v + prim->uv3.v) >> 1;
            v23 = (u32)(prim->uv2.v + prim->uv3.v) >> 1;
            vc = (u32)(prim->uv0.v + prim->uv1.v + prim->uv2.v + prim->uv3.v) >> 2;
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
                if (v & GTE_FLAG_REJECT) {
                    continue;
                }
                gte_stsxy3_ft4(pkt);
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
                if (v & GTE_FLAG_REJECT) {
                    continue;
                }
                gte_stsxy2(&pkt->x3);
                switch (j) {
                    case 0:
                        *(u32 *)&pkt->u0 = (prim->uv0.hi << 16) | (prim->uv0.v << 8) | prim->uv0.u;
                        *(u32 *)&pkt->u1 = (prim->uv1.hi << 16) | (v01 << 8) | u01;
                        *(u32 *)&pkt->u2 = (v02 << 8) | u02;
                        *(u32 *)&pkt->u3 = (vc << 8) | uc;
                        break;
                    case 1:
                        *(u32 *)&pkt->u0 = (prim->uv0.hi << 16) | (v01 << 8) | u01;
                        *(u32 *)&pkt->u1 = (prim->uv1.hi << 16) | (prim->uv1.v << 8) | prim->uv1.u;
                        *(u32 *)&pkt->u2 = (vc << 8) | uc;
                        *(u32 *)&pkt->u3 = (v13 << 8) | u13;
                        break;
                    case 2:
                        *(u32 *)&pkt->u0 = (prim->uv0.hi << 16) | (v02 << 8) | u02;
                        *(u32 *)&pkt->u1 = (prim->uv1.hi << 16) | (vc << 8) | uc;
                        *(u32 *)&pkt->u2 = (prim->uv2.v << 8) | prim->uv2.u;
                        *(u32 *)&pkt->u3 = (v23 << 8) | u23;
                        break;
                    case 3:
                        *(u32 *)&pkt->u0 = (prim->uv0.hi << 16) | (vc << 8) | uc;
                        *(u32 *)&pkt->u1 = (prim->uv1.hi << 16) | (v13 << 8) | u13;
                        *(u32 *)&pkt->u2 = (v23 << 8) | u23;
                        *(u32 *)&pkt->u3 = (prim->uv3.v << 8) | prim->uv3.u;
                        break;
                }
                gte_ldrgb(&prim->rgb);
                gte_lddp(dp);
                gte_dpcs();
                gte_strgb(&pkt->r0);
                ((volatile POLY_FT4 *)pkt)->code = prim->mode & ~PRIM_RAW;
                linkPrim(tag, pkt, POLY_FT4);
                pkt++;
            }
        }
    }
    return (PACKET *)pkt;
}

PACKET *SortTmdDivGT3(TmdGT3U *prim, SVECTOR *vtx, PACKET *packet, s32 n, s32 shift, GsOT *ot) {
    POLY_GT3 *pkt;
    SVECTOR m[3];
    s32 sz[3];
    s32 flg;
    s32 v;
    s32 dp;
    s32 i;
    s32 j;
    u32 *tag;
    /* MATCHING: u8 UV midpoints; the averages of two bytes need no mask. */
    u8 u01, v01, u02, v02, u12, v12;
    u32 r01, g01, b01;
    u32 r02, g02, b02;
    u32 r12, g12, b12;

    /* MATCHING: a copy of the parameter, as in SortTmdDivFT3. */
    pkt = (POLY_GT3 *)packet;
    for (i = 0; i < n; i++, prim++) {
        gte_ldv3(&vtx[prim->v0], &vtx[prim->v1], &vtx[prim->v2]);
        gte_rtpt();
        gte_stflg(&flg);
        gte_nclip();
        gte_stopz(&v);
        if (v <= 0) {
            continue;
        }
        gte_stsxy3_gt3(pkt);
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
            ((volatile POLY_GT3 *)pkt)->code = prim->mode & ~PRIM_RAW;
            *(u32 *)&pkt->u0 = *(u32 *)&prim->uv0;
            *(u32 *)&pkt->u1 = *(u32 *)&prim->uv1;
            *(u16 *)&pkt->u2 = *(u16 *)&prim->uv2;
            tag = (u32 *)ot->org + (v >> shift) + 1;
            linkPrim(tag, pkt, POLY_GT3);
            pkt++;
        }
        tag = (u32 *)ot->org + (v >> shift);
        if (v < SUBDIVIDE_Z) {
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
            u02 = (u32)(prim->uv0.u + prim->uv2.u) >> 1;
            v02 = (u32)(prim->uv0.v + prim->uv2.v) >> 1;
            u12 = (u32)(prim->uv1.u + prim->uv2.u) >> 1;
            v12 = (u32)(prim->uv1.v + prim->uv2.v) >> 1;
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
                if (v & GTE_FLAG_REJECT) {
                    continue;
                }
                gte_stsxy3_gt3(pkt);
                /* MATCHING: the UV words first, then the colours; the order
                 * decides which invariants loop.c hoists. */
                switch (j) {
                    case 0:
                        *(u32 *)&pkt->u0 = (prim->uv0.hi << 16) | (prim->uv0.v << 8) | prim->uv0.u;
                        *(u32 *)&pkt->u1 = (prim->uv1.hi << 16) | (v01 << 8) | u01;
                        *(u32 *)&pkt->u2 = (v02 << 8) | u02;
                        *(u32 *)&pkt->r0 =
                            GPU_COM_TG3 << 24 | (prim->c0.b << 16) | (prim->c0.g << 8) | prim->c0.r;
                        *(u32 *)&pkt->r1 = (b01 << 16) | (g01 << 8) | r01;
                        *(u32 *)&pkt->r2 = (b02 << 16) | (g02 << 8) | r02;
                        break;
                    case 1:
                        *(u32 *)&pkt->u0 = (prim->uv0.hi << 16) | (v01 << 8) | u01;
                        *(u32 *)&pkt->u1 = (prim->uv1.hi << 16) | (v12 << 8) | u12;
                        *(u32 *)&pkt->u2 = (v02 << 8) | u02;
                        *(u32 *)&pkt->r0 = GPU_COM_TG3 << 24 | (b01 << 16) | (g01 << 8) | r01;
                        *(u32 *)&pkt->r1 = (b12 << 16) | (g12 << 8) | r12;
                        *(u32 *)&pkt->r2 = (b02 << 16) | (g02 << 8) | r02;
                        break;
                    case 2:
                        *(u32 *)&pkt->u0 = (prim->uv0.hi << 16) | (v01 << 8) | u01;
                        *(u32 *)&pkt->u1 = (prim->uv1.hi << 16) | (prim->uv1.v << 8) | prim->uv1.u;
                        *(u32 *)&pkt->u2 = (v12 << 8) | u12;
                        *(u32 *)&pkt->r0 = GPU_COM_TG3 << 24 | (b01 << 16) | (g01 << 8) | r01;
                        *(u32 *)&pkt->r1 = (prim->c1.b << 16) | (prim->c1.g << 8) | prim->c1.r;
                        *(u32 *)&pkt->r2 = (b12 << 16) | (g12 << 8) | r12;
                        break;
                    case 3:
                        *(u32 *)&pkt->u0 = (prim->uv0.hi << 16) | (v02 << 8) | u02;
                        *(u32 *)&pkt->u1 = (prim->uv1.hi << 16) | (v12 << 8) | u12;
                        *(u32 *)&pkt->u2 = (prim->uv2.v << 8) | prim->uv2.u;
                        *(u32 *)&pkt->r0 = GPU_COM_TG3 << 24 | (b02 << 16) | (g02 << 8) | r02;
                        *(u32 *)&pkt->r1 = (b12 << 16) | (g12 << 8) | r12;
                        *(u32 *)&pkt->r2 = (prim->c2.b << 16) | (prim->c2.g << 8) | prim->c2.r;
                        break;
                }
                ((volatile POLY_GT3 *)pkt)->code = prim->mode & ~PRIM_RAW;
                linkPrim(tag, pkt, POLY_GT3);
                pkt++;
            }
        }
    }
    return (PACKET *)pkt;
}

PACKET *SortTmdDivGT4(TmdGT4U *prim, SVECTOR *vtx, PACKET *packet, s32 n, s32 shift, GsOT *ot) {
    POLY_GT4 *pkt;
    SVECTOR m[5];
    s32 sz[4];
    s32 flg;
    s32 v;
    s32 dp;
    s32 i;
    s32 j;
    u32 *tag;
    /* MATCHING: u8 UV midpoints, and grouped u, v, r, g, b so the spill
     * slots fall in retail's order. */
    u8 u01, u02, u13, u23, uc;
    u8 v01, v02, v13, v23, vc;
    u32 r01, r02, r13, r23, rc;
    u32 g01, g02, g13, g23, gc;
    u32 b01, b02, b13, b23, bc;

    /* MATCHING: a copy of the parameter, as in SortTmdDivFT3. */
    pkt = (POLY_GT4 *)packet;
    for (i = 0; i < n; i++, prim++) {
        gte_ldv3(&vtx[prim->v0], &vtx[prim->v1], &vtx[prim->v2]);
        gte_rtpt();
        gte_stflg(&flg);
        gte_nclip();
        gte_stopz(&v);
        if (v <= 0) {
            continue;
        }
        gte_stsxy3_gt4(pkt);
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
            gte_ldrgb3(&prim->c0, &prim->c1, &prim->c2);
            gte_lddp(dp);
            gte_dpct();
            gte_strgb3(&pkt->r0, &pkt->r1, &pkt->r2);
            gte_ldrgb(&prim->c3);
            gte_lddp(dp);
            gte_dpcs();
            gte_strgb(&pkt->r3);
            ((volatile POLY_GT4 *)pkt)->code = prim->mode & ~PRIM_RAW;
            *(u32 *)&pkt->u0 = *(u32 *)&prim->uv0;
            *(u32 *)&pkt->u1 = *(u32 *)&prim->uv1;
            *(u16 *)&pkt->u2 = *(u16 *)&prim->uv2;
            *(u16 *)&pkt->u3 = *(u16 *)&prim->uv3;
            tag = (u32 *)ot->org + (v >> shift) + 1;
            linkPrim(tag, pkt, POLY_GT4);
            pkt++;
        }
        tag = (u32 *)ot->org + (v >> shift);
        if (v < SUBDIVIDE_Z) {
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
            u01 = (u32)(prim->uv0.u + prim->uv1.u) >> 1;
            u02 = (u32)(prim->uv0.u + prim->uv2.u) >> 1;
            u13 = (u32)(prim->uv1.u + prim->uv3.u) >> 1;
            u23 = (u32)(prim->uv2.u + prim->uv3.u) >> 1;
            uc = (u32)(prim->uv0.u + prim->uv1.u + prim->uv2.u + prim->uv3.u) >> 2;
            v01 = (u32)(prim->uv0.v + prim->uv1.v) >> 1;
            v02 = (u32)(prim->uv0.v + prim->uv2.v) >> 1;
            v13 = (u32)(prim->uv1.v + prim->uv3.v) >> 1;
            v23 = (u32)(prim->uv2.v + prim->uv3.v) >> 1;
            vc = (u32)(prim->uv0.v + prim->uv1.v + prim->uv2.v + prim->uv3.v) >> 2;
            r01 = (u32)(prim->c0.r + prim->c1.r) >> 1;
            g01 = (u32)(prim->c0.g + prim->c1.g) >> 1;
            b01 = (u32)(prim->c0.b + prim->c1.b) >> 1;
            r02 = (u32)(prim->c0.r + prim->c2.r) >> 1;
            g02 = (u32)(prim->c0.g + prim->c2.g) >> 1;
            b02 = (u32)(prim->c0.b + prim->c2.b) >> 1;
            r13 = (u32)(prim->c1.r + prim->c3.r) >> 1;
            g13 = (u32)(prim->c1.g + prim->c3.g) >> 1;
            b13 = (u32)(prim->c1.b + prim->c3.b) >> 1;
            r23 = (u32)(prim->c2.r + prim->c3.r) >> 1;
            g23 = (u32)(prim->c2.g + prim->c3.g) >> 1;
            b23 = (u32)(prim->c2.b + prim->c3.b) >> 1;
            rc = (u32)(prim->c0.r + prim->c1.r + prim->c2.r + prim->c3.r) >> 2;
            gc = (u32)(prim->c0.g + prim->c1.g + prim->c2.g + prim->c3.g) >> 2;
            bc = (u32)(prim->c0.b + prim->c1.b + prim->c2.b + prim->c3.b) >> 2;
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
                if (v & GTE_FLAG_REJECT) {
                    continue;
                }
                gte_stsxy3_gt4(pkt);
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
                if (v & GTE_FLAG_REJECT) {
                    continue;
                }
                gte_stsxy2(&pkt->x3);
                switch (j) {
                    case 0:
                        *(u32 *)&pkt->u0 = (prim->uv0.hi << 16) | (prim->uv0.v << 8) | prim->uv0.u;
                        *(u32 *)&pkt->u1 = (prim->uv1.hi << 16) | (v01 << 8) | u01;
                        *(u32 *)&pkt->u2 = (v02 << 8) | u02;
                        *(u32 *)&pkt->u3 = (vc << 8) | uc;
                        *(u32 *)&pkt->r0 =
                            GPU_COM_TG4 << 24 | (prim->c0.b << 16) | (prim->c0.g << 8) | prim->c0.r;
                        *(u32 *)&pkt->r1 = (b01 << 16) | (g01 << 8) | r01;
                        *(u32 *)&pkt->r2 = (b02 << 16) | (g02 << 8) | r02;
                        *(u32 *)&pkt->r3 = (bc << 16) | (gc << 8) | rc;
                        break;
                    case 1:
                        *(u32 *)&pkt->u0 = (prim->uv0.hi << 16) | (v01 << 8) | u01;
                        *(u32 *)&pkt->u1 = (prim->uv1.hi << 16) | (prim->uv1.v << 8) | prim->uv1.u;
                        *(u32 *)&pkt->u2 = (vc << 8) | uc;
                        *(u32 *)&pkt->u3 = (v13 << 8) | u13;
                        *(u32 *)&pkt->r0 = GPU_COM_TG4 << 24 | (b01 << 16) | (g01 << 8) | r01;
                        *(u32 *)&pkt->r1 = (prim->c1.b << 16) | (prim->c1.g << 8) | prim->c1.r;
                        *(u32 *)&pkt->r2 = (bc << 16) | (gc << 8) | rc;
                        *(u32 *)&pkt->r3 = (b13 << 16) | (g13 << 8) | r13;
                        break;
                    case 2:
                        *(u32 *)&pkt->u0 = (prim->uv0.hi << 16) | (v02 << 8) | u02;
                        *(u32 *)&pkt->u1 = (prim->uv1.hi << 16) | (vc << 8) | uc;
                        *(u32 *)&pkt->u2 = (prim->uv2.v << 8) | prim->uv2.u;
                        *(u32 *)&pkt->u3 = (v23 << 8) | u23;
                        *(u32 *)&pkt->r0 = GPU_COM_TG4 << 24 | (b02 << 16) | (g02 << 8) | r02;
                        *(u32 *)&pkt->r1 = (bc << 16) | (gc << 8) | rc;
                        *(u32 *)&pkt->r2 = (prim->c2.b << 16) | (prim->c2.g << 8) | prim->c2.r;
                        *(u32 *)&pkt->r3 = (b23 << 16) | (g23 << 8) | r23;
                        break;
                    case 3:
                        *(u32 *)&pkt->u0 = (prim->uv0.hi << 16) | (vc << 8) | uc;
                        *(u32 *)&pkt->u1 = (prim->uv1.hi << 16) | (v13 << 8) | u13;
                        *(u32 *)&pkt->u2 = (v23 << 8) | u23;
                        *(u32 *)&pkt->u3 = (prim->uv3.v << 8) | prim->uv3.u;
                        *(u32 *)&pkt->r0 = GPU_COM_TG4 << 24 | (bc << 16) | (gc << 8) | rc;
                        *(u32 *)&pkt->r1 = (b13 << 16) | (g13 << 8) | r13;
                        *(u32 *)&pkt->r2 = (b23 << 16) | (g23 << 8) | r23;
                        *(u32 *)&pkt->r3 = (prim->c3.b << 16) | (prim->c3.g << 8) | prim->c3.r;
                        break;
                }
                ((volatile POLY_GT4 *)pkt)->code = prim->mode & ~PRIM_RAW;
                linkPrim(tag, pkt, POLY_GT4);
                pkt++;
            }
        }
    }
    return (PACKET *)pkt;
}

PACKET *SortTmdEnvMapGT3(TmdGT3 *prim, SVECTOR *vtx, SVECTOR *nrm, PACKET *packet, s32 n, s32 shift,
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
    page = (s16)(PSDIDX ^ 1) << 4;
    for (i = 0; i < n; i++, prim++) {
        gte_ldv3(&vtx[prim->v0], &vtx[prim->v1], &vtx[prim->v2]);
        gte_rtpt();
        gte_stflg(&v);
        if (v & GTE_FLAG_REJECT) {
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
        linkPrim(tag, pkt, POLY_FT3);
        pkt++;
    }
    return (PACKET *)pkt;
}

/* MATCHING: the packet parameter volatile and used directly, so loop builds
 * no reduced pointer beside it; the unused pair puts v at sp+0x18. */
PACKET *SortTmdEnvMapGT4(TmdGT4 *prim, SVECTOR *vtx, SVECTOR *nrm, volatile POLY_FT4 *pkt, s32 n,
                         s32 shift, GsOT *ot) {
    VECTOR mac;
    s32 unused[2];
    s32 v;
    s32 page;
    s32 y;
    s32 i;
    u32 *tag;

    page = (s16)(PSDIDX ^ 1) << 4;
    for (i = 0; i < n; i++, prim++) {
        gte_ldv3(&vtx[prim->v0], &vtx[prim->v1], &vtx[prim->v2]);
        gte_rtpt();
        gte_stflg(&v);
        if (v & GTE_FLAG_REJECT) {
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
        if (v & GTE_FLAG_REJECT) {
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
        linkPrim(tag, pkt, POLY_FT4);
        pkt++;
    }
    return (PACKET *)pkt;
}

PACKET *SortTmdLitEnvMapGT3(TmdGT3 *prim, SVECTOR *vtx, SVECTOR *nrm, PACKET *packet, s32 n,
                            s32 shift, GsOT *ot) {
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
        if (v & GTE_FLAG_REJECT) {
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
        linkPrim(tag, pkt, POLY_FT3);
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
        linkPrim(tag, g, POLY_GT3);
        gte_strgb3_gt3(g);
        /* MATCHING: a volatile store, which loop cannot rebase onto its
         * reduced pointer. */
        ((volatile POLY_GT3 *)g)->code = 0x34;
        pkt = (POLY_FT3 *)(g + 1);
    }
    return (PACKET *)pkt;
}

PACKET *SortTmdLitEnvMapGT4(TmdGT4 *prim, SVECTOR *vtx, SVECTOR *nrm, PACKET *packet, s32 n,
                            s32 shift, GsOT *ot) {
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
        if (v & GTE_FLAG_REJECT) {
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
        if (v & GTE_FLAG_REJECT) {
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
        linkPrim(tag, pkt, POLY_FT4);
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
        linkPrim(tag, g, POLY_GT4);
        gte_strgb(&g->r3);
        /* MATCHING: a volatile store, which loop cannot rebase onto its
         * reduced pointer. */
        ((volatile POLY_GT4 *)g)->code = 0x3C;
        pkt = (POLY_FT4 *)(g + 1);
    }
    return (PACKET *)pkt;
}

PACKET *SortTmdWireGT4(TmdGT4 *prim, SVECTOR *vtx, SVECTOR *nrm, PACKET *packet, s32 n, s32 shift,
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
        if (v & GTE_FLAG_REJECT) {
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
        if (v & GTE_FLAG_REJECT) {
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
        linkPrim(tag, pkt, LINE_G3);
        pkt++;
        tag = (u32 *)ot->org + (v >> shift);
        linkPrim(tag, pkt, LINE_G3);
        pkt++;
    }
    return (PACKET *)pkt;
}

void InitTmdSortFuncs(void) {
    gTmdSortFuncs[0][6] = SortTmdFastF3;
    gTmdSortFuncs[0][7] = SortTmdDivF3;
    gTmdSortFuncs[1][6] = SortTmdFastG3;
    gTmdSortFuncs[1][7] = SortTmdDivG3;
    gTmdSortFuncs[2][6] = SortTmdFastFT3;
    gTmdSortFuncs[2][7] = SortTmdDivFT3;
    gTmdSortFuncs[3][0] = SortTmdWireGT3;
    gTmdSortFuncs[3][6] = SortTmdFastGT3;
    gTmdSortFuncs[3][7] = SortTmdDivGT3;
    gTmdSortFuncs[4][6] = SortTmdFastF4;
    gTmdSortFuncs[4][7] = SortTmdDivF4;
    gTmdSortFuncs[5][6] = SortTmdFastG4;
    gTmdSortFuncs[5][7] = SortTmdDivG4;
    gTmdSortFuncs[6][6] = SortTmdFastFT4;
    gTmdSortFuncs[6][7] = SortTmdDivFT4;
    gTmdSortFuncs[7][0] = SortTmdWireGT4;
    gTmdSortFuncs[7][6] = SortTmdFastGT4;
    gTmdSortFuncs[7][7] = SortTmdDivGT4;
}

s32 SetTmdSortMode(s32 mode) {
    switch (mode) {
        case 0:
        case 1:
            gTmdSortFuncs[3][0] = SortTmdLitEnvMapGT3;
            gTmdSortFuncs[7][0] = SortTmdLitEnvMapGT4;
            break;
        case 2:
            gTmdSortFuncs[3][0] = SortTmdWireGT3;
            gTmdSortFuncs[7][0] = SortTmdWireGT4;
            break;
        case 3:
            gTmdSortFuncs[3][0] = SortTmdEnvMapGT3;
            gTmdSortFuncs[7][0] = SortTmdEnvMapGT4;
            break;
        case 4:
            gTmdSortFuncs[3][0] = SortTmdLitGT3;
            gTmdSortFuncs[7][0] = SortTmdLitGT4;
            break;
        case 5:
            gTmdSortFuncs[3][0] = SortTmdLitSemiGT3;
            gTmdSortFuncs[7][0] = SortTmdLitSemiGT4;
            break;
    }
}

PACKET *SortTmdFastF3(TmdF3 *prim, SVECTOR *vtx, POLY_F3 *pkt, s32 n, s32 shift, GsOT *ot) {
    s32 v;
    s32 dp;
    s32 i;
    u32 *tag;

    for (i = 0; i < n; i++, prim++) {
        gte_ldv3(&vtx[prim->v0], &vtx[prim->v1], &vtx[prim->v2]);
        gte_rtpt();
        gte_stflg(&v);
        if (v & GTE_FLAG_REJECT) {
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
        linkPrim(tag, pkt, POLY_F3);
        pkt++;
    }
    return (PACKET *)pkt;
}

PACKET *SortTmdFastF4(TmdF4 *prim, SVECTOR *vtx, POLY_F4 *pkt, s32 n, s32 shift, GsOT *ot) {
    s32 v;
    s32 dp;
    s32 i;
    u32 *tag;

    for (i = 0; i < n; i++, prim++) {
        gte_ldv3(&vtx[prim->v0], &vtx[prim->v1], &vtx[prim->v2]);
        gte_rtpt();
        gte_stflg(&v);
        if (v & GTE_FLAG_REJECT) {
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
        if (v & GTE_FLAG_REJECT) {
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
        linkPrim(tag, pkt, POLY_F4);
        pkt++;
    }
    return (PACKET *)pkt;
}

PACKET *SortTmdFastG3(TmdG3 *prim, SVECTOR *vtx, PACKET *packet, s32 n, s32 shift, GsOT *ot) {
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
        if (v & GTE_FLAG_REJECT) {
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
        linkPrim(tag, pkt, POLY_G3);
        pkt++;
    }
    return (PACKET *)pkt;
}

PACKET *SortTmdFastG4(TmdG4 *prim, SVECTOR *vtx, PACKET *packet, s32 n, s32 shift, GsOT *ot) {
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
        if (v & GTE_FLAG_REJECT) {
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
        if (v & GTE_FLAG_REJECT) {
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
        linkPrim(tag, pkt, POLY_G4);
        pkt++;
    }
    return (PACKET *)pkt;
}

PACKET *SortTmdFastFT3(TmdFT3 *prim, SVECTOR *vtx, PACKET *packet, s32 n, s32 shift, GsOT *ot) {
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
        if (v & GTE_FLAG_REJECT) {
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
        ((volatile POLY_FT3 *)pkt)->code = prim->mode & ~PRIM_RAW;
        *(u32 *)&pkt->u0 = *(u32 *)&prim->uv0;
        *(u32 *)&pkt->u1 = *(u32 *)&prim->uv1;
        *(u16 *)&pkt->u2 = *(u16 *)&prim->uv2;
        tag = (u32 *)ot->org + (v >> shift);
        linkPrim(tag, pkt, POLY_FT3);
        pkt++;
    }
    return (PACKET *)pkt;
}
