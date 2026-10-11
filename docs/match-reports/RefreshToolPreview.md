# RefreshToolPreview — MATCHED 35/35 (round 12, charlie)

> Renamed from `func_80037114` on 2026-10-11 (tools/rename.py). Address 0x80037114.

Unit `src/hit_tool.c`. Resets part of the play state around the current
entry: raises `D_80095774` for the duration, calls `ResetCollision`, clears
words 0, 1, 2, 11, 12 and halfwords 12-14 of `gTraps`, then
`UpdateObjectEntry` on the current `gObjectModels` record, `ReactToHits` on the
game state and `func_80029838`.

## Rounds

- Round 4 (charlie): STALL, length exact, 25/35, first diff 0x80037130.
  Register only: retail kept `gTraps`'s base in `$a1`, the build in
  `$v0`; one permuter search (110646 iterations, rc 124) found nothing.
- Round 6: matching UpdateActiveEntries (object) gave `UpdateObjectEntry` its
  real prototype, `s32 UpdateObjectEntry(ObjectModel *rec, Trap *r)`, and the
  `#ifdef NON_MATCHING` body in this unit was updated to pass
  `(Trap *)gTraps` as the second argument. Nobody rebuilt it live.
- Round 12 (charlie): the body as it stood under `#ifdef NON_MATCHING`,
  built live, is **35/35, `insertions 0 / deletions 0`, `OK: build matches
  retail`** with no edit to its statements. The `$a1` was the call's second
  argument: cc1 builds the base in `$a1` from the start because it is
  passed to `UpdateObjectEntry`. Only the wrapper was removed.

```c
void RefreshToolPreview(void) {
    D_80095774 = 1;
    ResetCollision();
    gTraps[0] = 0;
    gTraps[1] = 0;
    gTraps[2] = 0;
    ((s16 *)gTraps)[12] = 0;
    ((s16 *)gTraps)[13] = 0;
    ((s16 *)gTraps)[14] = 0;
    gTraps[11] = 0;
    gTraps[12] = 0;
    UpdateObjectEntry(&gObjectModels[gToolSection], (Trap *)gTraps);
    ReactToHits(gPlayer);
    func_80029838();
    D_80095774 = 0;
}
```

Builds: 2 this round.

### Proposed learning

- **A register-only stall whose odd register is an argument register
  (`$a1`..`$a3`) holding a value at a call: check the callee's argument
  count first.** A base built in `$a1` across stores was a second argument
  the round 4 prototype did not have. (RefreshToolPreview)
- **A preserved `NON_MATCHING` body can match after a header change
  elsewhere.** A round that changes a prototype should rebuild live every
  stalled caller of that function. (RefreshToolPreview, via UpdateActiveEntries's
  round 6 prototype)
