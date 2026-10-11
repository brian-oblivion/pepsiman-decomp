# ResetStageOverlay — MATCHED 76/76 (round 12, alpha)

> Renamed from `func_800413BC` on 2026-10-10 (tools/rename.py). Address 0x800413bc.

Unit `src/stage_overlay.c`. The second per-stage dispatcher, the twin of
StartStageOverlay: switches on `gStage` (0..14) and calls each stage's
overlay function; stages 6 and 7 share func_800F1908. Called from
stage_flow and editor.

The function is 76 words (0x130); the brief's 91 counted the jump table.

Matched on the first build, written with StartStageOverlay's spelling: one
`case` per stage, and a bare `__asm__("")` after the call in case 7 so
cross-jumping keeps the two identical arms apart.

```c
void ResetStageOverlay(void) {
    switch (gStage) {
        case 0:
            func_800FAA30();
            break;
        /* ... one call per case, in stage order ... */
        case 6:
            func_800F1908();
            break;
        case 7:
            /* MATCHING: keeps cross-jumping from merging this arm with case 6. */
            func_800F1908();
            __asm__("");
            break;
        /* ... */
        case 14:
            func_800F3A28();
            break;
    }
}
```

The full body is in `src/stage_overlay.c`.
