void SpinTrapAway(StageTrap *t) {
    s32 d;

    switch (t->state) {
        case 0:
            UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
            if (t->touched == 1) {
                t->homeX = 0;
                t->homeY = t->y;
                /* MATCHING: read as a member at a fixed address, which keeps
                 * the load below the stores into t. */
                t->homeRotY = ((SVECTOR *)gPlayerYaw)->vx;
                t->state++;
                t->timer = 0;
            }
            break;
        case 1:
            PolarToXZ(40, ANGLE_TO_DEG(t->homeRotY), (Vec3 *)gOffsetVec);
            t->x -= gOffsetVec[0] >> 12;
            t->z -= gOffsetVec[2] >> 12;
            t->timer = (t->timer + 10) % 360;
            t->rotX += 113;
            t->rotZ += 113;
            d = rsin((t->timer << 12) / 360) * 200 >> 12;
            t->y = t->homeY - (d >= 0 ? d : -d);
            if (++t->homeX == 20) {
                t->state++;
            }
            if ((gFrameCount & 1) && gSection - UpdatePathSegment(t) < 2) {
                DrawObjectModel(&gObjectModels[t->model], t);
            }
            break;
    }
}
