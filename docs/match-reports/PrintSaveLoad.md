# PrintSaveLoad — MATCHED 43/43 (round 6, bravo)

> Renamed from `func_80033D3C` on 2026-10-11 (tools/rename.py). Address 0x80033d3c.

Unit `src/editor.c`. The SAVE/LOAD twin of PrintYesNo: the same
two-line menu with the labels `sEdSaveText` ("  SAVE\n") and `sEdLoadText`
("  LOAD\n").

```c
void PrintSaveLoad(void) {
    FntPrint(sEdNewlines2);
    PrintTabs(4);
    if (D_8009574A == 0) {
        FntPrint(sEdColorWhite);
    } else {
        FntPrint(sEdColorBlue);
    }
    FntPrint(sEdSaveText);
    PrintTabs(4);
    if (D_8009574A == 1) {
        FntPrint(sEdColorWhite);
    } else {
        FntPrint(sEdColorBlue);
    }
    FntPrint(sEdLoadText);
}
```

First build; same levers as PrintYesNo (one call per arm).
