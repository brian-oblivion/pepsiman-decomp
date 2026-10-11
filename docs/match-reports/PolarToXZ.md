# PolarToXZ — MATCHED 33/33 (round 5, alpha)

> Renamed from `func_8002C188` on 2026-10-11 (tools/rename.py). Address 0x8002c188.

Unit `src/code_1a098.c`. Converts `deg` (`s16`, degrees) to a 4096-per-turn
angle and sets `out->x = rsin(a) * r`, `out->z = rcos(a) * r`.

Matched on the first build: `a = deg * 4096 / 360;` gives retail's
`sll 16` / `sra 4` and the 0xB60B60B7 magic division. The unit now includes
`libgte.h` for Sony's `rsin`/`rcos` prototypes (it includes cleanly after
`common.h`).
