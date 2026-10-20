void ToggleManholeWorkerOnApproach(StageTrap *t, s32 speed, u8 arg) {
    switch (t->state) {
        case 0:
            if (speed * speed > DistanceSquared((VECTOR *)&gPlayer.x, (VECTOR *)t)) {
                switch (arg) {
                    case 0:
                        t->homeRotX = 0;
                        t->model++;
                        break;
                    case 1:
                        t->homeRotX = 1;
                        t->model--;
                        break;
                }
                PlaySe(0x3B);
                t->state++;
            }
            if (gSection - UpdatePathSegment(t) < 2) {
                DrawObjectModel(&gObjectModels[t->model], t);
            }
            break;
        case 1:
            if (gSection - UpdatePathSegment(t) < 2) {
                if (t->homeRotX == 0) {
                    UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
                } else {
                    DrawObjectModel(&gObjectModels[t->model], t);
                }
            }
            break;
        default:
            return;
    }
    if (t->touched != 0 && gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
        PlaySe(0x36);
    }
}
