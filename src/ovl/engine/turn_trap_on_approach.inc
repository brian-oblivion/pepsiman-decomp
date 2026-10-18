void TurnTrapOnApproach(StageTrap *t, s32 speed, s8 arg) {
    s32 v;
    s32 w;

    switch (t->state) {
        case 0:
            if (speed * speed > DistanceSquared((VECTOR *)&gPlayer.x, (VECTOR *)t)) {
                t->timer = 0;
                t->state++;
                t->homeRotY = t->rotY;
            }
            if (gSection - UpdatePathSegment(t) < 2) {
                DrawObjectModel(&gObjectModels[t->model], t);
            }
            break;
        case 1:
            t->timer++;
            v = Lerp(0, ANGLE_DEG(45), t->timer, 10);
            switch (arg) {
                case 2:
                    t->rotY = t->homeRotY - v;
                    break;
                case 1:
                    t->rotY = t->homeRotY + v;
                    break;
            }
            if (gSection - UpdatePathSegment(t) < 2) {
                UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
            }
            if (t->timer == 10) {
                t->timer = 0;
                t->state++;
                t->model++;
                switch (arg) {
                    case 2:
                        t->homeRotY -= ANGLE_DEG(45);
                        break;
                    case 1:
                        t->homeRotY += ANGLE_DEG(45);
                        break;
                }
            }
            if (t->touched != 0 && gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
                PlaySe(SE_TRAP_HIT);
            }
            break;
        case 2:
            t->timer++;
            w = Lerp(0, ANGLE_DEG(45), t->timer, 10);
            switch (arg) {
                case 2:
                    t->rotY = t->homeRotY - w;
                    break;
                case 1:
                    t->rotY = t->homeRotY + w;
                    break;
            }
            if (t->timer == 10) {
                t->timer = 0;
                t->state++;
            }
            if (gSection - UpdatePathSegment(t) < 2) {
                UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
            }
            if (t->touched != 0 && gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
                PlaySe(SE_TRAP_HIT);
            }
            break;
        case 3:
            if (gSection - UpdatePathSegment(t) < 2) {
                UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
            }
            if (t->touched != 0 && gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
                PlaySe(SE_TRAP_HIT);
            }
            break;
    }
}
