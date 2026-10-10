# SetSave2Buffers — MATCHED (round 1, bravo)

> Renamed from `func_800394EC` on 2026-10-10 (tools/rename.py). Address 0x800394ec.

8/8 words. The sibling of `SetSave1Buffers` with the two buffers moved up by
0x8000 (both pointers are `%gp_rel`, declared `extern u8 *`):

```c
void SetSave2Buffers(void) {
    D_800959D4 = (u8 *)0x80195000;
    gSaveBuf = (u8 *)0x8018D000;
}
```
