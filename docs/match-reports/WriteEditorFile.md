# WriteEditorFile — MATCHED 22/22 (round 4, alpha)

> Renamed from `func_80033DE8` on 2026-10-11 (tools/rename.py). Address 0x80033de8.

Unit `src/code_1dc24.c`. Opens the host file `D_80011260`
("sim:\PS\PEPSI\DATA\TOOL0\TMP.TL0") with `O_CREAT | O_WRONLY` (0x202),
writes 0x2285C bytes from 0x8016D000, closes it and returns the count. A
copy of `code_24748`'s `WriteToolFile` with another path, buffer and size;
first build matched.

```c
s32 WriteEditorFile(void) {
    s32 fd;
    s32 n;

    fd = open(D_80011260, O_CREAT | O_WRONLY);
    n = write(fd, (void *)0x8016D000, 0x2285C);
    close(fd);
    return n;
}
```

`open`/`write`/`close` come from `libapi.h`, `O_*` from `sys/file.h`, both
now included by the unit. `D_80011260` is the unit's own rodata string,
declared `extern char D_80011260[];` (not retyped).
