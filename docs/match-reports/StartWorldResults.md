# StartWorldResults — MATCHED (32/32), round 3, runner bravo

> Renamed from `func_8003E4C4` on 2026-10-10 (tools/rename.py). Address 0x8003e4c4.

Unit `src/code_29f54.c`. Resets four state words (`gResultsStep`,
`gResultsTimer`, `gTallyCans` to 0, `gResultsActive` to 1), then copies the
8-byte pattern `D_800956B8` (`.sdata`, bytes `F0 .. F7 ...`) into `gResultsSlide`
and clears `gResultsSliding`, byte by byte. StartActResults and StartWorldResults are
the same body over `D_800956B0` and `D_800956B8`.

```c
typedef struct {
    u8 b[8];
} Bytes8;

extern s32 gTallyCans;
extern s32 gResultsActive;
extern s32 gResultsTimer;
extern s32 gResultsStep;
extern s8 gResultsSliding[8];
extern s8 gResultsSlide[8];
extern u8 D_800956B8[];

void StartWorldResults(void) {
    s32 i;
    Bytes8 buf;

    buf = *(Bytes8 *)D_800956B8;
    gResultsStep = 0;
    gResultsTimer = 0;
    gResultsActive = 1;
    gTallyCans = 0;
    for (i = 0; i < 8; i++) {
        gResultsSlide[i] = buf.b[i];
        gResultsSliding[i] = 0;
    }
}
```

## Levers

- **The 8-byte local.** Retail copies `D_800956B8` to `0($sp)` with `lwl`/`lwr`
  pairs: a block move of an alignment-1 aggregate. A struct of `u8 b[8]`
  assigned from `*(Bytes8 *)D_800956B8` gives it. The source is reached
  `lui $v0` / `addiu $t3, $v0, %lo`: cc1's split, so an array of unknown
  size.
- **`gResultsSlide` and `gResultsSliding` are arrays of KNOWN size 8.** Retail loads
  each base with `lui $a3` / `addiu $a3, $a3, %lo(sym)`: one register for
  both halves, the assembler's expansion of `la`. cc1 emits `la` for an
  object it classes as small data (-G8). With `extern s8 gResultsSlide[];` (and
  likewise `gResultsSliding`) the build was 28/32: cc1 split each address
  (`lui $v0` / `addiu $a3, $v0`). With `[8]` it is byte-exact. The other
  retail accesses agree: `UpdateActResults` and `UpdateWorldResults` read
  `gResultsSlide[0]` with `lb ... %gp_rel` (so `s8`) and store `gResultsSliding[0]`
  with `sb ... %gp_rel`.
- `s32 i` with `slti`: the counter is signed.

### Proposed learning

- **One register for both halves of a `lui`/`addiu` ADDRESS load: an object
  of known size at most 8 (`extern T sym[N];`), which cc1 reaches with
  `la`.** `extern T sym[];` gives cc1's split into two registers.
  (StartActResults, StartWorldResults)
- **An 8-byte local filled by `lwl`/`lwr` from a global: a struct copy of an
  alignment-1 struct** (`typedef struct { u8 b[8]; } Bytes8; buf = *(Bytes8
  *)D;`).
