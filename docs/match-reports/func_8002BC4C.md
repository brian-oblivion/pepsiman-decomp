# func_8002BC4C — MATCHED 45/45 (round 6, alpha)

Unit `src/code_1a098.c`. For each of the 200 Rec48 records that is live
(`unk36 != -1`) and whose Rec5C record (`D_800CF080[unk34]`) is marked 1,
calls `func_8002A7D8(&D_800D8D20[unk36], record)`.

```c
void func_8002BC4C(void) {
    u32 i;

    for (i = 0; i < 200; i++) {
        if (sRecs48[i].unk36 != -1 && D_800CF080[sRecs48[i].unk34].unk0 == 1) {
            func_8002A7D8(&D_800D8D20[sRecs48[i].unk36], &sRecs48[i]);
        }
    }
}
```

First build. Indexing `sRecs48[i]` strength-reduces to the walking
`$s0` at the record start, as retail has it; the `u32` counter gives the
`sltiu`.

**Prototype fix.** `func_8002A7D8` (this unit, still asm) takes two
arguments: its asm keeps `$a1` in `$s2` and clears byte 0x40 through it.
code_24748.c declared it locally with one (`Rec78 *`). The prototype now
lives in `include/code_1a098.h` (additive), code_24748.c includes that
header, its local prototype is gone, and its one call
(func_80037114) passes the second argument it was already leaving in `$a1`:
`func_8002A7D8(&D_800D8D20[D_80095A30], (Rec48 *)D_800A9008);`. The whole
image stays `OK` (cc1 reuses the `D_800A9008` address it had built for the
stores just before), and declcheck's LOCAL finding for it is gone.
