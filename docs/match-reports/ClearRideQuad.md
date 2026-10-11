# ClearRideQuad — MATCHED 3/3 (round 4, bravo)

> Renamed from `func_8002988C` on 2026-10-11 (tools/rename.py). Address 0x8002988c.

Unit `src/collision.c`. Clears the word `D_8009EF30`.

```c
void ClearRideQuad(void) {
    D_8009EF30[0] = 0;
}
```

First try. Retail stores through `lui $v0` / `sw $zero, %lo(D_8009EF30)($v0)`:
cc1's split, not the assembler's `$at` expansion of a scalar, so the unit
declares `extern s32 D_8009EF30[];` with a `MATCHING:` note. Its only caller
is ResetCollision, which ignores `$v0`: `void`.
