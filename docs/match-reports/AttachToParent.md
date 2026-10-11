# AttachToParent — MATCHED 63/63 (round 8, alpha)

> Renamed from `func_8002C724` on 2026-10-11 (tools/rename.py). Address 0x8002c724.

Unit `src/object.c`. Attaches Rec48 `b` to Rec48 `a`: transforms
`b`'s stored position (its words at 0xC..0x14, as an SVECTOR) by the
coordinate system of `a`'s Rec78 entry into `b`'s current position, copies
`a`'s two angles, adds `a`'s heading halfword to `b`'s stored one, and
updates `b`'s Rec78 entry (DrawObjectModel). Matched on the first build.

```c
void AttachToParent(Rec48 *a, Rec48 *b) {
    Obj34 *pa;
    Obj34 *pb;

    pa = (Obj34 *)a;
    pb = (Obj34 *)b;
    D_800957E4.vx = pb->unkC;
    D_800957E4.vy = pb->unk10;
    D_800957E4.vz = pb->unk14;
    TransformByCoord((GsCOORDINATE2 *)D_800D8D20[a->unk36].unk10, &D_800957E4, &D_8009F268);
    pb->unk0 = D_8009F268.vx;
    pb->unk4 = D_8009F268.vy;
    pb->unk8 = D_8009F268.vz;
    pb->unk2C = pa->unk2C;
    pb->unk30 = pa->unk30;
    pb->unk1A = pa->unk1A + pb->unk20;
    DrawObjectModel(&D_800D8D20[b->unk36], b);
}
```

## Declarations added (unit-local, in the .c)

- `extern SVECTOR D_800957E4;`: retail takes its address `lui`/`addiu`
  (cc1's `la` of an object of at most 8 bytes) and stores the three
  halfwords through `lui $at`. splat names the halfwords D_800957E6 and
  D_800957E8; `%lo(D_800957E4+2)` assembles to the same bytes.
- `extern VECTOR D_8009F268;`: read `lui` + `%lo` and then off the base.
- Prototypes for TransformByCoord (player) and DrawObjectModel (this unit,
  still asm; `(Rec78 *, Rec48 *)` like UpdateObjectEntry).

## Observation

Rec48's first 0x34 bytes have Obj34's layout (current and stored position,
halfword triples at 0x18 and 0x1E, angles at 0x2C/0x30). The shared Rec48
keeps them opaque, so the unit reads them through an `Obj34 *` view rather
than editing the shared header.
