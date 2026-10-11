# EditorPickBox — MATCHED 271/271 (round 10, echo)

> Renamed from `func_80030548` on 2026-10-11 (tools/rename.py). Address 0x80030548.

Unit `src/code_1dc24.c`. The box picker, the third of the unit's picker
family (EditorPickRecord on Obj48, EditorPickSprite on Rec3C), here on the
200-entry 0x5C-byte table `D_800CF080` (`sRecs5C`): counts the used records
into `sTotals.unk16`, clearing each one's `unk0` to 0; in state 0 picks the
one nearest the game position (its `unk50` position) into `sTotals.unk12`
(errors 9 and 11); in state 1 moves the game position to it, frees it on
flag bit 5 and stops there, or else steps the selection over free records
on flag bits 0 and 1. Second build matched.

## Shapes read from retail

- The family's levers all carry over: `i = 0; sTotals.unk16 = 0; for (;
  ...)`, `r = &tbl[i];` inside the search loop, the first step up clamping
  to 199, `s32` with no return.
- Unlike its siblings, the free path sets the record's `unk0` to -1 BEFORE
  clearing `gSubStep` and then jumps to the exit: `break;` out of the
  case.
- **The first build differed in three words at the top of case 1: retail
  builds `%hi(D_800CF080)` in `$a0` before the totals' base, the build in
  `$v0` after it.** Reading the record through `r = sRecs5C; sel.vx =
  r[sTotals.unk12].unk50[0]; ...` puts the table base first and matched.
  Indexing `sRecs5C[...]` directly (the macro is `((Rec5Cv *)D_800CF080)`)
  builds the totals' address first. The Obj48 twin indexes its cast table
  directly and matches the other way round.

## Declarations

- None new (`DistanceSquared` came with EditorPickRecord).
- `config/typeviews-warnings.txt`: one more code_1dc24 "control reaches
  end of non-void function".

```c
        case 1:
            r = sRecs5C;
            sel.vx = r[sTotals.unk12].unk50[0];
            sel.vy = r[sTotals.unk12].unk50[1];
            sel.vz = r[sTotals.unk12].unk50[2];
            DrawPin(&sel);
            if (D_800959D8 != 1 && (gPadTrig & 0x20)) {
                sRecs5C[sTotals.unk12].unk0 = -1;
                gSubStep = 0;
                break;
            }
            /* steps on bits 0 and 1 as EditorPickRecord */
```

(The full body is in `src/code_1dc24.c`.)

### Proposed learning

- **A table's `lui` built before another global's base, in a different
  register: the table read through a pointer local assigned first**, `r =
  tbl; ... r[g.idx].f`. Indexing the table directly builds the other base
  first. (EditorPickBox)
