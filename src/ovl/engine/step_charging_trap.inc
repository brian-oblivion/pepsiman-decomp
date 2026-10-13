/* MATCHING: an unused 16-byte local sizes retail's frame. */
void StepChargingTrap(StageTrap *t, s16 speed, s32 range) {
    s32 unused[4];

    switch (t->state) {
        case 0:
            t->homeX = t->x;
            t->homeY = t->y;
            t->homeZ = t->z;
            t->state++;
        case 1:
            StepTrapForward(t, speed);
            if (range * range < DistanceSquared((VECTOR *)&t->homeX, (VECTOR *)t)) {
                t->timer = 0;
                t->state++;
                t->homeRotY = t->rotY;
                PlaySe(62);
            }
            break;
        case 2:
            SnapToFloor(t);
            if (++t->timer < 5) {
                t->y -= 20;
                PolarToXZ(-(s16)(speed / 3), t->rotY * 360 / 4096, (Vec3 *)gOffsetVec);
                t->x += gOffsetVec[0] >> 12;
                t->z += gOffsetVec[2] >> 12;
            } else {
                t->timer = 0;
                t->state++;
                PlaySe(62);
            }
            break;
        case 3:
            SnapToFloor(t);
            t->y -= 20;
            if (gFrameCount % 5 == 0) {
                SpawnEffect(1, (s16)(t->x + (rand() % 200 - 100)), (s16)t->y,
                            (s16)(t->z + (rand() % 200 - 100)), (s16)(rand() % 3));
            }
            break;
    }
    if (gSection - UpdatePathSegment(t) < 2) {
        UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
    }
    if (t->touched != 0 && gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
        PlaySe(55);
    }
}
