# ReadSave2 — MATCHED 97/97 (round 9, delta)

> Renamed from `func_80038C74` on 2026-10-10 (tools/rename.py). Address 0x80038c74.

Unit `src/card.c`. Twin of ReadSave1 for the second save file
`"bu10:BISLPS-67890PEPTOOL"` (`sHitCardFile`), reading 0x18000 bytes. The asm
of the two differs only in the name and the size; ReadSave1's body with
those two changed matched on the first build. See ReadSave1's report for
the shared-fail-tail note.
