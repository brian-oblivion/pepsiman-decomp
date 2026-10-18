void StepFlungPiece(StageTrap *t) {
    s32 d;

    switch (t->state) {
        case 0:
            t->x = gPlayer.x;
            t->homeY = gPlayer.groundY - 50;
            t->z = gPlayer.z;
            t->rotY = gCourse.rot.vy + ANGLE_DEG(180);
            t->homeRotX = 0;
            t->timer = 0;
            t->state++;
        case 1:
            PolarToXZ(40, ANGLE_TO_DEG(t->rotY), (Vec3 *)gOffsetVec);
            t->x += gOffsetVec[0] >> 12;
            t->z += gOffsetVec[2] >> 12;
            t->y = t->homeY;
            t->timer = (t->timer + 10) % 360;
            d = rsin((t->timer << 12) / 360) * 200 >> 12;
            t->y = t->homeY - (d >= 0 ? d : -d);
            t->rotX -= 113;
            t->rotZ += 56;
            t->homeRotX++;
            gModeObjectModels[t->model].coord.super = NULL;
            if (gFrameCount & 3) {
                DrawObjectModel((ObjectModel *)&gModeObjectModels[t->model], t);
            }
            if (t->homeRotX > 60) {
                t->model = -1;
                t->state++;
                gRideSwitchState[0] = -1;
            }
            break;
    }
}
