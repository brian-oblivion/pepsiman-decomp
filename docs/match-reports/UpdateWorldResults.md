# UpdateWorldResults — MATCHED 309/309 (round 11, alpha)

> Renamed from `func_8003D960` on 2026-10-10 (tools/rename.py). Address 0x8003d960.

Unit `src/code_29f54.c`. The results screen, drawn each frame while
`gResultsActive` is set. It picks a rank sprite from the finishing place
`gStage` (places 0-5 as is, 9-14 minus 3), runs a timeline on the frame
counter `gResultsTimer` that switches on one row flag of `gResultsSliding` at frames
0, 60, 90, 94, 120, 150 and 154 and sets the state `gResultsStep` to 10 at
300 (state 10 clears `gResultsActive`), slides every switched-on row's offset
`gResultsSlide[i]` until it reaches 0, then draws the rows: the rank, three
headings with DrawResultNumber / DrawResultTime values beside them
(`gActCans`, `gActTimes`, `gWorldCans`, `gWorldTime`).

```c
/* gActCans[2] and gActTimes[2] are declared above UpdateActResults. */
extern s32 gWorldCans;
extern s32 gWorldTime;

void UpdateWorldResults(void) {
    s32 i;
    s32 k;
    s32 rank;
    SVECTOR pos;
    SColor color;

    if (gResultsActive == 0) {
        return;
    }
    k = gStage;
    if (k >= 0) {
        if (k < 6) {
            rank = k;
        } else if (k < 15) {
            if (k >= 9) {
                rank = k - 3;
            }
        }
    }
    color.r = -1;
    color.g = color.b = color.cd = 0x80;
    switch (gResultsStep) {
        case 0:
            switch (gResultsTimer) {
                case 0:
                    gResultsSliding[0] = 1;
                    break;
                case 60:
                    gResultsSliding[1] = 1;
                    break;
                case 90:
                    gResultsSliding[2] = 1;
                    break;
                case 94:
                    gResultsSliding[3] = 1;
                    break;
                case 120:
                    gResultsSliding[4] = 1;
                    break;
                case 150:
                    gResultsSliding[5] = 1;
                    break;
                case 154:
                    gResultsSliding[6] = 1;
                    break;
                case 300:
                    gResultsStep = 10;
                    break;
            }
            gResultsTimer++;
            break;
        case 10:
            gResultsActive = 0;
            break;
    }
    for (i = 0; i < 7; i++) {
        if (gResultsSliding[i] != 0) {
            if (gResultsSlide[i] != 0) {
                gResultsSlide[i]++;
            } else {
                gResultsSliding[i] = 0;
            }
        }
    }
    pos.vx = -80;
    pos.vy = gResultsSlide[0] * 4 - 88;
    DrawSprite(0x152, &pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
    pos.vx = 48;
    DrawSprite(rank / 3 + 0x153, &pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
    pos.vy = -48;
    pos.vx = gResultsSlide[1] * 32 - 40;
    DrawSprite(0x14F, &pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
    pos.vy = -24;
    pos.vx = gResultsSlide[2] * 32 - 128;
    DrawSprite(0x157, &pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
    pos.vx = gResultsSlide[2] * 32 + 128;
    DrawResultNumber(&pos, gActCans[0]);
    pos.vy = 0;
    pos.vx = gResultsSlide[3] * 32 - 128;
    DrawSprite(0x159, &pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
    pos.vx = gResultsSlide[3] * 32 + 128;
    DrawResultTime(&pos, gActTimes[0]);
    pos.vy = 24;
    pos.vx = gResultsSlide[4] * 32 - 40;
    DrawSprite(0x150, &pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
    pos.vy = 48;
    pos.vx = gResultsSlide[5] * 32 - 128;
    DrawSprite(0x157, &pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
    pos.vx = gResultsSlide[5] * 32 + 128;
    DrawResultNumber(&pos, gWorldCans);
    pos.vy = 72;
    pos.vx = gResultsSlide[6] * 32 - 128;
    DrawSprite(0x159, &pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
    pos.vx = gResultsSlide[6] * 32 + 128;
    DrawResultTime(&pos, gWorldTime);
}
```

## How it closed (3 builds)

| step | words |
| --- | --- |
| first body, the state test as `if (== 0) {switch} else if (== 10)` | every block right except the state test's layout (measured beside an unfinished DrawResultNumber, so no clean word count) |
| the state test as a two-case `switch (gResultsStep)` | 309/309 |

- **`bltz` on an `lbu`-loaded place**: the place goes into an `s32` local
  and is tested `k >= 0` first. Then `< 6`, then `< 15`, then `>= 9`, as
  nested `if`s; `rank` is left unset outside both ranges, which is the
  baselined "might be used uninitialized" warning.
- **The state test**: retail branches `beqz` to the timeline block and
  `beq 10` to the store, then jumps over both. That is a `switch` with
  `case 0:` and `case 10:` (bodies in source order), not an if/else-if.
- **The timeline switch** is a compare tree split at 94 with the case
  bodies in ascending source order (LEARNINGS: case bodies are laid out in
  source order).
- `DrawResultNumber` is still `INCLUDE_ASM`, so its prototype is in the `.c`.

### Proposed learning

- **`beqz` to one block and `beq K` to another, then a `j` over both, on
  one state variable: `switch (s) { case 0: ...; break; case K: ...;
  break; }`.** `if (s == 0) ... else if (s == K)` lays the `== 0` body out
  first as the fall-through. (UpdateWorldResults)
