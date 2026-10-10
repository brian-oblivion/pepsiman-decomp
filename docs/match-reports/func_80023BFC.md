# func_80023BFC — MATCHED 91/91 (round 7, delta)

Unit `src/code_13068.c`. On flag 0x40 of `D_800AC860` (outside mode
0xE): ends a pending low-nibble state of 0x3D0, sets the cap 0x3C0 to
30000, and once 0x34C reaches `D_800AC858[0] + 100` (a goal line) runs
the goal: `D_80095858` to 2 if clear, `D_800958EC = 1`, 0x3A6 += 2, and
past +1000 mode 12.

```c
/* MATCHING: non-void with no return keeps the last beqz delay slot a nop. */
s32 func_80023BFC(void) {
    s32 unused[2];

    if ((D_800AC860 & 0x40) && sGame.unk6 != 0xE) {
        if (sGame.unk3D0 & 0xF) {
            sGame.unk3CA = 1;
            if ((sGame.unk3D0 & 0xF) != 3) {
                sGame.unk398 = 20;
                sGame.unk3D0 &= 0xF0;
            }
            func_800287C0();
            sGame.unk0 = 0;
        }
        sGame.unk3C0 = 30000;
        if (sGame.unk34C >= D_800AC858[0] + 100) {
            if (D_80095858 == 0) {
                D_80095858 = 2;
            }
            D_800958EC = 1;
            sGame.unk0 = 0;
            sGame.unk3A6 += 2;
            func_800287C0();
            sGame.unk374 = 0;
            if (sGame.unk34C > D_800AC858[0] + 0x8C && (sGame.unk3D0 & 0xF) == 3) {
                sGame.unk3D0 &= 0xF0;
            }
            sGame.unk3CA = 1;
            if (sGame.unk34C > D_800AC858[0] + 1000) {
                sGame.unk6 = 12;
                sGame.unk3A6 = 0;
                sGame.unk3D0 &= 0xF0;
            }
        }
    }
}
```

GameState gained `unk374`, `unk3A6`, `unk3CA` (all s16).

## Declarations

- `D_800AC860`: `lui $v0` / `lw $v0` in one register, a scalar;
  `extern s32` in the `.c` (only this unit names it).
- `D_80095858` and `func_800287C0` were declared locally in
  `src/code_308ec.c` (not a round-7 unit). Both moved to
  `include/common.h` (ADDITIVE there) and deleted from code_308ec, so
  declcheck sees one declaration.

## Path

- First body as `void`: equal length, the last `beqz`'s delay slot held
  `li $v0, 0xC` where retail has a `nop`.
- `s32` with no `return`: 91/91, oracle OK. Second build.
- `s32 unused[2];` predicted from the frame (saved registers at sp+0x18 in
  a 0x28 frame with no stack locals) and right first time.

## Levers

- Non-void with no return (func_800283A0's lever) on the last forward
  branch. Adds a baselinable warning, plus the unused-variable one.
