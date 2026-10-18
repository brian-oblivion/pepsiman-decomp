void RunTrapThenTurn(StageTrap *t, u16 speed, s32 range, s8 dir) {
    switch (t->state) {
        case 0:
            t->homeX = t->x;
            t->homeY = t->y;
            t->homeZ = t->z;
            t->state++;
        case 1:
            StepTrapForward(t, speed);
            if (range * range < DistanceSquared((VECTOR *)&t->homeX, (VECTOR *)t)) {
                t->timer = 0;
                t->state++;
                t->homeRotY = t->rotY;
                if (speed > 30) {
                    PlaySe(88);
                }
            }
            if (LatchProximity(t, 2000) != 0) {
                PlaySe(82);
            }
            break;
        case 2:
            switch (dir) {
                case 2:
                    InterpRange(t->homeRotY, t->homeRotY - ANGLE_DEG(90), t->timer, 30, gOffsetVec);
                    t->rotY = gOffsetVec[1];
                    break;
                case 1:
                    InterpRange(t->homeRotY, t->homeRotY + ANGLE_DEG(90), t->timer, 30, gOffsetVec);
                    t->rotY = gOffsetVec[1];
                    break;
            }
            StepTrapForward(t, speed);
            if (++t->timer > 30) {
                t->timer = 0;
                t->state++;
            }
            break;
        case 3:
            StepTrapForwardLevel(t, speed);
            break;
    }
}
