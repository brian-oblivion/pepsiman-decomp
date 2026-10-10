# ResetRumble — MATCHED (26/26), round 1, runner charlie

> Renamed from `func_800154C4` on 2026-10-10 (tools/rename.py). Address 0x800154c4.

Points `gRumblePattern` at the table `gRumblePatterns` and unpacks its 10-byte header
into seven halfword globals, clearing an eighth.

```c
typedef struct {
    u16 unk0;
    u16 unk2;
    u8 unk4;
    u8 unk5;
    u8 unk6;
    u8 unk7;
    u16 unk8;
} RumbleStep;

extern RumbleStep *gRumblePattern;
extern u16 gRumbleIndex, gRumbleWait, gRumbleTime, gRumbleSmall,
           gRumbleLarge, gRumbleNext, gRumbleRepeat;   /* one per line in src */

void ResetRumble(void) {
    RumbleStep *hdr;

    hdr = (RumbleStep *)gRumblePatterns;
    gRumblePattern = hdr;
    gRumbleIndex = 0;
    gRumbleWait = hdr->unk0;
    gRumbleTime = hdr->unk2;
    gRumbleSmall = hdr->unk5;
    gRumbleLarge = hdr->unk4;
    gRumbleNext = hdr->unk8;
    gRumbleRepeat = hdr->unk6;
}
```

First attempt. `gRumblePatterns` keeps `include/common.h`'s `u16 []` declaration
(not edited this round); the cast to `RumbleStep *` gives retail's split
`lui` / `addiu %lo` base, with the first field read straight off `%lo`. The
eight destinations are `.sdata` globals outside `config/gp-symbols.txt`, so
plain complete externs give the `lui $at` pairs. All are stored with `sh`
and never loaded here, so their signedness is unknown (`u16` chosen).

### Proposed learning

When `include/common.h` is next edited: `gRumblePatterns` is read as a struct
(10-byte header: two `u16`, four `u8`, one `u16`), not as a `u16` array, by
this unit.
