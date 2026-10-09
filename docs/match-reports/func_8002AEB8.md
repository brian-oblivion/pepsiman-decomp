# func_8002AEB8 — MATCHED 45/45 (round 6, alpha; permuter lead)

Unit `src/code_1a098.c`. Path following: `D_800958A0` points at a table of
8-byte path points (x, z, then a direction dx, dz). The caller's current
segment `u->seg` moves forward when `u` is on the far side of the next
point (dot product of the next point's direction with `u - next` is
non-negative) and back when it is behind its current point (the negated
direction against the current point). Returns the new segment, also
written back. Called only through a pointer (no `jal` to it in the asm).

```c
extern PathPt *D_800958A0;

s32 func_8002AEB8(PathUser *u) {
    s32 unused[2];
    s32 i;
    s32 d;
    PathPt *next;
    PathPt *pt;

    i = u->seg;
    next = (PathPt *)(i * 8 + (u32)D_800958A0) + 1;
    d = next->dx * (u->x - next->x) + next->dz * (u->z - next->z);
    if (d >= 0) {
        i++;
    }
    pt = (PathPt *)(i * 8 + (u32)D_800958A0);
    d = -pt->dx * (u->x - pt->x) + -pt->dz * (u->z - pt->z);
    if (d >= 0) {
        i--;
    }
    u->seg = i;
    return i;
}
```

`PathPt` and `PathUser` are new unit-local views. `D_800958A0` is a
pointer scalar (`lui $a2` / `lw $a2, %lo` in one register). The leaf's
8-byte frame with no stack access is the `s32 unused[2]` lever.

About 12 builds plus one permuter search.

- `&D_800958A0[i + 1]` / `&D_800958A0[i]`: base-first `addu` and a size
  change. The integer sum `(PathPt *)(i * 8 + (u32)D_800958A0)` (`+ 1` for
  the next point) gives retail's index-first `addu` and `addiu 8`.
- With one `pt` reused for both points: 44/45 words, one short. Retail
  copies the parameter out of `$a0` at entry (`move $t1, $a0`) and uses
  `$a0` for a product; the build kept `u` in `$a0`, and every other
  register was shifted. A sum local, a duplicated tail and `return u->seg
  = i;` did not change it.
- Permuter (Gate 3: check 1 the scaffold compiled, base score 325;
  check 2 `--stack-diffs` 0 insertions / 1 deletion; check 3 the real tree
  through asm-differ showed the same single deletion, AGREE; funcdiff's
  positional 1/1 counted the next function's first word because the build
  was one word short). `timeout 600`, `-j 6`, exit 124 (timeout) after
  109047 iterations, best 20. The 20-scorer computed the first test before
  assigning `pt`, so the first point no longer shared `pt`'s pseudo; built
  in the real tree it gave retail's `move` and every register except the
  two sums (`$v0` where retail has `$v1`).
- Clean form: a separate `next` pointer for the first point. Same as the
  20-scorer.
- Then each sum in a local `d`: 45/45, `OK`.

### Proposed learning

- One pointer local reused for two different objects can keep a parameter
  in its argument register when retail copies it out at entry
  (`move $t1, $a0`); a local per object lets the allocator move it.
- A comparison's sum held in a local (`d = ...; if (d >= 0)`) can land in
  `$v1` where the inline expression lands in `$v0`.
