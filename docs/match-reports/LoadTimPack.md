# LoadTimPack — MATCHED (31/31), round 3, runner charlie

> Renamed from `func_8001797C` on 2026-10-10 (tools/rename.py). Address 0x8001797c.

Unit `src/code_7d74.c`. Walks a table of 16-byte entries at the start of a
data pack (entry 0's `+0xE` halfword is the count) and hands each entry's
data, `pack + offset`, to `LoadTim`, waiting for the GPU after each.

```c
typedef struct {
    s32 offset;
    u8 unk4[0xA];
    u16 count;
} PackEntry;

void LoadTim(void *data);

void LoadTimPack(PackEntry *pack) {
    PackEntry *e;
    u16 i;
    u16 n;

    e = pack;
    n = pack->count;
    for (i = 0; i < n; i++) {
        LoadTim((u8 *)pack + e->offset);
        e++;
        DrawSync(0);
    }
}
```

## Attempts (2)

1. `for (i = 0; i < pack->count; i++)`: 4 registers saved instead of 5,
   and the count reloaded with `lhu` at the bottom of every pass.
2. The count read once into a `u16` local before the loop: 31/31. Retail
   holds it in `$s2` and compares it with `sltu` against `i & 0xFFFF`.

### Proposed learning

- **A loop bound held in a saved register across calls, never reloaded:
  the source copied it into a local before the loop.** Testing the member
  in the `for` condition reloads it after every call. (LoadTimPack)
