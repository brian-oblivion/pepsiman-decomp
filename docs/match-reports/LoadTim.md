# LoadTim — MATCHED (67/67), round 7, runner alpha

> Renamed from `func_80017774` on 2026-10-10 (tools/rename.py). Address 0x80017774.

Unit `src/model.c`. Unpacks a TIM through `ReadTimHeader` into the
TimInfo at `gTimInfo`, uploads the pixels and, when present, the CLUT,
and returns the address just past the pixel data (width x height x the
mode's scale, added to the pixel pointer), or -1 when the TIM is bad.
Its prototype in the unit changed from `void` to `s32` (LoadTimPack
ignores the result; still 31/31).

```c
#define sTim ((TimInfo *)gTimInfo)

s32 LoadTim(void *data) {
    RECT rect;
    RECT clut;
    TimInfo *info;
    u16 w;
    u16 h;

    if (ReadTimHeader(data) == -1) {
        return -1;
    }
    w = sTim->pixRect.w;
    h = sTim->pixRect.h;
    rect.x = sTim->pixRect.x;
    rect.y = sTim->pixRect.y;
    rect.w = sTim->pixRect.w;
    rect.h = sTim->pixRect.h;
    LoadImage(&rect, sTim->pixel);
    info = sTim;
    if (info->hasClut == 1) {
        clut.x = info->clutRect.x;
        clut.y = info->clutRect.y;
        clut.w = info->clutRect.w;
        clut.h = info->clutRect.h;
        LoadImage(&clut, info->clut);
    }
    return (s16)w * (s16)h * sTim->unk1E + (s32)sTim->pixel;
}
```

## Attempts (2)

1. A `TimInfo *info` local reloaded from the global before each block:
   52/67, only registers wrong (the pointer in `$a0`, retail `$v0` in the
   first block and `$v1` in the last).
2. The first and last blocks read through the global each time (`sTim`
   macro), the CLUT block through the local: 67/67.

Retail's `lhu` into `$s0`/`$s1` and the later `sll`/`sra 16` are `u16`
locals read through `(s16)` casts; the width and height are loaded twice
(once for the locals, once for the rect), so they are separate reads in
the source.

### Proposed learning

- **A global struct pointer reloaded per block, held in `$v0`/`$v1`
  rather than `$a0`: read it through the global each time**, `#define
  sTim ((T *)G)`, not a pointer local. (LoadTim)
