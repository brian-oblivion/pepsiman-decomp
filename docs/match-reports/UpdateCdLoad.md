# UpdateCdLoad — MATCHED 116/116 (round 8, echo)

> Renamed from `func_80015584` on 2026-10-10 (tools/rename.py). Address 0x80015584.

Unit `src/main.c`. One step of a CD loading state machine in
`gCdLoadState`: seek to the next file's position (`CdControlF(CdlSetloc)`),
wait for the seek, `CdRead` its sectors into its buffer, wait for the read,
then advance to the next of `gCdLoadCount` files or finish in state 6. A
failed seek or read goes back to state 1 and counts a retry in
`gCdLoadRetries`. Returns the state.

```c
extern s16 gCdLoadIndex;
extern s32 gCdLoadReadSectors;
extern u8 gCdLoadRetries;

s8 UpdateCdLoad(void) {
    s32 r;

    switch (gCdLoadState) {
        case 0:
            break;
        case 1:
            CdControlF(CdlSetloc, (u_char *)gCdLoadLoc[gCdLoadIndex]);
            gCdLoadState++;
            break;
        case 2:
            r = CdSync(1, NULL);
            if (r == 2) {
                gCdLoadState++;
            } else if (r == 5) {
                gCdLoadState = 1;
                gCdLoadRetries++;
            }
            break;
        case 3:
            gCdLoadReadSectors = gCdLoadSectors[gCdLoadIndex] + 1;
            if (CdRead(gCdLoadReadSectors, (u_long *)gCdLoadDest[gCdLoadIndex], 0x80) == 0) {
                gCdLoadRetries++;
            } else {
                gCdLoadState++;
            }
            break;
        case 4:
            r = CdReadSync(1, NULL);
            if (r == 0) {
                gCdLoadState++;
            } else if (r == -1) {
                gCdLoadState = 1;
                gCdLoadRetries++;
            }
            break;
        case 5:
            if (++gCdLoadIndex >= gCdLoadCount) {
                gCdLoadState = 6;
                gCdLoadIndex = 0;
            } else {
                gCdLoadState = 1;
            }
            break;
    }
    return gCdLoadState;
}
```

Second build. The first had everything right except two layout points:

- The jump table covers 0..5 (`sltiu 6`), with entry 0 pointing at the
  exit: an empty `case 0: break;`. Without it the table is biased by -1.
- Retail shares only the final `sb` of every `gCdLoadState++` arm, jumping to
  a store that falls through to the exit; that store belongs to case 5's
  `gCdLoadState = 1`. Cross-jumping merges each arm's tail with the block that
  falls into the exit label, so that block must be the `= 1` arm: written
  `if (++x >= n) { = 6; ... } else { = 1; }`. With the `< n` arm first, the
  `= 6` arm fell into the exit, and cross-jumping merged whole
  `lbu`/`addiu`/`sb` increments across the cases instead.

### Proposed learning

- A switch whose arms each end in their own `lbu`/`addiu` and jump to ONE
  shared store just before the exit: the arm falling into the exit stores
  the same global, and cross-jumping merged only that store. Make that arm
  the last block in source (the `else` of its `if`).
