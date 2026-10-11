# SetSave1Buffers — MATCHED (round 1, bravo)

> Renamed from `func_80038374` on 2026-10-10 (tools/rename.py). Address 0x80038374.

8/8 words. Two `%gp_rel` pointer stores of constant addresses:

```c
void SetSave1Buffers(void) {
    sSaveData = (u8 *)0x8018D000;
    gSaveBuf = (u8 *)0x8016D000;
}
```

`sSaveData` is declared locally as `extern u8 *` (in `config/gp-symbols.txt`,
stored whole, so a small complete object). No lever needed: 2.8.1 emits both
`lui/ori` pairs before the stores, as retail does.
