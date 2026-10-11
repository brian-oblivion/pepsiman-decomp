# EditHitQuads — MATCHED 162/162 (round 9, echo)

> Renamed from `func_80035350` on 2026-10-11 (tools/rename.py). Address 0x80035350.

Unit `src/hit_tool.c`. Twin of EditHitPoints on the second record bank
(`gHitQuads`, 0x4C-byte `Rec4C` records, 100 of them, index
`gToolQuadIndex`, position from the record's first point, reinstalled by
`ApplyHitQuads`). The same body, the same levers; see EditHitPoints's
report.

```c
void EditHitQuads(void) {
    VECTOR pos;
    Bank4C *bank;
    BankEntry *e;
    Rec4C *src;
    Rec4C *dst;
    Rec4C *rec;
    s32 k;
    s32 n;

    switch (gSubStep) {
        case 0:
            gToolQuadIndex = 0;
            gSubStep = 1;
            break;
        case 1:
            bank = (Bank4C *)gHitQuads;
            e = bank->entries;
            e += gToolSection;
            if (e->unk4 == 0) {
                gToolMode = 0;
                D_80095A58 = 0;
            }
            if (gPadTrig & 8) {
                gToolQuadIndex++;
            }
            if (gPadTrig & 4) {
                gToolQuadIndex--;
            }
            e = bank->entries;
            e += gToolSection;
            gToolQuadIndex = gToolQuadIndex<0 ? 0 : gToolQuadIndex>(u32)(e->unk4 - 1) ? e->unk4 - 1 : gToolQuadIndex;
            rec = GetHitQuad(gToolSection, gToolQuadIndex);
            pos.vx = -rec->pts[0].x;
            pos.vy = rec->pts[0].y;
            gToolQuad = rec;
            pos.vz = -rec->pts[0].z;
            if (gPadTrig & 0x40) {
                gToolMode = 0;
                D_80095A58 = 0;
            }
            if (D_80095A28 != 1 && (gPadTrig & 0x20)) {
                bank = (Bank4C *)gHitQuads;
                e = bank->entries;
                e += gToolSection;
                dst = (Rec4C *)&bank->entries[gSectionCount];
                src = &dst[e->first + gToolQuadIndex + 1];
                dst += e->first + gToolQuadIndex;
                for (k = e->first + gToolQuadIndex; k < 99; k++) {
                    *dst = *src;
                    dst++;
                    /* MATCHING: the empty do/while gives the source pointer
                     * $a1 and the block move's end $a3. */
                    do {
                        src++;
                    } while (0);
                }
                n = gSectionCount;
                /* MATCHING: a byte-pointer store keeps the index load below it. */
                *(s32 *)((u8 *)e + 4) -= 1;
                k = gToolSection + 1;
                for (; k < n; k++) {
                    e++;
                    e->first--;
                }
                ApplyHitQuads();
            }
            DrawToolPin(&pos);
            DrawToolPole(&pos);
            break;
    }
}
```

## Levers

EditHitPoints's body with the bank, record type, index global, record
accessor, bound (99 for 399) and reinstaller swapped: 157/162 with the same
`$a1`/`$a3` swap, 162/162 with EditHitPoints's `do { src++; } while (0);`.
