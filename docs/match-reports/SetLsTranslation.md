# SetLsTranslation — MATCHED (35/35), round 3, runner bravo

> Renamed from `func_8003F8D4` on 2026-10-10 (tools/rename.py). Address 0x8003f8d4.

Unit `src/code_29f54.c`. Sets the GS local-screen matrix to a pure
translation: a fresh `GsCOORDINATE2` under `WORLD` with `coord.t` = (x, y, z),
then `GsGetLs` and `GsSetLsMatrix`.

```c
void SetLsTranslation(s16 x, s16 y, s16 z) {
    GsCOORDINATE2 coord;
    MATRIX ls;

    GsInitCoordinate2(WORLD, &coord);
    coord.coord.t[0] = x;
    coord.coord.t[1] = y;
    coord.coord.t[2] = z;
    coord.flg = 0;
    GsGetLs(&coord, &ls);
    GsSetLsMatrix(&ls);
}
```

The unit now includes `libgte.h`, `libgpu.h` and `libgs.h` (in that order;
`libgs.h` needs both) for the Sony prototypes and types. The frame (0x98)
is 0x10 of outgoing-argument space, the 0x50-byte `GsCOORDINATE2` at 0x10
(`t[]` at 0x28..0x30, `flg` at 0x10) and the `MATRIX` at 0x60. The
arguments are `s16` (the `sll`/`sra` pairs). The store order (t[0..2],
then `flg = 0` in the `jal` delay slot) is source order. First try.
