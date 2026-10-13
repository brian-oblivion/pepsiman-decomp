void StepDashBoard(StageTrap *t) {
    switch (t->state) {
        case 0:
            if (t->touched != 0) {
                gPlayer.action = ACT_DASH_START;
                t->timer = 0;
                t->state++;
                if (gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
                    PlaySe(0x71);
                }
            }
            if (gSection - UpdatePathSegment(t) < 2) {
                UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
            }
            break;
        case 1:
            t->timer++;
            t->rotX = Lerp(0, -0x4E3, t->timer, 3);
            if (t->timer == 3) {
                t->homeRotY = (t->rotY - 0x800) * 360 / 4096;
                t->timer = 0;
                t->state++;
            }
        case 2:
            if (gSection - UpdatePathSegment(t) < 2) {
                DrawObjectModel(&gObjectModels[t->model], t);
            }
            break;
    }
}
