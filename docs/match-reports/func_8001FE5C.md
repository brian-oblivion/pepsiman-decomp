# func_8001FE5C — MATCHED (216/216), round 8, runner delta

Unit `src/code_a0bc.c`. The quad twin of func_8001FBBC, installed by
func_80020CF8's mode 3 as the TMD handler for lit gouraud-textured quads:
projects each TmdGT4, culls on the GTE flags and on back faces, and writes a
POLY_FT4 (code 0x2D, texture page 0x161 plus the page flip of `D_800E474C`)
whose UVs come from each corner's normal rotated by the current matrix
(environment-map style: `u = (nx >> 8) - (x/8 - 0x60)`, `v = y/8 + 0x78 +
(ny >> 8) - page`). The local prototype changed from `PACKET
*func_8001FE5C();` to the full one (unit-local, nothing else declares it).

```c
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
```

## How it went

func_8001FBBC's body with a fourth vertex and a fourth UV corner:

1. As FBBC (a `volatile POLY_FT3 *` local copy): equal length but loop added
   a reduced pointer `pkt + 0x25` beside the packet and a saved `$s0`, and
   `v` sat at sp+0x10.
2. The packet as the parameter itself, `volatile POLY_FT4 *pkt`: retail uses
   `$a3` for every packet access, with no reduced pointer.
3. `s32 unused[2];` after `VECTOR mac;` puts `v` at sp+0x18 as retail; it
   was only visible once the saved register was gone (with the saved `$s0`
   it grew the frame). 216/216.

`config/typeviews-warnings.txt` gains `code_a0bc: warning: unused variable
`unused'`.

### Proposed learning

- A packet pointer retail uses straight from its argument register, with
  fixed offsets and no reduced pointer: a `volatile` packet PARAMETER. FBBC's
  volatile local copy is the shape when retail copies it to another register.
