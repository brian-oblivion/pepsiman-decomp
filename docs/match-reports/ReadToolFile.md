# ReadToolFile — MATCHED 22/22 (round 2, charlie)

> Renamed from `func_80037370` on 2026-10-11 (tools/rename.py). Address 0x80037370.

Unit `src/hit_tool.c`. Debug tool load: opens
`sim:\PS\PEPSI\DATA\TOOL1\TMP.TL1` (the `sim:` PC file server device) for
reading, reads 0x13868 bytes into the fixed RAM address
0x8018D000, closes it, and returns `read`'s result. Game code calling
libapi's file I/O, not Sony library code.

```c
s32 ReadToolFile(void) {
    s32 fd;
    s32 n;

    fd = open(sHitTmpFilePath, O_RDONLY);
    n = read(fd, (void *)0x8018D000, 0x13868);
    close(fd);
    return n;
}
```

## Externs

`open`, `read`, `close` from Sony's `libapi.h`; `O_RDONLY` (1) from
Sony's `sys/file.h`. The path is the rodata
string `sHitTmpFilePath`, kept as `INCLUDE_RODATA` and declared
`extern char sHitTmpFilePath[];` (`open` takes a non-const `char *`).

## Levers

None needed: first try, as the twin of `WriteToolFile`.
