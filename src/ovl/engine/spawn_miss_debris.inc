void SpawnMissDebris(void) {
    s16 models[15] = {4, 0, 0, 3, 0, 0, 0, 0, 0, 0, 6, 0, 2, 5, 0};
    s16 i;
    s16 n;
    StageTrap *t;

    for (i = 0; i < 8; i++) {
        n = FindFreeSlot48();
        if (n == -1) {
            break;
        }
        t = &sSpawnedTraps[n];
        t->switchNo = 0xB8;
        t->type = 0xFF;
        t->act = 0x6A;
        gSwitches[0xB8].state = SWITCH_ON;
        t->homeY = gPlayer.groundY - 50;
        t->x = gPlayer.x + (rand() % 200 - 100);
        t->z = gPlayer.z + (rand() % 200 - 100);
        t->model = models[gStage];
        t->homeZ = rand() % 10 + 5;
        t->homeX = sBurstHeights[i] / 2;
        t->rotY = ANGLE_DEG(sBurstSpins[i][0] * 10) + gCourse.rot.vy + ANGLE_DEG(180);
        t->homeRotZ = rand() % 20 + 10;
        t->homeRotX = 0;
        t->timer = 0;
        t->state = 0;
    }
}
