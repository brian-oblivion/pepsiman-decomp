# func_8002BEC0 — MATCHED 97/97 (round 8, alpha)

Unit `src/code_1a098.c`. One drift step of a `Drifter` (a unit-local
view: position at 4..0xC, step count at 0x10, base y at 0x14, bob phase in
degrees at 0x18), for the first 15 steps: moves it 20 units along the yaw
`D_800A7680[0].vy` (through func_8002C188 into `D_800D39C8`), advances
the phase by 10 degrees and sets y to the base minus `|200 sin(phase)|`.
Returns 1 on two steps of three, 0 on every third and once done.

```c
s32 func_8002BEC0(Drifter *d) {
    s32 b;
    u8 ret;
    s32 y;

    if (d->count < 15) {
        d->count++;
        ret = d->count % 3 != 0;
        func_8002C188(20, D_800A7680[0].vy * 360 / 4096, (Vec3 *)D_800D39C8);
        d->x -= ((Vec3 *)D_800D39C8)->x >> 12;
        d->z -= ((Vec3 *)D_800D39C8)->z >> 12;
        d->phase = (d->phase + 10) % 360;
        b = rsin(d->phase * 4096 / 360) * 200 >> 12;
        y = d->baseY;
        if (b < 0) {
            b = -b;
        }
        d->y = y - b;
    } else {
        ret = 0;
    }
    return ret;
}
```

## Path to the match (12 builds)

1. Natural order with an `s32 ret` and a ternary abs: 34/97. Three
   differences: `d` and `ret` in swapped saved registers, the phase stored
   right away, the abs flipped into `bltz` + `addu`.
2. The phase computed into a local and stored after x and z: the store
   moved but the value sat in `$a3`, not `$a0`.
3. A statement abs: `bgez`/`negu`/`subu` like retail, but `baseY`
   loaded after the branch.
4. `ret` declared last, `n = d->count + 1` as a local, `ret = (... % 3)
   ? 1 : 0`: no change, or worse.
5. **`u8 ret` (also `s16`): 67/97, the registers right.** Retail computes
   the test into `$a0` and copies it into `$s3`; a narrow result
   variable gives that copy, and it moves `ret`'s pseudo after `d`'s.
6. **`d->phase = (d->phase + 10) % 360;` after the x/z updates, and
   rsin of `d->phase`**: 79/97; the scheduler hoists the load and the CSE
   keeps the value in `$a0`.
7. **`y = d->baseY;` before the abs statement**: 97/97.

## Callee declaration (func_8002C188, same unit, already matched)

Retail passes the degrees unextended (`sra 12` straight into `$a1`), so
this call had compiled through an implicit declaration. func_8002C188's
parameter is now `s32 deg` with `(s16)deg` in the body (same bytes), and
the unit has a real prototype for it.

### Proposed learning

- A 0/1 test computed into a temporary and copied into a saved register
  (`sltu a0,zero,a0; move s3,a0`): the result variable is narrower than
  `s32` (`u8 ret`). (func_8002BEC0)
- A field loaded before an abs's `bgez` and subtracted after it: load it
  into a local before the abs statement. (func_8002BEC0)
