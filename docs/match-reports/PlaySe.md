# PlaySe — MATCHED 91/91 (round 10, delta)

> Renamed from `func_80042538` on 2026-10-10 (tools/rename.py). Address 0x80042538.

Unit `src/code_31cec.c`. Starts a sound sequence (`id`, the entry in the
sequence directory at 0x80101000): finds the first of slots 2..9 of
`gSeqIds` whose sequence has ended (or takes slot 9), closes what was
there, and unless loading is blocked (`gVabError`, returns -1) opens the
sequence on its VAB (`sSeVabBanks` maps ids from 70 up to a bank, lower ids
use bank 0), plays it once at full volume, and moves the round-robin
cursor `gSeSlot` on (wrapping to 2).

```c
extern s16 gSeqIds[];
extern u16 gSeSlot;
extern s32 gVabError;
extern s16 gBgmVab;
extern u8 sSeVabBanks[];

s32 PlaySe(s32 id) {
    s16 *seq;
    s16 i;
    u8 bank;
    s16 *vabs;
    u32 *hdr;

    for (gSeSlot = 2; gSeSlot < 10; gSeSlot++) {
        i = gSeSlot;
        seq = &gSeqIds[(u16)i];
        if (SsIsEos(*seq, 0) == 0) {
            break;
        }
    }
    SsSeqClose(*seq);
    if (gVabError != 0) {
        return -1;
    }
    hdr = (u32 *)0x80101000;
    hdr += (u16)id * 4;
    vabs = &gBgmVab;
    if ((u16)id < 70) {
        bank = 0;
    } else {
        bank = sSeVabBanks[(u16)id - 70];
    }
    gSeqIds[(u16)i] = SsSeqOpen((unsigned long *)(*hdr + 0x80101000), vabs[bank]);
    SsSeqPlay(gSeqIds[(u16)i], 1, 1);
    SsSeqSetVol(gSeqIds[(u16)i], 127, 127);
    if (++gSeSlot >= 10) {
        gSeSlot = 2;
    }
}
```

`include/common.h` changes the shared prototype from `void` to `s32
PlaySe(s32 id)`: retail returns -1 from a branch delay slot, which
a `void` function never sets. Callers ignore the result, so their code is
unchanged (the whole image verifies). The success path falls off the end
(baselined `-Wall` warning).

## Levers

| step | result |
| --- | --- |
| first write: `u16 i; i = gSeSlot; seq = &gSeqIds[i];`, the stores through `gSeqIds[i]` | 5/91: the stores reuse `seq`'s register |
| `s32 i`, stores through `gSeqIds[(u16)i]` | 55/91: the `sllv` by the `1` in `$a1` appears; retail's `andi s1, 0xFFFF` and fresh table base do not |
| `vabs = &gBgmVab;` pointer local, and `if (id < 70) bank = 0; else bank = ...;` | 62/91: the bank in `$v0` and its `sll` placed as retail; one word short (the `andi`) |
| `i = (s16)gSeSlot` | 90/91: the `andi` and the base appear, but the load becomes `lh` |
| **`s16 i; i = gSeSlot; seq = &gSeqIds[(u16)i];`, every store through `gSeqIds[(u16)i]`** | **91/91** |

- **`andi sN, 0xFFFF` on a value loaded `lhu`, before indexing: the
  variable is `s16` and the index is `(u16)i`.** The load stays `lhu`
  (the first use only needs the low half), and `(u16)` of an `s16` local
  keeps the mask because cc1 cannot see the upper bits. A `u16` or `s32`
  local has known upper bits and the `andi` is dropped.
- The permuter (Gate 3 run: scaffold 1 ins / 4 del against the same
  disassembly in the tree; one search, `-j 6 --stack-diffs`, stopped at
  38776 iterations once the manual match landed) found the
  `vabs` pointer local and a `do { } while (0)` body (no effect once
  `i` is `s16`). Its best, 86/91 in the tree, read `i` uninitialised
  (`if (i) {...} else {...}`): undefined, rejected, but it showed that
  the `andi` needs `i`'s upper bits to be unknown to combine.
- The bank lookup's `if (id < 70) bank = 0; else ...` (vs `bank = 0;
  if (id >= 70) ...`) gives the zero in the delay slot as retail; the
  `vabs` local puts the `sll` of the bank before the table's `lui`.

### Proposed learning

- **A redundant `andi 0xFFFF` on an `lhu`-loaded index: an `s16` local
  indexed through `(u16)i`.** (PlaySe)
- **A table index scaled before its table's `lui`/`addiu`: the table
  address went into a pointer local first**, `p = &G; ... p[k]`.
  (PlaySe)
