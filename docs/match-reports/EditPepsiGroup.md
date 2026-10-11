# EditPepsiGroup — MATCHED 11/11 (round 1, alpha)

> Renamed from `func_80033B08` on 2026-10-11 (tools/rename.py). Address 0x80033b08.

Unit `src/editor.c`. Calls `StepMenuCursor`, then copies the halfword
`gEditValue` into the first halfword of `D_80095B4C`. First build matched.

```c
extern u16 gEditValue;
extern u16 D_80095B4C[];
void StepMenuCursor(void);

void EditPepsiGroup(void) {
    StepMenuCursor();
    D_80095B4C[0] = gEditValue;
}
```

## Extern shapes, read from retail

- `gEditValue`: a `u16` scalar, not in `config/gp-symbols.txt`. Retail
  loads it `lui $v1; lhu $v1,%lo(..)($v1)` (the assembler's load expansion
  reuses the destination) and other units store it through `lui $at`.
- `D_80095B4C`: the store is `lui $v0; sh $v1,%lo(..)($v0)`, a compiler
  split, so it is an array (`u16 []`). Measured: declared as a `u16` scalar
  the store becomes `lui $at` and the function scores 7/11. Neighbouring
  halfwords `D_80095B50..5A` are read in `EditorEditSpriteValues`, so this is likely
  the head of a struct or table.
- `StepMenuCursor` is prototyped `void (void)` in the unit; its return value
  is unused here, its arguments are unknown.
