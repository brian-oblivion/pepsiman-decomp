void StepTrapType27(StageTrap *t, s32 arg) {
    StageTrap probe;
    s32 d;

    switch (t->state) {
        case 0:
            t->homeX = t->x;
            t->homeY = t->y;
            t->homeZ = t->z;
            t->state++;
        case 1:
            if (arg * arg > DistanceSquared((VECTOR *)&gPlayer.x, (VECTOR *)t)) {
                t->timer = 0;
                t->state++;
            }
            break;
        case 2:
            t->y += t->timer * 5;
            t->timer++;
            probe = *t;
            SnapToFloor(&probe);
            if (t->y > probe.y) {
                t->homeY = t->y;
                t->timer = 0;
                t->state++;
            }
            break;
        case 3:
            d = rsin((t->timer * 20 << 12) / 360) * 50 >> 12;
            t->timer++;
            t->y = t->homeY - (d >= 0 ? d : -d);
            if ((t->timer * 20 % 360 == 180 || t->timer * 20 % 360 == 0) &&
                gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
                PlaySe(0x3B);
            }
            if (t->timer >= 10) {
                t->timer = 0;
                t->state++;
            }
            break;
        case 4:
            t->rotX = -Lerp(0, 0x400, t->timer, 10);
            if (++t->timer >= 11) {
                t->timer = 0;
                t->state++;
            }
            break;
    }
    if (gSection - UpdatePathSegment(t) < 2) {
        if (t->state != 5) {
            UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
        } else {
            DrawObjectModel(&gObjectModels[t->model], t);
        }
    }
    if (t->touched != 0 && gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
        PlaySe(0x3B);
    }
}
