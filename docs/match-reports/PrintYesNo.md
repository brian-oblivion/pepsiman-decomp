# PrintYesNo — MATCHED 43/43 (round 6, bravo)

> Renamed from `func_80033C90` on 2026-10-11 (tools/rename.py). Address 0x80033c90.

Unit `src/editor.c`. Prints a two-line YES/NO menu with `FntPrint`:
a blank-line header (`D_800955DC`), then for each line an indent
(`PrintTabs(4)`), the highlight colour code `D_800954F4` when the
cursor `D_8009574A` is on that line or the plain one `D_8009550C`
otherwise, and the label (`D_80095628` "  YES\n", `D_80095630` "  NO").

```c
void PrintYesNo(void) {
    FntPrint(D_800955DC);
    PrintTabs(4);
    if (D_8009574A == 0) {
        FntPrint(D_800954F4);
    } else {
        FntPrint(D_8009550C);
    }
    FntPrint(D_80095628);
    PrintTabs(4);
    if (D_8009574A == 1) {
        FntPrint(D_800954F4);
    } else {
        FntPrint(D_8009550C);
    }
    FntPrint(D_80095630);
}
```

First build. The arms build their own `lui`/`addiu` before one shared
`jal FntPrint` at the join: one call per arm, cross-jumped (the shape of
PrintToolSlots, docs/LEARNINGS.md). The seven `.sdata` strings are reached
only from this unit; declared locally as `extern char D_...[];`
(`lui`/`addiu` address builds). `libgpu.h` is now included for `FntPrint`.
