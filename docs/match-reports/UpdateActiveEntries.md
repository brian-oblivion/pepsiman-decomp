# UpdateActiveEntries — MATCHED 45/45 (round 6, alpha)

> Renamed from `func_8002BC4C` on 2026-10-11 (tools/rename.py). Address 0x8002bc4c.

Unit `src/object.c`. For each of the 200 Trap records that is live
(`unk36 != -1`) and whose SwitchBox record (`gSwitches[unk34]`) is marked 1,
calls `UpdateObjectEntry(&gObjectModels[unk36], record)`.

```c
void UpdateActiveEntries(void) {
    u32 i;

    for (i = 0; i < 200; i++) {
        if (sRecs48[i].unk36 != -1 && gSwitches[sRecs48[i].unk34].unk0 == 1) {
            UpdateObjectEntry(&gObjectModels[sRecs48[i].unk36], &sRecs48[i]);
        }
    }
}
```

First build. Indexing `sRecs48[i]` strength-reduces to the walking
`$s0` at the record start, as retail has it; the `u32` counter gives the
`sltiu`.

**Prototype fix.** `UpdateObjectEntry` (this unit, still asm) takes two
arguments: its asm keeps `$a1` in `$s2` and clears byte 0x40 through it.
hit_tool.c declared it locally with one (`ObjectModel *`). The prototype now
lives in `include/object.h` (additive), hit_tool.c includes that
header, its local prototype is gone, and its one call
(RefreshToolPreview) passes the second argument it was already leaving in `$a1`:
`UpdateObjectEntry(&gObjectModels[gToolSection], (Trap *)gTraps);`. The whole
image stays `OK` (cc1 reuses the `gTraps` address it had built for the
stores just before), and declcheck's LOCAL finding for it is gone.
