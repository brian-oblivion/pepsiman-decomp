void TumbleTrapAway(StageTrap *t, s32 height, s8 spawn) {
    s16 n;
    s32 d;

    switch (t->state) {
        case 0:
            t->homeY = t->y;
            t->y -= height;
            if (t->touched != 0) {
                if (gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
                    PlaySe(SE_TRAP_HIT);
                }
                t->homeX = 0;
                t->homeY = t->y;
                t->homeRotY = gPlayer.yaw;
                t->homeRotZ = 0;
                t->timer = 0;
                t->state++;
            }
            if (gSection - UpdatePathSegment(t) < 2) {
                UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
            }
            if (t->touched != 0 && gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
                PlaySe(0x36);
            }
            t->y = t->homeY;
            break;
        case 1:
            PolarToXZ(40, t->homeRotY * 360 / 4096, (Vec3 *)gOffsetVec);
            t->x -= gOffsetVec[0] >> 12;
            t->z -= gOffsetVec[2] >> 12;
            t->timer += 10;
            t->rotX -= 113;
            d = rsin((t->timer << 12) / 360) * 150 >> 12;
            t->y = t->homeY - (d >= 0 ? d : -d);
            if (++t->homeX == 30) {
                t->state++;
            }
            if (gFrameCount % 3 != 0) {
                DrawObjectModel(&gObjectModels[t->model], t);
                if ((u16)(t->timer % 360) == 180 || (u16)(t->timer % 360) == 0) {
                    PlaySe(0x36);
                }
            }
            if (spawn != 0 && t->homeRotZ < 5 && (t->timer == 180 || t->timer == 270)) {
                /* MATCHING: the slot read as the word it returns, and the copy's
                 * target through a cast, order the slot's address as retail. */
                n = ((s32 (*)(void))FindFreeSlot48)();
                *(StageTrap *)&sSpawnedTraps[n] = *t;
                sSpawnedTraps[n].rotY = sSpawnedTraps[n].homeRotY = gCourse.rot.vy;
                sSpawnedTraps[n].rotZ = 0;
                sSpawnedTraps[n].rotX = 0;
                sSpawnedTraps[n].x -= 25 - rand() % 50;
                sSpawnedTraps[n].z -= 25 - rand() % 50;
                sSpawnedTraps[n].state = 0;
                SnapToFloor(&sSpawnedTraps[n]);
                sSpawnedTraps[n].model = rand() % 3 + 17;
                sSpawnedTraps[n].type = 0;
                sSpawnedTraps[n].act = 1;
                t->homeRotZ++;
            }
            break;
    }
}
