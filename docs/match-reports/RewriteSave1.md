# RewriteSave1 — MATCHED 94/94 (round 9, delta)

> Renamed from `func_8003796C` on 2026-10-10 (tools/rename.py). Address 0x8003796c.

Unit `src/code_27bc8.c`. Rewrites the first save file `"bu10:BISLPS-12345PEPTOOL"`
(`D_80011998`) in place: `open(name, 2)`, `lseek(fd, 0, 0)`, then
`write(fd, gSaveBuf, 0x1E000)`; after a short write it seeks back to 0 and
retries. Each step up to ten tries. Failure codes in `gCardError`: 5 (open),
6 (seek), 7 (write). Twin of RewriteSave2.

Matched on the first build, using CreateSave1's shape (single-exit `ret`,
`ret = -1;` before the code store).

```c
/** @brief Rewrites the first save file in place, up to ten tries per step; 0, or -1 with the failed step recorded. */
s32 RewriteSave1(void) {
    s32 ret;
    s16 i;
    s32 fd;
    u8 *buf;

    ret = 0;
    for (i = 0; i < 10; i++) {
        fd = open(D_80011998, 2);
        if (fd != -1) {
            goto opened;
        }
    }
    ret = -1;
    gCardError = 5;
    goto end;
opened:
    for (i = 0; i < 10; i++) {
        if (lseek(fd, 0, 0) != -1) {
            goto seeked;
        }
    }
    close(fd);
    ret = -1;
    gCardError = 6;
    goto end;
seeked:
    buf = gSaveBuf;
    for (i = 0; i < 10; i++) {
        if (write(fd, buf, 0x1E000) == 0x1E000) {
            goto written;
        }
        for (i = 0; i < 10; i++) {
            if (lseek(fd, 0, 0) != -1) {
                goto reseeked;
            }
        }
        close(fd);
        ret = -1;
        gCardError = 6;
        goto end;
    reseeked:;
    }
    close(fd);
    ret = -1;
    gCardError = 7;
    goto end;
written:
    close(fd);
end:
    return ret;
}
```

## Reading the asm

- **One `s16` counter shared by the write loop and the re-seek loop nested
  in it.** Retail resets `$s0` in the delay slot after `write`, and after a
  successful re-seek tests `(i + 1) < 10` without storing it back: the outer
  `i++` whose value the next write pass overwrites.
- The first seek loop's failure is `j` to the nested loop's failure block
  (`close; 6`): cross-jumping merged two identical source tails. Write both.
- `gSaveBuf` is read once into `$s4` before the write loop: a `buf` local.
