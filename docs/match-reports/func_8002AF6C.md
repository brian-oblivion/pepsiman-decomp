# func_8002AF6C — MATCHED 56/56 (round 8, alpha)

Unit `src/code_1a098.c`. Finds the path segment a `PathUser` is on with
func_8002AEB8's two half-plane tests, but does not store it; returns the
`ratan2` direction of that segment, from its point to the next.

```c
s16 func_8002AF6C(PathUser *u) {
    s32 unused[2];
    s32 i;
    s32 d;
    PathPt *next;
    PathPt *pt;
    PathPt *seg;

    i = u->seg;
    next = (PathPt *)(i * 8 + (u32)D_800958A0) + 1;
    d = next->dx * (u->x - next->x) + next->dz * (u->z - next->z);
    if (d >= 0) {
        i++;
    }
    pt = (PathPt *)(i * 8 + (u32)D_800958A0);
    d = -pt->dx * (u->x - pt->x) + -pt->dz * (u->z - pt->z);
    if (d >= 0) {
        i--;
    }
    seg = (PathPt *)(i * 8 + (u32)D_800958A0);
    return ratan2(seg[1].x - seg->x, seg[1].z - seg->z);
}
```

## Path to the match (5 builds)

1. func_8002AEB8's body, the store replaced by the `ratan2` through `pt`
   reassigned: frame 0x18 against 0x20, 36/56.
2. `s32 unused[2];` for the frame: 40/56, register colouring only. The
   second test's point lives in `$a0` in retail (the parameter is dead by
   then; `u->x`/`u->z` are in `$t0`/`$t1`), in `$a1` in the build.
3. One point pointer for all three: 24/56.
4. The last one through `next`: 32/56.
5. A third local `seg` for the last point: 56/56.

### Proposed learning

Pointer locals that are each assigned once colour differently from one
local reassigned: when retail's registers for two address computations
disagree with the build's, try a fresh local per computation before
reusing one. (func_8002AF6C; func_8002AEB8 went the other way)
