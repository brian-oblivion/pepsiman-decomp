void SwingGateContinuously(StageTrap *t, s8 arg) {
    StageTrap c;

    c = *t;
    c.model++;
    switch (t->state) {
        case 0:
            t->homeX = 0;
            t->homeRotY = t->rotY;
            t->state++;
        case 1:
            t->timer++;
            switch (t->homeX) {
                case 0:
                    c.rotY = t->homeRotY - Lerp(0, 0x800, t->timer, 40);
                    break;
                case 1:
                    c.rotY = t->homeRotY - Lerp(0x800, 0, t->timer, 40);
                    break;
            }
            if (gSection - UpdatePathSegment(t) < 2) {
                UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
                UpdateObjectEntry(&gObjectModels[c.model], (Trap *)&c);
            }
            if (t->timer == 40) {
                t->timer = 0;
                t->homeX ^= 1;
                if (arg == 1) {
                    t->homeZ = 0;
                    t->state++;
                }
            }
            if ((t->touched != 0 || c.touched != 0) && gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
                PlaySe(SE_TRAP_HIT);
            }
            break;
        case 2:
            t->timer++;
            switch (t->homeX) {
                case 0:
                    c.rotY = 0;
                    break;
                case 1:
                    c.rotY = 0x800;
                    break;
            }
            if (gSection - UpdatePathSegment(t) < 2) {
                UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
                UpdateObjectEntry(&gObjectModels[c.model], (Trap *)&c);
            }
            if ((t->touched != 0 || c.touched != 0) && gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
                PlaySe(SE_TRAP_HIT);
            }
            if (t->timer == 20) {
                t->timer = 0;
                t->state--;
            }
            break;
    }
}
