# LoadVabBank — MATCHED 62/62 (round 4, echo)

> Renamed from `func_80042A88` on 2026-10-10 (tools/rename.py). Address 0x80042a88.

Unit `src/code_31cec.c`. Two-step sound-bank switch, driven by the state
`gVabLoadStep`. Returns 1 at once when `bank` is 0 or already current
(`gVabBank`). In state 0 it starts loading the bank: the source entry
`&D_8007B6A4[bank]` and the size word `sSoundFileSectors[bank + 6]` go to the
loader globals (`gCdLoadLoc`, `gCdLoadSectors`), the destination
0x8016D000 to `gCdLoadDest`, two loader flags are set to 1 and the state
moves to 1. In state 1, once the loader's step byte `gCdLoadState` reaches
6, it closes the old VAB, records `bank` and calls LoadVab(1);
only then does it return 1.

```c
extern u16 gVabLoadStep;   /* common.h */
extern s32 gCdLoadLoc[]; /* common.h */
extern s32 gCdLoadDest[]; /* common.h */
extern s32 gCdLoadSectors[]; /* common.h */
extern s16 gSeVab;
extern u16 gVabBank;
extern s16 gCdLoadCount;
extern s8 gCdLoadState;
extern s32 D_8007B6A4[];
extern s32 sSoundFileSectors[];
void LoadVab(s16 arg0);

s32 LoadVabBank(u16 bank) {
    s32 ret = 0;

    if (bank == 0) {
        return 1;
    }
    if (bank == gVabBank) {
        return 1;
    }
    switch (gVabLoadStep) {
        case 0:
            gCdLoadLoc[0] = (s32)&D_8007B6A4[bank];
            gCdLoadSectors[0] = sSoundFileSectors[bank + 6];
            gCdLoadDest[0] = 0x8016D000;
            gCdLoadCount = 1;
            gCdLoadState = 1;
            gVabLoadStep++;
            break;
        case 1:
            if (gCdLoadState == 6) {
                SsVabClose(gSeVab);
                gVabBank = bank;
                LoadVab(1);
                ret = 1;
            }
            break;
    }
    return ret;
}
```

## Levers

| step | result |
| --- | --- |
| stores in the order the globals are listed in the asm (`gCdLoadSectors` last, where retail stores it in the `j` delay slot) | 21/62, the table load scheduled after the stores |
| `gCdLoadSectors = sSoundFileSectors[bank + 6];` written second, right after `gCdLoadLoc` | 61 words, one `li $v0, 1` short |
| **`gVabLoadStep++;` for the state store** | **62/62** |

- **A second `li $v0, 1` right before a store, with `$v0` already 1: the
  stored value is not the literal.** `gVabLoadStep = 1;` reuses the 1
  already in `$v0`; `gVabLoadStep++` (known 0 inside `case 0`) is folded
  to a fresh constant after CSE has passed it. A chained `a = b = 1`
  does not help.
- `ret` lives in `$a3` from the top: an `s32 ret = 0;` local returned at
  the end; the early exits return the literal 1 from branch delay slots.
- Declarations read from retail: `gCdLoadCount` and `gCdLoadState` are
  reached through `lui $at` (stores) and `lui $v1` / `lb $v1` (same
  register), so plain scalars outside the gp list; `gCdLoadState` is `s8`
  (`lb`). The tables are `lui`/`addiu` + scaled index, so `[]`.
- LoadVab sign-extends its argument with `sll`/`sra` 16, so its
  prototype here takes `s16`.

### Proposed learning

- **A constant reloaded (`li`) right before a store although the same
  register already holds it: the source stored something CSE only folds
  late, such as `x++` on a value known from a `switch` case.**
  (LoadVabBank)
