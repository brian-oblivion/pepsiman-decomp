# RunCardOp2 — MATCHED 176/176 (round 9, delta)

> Renamed from `func_800389B4` on 2026-10-10 (tools/rename.py). Address 0x800389b4.

Unit `src/code_27bc8.c`. Twin of RunCardOp1 for the second save file:
buffers at `0x80195000`/`0x8018D000`, checksum at buffer + 0x17FFE, and the
second file's helpers (FindSave2, CheckSave2Space, ReadSave2,
RewriteSave2, EraseSave2, BuildSave2Header, CreateSave2). The asm of the
two differs only in those constants and call targets; RunCardOp1's body
with them substituted matched on the first build. See RunCardOp1's report
for the levers.

FindSave2 and CheckSave2Space are tested with no `sll`/`sra`, like their
first-file twins, and become `s32` too (they return only 0/-1, still match,
and have no caller outside the unit). Prototypes for the later functions
are at the top of the `.c`.
