void RunTrainAhead(StageTrap *t, u16 speed) {
    switch (t->state) {
        case 0:
            t->timer = 0;
            t->state++;
            PlaySe(SE_TRAIN_START);
        case 1:
            if (LatchProximity(t, 2000) != 0) {
                PlaySe(SE_TRAIN_APPROACH);
            }
            PolarToXZ(speed, ANGLE_TO_DEG(t->rotY), (Vec3 *)gOffsetVec);
            t->x += gOffsetVec[0] >> 12;
            t->z += gOffsetVec[2] >> 12;
            UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
            if (t->touched != 0 && gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
                PlaySe(SE_OBSTACLE_HIT);
            }
            break;
    }
}
