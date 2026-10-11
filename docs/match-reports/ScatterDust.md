# ScatterDust — MATCHED 46/46 (round 3, delta)

> Renamed from `ScatterEffect4` on 2026-10-11 (tools/rename.py). Address 0x80022f68.

> Renamed from `func_80022F68` on 2026-10-11 (tools/rename.py). Address 0x80022f68.

Unit `src/player.c`. Spawns something (`SpawnEffect(4, ...)`) at a
position jittered from `pos`: x and z each get a random offset in
-80..79, y is kept, last argument 1. Callers pass `&sGame.unk348`, the
game-state record's position `VECTOR` (0x348, 0x34C, 0x350).

```c
void ScatterDust(VECTOR *pos) {
    SpawnEffect(4, (s16)pos->vx + (rand() % 160 - 80), (s16)pos->vy,
                  (s16)pos->vz + (rand() % 160 - 80), 1);
}
```

The unit now includes `rand.h` and prototypes `SpawnEffect` locally
with `s32` parameters (MATCHING note in the `.c`).

## Levers

- **Where the constant goes decides the add.** `(s16)pos->vx - 80 +
  rand() % 160` folds the -80 onto the remainder (`addiu v0,v0,-80` after
  the `%`), 28/46; `rand() % 160 + ((s16)pos->vx - 80)` was worse (16/46).
  Retail adds -80 to the loaded halfword: `(s16)pos->vx + (rand() % 160 -
  80)` gives that, 46/46. GCC reassociates the constant to the side other
  than the one it was written next to.
- **`(s16)` of a `VECTOR` member is an `lh` at the member's offset** (0, 4,
  8), no `lw` and no shifts.
- **The callee's `s16` parameters are not this unit's prototype.**
  stage defines `SpawnEffect(s32, s16, s16, s16, s16)`; with that
  prototype the caller sign-extends the two sums (20/46). Retail passes
  them unextended, so the local prototype takes `s32` (bravo found the
  mirror case for returns this round). `tools/declcheck.py` passes.

### Proposed learning

A constant written next to a call's result in `a - K + f()` is moved onto
`f()`'s side; to get `addiu` on the other operand, group it there,
`a + (f() - K)`.
