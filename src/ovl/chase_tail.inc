/* The end the act-3 chase stages share: placing the objects, the loader
 * and the reset, after the scene's lights. The stage's unit defines
 * CHASE_LIGHT_BLUE (the lights' blue), CHASE_FAR_COLOR (the fog's colour,
 * three components) and CHASE_VAB, its sound bank, and
 * may define CHASE_LOADER as the INCLUDE_ASM that stands for the loader
 * while its C does not build to the unit's bytes. */

/* Sets the scene's three lights and its fog. */
void SetChaseLights(void) {
    sLights.l[0].r = 0x80;
    sLights.l[0].g = 0x80;
    sLights.l[0].b = CHASE_LIGHT_BLUE;
    sLights.l[1].r = 0x80;
    sLights.l[1].g = 0x80;
    sLights.l[1].b = CHASE_LIGHT_BLUE;
    sLights.l[2].r = 0x80;
    sLights.l[2].g = 0x80;
    sLights.l[2].b = CHASE_LIGHT_BLUE;
    sLights.l[0].vx = 0;
    sLights.l[0].vy = 100;
    sLights.l[0].vz = 100;
    GsSetFlatLight(0, &sLights.l[0]);
    sLights.l[1].vx = 0x56;
    sLights.l[1].vy = 100;
    sLights.l[1].vz = -50;
    GsSetFlatLight(1, &sLights.l[1]);
    sLights.l[2].vx = -0x56;
    sLights.l[2].vy = 100;
    sLights.l[2].vz = -50;
    GsSetFlatLight(2, &sLights.l[2]);
    SetFogNearFar(3000, 8000, 250);
    SetFarColor(CHASE_FAR_COLOR);
}

/* Frees every object, then places the stage's 103. */
void PlaceChaseObjects(void) {
    Act3Obj *o;
    s32 i;

    o = sObjs;
    for (i = 0; i < 200; i++) {
        o[i].state = -1;
        o[i].timer = 0;
    }
    for (i = 0; i < sizeof(sPlacements) / sizeof(sPlacements[0]); i++) {
        o[i].kind = sPlacements[i].kind;
        o[i].x = sPlacements[i].x;
        o[i].y = sPlacements[i].y;
        o[i].z = sPlacements[i].z;
        o[i].state = 0;
    }
}

#ifdef CHASE_LOADER
CHASE_LOADER
#else
/* Loads the stage: its textures, its act-3 data and sound bank. */
s32 LoadChaseStage(void) {
    /* MATCHING: an s16 result keeps gSubStep's increment off its register;
     * an unused 8-byte local gives retail's frame. */
    s16 ret = 0;
    s32 unused[2];
    u32 i;

    switch (gSubStep) {
        case 0:
            SetBootReady(0);
            gCdLoadLoc[0] = (s32)&sChaseFileLoc[0];
            gCdLoadDest[0] = 0x8019D000;
            gCdLoadCount = 1;
            gCdLoadState = CD_LOAD_SEEK;
            gCdLoadSectors[0] = sChaseFileSectors[0];
            gSubStep++;
            break;
        case 1:
            if (gCdLoadState == CD_LOAD_DONE) {
                LoadTimList((unsigned long *)0x8019D000);
                gSubStep++;
            }
            break;
        case 2:
            gCdLoadLoc[0] = (s32)&sChaseFileLoc[1];
            gCdLoadDest[0] = 0x8019D000;
            gCdLoadCount = 1;
            gCdLoadState = CD_LOAD_SEEK;
            gCdLoadSectors[0] = sChaseFileSectors[1];
            gSubStep++;
            break;
        case 3:
            if (gCdLoadState == CD_LOAD_DONE) {
                LoadAct3Data((unsigned long *)0x8019D000);
                gSubStep++;
            }
            break;
        case 4:
            if (LoadVabBank(CHASE_VAB) == 1) {
                gCdLoadState = CD_LOAD_DONE;
                gSubStep++;
            }
            break;
        case 5:
        case 8:
            if (gCdLoadState == CD_LOAD_DONE) {
                gSubStep++;
            }
            break;
        case 6:
            gSectionCount = 0;
            gSubStep++;
            break;
        case 7:
            gCdLoadState = CD_LOAD_DONE;
            gSubStep++;
            break;
        case 9:
            for (i = 0; i < 200; i++) {
                ((Trap *)gTraps)[i].model = -1;
                ((Trap *)gTraps)[i].switchNo = -1;
                ((Trap *)gTraps)[i].marks = 1;
            }
            gCdLoadState = CD_LOAD_DONE;
            gSubStep++;
            break;
        case 10:
            gSubStep++;
            break;
        case 11:
            ret = 1;
            PlaceChaseObjects();
            SetBootReady(1);
            break;
    }
    FntPrint("CDEXEC %d", gSubStep);
    return ret;
}
#endif

/* Resets the stage: places its objects again. */
void ResetChaseStage(void) {
    PlaceChaseObjects();
    gWorldIntroStep = 0;
}
