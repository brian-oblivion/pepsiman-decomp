# UpdateObjectEntry — MATCHED 109/109 (round 9, charlie)

> Renamed from `func_8002A7D8` on 2026-10-11 (tools/rename.py). Address 0x8002a7d8.

Unit `src/object.c`. Runs DrawObjectModel on the ObjectModel entry, clears
`r->unk40`, then places each of the entry's first-buffer records
(`gLivePoints`, 0x2C bytes, `Placed2C`: UpdatePlacedPos) and ORs bit 0 of
CollidePlayer's answer into `r->unk40`. When the game head's byte 2 is set
it runs collision's PlaceCollisionQuad / GetQuadHeightUnderPlayer over the entry's
second-buffer records (`gLiveQuads`, 0x4C bytes) and copies a hit
(`D_80095898 == 1`, with the word `D_8009589C`) to the head at 0x3B8 and
0x3CC.

```c
/* The result of a second-buffer record's test: a flag, then a word. */
extern s8 D_80095898;  /**< 1 on a hit */
extern s32 D_8009589C; /**< the word reported with a hit */
/* MATCHING: per unit this round; hit_tool declares both the same way. */
extern u8 gLivePoints[]; /**< 0x2C-byte placed records (Placed2C) */
extern u8 gLiveQuads[]; /**< 0x4C-byte records */
/* collision's tests of a 0x4C-byte record; types not yet known. */
void PlaceCollisionQuad(void *rec);
void GetQuadHeightUnderPlayer(void *rec, s8 *out);

/** @brief Updates the entry `rec` and places and tests its records: the
 *         first-buffer ones collect a hit bit into `r->unk40`, and, when
 *         the game head allows, the second-buffer ones report a hit to it.
 *  @return nothing; the value is undefined. */
/* MATCHING: non-void with no return keeps two delay slots nops (the
 * unk2 test and the second loop's back branch). */
s32 UpdateObjectEntry(ObjectModel *rec, Trap *r) {
    s16 i;
    s16 start;
    s16 n;
    Placed2C *p;
    u8 *q;

    DrawObjectModel(rec, r);
    start = rec->unk72;
    n = rec->unk74;
    r->unk40 = 0;
    if (n != -1) {
        for (i = start; i < start + n; i++) {
            p = &((Placed2C *)gLivePoints)[i];
            UpdatePlacedPos(p);
            r->unk40 |= CollidePlayer((Query30 *)p) & 1;
        }
    }
    if (sGameHead.unk2 != 0) {
        start = rec->unk6E;
        n = rec->unk70;
        if (n != -1) {
            for (i = start; i < start + n; i++) {
                q = gLiveQuads + i * 0x4C;
                PlaceCollisionQuad(q);
                GetQuadHeightUnderPlayer(q, &D_80095898);
                if (D_80095898 == 1) {
                    sGameHead.unk3B8 = D_80095898;
                    sGameHead.unk3CC = D_8009589C;
                }
            }
        }
    }
}```

`GameHead` (unit-local) gained `unk2`, `unk3B8` and `unk3CC`; no offset of
an existing field moved.

Three builds. The first read `rec->unk72`/`unk74` in the `for` and
reloaded them every pass (8/109); retail loads start and count once, so
they are `s16` locals (54/109, length exact, ins/del 2/2). The residue was
two delay slots: retail keeps a `nop` after the `beqz` on `unk2` and in
the second loop's back branch, where the build filled them (the `move
s2, a1` and the next pass's `sll`). `s32` with no `return` closed both
(LEARNINGS, "a near-miss that differs only in delay slots").

Shared changes: `include/object.h`'s prototype is now `s32` (it was
`void`); the callers in editor and hit_tool ignore the value and the
image matches. One `control reaches end` line added to
`config/typeviews-warnings.txt`. `gLivePoints` / `gLiveQuads` are declared
in this unit as hit_tool declares them; declcheck flags hit_tool's two
lines as LOCAL until the merge moves both declarations to `common.h`.

### Proposed learning

- A loop bound read from a struct once, before the loop, with `lhu` +
  `sll/sra` (not `lh`) for the count: the start and count were `s16`
  locals; reading the members in the `for` reloads them every pass across
  the calls. A count that goes straight to the `-1` test from `lh` was
  read the same way, so the spelling alone did not decide `lh` vs `lhu`.
