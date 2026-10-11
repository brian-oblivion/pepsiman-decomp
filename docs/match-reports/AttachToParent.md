# AttachToParent — MATCHED 63/63 (round 8, alpha)

> Renamed from `func_8002C724` on 2026-10-11 (tools/rename.py). Address 0x8002c724.

Unit `src/object.c`. Attaches Trap `b` to Trap `a`: transforms
`b`'s stored position (its words at 0xC..0x14, as an SVECTOR) by the
coordinate system of `a`'s ObjectModel entry into `b`'s current position, copies
`a`'s two angles, adds `a`'s heading halfword to `b`'s stored one, and
updates `b`'s ObjectModel entry (DrawObjectModel). Matched on the first build.

```c
void AttachToParent(Trap *a, Trap *b) {
    Obj34 *pa;
    Obj34 *pb;

    pa = (Obj34 *)a;
    pb = (Obj34 *)b;
    gLocalPos.vx = pb->unkC;
    gLocalPos.vy = pb->unk10;
    gLocalPos.vz = pb->unk14;
    TransformByCoord((GsCOORDINATE2 *)gObjectModels[a->unk36].unk10, &gLocalPos, &gWorldPos);
    pb->unk0 = gWorldPos.vx;
    pb->unk4 = gWorldPos.vy;
    pb->unk8 = gWorldPos.vz;
    pb->unk2C = pa->unk2C;
    pb->unk30 = pa->unk30;
    pb->unk1A = pa->unk1A + pb->unk20;
    DrawObjectModel(&gObjectModels[b->unk36], b);
}
```

## Declarations added (unit-local, in the .c)

- `extern SVECTOR gLocalPos;`: retail takes its address `lui`/`addiu`
  (cc1's `la` of an object of at most 8 bytes) and stores the three
  halfwords through `lui $at`. splat names the halfwords D_800957E6 and
  D_800957E8; `%lo(gLocalPos+2)` assembles to the same bytes.
- `extern VECTOR gWorldPos;`: read `lui` + `%lo` and then off the base.
- Prototypes for TransformByCoord (player) and DrawObjectModel (this unit,
  still asm; `(ObjectModel *, Trap *)` like UpdateObjectEntry).

## Observation

Trap's first 0x34 bytes have Obj34's layout (current and stored position,
halfword triples at 0x18 and 0x1E, angles at 0x2C/0x30). The shared Trap
keeps them opaque, so the unit reads them through an `Obj34 *` view rather
than editing the shared header.
