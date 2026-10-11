# EditorPlaceBox — MATCHED 180/180 (round 9, alpha)

> Renamed from `func_80030278` on 2026-10-11 (tools/rename.py). Address 0x80030278.

Unit `src/editor.c`. Copies three box sizes from rodata
`D_800110EC` into a local, clears every used 0x5C-byte record of
`gSwitches` to 0 while counting them into `sTotals.unk16` (error 8 when
all 200 are used), moves `sTotals.unk12` to a free record if the current
one is taken, sizes the box there with `SetBoxCentred` (size picked by
`sTotals.unk14`), stores the game position and marks it -1; on flag bit 5
places it (error 7 with no current entry) and moves the slot on to the next
free record (error 8 when none). Second build matched.

## Shapes read from retail

- **A 12-byte local filled with `lwl`/`lwr` from a rodata symbol: a
  struct copy** of a halfword-aligned struct, `sizes = D_800110EC;` with
  `Sizes3 { Size2 s[3]; }` (LEARNINGS, StartActResults).
- The two searches end in identical error-8 blocks that cross-jumping
  merges into the second one, as retail shows.
- The only first-build residue: the flag test's `beqz` delay slot (`nop`
  in retail, filled in the build). Non-void with no return value closed it.

## Declarations

- `Totals28` (unit-local) gains `u16 unk16`.
- New unit-local `EditorSwitch` view (`#define sRecs5C`), `Size2`/`Sizes3`,
  `extern Sizes3 D_800110EC` (the rodata stays `INCLUDE_RODATA`), and a
  per-unit view `void SetBoxCentred(EditorSwitch *b, s16 w, s16 d)` (MATCHING
  note; object defines it on its `Box4`).

```c
s32 EditorPlaceBox(void) {
    Sizes3 sizes;
    u32 i;

    /* MATCHING: non-void with no return value keeps the flag test's delay
     * slot a nop. */
    sizes = D_800110EC;
    sTotals.unk16 = 0;
    for (i = 0; i < 200; i++) {
        if (sRecs5C[i].unk0 != -1) {
            sRecs5C[i].unk0 = 0;
            sTotals.unk16++;
        }
    }
    if (sTotals.unk16 == 200) {
        gToolMode = 8;
        return;
    }
    if (sRecs5C[sTotals.unk12].unk0 != -1) {
        for (i = 0; i < 200; i++) {
            if (sRecs5C[i].unk0 == -1) {
                break;
            }
        }
        if (i == 200) {
            gToolMode = 8;
            return;
        }
        sTotals.unk12 = i;
    }
    SetBoxCentred(&sRecs5C[sTotals.unk12], sizes.s[sTotals.unk14].w, sizes.s[sTotals.unk14].d);
    sRecs5C[sTotals.unk12].unk50[0] = sGameSave.unk348[0];
    sRecs5C[sTotals.unk12].unk50[1] = sGameSave.unk348[1];
    sRecs5C[sTotals.unk12].unk50[2] = sGameSave.unk348[2];
    sRecs5C[sTotals.unk12].unk0 = -1;
    if (gPadTrig & 0x20) {
        if (D_80095824 == -1) {
            gToolMode = 7;
            return;
        }
        sRecs5C[sTotals.unk12].unk0 = 0;
        for (i = 0; i < 200; i++) {
            if (sRecs5C[i].unk0 == -1) {
                break;
            }
        }
        if (i == 200) {
            gToolMode = 8;
            return;
        }
        sTotals.unk12 = i;
    }
}
```
