#include "common.h"
#include "libgte.h"
#include "libgpu.h"
#include "libgs.h"
#include "gte.h"

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

INCLUDE_ASM("asm/nonmatchings/code_11dc4", func_800215C4);

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

INCLUDE_ASM("asm/nonmatchings/code_11dc4", func_80021958);

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

INCLUDE_ASM("asm/nonmatchings/code_11dc4", func_80021D3C);

INCLUDE_ASM("asm/nonmatchings/code_11dc4", func_80021F80);

INCLUDE_ASM("asm/nonmatchings/code_11dc4", func_80022150);

INCLUDE_ASM("asm/nonmatchings/code_11dc4", func_8002230C);

INCLUDE_ASM("asm/nonmatchings/code_11dc4", func_80022554);
