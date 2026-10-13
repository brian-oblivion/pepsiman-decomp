void RunTrapAlongPath(StageTrap *t, u16 speed, s8 turn) {
    if (LatchProximity(t, 2000) != 0) {
        PlaySe(82);
    }
    t->state++;
    SnapToFloor(t);
    if (t->timer < 150 && speed != 0) {
        t->timer++;
        t->y -= 20;
        t->rotY = GetPathDirection(t) + (s8)(turn % 2) * 0x800;
        PolarToXZ(speed, t->rotY * 360 / 4096, (Vec3 *)gOffsetVec);
        t->x += gOffsetVec[0] >> 12;
        t->z += gOffsetVec[2] >> 12;
    }
    if (gSection - UpdatePathSegment(t) < 2) {
        UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
    }
    if (t->touched != 0 && gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
        PlaySe(55);
    }
}
