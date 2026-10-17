# PlaceStageTrap — MATCHED 68/68 with its jump table (82 words in all) (round 6, delta)

> Renamed from `CallStageHook` on 2026-10-17 (tools/rename.py). Address 0x80040e04.

> Renamed from `func_80040E04` on 2026-10-10 (tools/rename.py). Address 0x80040e04.

Unit `src/stage_flow.c`. Dispatches on the mode byte `gStage` (0..13)
to one of nine overlay routines, passing its arguments through, and returns
the result as `u16`; modes 2, 5, 8, 11 and out-of-range ones return 1.

```c
u16 PlaceTrap1_1(s16 arg0, s32 arg1, s32 arg2); /* and eight more */

u16 PlaceStageTrap(s16 arg0, s32 arg1, s32 arg2) {
    u16 ret = 1;

    switch (gStage) {
    case 0:
        ret = PlaceTrap1_1(arg0, arg1, arg2);
        break;
    case 1:
        ret = PlaceTrap1_2(arg0, arg1, arg2);
        break;
    /* 3: FA894, 4: FA580, 6 and 7: F1A48, 9: F9ABC, 10: F9CA4,
     * 12: FA060, 13: F922C */
    }
    return ret;
}
```

## Levers

- **Pass-through arguments set the register of `ret`.** With one `s16`
  argument the build kept `ret` in `$a1` (56/68: the eleven moves of the
  result plus the `li 1` and the final `andi`). Retail uses `$a3`, so
  `$a1` and `$a2` must be live across the switch: the function takes two
  more arguments and hands them to the callee unchanged, so nothing about
  them shows in the code but the register choice. Matched on the first try.
  Whether the callees really take three arguments is unproven (they live
  in an overlay); the prototypes are this unit's view.
- `arg0` as `s16` gives the per-call `sll`/`sra`.

### Proposed learning

- **A result register that skips `$a1`/`$a2` (e.g. `ret` in `$a3`) in a
  function that never touches them: the function has those arguments and
  passes them through to its callees untouched.** (PlaceStageTrap)
