# PlayBgm — MATCHED 46/46 (round 4, echo)

> Renamed from `func_80042150` on 2026-10-10 (tools/rename.py). Address 0x80042150.

Unit `src/sound.c`. Plays a song: stops and closes the sequence in
slot 0 of `gSeqIds`; unless `gVabError` is set (then returns -1),
opens song `song` of the sequence bank at 0x80101000 (VAB `gBgmVab`),
plays it once, sets its volume and `gBgmVolume` to 100 and clears
`gSoundFade`.

```c
#include "libsnd.h"
extern s16 gSeqIds[];
extern s16 gSoundFade;
extern s32 gVabError;
extern s16 gBgmVab;
extern s16 gBgmVolume;

s32 PlayBgm(u16 song) {
    u32 *hdr;

    SsSeqStop(gSeqIds[0]);
    SsSeqClose(gSeqIds[0]);
    if (gVabError != 0) {
        return -1;
    }
    hdr = (u32 *)0x80101000;
    hdr += song * 4;
    gSeqIds[0] = SsSeqOpen((unsigned long *)(*hdr + 0x80101000), gBgmVab);
    SsSeqPlay(gSeqIds[0], 1, 0);
    gBgmVolume = 100;
    SsSeqSetVol(gSeqIds[0], 100, 100);
    gSoundFade = 0;
}
```

**Deliberate `-Wall` warning:** "control reaches end of non-void
function". Retail returns -1 from the `bnez` delay slot on the early
exit and leaves `$v0` unset on the normal path, so the function is
`s32` with no final `return` (a `return 0;` adds a `move $v0, $zero`;
`void` drops the -1). Needs baselining in
`config/typeviews-warnings.txt`.

## Levers

- **The bank address materialised a second time after the entry address
  overwrote the first:** `hdr = K; hdr += song * 4;` then `*hdr + K`.
  Every single-expression spelling (`*(u32 *)(song * 16 + K) + K`,
  `(u8 *)K + ((u32 *)K)[song * 4]`, an `off` temporary) lets CSE reuse
  the first register for the addend (45 words, 3 differing). Modifying
  the pointer local kills CSE's knowledge that it holds K, so the
  literal addend is loaded again: 46/46. Same family as InitSeSeqs.
- `gVabError` is a plain `s32` reached `lui $v0` / `lw $v0` (not in the
  gp list).
- `SsSeqPlay`'s first argument is the sign-extended `SsSeqOpen` result,
  not a reload: the compiler reuses the assigned value of
  `gSeqIds[0]` without help.
