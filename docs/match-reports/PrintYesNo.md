# PrintYesNo — MATCHED 43/43 (round 6, bravo)

> Renamed from `func_80033C90` on 2026-10-11 (tools/rename.py). Address 0x80033c90.

Unit `src/editor.c`. Prints a two-line YES/NO menu with `FntPrint`:
a blank-line header (`sEdNewlines2`), then for each line an indent
(`PrintTabs(4)`), the highlight colour code `sEdColorWhite` when the
cursor `gMenuLine` is on that line or the plain one `sEdColorBlue`
otherwise, and the label (`sEdYesText` "  YES\n", `sEdNoText` "  NO").

```c
void PrintYesNo(void) {
    FntPrint(sEdNewlines2);
    PrintTabs(4);
    if (gMenuLine == 0) {
        FntPrint(sEdColorWhite);
    } else {
        FntPrint(sEdColorBlue);
    }
    FntPrint(sEdYesText);
    PrintTabs(4);
    if (gMenuLine == 1) {
        FntPrint(sEdColorWhite);
    } else {
        FntPrint(sEdColorBlue);
    }
    FntPrint(sEdNoText);
}
```

First build. The arms build their own `lui`/`addiu` before one shared
`jal FntPrint` at the join: one call per arm, cross-jumped (the shape of
PrintToolSlots, docs/LEARNINGS.md). The seven `.sdata` strings are reached
only from this unit; declared locally as `extern char D_...[];`
(`lui`/`addiu` address builds). `libgpu.h` is now included for `FntPrint`.
