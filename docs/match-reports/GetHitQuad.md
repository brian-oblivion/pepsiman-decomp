# GetHitQuad — MATCHED 13/13 (round 2, charlie)

> Renamed from `func_80036A84` on 2026-10-11 (tools/rename.py). Address 0x80036a84.

Unit `src/code_24748.c`. Returns a pointer to record `sub` of entry `idx` in
the bank `gHitQuads` points at: 80 eight-byte entries (`BankEntry`, the
first word the index of the entry's first record), then 0x4C-byte records
from byte 0x280 (`Bank4C`, `Rec4C`, declared in the unit).

```c
Rec4C *GetHitQuad(s32 idx, s32 sub) {
    Bank4C *bank = (Bank4C *)gHitQuads;
    Rec4C *recs = bank->recs;

    return &recs[bank->entries[idx].first + sub];
}
```

## Externs

`gHitQuads` is `extern s32` in `include/common.h` (2 units) and `%gp_rel`
in retail; the function casts it rather than retyping the shared
declaration.

## Levers

Twin of `GetHitPoint` (bank at `gHitQuads`, 0x4C-byte records); the same
shape matched first try. Indexing `bank->recs[...]` directly was not
measured here separately (in `GetHitPoint` it gives 10/13).
