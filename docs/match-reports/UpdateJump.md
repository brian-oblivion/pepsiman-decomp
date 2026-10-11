# UpdateJump — MATCHED 134/134 (round 10, bravo)

> Renamed from `func_80026848` on 2026-10-11 (tools/rename.py). Address 0x80026848.

Unit `src/player.c`. The twin of UpdateJumpMode3 for another mode set:
on starting the unk390 sequence it also clears `sJumpDistance`, records
unk34C in `sJumpPeakY` and copies the position words unk348..unk350 into
`sJumpStart`; the sequence ends in mode 9, 0xB or 0xA instead of 8/9.

```c
extern s32 sJumpPeakY;
extern s32 sJumpDistance;
extern s32 sJumpStart[];

/* MATCHING: as UpdateJumpMode3 for the switch; the volatile read keeps the unk348 and
 * unk350 loads below the stores to sJumpDistance, sJumpStart[1] and sJumpPeakY. */
void UpdateJump(void) {
    s16 n;

    if (sGame.unk0 != 0) {
        if ((gGamePadTrig & 0x20) &&
            (sGame.unk34C >= sGame.unk3C0 || (sGame.unk3B8 == 1 && sGame.unk3BC == 1)) &&
            sGame.unk390 == 0) {
            sJumpDistance = 0;
            sJumpStart[1] = sGame.unk34C;
            sJumpPeakY = sGame.unk34C;
            sJumpStart[0] = *(volatile s32 *)&sGame.unk348;
            sJumpStart[2] = sGame.unk350;
            sGame.unk38F = 1;
            sGame.unk3BC = 0;
            sGame.unk390 = 1;
            sGame.unk392 = 0;
            sGame.unk39C = 0;
            func_80018CA4(&sGame);
            sGame.unk6 = 8;
            sGame.unk398 = 0x23;
            sGame.unk394 = sGame.unk34C;
            ScatterEffect4((VECTOR *)&sGame.unk348);
            PlaySe(0x17);
        }
        if (sGame.unk390 == 1) {
            n = ++sGame.unk392;
            if (n < 3) {
                sGame.unk398 = 0x19;
            } else if ((u16)(n - 4) < 3) {
                sGame.unk398 = 0x12;
            } else if (n >= 9) {
                sGame.unk398 = 0x14;
                sGame.unk390 = 2;
                sGame.unk6 = 9;
                sGame.unk3A0 = 2;
            } else if (gGamePad & 0x20) {
                sGame.unk398 = 0x14;
            } else {
                sGame.unk390 = 2;
                switch (n) {
                    case 3:
                        sGame.unk6 = 0xB;
                        sGame.unk3A0 = 0;
                        break;
                    case 7:
                        sGame.unk6 = 0xA;
                        sGame.unk3A0 = 1;
                        break;
                    default:
                        sGame.unk6 = 9;
                        sGame.unk3A0 = 2;
                        break;
                }
            }
        }
        if (sGame.unk390 == 2) {
            sGame.unk390 = 0;
        }
    }
}
```

## Levers

- UpdateJumpMode3's body with the constants changed matched everything but
  the first block's schedule: 126/134, three insertions and three deletions.
  Retail keeps `lw 0x348` / `lw 0x350` below the stores to `sJumpDistance`,
  `sJumpStart[1]` and `sJumpPeakY`; the build hoisted the 0x348 load above
  them and sank the `sJumpDistance` store.
- Negatives, all byte-identical at 126: four orders of those five
  statements; a chained `sJumpPeakY = sJumpStart[1] = ...`; a three-word
  struct copy (worse, 23); a `VECTOR *` view of `sJumpStart`; a
  `GameState *g` for the two reads (cse folds it); a volatile store to
  `sJumpDistance`. A bare `__asm__("")` after the stores fixed the order but
  moved the call's `move a0, s0` and the value register.
- **Lever: `*(volatile s32 *)&sGame.unk348` for the first load.** One
  volatile read is enough; the unk350 load follows it.

### Proposed learning

A struct load retail keeps below stores to unrelated globals that the
build hoists above them (sched2 only, every statement order identical): a
volatile read, `*(volatile s32 *)&s.field`, on the first load. Cheaper
than a barrier, which also moves unrelated argument setup. (UpdateJump)
