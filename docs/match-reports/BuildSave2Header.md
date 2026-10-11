# BuildSave2Header — MATCHED 50/50 (round 5, echo)

> Renamed from `func_8003968C` on 2026-10-10 (tools/rename.py). Address 0x8003968c.

Unit `src/card.c`. `BuildSave1Header` for the second save file: header
`"SC"`, icon flag 0x13, 12 blocks, the 27-byte title from rodata
`sHitCardTitle`, then `CopySave2Icon`. Types as in `BuildSave1Header.md`.

```c
void BuildSave2Header(void) {
    gCardHeader.magic[0] = 'S';
    gCardHeader.magic[1] = 'C';
    gCardHeader.iconFlag = 0x13;
    gCardHeader.blocks = 12;
    gCardHeader.title = *(CardTitle *)sHitCardTitle;
    CopySave2Icon();
}
```

First build matched.
