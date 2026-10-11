# RewriteSave2 — MATCHED 94/94 (round 9, delta)

> Renamed from `func_80038DF8` on 2026-10-10 (tools/rename.py). Address 0x80038df8.

Unit `src/card.c`. Twin of RewriteSave1 for the second save file
`"bu10:BISLPS-67890PEPTOOL"` (`sHitCardFile`), writing 0x18000 bytes. The asm of
the two differs only in the name and the size; RewriteSave1's body with
those two changed matched on the first build. See RewriteSave1's report for
the shared-counter and cross-jump notes.
