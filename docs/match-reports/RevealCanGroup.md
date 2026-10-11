# RevealCanGroup — MATCHED 25/25 (round 4, alpha; landed by the head)

> Renamed from `MarkByGroup` on 2026-10-11 (tools/rename.py). Address 0x8002d1cc.

> Renamed from `func_8002D1CC` on 2026-10-11 (tools/rename.py). Address 0x8002d1cc.

Unit `src/object.c`. Sets `gCanGroupOn[n] = 1`, then marks with 1 every one
of the 200 block entries whose `unk6` equals `n`.

Alpha matched it on the first build, then restored the `INCLUDE_ASM` to stay
within the round's 8-function budget; the head landed the body after the
merges. Same shape as its sibling ShowRevealedCans: `s32` with no return, which
keeps the loop's delay slot a `nop` (round 1's ClearCanGroups lever).
