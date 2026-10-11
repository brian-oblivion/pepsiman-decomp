# FormatCard — MATCHED 28/28 (round 3, echo)

> Renamed from `func_800383F8` on 2026-10-10 (tools/rename.py). Address 0x800383f8.

Unit `src/code_27bc8.c`. Formats the memory card `"bu10:"` (`sCardDevice`, a `.sdata` string reached `lui`/`addiu`, so `extern char sCardDevice[]`) with libapi's `format`, up to ten tries. Returns 0 on the first success; after ten failures it stores 11 in the halfword `gCardError` and returns -1.

```c
s32 FormatCard(void) {
    s32 ret;
    s16 i;

    ret = 0;
    for (i = 0; i < 10; i++) {
        if (format(sCardDevice) != 0) {
            return ret;
        }
    }
    gCardError = 11;
    return -1;
}
```

## Levers

- **The success value lives in a callee-saved register: an `s32` local
  returned.** Retail sets `$s2` to 0 at entry, copies it to the counter
  (`addu $s0, $s2, $zero`) and returns it with `addu $v0, $s2, $zero`.
  The first build with an `s16` local (and `s16` return) gave the same
  frame but returned `sra $v0, $s2, 16`, 21/28. Declaring both the local
  and the function `s32` matched. `return 0;` gives a plain `addu $v0,
  $zero, $zero` and no `$s2` at all.
- The counter is `s16` (`sll`/`sra` before `slti 10`).
