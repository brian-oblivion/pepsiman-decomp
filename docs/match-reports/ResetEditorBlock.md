# ResetEditorBlock — MATCHED 21/21 (round 3, alpha)

> Renamed from `func_80033790` on 2026-10-11 (tools/rename.py). Address 0x80033790.

Unit `src/editor.c`. Clears a fixed 0x800-byte block at `0x801FD000`
and hands it to `BuildEmptyBlock`, then clears the 100-record `StageSprite` table
`gStageSprites` (0x1770 bytes) and resets it with `ClearStageSprites`. First
build matched.

```c
void ResetEditorBlock(void) {
    bzero((u8 *)0x801FD000, 0x800);
    BuildEmptyBlock((u8 *)0x801FD000);
    bzero((u8 *)gStageSprites, sizeof(StageSprite) * 100);
    ClearStageSprites(gStageSprites);
}
```

`bzero` comes from Sony's `memory.h`, now included by the unit.
`gStageSprites` is declared `extern StageSprite gStageSprites[];`: retail reaches it
`lui $s0` / `addiu $s0,$s0,%lo` once and keeps it in `$s0` across the call.
