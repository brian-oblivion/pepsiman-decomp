# InitLights — MATCHED (66/66), round 5, runner charlie

> Renamed from `func_80018094` on 2026-10-10 (tools/rename.py). Address 0x80018094.

Unit `src/code_7d74.c`. Lighting and fog set-up, called from main: three
flat lights in the table `gFlatLights` (directions (0, 100, 100), (86, 100,
-50), (-86, 100, -50), all coloured 0xD0 grey), ambient 0x400 on each
channel, light mode 0, and fog with `dqa` -0x3200, `dqb` 0x1400000 and
the colour from the bytes `gClearR`, `gClearG`, `gClearB`.

```c
typedef struct {
    GsF_LIGHT l[3];
} FlatLights;

#define sLights ((*(FlatLights *)gFlatLights).l)

void InitLights(void) {
    GsFOGPARAM fog;

    sLights[0].vx = 0;
    sLights[0].vy = 100;
    sLights[0].vz = 100;
    GsSetFlatLight(0, &sLights[0]);
    sLights[1].vx = 86;
    sLights[1].vy = 100;
    sLights[1].vz = -50;
    GsSetFlatLight(1, &sLights[1]);
    sLights[2].vx = -86;
    sLights[2].vy = 100;
    sLights[2].vz = -50;
    sLights[0].r = 0xD0;
    sLights[0].g = 0xD0;
    sLights[0].b = 0xD0;
    sLights[1].r = 0xD0;
    sLights[1].g = 0xD0;
    sLights[1].b = 0xD0;
    sLights[2].r = 0xD0;
    sLights[2].g = 0xD0;
    sLights[2].b = 0xD0;
    GsSetFlatLight(2, &sLights[2]);
    GsSetAmbient(0x400, 0x400, 0x400);
    GsSetLightMode(0);
    fog.dqa = -0x3200;
    fog.dqb = 0x1400000;
    fog.rfc = gClearR;
    fog.gfc = gClearG;
    fog.bfc = gClearB;
    GsSetFogParam(&fog);
}
```

## Attempts (2)

1. `#define sLights ((GsF_LIGHT *)gFlatLights)`: right instructions, but
   cc1 kept `&sLights[1]` in a fourth saved register and addressed light 1
   and 2's fields from it and from `$a1`; 67 words, frame 8 bytes bigger.
2. The table as a struct lvalue holding `GsF_LIGHT l[3]`: every field is
   an offset from the one base in `$s0`, as retail. 66/66.

## Declarations moved (no layout change)

- `gClearB`, `gClearG`, `gClearR` (`u8`, reached `lui` with the
  value in the same register: plain scalars) were declared locally in
  `src/main.c`; now in `include/common.h` (ADDITIVE) and removed from
  main.c.
- The prototype `void InitLights(void);` moved from `src/main.c` to
  `include/common.h` (ADDITIVE), as declcheck requires once the
  definition is C.

### Proposed learning

- The struct-lvalue lever (func_800285B0) holds for an array of Sony
  structs too: `(*(Wrapper *)D).l[i].f`, with a one-member `T l[N]`
  wrapper, folds every element's offset onto one base. Pointer arithmetic
  on `(T *)D` lets cc1 keep `&D[i]` as its own register. (InitLights)
