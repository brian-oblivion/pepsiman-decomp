# ResetClearColor — MATCHED (12/12), round 1, runner charlie

> Renamed from `func_80014CF0` on 2026-10-10 (tools/rename.py). Address 0x80014cf0.

Leaf, no arguments: stores 2 to one word global and clears three byte globals.

```c
void ResetClearColor(void) {
    /* MATCHING: retail reserves an 8-byte frame it never touches. */
    s32 unused[2];

    gVSyncMode = 2;
    gClearB = 0;
    gClearG = 0;
    gClearR = 0;
}
```

## Externs (declared in `src/main.c`)

All four lie in `.sdata` but are not in `config/gp-symbols.txt`, so retail
reaches each with `lui $at` / `%lo`: a plain complete `extern` gives exactly that.
`gVSyncMode` is `s32` (`sw`); the other three are `u8` (`sb $zero`, never loaded
here, so the sign is unknown).

## Lever

The frame. Retail opens with `addiu $sp,$sp,-8` and closes with
`addiu $sp,$sp,8` and uses no stack. Without a local, GCC 2.8.1 emits no frame
and the function is one word short. An unused `s32 unused[2];` reserves exactly
the 8 bytes; first attempt with it was byte-exact.
