void StepSprayTrap(StageTrap *t, s8 dir) {
    PlacedPos p;
    StageTrap c;
    s32 d;

    if (gSection - UpdatePathSegment(t) < 2) {
        UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
        if (t->touched != 0 && gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
            PlaySe(0x37);
        }
        if (dir != 0) {
            switch (t->state) {
                case 0:
                    switch (dir) {
                        case 2:
                            p.lx = -140;
                            break;
                        case 1:
                            p.lx = 140;
                            break;
                        case 3:
                            p.lx = 0;
                            break;
                    }
                    p.ly = 50;
                    p.lz = 0;
                    p.coord = (GsCOORDINATE2 *)gObjectModels[t->model].coord;
                    UpdatePlacedPos(&p);
                    t->homeX = p.pos.vx;
                    t->homeY = p.pos.vy;
                    t->homeZ = p.pos.vz;
                    t->state++;
                case 1:
                    c = *t;
                    SnapToFloor(&c);
                    if (!(gFrameCount & 3)) {
                        SpawnEffectWithParams(9, t->homeX, t->homeY, t->homeZ, 0, 20,
                                              (d = c.y - t->homeY, d >= 0 ? d : -d), 0, 0);
                    }
                    if (gFrameCount % 10 == 0) {
                        PlaySe(0x6A);
                    }
                    if (!(gFrameCount & 1)) {
                        SpawnEffect(4, (s16)t->homeX - (50 - rand() % 100),
                                    (s16)c.y + (20 - rand() % 50),
                                    (s16)t->homeZ - (50 - rand() % 100), 0);
                    }
                    break;
            }
        }
    }
}
