# RunMenu — MATCHED 31/31 (round 5, alpha)

> Renamed from `func_8003356C` on 2026-10-11 (tools/rename.py). Address 0x8003356c.

Unit `src/code_1dc24.c`. Clears `D_80095748`, stores `n` in `D_800958B2`, 1
in `D_800958B0`, wraps `D_8009574A` modulo `n`, calls `StepMenuCursor`, and
returns `D_8009574A` when bit 5 of `gPadTrig` is set, else -1.

Matched on the first build, in source order. The `div` has no divide-by-zero
trap here, and cc1 + maspsx emit none either; the `%` is plain.

Declarations: `D_8009574A`, `D_800958B0` and `gPadTrig` were declared
locally in `src/code_24748.c`; they moved to `include/common.h` (additive)
and the local copies were deleted (code_24748 is not one of round 5's
units). `D_800958B2` (also reached by code_24748's asm) was added to
`include/common.h` too. All four are reached `lui` (not in the gp list),
as plain externs.
