# func_8003AFAC — MATCHED 501/501 (round 11, alpha)

Unit `src/code_29f54.c`. The race HUD, drawn each frame: the lap count
`D_800958E8` left-aligned at the top left (a lone `0` when it is zero),
the speed dial (func_800198BC quads; its needle sprite chosen from the
distance counter `D_80095A84` advanced by `D_8009EF20[0]`, scaled by the
mode `D_800958A8`, blinking in mode 1), a sound cue at frame 330, the
seconds left (`D_80095988 / 30`, red under 11, blinking under 5), the
course progress marker (`D_8009578C` along the path, scaled by the count
stored before `D_800958A0`), the time with func_8003A4B4 and the score
`D_80095770` up to five digits.

```c
extern u32 D_80095A84;
extern Edge *D_800958A0;

/** @brief A CVECTOR whose first byte is signed (-1 means "no tint"). */
typedef struct {
    s8 r;  /**< red, or -1 */
    u8 g;  /**< green */
    u8 b;  /**< blue */
    u8 cd; /**< code byte */
} SColor;

/* MATCHING: code_a0bc takes a Sprite2D *, a type local to that unit; this
 * unit passes the same eight halfwords as an array. */
void func_800198BC(u16 id, s16 *quad, CVECTOR *color, u16 otz, GsOT *ot);
s32 func_8003F834(s32 arg0, s16 arg1, s16 arg2, s16 arg3, s16 arg4);

void func_8003AFAC(void) {
    s32 n;
    s32 k;
    s32 q;
    s32 digits;
    SVECTOR pos;
    s16 quad[8];
    SColor color;

    n = D_800958E8;
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
        func_8001B354(0x105, &pos, NULL, 0, &D_800ACEA8[D_80095750]);
    }
    k = 1;
    pos.vx = (digits - 1) * 12 - 120;
    pos.vy = -96;
    for (; k < 101; k *= 10) {
        q = n / k;
        if (q != 0) {
            func_8001B354(q % 10 + 0x105, &pos, NULL, 0, &D_800ACEA8[D_80095750]);
            pos.vx -= 12;
        }
    }
    quad[0] = quad[2] = -0x90;
    quad[1] = quad[3] = -0x68;
    quad[4] = quad[5] = 0x800;
    quad[6] = 0;
    quad[7] = 0;
    func_800198BC(0xFA, quad, NULL, 0, &D_800ACEA8[D_80095750]);
    quad[0] = quad[2] = -0x40;
    quad[1] = quad[3] = -0x58;
    quad[7] = 1;
    quad[6] = 0;
    D_80095A84 += D_8009EF20[0];
    switch (D_800958A8) {
        case 1:
            quad[4] = quad[5] = 0x800;
            if ((D_8009585C >> 1) & 1) {
                func_800198BC(D_80095A84 / 100 % 10 + 0xFB, quad, NULL, 0, &D_800ACEA8[D_80095750]);
            }
            break;
        case 2:
            quad[4] = quad[5] = 0xB50;
            func_800198BC(D_80095A84 / 100 % 10 + 0xFB, quad, NULL, 0, &D_800ACEA8[D_80095750]);
            break;
        case 0:
            break;
        default:
            quad[4] = quad[5] = 0x1000;
            func_800198BC(D_80095A84 / 100 % 10 + 0xFB, quad, NULL, 0, &D_800ACEA8[D_80095750]);
            break;
    }
    if (D_80095988 == 0x14A) {
        func_80042538(0x28);
        func_8003F834(10, 0, 0, 0, 0);
    }
    n = D_80095988;
    n /= 30;
    color.r = -1;
    if (n < 11) {
        color.g = 0x80;
        color.b = color.cd = 0;
    } else {
        color.g = color.b = color.cd = 0x80;
    }
    if (n >= 5 || ((D_8009585C >> 1) & 1)) {
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
            func_8001B354(0x123, &pos, (CVECTOR *)&color, 0, &D_800ACEA8[D_80095750]);
        }
        k = 1;
        pos.vx = 4;
        pos.vy = -0x68;
        for (; k < 101; k *= 10) {
            q = n / k;
            if (q != 0) {
                func_8001B354(q % 10 + 0x123, &pos, (CVECTOR *)&color, 0, &D_800ACEA8[D_80095750]);
                pos.vx -= 18;
            }
        }
    }
    color.r = 1;
    color.g = color.b = color.cd = 0x80;
    pos.vx = (D_8009578C * 11 << 15) / ((s32 *)D_800958A0)[-1] / 4096 + 0x34;
    pos.vy = -0x69;
    func_8001B354(((D_8009585C >> 2) & 3) + 0x12F, &pos, (CVECTOR *)&color, 0, &D_800ACEA8[D_80095750]);
    k = 1;
    pos.vx = 0x34;
    pos.vy = -0x64;
    func_8001B354(0x12E, &pos, NULL, 0, &D_800ACEA8[D_80095750]);
    pos.vx = 0x34;
    pos.vy = -0x54;
    func_8001B354(0x122, &pos, NULL, 0, &D_800ACEA8[D_80095750]);
    pos.vx = 0x90;
    pos.vy = -0x54;
    func_8003A4B4(&pos, D_80095980);
    n = D_80095770;
    pos.vx = 0x68;
    pos.vy = 0x4C;
    for (; k < 10001; k *= 10) {
        q = n / k;
        if (q != 0) {
            func_8001B354(q % 10 + 0x105, &pos, NULL, 0, &D_800ACEA8[D_80095750]);
            pos.vx -= 12;
        }
    }
    color.r = -1;
    color.g = color.b = color.cd = 0x80;
    pos.vx = 0x70;
    pos.vy = 0x38;
    func_8001B354(0x135, &pos, (CVECTOR *)&color, 0, &D_800ACEA8[D_80095750]);
}
```

## How it closed (5 builds)

| step | words |
| --- | --- |
| first body | 486/501, three local spots |
| `n = D_80095988; n /= 30;` (the quotient straight into `n`'s register) | |
| `k = 1` inside the blinking `if`, as the `for`'s init | |
| the last loop's `k = 1` moved: after `pos.vy`, then before the colour (length changed), then after the call | 497 -> 501/501 |

- **`sllv` by `$s1` for `(digits - 1) * 12`**: cse took the shift count 1
  from `k`, which holds 1: `k = 1;` is set before `pos.vx` is computed
  (as in func_8003A4B4).
- **`srlv`/`and` by `$s1` where retail has `srl 1` / `andi 1`**: the same
  cse in reverse; `k = 1` must not be live yet when `(D_8009585C >> 1) &
  1` is computed, so it is the `for`'s own init inside the `if`.
- **A `li $s1, 1` stolen into the wrong delay slot**: the last loop's
  `k = 1` after the progress-marker call; anywhere above it lands in the
  block of the `/ 4096`.
- **The second digit-count loop has no use**: retail keeps its divisions
  and drops the counter; written without one.
- **Unsigned `D_80095A84`**: `multu` / `srl` for `/ 100 % 10`, a `u32`.
- `switch (D_800958A8)` with an explicit `case 0: break;` gives retail's
  `beqz` straight to the join; the three needle calls share one tail.
- `D_800958A0` is read here as `Edge *` (unit-local type), `((s32
  *)D_800958A0)[-1]` being the record count stored before the table.
  declcheck flags the local extern (code_1a098 uses the symbol too, as
  `PathPt *`), left for the head.

### Proposed learning

- **A redundant loop of divisions with nothing kept: a digit count whose
  result the source never used.** Write the loop without the counter; the
  `div`s stay because they may trap. (func_8003AFAC)
- **A loop counter's constant init in the wrong delay slot or block: move
  the `k = 1;` statement past the next call**; dbr steals it from the
  target block otherwise. (func_8003AFAC)
