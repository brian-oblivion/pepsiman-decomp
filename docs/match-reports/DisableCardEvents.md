# DisableCardEvents — MATCHED 30/30 (round 3, echo)

> Renamed from `func_800387A8` on 2026-10-10 (tools/rename.py). Address 0x800387a8.

Unit `src/code_27bc8.c`. The inverse of `EnableCardEvents`: the same eight event descriptors, in the same order, through libapi's `DisableEvent`.

```c
void DisableCardEvents(void);
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

Matched on the first build.
