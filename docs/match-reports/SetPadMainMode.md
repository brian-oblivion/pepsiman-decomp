# SetPadMainMode — MATCHED 33/33 (round 4, delta)

> Renamed from `func_800153CC` on 2026-10-10 (tools/rename.py). Address 0x800153cc.

Unit `src/main.c`. Waits up to ten frames for pad 0 to report state 6
(`PadGetState`), counting in `gLoopI`; once it does, sets the pad's main
mode to the low bit of the argument and returns.

```c
extern s32 gLoopI;

void SetPadMainMode(s32 mode) {
    mode &= 1;
    gLoopI = 0;
    do {
        VSync(0);
        if (PadGetState(0) == 6) {
            PadSetMainMode(0, mode, 0);
            return;
        }
        gLoopI++;
    } while (gLoopI < 10);
}
```

Second build. `gLoopI` is a plain scalar (its load is `lui $v0` / `lw
$v0`, the same register). The first build passed `mode & 1` in the call:
cc1 kept `mode` in `$s1` and did the `andi` at the call. Retail does the
`andi` into `$s0` in the prologue, before `$s1` is saved, so the mask is a
statement of its own at the top (`mode &= 1;`). That also swaps which saved
register holds the constant 6.

### Proposed learning

A parameter masked in the prologue (`andi $s0, $a0, 1` among the register
saves) and used only later inside a loop: the source masked it once at the
top, `mode &= 1;`. Masking at the use keeps the raw argument in the saved
register.
