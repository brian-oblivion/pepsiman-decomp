# OnCdTrackReady — MATCHED (11/11), round 3, runner charlie

> Renamed from `func_80017614` on 2026-10-10 (tools/rename.py). Address 0x80017614.

Unit `src/code_7d74.c`. When the byte argument is 4, issues CD command 3
(`CdlPlay`) through `SendCdTrackCommand`.

```c
void OnCdTrackReady(u8 mode) {
    if (mode == 4) {
        SendCdTrackCommand(CdlPlay);
    }
}
```

## Attempts (1)

1. As above: 11/11 first build. The `andi $a0, 0xFF` on entry is the `u8`
   parameter. `CdlPlay` (3) is from `libcd.h`; `SendCdTrackCommand` sends its
   argument to `CdControl` as the command byte.
