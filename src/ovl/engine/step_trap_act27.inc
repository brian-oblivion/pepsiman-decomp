void StepTrapAct27(StageTrap *t) {
    StageTrap c;
    u16 i;
    s16 model;

    /* MATCHING: the model in a local keeps it in a saved register. */
    model = 0x44;
    switch (t->state) {
        case 0:
            if (gSection - UpdatePathSegment(t) < 2) {
                UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
            }
            if (t->touched != 0) {
                if (gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
                    PlaySe(0x39);
                }
                t->timer = 0;
                t->state++;
            }
            break;
        case 1:
            t->timer++;
            PolarToXZ(50, (gCourse.rot.vy + 0x800) * 360 / 4096, (Vec3 *)gOffsetVec);
            t->x += gOffsetVec[0] >> 12;
            t->z += gOffsetVec[2] >> 12;
            for (i = 0; i < 8; i++) {
                c = *t;
                c.model = model;
                c.rotY = gCourse.rot.vy;
                InterpSwing(sBurstHeights[i], t->timer, 30, gOffsetVec);
                c.y -= gOffsetVec[1] >> 12;
                /* MATCHING: timer * 4 + timer, not * 5: it loads the table first. */
                c.x += sBurstDirs[i][0] * (t->timer * 4 + t->timer);
                c.z += sBurstDirs[i][1] * (t->timer * 4 + t->timer);
                c.rotZ += (t->timer * (sBurstSpins[i][0] * 2 + 10) << 12) / 360;
                if (t->timer & 3) {
                    DrawObjectModel(&gObjectModels[c.model], &c);
                }
            }
            if (t->timer > 40) {
                t->model = -1;
                t->state++;
            }
            break;
    }
}
