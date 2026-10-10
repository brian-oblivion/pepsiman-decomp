# func_80028DBC — MATCHED 84/84 (round 11, charlie)

Unit `src/code_1902c.c`. Returns which of six consecutive points of the
12-byte table `D_8009F0D0`, starting at `start`, is nearest `pos`: it copies
each point and `pos` into local `VECTOR`s, stores func_800297A4's squared
distance in `dist[6]`, then picks the smallest (first one wins ties).

No caller is C yet; the signature is read from retail: `start` is `u16`
(the loop bound is `andi 0xFFFF` + 6, computed once) copied into an `s16`
counter (`sll`/`sra 16` at each test), and the index and result are `u8`
(`andi 0xFF`).

Levers:

- First build was 84/84 in length with a register swap: `dist[k++] = ...`
  increments `k` before `i`. Retail computes `i + 1` first; `i++, k++` in the
  `for` step with `dist[k] = ...` in the body matches. (LEARNINGS, "the
  order of stores in a strength-reduced loop"; this is the same for two
  plain counters.)

```c
typedef struct {
    s32 x;
    s32 y;
    s32 z;
} Point12;

extern Point12 D_8009F0D0[];
s32 func_800297A4(VECTOR *a, VECTOR *b);

u8 func_80028DBC(u16 start, VECTOR *pos) {
    VECTOR a;
    VECTOR b;
    s32 dist[6];
    s16 i;
    u8 k;
    s32 min;

    k = 0;
    for (i = start; i < start + 6; i++, k++) {
        a.vx = D_8009F0D0[i].x;
        a.vy = D_8009F0D0[i].y;
        a.vz = D_8009F0D0[i].z;
        b.vx = pos->vx;
        b.vy = pos->vy;
        b.vz = pos->vz;
        dist[k] = func_800297A4(&a, &b);
    }
    min = dist[0];
    k = 0;
    for (i = 1; i < 6; i++) {
        if (dist[i] < min) {
            min = dist[i];
            k = i;
        }
    }
    return k;
}
```

### Proposed learning

- **Two counters bumped in one loop, the later-declared one incremented
  first in retail: `for (...; i++, k++)` with `a[k] = ...` in the body**,
  not `a[k++] = ...`. (func_80028DBC)
