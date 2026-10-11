# LoadCommonModels — MATCHED 130/130 (round 8, echo)

> Renamed from `func_80013CDC` on 2026-10-11 (tools/rename.py). Address 0x80013cdc.

Unit `src/main.c`. Loads the stage's model data: maps the TMD pack at
0x8014D000 (`GsMapModelingData`, address kept in `gModelTmd`), sets up the
model `D_800963A0` and seven model slots from it (`InitModel`), builds
the slot list (`InitTodObjTable`), registers the entries of the directory at
0x80123000 into `gAnimations` and the stepper's two word tables (the same
loop as object's LoadStageSprites, from slot 0), then resets the stepper
(bytes 5..9) and runs its first step.

Declaration changes: the `Stepper` typedef moved up to here and its
`unk6[2]`/`unk9[0x337]` split into `unk6`, `unk7`, `unk9`, `unkA[0x16]`,
`unk20[100]`, `unk1B0[100]` (same size, no byte change elsewhere);
`StepAnimation`'s prototype moved up with it; `LoadCommonModels` is now
`void` (was an `s32` prototype); `gModelTmd` moved from `src/model.c`
to `include/common.h` (same type; declcheck's MULTI). main.c now includes
`object.h` (for `gTodCoords`) and carries unit-local copies of
`Model70` and `DirEnt16`.

```c
/* The stepper's state lives at the head of the game state. */
#define sStepper (*(Stepper *)gPlayer)

/** @brief A model object with its own coordinate system (a unit-local
 *         copy of object's view). */
typedef struct {
    GsDOBJ2 obj;         /**< the object handler */
    GsCOORDINATE2 coord; /**< the object's coordinate system */
    SVECTOR rot;         /**< rotation */
    SVECTOR scale;       /**< scale, 0x1000 = 1 */
} Model70;

/** @brief A 16-byte directory entry of a loaded file. */
typedef struct {
    s32 offset;   /**< byte offset of the entry from the directory */
    u8 unk4[0xA]; /**< not yet known */
    u16 count;    /**< entry count; read from the first entry only */
} DirEnt16;

extern Model70 D_800963A0;

/** @brief A 0x78-byte model slot: a Model70 and 8 bytes not yet known. */
typedef struct {
    Model70 m;   /**< the model */
    u8 unk70[8]; /**< not yet known */
} ModelSlot;

extern ModelSlot gModeObjectModels[];
extern u8 gTodObjs[];
extern u8 gTodCoordParams[];

/* MATCHING: object defines it on its own Model70 view. */
void InitModel(Model70 *m, unsigned long *tmd, u8 n);
/* MATCHING: model types the list, slots and data as its own records. */
void InitTodObjTable(s32 *list, u8 *slots, GsCOORDINATE2 *objs, u8 *data, s32 n);
/* MATCHING: stage_flow passes the game state's head in its own view. */
u8 StepAnimation(Stepper *obj, u16 index, u8 arg);

void LoadCommonModels(void) {
    /* MATCHING: one pointer reused for the pack, its second word and the
     * directory, so all three share a saved register. */
    s32 *p;
    s32 *q;

    p = (s32 *)0x8014D000;
    gModelTmd = (s32)((unsigned long *)((u8 *)p + *p) + 1);
    GsMapModelingData((unsigned long *)gModelTmd);
    InitModel(&D_800963A0, (unsigned long *)((u8 *)p + *p), 16);
    p = (s32 *)0x8014D010;
    InitModel(&gModeObjectModels[0].m, (unsigned long *)(*p + 0x8014D000), 0);
    InitModel(&gModeObjectModels[1].m, (unsigned long *)(*p + 0x8014D000), 1);
    InitModel(&gModeObjectModels[2].m, (unsigned long *)(*p + 0x8014D000), 2);
    InitModel(&gModeObjectModels[3].m, (unsigned long *)(*p + 0x8014D000), 3);
    InitModel(&gModeObjectModels[4].m, (unsigned long *)(*p + 0x8014D000), 4);
    InitModel(&gModeObjectModels[5].m, (unsigned long *)(*p + 0x8014D000), 5);
    InitModel(&gModeObjectModels[6].m, (unsigned long *)(*p + 0x8014D000), 6);
    InitTodObjTable(gTodObjTable, gTodObjs, gTodCoords, gTodCoordParams, 20);
    p = (s32 *)0x80123000;
    gLoopI = 0;
    gLoopJ = ((DirEnt16 *)p)->count;
    for (; gLoopI < gLoopJ; gLoopI++) {
        /* MATCHING: assigned inside the store, as in LoadStageSprites. */
        gAnimations[gLoopI] = (q = (s32 *)((u8 *)0x80123000 + *p)) + 1;
        p += 4;
        sStepper.unk20[gLoopI] = *gAnimations[gLoopI];
        gAnimations[gLoopI]++;
        sStepper.unk1B0[gLoopI] = gAnimations[gLoopI][1];
    }
    GsInitCoordinate2(WORLD, &gTodCoords[0]);
    sStepper.unk5 = 0;
    sStepper.unk6 = 0;
    /* MATCHING: retail stores 8 before 7. */
    sStepper.unk8 = 0xFF;
    sStepper.unk7 = 0;
    sStepper.unk9 = 0xFF;
    StepAnimation(&sStepper, 0, 0);
}
```

Seven builds. Points:

- The seven slots are 0x78 bytes apart, not `sizeof(Model70)` (0x70):
  `ModelSlot` wraps a Model70 and eight unknown bytes.
- Retail keeps 0x8014D000 in TWO saved registers: `$s2` as the address the
  first word is read through (and added to), `$s1` as the constant added to
  the second word. `$s2` is then reloaded with 0x8014D010 and later with
  the directory 0x80123000 before the seventh call. One `s32 *p` reused for
  all three gives exactly that; a separate `DirEnt16 *e` was hoisted to the
  prologue (assigned before the call) or landed in a scratch register
  (after it).
- `(u8 *)p + *p` then `(unsigned long *)... + 1` keeps the `+ 4` as its own
  `addiu`; `*p + 0x8014D000 + 4` folds it into the constant.
- The loop needs LoadStageSprites's `(q = base + off) + 1` store, which keeps
  the record pointer in its own register before the `+ 4`.
- The tail stores byte 8 before byte 7 in retail; source order follows.

### Proposed learning

- A constant address held in a saved register that is later reloaded
  with other addresses (before a call that does not use it): one pointer
  local reused for each, `p = A; ... p = B; ... p = C;`. A fresh local for
  the last one is hoisted or demoted.
