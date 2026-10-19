void StepTrapType12(StageTrap *t, u16 speed, s16 arg) {
    switch (t->state) {
        case 0:
            /* MATCHING: retail stores the slot unextended, as if from an s32. */
            t->aux = ((s32 (*)(void))FindFreeSlot48)();
            sSpawnedTraps[t->aux] = *t;
            sSpawnedTraps[t->aux].model = t->model + 1;
            sSpawnedTraps[t->aux].state = 4;
            sSpawnedTraps[t->aux].y = t->y -= 150;
            t->timer = 0;
            t->state++;
            PlaySe(0x53);
        case 1:
            if (LatchProximity(t, 2000) != 0) {
                PlaySe(SE_TRUCK_HORN);
            }
            PolarToXZ(speed, ANGLE_TO_DEG(t->rotY), (Vec3 *)gOffsetVec);
            sSpawnedTraps[t->aux].x = t->x += gOffsetVec[0] >> 12;
            sSpawnedTraps[t->aux].z = t->z += gOffsetVec[2] >> 12;
            t->timer++;
            if (t->timer > arg) {
                t->timer = 0;
                t->state++;
                t->homeRotY = t->rotY;
                PlaySe(SE_TRAP_SKID);
            }
            break;
        case 2:
            PolarToXZ(Lerp(45, 0, t->timer, 40), ANGLE_TO_DEG(t->homeRotY + 227), (Vec3 *)gOffsetVec);
            sSpawnedTraps[t->aux].x = t->x += (gOffsetVec[0] >> 12) - (2 - rand() % 5);
            sSpawnedTraps[t->aux].z = t->z += (gOffsetVec[2] >> 12) - (2 - rand() % 5);
            t->rotY -= 11;
            sSpawnedTraps[t->aux].rotY += 5;
            SpawnEffect(4, (s16)t->x - (100 - rand() % 200), (s16)t->y + 100,
                        (s16)t->z - (100 - rand() % 200), 0);
            if (++t->timer == 40) {
                t->timer = 0;
                t->state++;
            }
            break;
    }
    if (gSection - UpdatePathSegment(t) < 2) {
        UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
    }
    if (t->touched != 0 && gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
        PlaySe(SE_TRAP_HIT);
    }
}
