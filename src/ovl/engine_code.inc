/* The stage engine's code that all eight engine overlays share word for
 * word, in sections. Each #include of this file emits the next section, so
 * an overlay includes it once per section, in order, around its own code.
 * Only matched C lives here: a function still in asm or NON_MATCHING in any
 * overlay stays in that overlay's unit. */

#ifndef ENGINE_PART
#define ENGINE_PART 1
#endif

#if ENGINE_PART == 1

void UpdateStageSprites(void) {
    u32 i;
    EngSprite *r;
    s8 k;

    ResetSpriteWalks();
    for (i = 0; i < 100; i++) {
        r = &gStageSprites[i];
        if (r->id != -1 && (k = gSwitches[r->switchNo].state) == SWITCH_ON) {
            if (r->walk != 0 && r->walk == k) {
                StepSpriteWalk(r, i, r->walkArg, 5, 2500, 180);
            }
            if (r->flags & SPRITE_HITTABLE) {
                StepSprite(r->id, r);
                if (r->hit != 0) {
                    PlaySpriteHitSe(r->se);
                    r->driftCount = 0;
                    r->flags = (r->flags & SPRITE_MIRROR) | SPRITE_ACT_DRIFT;
                    r->driftBaseY = r->y;
                }
            } else if ((r->flags & SPRITE_ACT_MASK) != SPRITE_ACT_DRIFT || StepDrift(r) == 1) {
                DrawObjectSprite(r->id, r);
            }
        }
    }
}

void StepSpriteWalk(EngSprite *r, u16 i, s16 arg, u16 speed, s32 range, s32 limit) {
    switch ((s8)sWalkState[i]) {
        case 0:
            StartSpriteWalk(r, i, arg);
            sWalkState[i]++;
        case 1:
            sWalkTimer[i]++;
            if (range * range > DistanceSquared((VECTOR *)&gPlayer.x, (VECTOR *)&r->x)) {
                sWalkState[i]++;
                sWalkTimer[i] = 0;
            }
            break;
        case 2:
            AdvanceSpriteWalk(r, i, speed);
            if (++sWalkTimer[i] > limit) {
                sWalkState[i]++;
                sWalkTimer[i] = 0;
            }
        case 3:
            if (r->hit == 1) {
                r->driftCount = 0;
                r->flags = (r->flags & SPRITE_MIRROR) | SPRITE_ACT_DRIFT;
                r->driftBaseY = r->y;
                sWalkState[i] += 2;
                r->id = WALK_BASE(i) += 4;
            }
            break;
    }
}

void AdvanceSpriteWalk(EngSprite *r, u16 i, u16 speed) {
    if (gFrameCount % 5 == 0) {
        WALK_FRAME(i) = (WALK_FRAME(i) + 1) % 4;
        r->id = WALK_FRAME(i) + WALK_BASE(i);
    }
    r->x += rsin((WALK_ANGLE(i) << 12) / 360) * speed >> 12;
    r->z += rcos((WALK_ANGLE(i) << 12) / 360) * speed >> 12;
}

void UpdateStageTraps(void) {
    u32 i;
    StageTrap *t;

    ResetSpawnedTraps();
    for (i = 0; i < 200; i++) {
        t = &((StageTrap *)gTraps)[i];
        if (t->model != -1 && gSwitches[t->switchNo].state == SWITCH_ON) {
            StepStageTrap(t);
        }
    }
    for (i = 0; i < 200; i++) {
        t = &sSpawnedTraps[i];
        if (t->model != -1 && gSwitches[t->switchNo].state == SWITCH_ON) {
            StepStageTrap(t);
        }
    }
}

void StepStageTrap(StageTrap *t) {
    switch (t->type) {
        case 255:
            RunTrapTypeFF(t);
            break;
        case 0:
            RunTrapType00(t);
            break;
        case 1:
            RunTrapType01(t, 0);
            break;
        case 2:
            RunTrapType02(t);
            break;
        case 3:
            RunTrapType01(t, 1);
            break;
        case 4:
            RunTrapType04(t);
            break;
        case 5:
            RunTrapType05(t, 2);
            break;
        case 6:
            RunTrapType05(t, 1);
            break;
        case 7:
            RunTrapType07(t, 3000, 2);
            break;
        case 8:
            RunTrapType07(t, 3000, 1);
            break;
        case 9:
            RunTrapType09(t, 0);
            break;
        case 10:
            RunTrapType0A(t);
            break;
        case 11:
            RunTrapType0B(t, 2);
            break;
        case 12:
            RunTrapType0B(t, 1);
            break;
        case 13:
            RunTrapType0D(t, 2);
            break;
        case 14:
            RunTrapType0D(t, 1);
            break;
        case 15:
            RunTrapType0F(t, 0);
            break;
        case 16:
            RunTrapType10(t, 1);
            break;
        case 17:
            RunTrapType11(t, 3);
            break;
        case 18:
            RunTrapType12(t);
            break;
    }
    switch (t->type) {
        case 19:
            RunTrapType13(t);
            break;
        case 20:
            RunTrapType14(t);
            break;
        case 21:
            RunTrapType15(t, 0);
            break;
        case 22:
            RunTrapType15(t, 1);
            break;
        case 23:
            RunTrapType17(t);
            break;
        case 24:
            RunTrapType18(t, 0);
            break;
        case 25:
            RunTrapType18(t, 1);
            break;
        case 26:
            RunTrapType1A(t, 0);
            break;
        case 27:
            RunTrapType1B(t);
            break;
        case 28:
            RunTrapType1C(t);
            break;
        case 29:
            RunTrapType1D(t);
            break;
        case 30:
            RunTrapType1E(t);
            break;
        case 31:
            RunTrapType1F(t);
            break;
        case 32:
            RunTrapType20(t, 0);
            break;
        case 33:
            RunTrapType20(t, 1);
            break;
        case 34:
        case 35:
        case 36:
        case 37:
        case 38:
        case 40:
            RunTrapType22(t);
            break;
        case 39:
            RunTrapType27(t);
            break;
        case 41:
            RunTrapType29(t, 0);
            break;
        case 42:
            RunTrapType2A(t);
            break;
        case 43:
            RunTrapType2B(t);
            break;
        case 44:
            RunTrapType2C(t, 0);
            break;
        case 45:
            RunTrapType2C(t, 1);
            break;
        case 46:
            RunTrapType2E(t);
            break;
        case 47:
            RunTrapType2F(t);
            break;
        case 48:
            RunTrapType30(t);
            break;
        case 49:
            RunTrapType31(t);
            break;
        case 50:
            RunTrapType32(t, 0);
            break;
        case 51:
            RunTrapType32(t, 1);
            break;
    }
}

#undef ENGINE_PART
#define ENGINE_PART 2

#elif ENGINE_PART == 2

void StepCheckpoint(StageTrap *t, s32 id, s8 near) {
    Query30 q;

    switch (t->state) {
        case 0:
            t->homeX = t->x;
            t->homeY = t->y;
            t->homeZ = t->z;
            t->state++;
        case 1:
            if (near == 1 && DistanceSquared((VECTOR *)&gPlayer.x, (VECTOR *)t) < 1000000) {
                t->homeY -= 5;
            }
            t->timer++;
            t->y = t->homeY + (rsin((t->timer * 5 << 12) / 360) * 70 >> 12);
            t->x = t->homeX + (rsin((t->timer * 2 << 12) / 360) * 30 >> 12);
            t->z = t->homeZ + (rcos((t->timer * 2 << 12) / 360) * 30 >> 12);
            if (gSection - UpdatePathSegment(t) < 2) {
                DrawObjectModel(&gObjectModels[t->model], t);
            }
            q.pos.x = t->x;
            q.pos.y = t->y;
            q.pos.z = t->z;
            q.kind = 0;
            q.radius = 90;
            if (CollidePlayer(&q) != 0) {
                sSaved.valid = id;
                sSaved.pos.x = t->x;
                sSaved.pos.y = gPlayer.groundY;
                sSaved.pos.z = t->z;
                sSaved.section = t->section;
                sSaved.floorIndex = t->block;
                sSaved.cameraYaw = gCameraYaw;
                sSaved.cans = gCans;
                sSaved.viewY = gView[1];
                PlaySe(12);
                t->timer = 0;
                t->state++;
            }
            break;
        case 2:
            t->rotY += 227;
            t->y -= t->timer * 10;
            if (gSection - UpdatePathSegment(t) < 2) {
                DrawObjectModel(&gObjectModels[t->model], t);
            }
            if (++t->timer > 30) {
                t->state++;
            }
            break;
    }
}

#undef ENGINE_PART
#define ENGINE_PART 3

#elif ENGINE_PART == 3

void RunTrapType00(StageTrap *t) {
    switch (t->act) {
        case 0:
        default:
            if (gSection - UpdatePathSegment(t) < 2) {
                DrawObjectModel(&gObjectModels[t->model], t);
            }
            break;
        case 1:
            StepSolidTrap(t);
            break;
        case 2:
            KnockTrapAway(t);
            break;
        case 3:
            TumbleTrapAway(t, 70, 0);
            break;
        case 4:
            ShatterTrap(t);
            break;
        case 5:
            BurstTrapFast(t);
            break;
        case 6:
            BurstTrapSlow(t);
            break;
        case 7:
            SlideTrapAway(t, 0);
            break;
        case 8:
            TumbleTrapAct08(t);
            break;
        case 9:
            TumbleTrapAct09(t);
            break;
        case 11:
            TumbleTrapAct0B(t);
            break;
        case 12:
            KnockTrapAwayAct0C(t);
            break;
        case 13:
            StepDashBoard(t);
            break;
        case 15:
        case 16:
            SpinTrapAway(t);
            break;
        case 17:
            BurstTrapStacked(t);
            break;
        case 18:
            TumbleTrapAct12(t);
            break;
        case 19:
            TipTrapAway(t);
            break;
        case 14:
        case 20:
            StepFlipPanel(t);
            break;
        case 10:
        case 21:
            BurstTrapMedium(t);
            break;
        case 22:
            TumbleTrapAway(t, 50, 0);
            break;
        case 23:
            StepSprayTrap(t, 2);
            break;
        case 24:
            StepSprayTrap(t, 1);
            break;
        case 25:
            BurstTrapFastAlt(t);
            break;
        case 26:
            TumbleTrapAway(t, 20, 0);
            break;
        case 27:
            StepSmokingTumbleTrap(t);
            break;
        case 28:
            StepSmokingTrap(t);
            break;
        case 29:
            StepTrapAct1D(t);
            break;
        case 30:
            BurstRaisedTrap(t);
            break;
        case 31:
            TumbleTrapAway(t, 30, 1);
            break;
        case 32:
            TumbleTrapAway(t, 10, 2);
            break;
        case 33:
            StepTrapAct21(t);
            break;
        case 34:
            StepTrapAct22(t);
            break;
        case 35:
            StepTrapAct23(t);
            break;
        case 36:
            StepTrapAct24(t);
            break;
        case 37:
            StepTrapAct25(t, 2);
            break;
        case 38:
            StepTrapAct25(t, 1);
            break;
        case 39:
            StepTrapAct27(t);
            break;
        case 40:
            StepTrapAct28(t);
            break;
    }
}

const Speeds sChargeRanges = {{1500, 2000, 2500, 3000, 4000, 5000}};

void RunTrapType05(StageTrap *t, s8 dir) {
    switch (t->act) {
        case 1:
            RunTrapThenTurn(t, 20, 1000, dir);
            break;
        case 2:
            RunTrapThenTurn(t, 30, 1000, dir);
            break;
        case 3:
            RunTrapThenTurn(t, 40, 1000, dir);
            break;
        case 4:
            RunTrapThenTurn(t, 50, 1000, dir);
            break;
        case 5:
            RunTrapThenTurn(t, 60, 1000, dir);
            break;
        case 6:
            RunTrapThenTurn(t, 20, 1500, dir);
            break;
        case 7:
            RunTrapThenTurn(t, 30, 1500, dir);
            break;
        case 8:
            RunTrapThenTurn(t, 40, 1500, dir);
            break;
        case 9:
            RunTrapThenTurn(t, 50, 1500, dir);
            break;
        case 10:
            RunTrapThenTurn(t, 60, 1500, dir);
            break;
        case 11:
            RunTrapThenTurn(t, 20, 2000, dir);
            break;
        case 12:
            RunTrapThenTurn(t, 30, 2000, dir);
            break;
        case 13:
            RunTrapThenTurn(t, 40, 2000, dir);
            break;
        case 14:
            RunTrapThenTurn(t, 50, 2000, dir);
            break;
        case 15:
            RunTrapThenTurn(t, 60, 2000, dir);
            break;
        case 16:
            RunTrapThenTurn(t, 20, 2500, dir);
            break;
        case 17:
            RunTrapThenTurn(t, 30, 2500, dir);
            break;
        case 18:
            RunTrapThenTurn(t, 40, 2500, dir);
            break;
        case 19:
            RunTrapThenTurn(t, 50, 2500, dir);
            break;
        case 20:
            RunTrapThenTurn(t, 60, 2500, dir);
            break;
    }
}

void RunTrapType07(StageTrap *t, s32 range, s8 dir) {
    switch (t->act) {
        case 101:
            RunTrapThenTurn(t, 10, 500, dir);
            break;
        case 102:
            RunTrapThenTurn(t, 20, 500, dir);
            break;
        case 103:
            RunTrapThenTurn(t, 30, 500, dir);
            break;
        case 104:
            RunTrapThenTurn(t, 40, 500, dir);
            break;
        case 105:
            RunTrapThenTurn(t, 50, 500, dir);
            break;
        case 106:
            RunTrapThenTurn(t, 10, 1000, dir);
            break;
        case 107:
            RunTrapThenTurn(t, 20, 1000, dir);
            break;
        case 108:
            RunTrapThenTurn(t, 30, 1000, dir);
            break;
        case 109:
            RunTrapThenTurn(t, 40, 1000, dir);
            break;
        case 110:
            RunTrapThenTurn(t, 50, 1000, dir);
            break;
        case 111:
            RunTrapThenTurn(t, 10, 2000, dir);
            break;
        case 112:
            RunTrapThenTurn(t, 20, 2000, dir);
            break;
        case 113:
            RunTrapThenTurn(t, 30, 2000, dir);
            break;
        case 114:
            RunTrapThenTurn(t, 40, 2000, dir);
            break;
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
        case 10:
        case 11:
        case 12:
        case 13:
        case 14:
        case 15:
            ArmTrapByRange(t, range, t->type, t->act + 100);
            break;
        case 115:
            RunTrapThenTurn(t, 50, 2000, dir);
            break;
    }
}

#undef ENGINE_PART
#define ENGINE_PART 4

#elif ENGINE_PART == 4

void StepBouncingRoller(StageTrap *t, s32 arg) {
    s32 d;

    if (t->state == 0) {
        PolarToXZ(-10, t->homeRotY * 360 / 4096, (Vec3 *)gOffsetVec);
        t->x += gOffsetVec[0] >> 12;
        t->z += gOffsetVec[2] >> 12;
        t->timer += 10;
        t->rotX -= 113;
        d = rsin((t->timer << 12) / 360) * 150 >> 12;
        t->y = t->homeY - (d >= 0 ? d : -d);
        if (gSection - UpdatePathSegment(t) < 2) {
            UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
            if ((u16)(t->timer % 360) == 180 || (u16)(t->timer % 360) == 0) {
                PlaySe(0x36);
            }
        }
        if (t->touched != 0 && gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
            PlaySe(0x39);
        }
    }
}

#undef ENGINE_PART
#define ENGINE_PART 5

#elif ENGINE_PART == 5

void BurstTrapFast(StageTrap *t) {
    StageTrap c;
    u16 i;
    s16 model;

    switch (gStage) {
        case 5:
            model = 17;
            break;
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
        case 9:
        case 10:
        case 11:
            model = 12;
            break;
        case 14:
            break;
    }
    switch (t->state) {
        case 0:
            if (gSection - UpdatePathSegment(t) < 2) {
                UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
            }
            if (t->touched == 1) {
                t->timer = 0;
                t->state++;
                if (gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
                    PlaySe(0x38);
                }
            }
            break;
        case 1:
            t->timer++;
            for (i = 0; i < 8; i++) {
                c = *t;
                c.model = model;
                InterpSwing(sBurstHeights[i], t->timer, 30, gOffsetVec);
                c.y -= gOffsetVec[1] >> 12;
                /* MATCHING: * 3 << 1, not * 6: it loads the timer before the table. */
                c.x += sBurstDirs[i][0] * (t->timer * 3 << 1);
                c.z += sBurstDirs[i][1] * (t->timer * 3 << 1);
                c.rotZ += (t->timer * (sBurstSpins[i][0] * 2 + 10) << 12) / 360;
                DrawObjectModel(&gObjectModels[c.model], &c);
            }
            if (t->timer == 30) {
                t->state++;
            }
            break;
    }
}

void BurstTrapFastAlt(StageTrap *t) {
    StageTrap c;
    u16 i;
    s16 model;

    switch (gStage) {
        case 3:
            model = 17;
            break;
        case 4:
            model = 49;
            break;
        case 5:
            model = 39;
            break;
        case 0:
        case 1:
        case 2:
        case 9:
        case 10:
        case 11:
            model = 12;
            break;
        case 14:
            break;
    }
    switch (t->state) {
        case 0:
            if (gSection - UpdatePathSegment(t) < 2) {
                UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
            }
            if (t->touched == 1) {
                t->timer = 0;
                t->state++;
                if (gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
                    PlaySe(0x38);
                }
            }
            break;
        case 1:
            t->timer++;
            for (i = 0; i < 8; i++) {
                c = *t;
                c.model = model;
                InterpSwing(sBurstHeights[i], t->timer, 30, gOffsetVec);
                c.y -= gOffsetVec[1] >> 12;
                c.x += sBurstDirs[i][0] * (t->timer * 3 << 1);
                c.z += sBurstDirs[i][1] * (t->timer * 3 << 1);
                c.rotZ += (t->timer * (sBurstSpins[i][0] * 2 + 10) << 12) / 360;
                DrawObjectModel(&gObjectModels[c.model], &c);
            }
            if (t->timer == 30) {
                t->state++;
            }
            break;
    }
}

void BurstTrapSlow(StageTrap *t) {
    StageTrap c;
    u16 i;
    s16 model;

    switch (gStage) {
        case 0:
        case 1:
        case 2:
            model = 5;
            break;
        case 5:
            model = 39;
            break;
        case 3:
        case 4:
        case 9:
        case 10:
        case 11:
            model = 12;
            break;
        case 14:
            break;
    }
    switch (t->state) {
        case 0:
            if (gSection - UpdatePathSegment(t) < 2) {
                UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
            }
            if (t->touched == 1) {
                t->timer = 0;
                t->state++;
                if (gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
                    PlaySe(0x38);
                }
            }
            break;
        case 1:
            t->timer++;
            for (i = 0; i < 8; i++) {
                c = *t;
                c.model = model;
                InterpSwing(sBurstHeights[i], t->timer, 30, gOffsetVec);
                c.y -= gOffsetVec[1] >> 12;
                /* MATCHING: timer * 3 is reassociated onto the table's word. */
                c.x += sBurstDirs[i][0] * (t->timer * 2 + t->timer);
                c.z += sBurstDirs[i][1] * (t->timer * 2 + t->timer);
                c.rotZ += (t->timer * (sBurstSpins[i][0] * 2 + 10) << 12) / 360;
                if (t->timer % 3) {
                    DrawObjectModel(&gObjectModels[c.model], &c);
                }
            }
            if (t->timer == 40) {
                t->state++;
            }
            break;
    }
}

#undef ENGINE_PART
#define ENGINE_PART 6

#elif ENGINE_PART == 6

void BurstTrapMedium(StageTrap *t) {
    StageTrap c;
    u16 i;
    s16 model;

    switch (gStage) {
        case 9:
        case 10:
        case 11:
            model = 12;
            break;
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 12:
        case 13:
        case 14:
            model = 5;
            break;
    }
    switch (t->state) {
        case 0:
            if (gSection - UpdatePathSegment(t) < 2) {
                UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
            }
            if (t->touched == 1) {
                t->timer = 0;
                t->state++;
                if (gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
                    PlaySe(0x38);
                }
            }
            break;
        case 1:
            t->timer++;
            for (i = 0; i < 8; i++) {
                c = *t;
                c.model = model;
                InterpSwing(sBurstHeights[i], t->timer, 30, gOffsetVec);
                c.y -= gOffsetVec[1] >> 12;
                /* MATCHING: as in BurstTrapSlow, timer * 5 spelled out. */
                c.x += sBurstDirs[i][0] * (t->timer * 4 + t->timer);
                c.z += sBurstDirs[i][1] * (t->timer * 4 + t->timer);
                c.rotZ += (t->timer * (sBurstSpins[i][0] * 2 + 10) << 12) / 360;
                if (t->timer % 3) {
                    DrawObjectModel(&gObjectModels[c.model], &c);
                }
            }
            if (t->timer == 40) {
                t->state++;
            }
            break;
    }
}

#undef ENGINE_PART
#define ENGINE_PART 7

#elif ENGINE_PART == 7

void StepFlipPanel(StageTrap *t) {
    switch (t->state) {
        case 0:
            t->homeRotZ = 0;
        case 1:
            if (gSection - UpdatePathSegment(t) < 2) {
                UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
            }
            if (--t->homeRotZ < 0) {
                t->homeRotZ = 0;
            }
            if (t->homeRotZ == 0 && t->touched != 0) {
                t->timer = 0;
                t->state++;
                PlaySe(0x20);
                gPlayer.x = t->x;
                gPlayer.z = t->z;
            }
            break;
        case 2:
            t->timer++;
            t->rotX = Lerp(0, 0x400, t->timer, 3);
            if (t->timer == 3) {
                t->timer = 0;
                t->state++;
            }
            if (gSection - UpdatePathSegment(t) < 2) {
                DrawObjectModel(&gObjectModels[t->model], t);
            }
            break;
        case 3:
            t->timer++;
            t->rotX = ((rand() % 10 + 85) << 12) / 360;
            if (t->timer == 20) {
                t->timer = 0;
                t->state++;
            }
            if (gSection - UpdatePathSegment(t) < 2) {
                DrawObjectModel(&gObjectModels[t->model], t);
            }
            break;
        case 4:
            t->timer++;
            t->rotX = Lerp(0x400, 0, t->timer, 8);
            if (t->timer == 8) {
                t->state = 1;
                t->touched = 0;
                t->timer = 0;
                t->homeRotZ = 60;
            }
            if (gSection - UpdatePathSegment(t) < 2) {
                DrawObjectModel(&gObjectModels[t->model], t);
            }
            break;
    }
}

#undef ENGINE_PART
#define ENGINE_PART 8

#elif ENGINE_PART == 8

void TipTrapAway(StageTrap *t) {
    switch (t->state) {
        case 0:
            t->homeY = t->y;
            if (gSection - UpdatePathSegment(t) < 2) {
                UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
            }
            if (t->touched != 0) {
                t->homeX = 0;
                t->homeY = t->y;
                t->homeRotY = gPlayer.yaw;
                t->state++;
                t->timer = 0;
                if (gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
                    PlaySe(0x37);
                }
            }
            break;
        case 1:
            PolarToXZ(40, t->homeRotY * 360 / 4096, (Vec3 *)gOffsetVec);
            t->x -= gOffsetVec[0] >> 12;
            t->z -= gOffsetVec[2] >> 12;
            if (t->timer < 90) {
                t->rotX -= 113;
            }
            t->timer = (t->timer + 10) % 360;
            if (++t->homeX == 25) {
                t->state++;
            }
            if (gFrameCount % 3 && gSection - UpdatePathSegment(t) < 2) {
                DrawObjectModel(&gObjectModels[t->model], t);
            }
            break;
    }
}

#undef ENGINE_PART
#define ENGINE_PART 9

#elif ENGINE_PART == 9

void UpdateStageEvents(void) {
    if (gMissState <= 0) {
        StepMissSequence();
        UpdateStageEventHook();
        UpdateGoalEvents();
    }
}

void PlaySpriteHitSe(u8 se) {
    switch (se) {
        case 0:
            break;
        case 1:
            PlaySe(0x40);
            break;
        case 2:
            PlaySe(0x42);
            break;
        case 3:
            PlaySe(0x44);
            break;
        case 4:
            PlaySe(0x45);
            break;
    }
}

void ResetSpriteWalks(void) {
    if (gSkipLandRumble == 1 || sWalksReady == 0) {
        bzero(sWalkState, 100);
        sWalksReady = 1;
        sGoalLightsSet = 0;
    }
}

void StartSpriteWalk(EngSprite *r, u16 i, s16 angle) {
    r->driftBaseY = 0;
    r->driftCount = r->x;
    r->driftPhase = r->z;
    WALK_ANGLE(i) = angle;
    WALK_BASE(i) = r->id;
    WALK_FRAME(i) = rand() % 3;
    sWalkTimer[i] = 0;
}

void RunTrapTypeFF(StageTrap *t) {
    switch (t->act) {
        case 0:
            break;
        case 1:
            DrawTrapWhileLinked(t);
            break;
        case 5:
            EmitTrapParticles(t);
            break;
        case 10:
            StepCheckpoint(t, 1, 0);
            break;
        case 11:
            StepCheckpoint(t, 2, 0);
            break;
        case 12:
            StepCheckpoint(t, 1, 1);
            break;
        case 13:
            StepCheckpoint(t, 2, 1);
            break;
        case 100:
            StepBouncingRoller(t, 80);
            break;
        case 101:
            func_800F79A0(t, 50);
            break;
        case 102:
            func_800F79A0(t, 180);
            break;
        case 103:
            StepTrapFFAct67(t);
            break;
        case 104:
            StepTrapFFAct68(t);
            break;
        case 105:
            StepFlungPiece(t);
            break;
        case 106:
            StepMissDebris(t);
            break;
    }
}

void RunTrapType01(StageTrap *t, s8 reverse) {
    s16 dir;

    dir = 1;
    if (reverse == 1) {
        dir = -1;
    }
    switch (t->act) {
        case 0:
        default:
            RunTrapAhead(t, 0);
            break;
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
        case 10:
            RunTrapAhead(t, t->act * 10 * dir);
            break;
        case 11:
        case 12:
        case 13:
        case 14:
        case 15:
        case 16:
        case 17:
        case 18:
        case 19:
        case 20:
        case 21:
        case 22:
        case 23:
        case 24:
        case 25:
            RunTrapAheadNonstop(t, (t->act - 10) * 10 * dir);
            break;
    }
}

void RunTrapType02(StageTrap *t) {
    switch (t->act) {
        case 0:
        default:
            RunTrapAhead(t, 0);
            break;
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
        case 10:
            RunTrapAlongPath(t, t->act * 10, 0);
            break;
        case 11:
        case 12:
        case 13:
        case 14:
        case 15:
        case 16:
        case 17:
        case 18:
        case 19:
            RunTrapAlongPath(t, (t->act - 10) * 10, 1);
            break;
    }
}

#undef ENGINE_PART
#define ENGINE_PART 10

#elif ENGINE_PART == 10

void RunTrapType09(StageTrap *t, s8 reverse) {
    s16 dir;

    dir = 1;
    if (reverse == 1) {
        dir = -1;
    }
    RunTrapAlongPath(t, t->act * 10 + dir * 60, 0);
    DrawTrapWarning(t);
    if (t->state == 0) {
        PlaySe(0x11);
    }
}

void RunTrapType0A(StageTrap *t) {
    Speeds sp = sChargeRanges;

    StepChargingTrap(t, t->act * 10 + 60, sp.v[t->act / 5]);
    DrawTrapWarning(t);
    if (t->state == 0) {
        PlaySe(0x11);
    }
}

#undef ENGINE_PART
#define ENGINE_PART 11

#elif ENGINE_PART == 11

void RunTrapType32(StageTrap *t, s8 mode) {
    if (mode == 0) {
        switch (t->act) {
            case 0:
            case 1:
            case 2:
            case 3:
            case 4:
            case 5:
            case 6:
            case 7:
            case 8:
            case 9:
            case 10:
                StepTrapType32(t, t->act * 10, 0);
                break;
            case 11:
            case 12:
            case 13:
            case 14:
            case 15:
            case 16:
            case 17:
            case 18:
            case 19:
            case 20:
                StepTrapType32(t, -((t->act - 10) * 10), 2);
                break;
        }
    } else {
        switch (t->act) {
            case 0:
            case 1:
            case 2:
            case 3:
                StepTrapType32(t, (t->act + 1) * 10, mode);
                break;
        }
    }
}

#undef ENGINE_PART
#define ENGINE_PART 12

#elif ENGINE_PART == 12

void RunTrapType0F(StageTrap *t, s8 dir) {
    switch (t->act) {
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
        case 10:
            StepTrapType0F(t, t->act * 10, dir, 0);
            break;
        case 11:
            StepTrapType0F(t, 60, 5, 0);
            break;
    }
}

void RunTrapType10(StageTrap *t, s8 dir) {
    switch (t->act) {
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
        case 10:
            StepTrapType0F(t, t->act * 10, dir, 0);
            break;
        case 11:
        case 12:
        case 13:
        case 14:
        case 15:
        case 16:
        case 17:
        case 18:
        case 19:
        case 20:
            StepTrapType0F(t, (t->act - 10) * 10, dir + 1, 0);
            break;
    }
}

void RunTrapType11(StageTrap *t, s8 dir) {
    switch (t->act) {
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
            StepTrapType0F(t, (t->act + 6) * 10, dir, 0);
            break;
    }
}

void RunTrapType12(StageTrap *t) {
    switch (t->act) {
        case 0:
        case 1:
        case 2:
            StepTrapType12(t, (t->act + 1) * 10, 60);
            break;
        case 3:
        case 4:
        case 5:
            StepTrapType12(t, (t->act - 2) * 10, 90);
            break;
    }
}

void RunTrapType2B(StageTrap *t) {
    switch (t->act) {
        case 0:
            StepTrapType0F(t, 5, 4, 250);
            break;
        case 1:
            StepTrapType0F(t, 10, 4, 250);
            break;
        case 2:
            StepTrapType0F(t, 5, 4, 500);
            break;
        case 3:
            StepTrapType0F(t, 10, 4, 500);
            break;
        case 4:
            break;
    }
}

void RunTrapType1F(StageTrap *t) {
    StepTrapType1F(t, sType1FSpeeds[t->act]);
}

void RunTrapType0D(StageTrap *t, s8 dir) {
    StepTrapType0D(t, t->act * 10 + 70, 45, dir);
}

void RunTrapType1B(StageTrap *t) {
    StepTrapType1B(t, t->act * 10 + 20, 5000);
}

void RunTrapType1D(StageTrap *t) {
    StepTrapType1D(t, t->act * 10 + 10);
}

void RunTrapType30(StageTrap *t) {
    StepTrapType30(t, t->act * 10 + 80, 7000);
}

void RunTrapType18(StageTrap *t, s8 dir) {
    if (t->act < 10) {
        StepTrapType18(t, t->act * 10 + 10, 1200, dir);
    } else {
        StepTrapType18(t, -((t->act - 10) * 10 + 10), 1200, dir);
    }
}

void RunTrapType1A(StageTrap *t, s8 dir) {
    StepTrapType1A(t, t->act * 10 + 10, 1200, dir);
}

void RunTrapType1C(StageTrap *t) {
    switch (t->act) {
        case 0:
            StepTrapType1C(t, 1000, 1, 0);
            break;
        case 1:
            StepTrapType1C(t, 1000, 2, 0);
            break;
        case 2:
            StepTrapType1C(t, 2000, 1, 0);
            break;
        case 3:
            StepTrapType1C(t, 2000, 2, 0);
            break;
        case 4:
            StepTrapType1C(t, 3000, 1, 0);
            break;
        case 5:
            StepTrapType1C(t, 3000, 2, 0);
            break;
    }
}

void RunTrapType1E(StageTrap *t) {
    StepTrapType1E(t, t->act * 5 + 10);
}

void RunTrapType20(StageTrap *t, s8 dir) {
    StepTrapType20(t, (u8)(t->act % 3) * 10 + 10, 700, dir);
}

void RunTrapType2A(StageTrap *t) {
    switch (t->act) {
        case 0:
            StepTrapType2A(t, 5, 250);
            break;
        case 1:
            StepTrapType2A(t, 10, 250);
            break;
        case 2:
            StepTrapType2A(t, 5, 500);
            break;
        case 3:
            StepTrapType2A(t, 10, 500);
            break;
    }
}

void RunTrapType29(StageTrap *t, s8 unused) {
    switch (t->act) {
        case 0:
        case 1:
        case 2:
            StepTrapType29(t, 2, t->act);
            break;
        case 3:
        case 4:
        case 5:
            StepTrapType29(t, 1, t->act % 3);
            break;
    }
}

void RunTrapType27(StageTrap *t) {
    switch (t->act) {
        case 0:
        case 1:
        case 2:
        case 3:
            StepTrapType27(t, t->act * 1000 + 1000);
            break;
    }
}

void RunTrapType31(StageTrap *t) {
    switch (t->act) {
        case 0:
            StepTrapType31(t, 0);
            break;
        case 1:
            StepTrapType31(t, 1);
            break;
    }
}

void RunTrapType2F(StageTrap *t) {
    RunTrapAheadAtHeight(t, t->act * 10 + 30);
}

#undef ENGINE_PART
#define ENGINE_PART 13

#elif ENGINE_PART == 13

void StepSolidTrap(StageTrap *t) {
    if (gSection - UpdatePathSegment(t) < 2) {
        UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
    }
    if (t->touched != 0 && gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
        PlaySe(0x39);
    }
}

#undef ENGINE_PART
#define ENGINE_PART 14

#elif ENGINE_PART == 14

void ArmTrapByRange(StageTrap *t, s32 range, u8 type, u8 act) {
    if (t->state == 0) {
        RunTrapAhead(t, 0);
        if (range * range > DistanceSquared((VECTOR *)&gPlayer.x, (VECTOR *)t)) {
            t->state = 0;
            t->timer = 0;
            t->type = type;
            t->act = act;
        }
    }
}

#undef ENGINE_PART
#define ENGINE_PART 15

#elif ENGINE_PART == 15

void RunTrapType13(StageTrap *t) {
    switch (t->act) {
        case 0:
            TurnTrapOnApproach(t, 500, 1);
            break;
        case 1:
            TurnTrapOnApproach(t, 500, 2);
            break;
        case 2:
            TurnTrapOnApproach(t, 1000, 1);
            break;
        case 3:
            TurnTrapOnApproach(t, 1000, 2);
            break;
        case 4:
            TurnTrapOnApproach(t, 2000, 1);
            break;
        case 5:
            TurnTrapOnApproach(t, 2000, 2);
            break;
        case 6:
            TurnTrapOnApproach(t, 3000, 1);
            break;
        case 7:
            TurnTrapOnApproach(t, 3000, 2);
            break;
    }
}

void RunTrapType14(StageTrap *t) {
    switch (t->act) {
        case 0:
            OpenGateOnApproach(t, 1000, 1);
            break;
        case 1:
            OpenGateOnApproach(t, 1000, 2);
            break;
        case 2:
            OpenGateOnApproach(t, 2000, 1);
            break;
        case 3:
            OpenGateOnApproach(t, 2000, 2);
            break;
        case 4:
            OpenGateOnApproach(t, 3000, 1);
            break;
        case 5:
            OpenGateOnApproach(t, 3000, 2);
            break;
        case 6:
            SwingGateContinuously(t, 0);
            break;
        case 7:
            SwingGateContinuously(t, 1);
            break;
    }
}

void RunTrapType15(StageTrap *t, u8 arg) {
    switch (t->act) {
        case 0:
            ToppleTrapOnApproach(t, 2, 1000, arg);
            break;
        case 1:
            ToppleTrapOnApproach(t, 3, 1000, arg);
            break;
        case 2:
            ToppleTrapOnApproach(t, 1, 1000, arg);
            break;
        case 3:
            ToppleTrapOnApproach(t, 2, 2000, arg);
            break;
        case 4:
            ToppleTrapOnApproach(t, 3, 2000, arg);
            break;
        case 5:
            ToppleTrapOnApproach(t, 1, 2000, arg);
            break;
        case 6:
            ToppleTrapOnApproach(t, 2, 3000, arg);
            break;
        case 7:
            ToppleTrapOnApproach(t, 3, 3000, arg);
            break;
        case 8:
            ToppleTrapOnApproach(t, 1, 3000, arg);
            break;
    }
}

void RunTrapType17(StageTrap *t) {
    switch (t->act) {
        case 0:
            ShiftTrapOnApproach(t, 0, 1000);
            break;
        case 1:
            ShiftTrapOnApproach(t, 1, 1000);
            break;
        case 2:
            ShiftTrapOnApproach(t, 0, 2000);
            break;
        case 3:
            ShiftTrapOnApproach(t, 1, 2000);
            break;
        case 4:
            ShiftTrapOnApproach(t, 0, 3000);
            break;
        case 5:
            ShiftTrapOnApproach(t, 1, 3000);
            break;
        case 6:
            ShiftTrapOnApproach(t, 0, 3000);
            break;
        case 7:
            ShiftTrapOnApproach(t, 1, 3000);
            break;
    }
}

#undef ENGINE_PART
#define ENGINE_PART 16

#elif ENGINE_PART == 16

void RunTrapType22(StageTrap *t) {
    switch (t->act) {
        case 0:
            switch (gStage) {
                case 0:
                    func_800F82E0(t);
                    return;
                case 1:
                    func_800F8454(t);
                    break;
            }
        case 1:
            switch (gStage) {
                case 0:
                    func_800F87E4(t);
                    break;
                case 3:
                    func_800F84A8(t);
                    break;
                case 9:
                    func_800F74FC(t);
                    break;
                case 12:
                    func_800F7968(t);
                    break;
            }
            break;
    }
}

void RunTrapType2C(StageTrap *t, u8 arg) {
    switch (t->act) {
        case 0:
            SwapTrapModelOnApproach(t, 500, arg);
            break;
        case 1:
            SwapTrapModelOnApproach(t, 1000, arg);
            break;
        case 2:
            SwapTrapModelOnApproach(t, 1500, arg);
            break;
    }
}

void RunTrapType2E(StageTrap *t) {
    StepHoppingTrap(t, t->act);
}

#undef ENGINE_PART
#define ENGINE_PART 17

#elif ENGINE_PART == 17

void BurstTrapStacked(StageTrap *t) {
    StageTrap copy;

    BurstTrapSlow(t);
    if (t->state == 1) {
        copy = *t;
        copy.y -= 150;
        BurstTrapSlow(&copy);
    }
}

#undef ENGINE_PART
#define ENGINE_PART 18

#elif ENGINE_PART == 18

void UpdateGoalEvents(void) {
    UpdateGoalEventHook();
    StepGoalSequence();
    CycleGoalLights(sGoalLightsSet);
}

#undef ENGINE_PART
#define ENGINE_PART 19

#endif
