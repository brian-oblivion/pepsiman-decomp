# CheckSave2Space — MATCHED 29/29 (round 3, echo)

> Renamed from `func_80039618` on 2026-10-10 (tools/rename.py). Address 0x80039618.

Unit `src/card.c`. Same sum as `CheckSave1Space`; returns -1 if it is 4 or more, 0 otherwise.

```c
s16 CheckSave2Space(void) {
    s16 sum;
    s16 i;

    sum = 0;
    for (i = 0; i < gCardFileCount; i++) {
        sum += gCardFiles[i].blocks.lo;
    }
    if (sum >= 4) {
        return -1;
    }
    return 0;
}
```

## Levers

- **Which return the branch carries follows the `if`.** `if (sum < 4)
  return 0; return -1;` gave `bnez` with 0 in its slot (26/29). Retail's
  `beqz` with -1 in the slot is `if (sum >= 4) return -1; return 0;`.

`Rec18` became the file name plus a `BlockCount` union in round 5 (see
`ScanCardFiles.md`); the code above uses the new member names.
