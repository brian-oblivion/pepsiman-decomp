void StepTrapType1F(StageTrap *t, u16 arg) {
    s32 d;

    switch (t->state) {
        case 0:
            t->homeRotZ = (rand() % 3 + 1) * 30;
            t->timer = rand() % 360;
            t->homeX = t->y;
            t->state++;
        case 1:
            t->y = t->homeX - 30;
            d = rsin((t->timer * 20 << 12) / 360) * t->homeRotZ >> 12;
            t->homeY = -(d >= 0 ? d : -d);
            if (t->homeY > 0) {
                t->y = t->homeY;
            }
            PolarToXZ(arg, t->rotY * 360 / 4096, (Vec3 *)gOffsetVec);
            t->rotX += (arg << 12) / 360;
            t->x += gOffsetVec[0] >> 12;
            t->z += gOffsetVec[2] >> 12;
            t->timer++;
            if (gSection - UpdatePathSegment(t) < 2) {
                UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
            }
            if (t->touched != 0 && gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
                PlaySe(0x3F);
            }
            break;
    }
}
