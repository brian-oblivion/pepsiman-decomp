# LinkPropsTmd — MATCHED 48/48 (round 5, bravo)

> Renamed from `func_8003E07C` on 2026-10-10 (tools/rename.py). Address 0x8003e07c.

Unit `src/stage.c`. Sets up a TMD model: skips the TMD ID word, maps
the data (`GsMapModelingData`), stores the object count in `gPropObjCount`,
links every object into the `GsDOBJ2` table `gPropObjs`, then gives each
object the coordinate system `gStageCoord` and attribute 0.

```c
extern u32 gPropObjCount;
extern GsDOBJ2 gPropObjs[];
extern GsCOORDINATE2 gStageCoord;

void LinkPropsTmd(unsigned long *tmd) {
    u32 i;
    GsDOBJ2 *obj;

    tmd++;
    GsMapModelingData(tmd);
    tmd++;
    gPropObjCount = *tmd;
    tmd++;
    for (i = 0; i < gPropObjCount; i++) {
        GsLinkObject4((unsigned long)tmd, &gPropObjs[i], i);
    }
    obj = gPropObjs;
    for (i = 0; i < gPropObjCount; i++) {
        obj->coord2 = &gStageCoord;
        obj->attribute = 0;
        obj++;
    }
}
```

## Declarations

- `gPropObjCount` sits in `.sbss` but every access is `lui $at` / `lui $v0`
  (not in `config/gp-symbols.txt`): a plain `u32` scalar. Unsigned because
  both loops test `sltu`.
- `gPropObjs` and `gStageCoord`: addresses built `lui $v0` / `addiu $sN,
  $v0` (two registers), so an unknown-size array and an 80-byte struct.
  All three are referenced only in this unit (the other readers,
  LoadStageData and friends, are still asm), so they are declared in the
  `.c`.
- The parameter is `unsigned long *`, not `u_long *`: this project's
  `u_long` is `unsigned int`, and libgs.h's `GsMapModelingData` takes
  Sony's `unsigned long *`; `u32 *`/`u_long *` raise a NEW
  incompatible-pointer warning in `tools/typeviews.py --warnings` (same
  code).

## Path (2 builds)

First build 39/48: the second loop written `gPropObjs[i].coord2 = ...`
puts the table's `lui`/`addiu` after the count's load and the coordinate
address before; retail builds the table base first. A walking pointer set
before the loop (`obj = gPropObjs; ... obj++;`, docs/LEARNINGS.md's
"pointer bumped" / "separate struct pointer" entries) matched.
