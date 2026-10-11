# ScatterDarkDust — MATCHED 46/46 (round 5, delta)

> Renamed from `ScatterEffect6` on 2026-10-11 (tools/rename.py). Address 0x80023020.

> Renamed from `func_80023020` on 2026-10-11 (tools/rename.py). Address 0x80023020.

Unit `src/player.c`. The twin of ScatterDust with spawn id 6 instead
of 4: `SpawnEffect(6, ...)` at `pos` with x and z jittered by a random
offset in -80..79, y kept, last argument 1.

```c
void ScatterDarkDust(VECTOR *pos) {
    SpawnEffect(6, (s16)pos->vx + (rand() % 160 - 80), (s16)pos->vy,
                  (s16)pos->vz + (rand() % 160 - 80), 1);
}
```

## Levers

- None new. ScatterDust's body with the id changed matched on the first
  build: the grouping `a + (rand() % 160 - 80)` puts the `addiu -80` on the
  loaded halfword, and the unit's `s32`-parameter view of `SpawnEffect`
  keeps the sums unextended.
