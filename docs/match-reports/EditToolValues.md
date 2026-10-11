# EditToolValues — MATCHED 56/56 (round 4, charlie)

> Renamed from `func_800371A0` on 2026-10-11 (tools/rename.py). Address 0x800371a0.

Unit `src/hit_tool.c`. One frame of the two-line value editor whose menu
`PrintToolSlots` prints: remembers the highlighted line, runs the editor
(`StepMenuCursor`), then either loads the newly highlighted line's saved value
and limit into the edit variables, or, if the line did not change, saves the
edited value back. Bit 6 of the flag word clears `gToolMode`.

```c
void EditToolValues(void) {
    s32 unused[2];
    s16 line = gMenuLine;

    StepMenuCursor();
    if (line != gMenuLine) {
        switch (gMenuLine) {
            case 0:
                gEditMax = 0x33;
                gEditValue = gToolRadius;
                break;
            case 1:
                gEditMax = 0x100;
                gEditValue = gToolType;
                break;
        }
    } else {
        switch (line) {
            case 0:
                gToolRadius = gEditValue;
                break;
            case 1:
                gToolType = gEditValue;
                break;
        }
    }
    if (gPadTrig & 0x40) {
        gToolMode = 0;
    }
}
```

## Externs

- `gToolRadius` (`u16`, `lhu`) and `gToolType` (`u8`, `lbu`): `%gp_rel`.
- `gEditValue`: `u16`, the type `object` already gave it; the `u8`
  store reads it with `lbu` (cc1 narrows the load). Now used by two units,
  so it moved to `include/common.h` (additive). `object.c` still carries
  its own identical `extern u16 gEditValue;` (line 89), which declcheck now
  reports as MULTI/LOCAL; that unit is alpha's this round, so the two-line
  deletion is left to the merge.
- `StepMenuCursor` (an `INCLUDE_ASM` of `object`): prototype added to
  `include/common.h` (additive).

## Levers

| C | result |
| --- | --- |
| as above without `unused` | 50/56: frame 0x18, `$s0`/`$ra` at 0x10/0x14 |
| **`s32 unused[2];`** | **56/56**: frame 0x20, saves at 0x18/0x1C |

The two arms of the first `switch` share their stores by cross-jumping
(retail loads the pair into `$v0`/`$v1` and jumps to one store block); the
plain per-case assignments give that without help.

The `unused` local triggers `-Wall`'s unused-variable warning, NEW in
`tools/typeviews.py --warnings` (`hit_tool: unused variable 'unused'`), to
be baselined.
