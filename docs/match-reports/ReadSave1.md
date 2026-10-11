# ReadSave1 — MATCHED 97/97 (round 9, delta)

> Renamed from `func_800377E8` on 2026-10-10 (tools/rename.py). Address 0x800377e8.

Unit `src/card.c`. Reads the first save file `"bu10:BISLPS-12345PEPTOOL"`
(`D_80011998`) into `gSaveBuf`: `open(name, 1)`, `lseek(fd, 0, 0)`, then
`read(fd, buf, 0x1E000)`, seeking back after a short read. Each step up to
ten tries. Failure codes in `gCardError`: 1 (open), 2 (seek), 3 (read).
Twin of ReadSave2.

Matched on the first build.

```c
/** @brief Reads the first save file into the save buffer, up to ten tries per step; 0, or -1 with the failed step recorded. */
s32 ReadSave1(void) {
    s32 ret;
    s16 i;
    s32 fd;
    u8 *buf;
    s16 tries;

    ret = 0;
    for (i = 0; i < 10; i++) {
        fd = open(D_80011998, 1);
        if (fd != -1) {
            goto opened;
        }
    }
    gCardError = 1;
    ret = -1;
    goto end;
opened:
    for (i = 0; i < 10; i++) {
        if (lseek(fd, 0, 0) != -1) {
            goto seeked;
        }
    }
    close(fd);
    gCardError = 2;
    ret = -1;
    goto end;
seeked:
    buf = gSaveBuf;
    for (tries = 0; tries < 10; tries++) {
        if (read(fd, buf, 0x1E000) == 0x1E000) {
            goto done;
        }
        for (i = 0; i < 10; i++) {
            if (lseek(fd, 0, 0) != -1) {
                goto reseeked;
            }
        }
        close(fd);
        gCardError = 2;
        ret = -1;
        goto end;
    reseeked:;
    }
    close(fd);
    gCardError = 3;
    ret = -1;
    goto end;
done:
    close(fd);
end:
    return ret;
}
```

## Reading the asm

- **Here retail SHARES one fail tail** (`.L80037930: sh v0, G; j end; li
  s3, -1`, every failure jumping there with its code in `$v0`): the order
  `G = K; ret = -1;` that cross-jumping can merge. The write functions
  (CreateSave1, RewriteSave1) keep separate tails and need `ret = -1;`
  first. Read which shape the asm shows before choosing the order.
- Two counters, unlike RewriteSave1: the read loop counts in `$s2`, the seek
  loops in `$s0`, so the read loop has its own `s16` local.
