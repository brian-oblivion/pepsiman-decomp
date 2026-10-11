# ProbePosition — MATCHED 20/20 (round 3, alpha)

> Renamed from `func_8002D0F0` on 2026-10-11 (tools/rename.py). Address 0x8002d0f0.

Unit `src/object.c`. Fills a 0x30-byte argument block on the stack
(`Query30`: a three-word position, a word at 0x14 and a byte at 0x18) and
returns `CollidePlayer`'s result as a signed byte.

```c
s8 ProbePosition(Vec3 *pos) {
    Query30 q;

    q.pos.x = pos->x;
    q.pos.y = pos->y;
    q.pos.z = pos->z;
    q.unk18 = 0;
    q.unk14 = 50;
    return CollidePlayer(&q);
}
```

The position is copied field by field: retail's `lw`/`sw` through `$v0`,
`$v0` and `$v1` is the per-field shape (a struct assignment gives a block
move through other registers). With `unk14` assigned before `unk18` the
build was 18/20 with the two stores swapped; assigning `unk18` first
matched (independent stores to a local come out in source order, as in
docs/LEARNINGS.md). The `sll`/`sra 24` on the result is the `s8` return
type.
