# FindSave1 — MATCHED 38/38 (round 5, echo)

> Renamed from `func_800384DC` on 2026-10-10 (tools/rename.py). Address 0x800384dc.

Unit `src/card.c`. Points `gCardFileName` at the name
`"BISLPS-12345PEPTOOL"` (rodata `D_800119B4`) and looks for it among the
first `gCardFileCount` 0x18-byte records of `gCardFiles`, comparing 0x14 bytes
with `CompareBytes`. Returns 0 if found, -1 if not. `FindSave2` is the
same with `"BISLPS-67890PEPTOOL"`. No direct caller in the asm.

```c
s16 FindSave1(void) {
    s16 i;

    gCardFileName = "BISLPS-12345PEPTOOL";
    for (i = 0; i < gCardFileCount; i++) {
        if (CompareBytes((u8 *)gCardFiles[i].name, (u8 *)gCardFileName, 0x14) == 0) {
            return 0;
        }
    }
    return -1;
}
```

## Notes

- **The string is a literal.** `D_800119B4` lived only in this function's
  own `.s` (splat migrated it there), so the literal is now its definition,
  as in `WaitMovieFrameDecoded`. cc1 puts it in `.rodata` at the function's place,
  right after `INCLUDE_RODATA(sToolCardFile)`, and the whole image verifies.
- **The callee's return type was `s32`** (first build 20/38, one word long):
  with `s16 CompareBytes(...)` cc1 sign-extends the result (`sll 16`)
  before the `bnez`. Retail tests `$v0` directly. `CompareBytes` is defined
  in this unit and returns only constants, so its definition became `s32`
  and still matches 20/20; both of its callers are in this unit.

### Proposed learning

A `bnez $v0` right after a call, with no `sll`/`sra` between: the callee
returns `s32` in this unit's view. When the callee is defined in the same
unit and returns only constants, change its definition (the width does not
change its own code). (FindSave1, CompareBytes)

`Rec18` became the file name plus a `BlockCount` union in round 5 (see
`ScanCardFiles.md`); the code above uses the new member names.
