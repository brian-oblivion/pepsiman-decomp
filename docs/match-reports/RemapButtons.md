# RemapButtons — MATCHED (97/97), round 8, runner charlie

> Renamed from `func_8003DE34` on 2026-10-10 (tools/rename.py). Address 0x8003de34.

Unit `src/stage.c`. Copies the two button words `gPadTrig` and
`gPad` into `gGamePadTrig` and `gGamePad`, clears bits 5..7 of each,
and moves those three bits to positions `5 + row[0..2]`, where `row` is one
4-byte row of a 16-byte rodata table (`D_80011EB0`, which splat places in
card's rodata) chosen by the `s8` `gButtonConfig`. A button remap.

```c
typedef struct {
    s8 b[4][4];
} Bytes16;

extern Bytes16 D_80011EB0;
extern s8 gButtonConfig;
extern s32 gGamePad; /* gGamePadTrig: common.h */

void RemapButtons(void) {
    Bytes16 tbl;
    s32 k;
    u32 a;
    u32 b;

    tbl = D_80011EB0;
    a = gPadTrig;
    b = gPad;
    k = gButtonConfig;
    gGamePadTrig = a;
    gGamePad = b;
    gGamePadTrig = a & ~0xE0;
    gGamePad = b & ~0xE0;
    gGamePadTrig |= ((a & 0x20) >> 5) << (tbl.b[k][0] + 5);
    gGamePadTrig |= ((a & 0x40) >> 6) << (tbl.b[k][1] + 5);
    gGamePadTrig |= ((a & 0x80) >> 7) << (tbl.b[k][2] + 5);
    gGamePad |= ((b & 0x20) >> 5) << (tbl.b[k][0] + 5);
    gGamePad |= ((b & 0x40) >> 6) << (tbl.b[k][1] + 5);
    gGamePad |= ((b & 0x80) >> 7) << (tbl.b[k][2] + 5);
}
```

## Levers

- **The 16-byte local** is a struct copy from an extern struct (three
  `lwl`/`lwr` words, then the fourth): `tbl = D_80011EB0;`. The table stays
  `INCLUDE_RODATA` in card and is only referenced here.
- **`srl`, not `sra`, after the masks: the copies are `u32`** (88/97 with
  `s32`, which gives `sra`).
- **The row index in its own local, indexed at every use.** A `s8 *row =
  tbl.b[gButtonConfig];` pointer (before or after the stores) was 94/97: the
  `sll`/`addu` of the row address rose above `andi 0x20`. The `lb` of
  `row[0]` and the `lbu`+`sll`/`sra` of `row[1]`/`row[2]` came out the same
  in both spellings.

- `gGamePadTrig` is now used by two units (player and this one), so its
  declaration moved from player.c into `include/common.h` (additive in
  the header; the one-line local extern in player.c removed).

### Proposed learning

- **A local's row address computed after an unrelated `andi`: index the
  local array with an index local at each use**, `tbl.b[k][0]`, not through
  a row pointer. (RemapButtons)
