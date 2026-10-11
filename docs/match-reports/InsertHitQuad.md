# InsertHitQuad — MATCHED 100/100 (round 7, echo)

> Renamed from `func_80034BCC` on 2026-10-11 (tools/rename.py). Address 0x80034bcc.

Unit `src/hit_tool.c`. Inserts a record into the second bank
(`gHitQuads`) for the current entry `gToolSection`: shifts the bank's
0x4C-byte records up by one from the end of that entry's range to record
99, takes the new record (`GetHitQuad(entry, entry's count)`) into
`gToolQuad`, fills its four 6-byte points and its `unk48` from the
dispatch object `gToolNewQuad`, bumps the entry's count and the start index
of every later entry (up to `gSectionCount`), counts the insert in
`gToolQuadIndex` and reinstalls the bank (`ApplyHitQuads`).

```c
/* MATCHING: the last loop enters at its test (a for or while is rotated). */
void InsertHitQuad(void) {
    Bank4C *bank;
    BankEntry *e;
    Rec4C *recs;
    Rec4C *src;
    Rec4C *dst;
    Rec4C *rec;
    s32 k;
    s32 n;

    bank = (Bank4C *)gHitQuads;
    e = bank->entries;
    e += gToolSection;
    recs = (Rec4C *)&bank->entries[gSectionCount];
    src = &recs[98];
    dst = &recs[99];
    for (k = e->first + e->unk4; k < 100; k++) {
        *dst = *src;
        dst--;
        src--;
    }
    bank = (Bank4C *)gHitQuads;
    e = bank->entries;
    e += gToolSection;
    rec = GetHitQuad(gToolSection, e->unk4);
    gToolQuad = rec;
    for (k = 0; k < 4; k++) {
        rec->pts[k].x = gToolNewQuad.pts[k].x;
        rec->pts[k].y = gToolNewQuad.pts[k].y;
        rec->pts[k].z = gToolNewQuad.pts[k].z;
    }
    gToolQuad->unk48 = gToolNewQuad.unk48;
    e = (BankEntry *)gHitQuads;
    e += gToolSection;
    e->unk4++;
    n = gSectionCount;
    k = gToolSection + 1;
    goto test;
    do {
        e++;
        e->first++;
        k++;
    test:;
    } while (k < n);
    gToolQuadIndex++;
    ApplyHitQuads();
}
```

Unit-local types: `Pt6` (three `s16`); `Rec4C` and `Obj48` now start with
`Pt6 pts[4]` (sizes unchanged). `gToolQuad` (only this unit reaches it) is
declared in the unit as `Rec4C *`. The records start right after the
`gSectionCount` entries, so `recs = &bank->entries[gSectionCount]`.

## Levers (about 30 builds)

| C | words / asm-differ |
| --- | --- |
| first draft: one `dst` pointer, `*dst = dst[-1]` | 17/100 |
| separate `src` and `dst` pointers, both decremented | loop body right |
| `Pt6` struct copy `to[k] = from[k]` | `lwl`/`lwr` + `lh`: wrong |
| **member copies indexed on both sides** `rec->pts[k].x = gToolNewQuad.pts[k].x` | copy loop right (`lhu`/`sh`, offsets 0/2/4) |
| last loop as `for` / `while` (with or without a local bound) | rotated: `beqz` guard instead of retail's `j` to the test |
| **`goto test; do { ... test:; } while (k < n);`, bound in a local** | loop right |
| `&bank->entries[idx]` in one expression | index loaded before the bank pointer, no `move` copy |
| **`bank = D; e = bank->entries; e += idx;`** in the prologue and for the call's argument | bank loaded first, kept in one register and copied (`move a0, a2`) for `recs` |
| `unk48` store after recomputing `e` | loads of `gHitQuads` / `gToolSection` in the wrong order |
| **`unk48` store first, then `e = (BankEntry *)gHitQuads; e += idx;`** | **100/100** |

### Proposed learning

- **A loop entered by a `j` to its test at the bottom (no `beqz` guard
  copying the test): `goto test; do { ... test:; } while (c);`.** 2.8.1
  rotates every `for` and `while` tried here into a guarded do/while.
  (InsertHitQuad)
- **A struct pointer loaded first and copied (`move`) before an indexed
  element address is built in place: `p = base; p += i;` as separate
  statements.** `&base[i]` loads the index first. (InsertHitQuad)
