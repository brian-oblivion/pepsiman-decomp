# InsertHitPoint — MATCHED 100/100 (round 7, echo)

> Renamed from `func_80034F38` on 2026-10-11 (tools/rename.py). Address 0x80034f38.

Unit `src/code_24748.c`. The first-bank twin of InsertHitQuad: inserts a
0x2C-byte record into the bank at `gHitPoints` for the current entry
`gToolSection` (shifting records up to record 399), fills the new record
(`GetHitPoint`) from the game state's position (x and z negated) and the
menu values `gToolRadius` (times ten) and `gToolType`, tags it with the
entry index, keeps it in `gToolPoint`, bumps the entry's count and the
later entries' start indices, counts the insert in `gToolPointIndex` and
reinstalls the bank (`ApplyHitPoints`).

```c
/* MATCHING: the last loop enters at its test (a for or while is rotated). */
void InsertHitPoint(void) {
    Bank2C *bank;
    BankEntry *e;
    Rec2C *recs;
    Rec2C *src;
    Rec2C *dst;
    Rec2C *rec;
    s32 k;
    s32 n;

    bank = (Bank2C *)gHitPoints;
    e = bank->entries;
    e += gToolSection;
    recs = (Rec2C *)&bank->entries[gSectionCount];
    src = &recs[398];
    dst = &recs[399];
    for (k = e->first + e->unk4; k < 400; k++) {
        *dst = *src;
        dst--;
        src--;
    }
    bank = (Bank2C *)gHitPoints;
    e = bank->entries;
    e += gToolSection;
    rec = GetHitPoint(gToolSection, e->unk4);
    rec->unk1C = -sGamePos.unk348;
    rec->unk20 = sGamePos.unk34C;
    gToolPoint = rec;
    rec->unk24 = -sGamePos.unk350;
    rec->unk14 = (s16)gToolRadius * 10;
    rec->unk28 = gToolSection;
    rec->unk18 = gToolType;
    e = (BankEntry *)gHitPoints;
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
    gToolPointIndex++;
    ApplyHitPoints();
}
```

`Rec2C` (unit-local) gained its known members (`unk14`, `unk18`,
`unk1C`..`unk24`, `unk28`; size unchanged) and `gToolPoint` (this unit only)
is declared in the unit as `Rec2C *`.

## Levers

Everything outside the record fill came straight from InsertHitQuad's
match (separate `bank`/`e` statements, two copy pointers, the `goto` loop).

| fill order | words |
| --- | --- |
| retail's store order (1C, 20, A44, 28, 18, 24, 14) | 28/100, one word short |
| load order (1C, 14, 28, 20, 24, 18, A44) | 47/100 |
| 1C, 20, 24, 14, 28, 18, A44 | 64/100, length right |
| **1C, 20, A44, 24, 14, 28, 18** | **100/100** |

The last row came from a bounded enumeration (`timeout 900`, rc 0): every
order of the seven statements keeping 1C < 20 < 24 (their loads from the
game state cannot pass the record stores), stopped at the first match.
Stores through `rec` at different offsets and stores to fixed globals
reorder freely in the scheduler, so retail's store order is not the source
order here.

### Proposed learning

- **Independent stores through one pointer (different members) and to
  fixed globals do not keep source order; only loads that may alias the
  stores pin them.** When the stores look shuffled, enumerate the orders
  that keep the pinned statements in sequence. (InsertHitPoint)
