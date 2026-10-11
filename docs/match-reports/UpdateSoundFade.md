# UpdateSoundFade — MATCHED 204/204 (round 10, delta)

> Renamed from `func_80042208` on 2026-10-10 (tools/rename.py). Address 0x80042208.

Unit `src/code_31cec.c`. Per-frame music fade driver, stepped on
`gSoundFade`: state 0 arms a 50-step fade (`gSoundFadeTimer`) and resets the
volumes; state 1 fades the main sequence out (`gBgmVolume`, 100 down to
0 over the steps, clamped to 0..127) and closes it at the end; state 3
does the same for all ten sequences (`gSeVolume` for slots 2..9), stops
them at the end and falls into state 4; state 4 fades the CD mix
(`gCdVolume`, 80 down, clamped to 0..80) through SetCdVolume and, at
the end, calls StopCdTrack. A step count gone negative clears the
state.

```c
/* MATCHING: code_7d74 defines it with s16 arguments and result; this unit
 * passes and reads them as s32. */
s32 Lerp(s32 from, s32 to, s32 step, s32 steps);
void SetCdVolume(s32 vol);

extern s16 gCdVolume;
extern s16 gSoundFadeTimer;
extern s16 gSeVolume;

void UpdateSoundFade(void) {
    s16 i;

    switch (gSoundFade) {
        case 0:
            gSoundFadeTimer = 50;
            gSeVolume = 127;
            gBgmVolume = 127;
            gCdVolume = 80;
            break;
        case 1:
            gBgmVolume = Lerp(0, 100, --gSoundFadeTimer, 50);
            gBgmVolume = gBgmVolume < 0 ? 0 : gBgmVolume > 127 ? 127 : gBgmVolume;
            if (gSoundFadeTimer == 0) {
                SsSeqStop(gSeqIds[0]);
                SsSeqClose(gSeqIds[0]);
                gSoundFade = 0;
                gSeVolume = 127;
                gBgmVolume = 127;
            }
            SsSeqSetVol(gSeqIds[0], gBgmVolume, gBgmVolume);
            break;
        case 3:
            gBgmVolume = Lerp(0, 100, --gSoundFadeTimer, 50);
            gSeVolume = gBgmVolume = gBgmVolume < 0 ? 0 : gBgmVolume > 127 ? 127 : gBgmVolume;
            SsSeqSetVol(gSeqIds[0], gBgmVolume, gBgmVolume);
            for (i = 2; i < 10; i++) {
                SsSeqSetVol(gSeqIds[i], gSeVolume, gSeVolume);
            }
            if (gSoundFadeTimer == 0) {
                SsSeqStop(gSeqIds[0]);
                SsSeqClose(gSeqIds[0]);
                for (i = 2; i < 10; i++) {
                    SsSeqStop(gSeqIds[i]);
                }
                for (i = 0; i < 10; i++) {
                    SsSeqSetVol(gSeqIds[i], 0, 0);
                }
                gSeVolume = 127;
                gBgmVolume = 127;
                gSoundFade = 0;
                gCdVolume = 80;
            }
        case 4:
            gCdVolume = Lerp(0, 80, --gSoundFadeTimer, 50);
            gCdVolume = gCdVolume < 0 ? 0 : gCdVolume > 80 ? 80 : gCdVolume;
            SetCdVolume(gCdVolume);
            if (gSoundFadeTimer == 0) {
                gCdVolume = 80;
                StopCdTrack();
                gSoundFade = 0;
            }
            break;
    }
    if (gSoundFadeTimer < 0) {
        gSoundFade = 0;
    }
}
```

## Levers

| step | result |
| --- | --- |
| first write: each clamp as one nested-ternary assignment of the global (LEARNINGS, SelectToolSection), state 3 falling into state 4 with no `break`, `s16` loop counters | **204/204 on the first build** |

- `Lerp` is defined in code_7d74 as `s16 (s16, s16, u16, u16)`.
  This unit passes the step count sign-extended (`sll`/`sra` 16, so not
  `u16`) and re-extends the stored result before testing it, so its view
  is `s32` throughout, declared in the `.c` with a `MATCHING:` note.
- **SetCdVolume's parameter changed from `u8` to `s32`** (its own
  definition, in this unit). The call here passes `(s16)gCdVolume`
  sign-extended, which a `u8` prototype would turn into `andi 0xFF`;
  SetCdVolume only stores the value with `sb`, so `s32` keeps it 19/19
  (LEARNINGS: a `u8` parameter only ever stored with `sb` may be `s32`).
- State 3's `gSeVolume = gBgmVolume = clamp` stores `B18` then `B0C`
  (right to left) and passes the clamped value to the first
  `SsSeqSetVol` without a reload; the loop then reloads `gSeVolume`
  after each call.
