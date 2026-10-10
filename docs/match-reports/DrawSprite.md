# DrawSprite — MATCHED (90/90), round 6, runner charlie

> Renamed from `func_8001B354` on 2026-10-10 (tools/rename.py). Address 0x8001b354.

Unit `src/code_a0bc.c`. Draws sprite `id` of the table `gSpriteDefs` at
`pos`: builds a SPRT (0x64 with a colour, 0x65 raw-textured without one,
semi-transparent when the colour's first byte is not -1) and a DR_TPAGE
after it in the primitive buffer `GsOUT_PACKET_P`, links both into
`ot->org[mode]` and advances the buffer. The first colour byte doubles as
the blend mode (`<< 5` into the tpage word).

```c
void DrawSprite(u16 id, SVECTOR *pos, CVECTOR *color, s32 mode, GsOT *ot) {
    Sprite8 *e;
    u32 *p;
    u32 *q;
    u32 *tag;
    u32 c;

    e = &gSpriteDefs[id];
    p = (u32 *)GsOUT_PACKET_P;
    p[1] = color != NULL ? 0x64000000 | (color->cd << 16) | (color->b << 8) | color->g : 0x65000000;
    if (color != NULL && (s8)color->r != -1) {
        p[1] |= 0x02000000;
    }
    p[2] = (pos->vy << 16) | (u16)pos->vx;
    p[3] = (e->unk5 << 8) | e->unk4 | (e->unk2 << 16);
    p[4] = (e->unk7 << 16) | e->unk6;
    tag = (u32 *)ot->org + (u16)mode;
    p[0] = (*tag & 0xFFFFFF) | 0x04000000;
    q = p + 5;
    if (color != NULL && (s8)color->r != -1) {
        c = (s8)color->r << 5 | 0xE1000600;
        q[1] = e->unk0 | c;
    } else {
        q[1] = e->unk0 | 0xE1000600;
    }
    q[0] = ((u32)p & 0xFFFFFF) | 0x01000000;
    *tag = (u32)q & 0xFFFFFF;
    GsOUT_PACKET_P = (u8 *)(q + 2);
}
```

The signature is runner delta's caller-side prototype (include/code_a0bc.h
on runner/delta); this definition agrees with it.

## Attempts (about 25 builds)

1. Straight if/else for the colour word, `unk0 | ((r << 5) | K)` for the
   tpage word: 73/90, both words regrouped (`cd<<16 | 0x64000000` first;
   `unk0 | 0xE1000600` first).
2. Parenthesising either word in retail's grouping: no effect; cc1 folds
   the constant out. `+` instead of `|` keeps the grouping but emits
   `addu` (85/90).
3. Tpage term through a local, `c = r << 5 | K; q[1] = e->unk0 | c;`:
   tpage word exact (89/90 with the `+` colour word).
4. About a dozen colour-word spellings (locals, `U` suffix, casts, shifted
   groups) in the if/else: none better than 88/90.
5. The colour word as a ternary, `color ? K | cd<<16 | b<<8 | g :
   0x65000000`: 90/90.

`GsOUT_PACKET_P` (the primitive buffer cursor, reached by five units) was
added to `include/common.h` as `extern u8 *GsOUT_PACKET_P;`, the type runner
delta declared locally in code_308ec this round; that local becomes a
declcheck MULTI at merge and should be deleted then.

### Proposed learning

- **An OR of shifted bytes and a constant that cc1 regroups (the constant
  OR'd into the wrong term) in an if/else: write the value as a ternary**,
  `x = p ? K | a<<16 | b<<8 | c : K2;`. A local holding `term | K` fixes the
  same regrouping in a plain statement. (DrawSprite)
