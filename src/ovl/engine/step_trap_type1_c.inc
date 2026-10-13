void StepTrapType1C(StageTrap *t, s32 speed, s32 arg, s32 arg2) {
    switch (t->state) {
        case 0:
            if (speed * speed > DistanceSquared((VECTOR *)&gPlayer.x, (VECTOR *)t)) {
                t->timer = 0;
                t->state++;
                t->homeRotY = t->rotY;
            }
            DrawObjectModel(&gObjectModels[t->model], t);
            break;
        case 1:
            t->timer++;
            switch ((s8)arg) {
                case 2:
                    t->rotZ = -Lerp(0, 0x400, t->timer, 10);
                    break;
                case 1:
                    t->rotZ = Lerp(0, 0x400, t->timer, 10);
                    break;
            }
            if (t->timer == 10) {
                t->timer = 0;
                t->state++;
                PlaySe(0x3D);
            }
            UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
            break;
        case 2:
            t->timer++;
            t->y -= 10;
            if (t->timer == 2) {
                t->timer = 0;
                t->state++;
            }
            UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
            break;
        case 3:
            t->timer++;
            t->y += 10;
            if (t->timer == 4) {
                t->timer = 0;
                t->state++;
                PlaySe(0x3D);
            }
        case 4:
            if (gSection - UpdatePathSegment(t) < 2) {
                UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
            }
            break;
    }
}
