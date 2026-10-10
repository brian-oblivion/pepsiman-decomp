# WaitCardSwEvent — MATCHED 28/28 (round 3, echo)

> Renamed from `func_80038820` on 2026-10-10 (tools/rename.py). Address 0x80038820.

Unit `src/code_27bc8.c`. Waits for one of the first four memory-card events (`gCardSwIoe`..`gCardSwNew`, probably the software end, error, timeout and new-card events of a `_card_info` or `_card_load` request) and returns its index 0-3.

```c
s16 WaitCardSwEvent(void);
void ClearCardSwEvents(void);

s16 CompareBytes(u8 *a, u8 *b, s16 n) {
    s16 i;

    for (i = 0; i < n; i++) {
        if (*a != *b) {
            return -1;
        }
        a++;
        b++;
    }
    return 0;
}
```

Matched on the first build. `while (1)` with four `if (TestEvent(e) == 1) return k;`: GCC keeps the 1 in `$s0`, puts each return value in the `beq` delay slot, and the last test becomes the loop's `bne` back to the top. The function has no exit other than the returns, so no trailing return is needed. `s16` is the prototype `CheckCard` already used (its caller sign-extends the result).
