# UpdateToolView — MATCHED (round 13, delta)

> Renamed from `func_800356FC` on 2026-10-11 (tools/rename.py). Address 0x800356fc.

Stalled at 154/157 in rounds 9 (echo) and 12 (charlie).

Unit `src/hit_tool.c`. Steps the camera offset by `gToolViewStep`: state 0
saves the current offset (`gRefView[0..2]`) as the start
(`D_800CEBA0`) and starts the step counter `D_80095A20`; state 1
interpolates over 30 steps (`Lerp`) towards row `gToolViewPreset` of a
five-row rodata table (`D_800115DC`, copied to the stack as one block)
scaled by `gToolZoom`; state 2 restores the saved value of `gToolMode`
and advances to state 3; state 3 jumps straight to the target and clears
the next three words. Every call clears `D_800DB2B8`.

The body is live in `src/hit_tool.c`. The stalled body below is kept for
the record; the match differs from it in case 2 only (`gToolViewStep++;`).

```c
#if 0
typedef struct {
    s32 v[5][3];
} Rows5;

extern Rows5 D_800115DC;
extern s32 D_800CEBA0[];
extern u8 D_80095A20;
extern s32 D_800DB2B8[];
extern s32 gRefView[];  /* common.h */
extern u8 gToolViewStep;
extern s16 D_80095A22;
extern s16 gToolMode;    /* common.h */
extern u8 gToolViewPreset;
extern u8 gToolZoom;
/* per-unit view; model defines it (s16, s16, u16, u16) returning s16 */
s32 Lerp(s32 a, s32 b, u16 t, u16 n);

void UpdateToolView(void) {
    Rows5 rows;

    rows = D_800115DC;
    switch (gToolViewStep) {
        case 0:
            gToolViewStep = 1;
            D_80095A20 = 0;
            D_800CEBA0[0] = gRefView[0];
            D_800CEBA0[1] = gRefView[1];
            D_800CEBA0[2] = gRefView[2];
            break;
        case 1:
            gRefView[0] = Lerp(D_800CEBA0[0], rows.v[(s8)gToolViewPreset][0] * (s8)gToolZoom,
                                          D_80095A20, 30);
            gRefView[1] = Lerp(D_800CEBA0[1], rows.v[(s8)gToolViewPreset][1] * (s8)gToolZoom,
                                          D_80095A20, 30);
            gRefView[2] = Lerp(D_800CEBA0[2], rows.v[(s8)gToolViewPreset][2] * (s8)gToolZoom,
                                          D_80095A20, 30);
            if (++D_80095A20 == 30) {
                gToolViewStep++;
            }
            break;
        case 2:
            gToolMode = D_80095A22;
            gToolViewStep = 2;
            break;
        case 3:
            gRefView[0] = rows.v[(s8)gToolViewPreset][0] * (s8)gToolZoom;
            gRefView[1] = rows.v[(s8)gToolViewPreset][1] * (s8)gToolZoom;
            gRefView[2] = rows.v[(s8)gToolViewPreset][2] * (s8)gToolZoom;
            gRefView[3] = 0;
            gRefView[4] = 0;
            gRefView[5] = 0;
            break;
    }
    D_800DB2B8[0] = 0;
}
#endif
```

## The residue

Register only, three words, all in case 2: funcdiff `insertions 0 /
deletions 0`. Retail stores `gToolViewStep = 2` from `$v0`, the constant the
dispatch loaded for its `beq` against 2, and loads `D_80095A22` into `$v1`.
The build stores the switch value itself (`$v1`, known to equal 2 on that
edge) and loads `D_80095A22` into `$v0`. cse picks the jump-equivalent
register over the compare constant; nothing in the C moved that choice.

## Levers

| C | words |
| --- | --- |
| first body | 144/157 |
| `extern s32 D_800DB2B8[];` (retail splits it `lui`/`%lo`) | 146 |
| case 3: the three zero stores after the third product | 154 |
| case 2's two statements in either order | 154 |
| switch on an `s32`, `u8`, `s16` or `u16` local copy | 154 (`u32`: 28, changes the dispatch) |
| `gToolViewStep = (u8)(gToolViewStep * 0 + 2)` | 154 |
| `s32` function with no return | 154 |
| an if/else-if chain instead of the switch | 13 |

Gate 3: the scaffold's `--debug --stack-diffs` gives insertions 0 /
deletions 0 with 3 register differences (score 15); the real build's
funcdiff line is insertions 0 / deletions 0. AGREE. Permuter: `-j 4
--stack-diffs --best-only`, `timeout 1500`, rc 124 (bound fired), 89,545
iterations, nothing below the base score 15 (one equal-score candidate,
which only added a dead store).

### Proposed learning

- A store of a case's own constant to the switched global uses the switch
  value's register on 2.8.1 (jump equivalence). Retail using the compare
  constant's register here has no C spelling found yet. (UpdateToolView)

## Round 12 (charlie): the mechanism, and why the C levers miss it

Inherited body rebuilt first: 154/157, `insertions 0 / deletions 0`
(agrees with round 9). About 6 builds, no permuter (round 9's search
already covered the register-tie space; the residue is a cse decision, not
an allocation one).

Read from `cc1 -dr -dj -ds` dumps of the unit, against GCC 2.8.1's
`cse.c`:

- The case 2 store becomes `(subreg:QI (reg 221))` in **cse1**; every later
  pass keeps it. 221 is the switch index, 224 the pseudo the tree loads
  with 2 for its `beq`. cse follows the taken `beq` into case 2
  (`cse_end_of_basic_block`: the case label has one use and a BARRIER
  before it), `record_jump_cond` merges 224's class into 221's, and the
  QImode constant 2 is replaced by a lowpart of the class's canonical
  register. `make_regs_eqv` keeps the older register (221) canonical
  unless the newcomer lives beyond the extended block AND past the old
  register's last use. 224's only use is the compare, so 221 always wins.
  Retail's `sb $v0` is that canonical choice going the other way.
- Diagnostic, flags only, NOT a fix: `-fno-cse-follow-jumps` stops the
  substitution and gives retail's `lhu $v1` / `sb $v0`, but case 2 keeps a
  `li $v0, 2` that reorg does not delete (one word long), and case 0
  changes. So retail did substitute, and picked 224.
- A second use of the case-2 label (a `goto` from case 3 that cse folds)
  does not stop it: cse deletes that goto on an earlier path, the label
  drops back to one use, and a later path follows it.
- Bare `__asm__("")` at the top of case 2, `__asm__ __volatile__("" : :)`
  (diagnostic), and the first statement wrapped in `do { } while (0);`: no
  change (153 for the last; it reorders the stores). cse1 has no
  volatile-asm flush in this version.

What would close it is something that makes the compare constant's pseudo
outlive the switch index in the RTL (`REGNO_LAST_UID`), or index the
switch from something cse does not merge with the compare register. No C
spelling found. Do not re-run the same permuter search; a new attempt
needs a shape that changes the compare tree's pseudos.

### Proposed learning (round 12)

- **A store of a case's own constant uses the switch index's register:
  cse's jump equivalence merges the compare constant's pseudo into the
  index's class, and `make_regs_eqv` keeps the longer-lived index
  canonical.** `-fno-cse-follow-jumps` shows retail's register choice but
  leaves an extra `li`. (UpdateToolView, still open)

## Round 13 (delta): MATCHED

Inherited body rebuilt first: 154/157, `insertions 0 / deletions 0`.

The residue was a misreading, not a register choice. Retail's case 2 is

```
beq  $v1, $v0, .L800358D0
 addiu $v0, $zero, 0x3      # delay slot: runs on the taken edge too
.L800358D0:
lhu  $v1, D_80095A22
sb   $v0, gToolViewStep        # stores 3
```

The `sb $v0` stores 3, the value the case-3 compare loads in the beq's
delay slot, not 2. Case 2 advances the state to 3.

| C (case 2) | result |
| --- | --- |
| `gToolViewStep = 2;` (inherited) | 154/157 |
| `switch` on a local `s`, `s = 0;` in case 2 (kills the index's equivalence) | store moves to the compare register, which then lives into case 2 and steals `$a0`: 32/157 (shifted) |
| as above, with `s = (u16)D_80095A22; ...; gToolMode = s;` | 18/157 (load hoisted) |
| `gToolViewStep = 3;`, either statement order | one word long: a `li $v0, 3` stays in case 2 |
| `gToolViewStep++;` | **157/157, build OK** |

With `++`, cse knows the index is 2 on that edge and folds the
load-add to a constant 3; reorg then fills the beq's delay slot with the
fall-through `li $v0, 3` and the case's own `li` goes as redundant. The
explicit `= 3` keeps its `li`. About 10 builds, no permuter.

### Proposed learning

- **A store whose register is loaded in the delay slot of the branch that
  reaches it stores the delay slot's value.** Read that `li` before
  calling the diff a register choice: here `sb $v0` after `beq ...; li $v0,
  3` stored 3, and the source was `state++` on the case for 2.
  (UpdateToolView)
