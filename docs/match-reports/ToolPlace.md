# ToolPlace — MATCHED 24/24 (round 2, charlie)

> Renamed from `func_80036EF0` on 2026-10-11 (tools/rename.py). Address 0x80036ef0.

Unit `src/hit_tool.c`. When bit 5 of the flag word `gPadTrig` is set,
dispatches on the state byte `gToolKind`: 0 calls `InsertHitPoint`, 1 calls
`PlaceHitQuad`.

```c
void ToolPlace(void) {
    if (gPadTrig & 0x20) {
        switch (gToolKind) {
            case 0:
                InsertHitPoint();
                break;
            case 1:
                PlaceHitQuad();
                break;
        }
    }
}
```

## Externs

`gPadTrig` is loaded `lui $v0; lw %lo(..)($v0)` and is not in
`config/gp-symbols.txt`: a plain `extern s32` in the unit. `gToolKind` is
`%gp_rel` and in the list: a plain `extern u8`. Both callees are in this
unit; their prototypes are in the `.c`.

## Levers

None needed: first try. A `switch` gives retail's `beqz` / `beq 1` / `j`
skeleton directly.
