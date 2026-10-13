void StepRollingTrap(StageTrap *t, u16 speed) {
    SnapToFloor(t);
    PolarToXZ(speed, t->rotY * 360 / 4096, (Vec3 *)gOffsetVec);
    t->x += gOffsetVec[0] >> 12;
    t->z += gOffsetVec[2] >> 12;
    if (LatchProximity(t, 500) != 0) {
        PlaySe(120);
    }
    if (gSection - UpdatePathSegment(t) < 2) {
        UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
    }
    if (t->touched != 0 && gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
        PlaySe(55);
    }
}
