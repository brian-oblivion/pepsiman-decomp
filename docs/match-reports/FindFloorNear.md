# FindFloorNear — MATCHED (30/30), round 7, runner alpha

> Renamed from `func_80018D70` on 2026-10-10 (tools/rename.py). Address 0x80018d70.

Unit `src/model.c`. Round 3 (runner charlie) skipped it at the
budget, 1 word short, 3/30; round 7 matched it. Copies a 3-word position
to the stack, lets `UpdateSection` update `cur` through its address from
the position's x and z, then asks `QueryFloor(arg, cur, &pos,
gFloorData)`; returns -1 when that gives 0x7FFF, else the updated `cur`.

```c
typedef struct {
    s32 x;
    s32 y;
    s32 z;
} Vec3i;

extern s32 gFloorData;
s32 QueryFloor(void *out, s32 index, Vec3i *pos, s32 data);
/* MATCHING: this unit's view; stage defines the coordinates s16. */
void UpdateSection(s32 *index, s32 x, s32 y);

s32 FindFloorNear(void *pos, void *arg, s32 cur) {
    Vec3i p;

    p.x = ((Vec3i *)pos)->x;
    p.y = ((Vec3i *)pos)->y;
    p.z = ((Vec3i *)pos)->z;
    UpdateSection(&cur, p.x, p.z);
    if (QueryFloor(arg, cur, &p, gFloorData) == 0x7FFF) {
        return -1;
    }
    return cur;
}
```

## What round 3 missed

Retail's three copy registers (`$a1`, `$v1`, `$a2`) are not a copy
idiom: `$a1` and `$a2` are the second and third ARGUMENTS of
UpdateSection, the position's x and z, loaded once and both stored and
passed. Round 3 called `UpdateSection(&arg2)` with one argument.

## Attempts (round 7: 2)

1. The body above with UpdateSection seen as `(s32 *, s16, s16)`, its
   definition: 17/30. The arguments were reloaded from the stack with
   `lh`.
2. The unit's view `(s32 *, s32, s32)` (LEARNINGS: a caller passing
   arguments unextended saw `s32` parameters): 30/30.

## Declarations

- `FindFloorNear`'s prototype moved from object.c into
  `include/common.h` (additive); object.c's local copy is deleted.
- `QueryFloor`'s first parameter is now `void *` in this unit (both
  callers here pass different buffers).

### Proposed learning

- **Registers of a field copy that look wrong may be call arguments:**
  check whether the copied values sit in `$a1`.. when the next `jal`
  runs. (FindFloorNear)
