# InitPads — MATCHED 32/32 (round 4, delta)

> Renamed from `func_8001534C` on 2026-10-10 (tools/rename.py). Address 0x8001534c.

Unit `src/main.c`. Starts the pads on the two buffers `gPadBuf` and
`D_80095BE8` (through `StartPads`), then clears six words of each of
three word arrays.

```c
void StartPads(u8 *a, u8 *b);

extern u8 gPadBuf[];
extern u8 D_80095BE8[];
extern s32 gPadPrev[2];
extern s32 gPadPressed[2];
extern s32 gPadHeld[2];

void InitPads(void) {
    s16 i;

    StartPads(gPadBuf, D_80095BE8);
    for (i = 0; i < 6; i++) {
        gPadPrev[i] = 0;
        gPadPressed[i] = 0;
        gPadHeld[i] = 0;
    }
}
```

Second build. The loop body (`sll 16` / `sra 14`, the `s16` counter
re-extended for `slti 6`) came out right first time. The first build
declared the three word arrays `[]` and got each base split over two
registers (`lui $v0` / `addiu $t0, $v0, %lo`); `[6]` (24 bytes) splits the
same way. Retail builds each base in one register (`lui $t0` / `addiu $t0,
$t0`), cc1's `la` of an object of at most 8 bytes, so they are declared
`[2]`. Their real extent is not known: `gPadHeld` is only 8 bytes below
`gPadPrev` in the symbol map, yet the loop writes 24 bytes from each.
The two buffers passed as arguments are `[]`: an address that goes straight
into an argument register is built in that register either way.

### Proposed learning

The `la` lever (LEARNINGS "An ADDRESS built in one register") also holds
for loop bases indexed past the declared size: declare `[2]` of `s32` (8
bytes), not the true count, when retail builds the base in one register.
`[6]` splits.
