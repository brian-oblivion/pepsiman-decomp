# ReadEditorFile — MATCHED 22/22 (round 4, alpha)

> Renamed from `func_80033E40` on 2026-10-11 (tools/rename.py). Address 0x80033e40.

Unit `src/editor.c`. The read twin of `WriteEditorFile`: opens
`D_80011260` with `O_RDONLY` (1), reads 0x2285C bytes into 0x8016D000,
closes and returns the count. First build matched.

```c
s32 ReadEditorFile(void) {
    s32 fd;
    s32 n;

    fd = open(D_80011260, O_RDONLY);
    n = read(fd, (void *)0x8016D000, 0x2285C);
    close(fd);
    return n;
}
```
