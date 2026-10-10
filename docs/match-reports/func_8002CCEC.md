# func_8002CCEC — MATCHED 246/246 (round 11, echo; round 9 stall by charlie)

Unit `src/code_1a098.c`. Draws the markers of the current zone: for
each group of the zone (`D_80095934[D_8009578C]`, group list
`D_8009593C`) below `D_800959C8`, each block entry of the group marked
2 in `D_800A7550` whose flag index (`unk6`) is set in `D_800A74D0`,
non-zero and not yet done (bit 15): places a 75x75 sprite 0xFA at the
entry relative to the player, sorts it near or far as func_8002B8F8 does,
and when func_8002D0F0 reports the player 50 units above it, runs
func_800414EC, plays 0x35 and func_8003F834 (7, or 8 for state 2, none
for 0), and sets bit 15.

## Round 11 (echo): matched

REVISIT. The body below, rebuilt exactly as preserved (with
`func_8001B004`'s prototype), still scored 242/246, `insertions 4 /
deletions 4` (positional skeleton diffs 4): the residue under "Residue" below, unchanged.

Round 9's abs lever closed it on the first build: the abs written as
`v >= 0 ? v : -v` (LEARNINGS, func_8003B780: "`bgez r; nop; negu r` with
the delay slot unfilled is cc1's abs"). The `absInt` helper became

```c
static __inline__ s32 absInt(s32 x) {
    return x >= 0 ? x : -x;
}
```

and nothing else changed: 246/246, image OK. The `if (x < 0) x = -x;`
form let the scheduler fill the `bgez` slot with `size.vx = 75`; the
ternary leaves it empty, so the store falls to the range test's slot as
in retail. Round 10's `s32`-with-no-return lever was not needed. The
unused `Group8 *g` local of the preserved body was dropped (no code
change).

Declarations left in the unit for the head (declcheck LOCAL/MULTI):
`D_80095934` and `D_8009593C` (code_29f54.c declares both too, with its
own `Run` type for the first), `func_8001B004` (defined in code_a0bc,
not in `code_a0bc.h`) and `func_800414EC` (defined in code_31cec).

The round 9 body follows as it was preserved.

Best body (built in round 9: 242/246, length exact, `insertions 4 /
deletions 4` opcode-level, i.e. two words moved). It needs, besides the
unit's declarations, `func_8001B004`'s prototype (`void func_8001B004(u16
id, SVECTOR *size, CVECTOR *color, s32 shift, GsOT *ot);`) and the unit's
`GameHead` with `unk348`/`unk350` (in the tree since func_8002A328):

```c
#if 0
/** @brief A zone's run of group indices. */
typedef struct {
    s16 start; /**< first index into the group list */
    s16 count; /**< how many */
} Zone4;

/** @brief A run of block entries. */
typedef struct {
    u32 first; /**< first entry */
    u32 count; /**< how many */
} Group8;

extern Zone4 *D_80095934; /**< the zones */
extern s8 *D_8009593C;    /**< the group list the zones index */
void func_800414EC(s16 n);
s8 func_8002D0F0(Vec3 *pos);

static __inline__ s32 absInt(s32 x) {
    if (x < 0) {
        x = -x;
    }
    return x;
}

void func_8002CCEC(void) {
    GsCOORDINATE2 coord;
    MATRIX mat;
    SVECTOR size;
    Vec3 pos;
    s32 j;
    u32 i;
    u32 k;
    u16 n;
    Group8 *g;
    Ent8 *e;
    s16 m;

    GsInitCoordinate2(WORLD, &coord);
    for (j = 0; j < D_80095934[D_8009578C].count; j++) {
        if (D_8009593C[D_80095934[D_8009578C].start + j] < D_800959C8) {
            for (i = 0;
                 i < ((Group8 *)D_800959C0)[D_8009593C[D_80095934[D_8009578C].start + j]].count; i++) {
                k = ((Group8 *)D_800959C0)[D_8009593C[D_80095934[D_8009578C].start + j]].first + i;
                if ((s8)D_800A7550[k] != 2) {
                    continue;
                }
                e = (Ent8 *)(k * 8 + (u32)D_800959C4);
                n = e->unk6;
                if ((s8)D_800A74D0[e->unk6] == 0 || e->unk6 == 0 || (e->unk6 & 0x8000)) {
                    continue;
                }
                coord.coord.t[0] = -sGameHead.unk348 + e->unk0;
                coord.coord.t[1] = e->unk2;
                coord.coord.t[2] = -sGameHead.unk350 + (s16)e->unk4;
                coord.flg = 0;
                GsGetLs(&coord, &mat);
                GsSetLsMatrix(&mat);
                size.vy = 75;
                size.vx = 75;
                if (absInt(coord.coord.t[0]) > 700 || absInt(coord.coord.t[2]) > 700) {
                    func_8001B004(0xFA, &size, NULL, 2, &D_800A7318[D_80095750]);
                } else {
                    func_8001A69C(0xFA, &size, NULL, 2, &D_800ACEA8[D_80095750]);
                }
                pos.x = coord.coord.t[0] - D_800A7308[0];
                pos.y = coord.coord.t[1] - 50;
                pos.z = coord.coord.t[2] - D_800A7308[2];
                if (func_8002D0F0(&pos)) {
                    m = n;
                    func_800414EC(m);
                    switch (m) {
                        case 0:
                            break;
                        case 1:
                        default:
                            func_80042538(0x35);
                            func_8003F834(7, ((Ent8 *)(k * 8 + (u32)D_800959C4))->unk0,
                                          ((Ent8 *)(k * 8 + (u32)D_800959C4))->unk2 - 50,
                                          (s16)((Ent8 *)(k * 8 + (u32)D_800959C4))->unk4, 0);
                            break;
                        case 2:
                            func_80042538(0x35);
                            func_8003F834(8, ((Ent8 *)(k * 8 + (u32)D_800959C4))->unk0,
                                          ((Ent8 *)(k * 8 + (u32)D_800959C4))->unk2 - 50,
                                          (s16)((Ent8 *)(k * 8 + (u32)D_800959C4))->unk4, 0);
                            break;
                    }
                    ((Ent8 *)(k * 8 + (u32)D_800959C4))->unk6 |= 0x8000;
                }
            }
        }
    }
}
#endif
```

## Residue

```
retail                          build
8002CE74 lw   v1, 0x30(sp)       lw   v1, 0x30(sp)
8002CE78 li   v0, 75             li   v0, 75
8002CE7C sh   v0, 0x8A(sp)       sh   v0, 0x8A(sp)
8002CE80 bgez v1, +8             bgez v1, +8
8002CE84 nop                     sh   v0, 0x88(sp)
8002CE88 negu v1, v1             negu v1, v1
8002CE8C slti v1, v1, 701        slti v0, v1, 701
8002CE90 beqz v1, far            beqz v0, far
8002CE94 sh   v0, 0x88(sp)       nop
```

`size.vx = 75` sits in the first range test's slot in retail, in the
abs's `bgez` slot in the build; with `v0` still holding 75, retail
puts the compare in `v1`.

## What was tried (about 12 builds and one permuter run)

- The vertex address as an integer sum (`k * 8 + (u32)D_800959C4`) and
  the call arguments read through the sum, not a pointer local: 87 ->
  238. `-sGameHead.unk348 + e->unk0` (subtrahend loaded first, LEARNINGS
  "operand order sets load order"): 242.
- The two 75 stores as `vy, vx`, `vx, vy` and chained
  (`size.vx = size.vy = 75`): 242, 241, 242.
- The range test with `absInt`, with `ax`/`az` locals and `goto far`,
  and with the helper written as a ternary: all 242.
- `size.vx = 75` between the first abs and its test: the compare moves to
  the 75's register but the `vy` store falls into the `bgez` slot and the
  function loses a word (90/246).

Permuter: one run, `--stack-diffs`, 900 s bound (rc 124), 68965
iterations, base 130, no improvement. The scaffold uses the `ax`/`az`
locals form (the setup script strips `static __inline__` bodies, so a
scaffold with `absInt` calls it out of line). Gate 3: scaffold 0 ins / 0
del with 2 reorders and 2 register differences, real build 4/4 opcode
ins/del for the same two moved stores: AGREE.

### Proposed learning

- `tools/setup-permuter.sh` keeps only the target function: a seed that
  calls a `static __inline__` helper scores the helper as an out-of-line
  call. Write the helper's body out in the seed.
