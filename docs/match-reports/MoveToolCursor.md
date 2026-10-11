# MoveToolCursor — MATCHED 144/144 (round 9, echo)

> Renamed from `func_80034388` on 2026-10-11 (tools/rename.py). Address 0x80034388.

Unit `src/code_24748.c`. Steps by `gToolStep` either the dispatch object's
two sizes (`gToolQuadW`/`gToolQuadH`, state 1 of `gToolQuadStage`, each clamped
to 0..1000) or the game state's x and z (otherwise), then y on
`D_80095958`'s bits, and on flag bit 4 leaves state 1 with error code 1.

```c
extern s32 D_80095958; /**< a pad word; bits 12 and 14 step the position's y */

void MoveToolCursor(void) {
    s32 flags;

    if (gToolQuadStage == 1) {
        flags = gPadTrig;
        if (flags & 0x1000) {
            gToolQuadH += gToolStep;
        }
        if (flags & 0x4000) {
            gToolQuadH -= gToolStep;
        }
        if (flags & 0x2000) {
            gToolQuadW += gToolStep;
        }
        if (flags & 0x8000) {
            gToolQuadW -= gToolStep;
        }
        gToolQuadW = gToolQuadW < 0 ? 0 : gToolQuadW > 1000 ? 1000 : gToolQuadW;
        gToolQuadH = gToolQuadH < 0 ? 0 : gToolQuadH > 1000 ? 1000 : gToolQuadH;
    } else {
        if (gPadTrig & 0x1000) {
            sGamePos.unk350 -= gToolStep;
        }
        if (gPadTrig & 0x4000) {
            sGamePos.unk350 += gToolStep;
        }
        if (gPadTrig & 0x2000) {
            sGamePos.unk348 -= gToolStep;
        }
        if (gPadTrig & 0x8000) {
            sGamePos.unk348 += gToolStep;
        }
    }
    if (D_80095958 & 0x1000) {
        sGamePos.unk34C -= gToolStep;
    }
    if (D_80095958 & 0x4000) {
        sGamePos.unk34C += gToolStep;
    }
    if (gPadTrig & 0x10) {
        gToolQuadStage = 0;
        gToolMode = 1;
    }
}
```

## Levers

First build matched. The state-1 arm reads `gPadTrig` once into a local
(`$a0`, tests in delay slots); the other arm reloads it for every test, so
it reads the global each time. The clamps are SelectToolSection's nested
ternary.

## Declarations

`D_80095958` is declared in this `.c` (code_1dc24 declares it in its own);
declcheck lists it LOCAL. It belongs in `common.h` at merge.
