# LoadSoundData — MATCHED 131/131 (round 10, delta)

> Renamed from `func_800426A4` on 2026-10-10 (tools/rename.py). Address 0x800426a4.

Unit `src/sound.c`. Sound start-up state machine stepped on
`gSubStep`, returning 1 once finished. State 0 queues two CD loads for
the loader (positions from the location table `D_8007B69C`, entries 0 and
2, to 0x80101000 and 0x8016D000, sizes from `D_8007B6CC`); state 1 waits
for the loader's step byte `gCdLoadState` to reach 6 and opens bank 0
(LoadVab); state 2 queues one more load (entry 3); state 3 opens
bank 1, opens the sequences (InitSeSeqs) and records bank 1 as current
(`gVabBank`); state 4 sets volumes and reverb (InitReverb's body,
written out) and returns 1.

```c
extern s32 D_8007B69C[];
extern s32 D_8007B6CC[];
extern u16 gVabBank;
void LoadVab(s16 arg0);

s32 LoadSoundData(void) {
    s32 ret = 0;

    switch (gSubStep) {
        case 0:
            gCdLoadLoc[0] = (s32)&D_8007B69C[0];
            gCdLoadDest[0] = 0x80101000;
            gCdLoadLoc[1] = (s32)&D_8007B69C[2];
            gCdLoadDest[1] = 0x8016D000;
            gCdLoadCount = 2;
            gCdLoadState = 1;
            gCdLoadSectors[0] = D_8007B6CC[4];
            gCdLoadSectors[1] = D_8007B6CC[6];
            gSubStep++;
            break;
        case 1:
            if (gCdLoadState == 6) {
                LoadVab(0);
                gSubStep++;
            }
            break;
        case 2:
            gCdLoadLoc[0] = (s32)&D_8007B69C[3];
            gCdLoadDest[0] = 0x8016D000;
            gCdLoadCount = 1;
            gCdLoadState = 1;
            gCdLoadSectors[0] = D_8007B6CC[7];
            gSubStep++;
            break;
        case 3:
            if (gCdLoadState == 6) {
                LoadVab(1);
                InitSeSeqs();
                gVabBank = 1;
                gSubStep++;
            }
            break;
        case 4:
            SsSetMVol(0, 0);
            SsSetRVol(0, 0);
            SsUtSetReverbType(SS_REV_TYPE_STUDIO_B);
            VSync(0);
            SsSetMVol(127, 127);
            SsSetRVol(120, 120);
            SsUtReverbOn();
            SsUtSetReverbDepth(48, 48);
            ret = 1;
            break;
    }
    return ret;
}
```

## Levers

| step | result |
| --- | --- |
| loader stores grouped by table (`gCdLoadLoc[0]`, `[1]`, then `gCdLoadDest[0]`, `[1]`) | 110/131, only state 0's store order and registers off |
| **stores paired per load: `gCdLoadLoc[0]`, `gCdLoadDest[0]`, `gCdLoadLoc[1]`, `gCdLoadDest[1]`** | **131/131** |

- Independent global-table stores come out in source order (LEARNINGS,
  local structs); the same holds for these array stores.
- `D_8007B69C` is the start of the CD location table that LoadVabBank
  reaches as `D_8007B6A4` (= entry 2): splat split one table at each
  address an instruction names. Entry `k`'s size is `D_8007B6CC[k + 4]`.
  A head-level merge of the two names (one `CdlLOC` table) would make
  LoadVabBank read `&tbl[bank + 2]`; not done here (no renames this
  round).
- State 4 is InitReverb's body. That function is defined later in the
  unit, so 2.8.1 cannot have inlined it; writing the calls out matches.
  A shared `static __inline__` helper would also be byte-identical.
- The `ret` local in `$s0`, copied to `$v0` in each `j` delay slot, is the
  usual single `s32 ret = 0;` (LEARNINGS).

### Proposed learning

- **A pair of loader stores interleaved (`A[0]`, `B[0]`, `A[1]`, `B[1]`):
  the source wrote them paired**, not grouped by table. (LoadSoundData)
