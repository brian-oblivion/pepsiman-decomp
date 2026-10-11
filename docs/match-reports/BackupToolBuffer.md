# BackupToolBuffer — MATCHED 27/27 (round 5, alpha)

> Renamed from `func_80033930` on 2026-10-11 (tools/rename.py). Address 0x80033930.

Unit `src/editor.c`. Calls `SaveEditorSlot(0, 0x8016D000)`, then copies
the 64 KiB at 0x8016D000 to 0x8017D000.

Matched on the first build. The four-`lw`/four-`sw` loop with an end
pointer is cc1's block move for a large struct assignment: a unit-local
`Page64K` (`u8 b[0x10000]`) and `*(Page64K *)0x8017D000 = *(Page64K
*)0x8016D000;`. The source address is rebuilt after the call (cc1 does not
keep the call argument's constant across it), as retail shows.

### Proposed learning

- **A loop of four `lw` then four `sw`, both pointers bumped by 16, to an end
  pointer: a struct assignment of a large object** (cc1's block move).
  `*(Big *)dst = *(Big *)src;` with a one-member byte-array struct.
  (BackupToolBuffer, RestoreToolBuffer)
