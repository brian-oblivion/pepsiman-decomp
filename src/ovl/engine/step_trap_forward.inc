void StepTrapForward(StageTrap *t, u16 speed) {
    if (LatchProximity(t, 2000) != 0) {
        PlaySe(82);
    }
    if (SnapToFloor(t) == -1) {
        t->y = t->homeY;
    }
    t->y -= 20;
    PolarToXZ(speed, ANGLE_TO_DEG(t->rotY), (Vec3 *)gOffsetVec);
    t->x += gOffsetVec[0] >> 12;
    t->z += gOffsetVec[2] >> 12;
    if (gSection - UpdatePathSegment(t) < 2) {
        UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
    }
    if (t->touched != 0 && gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
        PlaySe(SE_TRAP_HIT);
    }
}
