# UpdateActResults — MATCHED 819/819 (round 11, alpha)

> Renamed from `func_8003CC94` on 2026-10-10 (tools/rename.py). Address 0x8003cc94.

Unit `src/code_29f54.c`. The time-trial results screen, drawn each frame
while `gResultsActive` is set; the twin of UpdateWorldResults with a state machine
on `gResultsStep` (a jump table over 0..50): the row timeline (state 0),
counting the collected cans `gTallyCans` up to the lap total with a
sound every 10 and a bonus every 25 (10), the "all cans" flourish (11),
the new-record check and banner against the per-course best table
`gBestCans[course][slot]` (10, 11, 15), the second timeline (20), the time
record against `gBestTimes` (30, 35), and the final tally (50: every time
under its rodata par `gTargetTimes` sets `D_800957D5`, all cans in all
courses sets bit 2 of `gUnlocks`, and the run's cans and time go to
`gActCans` / `gActTimes` by `rank % 3`). It then slides the rows in and
draws the score, the banners and the four results lines.

```c
/* MATCHING: 8-byte arrays (rank % 3 indexes them); as scalars indexed
 * through & the stores in UpdateActResults schedule the address first. */
extern s32 gActCans[2];
extern s32 gActTimes[2];
extern s32 gBestCans[][2];
extern s32 gBestTimes[][2];
extern s32 gTargetTimes[][2];
extern u8 gUnlocks;

void DrawResultNumber(SVECTOR *pos, s32 n);

void UpdateActResults(void) {
    s32 i;
    s32 k;
    s32 q;
    s32 n;
    s32 place;
    s32 rank;
    SVECTOR pos;
    s16 quad[8];
    SColor color;
    s16 tile[4];

    if (gResultsActive == 0) {
        return;
    }
    place = gStage;
    if (place >= 0) {
        if (place < 6) {
            rank = place;
        } else if (place < 15) {
            if (place >= 9) {
                rank = place - 3;
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
                case 24:
                    gResultsSlide[1] = -24;
                    break;
                case 40:
                    gResultsSliding[1] = 1;
                    break;
                case 64:
                    gResultsSliding[2] = 1;
                    break;
                case 68:
                    gResultsSliding[3] = 1;
                    break;
                case 80:
                    gResultsStep = 10;
                    break;
            }
            gResultsTimer++;
            break;
        case 10:
            if (gTallyCans < gCans) {
                if (gTallyCans % 10 == 0) {
                    PlaySe(0x2A);
                }
                gTallyCans++;
                if (gTallyCans != 0 && gTallyCans % 25 == 0) {
                    gRetries++;
                    PlaySe(0x22);
                }
            }
            if (gTallyCans == gCans) {
                if (gTallyCans == 100) {
                    PlaySe(0x2F);
                    tile[1] = 0x1FB;
                    tile[2] = 0x10;
                    tile[3] = 1;
                    gResultsSlide[6] = -9;
                    tile[0] = 0;
                    gResultsTimer = 0;
                    gResultsStep = 11;
                } else if (gBestCans[rank / 3][rank % 3] < gTallyCans) {
                    PlaySe(0x25);
                    gResultsSlide[7] = -9;
                    gResultsTimer = 0;
                    gResultsStep = 15;
                } else {
                    gResultsTimer = 0;
                    gResultsStep = 20;
                }
            }
            break;
        case 11:
            if (gResultsSlide[6] == 0) {
                if (gResultsTimer < 36) {
                    tile[1] = 0x1FB;
                    tile[2] = 0x10;
                    tile[3] = 1;
                    tile[0] = gResultsTimer % 14 * 16;
                }
                if (++gResultsTimer == 60) {
                    gRetries++;
                    PlaySe(0x22);
                    gResultsSlide[6] = 1;
                }
            } else if (++gResultsSlide[6] == 9) {
                if (gTallyCans > gBestCans[rank / 3][rank % 3]) {
                    PlaySe(0x25);
                    gResultsSlide[7] = -9;
                    gResultsTimer = 0;
                    gResultsStep = 15;
                } else {
                    gResultsTimer = 0;
                    gResultsStep = 20;
                }
            }
            break;
        case 15:
            tile[3] = 1;
            tile[1] = 0x1FA;
            tile[2] = 0x10;
            tile[0] = gResultsTimer % 14 * 16;
            if (gResultsSlide[7] == 0) {
                if (++gResultsTimer == 60) {
                    gBestCans[rank / 3][rank % 3] = gTallyCans;
                    gResultsSlide[7] = 1;
                }
            } else if (++gResultsSlide[7] == 9) {
                gResultsTimer = 0;
                gResultsStep = 20;
            }
            break;
        case 20:
            switch (gResultsTimer) {
                case 15:
                    gResultsSliding[4] = 1;
                    break;
                case 19:
                    gResultsSliding[5] = 1;
                    break;
                case 31:
                    gResultsStep = 30;
                    break;
            }
            gResultsTimer++;
            break;
        case 30:
            if (gElapsed < gBestTimes[rank / 3][rank % 3]) {
                PlaySe(0x25);
                gResultsSlide[7] = -9;
                gResultsTimer = 0;
                gResultsStep = 35;
            } else {
                gResultsTimer = 0;
                gResultsStep = 50;
            }
            break;
        case 35:
            tile[3] = 1;
            tile[1] = 0x1FA;
            tile[2] = 0x10;
            tile[0] = gResultsTimer % 14 * 16;
            if (gResultsSlide[7] == 0) {
                if (++gResultsTimer == 60) {
                    gBestTimes[rank / 3][rank % 3] = gElapsed;
                    gResultsSlide[7] = 1;
                }
            } else if (++gResultsSlide[7] == 9) {
                gResultsTimer = 0;
                gResultsStep = 50;
            }
            break;
        case 50:
            if (++gResultsTimer == 60) {
                n = 0;
                for (i = 0; i < 4; i++) {
                    if (gBestTimes[i][0] >= gTargetTimes[i][0]) {
                        n = 1;
                    }
                    if (gBestTimes[i][1] >= gTargetTimes[i][1]) {
                        n = 1;
                    }
                }
                if (n == 0) {
                    D_800957D5 = 1;
                }
                n = 0;
                for (i = 0; i < 4; i++) {
                    n += gBestCans[i][0] + gBestCans[i][1];
                }
                if (n == 800) {
                    gUnlocks |= 4;
                }
                gResultsActive = 0;
                gActCans[rank % 3] = gTallyCans;
                gActTimes[rank % 3] = gElapsed;
            }
            break;
    }
    for (i = 0; i < 6; i++) {
        if (gResultsSliding[i] != 0) {
            if (gResultsSlide[i] != 0) {
                gResultsSlide[i]++;
            } else {
                gResultsSliding[i] = 0;
            }
        }
    }
    k = 1;
    n = gRetries;
    pos.vx = 0x68;
    pos.vy = -0x44;
    for (; k < 10001; k *= 10) {
        q = n / k;
        if (q != 0) {
            DrawSprite(q % 10 + 0x105, &pos, NULL, 0, &gSubOt[gActiveBuff]);
            pos.vx -= 12;
        }
    }
    color.r = -1;
    color.g = color.b = color.cd = 0x80;
    pos.vx = 0x70;
    pos.vy = -0x58;
    DrawSprite(0x135, &pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
    color.r = -1;
    color.g = color.b = 0x80;
    color.cd = 0;
    pos.vy = -16;
    pos.vx = gResultsSlide[6] * 32 - 0x34;
    DrawSprite(0x15B, &pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
    pos.vy = -16;
    pos.vx = gResultsSlide[7] * 32 - 0x40;
    DrawSprite(0x15C, &pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
    color.r = -1;
    color.g = color.b = color.cd = 0x80;
    pos.vx = -0x28;
    pos.vy = gResultsSlide[0] * 4 - 0x48;
    DrawSprite(rank % 3 + 0x14F, &pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
    if (gResultsSlide[1] != -25) {
        quad[1] = -0x20;
        quad[3] = -0x40;
        quad[4] = quad[5] = (-gResultsSlide[1] << 8) + 0x1000;
        quad[0] = 0;
        quad[2] = 0;
        quad[6] = 0;
        quad[7] = 1;
        DrawSpriteTransformed(0x151, quad, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
    }
    pos.vy = 0;
    pos.vx = gResultsSlide[2] * 32 - 0x80;
    DrawSprite(0x157, &pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
    pos.vx = gResultsSlide[2] * 32 + 0x80;
    DrawResultNumber(&pos, gTallyCans);
    pos.vy = 0x18;
    pos.vx = gResultsSlide[3] * 32 - 0x80;
    DrawSprite(0x158, &pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
    pos.vx = gResultsSlide[3] * 32 + 0x80;
    DrawResultNumber(&pos, gBestCans[rank / 3][rank % 3]);
    pos.vy = 0x38;
    pos.vx = gResultsSlide[4] * 32 - 0x80;
    DrawSprite(0x159, &pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
    pos.vx = gResultsSlide[4] * 32 + 0x80;
    DrawResultTime(&pos, gElapsed);
    pos.vy = 0x50;
    pos.vx = gResultsSlide[5] * 32 - 0x80;
    DrawSprite(0x15A, &pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
    pos.vx = gResultsSlide[5] * 32 + 0x80;
    DrawResultTime(&pos, gBestTimes[rank / 3][rank % 3]);
}
```

`SColor` is the unit's signed-red colour (declared above DrawHud);
`DrawSpriteTransformed`'s unit view is declared there too.

## How it closed (about 12 builds; one permuter search, no candidate)

| step | words |
| --- | --- |
| first body, written from UpdateWorldResults and the jump table | 801/819, same length |
| the place in its own local (`place`), not the digit counter `k` | |
| `gTallyCans > table` in case 11 (operand order sets load order) | |
| the tally's flag and sum in one local `n` | 816/819, one `sll` scheduled after the table address |
| index local `i = rank % 3` | 812 |
| pointer local for the two best tables | 800 |
| integer-sum spellings, either operand order | 816 |
| `gResultsActive = 0;` after the stores | 800 |
| `gActCans` / `gActTimes` declared `s32 [2]` instead of `s32` (UpdateWorldResults now reads `[0]`, still 309/309) | 819/819 |

The permuter (Gate 3: scaffold disassembly identical to the in-tree
object; bound 900 s) found nothing better than 816 in about 50,000
iterations; the declaration change was manual, made while it ran.

- **`tile`**: four halfwords at sp+0x38 written in states 10, 11, 15 and
  35 and never read. GCC 2.8.1 keeps stores to a local whose address never
  escapes; like DrawResultTime's unused shadow colour.
- **`rank`**: as in UpdateWorldResults (`bltz` on the `lbu`-loaded place,
  `rank` unset outside both ranges; the warning is baselined).
- **The `% 25` bonus test** is `!= 0 &&` first: retail tests the count for
  zero before the division.
- **Case tails**: every `gResultsTimer = 0; gResultsStep = K;` pair and every
  `gResultsTimer++` cross-jump into shared tails; written per case.

### Proposed learning

- **An indexed store whose index retail computes before the table's
  `lui`/`addiu`, when the same symbol is read elsewhere with a
  single-register load: declare it as an array of at most 8 bytes**, not
  a scalar indexed through `&`. Small arrays keep the plain reads
  single-register. (UpdateActResults, `gActCans[2]`)
- **Stores to a local the function never reads, kept in place: no dead
  store elimination for stack locals in 2.8.1.** Write them; they cost
  nothing else. (UpdateActResults, DrawResultTime)
