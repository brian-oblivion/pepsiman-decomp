# func_8001FBBC — MATCHED (168/168), round 7, runner charlie

Unit `src/code_a0bc.c`. The environment-map handler (row 3, column 0 of the
handler table in one of func_80020C14's modes): each lit gouraud-textured
TMD triangle becomes a raw-textured POLY_FT3 (code 0x25) whose UVs come from
the rotated vertex normals (`rtv0`, MAC1/MAC2 >> 8) offset by the screen
position, on texture page 0x161 plus a 256-line offset picked by the
display buffer (`D_800E474C`, libgs's PSDIDX, XOR 1). Linked with length 7.
Needed `gte_rtv0` and `gte_stlvnl` in `include/gte.h` (added).

```c
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
```

## Levers

- **`volatile POLY_FT3 *pkt`.** The first body (13/168) had loop reduce a
  `pkt+0x1D` giv and rebase every packet field onto it, and hoisted the y
  reads above the u stores. Retail keeps every field on the biv and reads
  and writes them in source order; a volatile packet pointer does both
  (loop cannot rebase volatile references; the scheduler cannot reorder
  them). 162/168.
- **The v coordinate built in one local through compound statements**,
  `y = (pkt->y0 >> 3) + 0x78; y += mac.vy >> 8; pkt->v0 = y - page;`.
  As one expression cc1 moves the 0x78 onto the normal term; as
  `y + (mac.vy >> 8) - page` the registers swap. The func_80018D04 shape.

Twelve builds.

### Proposed learning

- **A packet filled field by field with every access on the incoming
  pointer, reads interleaved with writes in source order, and no reduced
  loop pointer: the packet pointer is `volatile`.** (func_8001FBBC; also
  func_80019CD8 outside a loop)
