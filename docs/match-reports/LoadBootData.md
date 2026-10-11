# LoadBootData — MATCHED (105/105), round 7, runner bravo

> Renamed from `func_80013B38` on 2026-10-10 (tools/rename.py). Address 0x80013b38.

A three-step loader state machine on `gSubStep`: step 0 fills the
pointer tables `gCdLoadLoc` (entries of `sBootFileLoc`), `gCdLoadDest` (five
fixed RAM load addresses) and `gCdLoadSectors` (a copy of `sBootFileSectors`), sets
`gCdLoadCount = 5`, `gCdLoadState = 1` and moves to step 1; step 1 waits for
`gCdLoadState == 6`; step 2 sets up the loaded data (`LoadCommonModels`, the
pack at 0x8018D000, the TIM run at 0x8016D000, `DefineHudSprites`) and
returns 1. `main` tests the result as a signed byte.

```c
extern u16 gSubStep;
extern s32 sBootFileLoc[];
extern s32 sBootFileSectors[];
s32 LoadCommonModels(void);

/* MATCHING: model types the pack as its own PackEntry. */
void LoadTimPack(void *pack);

/* MATCHING: arity-ok, retail passes an argument the empty definition ignores. */
void SetBootReady(s32 arg);

s8 LoadBootData(void) {
    /* MATCHING: a byte, so the later constant 1 is not copied from it. */
    s8 ret;

    ret = 0;
    switch (gSubStep) {
        case 0:
            gCdLoadLoc[0] = (s32)&sBootFileLoc[0];
            gCdLoadDest[0] = 0x8014D000;
            gCdLoadLoc[1] = (s32)&sBootFileLoc[1];
            gCdLoadDest[1] = 0x8018D000;
            gCdLoadLoc[2] = (s32)&sBootFileLoc[2];
            gCdLoadDest[2] = 0x80123000;
            gCdLoadLoc[3] = (s32)&sBootFileLoc[3];
            gCdLoadDest[3] = 0x8016D000;
            gCdLoadLoc[4] = (s32)&sBootFileLoc[4];
            gCdLoadDest[4] = 0x80101000;
            gCdLoadCount = 5;
            gCdLoadSectors[0] = sBootFileSectors[0];
            gCdLoadSectors[1] = sBootFileSectors[1];
            gCdLoadSectors[2] = sBootFileSectors[2];
            gCdLoadSectors[3] = sBootFileSectors[3];
            gCdLoadSectors[4] = sBootFileSectors[4];
            gCdLoadState = 1;
            gSubStep = 1;
            break;
        case 1:
            if (gCdLoadState == 6) {
                SetBootReady(0);
                gSubStep++;
            }
            break;
        case 2:
            ret = 1;
            LoadCommonModels();
            LoadTimPack((void *)0x8018D000);
            LoadTimList((unsigned long *)(*(s32 *)0x8016D000 + 0x8016D000));
            DefineHudSprites();
            SetBootReady(1);
            break;
    }
    return ret;
}
```

## Attempts (6)

1. `s32` function and `s32 ret`: 104/105. The one difference is the
   argument of the last call: retail `li $a0, 1`, the build `move $a0, $s0`,
   because `ret` already holds 1 in `$s0` and the post-reload CSE reuses it.
2. `ret = 1` moved after the calls: still 104/105 (the copy is still made).
3. `SetBootReady`'s parameter as `u8`, then `s16`: still 104/105.
4. `ret` as `u8` or `s16` (with an `s32` or matching return type): 105/105
   in every combination. A narrower register mode is not reused for an
   `SImode` constant.
5. Final choice `s8` for both, since `main` sign-extends the result from 8
   bits (`sll 24` / `sra 24` after the `jal`): 105/105.

The five-word copy into `gCdLoadSectors` is five separate assignments
(LEARNINGS: more than four registers per batch, first word through `%lo`).

## Declarations

- `gCdLoadCount` (s16) and `gCdLoadState` (s8) moved from `src/sound.c` to
  `include/common.h` (additive).
- `LoadTimList` and `DefineHudSprites` (stage) added to
  `include/common.h` (additive).
- Per-unit views in `src/main.c` with `MATCHING:` notes:
  `LoadTimPack(void *)` (model types it with its local `PackEntry`)
  and `SetBootReady(s32)` (an empty definition retail calls with one
  argument).

### Proposed learning

- **A constant argument that retail rematerialises (`li $a0, 1`) while a
  saved register already holds the same value: that variable is narrower
  than `s32`** (here an `s8` result). With an `s32` local the post-reload
  CSE copies the register instead. (LoadBootData)
