void ResetSpawnedTraps(void) {
    u32 i;
    StageTrap *t;
    ModelSlot *a;
    ModelSlot *b;
    ObjectModel *m;

    if (gSkipLandRumble == 1 || sSpawnedTrapsReady == 0) {
        bzero((u8 *)sSpawnedTraps, 200 * sizeof(StageTrap));
        for (i = 0; i < 200; i++) {
            sSpawnedTraps[i].switchNo = -1;
            sSpawnedTraps[i].model = -1;
            sSpawnedTraps[i].state = 0;
            sSpawnedTraps[i].timer = 0;
        }
        for (i = 0; i < 10; i++) {
            D_800D39D8[i].model = -1;
            D_800D39D8[i].state = 0;
            D_800D39D8[i].timer = 0;
        }
        for (i = 0; i < 2; i++) {
            sGoalTraps[i].model = -1;
            sGoalTraps[i].state = 0;
            sGoalTraps[i].timer = 0;
        }
        sSpawnedTrapsReady = 1;
        gSlotTable = (s32)sSpawnedTraps;
        gSlotTableSize = 200 * sizeof(StageTrap);
        /* MATCHING: the model table in a local, so its base is built before
         * the trap table's. */
        m = gObjectModels;
        t = (StageTrap *)gTraps;
        a = (ModelSlot *)&m[t[194].model];
        b = (ModelSlot *)&m[t[195].model];
        b->scale.vy = a->scale.vy = b->scale.vx = a->scale.vx = 0x1000;
    }
}
