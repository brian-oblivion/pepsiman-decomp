void ToppleTrapOnApproach(StageTrap *t, s32 side, s32 speed, u8 kind) {
    s16 n;

    switch (kind) {
        case 0:
            n = 12;
            break;
        case 1:
            n = 10;
            break;
    }
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
            switch ((u8)side) {
                case 1:
                case 2:
                    t->y += t->timer * 4;
                    break;
                case 3:
                    t->y += t->timer * 12;
                    if (t->y > gFloorY[0] - 30) {
                        t->y = gFloorY[0] - 30;
                    }
                    break;
            }
            switch ((u8)side) {
                case 2:
                    t->rotZ = -Lerp(0, 227, t->timer, n);
                    break;
                case 1:
                    t->rotZ = Lerp(0, 227, t->timer, n);
                    break;
            }
            if (t->timer == n) {
                t->timer = 0;
                t->state++;
                ScatterTrapDust(t, side);
                if (gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
                    PlaySe(SE_TRAP_HIT);
                }
            }
            if (gSection - UpdatePathSegment(t) < 2) {
                UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
            }
            break;
        case 2:
            t->timer++;
            t->y -= 10;
            if (t->timer == 2) {
                t->timer = 0;
                t->state++;
            }
            if (gSection - UpdatePathSegment(t) < 2) {
                UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
            }
            break;
        case 3:
            t->timer++;
            t->y += 10;
            if (t->timer == 4) {
                t->timer = 0;
                t->state++;
                ScatterTrapDust(t, side);
                if (gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
                    PlaySe(SE_TRAP_HIT);
                }
            }
        case 4:
            if (gSection - UpdatePathSegment(t) < 2) {
                UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
            }
            if (t->touched != 0 && gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
                PlaySe(SE_TRAP_HIT);
            }
            break;
    }
}
