# func_80031AEC — MATCHED 258/258 (round 10, echo)

Unit `src/code_1dc24.c`. The Rec3C picker, func_80031064's twin on the
100-entry Rec3C table `D_800A7898`: counts the used records (`unk0 != -1`)
into `sTotals.unk30`; in state 0 picks the one nearest the game position
(its `unk4` position) into `sTotals.unk28` (errors 9 and 11 as the twin);
in state 1 copies its `unk2C` to `sTotals.unk2A`, moves the game position
to it, frees it on flag bit 5 and steps the selection over free records on
flag bits 0 and 1. First build matched.

## Shapes read from retail

- Instruction for instruction func_80031064 with the table, the field
  offsets, the record size (`sll 4; subu; sll 2` = 0x3C) and the bounds
  (100, clamp 99) changed. The twin's body was written by substitution and
  matched on its first build; every lever of func_80031064 carries over,
  including the first step up clamping to 99 rather than wrapping to 0.
- Not shared through a `static __inline__` helper: the two tables have
  different record types and field offsets.

## Declarations

- None new beyond func_80031064's (the `func_800297A4` prototype).
- `config/typeviews-warnings.txt`: one more code_1dc24 "control reaches
  end of non-void function".

```c
s32 func_80031AEC(void) {
    /* func_80031064 with D_800A7898 for ((Obj48 *)D_800A9008), unk0 for
     * unk36, unk4[] for unk0[], unk2C for unk34, sTotals.unk30 / unk28 /
     * unk2A for unk1A / unk18 / unk1E, and 100 / 99 for 200 / 199 */
}
```

(The full body is in `src/code_1dc24.c`.)
