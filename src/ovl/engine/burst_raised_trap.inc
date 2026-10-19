void BurstRaisedTrap(StageTrap *t) {
    StageTrap c;
    u16 i;

    switch (t->state) {
        case 0:
            t->homeY = t->y;
            t->y -= 50;
            if (gSection - UpdatePathSegment(t) < 2) {
                UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
            }
            if (t->touched == 1) {
                t->timer = 0;
                t->state++;
                if (gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
                    PlaySe(SE_CRASH);
                }
            }
            t->y = t->homeY;
            break;
        case 1:
            t->timer++;
            if (t->timer % 3) {
                for (i = 0; i < 8; i++) {
                    c = *t;
                    c.model = 0;
                    InterpSwing(sBurstHeights[i], t->timer, 30, gOffsetVec);
                    c.y -= gOffsetVec[1] >> 12;
                    /* MATCHING: a shift, not * 2, keeps the timer load first. */
                    c.x += sBurstDirs[i][0] * (t->timer << 1);
                    c.z += sBurstDirs[i][1] * (t->timer << 1);
                    c.rotZ += (t->timer * (sBurstSpins[i][0] * 2 + 10) << 12) / 360;
                    if (gSection - UpdatePathSegment(t) < 2) {
                        DrawObjectModel(&gObjectModels[c.model], &c);
                    }
                }
            }
            if (t->timer == 30) {
                t->state++;
            }
            break;
    }
}
