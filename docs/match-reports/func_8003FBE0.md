# func_8003FBE0 — MATCHED (75/75), round 6, runner echo (stalled in round 5, bravo: 4 words long)

## Round 6: matched (echo)

Same lever as func_8003FA88 and func_8003F960: the matrix set-up is the
inlined `setLs` helper (func_8003F8D4's body), which rebuilds `&coord`
and `&ls` at every call. First build:

```c
s32 func_8003FBE0(Slot *p, GsOT *ot, s16 x, s16 y, s16 z) {
    SVECTOR size;
    CVECTOR color;

    if (p->unk2 == 0) {
        setLs(x + p->unkC, y + p->unkE, z + p->unk10);
        size.vy = size.vx = 0x32;
        color.r = 1;
        color.g = color.b = color.cd = 0x80 - (p->unk0 << 4);
        func_8001A3D4(0x11A, &size, &color, 2, ot);
        return ++p->unk0 == 8;
    }
    p->unk2--;
    return 0;
}
```

## Round 5 (bravo)

Unit `src/code_29f54.c`. The fading twin of func_8003F960: same countdown
and matrix set-up, then sprite 0x11A at size 0x32 in colour
(1, c, c, c) with `c = 0x80 - frame * 16`, frame advanced, 1 returned at
frame 8. Parked under `#ifdef NON_MATCHING` in the unit.

```c
s32 func_8003FBE0(Slot *p, GsOT *ot, s16 x, s16 y, s16 z) {
    SVECTOR size;
    CVECTOR color;
    GsCOORDINATE2 coord;
    MATRIX ls;
    s32 ret;

    if (p->unk2 != 0) {
        p->unk2--;
        ret = 0;
    } else {
        x += p->unkC;
        y += p->unkE;
        z += p->unk10;
        GsInitCoordinate2(WORLD, &coord);
        coord.coord.t[0] = x;
        coord.coord.t[1] = y;
        coord.coord.t[2] = z;
        coord.flg = 0;
        GsGetLs(&coord, &ls);
        GsSetLsMatrix(&ls);
        size.vy = size.vx = 0x32;
        color.r = 1;
        color.g = color.b = color.cd = 0x80 - (p->unk0 << 4);
        func_8001A3D4(0x11A, &size, &color, 2, ot);
        ret = ++p->unk0 == 8;
    }
    return ret;
}
```

One build, from func_8003F960's body. Everything matches (including
`lbu` of the frame for the colour and `li v0, -0x80` / `subu`) except
the residue described in docs/match-reports/func_8003F960.md: retail
rematerialises `&coord` (sp+0x28) and `&ls` (sp+0x78) at each call, the
build keeps each in `s3`, which costs `s5` (save/restore), two `move`s
and an 0xB8 frame. No separate permuter search: it is the same body and
the same residue as func_8003F960, whose search found nothing.
