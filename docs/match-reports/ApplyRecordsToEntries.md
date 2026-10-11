# ApplyRecordsToEntries — MATCHED 38/38 (round 6, bravo)

> Renamed from `func_80033BF8` on 2026-10-11 (tools/rename.py). Address 0x80033bf8.

Unit `src/editor.c`. Walks the 200 Trap records at `gTraps`;
for each one whose `unk36` is not -1 and whose `unk34` equals the
halfword at 0x1E of `gEditorState` (`sTotals.unk1E`), calls
`UpdateObjectEntry(&gObjectModels[r->unk36], r)`.

```c
void ApplyRecordsToEntries(void) {
    u32 i;
    Trap *r;

    for (i = 0; i < 200; i++) {
        r = &((Trap *)gTraps)[i];
        if (r->unk36 != -1 && sTotals.unk1E == r->unk34) {
            UpdateObjectEntry(&gObjectModels[r->unk36], r);
        }
    }
}
```

## Levers

- **Record pointer taken per pass inside the loop**, `r = &tbl[i];`. With
  `r` set before the loop and bumped with `r++` at the end (0/38, two words
  long): strength reduction biased the pointer to `+0x34`, and the -1 was
  hoisted into a sixth saved register `s5`. Taking `&tbl[i]` in the body
  gives retail's record-start pointer in `s0`, the `li v0, -1` rebuilt
  every pass, and four saved registers: 38/38 on the second build.

## Declarations

- `EditorState` (unit-local) gained `u16 unk1E` split out of its leading pad
  (offsets unchanged; whole image OK).
- `UpdateObjectEntry` (defined in object, still asm) takes two arguments:
  its asm copies `$a1` into `$s2` at entry. The prototype `void
  UpdateObjectEntry(ObjectModel *rec, Trap *r);` was appended to
  `include/object.h` (additive). hit_tool.c's local one-argument
  declaration (its matched caller passes only the record) got a
  `MATCHING:` note so declcheck lists it as deliberate.

### Proposed learning

- **A loop over records whose pointer sits at the record start, with a
  constant compare rebuilt every pass: `r = &tbl[i];` inside the body.** A
  pointer bumped by `r++` lets loop optimisation bias it to a field and
  hoist the constant into a saved register. (ApplyRecordsToEntries)
