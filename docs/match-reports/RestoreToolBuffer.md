# RestoreToolBuffer — MATCHED 27/27 (round 5, alpha)

> Renamed from `func_8003399C` on 2026-10-11 (tools/rename.py). Address 0x8003399c.

Unit `src/editor.c`. The mirror of BackupToolBuffer: copies the 64 KiB at
0x8017D000 back to 0x8016D000, then calls `LoadEditorSlot(0, 0x8016D000)`.

Matched on the first build with the same `Page64K` struct assignment.
