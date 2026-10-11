# DrawBillboard — MATCHED (178/178), round 7, runner charlie

> Renamed from `func_8001A3D4` on 2026-10-10 (tools/rename.py). Address 0x8001a3d4.

Unit `src/draw.c`. The centred twin of DrawStandingBillboard: draws sprite `id`
of `gSpriteDefs` as a billboard quad centred on the current transformation's
origin in both x and y (DrawStandingBillboard stands it on y = 0), rotated by
`GsIDMATRIX`, into a POLY_FT4 in `GsOUT_PACKET_P`, linked at its average depth
`>> shift`. Its prototype in `include/draw.h` was corrected: `u16 id`
(retail masks it, `andi 0xFFFF`) and the fourth argument is the OT shift,
not a mode; the callers in collision, hit_tool and stage build
unchanged (whole image OK).

```c
void DrawBillboard(u16 id, SVECTOR *size, CVECTOR *color, s32 shift, GsOT *ot) {
    SVECTOR sv[4];
    s32 v;
    s32 z;
    Sprite8 *e;
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
    GsOUT_PACKET_P = (u8 *)(p + 1);
}
```

## How it went

DrawStandingBillboard's body with the vertex setup changed matched on the first
build: the three levers carry over unchanged (both flag tests `v &
0x80000000`, the zero vertex stores last, the OT index read before the last
UV store).
