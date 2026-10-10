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

/** @brief A flat-textured unlit TMD triangle: a header, three UV words
 *  (the CLUT, the texture page, padding in their top halves), a colour and
 *  three vertex indices. */
typedef struct {
    u8 olen;     /**< the TMD header: output length */
    u8 ilen;     /**< input length */
    u8 flag;     /**< flags */
    u8 mode;     /**< the primitive code, copied into the packet */
    u32 uv0;     /**< u0, v0 and the CLUT, copied whole */
    u32 uv1;     /**< u1, v1 and the texture page, copied whole */
    u16 uv2;     /**< u2, v2 */
    u16 pad0;    /**< padding */
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

/** @brief A primitive handler: one entry of the handler table. */
typedef PACKET *(*PrimFunc)();

/* The handler table, eight rows of eight handlers. */
extern PrimFunc D_800E48E8[8][8];

PACKET *func_8001B4BC();
PACKET *func_8001B9A4();
PACKET *func_8001C13C();
PACKET *func_8001C878();
PACKET *func_8001D39C();
PACKET *func_8001DAD4();
PACKET *func_8001E558();
PACKET *func_8001EE30();
PACKET *func_8001FBBC();
PACKET *func_8001FE5C();
PACKET *func_800201BC();
PACKET *func_80020520();
PACKET *func_8002097C();
PACKET *func_80020DD8(TmdF3 *prim, SVECTOR *vtx, POLY_F3 *pkt, s32 n, s32 shift, GsOT *ot);
PACKET *func_80020F24(TmdF4 *prim, SVECTOR *vtx, POLY_F4 *pkt, s32 n, s32 shift, GsOT *ot);
PACKET *func_800210B4(TmdG3 *prim, SVECTOR *vtx, POLY_G3 *pkt, s32 n, s32 shift, GsOT *ot);
PACKET *func_80021240();
PACKET *func_80021434(TmdFT3 *prim, SVECTOR *vtx, POLY_FT3 *pkt, s32 n, s32 shift, GsOT *ot);

/* Handlers code_11dc4 defines. */
PACKET *func_800215C4();
PACKET *func_800217A8();
PACKET *func_80021958();
PACKET *func_80021B88();
PACKET *func_80021D3C();
PACKET *func_80021F80();
PACKET *func_80022150();
PACKET *func_8002230C();

INCLUDE_ASM("asm/nonmatchings/code_a0bc", func_800198BC);

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

INCLUDE_ASM("asm/nonmatchings/code_a0bc", func_80019F24);

INCLUDE_ASM("asm/nonmatchings/code_a0bc", func_8001A3D4);

INCLUDE_ASM("asm/nonmatchings/code_a0bc", func_8001A69C);

INCLUDE_ASM("asm/nonmatchings/code_a0bc", func_8001A950);

INCLUDE_ASM("asm/nonmatchings/code_a0bc", func_8001ACB4);

INCLUDE_ASM("asm/nonmatchings/code_a0bc", func_8001B004);

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

INCLUDE_ASM("asm/nonmatchings/code_a0bc", func_8001B4BC);

INCLUDE_ASM("asm/nonmatchings/code_a0bc", func_8001B9A4);

INCLUDE_ASM("asm/nonmatchings/code_a0bc", func_8001C13C);

INCLUDE_ASM("asm/nonmatchings/code_a0bc", func_8001C878);

INCLUDE_ASM("asm/nonmatchings/code_a0bc", func_8001D39C);

INCLUDE_ASM("asm/nonmatchings/code_a0bc", func_8001DAD4);

INCLUDE_ASM("asm/nonmatchings/code_a0bc", func_8001E558);

INCLUDE_ASM("asm/nonmatchings/code_a0bc", func_8001EE30);

INCLUDE_ASM("asm/nonmatchings/code_a0bc", func_8001FBBC);

INCLUDE_ASM("asm/nonmatchings/code_a0bc", func_8001FE5C);

INCLUDE_ASM("asm/nonmatchings/code_a0bc", func_800201BC);

INCLUDE_ASM("asm/nonmatchings/code_a0bc", func_80020520);

INCLUDE_ASM("asm/nonmatchings/code_a0bc", func_8002097C);

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

#ifdef NON_MATCHING
PACKET *func_800210B4(TmdG3 *prim, SVECTOR *vtx, POLY_G3 *pkt, s32 n, s32 shift, GsOT *ot) {
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
        gte_stsxy3_g3(pkt);
        gte_avsz3();
        gte_stotz(&v);
        gte_stdp(&dp);
        gte_ldrgb3(&prim->c0, &prim->c1, &prim->c2);
        gte_lddp(dp);
        gte_dpct();
        gte_strgb3(&pkt->r0, &pkt->r1, &pkt->r2);
        pkt->code = prim->mode;
        tag = (u32 *)ot->org + (v >> shift);
        *(u32 *)pkt = (*tag & 0xFFFFFF) | 0x06000000;
        *tag = (u32)pkt & 0xFFFFFF;
        pkt++;
    }
    return (PACKET *)pkt;
}
#else
INCLUDE_ASM("asm/nonmatchings/code_a0bc", func_800210B4);
#endif

INCLUDE_ASM("asm/nonmatchings/code_a0bc", func_80021240);

#ifdef NON_MATCHING
PACKET *func_80021434(TmdFT3 *prim, SVECTOR *vtx, POLY_FT3 *pkt, s32 n, s32 shift, GsOT *ot) {
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
        gte_stsxy3_ft3(pkt);
        gte_avsz3();
        gte_stotz(&v);
        gte_stdp(&dp);
        gte_ldrgb(&prim->rgb);
        gte_lddp(dp);
        gte_dpcs();
        gte_strgb(&pkt->r0);
        pkt->code = prim->mode & 0xFE;
        *(u32 *)&pkt->u0 = prim->uv0;
        *(u32 *)&pkt->u1 = prim->uv1;
        *(u16 *)&pkt->u2 = prim->uv2;
        tag = (u32 *)ot->org + (v >> shift);
        *(u32 *)pkt = (*tag & 0xFFFFFF) | 0x07000000;
        *tag = (u32)pkt & 0xFFFFFF;
        pkt++;
    }
    return (PACKET *)pkt;
}
#else
INCLUDE_ASM("asm/nonmatchings/code_a0bc", func_80021434);
#endif
