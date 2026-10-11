# PrintEditorCaption — MATCHED 268/268 (round 10, echo)

> Renamed from `func_8002F270` on 2026-10-11 (tools/rename.py). Address 0x8002f270.

Unit `src/editor.c`. The tool mode's caption printer: while
`gEditorInfo` is 1, prints with `FntPrint` the state of the current tool
mode (`sTotals.unk3`): mode 0 the selected point and the entry's point
count, mode 1 the selected switch, mode 2 the selected EditorTrap record (model
or "OPTIONS", switch, type, act), mode 3 the selected StageSprite sprite (model,
switch, hit on/off from bit 7 of `unk26`, the act kind from its low six
bits, the type in `unk27`, the sound-effect voice in `unk28` through a
five-way jump table); then draws the frame with `SortSemiQuad(0x7C21,
...)`. Fourth build matched (the strings are rodata and `.sdata`, all
referenced by symbol).

## Shapes read from retail

- After the mode-2 `if`/`else` join and the mode-3 ON/OFF join, retail
  rebuilds both bases with `lui`/`addiu` and reads `sTotals.unk28` as
  `%lo(D_80095B50)`: cse loses the saved bases at a join. Plain
  `sTotals.unk28` gives it; no other spelling needed.
- The selection of the ON/OFF string jumps on `bnez` to " ON  / ": the
  clear-bit arm written first, `if (!(x & 0x80)) FntPrint(OFF); else
  FntPrint(ON);`. The other way round gives `beqz`. Each arm's call is
  merged at the join by cross-jumping, as are the act-kind arms.
- **After the mode-2 join retail puts the table base in `$s1` and the
  totals' base in `$s0`; indexing the cast table directly swapped them.**
  `r = (EditorTrap *)gTraps;` before the three reads fixed it, the same
  lever as EditorPickBox's.
- Frame 0x40 with no stack local: `s32 unused[2];` (LEARNINGS); new
  baselined "unused variable" warning.

## Declarations

- 28 string symbols declared in the unit as `extern char D_…[]` with their
  text in the comment (20 rodata, 7 `.sdata`, all new).
- `config/typeviews-warnings.txt`: one more editor "unused variable
  `unused'".

```c
        case 2:
            FntPrint(sEdInfoTrapNoFmt, sTotals.unk18);
            if (((EditorTrap *)gTraps)[sTotals.unk18].unk38 != -1) {
                FntPrint(sEdInfoTrapModelFmt, ((EditorTrap *)gTraps)[sTotals.unk18].unk38);
            } else {
                FntPrint(sEdOptionsText);
            }
            r = (EditorTrap *)gTraps;
            FntPrint(sEdInfoTrapSwitchFmt, r[sTotals.unk18].unk34);
            FntPrint(sEdInfoTypeFmt, r[sTotals.unk18].unk42);
            FntPrint(sEdInfoActFmt, r[sTotals.unk18].unk43);
            break;
```

(The full body is in `src/editor.c`.)
