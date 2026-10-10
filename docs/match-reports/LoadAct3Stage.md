# LoadAct3Stage — MATCHED 264/264 (round 11, delta)

> Renamed from `func_80015CC8` on 2026-10-10 (tools/rename.py). Address 0x80015cc8.

Unit `src/main.c`. A stage loader stepped once per frame on
`gSubStep`: it requests the stage's sound bank (LoadVabBank, bank
2..6 for stages 2, 5, 8, 11, 14), reads the stage's two files from disc
through the `gCdLoadLoc` / `gCdLoadSectors` / `gCdLoadDest` request slots
into 0x8019D000, hands them to LoadTimList and LoadAct3Data, calls the
stage's overlay entry (func_800F6C60 .. func_800F377C) and finishes with
`gSubStep = 100`. Called from UpdateAct3LoadScreen.

The body is in `src/main.c`. The new declarations it needs:

```c
extern CdlLOC D_80073D00[];
extern s32 D_80073D24[];
/* ... one CdlLOC[] / s32[] pair per stage: D_80073DD8/D_80073DFC,
 * D_80073EB0/D_80073ED4, D_80073F88/D_80073FAC, D_80074060/D_80074084 */

/* MATCHING: code_31cec defines it with a u16 bank; this caller passes it
 * unextended, so its view is s32. */
s32 LoadVabBank(s32 bank);
void LoadStageOverlay(u16 sel);
```

## Levers

Three builds: 81 -> 263 -> 264.

- **One pair of extern arrays per stage, not one table of 0xD8-byte
  records.** The five stages' directories sit 0xD8 apart, so a record
  array `StageDir D_80073D00[]` looked right, but cc1 then CSEs the record
  base between the `&rec.loc[k]` address and the `rec.size[k]` load and
  drops a `lui` per arm (52 insertions / 52 deletions). Retail rebuilds
  `lui` for each symbol: separate `CdlLOC[]` and `s32[]` objects.
  Case 6 reads index 1 of the same arrays (`&D_80073D00[1]`,
  `D_80073D24[1]`), which splat named D_80073D04 / D_80073D28.
- **The bank is passed with no `andi 0xFFFF`: the caller's view of
  LoadVabBank takes `s32`** (LEARNINGS "an argument passed unextended
  though the callee narrows it"). The local holding it is `s32` and is
  left unset on the default path, as retail jumps to the call with `$a0`
  untouched; `-Wall`'s "might be used uninitialized" is baselined.
- Each arm's `gSubStep++` merges into one store at the tail with the
  value in `$v0`; case 1's `addu v0, v0, v1` reuses the 1 from the
  `== 1` test. Plain C gives both.

### Proposed learning

- **Per-arm `lui` for addresses a fixed stride apart: separate extern
  arrays, not a record table.** A record array lets cc1 share the record
  base between an address and a load in the same arm. (LoadAct3Stage)
