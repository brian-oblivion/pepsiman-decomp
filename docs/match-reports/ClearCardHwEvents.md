# ClearCardHwEvents — MATCHED (round 1, bravo)

> Renamed from `func_80038948` on 2026-10-10 (tools/rename.py). Address 0x80038948.

18/18 words. The sibling of `ClearCardSwEvents` on the next four descriptors
(probably the hardware card events):

```c
void ClearCardHwEvents(void) {
    TestEvent(gCardHwIoe);
    TestEvent(gCardHwError);
    TestEvent(gCardHwTimeout);
    TestEvent(gCardHwNew);
}
```

`gCardHwIoe`..`gCardHwNew` are `%gp_rel` words, declared locally as
`extern s32`.
