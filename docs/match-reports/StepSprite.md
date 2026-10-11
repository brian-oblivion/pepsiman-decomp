# StepSprite — MATCHED 64/64 (round 8, alpha)

> Renamed from `func_8002C894` on 2026-10-11 (tools/rename.py). Address 0x8002c894.

Unit `src/object.c`. Runs a StageSprite record's step (DrawObjectSprite), then
asks the position query (CollidePlayer) about a point half the record's
height below it with a third of `|unk20|` as range; keeps bit 0 of the
answer in `unk25`, and on a hit while flagged (bit 7 of `unk26`) in state 2
moves to state 3 (keeping bit 6, clearing bit 7), bumps the counter at 0
and resets the words at 0x10..0x18, `unk14` from the fixed object's y.

```c
void StepSprite(s16 id, StageSprite *r) {
    Query30 q;
    s32 v;

    DrawObjectSprite(id, r);
    q.pos.x = r->unk4;
    q.pos.y = r->unk8 - (s16)r->unk1E / 2;
    q.pos.z = r->unkC;
    r->unk24 = 1;
    q.unk18 = 1;
    q.unk14 = (r->unk20 < 0 ? -r->unk20 : r->unk20) / 3;
    v = CollidePlayer(&q) & 1;
    r->unk25 = v;
    if (v) {
        if (r->unk26 & 0x80) {
            if ((r->unk26 & 0x3F) == 2) {
                r->unk10 = 0;
                r->unk0++;
                r->unk26 = (r->unk26 & 0x40) | 3;
                r->unk14 = ((VECTOR *)D_8009EEC0)->vy;
                r->unk18 = 0;
            }
        }
    }
}
```

`StageSprite` is declared in this unit with the fields above (editor has its
own, smaller view of the same 0x3C-byte record). Retail passes `id` to
DrawObjectSprite through `andi 0xFFFF`: that callee takes a `u16` in this
unit's view.

## The lever: the global read as a struct member

Everything but the last block matched on build 1 (44/64, one word short):
retail loads the counter first, stores the flags byte, then loads
`D_8009EEC0`'s y, then stores `unk18` and `unk14`; the build hoisted the
`unk18` store above the flags and filled the `bne` delay slot with the
global's `lui`. Invariant under every statement order tried, `s32` with no
return, and `&=`/`|=` for the flags (all 44/64). Byte-wide bitfields for
the flags (`u8 state:6, b6:1, b7:1`, also inside a one-byte sub-struct)
give word accesses (`lw`/`and`/`sw` at 0x24), 39/64.

Reading the y as `((VECTOR *)D_8009EEC0)->vy` instead of `*(s32
*)(D_8009EEC0 + 4)` matched (build 8). A struct member read is ordered
against the record's member stores differently from a plain word read.

## Declaration note (needs the head or bravo)

declcheck reports LOCAL for editor.c:82-83, that unit's own
prototypes of StepSprite (now defined here) and DrawObjectSprite (used
here). The definition keeps editor's `(s16, StageSprite *)` type, so there is
no TYPE hit. editor has DrawObjectSprite as `(s16, StageSprite *)`; this unit
has it as `(u16, StageSprite *)` (retail's `andi 0xFFFF`). editor was live
under bravo this round, so I left its file alone and asked on the
broadcast for a `MATCHING:` per-unit-view note there.

### Proposed learning

A global read that must stay below stores through a pointer parameter:
read it as a struct member (`((VECTOR *)D)->vy`); a plain `*(s32 *)(D +
4)` lets the scheduler move the parameter's stores above it.
(StepSprite; compare SetCurrentBlock, where a byte-pointer read was the
fix in the opposite direction)

## Round 9 (charlie): now a wrapper

UpdateActiveSprites inlines this body, so it moved into `static __inline__
queryRec(s16 id, StageSprite *r)` ahead of UpdateActiveSprites, and StepSprite is
`queryRec(id, r);`. Still 64/64.
