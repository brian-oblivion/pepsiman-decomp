# UpdateLoadScreenModel — MATCHED 171/171 (round 8, echo)

> Renamed from `func_80016D14` on 2026-10-10 (tools/rename.py). Address 0x80016d14.

Unit `src/main.c`. A demo/replay frame: places the reference view
`gRefView` (as BeginLoadScreenModel does) and sets it, steps and draws the
state copy `gLoadScreenModel` with the bodies of StepStepper and DrawStepper
inlined, runs LoadStage, and at state 100 clears the fog, sets state
14 and starts track `gStage` (as UpdateLoadScreen's tail does).

The step and draw bodies now live once, as `static __inline__ stepObj` and
`drawObj` ahead of this function; StepStepper and DrawStepper are
one-line wrappers that still match (22/22, 94/94). The `ModelSet` view,
`sModels`, `D_80095758`, `D_80095980` and LoadStage's prototype moved
up to here (no byte change).

```c
/** @brief A list of models drawn together (a unit-local view). */
typedef struct {
    GsDOBJ2 *objs; /**< the models */
    s32 count;     /**< how many */
} ModelSet;

/* MATCHING: a struct lvalue over the shared array declaration. */
#define sModels (*(ModelSet *)gTodObjTable)

extern s32 D_80095758;

void LoadStage(void);

extern s32 D_80095980;

/** @brief One step of @p obj: runs the stepper and marks a finished 0x61
 *         channel. Inlined here and by the out-of-line step below. */
static __inline__ void stepObj(Stepper *obj) {
    /* MATCHING: signed, so the zero test is `sll 24`, not `andi 0xFF`. */
    s8 result;

    result = func_80017F0C(obj, 0, obj->unk4);
    obj->unk340 = result;
    switch (obj->unk5) {
        case 0x60:
            break;
        case 0x61:
            if (result == 0) {
                obj->unk8 = 0xFF;
            }
            break;
    }
}

/** @brief Draws @p obj's models at its position and rotation. Inlined
 *         here and by the out-of-line draw below. */
static __inline__ void drawObj(Stepper *obj) {
    MATRIX m;
    SVECTOR rot;
    GsDOBJ2 *o;
    s32 i;

    func_80020CF8(D_80095758);
    o = sModels.objs;
    o->coord2->coord.t[0] = obj->unk348;
    o->coord2->coord.t[1] = obj->unk34C;
    o->coord2->coord.t[2] = obj->unk350;
    rot.vx = 0;
    rot.vy = obj->unk380;
    rot.vz = 0;
    func_80018AE0(&rot, o->coord2);
    for (i = 0; i < sModels.count; o++, i++) {
        o->coord2->flg = 0;
        if (o->id != -1 && o->tmd != NULL) {
            GsGetLs(o->coord2, &m);
            GsSetLsMatrix(&m);
            GsGetLw(o->coord2, &m);
            GsSetLightMatrix(&m);
            /* MATCHING: retail runs this sort on the scratchpad stack. */
            SetSpadStack();
            GsSortObject4J(o, &gSubOt[gActiveBuff], 2, (u_long *)0x1F800000);
            ResetSpadStack();
        }
    }
}

void UpdateLoadScreenModel(void) {
    /* MATCHING: retail reserves 8 bytes below the inlined draw's locals. */
    s32 unused[2];

    gRefView[0] = rsin(0x800) * 400 / 4096;
    gRefView[1] = -150;
    gRefView[2] = rcos(0x800) * 400 / 4096;
    gRefView[3] = 0;
    gRefView[4] = -150;
    gRefView[5] = 0;
    GsSetRefView2((GsRVIEW2 *)gRefView);
    stepObj((Stepper *)&gLoadScreenModel);
    drawObj((Stepper *)&gLoadScreenModel);
    LoadStage();
    if (gSubStep == 100) {
        SetFogNearFar(0, 0, 250);
        SetFarColor(0, 0, 0);
        gModeStep = 4;
        D_80095980 = 0;
        gSubStep = 0;
        gGameMode = 14;
        PlayStageMusic(gStage);
    }
}
```

Second build. Retail's body is StepStepper and DrawStepper inlined on
`(Stepper *)&gLoadScreenModel`: same registers, the scratchpad-stack sort, the
`sll 24` test of the step result. The first build matched every
instruction but the frame: retail's is 8 bytes larger with the inlined
draw's `MATRIX` at `sp+0x18`, not `sp+0x10`. An unused `s32 unused[2]` in
the caller closes it: the caller's own locals are laid out before the
inlined helpers' (LEARNINGS, "a non-leaf frame whose locals all sit N
bytes high").

### Proposed learning

- A function whose body repeats another function of the unit with a fixed
  argument: write the callee's body once as a `static __inline__` helper
  and call it from both; the out-of-line function as a one-line wrapper
  can still match (StepStepper, DrawStepper here; it did NOT for
  StartRumble under UpdateRumble, which needed a duplicate).
