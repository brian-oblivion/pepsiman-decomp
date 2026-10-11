# EditorEditPoints — MATCHED 193/193 (round 9, alpha)

> Renamed from `func_8002FF74` on 2026-10-11 (tools/rename.py). Address 0x8002ff74.

Unit `src/code_1dc24.c`. The point editor, a two-state step on
`gSubStep` and the delete twin of EditorInsertPoint. State 0 selects the
current entry's first point into `sTotals.unk22` and moves to state 1.
State 1 moves the game position to the selected point
(`DrawPin`), errors 10 on an empty or missing entry, and unless
`D_800959D8` is 1, on flag bit 5 deletes the point (error 9 on an empty
entry): shifts the later points down, decrements the entry's count and
every later entry's start. Then flag bits 0 and 1 step the selection and
it is clamped to the entry. About 20 builds.

## Shapes read from retail

- **Every entry access as `e = (Span8 *)gCanSpans; e += k;`** (retail's
  `addu a1, a1, v0`, base register as destination; EditorInsertPoint's
  lever). With indexed spellings the build also dropped the second
  `count == 0` test: CSE proved it from the first. Retail reloads the
  pointer and the count; the per-statement local reproduces that (43/193 ->
  181/193).
- **The clamp compares unsigned (`sltu`)**: `(u32)` casts on the
  `Span8` fields (its `s32` fields gave `slt`).
- **`start - 1 + count` with `start` loaded first: written
  `start + (count - 1)`**. fold moves the constant onto the FIRST operand
  of the outer sum, so `start - 1 + count` and `count + (start - 1)` both
  come out as `(count - 1) + start`. The narrowed store is the other way
  round: `(u32)count - 1 + start` gives retail's `ori 0xFFFF`, `addu`,
  `addu` with `start` loaded first. Twelve spellings tried
  (165..191/193).
- Non-void with no return value: the loop and flag tests keep their `nop`
  delay slots (181 -> 186/193).

## Declarations

- `Totals28` (unit-local) gains `u16 unk22`.
- `DrawPin(VECTOR *pos)` (hit_tool) declared in the `.c` with
  `TransformByCoord` under one MATCHING note: their units have no header.

```c
s32 EditorEditPoints(void) {
    VECTOR pos;
    Pt8 *dst;
    Pt8 *src;
    Span8 *e;
    s32 i;
    u32 j;

    /* MATCHING: non-void with no return value; each entry access is
     * `e = base; e += k;`, which also keeps the second count test. */
    switch (gSubStep) {
        case 0:
            e = (Span8 *)gCanSpans;
            e += gCurBlock;
            gSubStep = 1;
            sTotals.unk22 = e->start;
            return;
        case 1:
            break;
        default:
            return;
    }
    src = (Pt8 *)gCanPoints;
    src += sTotals.unk22;
    pos.vx = src->x;
    pos.vy = src->y;
    pos.vz = src->z;
    DrawPin(&pos);
    e = (Span8 *)gCanSpans;
    e += gCurBlock;
    if (e->count == 0 || gCurBlock == -1) {
        gToolMode = 10;
        return;
    }
    if (D_800959D8 != 1 && (gPadTrig & 0x20)) {
        e = (Span8 *)gCanSpans;
        e += gCurBlock;
        if (e->count == 0) {
            gToolMode = 9;
            return;
        }
        e = (Span8 *)gCanSpans;
        e += gCurBlock;
        dst = (Pt8 *)gCanPoints;
        src = dst;
        src += sTotals.unk22 + 1;
        dst += sTotals.unk22;
        for (i = e->start + e->count; i < 199; i++) {
            *dst = *src;
            dst++;
            src++;
        }
        e = (Span8 *)gCanSpans;
        e += gCurBlock;
        e->count--;
        e = (Span8 *)gCanSpans;
        e += gCurBlock + 1;
        for (j = gCurBlock + 1; j < gStageObjCount; j++) {
            e->start--;
            e++;
        }
    }
    if (gPadTrig & 1) {
        sTotals.unk22--;
    }
    if (gPadTrig & 2) {
        sTotals.unk22++;
    }
    e = (Span8 *)gCanSpans;
    e += gCurBlock;
    if (sTotals.unk22 < (u32)e->start) {
        sTotals.unk22 = e->start;
    }
    /* MATCHING: the compare as start + (count - 1), the store as
     * count - 1 + start; unsigned, as retail's sltu. */
    if ((u32)e->start + (e->count - 1) < sTotals.unk22) {
        sTotals.unk22 = (u32)e->count - 1 + e->start;
    }
}
```

### Proposed learning

- A sum with a constant whose loads must come in source order: put the
  constant inside the SECOND operand, `a + (b - 1)`; fold rewrites both
  `a - 1 + b` and `b + (a - 1)` to `(b - 1) + a`. (EditorEditPoints)
- A test the build drops as already proven, which retail repeats with
  fresh loads of the pointer: re-derive the element pointer as its own
  statements (`e = base; e += k;`) before the test. (EditorEditPoints)
