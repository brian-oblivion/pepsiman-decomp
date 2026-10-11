# PlaceHitQuad — MATCHED 38/38 (round 4, charlie)

> Renamed from `func_80036F50` on 2026-10-11 (tools/rename.py). Address 0x80036f50.

Unit `src/hit_tool.c`. State 1 of the two-state dispatch in
`ToolPlace`, itself a two-state machine on `gToolQuadStage`: state 0 sets two
halfwords to 100, points field 0x48 of `gToolNewQuad` at byte 0x10 of the
current `D_800D8D20` record, calls `SetBoxAround` and moves to state 1;
state 1 calls `InsertHitQuad` and goes back to 0. First build matched.

```c
void PlaceHitQuad(void) {
    switch (gToolQuadStage) {
        case 0:
            gToolQuadW = 100;
            gToolQuadH = 100;
            gToolNewQuad.unk48 = D_800D8D20[gToolSection].unk10;
            SetBoxAround(&gToolNewQuad, D_8009EEC0, 100, 100);
            gToolQuadStage = 1;
            break;
        case 1:
            InsertHitQuad();
            gToolQuadStage = 0;
            break;
    }
}
```

## Externs and types

- `gToolQuadStage` (`u8`), `gToolQuadW`, `gToolQuadH` (`s16`): `%gp_rel` in
  retail, plain externs.
- `gToolNewQuad`: retail builds its address once (`lui`/`addiu $a0`) and stores
  through `0x48($a0)`, so a complete object larger than 8 bytes: unit-local
  `Obj48` with the pointer at 0x48.
- The `D_800D8D30` in retail's `%lo(D_800D8D30)` is `D_800D8D20 + 0x10`; `Rec78`
  gained an `unk10` member so the source names it (same bytes, the
  relocation addend differs only in the symbol chosen by splat).
- Retail copies `$a2` into `$a3` (`move a3, a2`) for the two 100s: two equal
  literal arguments, no local needed.
