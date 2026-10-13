void RunTrapAheadAtHeight(StageTrap *t, u16 speed) {
    switch (t->state) {
        case 0:
            t->timer = 0;
            t->state++;
            PlaySe(109);
        case 1:
            if (LatchProximity(t, 2000) != 0) {
                PlaySe(110);
            }
            PolarToXZ(speed, t->rotY * 360 / 4096, (Vec3 *)gOffsetVec);
            t->x += gOffsetVec[0] >> 12;
            t->z += gOffsetVec[2] >> 12;
            UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
            if (t->touched != 0 && gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
                PlaySe(57);
            }
            break;
    }
}
