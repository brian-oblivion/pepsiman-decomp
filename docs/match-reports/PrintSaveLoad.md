# PrintSaveLoad — MATCHED 43/43 (round 6, bravo)

> Renamed from `func_80033D3C` on 2026-10-11 (tools/rename.py). Address 0x80033d3c.

Unit `src/editor.c`. The SAVE/LOAD twin of PrintYesNo: the same
two-line menu with the labels `D_80095638` ("  SAVE\n") and `D_80095640`
("  LOAD\n").

```c
void PrintSaveLoad(void) {
    FntPrint(D_800955DC);
    PrintTabs(4);
    if (D_8009574A == 0) {
        FntPrint(D_800954F4);
    } else {
        FntPrint(D_8009550C);
    }
    FntPrint(D_80095638);
    PrintTabs(4);
    if (D_8009574A == 1) {
        FntPrint(D_800954F4);
    } else {
        FntPrint(D_8009550C);
    }
    FntPrint(D_80095640);
}
```

First build; same levers as PrintYesNo (one call per arm).
