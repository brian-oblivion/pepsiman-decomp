# DecodeNextMovieFrame — MATCHED 38/38 (round 4, echo)

> Renamed from `func_80041D88` on 2026-10-10 (tools/rename.py). Address 0x80041d88.

Unit `src/code_31cec.c`. Waits for the next movie frame's bitstream
(`GetNextMovieFrame`, Sony's sample `strNext`), flips the VLC buffer index,
VLC-decodes the frame into it and frees the ring sector. Returns -1 when
the volatile countdown `gMovieTimeout` (0x800000) runs out. This is the
`strNextVlc` routine of Sony's movie playback sample, compiled into the
game's unit (not a linked Sony object).

```c
#include "libpress.h"
#include "libgte.h"
#include "libgpu.h"

typedef struct {
    u_long *vlcbuf[2]; /* 0x00 */
    s32 vlcid;         /* 0x08 */
    u_short *imgbuf;   /* 0x0C */
    RECT rect[2];      /* 0x10 */
    s32 rectid;        /* 0x20 */
    RECT slice;        /* 0x24 */
    s32 isdone;        /* 0x2C */
} DECENV;

extern volatile s32 gMovieTimeout;
u_long *GetNextMovieFrame(DECENV *dec);

s32 DecodeNextMovieFrame(DECENV *dec) {
    u_long *next;

    gMovieTimeout = 0x800000;
    while ((next = GetNextMovieFrame(dec)) == NULL) {
        if (--gMovieTimeout == 0) {
            return -1;
        }
    }
    dec->vlcid = dec->vlcid == 0;
    DecDCTvlc(next, dec->vlcbuf[dec->vlcid]);
    StFreeRing(next);
    return 0;
}
```

## Levers

- **`gMovieTimeout` is volatile.** Every retail access (here and in
  GetNextMovieFrame, WaitMovieFrameDecoded) stores the decremented value and loads it
  again before the test. Declared unit-locally with a `MATCHING:` note.
- **Loop shape: `while ((next = f()) == NULL) { if (--cnt == 0) return
  -1; }` then the success code.** Retail has the call at the loop head,
  `beqz` to the countdown block at the bottom and the success block
  fall-through ending `j` to the epilogue. Both `do { if (next) { ...;
  return 0; } } while (--cnt);` and the `continue` variant put the
  success block after the loop and the countdown first (12/38).

### Proposed learning

- **Call at a loop's head, `beqz` forward to a block that branches back:
  a `while ((x = f()) == 0) { ... }` with the work after the loop.** A
  `do`/`while` whose body returns on success lays the blocks out the
  other way round. (DecodeNextMovieFrame)
