# func_8002FDB4 — MATCHED 112/112 (round 8, bravo)

Unit `src/code_1dc24.c`. On flag bit 5: error 8 in `D_800958DA` when
`sTotals.unk26` is 200, error 7 when there is no current entry
(`D_80095824 == -1`); otherwise shifts the block's 8-byte points
(`D_800959C4`) up by one from the end of the current entry, writes the
game position (`sGameSave.unk348`, as halfwords) and `D_80095B4C[0] + 1`
as the new point, bumps the entry's count and the start of every later
entry (`D_800959C0`, `D_80095794` entries), and rebuilds the block header
with `func_8002D0C4(0x801FD000)`.

## How it closed

The structure matched early (19/112 at first, 36/112 once the entry pointer
of the first part was the same local as the second's). The rest were
aliasing and register-allocation shapes:

1. **`D_800959C0` reloaded after the count store, `D_80095824` not:
   `D_800959C0` / `D_800959C4` are reached as MEMBERS of a struct at a
   fixed address**, here a unit-local view `#define sCur (*(BlockCur
   *)&D_800959C0)`. GCC 2.8.1 assumes a member store through a pointer
   cannot alias a scalar at a fixed address, but may alias a member of a
   fixed struct, so the scalar `D_80095824` stays in its register and the
   pointer is reloaded. Declared as plain scalars (common.h), neither is
   reloaded. (`*(s32 *)` and `s32 *` stores were tried for the count and
   change nothing about that reload.)
2. **The old count read into the later loop counter `j`** (`j = e->count;
   e->count = j + 1;`), a permuter lead (output score 555 from 2080):
   `n = e->count++` with its own local copies it into a new register.
3. **The second loop's pointer loaded, then advanced**: `e = (Span8
   *)sCur.ents; e += D_80095824 + 1;`.
4. **One `Pt8 *p` for both the shift's source pointer and the new point**
   (permuter lead, 280 then 95): 78/112, then 108/112.
5. **Each entry pointer loaded, then advanced**, `e = (Span8 *)sCur.ents;
   e += D_80095824;`, in both parts: 111/112, the last word being data
   drift from another live near-miss in the unit; byte-exact once that was
   wrapped.

The permuter's 95 also wrapped the second half in `do { } while (0);`;
the match does not need it.

Permuter: one bounded search split in two runs (the first on the 36/112
base, 24385 iterations, stopped once a lead moved the base; the second on
the 64/112 base, about 26000 iterations, stopped at the match). Gate 3:
scaffold compiled and scored (check 1), `--debug --stack-diffs` 1 ins / 1
del against funcdiff's 21/21 (check 2), objdump of base.o and the real
object identical apart from addresses, so AGREE (check 3).

## Declarations

- `Span8` (an entry as `start`, `count` words) and `Pt8` (an 8-byte point,
  three `s16` and a `u16`): unit-local views. code_1a098.h's `Ent8` is the
  same entry seen as halfwords.
- `BlockCur` / `sCur`: the unit's member view of `D_800959C0` and
  `D_800959C4` (see 1), with a `MATCHING:` note.
- `D_80095824` (`s32`) declared in the unit. `D_800958DA` moved from
  code_24748.c to `include/common.h`, type unchanged (declcheck).

```c
void func_8002FDB4(void) {
    s16 pos[3];
    Pt8 *dst;
    Pt8 *p;
    Span8 *e;
    s32 i;
    u32 j;

    if (D_80095970 & 0x20) {
        if (sTotals.unk26 == 200) {
            D_800958DA = 8;
        } else if (D_80095824 == -1) {
            D_800958DA = 7;
        } else {
            e = (Span8 *)sCur.ents;
            e += D_80095824;
            p = (Pt8 *)sCur.pts;
            dst = p;
            pos[0] = sGameSave.unk348[0];
            pos[1] = sGameSave.unk348[1];
            pos[2] = sGameSave.unk348[2];
            p += 198;
            dst += 199;
            for (i = e->start + e->count; i < 200; i++) {
                *dst = *p;
                dst--;
                p--;
            }
            e = (Span8 *)sCur.ents;
            e += D_80095824;
            p = (Pt8 *)sCur.pts;
            j = e->count;
            e->count = j + 1;
            p += e->start + j;
            e = (Span8 *)sCur.ents;
            e += D_80095824 + 1;
            p->x = pos[0];
            p->y = pos[1];
            p->z = pos[2];
            p->tag = D_80095B4C[0] + 1;
            for (j = D_80095824 + 1; j < D_80095794; j++) {
                e->start++;
                e++;
            }
            func_8002D0C4((BlockHeader *)0x801FD000);
        }
    }
}
```

### Proposed learning

- **A global pointer reloaded after a member store through a pointer, while
  a scalar global read nearby is not: reach the pointer as a member of a
  struct at a fixed address** (`#define sCur (*(T *)&D_X)`). The store-side
  rule in "Declarations" (a member store does not alias a fixed scalar) has
  this converse. (func_8002FDB4)
- **A pointer local loaded into its own register and then advanced in place
  (`lw a2, G; addu a2, a2, off`): `p = G; p += k;`**; `p = G + k` loads into
  a scratch register first. (func_8002FDB4)
