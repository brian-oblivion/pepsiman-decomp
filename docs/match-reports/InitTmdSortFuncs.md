# InitTmdSortFuncs — MATCHED (57/57), round 6, runner charlie

> Renamed from `func_80020C14` on 2026-10-10 (tools/rename.py). Address 0x80020c14.

Unit `src/code_a0bc.c`. Fills the primitive-handler table `gTmdSortFuncs`:
columns 6 and 7 of each of the eight rows, plus row 3 and row 7 column 0
(the slots SetTmdSortMode later switches). Every handler is a function of
this unit, except SortTmdFastFT4, SortTmdFastGT3, SortTmdFastGT4 and
SortTmdWireGT3 (code_11dc4).

```c
typedef void (*PrimFunc)();
extern PrimFunc gTmdSortFuncs[8][8];

void InitTmdSortFuncs(void) {
    gTmdSortFuncs[0][6] = SortTmdFastF3;
    gTmdSortFuncs[0][7] = SortTmdDivF3;
    gTmdSortFuncs[1][6] = SortTmdFastG3;
    gTmdSortFuncs[1][7] = SortTmdDivG3;
    gTmdSortFuncs[2][6] = SortTmdFastFT3;
    gTmdSortFuncs[2][7] = SortTmdDivFT3;
    gTmdSortFuncs[3][0] = SortTmdWireGT3;
    gTmdSortFuncs[3][6] = SortTmdFastGT3;
    gTmdSortFuncs[3][7] = SortTmdDivGT3;
    gTmdSortFuncs[4][6] = SortTmdFastF4;
    gTmdSortFuncs[4][7] = SortTmdDivF4;
    gTmdSortFuncs[5][6] = SortTmdFastG4;
    gTmdSortFuncs[5][7] = SortTmdDivG4;
    gTmdSortFuncs[6][6] = SortTmdFastFT4;
    gTmdSortFuncs[6][7] = SortTmdDivFT4;
    gTmdSortFuncs[7][0] = SortTmdWireGT4;
    gTmdSortFuncs[7][6] = SortTmdFastGT4;
    gTmdSortFuncs[7][7] = SortTmdDivGT4;
}
```

## Attempts (1)

1. As above, the table declared with a known size (`[8][8]`) so cc1 builds
   its address once (`lui`/`addiu`) and stores at offsets: 57/57.

The handlers are game code (no Sony object covers them), but they look like
custom TMD primitive handlers in the style of libgs's GsFCALL tables: each
takes four register arguments and two on the stack and loops `count` times
over packed primitives.
