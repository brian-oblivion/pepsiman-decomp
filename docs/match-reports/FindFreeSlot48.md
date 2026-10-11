# FindFreeSlot48 — MATCHED 23/23 (round 4, bravo)

> Renamed from `func_8002882C` on 2026-10-10 (tools/rename.py). Address 0x8002882c.

Unit `src/collision.c`. Searches the table of 72-byte records that
`gSlotTable` points to (`gSlotTableSize` bytes long) for the first one whose
halfword at 0x36 is -1, and returns its index, or -1.

```c
typedef struct {
    u8 pad0[0x36];
    s16 unk36; /**< -1 when the record is free */
    u8 pad38[0x48 - 0x38];
} Slot48;

s16 FindFreeSlot48(void) {
    Slot48 *slot = (Slot48 *)gSlotTable;
    u32 i;

    for (i = 0; i < gSlotTableSize / sizeof(Slot48); i++) {
        if (slot->unk36 == -1) {
            return i;
        }
        slot++;
    }
    return -1;
}
```

First try. The `multu` by 0x38E38E39 and `srl 4` is an unsigned divide by
72: `gSlotTableSize` (shared `s32`) divided by `sizeof`, which promotes it to
unsigned. The `sltu` loop test is LEARNINGS' `u32` counter; the `sll`/`sra`
on the found index is the `s16` return. No direct caller in the asm
(probably reached through a pointer). `gSlotTable` stays the shared `s32`
and is cast here.
