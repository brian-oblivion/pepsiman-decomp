void ShiftTrapOnApproach(StageTrap *t, s8 dir, s32 speed) {
    s32 step;

    switch (dir) {
        case 0:
            step = 10;
            break;
        case 1:
            step = -10;
            break;
    }
    switch (t->state) {
        case 0:
            t->homeX = t->x;
            t->homeY = t->y;
            t->homeZ = t->z;
            t->state++;
        case 1:
            if (gSection - UpdatePathSegment(t) < 2) {
                UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
            }
            if (speed * speed > DistanceSquared((VECTOR *)&gPlayer.x, (VECTOR *)t)) {
                t->timer = 0;
                t->state++;
            }
            if (t->touched != 0 && gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
                PlaySe(0x37);
            }
            break;
        case 2:
            if (gSection - UpdatePathSegment(t) < 2) {
                UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
            }
            t->y += step;
            if (DistanceSquared((VECTOR *)&t->homeX, (VECTOR *)t) > 22500) {
                t->timer = 0;
                t->state++;
            }
            if (t->touched != 0 && gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
                PlaySe(0x37);
            }
            break;
        case 3:
            if (gSection - UpdatePathSegment(t) < 2) {
                UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
            }
            if (t->touched != 0 && gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
                PlaySe(0x37);
            }
            break;
    }
}
