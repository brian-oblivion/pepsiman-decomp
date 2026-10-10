# ClearCardSwEvents — MATCHED (round 1, bravo)

> Renamed from `func_80038900` on 2026-10-10 (tools/rename.py). Address 0x80038900.

18/18 words. Polls four event descriptors with libapi's `TestEvent`, results
discarded:

```c
void ClearCardSwEvents(void) {
    TestEvent(gCardSwIoe);
    TestEvent(gCardSwError);
    TestEvent(gCardSwTimeout);
    TestEvent(gCardSwNew);
}
```

The four descriptors are `%gp_rel` words, declared locally as `extern s32`.
`TestEvent` is prototyped in the unit as `long TestEvent(long event);`, the
signature of `include/psyq/libapi.h` (no unit includes the Psy-Q headers yet).
The four are probably the memory card's software/hardware end, error and
timeout events (`CheckCard` calls this before `_card_info`).
