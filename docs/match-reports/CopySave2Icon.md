# CopySave2Icon — MATCHED 269/269 (round 9, delta)

> Renamed from `func_800390B8` on 2026-10-10 (tools/rename.py). Address 0x800390b8.

Unit `src/card.c`. Twin of CopySave1Icon for the second save file: the
three icon TIMs come from `sHitCardIcon0`, `sHitCardIcon1` and `sHitCardIcon2`; the
destinations (`sHeaderIcon0`, `sHeaderIcon1`, `sHeaderIcon2`) are the same card
header. The asm of the two differs only in the three rodata symbols;
CopySave1Icon's body with them substituted matched on the first build. See
CopySave1Icon's report for the array-destination lever. Called by
BuildSave2Header.
