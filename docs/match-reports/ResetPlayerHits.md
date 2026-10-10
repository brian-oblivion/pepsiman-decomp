# ResetPlayerHits — MATCHED 12/12 (round 4, bravo)

> Renamed from `func_8002985C` on 2026-10-10 (tools/rename.py). Address 0x8002985c.

Unit `src/code_1902c.c`. Clears bytes 0, 10 and 20 of `gPlayerHits`, then
calls PlacePlayerPoints.

```c
void ResetPlayerHits(void) {
    gPlayerHits[0] = 0;
    gPlayerHits[10] = 0;
    gPlayerHits[20] = 0;
    PlacePlayerPoints();
}
```

First try, with the shared `extern u8 gPlayerHits[];`. The ten-byte stride
suggests three 10-byte records whose first byte is a flag; not typed yet.
