# ClearCanGroups — MATCHED 11/11 (round 1, alpha)

> Renamed from `ClearFlags128` on 2026-10-11 (tools/rename.py). Address 0x8002d140.

> Renamed from `func_8002D140` on 2026-10-11 (tools/rename.py). Address 0x8002d140.

Unit `src/object.c`. Clears the 128 bytes of `gCanGroupOn` (declared in
the unit as `extern u8 gCanGroupOn[]`: retail reaches it as a compiler-split
`lui $v0` / `addiu $a0,$v0,%lo`, hoisted out of the loop).

```c
s32 ClearCanGroups(void) {
    u32 i;

    /* MATCHING: non-void with no return ... */
    for (i = 0; i < 128; i++) {
        gCanGroupOn[i] = 0;
    }
}
```

## The lever: a non-void return type

Retail's `bnez` back to the loop top has a `nop` in its delay slot. The
`void` version is 9/11: reorg fills that slot from the loop target with
`addu $v0,$v1,$a0` and the branch targets the `sb`. Declaring the function
`s32` with no `return` makes `$v0` (the return register) live at the exit,
so an insn that sets `$v0` can no longer be hoisted into a slot that also
runs on the fall-through path. GCC warns "control reaches end of non-void
function"; the source was probably a K&R implicit-`int` function.

Negatives at 9/11 (void): `do/while`; `volatile u8 gCanGroupOn[]`; a
pointer temp `p = &gCanGroupOn[i]; *p = 0;`. A `goto` loop is 7/11 (the base
address is no longer hoisted).

The unit's `ShowRevealedCans` and `RevealCanGroup` show the same `nop` slot
after a loop whose top sets `$v0` or `$v1` from a load; try a non-void
return there first.

### Proposed learning

A `nop` in a loop's back-branch delay slot where the build fills it from
the loop target, with the stolen insn writing `$v0`: the function is
non-void (returns int without a `return`, likely K&R implicit int). The
live return register blocks reorg's fill from the target.
