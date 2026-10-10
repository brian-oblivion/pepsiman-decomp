# OpenCardEvents — MATCHED 90/90 (round 9, delta)

> Renamed from `func_80038124` on 2026-10-10 (tools/rename.py). Address 0x80038124.

Unit `src/code_27bc8.c`. Memory card event setup: inside a critical section,
opens the four software-card events (`0xF4000001`) and the four
hardware-card events (`0xF0000011`) for the specs 4 (I/O end), 0x8000
(error), 0x100 (timeout) and 0x2000 (new card), mode 0x2000 (EvMdNOINTR), no
handler, storing the descriptors in `gCardSwIoe`..`gCardHwNew`; then disables
all eight. The disables are written out, not a call to DisableCardEvents, which
has the same body.

Matched on the first build.

```c
/** @brief Opens the eight memory card events (four software, four hardware) and leaves them disabled. */
void OpenCardEvents(void) {
    EnterCriticalSection();
    gCardSwIoe = OpenEvent(0xF4000001, 4, 0x2000, NULL);
    gCardSwError = OpenEvent(0xF4000001, 0x8000, 0x2000, NULL);
    gCardSwTimeout = OpenEvent(0xF4000001, 0x100, 0x2000, NULL);
    gCardSwNew = OpenEvent(0xF4000001, 0x2000, 0x2000, NULL);
    gCardHwIoe = OpenEvent(0xF0000011, 4, 0x2000, NULL);
    gCardHwError = OpenEvent(0xF0000011, 0x8000, 0x2000, NULL);
    gCardHwTimeout = OpenEvent(0xF0000011, 0x100, 0x2000, NULL);
    gCardHwNew = OpenEvent(0xF0000011, 0x2000, 0x2000, NULL);
    ExitCriticalSection();
    DisableEvent(gCardSwIoe);
    DisableEvent(gCardSwError);
    DisableEvent(gCardSwTimeout);
    DisableEvent(gCardSwNew);
    DisableEvent(gCardHwIoe);
    DisableEvent(gCardHwError);
    DisableEvent(gCardHwTimeout);
    DisableEvent(gCardHwNew);
}
```

## Notes

- `addiu a1, 0x2000; addu a2, a1, zero` for the fourth spec is cc1 reusing
  the equal constant; no source spelling needed.
