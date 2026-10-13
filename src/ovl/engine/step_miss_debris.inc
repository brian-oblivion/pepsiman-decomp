void StepMissDebris(StageTrap *t) {
    s32 d;

    if (t->state == 0) {
        PolarToXZ(t->homeRotZ, t->rotY * 360 / 4096, (Vec3 *)gOffsetVec);
        t->x += gOffsetVec[0] >> 12;
        t->z += gOffsetVec[2] >> 12;
        t->y = t->homeY;
        t->timer = (t->timer + t->homeZ) % 360;
        d = rsin((t->timer << 12) / 360) * t->homeX >> 12;
        t->homeRotX++;
        t->y = t->homeY - (d >= 0 ? d : -d);
        t->rotX -= (t->homeRotZ << 12) / 360;
        t->rotZ += (t->homeZ << 12) / 360;
        gModeObjectModels[t->model].coord.super = NULL;
        if (gFrameCount & 3) {
            DrawObjectModel((ObjectModel *)&gModeObjectModels[t->model], t);
        }
        if (t->homeRotX > 60) {
            t->model = -1;
            t->state++;
        }
    }
}
