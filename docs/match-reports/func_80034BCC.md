# func_80034BCC — MATCHED 100/100 (round 7, echo)

Unit `src/code_24748.c`. Inserts a record into the second bank
(`D_80095A4C`) for the current entry `D_80095A30`: shifts the bank's
0x4C-byte records up by one from the end of that entry's range to record
99, takes the new record (`func_80036A84(entry, entry's count)`) into
`D_80095A34`, fills its four 6-byte points and its `unk48` from the
dispatch object `D_800DF9C0`, bumps the entry's count and the start index
of every later entry (up to `D_8009588E`), counts the insert in
`D_80095A2C` and reinstalls the bank (`func_80036878`).

```c
/* MATCHING: the last loop enters at its test (a for or while is rotated). */
void func_80034BCC(void) {
    Bank4C *bank;
    BankEntry *e;
    Rec4C *recs;
    Rec4C *src;
    Rec4C *dst;
    Rec4C *rec;
    s32 k;
    s32 n;

    bank = (Bank4C *)D_80095A4C;
    e = bank->entries;
    e += D_80095A30;
    recs = (Rec4C *)&bank->entries[D_8009588E];
    src = &recs[98];
    dst = &recs[99];
    for (k = e->first + e->unk4; k < 100; k++) {
        *dst = *src;
        dst--;
        src--;
    }
    bank = (Bank4C *)D_80095A4C;
    e = bank->entries;
    e += D_80095A30;
    rec = func_80036A84(D_80095A30, e->unk4);
    D_80095A34 = rec;
    for (k = 0; k < 4; k++) {
        rec->pts[k].x = D_800DF9C0.pts[k].x;
        rec->pts[k].y = D_800DF9C0.pts[k].y;
        rec->pts[k].z = D_800DF9C0.pts[k].z;
    }
    D_80095A34->unk48 = D_800DF9C0.unk48;
    e = (BankEntry *)D_80095A4C;
    e += D_80095A30;
    e->unk4++;
    n = D_8009588E;
    k = D_80095A30 + 1;
    goto test;
    do {
        e++;
        e->first++;
        k++;
    test:;
    } while (k < n);
    D_80095A2C++;
    func_80036878();
}
```

Unit-local types: `Pt6` (three `s16`); `Rec4C` and `Obj48` now start with
`Pt6 pts[4]` (sizes unchanged). `D_80095A34` (only this unit reaches it) is
declared in the unit as `Rec4C *`. The records start right after the
`D_8009588E` entries, so `recs = &bank->entries[D_8009588E]`.

## Levers (about 30 builds)

| C | words / asm-differ |
| --- | --- |
| first draft: one `dst` pointer, `*dst = dst[-1]` | 17/100 |
| separate `src` and `dst` pointers, both decremented | loop body right |
| `Pt6` struct copy `to[k] = from[k]` | `lwl`/`lwr` + `lh`: wrong |
| **member copies indexed on both sides** `rec->pts[k].x = D_800DF9C0.pts[k].x` | copy loop right (`lhu`/`sh`, offsets 0/2/4) |
| last loop as `for` / `while` (with or without a local bound) | rotated: `beqz` guard instead of retail's `j` to the test |
| **`goto test; do { ... test:; } while (k < n);`, bound in a local** | loop right |
| `&bank->entries[idx]` in one expression | index loaded before the bank pointer, no `move` copy |
| **`bank = D; e = bank->entries; e += idx;`** in the prologue and for the call's argument | bank loaded first, kept in one register and copied (`move a0, a2`) for `recs` |
| `unk48` store after recomputing `e` | loads of `D_80095A4C` / `D_80095A30` in the wrong order |
| **`unk48` store first, then `e = (BankEntry *)D_80095A4C; e += idx;`** | **100/100** |

### Proposed learning

- **A loop entered by a `j` to its test at the bottom (no `beqz` guard
  copying the test): `goto test; do { ... test:; } while (c);`.** 2.8.1
  rotates every `for` and `while` tried here into a guarded do/while.
  (func_80034BCC)
- **A struct pointer loaded first and copied (`move`) before an indexed
  element address is built in place: `p = base; p += i;` as separate
  statements.** `&base[i]` loads the index first. (func_80034BCC)
