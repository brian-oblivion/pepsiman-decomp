# StartScreenWipe — MATCHED (11/11), round 3, runner bravo

> Renamed from `func_8003E40C` on 2026-10-10 (tools/rename.py). Address 0x8003e40c.

Unit `src/code_29f54.c`. Clears `gWipeFrame`, sets `gWipeActive` to 1, then
calls `PlaySe(0x19)` (unit `code_31cec`, still asm; prototyped in this
unit).

```c
void StartScreenWipe(void) {
    gWipeFrame = 0;
    gWipeActive = 1;
    PlaySe(0x19);
}
```

Both globals are `%gp_rel` words: plain `extern s32` in the unit. First try.
