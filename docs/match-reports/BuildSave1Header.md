# BuildSave1Header — MATCHED 50/50 (round 5, echo)

> Renamed from `func_800385E0` on 2026-10-10 (tools/rename.py). Address 0x800385e0.

Unit `src/code_27bc8.c`. Fills the start of a memory card file header at
`gCardHeader`: magic `"SC"`, icon flag 0x13, 15 blocks, and the 27-byte
Shift-JIS title from rodata `sToolCardTitle`; then calls `CopySave1Icon`.
`BuildSave2Header` is the same with 12 blocks, title `sHitCardTitle` and
`CopySave2Icon`.

```c
typedef struct {
    u8 b[27];
} CardTitle;

typedef struct {
    u8 magic[2];
    u8 iconFlag;
    u8 blocks;
    CardTitle title;
} CardHeader;

extern CardHeader gCardHeader;
extern u8 sToolCardTitle[];

void CopySave1Icon(void);

void BuildSave1Header(void) {
    gCardHeader.magic[0] = 'S';
    gCardHeader.magic[1] = 'C';
    gCardHeader.iconFlag = 0x13;
    gCardHeader.blocks = 15;
    gCardHeader.title = *(CardTitle *)sToolCardTitle;
    CopySave1Icon();
}
```

## Notes

- First build matched.
- **The title is a block move of a 27-byte, byte-aligned struct**: six
  `lwl`/`lwr` + `swl`/`swr` pairs and three byte copies. A one-member
  `u8 b[27]` struct assigned from the rodata (alpha's round-5 lever for the
  large copies, here at an odd size and unaligned).
- **`gCardHeader` is a struct of more than 8 bytes**: the first `sb` goes
  through `%lo(gCardHeader)` and the rest through the base built with
  `addiu`, which is cc1's split of a large object. Only this unit reaches
  it (also `RunCardOp1`, `RunCardOp2`), so the typedef is unit-local.
  The header's real size (a PS1 save header is 0x200 bytes) is not needed
  yet.
