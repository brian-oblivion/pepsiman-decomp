#include "common.h"
#include "libgte.h"
#include "libgpu.h"
#include "libgs.h"
#include "gte.h"

/** @brief A flat-textured unlit TMD quad: a header, four UV words (the
 *  CLUT and the texture page in the first two's top halves), a colour and
 *  four vertex indices. */
typedef struct {
    u8 olen;     /**< the TMD header: output length */
    u8 ilen;     /**< input length */
    u8 flag;     /**< flags */
    u8 mode;     /**< the primitive code, copied into the packet */
    u32 uv0;     /**< u0, v0 and the CLUT, copied whole */
    u32 uv1;     /**< u1, v1 and the texture page, copied whole */
    u16 uv2;     /**< u2, v2 */
    u16 pad0;    /**< padding */
    u16 uv3;     /**< u3, v3 */
    u16 pad1;    /**< padding */
    CVECTOR rgb; /**< the colour, depth-cued per frame */
    u16 v0;      /**< first vertex index */
    u16 v1;      /**< second vertex index */
    u16 v2;      /**< third vertex index */
    u16 v3;      /**< fourth vertex index */
} TmdFT4;

/** @brief A gouraud-textured unlit TMD triangle: a header, three UV words,
 *  three colours and three vertex indices. */
typedef struct {
    u8 olen;    /**< the TMD header: output length */
    u8 ilen;    /**< input length */
    u8 flag;    /**< flags */
    u8 mode;    /**< the primitive code, copied into the packet */
    u32 uv0;    /**< u0, v0 and the CLUT, copied whole */
    u32 uv1;    /**< u1, v1 and the texture page, copied whole */
    u16 uv2;    /**< u2, v2 */
    u16 pad0;   /**< padding */
    CVECTOR c0; /**< first vertex colour */
    CVECTOR c1; /**< second vertex colour */
    CVECTOR c2; /**< third vertex colour */
    u16 v0;     /**< first vertex index */
    u16 v1;     /**< second vertex index */
    u16 v2;     /**< third vertex index */
    u16 pad1;   /**< padding */
} TmdGT3U;

/** @brief A gouraud-textured unlit TMD quad: a header, four UV words, four
 *  colours and four vertex indices. */
typedef struct {
    u8 olen;    /**< the TMD header: output length */
    u8 ilen;    /**< input length */
    u8 flag;    /**< flags */
    u8 mode;    /**< the primitive code, copied into the packet */
    u32 uv0;    /**< u0, v0 and the CLUT, copied whole */
    u32 uv1;    /**< u1, v1 and the texture page, copied whole */
    u16 uv2;    /**< u2, v2 */
    u16 pad0;   /**< padding */
    u16 uv3;    /**< u3, v3 */
    u16 pad1;   /**< padding */
    CVECTOR c0; /**< first vertex colour */
    CVECTOR c1; /**< second vertex colour */
    CVECTOR c2; /**< third vertex colour */
    CVECTOR c3; /**< fourth vertex colour */
    u16 v0;     /**< first vertex index */
    u16 v1;     /**< second vertex index */
    u16 v2;     /**< third vertex index */
    u16 v3;     /**< fourth vertex index */
} TmdGT4U;

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

PACKET *func_800215C4(TmdFT4 *prim, SVECTOR *vtx, PACKET *packet, s32 n, s32 shift, GsOT *ot) {
    POLY_FT4 *pkt;
    s32 v;
    s32 dp;
    s32 i;
    u32 *tag;

    /* MATCHING: the packet pointer is a copy of the parameter, so the
     * loop starts its reduced pointer from it, not from $a2. */
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
        gte_stdp(&dp);
        gte_ldrgb(&prim->rgb);
        gte_lddp(dp);
        gte_dpcs();
        gte_strgb(&pkt->r0);
        /* MATCHING: a volatile store, which loop cannot rebase onto its
         * reduced pointer. */
        ((volatile POLY_FT4 *)pkt)->code = prim->mode & 0xFE;
        *(u32 *)&pkt->u0 = prim->uv0;
        *(u32 *)&pkt->u1 = prim->uv1;
        *(u16 *)&pkt->u2 = prim->uv2;
        *(u16 *)&pkt->u3 = prim->uv3;
        tag = (u32 *)ot->org + (v >> shift);
        *(u32 *)pkt = (*tag & 0xFFFFFF) | 0x09000000;
        *tag = (u32)pkt & 0xFFFFFF;
        pkt++;
    }
    return (PACKET *)pkt;
}

PACKET *func_800217A8(TmdGT3U *prim, SVECTOR *vtx, PACKET *packet, s32 n, s32 shift, GsOT *ot) {
    POLY_GT3 *pkt;
    s32 v;
    s32 dp;
    s32 i;
    u32 *tag;

    /* MATCHING: the packet pointer is a copy of the parameter, so the
     * loop starts its reduced pointer from it, not from $a2. */
    pkt = (POLY_GT3 *)packet;

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
        gte_stsxy3_gt3(pkt);
        gte_avsz3();
        gte_stotz(&v);
        gte_stdp(&dp);
        gte_ldrgb3(&prim->c0, &prim->c1, &prim->c2);
        gte_lddp(dp);
        gte_dpct();
        gte_strgb3(&pkt->r0, &pkt->r1, &pkt->r2);
        /* MATCHING: a volatile store, which loop cannot rebase onto its
         * reduced pointer. */
        ((volatile POLY_GT3 *)pkt)->code = prim->mode & 0xFE;
        *(u32 *)&pkt->u0 = prim->uv0;
        *(u32 *)&pkt->u1 = prim->uv1;
        *(u16 *)&pkt->u2 = prim->uv2;
        tag = (u32 *)ot->org + (v >> shift);
        *(u32 *)pkt = (*tag & 0xFFFFFF) | 0x09000000;
        *tag = (u32)pkt & 0xFFFFFF;
        pkt++;
    }
    return (PACKET *)pkt;
}

PACKET *func_80021958(TmdGT4U *prim, SVECTOR *vtx, PACKET *packet, s32 n, s32 shift, GsOT *ot) {
    POLY_GT4 *pkt;
    s32 v;
    s32 dp;
    s32 i;
    u32 *tag;

    /* MATCHING: the packet pointer is a copy of the parameter, so the
     * loop starts its reduced pointer from it, not from $a2. */
    pkt = (POLY_GT4 *)packet;

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
        gte_stsxy3_gt4(pkt);
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
         * reduced pointer. */
        ((volatile POLY_GT4 *)pkt)->code = prim->mode & 0xFE;
        *(u32 *)&pkt->u0 = prim->uv0;
        *(u32 *)&pkt->u1 = prim->uv1;
        *(u16 *)&pkt->u2 = prim->uv2;
        *(u16 *)&pkt->u3 = prim->uv3;
        tag = (u32 *)ot->org + (v >> shift);
        *(u32 *)pkt = (*tag & 0xFFFFFF) | 0x0C000000;
        *tag = (u32)pkt & 0xFFFFFF;
        pkt++;
    }
    return (PACKET *)pkt;
}

PACKET *func_80021B88(TmdGT3 *prim, SVECTOR *vtx, SVECTOR *nrm, PACKET *packet, s32 n, s32 shift,
                      GsOT *ot) {
    POLY_GT3 *pkt;
    CVECTOR c;
    s32 v;
    s32 i;
    u32 *tag;

    /* MATCHING: the packet pointer is a copy of the parameter, so the
     * loop starts its reduced pointer from it, not from $a3. */
    pkt = (POLY_GT3 *)packet;

    for (i = 0; i < n; i++, prim++) {
        gte_ldv3(&vtx[prim->v0], &vtx[prim->v1], &vtx[prim->v2]);
        gte_rtpt();
        gte_stflg(&v);
        if (v < 0) {
            continue;
        }
        gte_nclip();
        gte_stopz(&v);
        if (v >= 0) {
            continue;
        }
        gte_avsz3();
        gte_stotz(&v);
        gte_stsxy3_gt3(pkt);
        c.r = c.g = c.b = 0x80;
        gte_ldrgb(&c);
        gte_ldv3(&nrm[prim->n0], &nrm[prim->n1], &nrm[prim->n2]);
        gte_ncct();
        *(u32 *)&pkt->u0 = prim->uv0;
        *(u32 *)&pkt->u1 = prim->uv1;
        *(u16 *)&pkt->u2 = prim->uv2;
        tag = (u32 *)ot->org + (v >> shift);
        *(u32 *)pkt = (*tag & 0xFFFFFF) | 0x09000000;
        *tag = (u32)pkt & 0xFFFFFF;
        gte_strgb3_gt3(pkt);
        /* MATCHING: a volatile store, which loop cannot rebase onto its
         * reduced pointer. */
        ((volatile POLY_GT3 *)pkt)->code = 0x36;
        pkt++;
    }
    return (PACKET *)pkt;
}

PACKET *func_80021D3C(TmdGT4 *prim, SVECTOR *vtx, SVECTOR *nrm, PACKET *packet, s32 n, s32 shift,
                      GsOT *ot) {
    POLY_GT4 *pkt;
    CVECTOR c;
    s32 v;
    s32 i;
    u32 *tag;

    /* MATCHING: the packet pointer is a copy of the parameter, so the
     * loop starts its reduced pointer from it, not from $a3. */
    pkt = (POLY_GT4 *)packet;

    for (i = 0; i < n; i++, prim++) {
        gte_ldv3(&vtx[prim->v0], &vtx[prim->v1], &vtx[prim->v2]);
        gte_rtpt();
        gte_stflg(&v);
        if (v & 0x80000000) {
            continue;
        }
        gte_nclip();
        gte_stopz(&v);
        if (v >= 0) {
            continue;
        }
        gte_stsxy3_gt4(pkt);
        gte_ldv0(&vtx[prim->v3]);
        gte_rtps();
        gte_stflg(&v);
        if (v & 0x80000000) {
            continue;
        }
        gte_stsxy2(&pkt->x3);
        gte_avsz4();
        gte_stotz(&v);
        c.r = c.g = c.b = 0x80;
        gte_ldrgb(&c);
        gte_ldv3(&nrm[prim->n0], &nrm[prim->n1], &nrm[prim->n2]);
        gte_ncct();
        *(u32 *)&pkt->u0 = prim->uv0;
        *(u32 *)&pkt->u1 = prim->uv1;
        *(u16 *)&pkt->u2 = prim->uv2;
        *(u16 *)&pkt->u3 = prim->uv3;
        gte_strgb3_gt3(pkt);
        gte_ldv0(&nrm[prim->n3]);
        gte_nccs();
        tag = (u32 *)ot->org + (v >> shift);
        *(u32 *)pkt = (*tag & 0xFFFFFF) | 0x0C000000;
        *tag = (u32)pkt & 0xFFFFFF;
        gte_strgb(&pkt->r3);
        /* MATCHING: a volatile store, which loop cannot rebase onto its
         * reduced pointer. */
        ((volatile POLY_GT4 *)pkt)->code = 0x3E;
        pkt++;
    }
    return (PACKET *)pkt;
}

PACKET *func_80021F80(TmdGT3 *prim, SVECTOR *vtx, SVECTOR *nrm, PACKET *packet, s32 n, s32 shift,
                      GsOT *ot) {
    LINE_G4 *pkt;
    CVECTOR c;
    s32 v;
    s32 i;
    u32 *tag;

    /* MATCHING: the packet pointer is a copy of the parameter, so the
     * loop starts its reduced pointer from it, not from $a3. */
    pkt = (LINE_G4 *)packet;

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
        gte_stsxy3(&pkt->x0, &pkt->x1, &pkt->x2);
        *(u32 *)&pkt->x3 = *(u32 *)&pkt->x0;
        c.r = 0x80;
        c.g = 0x80;
        c.b = 0x80;
        gte_ldrgb(&c);
        gte_ldv3(&nrm[prim->n0], &nrm[prim->n1], &nrm[prim->n2]);
        gte_ncct();
        gte_strgb3(&pkt->r0, &pkt->r1, &pkt->r2);
        *(u32 *)&pkt->r3 = *(u32 *)&pkt->r0;
        /* MATCHING: volatile stores, which loop cannot rebase onto its
         * reduced pointer. */
        ((volatile LINE_G4 *)pkt)->code = 0x5C;
        ((volatile LINE_G4 *)pkt)->pad = 0x55555555;
        tag = (u32 *)ot->org + (v >> shift);
        *(u32 *)pkt = (*tag & 0xFFFFFF) | 0x09000000;
        *tag = (u32)pkt & 0xFFFFFF;
        pkt++;
    }
    return (PACKET *)pkt;
}

PACKET *func_80022150(TmdGT3 *prim, SVECTOR *vtx, SVECTOR *nrm, PACKET *packet, s32 n, s32 shift,
                      GsOT *ot) {
    POLY_GT3 *pkt;
    CVECTOR c;
    s32 v;
    s32 i;
    u32 *tag;

    /* MATCHING: the packet pointer is a copy of the parameter, so the
     * loop starts its reduced pointer from it, not from $a3. */
    pkt = (POLY_GT3 *)packet;

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
        gte_stsxy3_gt3(pkt);
        c.r = c.g = c.b = 0x80;
        gte_ldrgb(&c);
        gte_ldv3(&nrm[prim->n0], &nrm[prim->n1], &nrm[prim->n2]);
        gte_ncct();
        *(u32 *)&pkt->u0 = prim->uv0;
        *(u32 *)&pkt->u1 = prim->uv1;
        *(u16 *)&pkt->u2 = prim->uv2;
        tag = (u32 *)ot->org + (v >> shift);
        *(u32 *)pkt = (*tag & 0xFFFFFF) | 0x09000000;
        *tag = (u32)pkt & 0xFFFFFF;
        gte_strgb3_gt3(pkt);
        /* MATCHING: a volatile store, which loop cannot rebase onto its
         * reduced pointer. */
        ((volatile POLY_GT3 *)pkt)->code = 0x34;
        pkt++;
    }
    return (PACKET *)pkt;
}

INCLUDE_ASM("asm/nonmatchings/code_11dc4", func_8002230C);

INCLUDE_ASM("asm/nonmatchings/code_11dc4", func_80022554);
