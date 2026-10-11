# MarkCpuTime — MATCHED 49/49 (round 5, bravo)

> Renamed from `func_8003E29C` on 2026-10-10 (tools/rename.py). Address 0x8003e29c.

Unit `src/stage.c`. Appends an entry to a 16-slot queue: if the count
`gCpuMarkCount` is below 16, stores `VSync(1)` (the horizontal-blank counter)
in the `s32` table `gCpuMarkTimes` and the three arguments in the byte tables
`sCpuMarkR`, `sCpuMarkG` and `sCpuMarkB`, then bumps the count.

```c
#include "libetc.h" /* VSync */

extern s32 gCpuMarkCount;
extern s32 gCpuMarkTimes[];
extern u8 sCpuMarkR[];
extern u8 sCpuMarkG[];
extern u8 sCpuMarkB[];

void MarkCpuTime(s32 a, s32 b, s32 c) {
    if (gCpuMarkCount < 16) {
        gCpuMarkTimes[gCpuMarkCount] = VSync(1);
        sCpuMarkR[gCpuMarkCount] = a;
        sCpuMarkG[gCpuMarkCount] = b;
        sCpuMarkB[gCpuMarkCount] = c;
        gCpuMarkCount++;
    }
}
```

First try (the second build only added the include). Notes:

- `gCpuMarkCount` is reached `lui` (`lw $v0` / `lui $at` + `sw`): a plain
  `s32` (`slti`). The four tables are reached through `lui`/`addiu` into a
  different register: `[]`. Retail reloads the count after each BYTE store
  but not after the word store to `gCpuMarkTimes`; the plain spelling gives
  exactly that (a byte store may alias anything in 2.8.1's alias rules).
- The arguments are `s32`: stored only by `sb`, no extension.
- The unit now includes `libetc.h` (after `libgpu.h`, as main.c does) for
  `VSync`'s prototype; without it declcheck reports IMPLICIT and `-Wall`
  a NEW implicit-declaration warning.
- Other readers: DrawCpuMeter (this unit, asm) uses all five; main.s
  clears `gCpuMarkCount` through `lui $at`. No other C declares them.
