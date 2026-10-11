# StoreHitSlot — MATCHED 114/114 (round 9, echo)

> Renamed from `func_80036184` on 2026-10-11 (tools/rename.py). Address 0x80036184.

Unit `src/code_24748.c`. The inverse of LoadHitSlot: stores the two
working buffers (0x4744 bytes at 0x8016D000, 0x2034 bytes at 0x8017D000)
back into hit-data slot `n` of the tool buffer at 0x8018D000. The
`HitBlockA`/`HitBlockB`/`HitSlot`/`ToolBuf` typedefs moved up the unit,
above this function, unchanged.

```c
void StoreHitSlot(s16 n) {
    ToolBuf *tool = (ToolBuf *)0x8018D000;

    tool->slots[n].a = *(HitBlockA *)0x8016D000;
    (&tool->slots[n])->b = *(HitBlockB *)0x8017D000;
}
```

## Levers

| C | words |
| --- | --- |
| `tool->slots[n].b` for the second copy | 113/114: second sum `addu v0, a3, v0` (base first) |
| **`(&tool->slots[n])->b`** (LoadHitSlot's lever, now on the destination side) | **114/114** |

### Proposed learning

- LoadHitSlot's `(&p->arr[i])->field` lever holds when the slot is the
  copy's DESTINATION too. (StoreHitSlot)
