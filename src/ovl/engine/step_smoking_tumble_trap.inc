void StepSmokingTumbleTrap(StageTrap *t) {
    PlacedPos p;
    s32 x;
    s32 z;

    p.lx = 0;
    p.ly = -70;
    p.lz = 0;
    p.coord = (GsCOORDINATE2 *)gObjectModels[t->model].coord;
    UpdatePlacedPos(&p);
    t->homeX = p.pos.vx;
    t->homeY = p.pos.vy;
    t->homeZ = p.pos.vz;
    if (gFrameCount % 5 == 0) {
        /* MATCHING: x - (k - r), not x - k + r: retail subtracts k from x first. */
        x = (s16)p.pos.vx - (20 - rand() % 40);
        z = (s16)p.pos.vz - (20 - rand() % 40);
        SpawnEffect(1, x, (s16)p.pos.vy, z, (s16)(rand() % 3));
    }
    TumbleTrapAway(t, 70, 0);
}
