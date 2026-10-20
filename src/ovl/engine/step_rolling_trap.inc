void StepRollingTrap(StageTrap *t, u16 speed) {
    SnapToFloor(t);
    PolarToXZ(speed, ANGLE_TO_DEG(t->rotY), (Vec3 *)gOffsetVec);
    t->x += gOffsetVec[0] >> 12;
    t->z += gOffsetVec[2] >> 12;
    if (LatchProximity(t, 500) != 0) {
        PlaySe(SE_BIKE_ENGINE);
    }
    if (gSection - UpdatePathSegment(t) < 2) {
        UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
    }
    if (t->touched != 0 && gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
        PlaySe(SE_TRAP_HIT);
    }
}
