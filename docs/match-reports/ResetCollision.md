# ResetCollision — MATCHED 11/11 (round 4, bravo)

> Renamed from `func_8002980C` on 2026-10-10 (tools/rename.py). Address 0x8002980c.

Unit `src/code_1902c.c`. Clears byte 1 of `D_8009EB78`, then calls
ResetPlayerHits and func_8002988C.

```c
void ResetCollision(void) {
    D_8009EB78[1] = 0;
    ResetPlayerHits();
    func_8002988C();
}
```

First try. Retail names the store target `D_8009EB79`; `D_8009EB78[1]` folds
to the same `%hi/%lo` pair and the same bytes. Callers (func_80037114,
func_8002B220, func_8002B04C) ignore `$v0`: `void`.
