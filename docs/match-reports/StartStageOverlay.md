# StartStageOverlay — MATCHED 56/56 (round 12, alpha)

> Renamed from `func_800412DC` on 2026-10-10 (tools/rename.py). Address 0x800412dc.

Unit `src/stage_overlay.c`. Dispatches on the current stage `gStage`
(0..13) to a per-stage overlay function; stages 2, 5, 8 and 11 have none.
Stages 6 and 7 call the same function, CopySpriteShapes3. Called from main.c
and stage_flow.

The function is 56 words (0xE0); the brief's 70 counted the jump table.

```c
void StartStageOverlay(void) {
    switch (gStage) {
        case 0:
            CopySpriteShapes1_1();
            break;
        case 1:
            CopySpriteShapes1_2();
            break;
        case 3:
            CopySpriteShapes2_1();
            break;
        case 4:
            CopySpriteShapes2_2();
            break;
        case 6:
            CopySpriteShapes3();
            break;
        case 7:
            /* MATCHING: keeps cross-jumping from merging this arm with case 6. */
            CopySpriteShapes3();
            __asm__("");
            break;
        case 9:
            CopySpriteShapes4_1();
            break;
        case 10:
            CopySpriteShapes4_2();
            break;
        case 12:
            CopySpriteShapes5_1();
            break;
        case 13:
            CopySpriteShapes5_2();
            break;
    }
}
```

## Notes

- First build 36/56, length exact: cases 6 and 7 were merged into one arm
  (one block shorter, the table pointing both at it). A bare `__asm__("")`
  after the call in case 7 keeps them apart (LEARNINGS, main), byte-exact on
  the second build.
- The overlay functions are prototyped in the `.c` (`void (void)`); the
  calls pass nothing and use no result.
