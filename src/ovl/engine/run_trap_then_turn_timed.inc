void RunTrapThenTurnTimed(StageTrap *t, u16 speed, s32 arg, s8 dir) {
    switch (t->state) {
        case 0:
            t->homeRotZ = 0;
            t->homeX = t->x;
            t->homeY = t->y;
            t->homeZ = t->z;
            t->state++;
            PlaySe(0x11);
        case 1:
            StepTrapForward(t, speed);
            DrawTrapWarning(t);
            if (gSection - UpdatePathSegment(t) < 2) {
                if (++t->homeRotZ > arg) {
                    t->timer = 0;
                    t->state++;
                    t->homeRotY = t->rotY;
                }
            }
            break;
        case 2:
            switch (dir) {
                case 2:
                    InterpRange(t->homeRotY, t->homeRotY - 0x400, t->timer, 30, gOffsetVec);
                    t->rotY = gOffsetVec[1];
                    break;
                case 1:
                    InterpRange(t->homeRotY, t->homeRotY + 0x400, t->timer, 30, gOffsetVec);
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
