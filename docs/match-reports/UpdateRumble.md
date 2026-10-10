# UpdateRumble — MATCHED 126/126 (round 8, echo)

> Renamed from `func_80014DB0` on 2026-10-10 (tools/rename.py). Address 0x80014db0.

Unit `src/main.c`. Per-frame step of a record player over the 10-byte
`RumbleStep` table at `gRumblePattern`: while `gRumbleTime` frames remain and
`gRumbleEnabled == 1`, it re-sends the two bytes `gRumbleSmall`/`gRumbleLarge`
through `SetPadVibration`; otherwise it clears them and counts down
`gRumbleWait`. When that runs out it either repeats the current record
(`gRumbleRepeat` times) or moves to the record `gRumbleNext` names.

The RumbleStep typedef and its globals moved up from StartRumble to
here (no byte change). `gRumbleNext` is now declared `s16` (was `u16`; only
stores elsewhere in the unit, so no other byte changes).

```c
extern u8 gRumbleEnabled;

void SetPadVibration(s32 offset, u8 a, u8 b);

/**
 * @brief Selects record @p index of @p tbl and unpacks it into the record
 *        globals. The same body as the out-of-line selector below; inlined
 *        with the table head as @p tbl, its store of the head drops out.
 */
static __inline__ void setRecord(RumbleStep *tbl, u16 index) {
    u8 *rec;

    gRumblePattern = tbl;
    tbl += index;
    gRumbleIndex = index;
    gRumbleWait = tbl->unk0;
    gRumbleTime = tbl->unk2;
    /* MATCHING: byte-pointer reads keep each load below the prior store. */
    rec = (u8 *)tbl;
    gRumbleSmall = rec[5];
    gRumbleLarge = rec[4];
    gRumbleNext = *(u16 *)(rec + 8);
    gRumbleRepeat = rec[6];
}

/**
 * @brief setRecord's twin for repeating the current record: the same
 *        unpacking, with the repeat count @p count stepped down instead of
 *        reloaded. Inlined, so its parameter conversions survive.
 */
static __inline__ void repeatRecord(RumbleStep *tbl, u16 index, u8 count) {
    u8 *rec;

    gRumblePattern = tbl;
    tbl += index;
    gRumbleIndex = index;
    gRumbleWait = tbl->unk0;
    gRumbleTime = tbl->unk2;
    rec = (u8 *)tbl;
    gRumbleSmall = rec[5];
    gRumbleLarge = rec[4];
    gRumbleNext = *(u16 *)(rec + 8);
    gRumbleRepeat = count - 1;
}

/* MATCHING: s32 with a bare return, so the `bgtz` keeps its `nop` slot. */
/* MATCHING: s32 with a bare return, so the `bgtz` keeps its `nop` slot. */
s32 UpdateRumble(void) {
    if ((s16)gRumbleTime > 0 && gRumbleEnabled == 1) {
        SetPadVibration(0, gRumbleSmall, gRumbleLarge);
        gRumbleTime--;
        return;
    }
    gRumbleTime = 0;
    gRumbleSmall = 0;
    gRumbleLarge = 0;
    SetPadVibration(0, 0, 0);
    if ((s16)gRumbleRepeat > 0) {
        if ((s16)--gRumbleWait <= 0) {
            if ((s16)gRumbleRepeat < 2) {
                gRumbleRepeat = 0;
                if (gRumbleNext != 0) {
                    setRecord(gRumblePattern, gRumbleNext);
                }
            } else {
                repeatRecord(gRumblePattern, gRumbleIndex, gRumbleRepeat);
            }
        }
    } else {
        gRumbleWait = 0;
    }
}
```

About twenty builds. The path:

- Straight-line C reached the right shape quickly, but retail carries
  redundant zero-extensions (`lhu` + `andi 0xFFFF` on the index in both
  arms, `lbu` + `andi 0xFF` on the repeat count) and the first arm stores
  `gRumbleIndex` from the raw `lhu` after the record's first two loads. That
  is StartRumble's own body (compare its retail: `andi v1, a1, 0xFFFF`,
  `sh a1` to `gRumbleIndex` after the two `lhu`). Writing the arm as an
  inline copy of it, `setRecord(gRumblePattern, gRumbleNext)`, matched the arm
  outright: the inlined `u16` parameter keeps its `andi`, and
  `gRumblePattern = gRumblePattern` is deleted as a store of the value just
  loaded. A plain local or a `(u16)` cast never kept the `andi`.
- The second arm the same way, with a `u8 count` parameter for the
  decrement. Its last residue was the position of the count's `lbu`: with
  the `gRumbleRepeat = count - 1` store written before the `gRumbleNext` store
  the load scheduled early; written last, it lands where retail has it.
- `gRumbleNext` is tested with its own `lh` and indexed with a separate
  `lhu`: a signed declaration with a plain `!= 0` test. With `u16` and an
  `(s16)` cast the test folds into the `lhu`.
- The `bgtz` after the `gRumbleWait` decrement keeps a `nop` slot: `s32`
  with a bare `return;` (LEARNINGS, forward-branch variant).
- StartRumble written as `setRecord(tbl, index);` does NOT match
  (15/29); it keeps its own copy of the body.

### Proposed learning

- Redundant `andi 0xFFFF` / `andi 0xFF` right after an `lhu` / `lbu` of a
  global: a narrow parameter of an INLINED helper (2.8.1 inlines at RTL
  level and keeps the parameter conversion). Look for an existing function
  with the same body; here StartRumble, whose global-head store vanishes
  when inlined with the head as argument.
