void SlideTrapAway(StageTrap *t, s32 arg) {
    switch (t->state) {
        case 0:
            if (gSection - UpdatePathSegment(t) < 2) {
                UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
            }
            if (t->touched != 0) {
                t->homeRotY = gPlayer.yaw;
                t->timer = 0;
                t->state++;
                if (gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
                    PlaySe(SE_KNOCK_AWAY);
                }
            }
            break;
        case 1:
            PolarToXZ(40, ANGLE_TO_DEG(t->homeRotY), (Vec3 *)gOffsetVec);
            t->x -= gOffsetVec[0] >> 12;
            t->z -= gOffsetVec[2] >> 12;
            t->rotZ += 11;
            if (++t->timer == 15) {
                t->state = -1;
                t->timer = 0;
            }
            if (!(gFrameCount & 1) && t->state != -1) {
                DrawObjectModel(&gObjectModels[t->model], t);
            }
            break;
    }
}
