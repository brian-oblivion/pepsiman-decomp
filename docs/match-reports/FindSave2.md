# FindSave2 — MATCHED 38/38 (round 5, echo)

> Renamed from `func_80039580` on 2026-10-10 (tools/rename.py). Address 0x80039580.

Unit `src/card.c`. `FindSave1` with the other name: points
`gCardFileName` at `"BISLPS-67890PEPTOOL"` (rodata `D_80011C40`, migrated into
this function's `.s`, so now a literal) and looks for it among the first
`gCardFileCount` records of `gCardFiles`. Returns 0 if found, -1 if not.

```c
s16 FindSave2(void) {
    s16 i;

    gCardFileName = "BISLPS-67890PEPTOOL";
    for (i = 0; i < gCardFileCount; i++) {
        if (CompareBytes((u8 *)gCardFiles[i].name, (u8 *)gCardFileName, 0x14) == 0) {
            return 0;
        }
    }
    return -1;
}
```

Matched on the first build once `CompareBytes` returned `s32` (see
`FindSave1.md`). The literal lands after `INCLUDE_RODATA(sHitCardFile)`,
where retail has it.

`Rec18` became the file name plus a `BlockCount` union in round 5 (see
`ScanCardFiles.md`); the code above uses the new member names.
