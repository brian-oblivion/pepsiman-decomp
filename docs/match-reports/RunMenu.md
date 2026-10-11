# RunMenu — MATCHED 31/31 (round 5, alpha)

> Renamed from `func_8003356C` on 2026-10-11 (tools/rename.py). Address 0x8003356c.

Unit `src/editor.c`. Clears `gEditValue`, stores `n` in `gMenuLineCount`, 1
in `gEditMax`, wraps `gMenuLine` modulo `n`, calls `StepMenuCursor`, and
returns `gMenuLine` when bit 5 of `gPadTrig` is set, else -1.

Matched on the first build, in source order. The `div` has no divide-by-zero
trap here, and cc1 + maspsx emit none either; the `%` is plain.

Declarations: `gMenuLine`, `gEditMax` and `gPadTrig` were declared
locally in `src/hit_tool.c`; they moved to `include/common.h` (additive)
and the local copies were deleted (hit_tool is not one of round 5's
units). `gMenuLineCount` (also reached by hit_tool's asm) was added to
`include/common.h` too. All four are reached `lui` (not in the gp list),
as plain externs.
