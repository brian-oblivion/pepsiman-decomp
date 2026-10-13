void StepSmokingTrap(StageTrap *t) {
    PlacedPos p;
    s32 x;
    s32 z;

    p.lx = 0;
    p.ly = -50;
    p.lz = -100;
    p.coord = (GsCOORDINATE2 *)gObjectModels[t->model].coord;
    UpdatePlacedPos(&p);
    t->homeX = p.pos.vx;
    t->homeY = p.pos.vy;
    t->homeZ = p.pos.vz;
    if (gFrameCount % 5 == 0) {
        x = (s16)p.pos.vx - (100 - rand() % 200);
        z = (s16)p.pos.vz - (200 - rand() % 400);
        SpawnEffect(1, x, (s16)p.pos.vy, z, (s16)(rand() % 3));
    }
    if (gSection - UpdatePathSegment(t) < 2) {
        UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
    }
}
