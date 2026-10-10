# PlayStageMusic — MATCHED (38/38) after per-unit gp lists

> Renamed from `func_80014C58` on 2026-10-10 (tools/rename.py). Address 0x80014c58.

**Resolved by the head after round 5:** maspsx now gets each unit's own gp
list (`config/gp/<unit>.txt`, from tools/gpsyms.py): only the symbols retail
reaches through `$gp` from that unit's code. `gSoundFade` is reached `$gp` from code_31cec, `lui` from main,
so it is no longer gp-relative here and the body is live. Original title:
PlayStageMusic — STALL: 2 words short, 22/38, first diff at word 22 (`lui $v0` before `lh gSoundFade`, built as `lh %gp_rel`) — round 4, runner delta

Unit `src/main.c`. Copies a 15-byte table (`D_80010000`) to the stack,
clears the halfword flag `gSoundFade` if it is set, and calls
`PlayCdTrack(table[idx], 1)`.

**The C is right; the shared gp list blocks it.** `gSoundFade` is in
`config/gp-symbols.txt` because code_31cec reaches it `%gp_rel` (retail
`PlayBgm`, `UpdateSoundFade`, and the matched C setter in that unit).
This function reaches it absolutely: `lui $v0` / `lh $v0, %lo` and `lui
$at` / `sh $zero, %lo`. maspsx's gp list is global, so a plain extern is
made gp-relative here too (two words short), and declaring it `[]` makes
cc1 split it into `lui $a1` / `lh $v0, %lo($a1)` / `sh $zero, %lo($a1)`,
which is neither. Proof: the same pipeline (Makefile flags, maspsx with
`--nop-at-expansion`) with a copy of the gp list minus `gSoundFade` gives
38/38, relocations masked. Parked under `NON_MATCHING`; posted to the
broadcast.

```c
#if 0
typedef struct {
    u8 b[15];
} Bytes15;

extern u8 D_80010000[];
extern s16 gSoundFade;

void PlayCdTrack(s32 a, s32 b);

void PlayStageMusic(u8 idx) {
    Bytes15 tbl;

    tbl = *(Bytes15 *)D_80010000;
    if (gSoundFade != 0) {
        gSoundFade = 0;
    }
    PlayCdTrack(tbl.b[idx], 1);
}
#endif
```

The table copy is a struct copy from the unit's own rodata (LEARNINGS, "An
8-byte local filled with lwl/lwr"): a 15-byte one-member struct gives
retail's three `lwl`/`lwr` words and three `lb`/`sb` bytes exactly. The
data stays `INCLUDE_RODATA` at the top of the unit, where retail has it
ahead of `main`'s jump tables; an initialised local array would make cc1
emit its own copy later in `.rodata`. The extern and prototype sit inside
the `#ifdef` so declcheck does not see a second unit declaring
`gSoundFade` while the body is parked.

### Proposed learning

- A 15-byte block copy (`lwl`/`lwr` x3 then `lb`/`sb` x3) is a struct copy
  of a `u8 b[15]` struct; the tail bytes come out as signed `lb`.
- Retail reaches some `.sdata` symbols gp-relative from one unit and
  absolutely from another (`gSoundFade`: code_31cec vs main). A global gp
  list cannot express that; a per-unit override is needed.
