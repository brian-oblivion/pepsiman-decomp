# DumpMemory — MATCHED 62/62 (round 4, delta)

> Renamed from `func_800149D0` on 2026-10-10 (tools/rename.py). Address 0x800149d0.

Unit `src/main.c`. A debug hex dump on the font layer: prints a header,
the start address, then 16 rows of 8 bytes, each byte with its own format
and each row ended by a newline and a tab. `sDumpOffset` (an offset, cleared
on entry) is added to the base.

```c
extern char sNewline[];
extern char sTab[];
extern char sWhiteLineText[];
extern char sHexAddrFmt[];
extern char sHexByteFmt[];
extern s32 sDumpOffset;

void DumpMemory(u8 *base) {
    u8 *p;
    s32 row;
    s32 col;
    s16 i;
    s16 j;

    sDumpOffset = 0;
    FntPrint(sWhiteLineText);
    p = base + sDumpOffset;
    FntPrint(sHexAddrFmt, p);
    for (i = 0; i < 1; i++) {
        FntPrint(sTab);
    }
    for (row = 0; row < 16; row++) {
        for (col = 0; col < 8; col++) {
            FntPrint(sHexByteFmt, *p);
            p++;
        }
        FntPrint(sNewline);
        for (j = 0; j < 1; j++) {
            FntPrint(sTab);
        }
    }
}
```

Third build. The two one-pass `s16` loops printing `sTab` are
`PrintTabs(1)` written out (same `sll 16` / `blez` test); a call would
be a `jal`. The strings are `.sdata` reached `lui`/`addiu`, so `[]`.

- Build 1, one `s16 i` for both one-pass loops: a sixth saved register and
  a 0x30 frame. Retail shares `$s2` between the first loop's counter and
  `row`, and `$s0` between the second's and `col`: two separate counters
  (`i`, `j`) let the allocator pair each with a variable whose life does
  not overlap. 59/62.
- Build 2, `FntPrint(sHexByteFmt, *p++)`: the load and increment come before
  the format's `addiu`. Retail builds the format first; the increment as
  its own statement after the call gives that. 62/62.

### Proposed learning

- One loop counter reused by two disjoint loops costs a saved register
  when retail shares each loop's register with a different variable: give
  each loop its own counter.
- `f(fmt, *p++)` loads and bumps `p` before `fmt`'s `addiu`; `f(fmt, *p);
  p++;` keeps the argument order.
