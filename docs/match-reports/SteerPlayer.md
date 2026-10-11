# SteerPlayer — MATCHED 52/52 (round 5, delta)

> Renamed from `func_80023764` on 2026-10-11 (tools/rename.py). Address 0x80023764.

Unit `src/player.c`. While the game-state flag byte (`sGame.unk0`) is
set, picks a per-frame update by the byte `gPadType`: for 0x73 and 0x41,
pad bit 0x8000 of `gPad` calls `ApplySteer(3, 16, 15)`, bit 0x2000
calls `ApplySteer(2, 16, 15)`, otherwise `RecentreSteer()`; any other
value calls `RecentreSteer()`. If the flag is (or became) clear, copies the
camera angle `D_800A7680[0].vy` into `D_8009EEF8`.

```c
extern u8 gPadType;
extern s32 D_8009EEF8[];
extern SVECTOR D_800A7680[];   /* unit */
extern s32 gPad;         /* unit */

void ApplySteer(u8 mode, u16 a, u16 b);
void RecentreSteer(void);

void SteerPlayer(void) {
    if (sGame.unk0 != 0) {
        switch (gPadType) {
            case 0x73:
                if (gPad & 0x8000) {
                    ApplySteer(3, 16, 15);
                } else if (gPad & 0x2000) {
                    ApplySteer(2, 16, 15);
                } else {
                    RecentreSteer();
                }
                break;
            case 0x41:
                /* the same body again */
                ...
                break;
            default:
                RecentreSteer();
                break;
        }
        if (sGame.unk0 != 0) {
            return;
        }
    }
    D_8009EEF8[0] = D_800A7680[0].vy;
}
```

## Levers

- **Case bodies are laid out in source order, not by the compare order.**
  Retail compares 0x41 first but places the 0x73 body first (the `bne
  0x73` falls into it). Written 0x41-then-0x73 the build adds a `j` to the
  default and swaps the bodies; 0x73 first, 51/52.
- **The first `beqz` jumps straight to the store: the store follows an
  early `return` inside the `if`.** With `if (unk0 != 0) {...} if (unk0 ==
  0) store;` the first branch lands on the re-test (51/52, word 3).
  `if (unk0 != 0) { ...; if (unk0 != 0) return; } store;` gives the
  retail target, and the `nop` in the re-test's `bnez` slot came with it,
  as `void` (no non-void lever needed).
- The two identical case bodies are not merged in source: retail repeats
  them, sharing only the `ApplySteer` call tail (cross-jumping).

## Declarations

- `D_800A7682` in the retail asm is `D_800A7680[0].vy` of the unit's
  existing `SVECTOR D_800A7680[]`.
- `ApplySteer`'s prototype (`u8, u16, u16`) is a guess from its
  prologue (`andi 0xFF` on `$a0`, `andi 0xFFFF` on `$a1`); the caller only
  passes constants, so it does not affect this match. Revisit when that
  function is decompiled.
