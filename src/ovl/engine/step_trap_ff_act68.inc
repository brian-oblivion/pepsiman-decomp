void StepTrapFFAct68(StageTrap *t) {
    StageTrap c;
    u16 i;
    s16 model;
    s32 h;
    u16 yaw;

    model = 12;
    switch (t->state) {
        case 0:
            PolarToXZ(t->homeRotZ, ANGLE_TO_DEG(t->homeRotY), (Vec3 *)gOffsetVec);
            t->x += gOffsetVec[0] >> 12;
            t->z += gOffsetVec[2] >> 12;
            t->timer += 10;
            h = rsin((t->timer / t->homeX << 12) / 360) * t->homeRotX >> 12;
            /* MATCHING: h >= 0 ? h : -h folds to one abs insn, which the
             * scheduler moves loads across; an if gives a branch. */
            t->y = t->homeY - (h >= 0 ? h : -h);
            if (t->timer > 180) {
                t->homeX = 3;
                t->homeRotX = 10;
                t->homeRotZ = -5;
            }
            if (t->timer > 360) {
                t->homeX = 8;
                t->homeRotX = 2;
                t->homeRotZ = -2;
            }
            if (t->timer > 540) {
                t->homeRotX = 0;
                t->homeRotZ = 0;
            }
            if (t->timer == 180) {
                PlaySe(0x38);
            }
            if (t->aux != 3) {
                t->homeZ -= ++t->counter;
                yaw = gCourse.rot.vy;
                if (t->homeZ < 0) {
                    t->homeZ = 0;
                }
                if (t->aux == 2) {
                    yaw += (t->homeZ << 12) / 360;
                }
                if (t->aux == 1) {
                    yaw += -(t->homeZ << 12) / 360;
                }
                t->rotY = yaw;
            }
            if (gSection - UpdatePathSegment(t) < 2) {
                UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
            }
            if (t->touched == 1) {
                PlaySe(0x38);
                t->timer = 0;
                t->state++;
            }
            break;
        case 1:
            t->timer++;
            for (i = 0; i < 8; i++) {
                c = *t;
                c.model = model;
                c.rotY = gCourse.rot.vy;
                InterpSwing(sBurstHeights[i], t->timer, 30, gOffsetVec);
                c.y -= gOffsetVec[1] >> 12;
                c.x += sBurstDirs[i][0] * (t->timer * 4 + t->timer);
                c.z += sBurstDirs[i][1] * (t->timer * 4 + t->timer);
                c.rotZ += (t->timer * (sBurstSpins[i][0] * 2 + 10) << 12) / 360;
                if (t->timer % 3) {
                    DrawObjectModel(&gObjectModels[c.model], &c);
                }
            }
            if (t->timer > 40) {
                t->model = -1;
                t->state++;
            }
            break;
    }
}
