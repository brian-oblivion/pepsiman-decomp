void ToppleCactusOnHit(StageTrap *t) {
    VECTOR v;

    switch (t->state) {
        case 0:
            if (gSection - UpdatePathSegment(t) < 2) {
                UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
            }
            if (t->touched == 1) {
                t->timer = 0;
                t->state++;
                if (gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
                    PlaySe(0x36);
                }
                if (gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
                    PlaySe(0x7D);
                }
            }
            break;
        case 1:
            t->timer++;
            t->rotX = Lerp(0, -ANGLE_DEG(90), t->timer, 5);
            if (t->timer == 5) {
                t->timer = 0;
                t->state++;
            }
            if (gSection - UpdatePathSegment(t) < 2) {
                DrawObjectModel(&gObjectModels[t->model], t);
            }
            break;
        case 2:
            t->timer++;
            if ((u16)(t->timer % 3) == 0) {
                v.vx = t->x + (rsin((t->rotY + ANGLE_DEG(180)) * (10 * (rand() % 10))) >> 12);
                v.vz = t->z + (rcos((t->rotY + ANGLE_DEG(180)) * (10 * (rand() % 10))) >> 12);
                v.vy = t->y;
                ScatterDust(&v);
            }
            if (t->timer == 20) {
                t->state++;
            }
        case 3:
            if (gSection - UpdatePathSegment(t) < 2) {
                DrawObjectModel(&gObjectModels[t->model], t);
            }
            break;
    }
}
