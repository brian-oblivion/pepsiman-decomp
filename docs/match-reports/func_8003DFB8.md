# func_8003DFB8 — MATCHED 49/49 (round 5, bravo)

Unit `src/code_29f54.c`. The twin of func_8003E07C (same report's notes
apply) for a second model table: count in `D_80095794`, objects in the
`GsDOBJ2` table `D_800AC868`, same coordinate system `D_800A72B8`, and
attribute 0x200 instead of 0.

```c
extern u32 D_80095794;
extern GsDOBJ2 D_800AC868[];
extern GsCOORDINATE2 D_800A72B8;

void func_8003DFB8(unsigned long *tmd) {
    u32 i;
    GsDOBJ2 *obj;

    tmd++;
    GsMapModelingData(tmd);
    tmd++;
    D_80095794 = *tmd;
    tmd++;
    for (i = 0; i < D_80095794; i++) {
        GsLinkObject4((unsigned long)tmd, &D_800AC868[i], i);
    }
    obj = D_800AC868;
    for (i = 0; i < D_80095794; i++) {
        obj->coord2 = &D_800A72B8;
        obj->attribute = 0x200;
        obj++;
    }
}
```

First try, from func_8003E07C's body. Declarations as there: the count is
reached `lui` (a plain `u32`, `sltu` tests), the table's address over two
registers (`[]`). Only this unit references them.
