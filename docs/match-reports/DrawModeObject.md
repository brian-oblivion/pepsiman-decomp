# DrawModeObject — MATCHED 272/272 (round 10, bravo)

> Renamed from `func_800272D4` on 2026-10-11 (tools/rename.py). Address 0x800272d4.

Unit `src/player.c`. Places the stage objects of the current mode
(low nibble of unk3D0, then the stage `gStage`): for each it fills
the SlotRec of the slot table at `gModeObjects` (position minus the view
offset `gWorldShift`, a few angles), points the paired model slot in
`gModeObjectModels` at a coordinate system in `gTodCoords` and calls
DrawObjectModel on the pair. Mode 2 also turns the object by half of
unk3A8 degrees and plays sound 0x81 every 20 frames. Slot 1 is placed
whenever `D_8009EF48[0] >> 4` is 5.

```c
/** @brief A 0x78-byte model slot, paired with the SlotRec of the same
 *         index; main's ModelSlot is the same record. */
typedef struct {
    u8 pad0[0x58];
    s32 unk58; /**< the address of the coordinate system the model hangs from */
    u8 pad5C[0x78 - 0x5C];
} ObjRec;

extern SlotRec gModeObjects[];
/* MATCHING: main declares this as its ModelSlot; this unit's view names the word at 0x58. */
extern ObjRec gModeObjectModels[];
/* MATCHING: object types these Rec78 and Rec48; this unit's records are views of them. */
void DrawObjectModel(Rec78 *rec, Rec48 *r);

void DrawModeObject(void) {
    switch (sGame.unk3D0 & 0xF) {
        case 1:
            gModeObjects[0].unk18 = -0x88;
            gModeObjects[0].unk1A = 0xAA;
            gModeObjects[0].unk4 = 0;
            gModeObjectModels[0].unk58 = (s32)&gTodCoords[16];
            gModeObjects[0].unk0 = -gWorldShift[0];
            gModeObjects[0].unk8 = -gWorldShift[2];
            DrawObjectModel((Rec78 *)&gModeObjectModels[gModeObjects[0].unk36], (Rec48 *)&gModeObjects[0]);
            break;
        case 4:
            gModeObjects[5].unk18 = -0x88;
            gModeObjects[5].unk1C = -0x88;
            gModeObjects[5].unk1A = 0xAA;
            gModeObjects[5].unk4 = 0;
            gModeObjectModels[5].unk58 = (s32)&gTodCoords[16];
            gModeObjects[5].unk0 = -gWorldShift[0];
            gModeObjects[5].unk8 = -gWorldShift[2];
            DrawObjectModel((Rec78 *)&gModeObjectModels[gModeObjects[5].unk36], (Rec48 *)&gModeObjects[5]);
            break;
        case 2:
            switch (gStage) {
                case 3:
                    gModeObjects[3].unk4 = 0x3C;
                    gModeObjectModels[3].unk58 = (s32)&gTodCoords[0];
                    gModeObjects[3].unk0 = -gWorldShift[0];
                    gModeObjects[3].unk8 = -gWorldShift[2];
                    gModeObjects[3].unk18 += ANGLE_DEG(sGame.unk3A8 / 2);
                    DrawObjectModel((Rec78 *)&gModeObjectModels[gModeObjects[3].unk36], (Rec48 *)&gModeObjects[3]);
                    if (gFrameCount % 20 == 0) {
                        PlaySe(0x81);
                    }
                    break;
                case 10:
                    gModeObjects[6].unk4 = 0x32;
                    gModeObjectModels[6].unk58 = (s32)&gTodCoords[0];
                    gModeObjects[6].unk0 = -gWorldShift[0];
                    gModeObjects[6].unk8 = -gWorldShift[2];
                    gModeObjects[6].unk18 += ANGLE_DEG(sGame.unk3A8 / 2);
                    DrawObjectModel((Rec78 *)&gModeObjectModels[gModeObjects[6].unk36], (Rec48 *)&gModeObjects[6]);
                    if (gFrameCount % 20 == 0) {
                        PlaySe(0x81);
                    }
                    break;
            }
            break;
        case 3:
            switch (gStage) {
                case 0:
                    gModeObjects[4].unk4 = 0;
                    gModeObjectModels[4].unk58 = (s32)&gTodCoords[1];
                    gModeObjects[4].unk0 = -gWorldShift[0];
                    gModeObjects[4].unk8 = -gWorldShift[2];
                    DrawObjectModel((Rec78 *)&gModeObjectModels[gModeObjects[4].unk36], (Rec48 *)&gModeObjects[4]);
                    break;
                case 12:
                    gModeObjects[2].unk4 = 0;
                    gModeObjectModels[2].unk58 = (s32)&gTodCoords[1];
                    gModeObjects[2].unk0 = -gWorldShift[0];
                    gModeObjects[2].unk8 = -gWorldShift[2];
                    DrawObjectModel((Rec78 *)&gModeObjectModels[gModeObjects[2].unk36], (Rec48 *)&gModeObjects[2]);
                    break;
            }
            break;
    }
    if ((D_8009EF48[0] >> 4) == 5) {
        gModeObjects[1].unk4 = 0;
        gModeObjects[1].unk0 = -gWorldShift[0];
        gModeObjects[1].unk8 = -gWorldShift[2];
        DrawObjectModel((Rec78 *)&gModeObjectModels[gModeObjects[1].unk36], (Rec48 *)&gModeObjects[1]);
    }
}
```

`SlotRec` gained `unk0`, `unk4`, `unk8` (s32) and `unk18`, `unk1A`,
`unk1C` (s16). Matched on the first build.

## Levers

- The case bodies follow AttachModeObject's order (1, 4, 2, 3) and its
  nested `switch (gStage)`; one call per arm, which cross-jumping
  folds into the shared `jal` tails retail has.
- The addresses stored at +0x58 are `&gTodCoords[16]`, `[0]` and `[1]`
  (0x50-byte GsCOORDINATE2s): `D_800D8BE0` and `D_800D8730` in the asm
  are those elements, not their own objects.
- `ANGLE_DEG(sGame.unk3A8 / 2)` gives the `sra 1; sll 12` and the /360
  magic multiply unchanged.

### Proposed learning

The word symbols AttachModeObject stores to (`D_800D83C8`, `D_800D8530`,
`D_800D8620`, `D_800D8698`, ...) are `gModeObjectModels[k].unk58` for k = 0, 3,
5, 6; DrawModeObject reaches the same words through the table base. A
`gModeObjectModels` view with `unk58` could replace those per-slot externs
(head's call; AttachModeObject matched as it is).
