# EditHitPoints — MATCHED 162/162 (round 9, echo)

> Renamed from `func_800350C8` on 2026-10-11 (tools/rename.py). Address 0x800350c8.

Unit `src/hit_tool.c`. Twin of EditHitQuads (same 162 words, other
bank). Deletes a record from the first record bank: state 0 of
`gSubStep` resets the record index; state 1 steps and clamps it within
the current entry's records, shows the record (`DrawToolPin` /
`DrawToolPole` on its negated position), and on flag bit 5 shifts the
bank's later records down one, decrements the entry's count and the later
entries' first indices, and reinstalls the bank (`ApplyHitPoints`).

```c
void EditHitPoints(void) {
    VECTOR pos;
    Bank2C *bank;
    BankEntry *e;
    Rec2C *src;
    Rec2C *dst;
    Rec2C *rec;
    s32 k;
    s32 n;

    switch (gSubStep) {
        case 0:
            gToolPointIndex = 0;
            gSubStep = 1;
            break;
        case 1:
            bank = (Bank2C *)gHitPoints;
            e = bank->entries;
            e += gToolSection;
            if (e->unk4 == 0) {
                gToolMode = 0;
                sToolPickMode = 0;
            }
            if (gPadTrig & 8) {
                gToolPointIndex++;
            }
            if (gPadTrig & 4) {
                gToolPointIndex--;
            }
            e = bank->entries;
            e += gToolSection;
            gToolPointIndex = gToolPointIndex<0 ? 0 : gToolPointIndex>(u32)(e->unk4 - 1) ? e->unk4 - 1 : gToolPointIndex;
            rec = GetHitPoint(gToolSection, gToolPointIndex);
            pos.vx = -rec->unk1C;
            pos.vy = rec->unk20;
            gToolPoint = rec;
            pos.vz = -rec->unk24;
            if (gPadTrig & 0x40) {
                gToolMode = 0;
                sToolPickMode = 0;
            }
            if (sToolInfoMode != 1 && (gPadTrig & 0x20)) {
                bank = (Bank2C *)gHitPoints;
                e = bank->entries;
                e += gToolSection;
                dst = (Rec2C *)&bank->entries[gSectionCount];
                src = &dst[e->first + gToolPointIndex + 1];
                dst += e->first + gToolPointIndex;
                for (k = e->first + gToolPointIndex; k < 399; k++) {
                    *dst = *src;
                    dst++;
                    /* MATCHING: the empty do/while gives the source pointer
                     * $a1 and the block move's end $a3. */
                    do {
                        src++;
                    } while (0);
                }
                n = gSectionCount;
                /* MATCHING: a byte-pointer store keeps the index load below it. */
                *(s32 *)((u8 *)e + 4) -= 1;
                k = gToolSection + 1;
                for (; k < n; k++) {
                    e++;
                    e->first--;
                }
                ApplyHitPoints();
            }
            DrawToolPin(&pos);
            DrawToolPole(&pos);
            break;
    }
}
```

## Levers

| C | words |
| --- | --- |
| first body (`&bank->entries[i]`, `k` set before `src`/`dst`, `e->unk4--`) | 15/162, 4 words short |
| `e = bank->entries; e += i;` (InsertHitPoint's shape), `n` read before the store | 114/162 |
| `for (...) { e++; e->first--; }` (the bump at the loop top) and `src` before `dst` | 132 |
| the index load kept below the count store: `__asm__("")` or `*(s32 *)((u8 *)e + 4) -= 1;` | 139 |
| `src`/`dst` from `e->first + gToolPointIndex` again, `for (k = <same>; ...)` | 151 |
| `src++; dst++;` | 153 |
| **base pointer advanced in place**: `dst = (Rec2C *)&bank->entries[n]; src = &dst[k + 1]; dst += k;` | 156 |
| `dst++; src++;` with that | 157: `$a1`/`$a3` swapped between the source pointer and the block move's end |
| **`do { src++; } while (0);`** (permuter) | **162/162** |
| `src = dst + 1`, `recs[k] = recs[k + 1]`, `*dst++ = *src++`, the step in the `for`, declaration order, a second counter, a `while` loop, `__asm__("")` in the body, the do/while around the copy instead | 88 to 157 |

Gate 3: scaffold `--debug --stack-diffs` insertions 0 / deletions 0, 7
register differences (score 35); real build insertions 0 / deletions 0.
AGREE. Permuter `-j 4 --stack-diffs --best-only`, stopped on zero after
3,090 iterations; the candidate verified in-tree (whole oracle green) and
also closes EditHitQuads.

### Proposed learning

- **A store through a struct pointer lets a later load of a fixed global
  rise above it; `*(s32 *)((u8 *)p + off) -= 1` keeps it below** (the
  store-side twin of the byte-pointer read). (EditHitPoints)
- **Two block-move pointers computed from one base with the destination in
  the base's register: the base was the destination variable, advanced in
  place**, `dst = base; src = &dst[k + 1]; dst += k;`. (EditHitPoints)
- **A pointer bump wrapped in `do { } while (0)` changes which hard
  register a loop pointer gets** without changing any instruction: a
  permuter shape for a two-register swap in a loop. (EditHitPoints,
  EditHitQuads)
