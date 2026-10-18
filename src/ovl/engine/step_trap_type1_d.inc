void StepTrapType1D(StageTrap *t, u16 speed) {
    VECTOR v;
    StageTrap *base;
    u16 phase;

    base = t;
    switch (t->state) {
        case 0:
            SnapToFloor(t);
            t->homeY = t->y;
            t->state++;
        case 1:
            base->y = base->homeY;
            base->y -= (u16)(t->timer % 10) * 3;
            PolarToXZ(speed, ANGLE_TO_DEG(base->rotY), (Vec3 *)gOffsetVec);
            base->x += gOffsetVec[0] >> 12;
            base->z += gOffsetVec[2] >> 12;
            t->timer++;
            phase = t->timer % 20;
            t++;
            if (phase < 15) {
                t->rotX = Lerp(ANGLE_DEG(60), -ANGLE_DEG(60), phase, 15);
            } else {
                t->rotX = Lerp(-ANGLE_DEG(60), ANGLE_DEG(60), phase - 15, 5);
            }
            t->y = base->y - 100;
            t->rotY = base->rotY;
            t->x += gOffsetVec[0] >> 12;
            t->z += gOffsetVec[2] >> 12;
            t->model = base->model + 1;
            t++;
            if (phase < 15) {
                t->rotX = Lerp(-ANGLE_DEG(60), ANGLE_DEG(90), phase, 15);
            } else {
                t->rotX = Lerp(ANGLE_DEG(90), -ANGLE_DEG(60), phase - 15, 5);
            }
            t->y = base->y - 100;
            t->rotY = base->rotY;
            t->x += gOffsetVec[0] >> 12;
            t->z += gOffsetVec[2] >> 12;
            t->model = base->model + 2;
            t = base;
            UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
            t++;
            DrawObjectModel(&gObjectModels[t->model], t);
            t->model = -1;
            t++;
            DrawObjectModel(&gObjectModels[t->model], t);
            t->model = -1;
            if (base->touched != 0 && gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
                PlaySe(0x1C);
            }
            v.vx = t->x - (25 - rand() % 50);
            v.vy = t->y + 70;
            v.vz = t->z - (25 - rand() % 50);
            ScatterDust(&v);
            if (LatchProximity(t, 1000) != 0) {
                PlaySe(0x79);
            }
            break;
    }
}
