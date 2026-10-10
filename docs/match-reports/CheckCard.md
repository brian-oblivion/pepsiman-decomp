# CheckCard — MATCHED (round 1, bravo)

> Renamed from `func_80038394` on 2026-10-10 (tools/rename.py). Address 0x80038394.

25/25 words. A memory-card probe of port 0x10 (slot 1 on port B in libcard's
numbering): clears the pending card events, asks `_card_info`, waits for the
outcome, -1 when the outcome is 2, 0 otherwise.

```c
s16 CheckCard(void) {
    s16 result;

    result = 0;
    EnableCardEvents();
    ClearCardSwEvents();
    _card_info(0x10);
    if (WaitCardSwEvent() == 2) {
        result = -1;
    }
    DisableCardEvents();
    return result;
}
```

Prototypes in the unit: `long _card_info(long chan);` (libapi.h's
signature), and the unit's own `EnableCardEvents`, `DisableCardEvents`,
`ClearCardSwEvents` as `void (void)` and `WaitCardSwEvent` as `s16 (void)` (its
result is sign-extended from 16 bits before the compare). They are defined
later in the unit, so the prototypes sit at the top.
