# CreateSave2 — MATCHED 82/82 (round 9, delta)

> Renamed from `func_80038F70` on 2026-10-10 (tools/rename.py). Address 0x80038f70.

Unit `src/code_27bc8.c`. Twin of CreateSave1 for the second save file
`"bu10:BISLPS-67890PEPTOOL"` (`D_80011C24`): `open(name, 0xC0200)` (O_CREAT,
12 blocks), then a write of 0x18000 bytes from `gSaveBuf`. Same failure
codes 8, 9, 10 in `gCardError`.

Matched on the first build by copying CreateSave1's body and changing the
name, the create mode and the size; the asm of the two differs only in those
immediates and the labels. See CreateSave1's report for the levers (single
exit `ret`, `ret = -1;` before the global store to keep the fail tails
apart).

```c
s32 CreateSave2(void) {
    s32 ret;
    s16 i;
    s32 fd;

    ret = 0;
    for (i = 0; i < 10; i++) {
        fd = open(D_80011C24, 0xC0200);
        if (fd != -1) {
            goto created;
        }
    }
    ret = -1;
    gCardError = 8;
    goto end;
created:
    close(fd);
    for (i = 0; i < 10; i++) {
        fd = open(D_80011C24, 2);
        if (fd != -1) {
            goto opened;
        }
    }
    ret = -1;
    gCardError = 9;
    goto end;
opened:
    for (i = 0; i < 10; i++) {
        if (write(fd, gSaveBuf, 0x18000) == 0x18000) {
            goto written;
        }
    }
    close(fd);
    ret = -1;
    gCardError = 10;
    goto end;
written:
    close(fd);
end:
    return ret;
}
```
