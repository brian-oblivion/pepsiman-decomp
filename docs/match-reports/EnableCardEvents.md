# EnableCardEvents — MATCHED 30/30 (round 3, echo)

> Renamed from `func_80038730` on 2026-10-10 (tools/rename.py). Address 0x80038730.

Unit `src/code_27bc8.c`. Enables the eight memory-card event descriptors with libapi's `EnableEvent`: the four of `ClearCardSwEvents` (`gCardSwIoe`..`gCardSwNew`), then the four of `ClearCardHwEvents` (`gCardHwIoe`..`gCardHwNew`).

```c
void EnableCardEvents(void);
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

Matched on the first build. The descriptors are the unit's existing `extern s32` `%gp_rel` words; `EnableEvent` comes from `libapi.h`.
