# LatchProximity — MATCHED 32/32 (round 5, alpha)

> Renamed from `func_8002C6A4` on 2026-10-11 (tools/rename.py). Address 0x8002c6a4.

Unit `src/code_1a098.c`. If bit 1 of a Rec48's `unk41` is clear and the
squared distance `DistanceSquared(D_8009EEC0, r)` is below `range * range`,
sets the bit and returns 1; otherwise returns 0.

Matched on the first build, in the natural shape: an `s32 ret = 0;` local
(retail returns it from `$s2`, LEARNINGS' saved-register zero), the square
computed before the call.

Declarations: `DistanceSquared` (code_1902c, takes two `VECTOR *`) gets a
per-unit `void *` view in the `.c` with a `MATCHING:` note, so the unit
needs no libgte. `D_8009EEC0` moved from `src/code_24748.c` (local copy
deleted; not a round-5 unit) to `include/common.h` as `u8 []`
(additive); retail builds its address `lui`/`addiu` in one register, which
an unknown-size array also gives for a bare address.
