# ResetStageObjects — MATCHED 23/23 (round 4, alpha)

> Renamed from `func_8002C47C` on 2026-10-11 (tools/rename.py). Address 0x8002c47c.

Unit `src/object.c`. Sets byte 0 of every one of the 200 `Rec5C`
records of `D_800CF080` that is not -1 to 0, then calls `RefreshAllZones` and
`ClearObjectHits`. First build matched.

```c
void ResetStageObjects(void) {
    u32 i;

    for (i = 0; i < 200; i++) {
        if (D_800CF080[i].unk0 != -1) {
            D_800CF080[i].unk0 = 0;
        }
    }
    RefreshAllZones();
    ClearObjectHits();
}
```

`D_800CF080` is reached `lui`/`addiu` into a walking pointer: declared in the
unit as `extern Rec5C D_800CF080[];`. `lb` on byte 0 matches `Rec5C.unk0`
being `s8`. `u32 i` gives the `sltiu` count-up (LEARNINGS, Loops).
