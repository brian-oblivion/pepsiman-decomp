# UpdateActiveSprites — MATCHED 112/112 (round 9, charlie)

> Renamed from `func_8002BD00` on 2026-10-11 (tools/rename.py). Address 0x8002bd00.

Unit `src/code_1a098.c`. Walks the 100 Rec3C records (`D_800A7898`). For
each live one (`unk0 != -1`) whose Rec5C record (`D_800CF080[unk2C]`) has
byte 0 equal to 1: a record with flag bit 7 runs StepSprite's body
inlined (update, query, state 2 -> 3); a state-3 record runs the drift
step StepDrift and is updated when that returns 1; any other record is
updated with DrawObjectSprite.

```c
/** @brief Updates `r` (id `id`) and queries around it; a hit in state 2
 *         of a flagged record moves it to state 3. Inlined here and by the
 *         out-of-line copy below. */
static __inline__ void queryRec(s16 id, Rec3C *r) {
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

/** @brief Steps every live Rec3C record whose Rec5C record is marked 1:
 *         a flagged one is updated and queried, a state-3 one drifts and is
 *         updated on a drift step, any other one is updated. */
/* MATCHING: the record pointer is taken inside the body. */
void UpdateActiveSprites(void) {
    u32 i;
    Rec3C *r;
    s8 k;

    for (i = 0; i < 100; i++) {
        r = &D_800A7898[i];
        if ((s16)r->unk0 != -1) {
            k = D_800CF080[r->unk2C].unk0;
            if (k == 1) {
                if (r->unk26 & 0x80) {
                    queryRec(r->unk0, r);
                } else if ((r->unk26 & 0x3F) != 3 || (s8)StepDrift((Drifter *)r) == 1) {
                    DrawObjectSprite(r->unk0, r);
                }
            }
        }
    }
}```

`Rec3C` (unit-local) gained `s16 unk2C`; no other offset moved.

Three builds. The first wrote StepSprite's body out in place with the
loop as `for (i = 0, r = D_800A7898; i < 100; i++, r++)`: right blocks,
but the update's `id` load came late and a second pointer biased by 0x18
appeared (3/112). The body as a `static __inline__` helper with an `s16
id` parameter put the `lhu` of `unk0` at the loop head beside the `lh`
test, as retail has it. `r = &D_800A7898[i];` inside the body removed the
biased pointer: 112/112 (LEARNINGS, "a record loop whose pointer stays at
the record start").

StepSprite is now a one-line wrapper of the helper and still matches
(64/64; note added to its report).

### Proposed learning

- An argument load (`lhu`) at the loop head, long before the call it
  feeds, beside a different-width test load of the same field (`lh`): the
  call is an inlined helper whose parameter is evaluated at the call site,
  and the test reads the field through a cast. Confirms the
  inline-helper learning (UpdateLoadScreenModel) on a third pair.
