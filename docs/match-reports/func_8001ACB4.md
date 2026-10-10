# func_8001ACB4 — MATCHED (212/212), round 8, runner delta

Unit `src/code_a0bc.c`. The rotating twin of func_8001A69C: copies the
billboard matrix `D_800E4858`, rotates the copy by `angle` about Z
(`RotMatrixZ`), loads it, and draws sprite `id` of `D_800DD0A0` as a quad
standing on y = 0 into a POLY_FT4 in `D_800E48D0`. The angle is an `s16`
parameter (retail narrows `$a3` with `sll`/`sra` before the call; an `s32`
with `(s16)angle` at the call matches too). No callers in C yet.

```c
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
```

## How it went

func_8001A69C's body with the matrix copy and the call in front matched
the whole tail at once. The only miss (174/212, then 185/212) was the prologue:
the frame address `&m` (`addiu $s0, $sp, 0x30`) was scheduled to the top of
the function and got `$s1`, swapping it with `size`.

- A `MATRIX *mp = &m;` local fixed the register (185/212) but not the
  position.
- Writing `e = &D_800DD0A0[id];` BEFORE `m = D_800E4858;` (retail's
  instruction order shows the copy first) put the address after the entry's
  `addu` and matched; the pointer local is then unnecessary.

### Proposed learning

- A frame address (`addiu $sN, $sp, K`) that sched2 hoists to the prologue
  while retail builds it late, right before the call, with saved registers
  swapped as a result: move an unrelated statement in front of the struct
  copy that precedes it. The order of a struct copy and its neighbours in
  the source sets where the address lands, even when retail's own order of
  the copy differs.
