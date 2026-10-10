# CheckSaveSum — MATCHED (round 1, bravo)

> Renamed from `func_8003831C` on 2026-10-10 (tools/rename.py). Address 0x8003831c.

22/22 words. Verifies the 16-bit checksum of the 0xEFFF little-endian
halfwords at `gSaveBuf` against `expected`: 0 on a match, -1 otherwise.

```c
s16 CheckSaveSum(u16 expected) {
    u8 *p;
    u16 sum;
    s32 i;
    u8 lo;
    u8 hi;

    p = gSaveBuf;
    sum = 0;
    for (i = 0; i < 0xEFFF; i++) {
        lo = *p++;
        hi = *p++;
        sum += lo | (hi << 8);
    }
    if (sum != expected) {
        return -1;
    }
    return 0;
}
```

The loop is `SumSaveBuffer`'s body unchanged (that function returns the sum;
this one checks it). No lever: 2.8.1 keeps the sum unmasked in the loop and
masks both sides once at the compare, as retail.
