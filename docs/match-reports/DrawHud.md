# DrawHud — MATCHED 501/501 (round 11, alpha)

> Renamed from `func_8003AFAC` on 2026-10-10 (tools/rename.py). Address 0x8003afac.

Unit `src/stage.c`. The race HUD, drawn each frame: the lap count
`gCans` left-aligned at the top left (a lone `0` when it is zero),
the speed dial (DrawSpriteTransformed quads; its needle sprite chosen from the
distance counter `D_80095A84` advanced by `D_8009EF20[0]`, scaled by the
mode `gBonusLevel`, blinking in mode 1), a sound cue at frame 330, the
seconds left (`gTimeLeft / 30`, red under 11, blinking under 5), the
course progress marker (`gSection` along the path, scaled by the count
stored before `gSectionEdges`), the time with DrawHudTime and the score
`gRetries` up to five digits.

```c
extern u32 D_80095A84;
extern Edge *gSectionEdges;

/** @brief A CVECTOR whose first byte is signed (-1 means "no tint"). */
typedef struct {
    s8 r;  /**< red, or -1 */
    u8 g;  /**< green */
    u8 b;  /**< blue */
    u8 cd; /**< code byte */
} SColor;

/* MATCHING: draw takes a Sprite2D *, a type local to that unit; this
 * unit passes the same eight halfwords as an array. */
void DrawSpriteTransformed(u16 id, s16 *quad, CVECTOR *color, u16 otz, GsOT *ot);
s32 SpawnEffect(s32 arg0, s16 arg1, s16 arg2, s16 arg3, s16 arg4);

void DrawHud(void) {
    s32 n;
    s32 k;
    s32 q;
    s32 digits;
    SVECTOR pos;
    s16 quad[8];
    SColor color;

    n = gCans;
    digits = 0;
    for (k = 1; k < 101; k *= 10) {
        if (n / k == 0) {
            break;
        }
        digits++;
    }
    if (n == 0) {
        pos.vx = -120;
        pos.vy = -96;
        DrawSprite(0x105, &pos, NULL, 0, &gSubOt[gActiveBuff]);
    }
    k = 1;
    pos.vx = (digits - 1) * 12 - 120;
    pos.vy = -96;
    for (; k < 101; k *= 10) {
        q = n / k;
        if (q != 0) {
            DrawSprite(q % 10 + 0x105, &pos, NULL, 0, &gSubOt[gActiveBuff]);
            pos.vx -= 12;
        }
    }
    quad[0] = quad[2] = -0x90;
    quad[1] = quad[3] = -0x68;
    quad[4] = quad[5] = 0x800;
    quad[6] = 0;
    quad[7] = 0;
    DrawSpriteTransformed(0xFA, quad, NULL, 0, &gSubOt[gActiveBuff]);
    quad[0] = quad[2] = -0x40;
    quad[1] = quad[3] = -0x58;
    quad[7] = 1;
    quad[6] = 0;
    D_80095A84 += D_8009EF20[0];
    switch (gBonusLevel) {
        case 1:
            quad[4] = quad[5] = 0x800;
            if ((gFrameCount >> 1) & 1) {
                DrawSpriteTransformed(D_80095A84 / 100 % 10 + 0xFB, quad, NULL, 0, &gSubOt[gActiveBuff]);
            }
            break;
        case 2:
            quad[4] = quad[5] = 0xB50;
            DrawSpriteTransformed(D_80095A84 / 100 % 10 + 0xFB, quad, NULL, 0, &gSubOt[gActiveBuff]);
            break;
        case 0:
            break;
        default:
            quad[4] = quad[5] = 0x1000;
            DrawSpriteTransformed(D_80095A84 / 100 % 10 + 0xFB, quad, NULL, 0, &gSubOt[gActiveBuff]);
            break;
    }
    if (gTimeLeft == 0x14A) {
        PlaySe(0x28);
        SpawnEffect(10, 0, 0, 0, 0);
    }
    n = gTimeLeft;
    n /= 30;
    color.r = -1;
    if (n < 11) {
        color.g = 0x80;
        color.b = color.cd = 0;
    } else {
        color.g = color.b = color.cd = 0x80;
    }
    if (n >= 5 || ((gFrameCount >> 1) & 1)) {
        /* MATCHING: a digit count whose result is never used; the
         * divisions stay. */
        for (k = 1; k < 101; k *= 10) {
            if (n / k == 0) {
                break;
            }
        }
        if (n == 0) {
            pos.vx = 4;
            pos.vy = -0x68;
            DrawSprite(0x123, &pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
        }
        k = 1;
        pos.vx = 4;
        pos.vy = -0x68;
        for (; k < 101; k *= 10) {
            q = n / k;
            if (q != 0) {
                DrawSprite(q % 10 + 0x123, &pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
                pos.vx -= 18;
            }
        }
    }
    color.r = 1;
    color.g = color.b = color.cd = 0x80;
    pos.vx = (gSection * 11 << 15) / ((s32 *)gSectionEdges)[-1] / 4096 + 0x34;
    pos.vy = -0x69;
    DrawSprite(((gFrameCount >> 2) & 3) + 0x12F, &pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
    k = 1;
    pos.vx = 0x34;
    pos.vy = -0x64;
    DrawSprite(0x12E, &pos, NULL, 0, &gSubOt[gActiveBuff]);
    pos.vx = 0x34;
    pos.vy = -0x54;
    DrawSprite(0x122, &pos, NULL, 0, &gSubOt[gActiveBuff]);
    pos.vx = 0x90;
    pos.vy = -0x54;
    DrawHudTime(&pos, gElapsed);
    n = gRetries;
    pos.vx = 0x68;
    pos.vy = 0x4C;
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
    pos.vy = 0x38;
    DrawSprite(0x135, &pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
}
```

## How it closed (5 builds)

| step | words |
| --- | --- |
| first body | 486/501, three local spots |
| `n = gTimeLeft; n /= 30;` (the quotient straight into `n`'s register) | |
| `k = 1` inside the blinking `if`, as the `for`'s init | |
| the last loop's `k = 1` moved: after `pos.vy`, then before the colour (length changed), then after the call | 497 -> 501/501 |

- **`sllv` by `$s1` for `(digits - 1) * 12`**: cse took the shift count 1
  from `k`, which holds 1: `k = 1;` is set before `pos.vx` is computed
  (as in DrawHudTime).
- **`srlv`/`and` by `$s1` where retail has `srl 1` / `andi 1`**: the same
  cse in reverse; `k = 1` must not be live yet when `(gFrameCount >> 1) &
  1` is computed, so it is the `for`'s own init inside the `if`.
- **A `li $s1, 1` stolen into the wrong delay slot**: the last loop's
  `k = 1` after the progress-marker call; anywhere above it lands in the
  block of the `/ 4096`.
- **The second digit-count loop has no use**: retail keeps its divisions
  and drops the counter; written without one.
- **Unsigned `D_80095A84`**: `multu` / `srl` for `/ 100 % 10`, a `u32`.
- `switch (gBonusLevel)` with an explicit `case 0: break;` gives retail's
  `beqz` straight to the join; the three needle calls share one tail.
- `gSectionEdges` is read here as `Edge *` (unit-local type), `((s32
  *)gSectionEdges)[-1]` being the record count stored before the table.
  declcheck flags the local extern (object uses the symbol too, as
  `PathPt *`), left for the head.

### Proposed learning

- **A redundant loop of divisions with nothing kept: a digit count whose
  result the source never used.** Write the loop without the counter; the
  `div`s stay because they may trap. (DrawHud)
- **A loop counter's constant init in the wrong delay slot or block: move
  the `k = 1;` statement past the next call**; dbr steals it from the
  target block otherwise. (DrawHud)
