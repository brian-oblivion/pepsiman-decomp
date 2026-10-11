# EditorPickRecord — MATCHED 258/258 (round 10, echo)

> Renamed from `func_80031064` on 2026-10-11 (tools/rename.py). Address 0x80031064.

Unit `src/editor.c`. The EditorTrap record picker: counts the live records
of the 200-entry table `gTraps` into `sTotals.unk1A`; in state 0
(`gSubStep`) picks the record nearest the game position by
`DistanceSquared` into `sTotals.unk18` (error 9 in `gToolMode` with no
record, 11 when none is nearer than 25000000) and moves to state 1; in
state 1 copies the record's `unk34` to `sTotals.unk1E`, moves the game
position to it (`DrawPin`), frees it on flag bit 5 (unless
`gEditorInfo` is 1) and steps the selection down or up on flag bits 0 and 1,
skipping free records. EditorPickSprite is its twin on the StageSprite table.
Fourth build matched.

## Shapes read from retail

- The search loop keeps one pointer (`$s0`) at the record start and
  rebuilds `-1` every pass: `r = &tbl[i];` inside the body (LEARNINGS,
  "Loops"). `r++` at the bottom gave a biased pointer, a hoisted `-1` and
  two more saved registers (11/258).
- `beq $v0, $s0` testing `gEditorInfo == 1` reuses the state value (1)
  held in `$s0`: `switch (gSubStep)`, nothing to write.
- The step-down and step-up each clamp once, then rotate a `while` over
  free records. **Retail's first step up clamps to 199, not 0**; only the
  loop wraps to 0. Kept as retail has it.
- `nop` in the delay slot of the `& 2` test's `beqz`: `s32` with no return
  (LEARNINGS, "Control flow and frames"); new baselined warning.
- **The prologue's `move $s1, $zero` before the `lui`/`addiu` of
  `gEditorState`: the counter cleared first**, `i = 0; sTotals.unk1A = 0;
  for (; i < 200; i++)`, as EditorPlaceSprite does. `for (i = 0; ...)` after
  the store put the address first (254/258).

## Declarations

- `DistanceSquared` (collision, no header) prototyped in this `.c` as
  `s32 (VECTOR *, VECTOR *)`, collision's own definition.
- `config/typeviews-warnings.txt`: one more editor "control reaches
  end of non-void function".

```c
s32 EditorPickRecord(void) {
    VECTOR pos;
    VECTOR at;
    VECTOR sel;
    EditorTrap *r;
    u32 i;
    s32 best;
    s32 d;

    i = 0;
    sTotals.unk1A = 0;
    for (; i < 200; i++) {
        if (((EditorTrap *)gTraps)[i].unk36 != -1) {
            sTotals.unk1A++;
        }
    }
    switch (gSubStep) {
        case 0:
            /* ... */
            for (i = 0; i < 200; i++) {
                r = &((EditorTrap *)gTraps)[i];
                if (r->unk36 != -1) {
                    /* copy r->unk0 to `at`, d = DistanceSquared(&pos, &at),
                     * keep the smallest in best / sTotals.unk18 */
                }
            }
            /* ... */
            break;
        case 1:
            /* ... */
            if (gPadTrig & 2) {
                if (++sTotals.unk18 >= 200) {
                    sTotals.unk18 = 199;
                }
                while (((EditorTrap *)gTraps)[sTotals.unk18].unk36 == -1) {
                    if (++sTotals.unk18 >= 200) {
                        sTotals.unk18 = 0;
                    }
                }
            }
            break;
    }
}
```

(The full body is in `src/editor.c`.)

### Proposed learning

- **`move $sN, $zero` for a loop counter scheduled above a global's
  address in the prologue: `i = 0;` written before the store that precedes
  the loop**, `for (; i < N; i++)`. (EditorPickRecord, EditorPickSprite; the
  same shape as EditorPlaceSprite)
