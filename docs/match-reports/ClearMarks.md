# ClearMarks — MATCHED 11/11 (round 1, alpha)

> Renamed from `func_8002C5A4` on 2026-10-11 (tools/rename.py). Address 0x8002c5a4.

Unit `src/object.c`. Clears byte 0 of the first `count` records of a
table of 0x5C-byte records (`SwitchBox`, the type `ClearSwitches` and
`ClearSwitchBoxUpper` walk). First build matched.

```c
void ClearMarks(SwitchBox *recs, u16 count) {
    u16 i;

    for (i = 0; i < count; i++) {
        recs->unk0 = 0;
        recs++;
    }
}
```

Both the count and the counter are `u16`: retail masks the argument once
(`andi $a1,$a1,0xFFFF`) and the counter on every test (`andi $v0,$v1,0xFFFF;
sltu`). The `for` gives the `beqz` entry guard by itself.
