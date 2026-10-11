# CheckSave1Space — MATCHED 27/27 (round 3, echo)

> Renamed from `func_80038574` on 2026-10-10 (tools/rename.py). Address 0x80038574.

Unit `src/card.c`. Sums the halfword at 0x14 of the first `gCardFileCount` 0x18-byte records of `gCardFiles` into an `s16`, and returns -1 if the sum is positive, 0 otherwise.

```c
s16 CheckSave1Space(void) {
    s16 sum;
    s16 i;

    sum = 0;
    for (i = 0; i < gCardFileCount; i++) {
        sum += gCardFiles[i].blocks.lo;
    }
    if (sum > 0) {
        return -1;
    }
    return 0;
}
```

Matched on the first build. `gCardFiles` is reached `lui`/`addiu` with the
record base in a register, so it is an array of a unit-local `Rec18`
(0x14 bytes unknown, `u16 unk14`, 2 bytes unknown). The `s16` counter and
sum give the `sll`/`sra` pairs; `if (sum > 0) return -1;` gives `bgtz` on
the shifted sum with -1 in its delay slot.

`Rec18` became the file name plus a `BlockCount` union in round 5 (see
`ScanCardFiles.md`); the code above uses the new member names.
