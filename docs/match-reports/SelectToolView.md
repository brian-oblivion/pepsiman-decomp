# SelectToolView — MATCHED 73/73 (round 4, charlie)

> Renamed from `func_800355D8` on 2026-10-11 (tools/rename.py). Address 0x800355d8.

Unit `src/hit_tool.c`. Input handling for two byte settings: flag bits 0
and 1 step `gToolViewPreset` up and down, bits 7 and 6 step `gToolZoom`; each
step also saves `gToolMode` into `sToolModeSaved`, sets it to 6 and clears
`gToolViewStep`. Then `gToolZoom` is clamped to 1..20 and `gToolViewPreset` wraps
within 0..4.

```c
void SelectToolView(void) {
    s32 flags = gPadTrig;
    u8 v;

    if (flags & 1) {
        gToolViewPreset++;
        sToolModeSaved = gToolMode;
        gToolMode = 6;
        gToolViewStep = 0;
    }
    /* ... the same for bit 1 (gToolViewPreset--), bit 7 (gToolZoom++) and
     * bit 6 (gToolZoom--) ... */
    v = gToolZoom;
    if ((s8)gToolZoom > 0) {
        if ((s8)gToolZoom > 20) {
            v = 20;
        }
    } else {
        v = 1;
    }
    gToolZoom = v;
    gToolViewPreset = (s8)gToolViewPreset < 0 ? 4 : (s8)gToolViewPreset > 4 ? 0 : gToolViewPreset;
}
```

## Externs

All the bytes and halfwords are `%gp_rel` plain externs except
`gToolMode` (`lui`, `s16`). The two settings are `u8` read through an
`(s8)` cast for the tests (`lb` then `lbu`, LEARNINGS).

## Levers

| C for the `gToolZoom` clamp | result |
| --- | --- |
| nested ternary like the `gToolViewPreset` one | 69/73: value in `$a0`, `beqz` with 20 in the slot, then `move` |
| ternary with `< 21 ? x : 20` | 69/73, the same code |
| `v = x; if (x < 1) v = 1; else if (x > 20) v = 20;` | 67/73: value in `$v1` now, but the `= 1` block comes first |
| **`v = x; if (x > 0) { if (x > 20) v = 20; } else v = 1;`** | **73/73** |

The two clamps in one function take different shapes: the wrap of
`gToolViewPreset` is the nested ternary (`move $v1, $a0` in a `j` slot), the clamp
of `gToolZoom` is a local preloaded with the value (`lbu` straight into the
result register), with the low-side assignment in the `else` placed last.

### Proposed learning

A clamp whose value is loaded straight into the stored register, with the
"too low" block laid out after the "too high" `j`: a local preloaded with the
value and nested `if`s, `v = x; if (x > lo) { if (x > hi) v = hi; } else v
= lo; x = v;`. The nested ternary gives `move` of the value in a `j` slot.
(SelectToolView)
