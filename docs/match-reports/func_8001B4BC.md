# func_8001B4BC — MATCHED (314/314), round 10, runner charlie

Unit `src/code_a0bc.c`. The TMD handler for flat unlit triangles in
column 7 of the handler table (func_80020C14, row 0): the near-depth twin
of func_80020DD8. Per front-facing triangle it takes the deepest of the
three screen Zs over 4 as the depth; unless the transform overflowed it
links a depth-cued POLY_F3 one OT slot further in; and when the depth is
under 250 it also splits the triangle at its edge midpoints into four
and draws each (undepth-cued, the TMD colour copied as a word) at its own
`avsz3` depth.

New: `gte_stsz3c(r1)` in `include/gte.h` (additive; Sony's name, SZ1-SZ3
stored at +0, +4, +8 from one base). The local prototype changed from
`PACKET *func_8001B4BC();` to the full one.

```c
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
```

## How it went

1. Read the asm: the flag stored before `nclip` and tested only later
   (`bltz`), one `swc2 $17/$18/$19` run off one base (`gte_stsz3c`), a
   three-way max, the midpoint block in `sp+0..0x17`, and a four-pass loop
   whose `beq 1 / slti 2 / beqz / beq 2 / beq 3` compare tree is a
   `switch (j)` over cases 0-3 choosing the sub-triangle's vertices.
2. First body: 312/314, two loads swapped in the max: `v < sz[2]` loads
   `v` first.
3. `sz[2] > v`: 314/314.

### Proposed learning

- Three consecutive `swc2 $17..$19` at +0/+4/+8 from one register:
  `gte_stsz3c(arr)` (now in include/gte.h).
