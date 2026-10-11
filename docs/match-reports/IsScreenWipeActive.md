# IsScreenWipeActive — MATCHED (3/3), round 3, runner bravo

> Renamed from `func_8003E438` on 2026-10-10 (tools/rename.py). Address 0x8003e438.

Unit `src/stage.c`. Getter: returns the word `gWipeActive`.

```c
s32 IsScreenWipeActive(void) {
    return gWipeActive;
}
```

No `jal` reaches it in `asm/` (likely called through a pointer), so the
return type is read from the body: `lw $v0` and nothing else, a getter, not a
stub. `gWipeActive` is in `config/gp-symbols.txt` and every retail access is a
`%gp_rel` `lw`/`sw`: a plain `extern s32` in the unit (the only unit that
reaches it). First try.
