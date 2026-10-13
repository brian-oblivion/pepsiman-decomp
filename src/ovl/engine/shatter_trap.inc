void ShatterTrap(StageTrap *t) {
    u8 pad[0x28];
    StageTrap c;
    s16 i;

    switch (t->state) {
        case 0:
            SnapToFloor(t);
            t->timer++;
            if (gSection - UpdatePathSegment(t) < 2) {
                UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
            }
            if (t->touched == 1) {
                if (gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
                    PlaySe(0x38);
                }
                t->homeY = sFloorHit[0];
                t->timer = 0;
                t->model++;
                t->state++;
                UnusedHook1(t);
            }
            break;
        case 1:
            if (t->timer % 3 != 0) {
                for (i = 0; i < 6; i++) {
                    c = *t;
                    InterpSwing(sBurstHeights[i], t->timer, 30, gOffsetVec);
                    c.y -= gOffsetVec[1] >> 12;
                    /* MATCHING: a shift, not * 4: it loads the timer before the table. */
                    c.x += sBurstDirs[i][0] * (t->timer << 2);
                    c.z += sBurstDirs[i][1] * (t->timer << 2);
                    c.rotZ += (t->timer * (sBurstSpins[i][0] * 2 + 10) << 12) / 360;
                    DrawObjectModel(&gObjectModels[t->model], &c);
                }
            }
            if (++t->timer == 30) {
                t->state = -1;
            }
            break;
    }
}
