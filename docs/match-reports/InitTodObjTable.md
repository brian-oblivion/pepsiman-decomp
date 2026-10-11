# InitTodObjTable — MATCHED (24/24), round 3, runner charlie

> Renamed from `func_80019684` on 2026-10-10 (tools/rename.py). Address 0x80019684.

Unit `src/model.c`. Initialises a `SlotList` over `n` 16-byte slots:
each slot gets `0x80000000`, a pointer to its 0x50-byte object, 0 and -1,
and each object's `+0x44` word gets a pointer into a 0x28-byte-stride
array.

```c
void InitTodObjTable(SlotList *list, Slot *slots, u8 *objs, s32 data, s32 n) {
    s32 i;

    list->count = 0;
    list->slots = slots;
    list->capacity = n;
    for (i = 0; i < n; i++) {
        slots->unk0 = 0x80000000;
        slots->unkC = -1;
        slots->unk4 = (s32)objs;
        *(s32 *)(objs + 0x44) = data;
        slots->unk8 = 0;
        slots++;
        objs += 0x50;
        data += 0x28;
    }
}
```

`Slot` and `SlotList` are in `FindTodObj.md`.

## Attempts (1)

1. As above: 24/24 first build. The stores came out in source order, and
   2.8.1 split the walking pointer into two (`$t0` for `+0`, `$a1` biased
   by 8 for `+4`/`+8`/`+0xC`) on its own. The object type is unknown, so
   `objs` stays a byte pointer and `data` an `s32`.
