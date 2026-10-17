void OpenGateOnApproach(StageTrap *t, s32 speed, s8 arg) {
    StageTrap c;
    s32 v;

    c = *t;
    c.model++;
    switch (t->state) {
        case 0:
            if (speed * speed > DistanceSquared((VECTOR *)&gPlayer.x, (VECTOR *)t)) {
                t->timer = 0;
                t->state++;
                t->homeRotY = t->rotY;
            }
            if (gSection - UpdatePathSegment(t) < 2) {
                UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
                UpdateObjectEntry(&gObjectModels[c.model], (Trap *)&c);
            }
            if ((t->touched != 0 || c.touched != 0) && gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
                PlaySe(SE_TRAP_HIT);
            }
            break;
        case 1:
            t->timer++;
            v = Lerp(0, 0x800, t->timer, 40);
            switch (arg) {
                case 2:
                    c.rotY = t->homeRotY - v;
                    break;
                case 1:
                    c.rotY = t->homeRotY + v;
                    break;
            }
            if (gSection - UpdatePathSegment(t) < 2) {
                UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
                UpdateObjectEntry(&gObjectModels[c.model], (Trap *)&c);
            }
            if (t->timer == 40) {
                t->timer = 0;
                t->state++;
            }
            if ((t->touched != 0 || c.touched != 0) && gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
                PlaySe(SE_TRAP_HIT);
            }
            break;
        case 2:
            if (gSection - UpdatePathSegment(t) < 2) {
                UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
                c.rotY += 0x800;
                UpdateObjectEntry(&gObjectModels[c.model], (Trap *)&c);
            }
            if ((t->touched != 0 || c.touched != 0) && gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
                PlaySe(SE_TRAP_HIT);
            }
            break;
    }
}
