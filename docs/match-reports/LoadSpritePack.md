# LoadSpritePack — MATCHED 76/76 (round 8, alpha)

> Renamed from `func_8002B7C8` on 2026-10-10 (tools/rename.py). Address 0x8002b7c8.

Unit `src/object.c`. Loads every TIM of a packed directory
(LoadTim), waits for the upload, and registers each as a texture
through DefineSprite, numbered from `id` on: the texture page from the
image's VRAM position, u from x within the page scaled by the pixel mode,
v from y, the size and the CLUT position.

```c
#define sTim ((TimInfo *)gTimInfo)

void LoadSpritePack(DirEnt16 *dir, u16 id) {
    DirEnt16 *e;
    u16 i;
    u16 n;
    s32 tp;
    TimInfo *t;

    e = dir;
    n = dir->count;
    for (i = 0; i < n; i++) {
        LoadTim((u8 *)dir + e->offset);
        DrawSync(0);
        tp = GetTPage(0, 0, sTim->pixRect.x, sTim->pixRect.y);
        t = sTim;
        /* MATCHING: x mod 64 spelled out; % narrows to a halfword. */
        DefineSprite(id, 0, (u8)t->unk1C, (u8)t->pixRect.h, (u8)tp,
                      (u8)((t->pixRect.x - t->pixRect.x / 64 * 64) * t->unk1E),
                      (u8)(t->pixRect.y % 256), t->clutRect.x, (u16)t->clutRect.y);
        id++;
        e++;
    }
}
```

## Path to the match (6 builds)

- The loop shape (guard, `u16` counter and bound compared after
  `andi`, entry pointer bumped by 16) and the reload of `gTimInfo`
  after each call came out right from the start.
- `x % 64 * scale` is two words long: 2.8.1 shortens the `%` of an
  `s16` to a halfword operation and re-extends the result (`sll 16; sra
  16`) before the multiply. Retail has none: `x - x / 64 * 64` gives the
  same `sra 6; sll 6; subu` without the narrowing. `scale * (x % 64)` is
  worse (28/76).
- With TimInfo's `s16` CLUT y the last argument loads `lh`; retail's
  `lhu` is a `(u16)` cast (the `s32` parameter makes no conversion of
  its own).

## Declarations moved (stated for the head)

- `TimInfo` (the unpacked TIM header that `gTimInfo` points at) moved
  from `src/model.c` to `include/model.h`, unchanged; this unit
  includes the header and has its own `sTim` macro like model.
- `s32 LoadTim(void *data);` moved from a forward declaration in
  `src/model.c` to `include/model.h` (declcheck LOCAL once a
  second unit uses it). model had no runner this round.

### Proposed learning

`x % 2^k` on an `s16` that feeds a wider multiply, where retail has no
`sll 16; sra 16` after the `subu`: write `x - x / 2^k * 2^k`. The `%` is
shortened to a halfword and re-extended. (LoadSpritePack)
