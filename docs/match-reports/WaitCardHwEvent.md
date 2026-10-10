# WaitCardHwEvent — MATCHED 28/28 (round 3, echo)

> Renamed from `func_80038890` on 2026-10-10 (tools/rename.py). Address 0x80038890.

Unit `src/code_27bc8.c`. `WaitCardSwEvent` for the second four event descriptors (`gCardHwIoe`..`gCardHwNew`): waits for one and returns its index 0-3.

```c
s16 WaitCardHwEvent(void) {
    while (1) {
        if (TestEvent(gCardHwIoe) == 1) {
            return 0;
        }
        if (TestEvent(gCardHwError) == 1) {
            return 1;
        }
        if (TestEvent(gCardHwTimeout) == 1) {
            return 2;
        }
        if (TestEvent(gCardHwNew) == 1) {
            return 3;
        }
    }
}
```

Matched on the first build, same shape as `WaitCardSwEvent`.
