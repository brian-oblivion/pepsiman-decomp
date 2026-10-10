# LinkStageTmd — MATCHED 49/49 (round 5, bravo)

> Renamed from `func_8003DFB8` on 2026-10-10 (tools/rename.py). Address 0x8003dfb8.

Unit `src/code_29f54.c`. The twin of LinkPropsTmd (same report's notes
apply) for a second model table: count in `gStageObjCount`, objects in the
`GsDOBJ2` table `gStageObjs`, same coordinate system `gStageCoord`, and
attribute 0x200 instead of 0.

```c
extern u32 gStageObjCount;
extern GsDOBJ2 gStageObjs[];
extern GsCOORDINATE2 gStageCoord;

void LinkStageTmd(unsigned long *tmd) {
    u32 i;
    GsDOBJ2 *obj;

    tmd++;
    GsMapModelingData(tmd);
    tmd++;
    gStageObjCount = *tmd;
    tmd++;
    for (i = 0; i < gStageObjCount; i++) {
        GsLinkObject4((unsigned long)tmd, &gStageObjs[i], i);
    }
    obj = gStageObjs;
    for (i = 0; i < gStageObjCount; i++) {
        obj->coord2 = &gStageCoord;
        obj->attribute = 0x200;
        obj++;
    }
}
```

First try, from LinkPropsTmd's body. Declarations as there: the count is
reached `lui` (a plain `u32`, `sltu` tests), the table's address over two
registers (`[]`). Only this unit references them.
