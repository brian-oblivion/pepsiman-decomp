/**
 * @file engine_code.inc
 * @brief The stage engine's shared code: stage sprites, traps, checkpoints
 *        and the per-frame stage events.
 *
 * Identical in the eight overlays built on the engine: ovl03 (stage 1-1),
 * ovl04 (1-2), ovl06 (2-1), ovl07 (2-2), ovl12 (4-1), ovl13 (4-2), ovl15
 * (5-1) and ovl16 (5-2). Each inclusion emits the next section, so an
 * overlay includes it once per section, in order, around its own code; a
 * function not yet C in every overlay stays in that overlay's unit.
 *
 * Once a frame the engine steps the stage events (the miss sequence, the
 * stage's own event, the goal), the traps and the stage sprites, each while
 * its switch is on. A trap's type picks its dispatcher, its act what the
 * dispatcher hands to a step routine. An overlay with no trap of a type
 * keeps that type's step as an empty stub. Types 34 to 38 and 40 and two
 * acts of type 255 call routines only one stage defines, picked by stage.
 */

#ifndef ENGINE_PART
#define ENGINE_PART 1
#endif

#if ENGINE_PART == 1

/**
 * @brief Steps the 100 stage sprites, once a frame after the traps: each
 *        live sprite whose switch is on walks if it is a walker, then is
 *        drawn and tested against the player if it can be hit, or drawn
 *        (drifting away after a hit) if not. A hit plays the sprite's sound
 *        and starts it drifting.
 */
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

/**
 * @brief Steps a walking sprite: it waits until the player comes within
 *        `range`, walks along its heading for `limit` frames, then stands;
 *        when the player hits it, it takes its hit frame and drifts away.
 * @param r     the sprite
 * @param i     its index in gStageSprites, which picks its walk state
 * @param angle its heading in degrees, set when the walk starts
 * @param speed how far it walks each frame
 * @param range how near the player must come to set it walking
 * @param limit how many frames it walks
 */
void StepSpriteWalk(EngSprite *r, u16 i, s16 angle, u16 speed, s32 range, s32 limit) {
    switch ((s8)sWalkState[i]) {
        case 0:
            StartSpriteWalk(r, i, angle);
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

/**
 * @brief Takes one step of a walking sprite: moves it `speed` units along
 *        its heading and, every fifth frame, shows the next of its four
 *        walk frames.
 * @param r     the sprite
 * @param i     its index in gStageSprites, which picks its walk state
 * @param speed how far it moves
 */
void AdvanceSpriteWalk(EngSprite *r, u16 i, u16 speed) {
    if (gFrameCount % 5 == 0) {
        WALK_FRAME(i) = (WALK_FRAME(i) + 1) % 4;
        r->id = WALK_FRAME(i) + WALK_BASE(i);
    }
    r->x += rsin((WALK_ANGLE(i) << 12) / 360) * speed >> 12;
    r->z += rcos((WALK_ANGLE(i) << 12) / 360) * speed >> 12;
}

/**
 * @brief Steps the traps, once a frame after the stage events: the 200
 *        placed ones (gTraps), then the 200 spawned during the stage, each
 *        that has a model and whose switch is on. The spawned ones are
 *        cleared first on a start or restart.
 */
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

/**
 * @brief Runs one frame of a trap by its type: each type (0 to 51, and 255
 *        for the special objects) calls its dispatcher, which reads the
 *        trap's act; pairs of types share one dispatcher with a direction
 *        or mode, and types 34 to 38 and 40 all run the stage's own trap.
 *        Any other type does nothing.
 * @param t the trap
 */
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

/**
 * @brief Steps a checkpoint: it floats in place, bobbing and circling, and
 *        when the player touches it, saves the state a miss resumes from
 *        (its position at the player's ground height, the section, floor,
 *        camera heading and height, and the cans), plays a chime and spins
 *        up out of sight over 30 frames.
 * @param t    the checkpoint
 * @param id   its number, saved as the resume point's: 1 or 2
 * @param near 1 to make it rise while the player is within 1000 units
 */
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

/**
 * @brief Type 0, the course's standing objects: the act picks how one
 *        reacts to the player. Act 0 (and any act without a case) is only
 *        drawn; the others are solid, are knocked, tipped, slid or spun
 *        away, tumble from a height, shatter or burst into pieces, smoke,
 *        spray, or flip, each by its own step routine.
 * @param t the trap
 */
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

/** @brief A type-10 trap's charge distance, by its act / 5. */
const Speeds sChargeRanges = {{1500, 2000, 2500, 3000, 4000, 5000}};

/**
 * @brief Types 5 and 6: a trap that runs ahead, then turns a quarter turn.
 *        Acts 1 to 20 set its speed (20 to 60, by the act's place in each
 *        group of five) and how far it runs before turning (1000, 1500,
 *        2000 or 2500, by the group); act 0 does nothing.
 * @param t   the trap
 * @param dir the side it turns to: 2 for type 5, 1 for type 6
 */
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

/**
 * @brief Types 7 and 8: a trap that waits for the player, then runs ahead
 *        and turns a quarter turn. Acts 1 to 15 wait on the floor until the
 *        player comes within `range`, then become act + 100; acts 101 to
 *        115 run at 10 to 50 (by the act's place in each group of five) for
 *        500, 1000 or 2000 units (by the group) before turning.
 * @param t     the trap
 * @param range how near the player must come to start it
 * @param dir   the side it turns to: 2 for type 7, 1 for type 8
 */
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

/**
 * @brief Type 255 act 100: a roller that rolls 10 units a frame against
 *        its placed heading, spinning and bouncing up to 150 units, with a
 *        thud each time it lands while in view; touching a player who can
 *        be hit plays the hit sound.
 * @param t   the roller
 * @param arg unused
 */
void StepBouncingRoller(StageTrap *t, s32 arg) {
    s32 d;

    if (t->state == 0) {
        PolarToXZ(-10, ANGLE_TO_DEG(t->homeRotY), (Vec3 *)gOffsetVec);
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

/**
 * @brief Type 0 act 5: an object that bursts when the player touches it.
 *        Until then it is drawn and tested; on a touch it plays the burst
 *        sound (when the player can be hit) and is drawn as eight pieces
 *        that fly out quickly, arcing and spinning, for 30 frames, then is
 *        gone. The pieces' model depends on the stage; stages without a
 *        case leave it unset.
 * @param t the trap
 */
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

/**
 * @brief Type 0 act 25: the same burst as BurstTrapFast, eight fast pieces
 *        for 30 frames, with another choice of piece model per stage.
 * @param t the trap
 */
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

/**
 * @brief Type 0 act 6: an object that bursts when the player touches it,
 *        into eight pieces that spread slowly, flickering (drawn on two
 *        frames of every three), for 40 frames. The pieces' model depends
 *        on the stage.
 * @param t the trap
 */
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

/**
 * @brief Type 0 acts 10 and 21: an object that bursts when the player
 *        touches it, into eight flickering pieces that spread faster than
 *        BurstTrapSlow's, for 40 frames. The pieces' model depends on the
 *        stage.
 * @param t the trap
 */
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

/**
 * @brief Type 0 acts 14 and 20: a panel that flips up under the player.
 *        When the player touches it, it plays a sound, centres the player
 *        on it, opens a quarter turn in 3 frames, shakes open for 20 frames,
 *        closes in 8 and then waits 60 frames before it can flip again.
 * @param t the panel
 */
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
            t->rotX = Lerp(0, ANGLE_DEG(90), t->timer, 3);
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
            t->rotX = Lerp(ANGLE_DEG(90), 0, t->timer, 8);
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

/**
 * @brief Type 0 act 19: an object that tips over and is pushed away along
 *        the player's heading when the player touches it, flickering, for
 *        25 frames; then it is gone.
 * @param t the trap
 */
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
                    PlaySe(SE_TRAP_HIT);
                }
            }
            break;
        case 1:
            PolarToXZ(40, ANGLE_TO_DEG(t->homeRotY), (Vec3 *)gOffsetVec);
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

/**
 * @brief Steps the stage's events, once a frame before the traps, while
 *        the stage is playing or a miss is playing out: the miss sequence,
 *        the stage's own event and the goal's events, the first two being
 *        each overlay's own.
 */
void UpdateStageEvents(void) {
    if (gMissState <= 0) {
        StepMissSequence();
        UpdateStageEventHook();
        UpdateGoalEvents();
    }
}

/**
 * @brief Plays the sound a stage sprite makes when the player hits it.
 * @param se the sprite's sound: 0 none, 1 to 4 one of four sounds; any
 *           other is silent
 */
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

/**
 * @brief Clears the walking sprites' states on a start or restart, or the
 *        first time it runs, and turns the goal lights back to their first
 *        set.
 */
void ResetSpriteWalks(void) {
    if (gSkipLandRumble == 1 || sWalksReady == 0) {
        bzero(sWalkState, 100);
        sWalksReady = 1;
        sGoalLightsSet = 0;
    }
}

/**
 * @brief Starts a sprite's walk: keeps its start position, sets its
 *        heading, takes its current sprite as the walk's first frame and
 *        starts on a random frame.
 * @param r     the sprite
 * @param i     its index in gStageSprites, which picks its walk state
 * @param angle its heading in degrees
 */
void StartSpriteWalk(EngSprite *r, u16 i, s16 angle) {
    r->driftBaseY = 0;
    r->driftCount = r->x;
    r->driftPhase = r->z;
    WALK_ANGLE(i) = angle;
    WALK_BASE(i) = r->id;
    WALK_FRAME(i) = rand() % 3;
    sWalkTimer[i] = 0;
}

/**
 * @brief Type 255, the special objects, by act: 1 is drawn only while a
 *        linked trap is in its early states, 5 gives off particles, 10 to
 *        13 are the checkpoints (1 and 2; 12 and 13 rise as the player
 *        nears), 100 a bouncing roller, 101 and 102 stage 4-1's own object,
 *        103 and 104 the overlay's own, 105 a piece flung from the player
 *        and 106 a miss's debris. Act 0 and any other do nothing.
 * @param t the trap
 */
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
            StepTrapFFAct65(t, 50);
            break;
        case 102:
            StepTrapFFAct65(t, 180);
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

/**
 * @brief Types 1 and 3: a trap that runs straight ahead on the floor. Acts
 *        1 to 10 run at act * 10 for up to 150 frames; acts 11 to 25 at
 *        (act - 10) * 10 without stopping; act 0 (and any other) stands.
 * @param t       the trap
 * @param reverse 1 (type 3) to run backwards
 */
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

/**
 * @brief Type 2: a trap that runs along the course on the floor for up to
 *        150 frames. Acts 1 to 10 run ahead at act * 10; acts 11 to 19 face
 *        back along the course at (act - 10) * 10; act 0 (and any other)
 *        stands.
 * @param t the trap
 */
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

/**
 * @brief Type 9: a trap that runs along the course at act * 10 + 60 for up
 *        to 150 frames, sounding a warning on its first frame.
 * @param t       the trap
 * @param reverse 1 to take 60 off the speed instead
 */
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

/**
 * @brief Type 10: a trap that charges at act * 10 + 60, sounding a warning
 *        on its first frame: it charges until it has gone its act's
 *        distance (sChargeRanges), recoils, then stands kicking up dust.
 * @param t the trap
 */
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

/**
 * @brief Types 50 and 51: hands the act's speed to the overlay's type-50
 *        step. With mode 0, acts 0 to 10 give act * 10 and acts 11 to 20
 *        (act - 10) * 10 backwards in mode 2; with mode 1, acts 0 to 3 give
 *        (act + 1) * 10.
 * @param t    the trap
 * @param mode 0 for type 50, 1 for type 51
 */
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

/**
 * @brief Type 15: hands the overlay's type-15 step a speed of act * 10 for
 *        acts 0 to 10, or 60 in mode 5 for act 11.
 * @param t   the trap
 * @param dir the step's mode for acts 0 to 10: 0
 */
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

/**
 * @brief Type 16: hands the overlay's type-15 step a speed of act * 10 for
 *        acts 0 to 10, or (act - 10) * 10 in the next mode for acts 11 to
 *        20.
 * @param t   the trap
 * @param dir the step's mode for acts 0 to 10: 1
 */
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

/**
 * @brief Type 17: hands the overlay's type-15 step a speed of
 *        (act + 6) * 10 for acts 0 to 6.
 * @param t   the trap
 * @param dir the step's mode: 3
 */
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

/**
 * @brief Type 18: a two-part trap that runs ahead, then skids to a stop
 *        spinning and throwing out effects: acts 0 to 2 run at (act + 1) * 10 for 60
 *        frames, acts 3 to 5 at (act - 2) * 10 for 90.
 * @param t the trap
 */
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

/**
 * @brief Type 43: hands the overlay's type-15 step, in mode 4, a speed of
 *        5 or 10 and a distance of 250 or 500 by act (0 to 3); act 4 does
 *        nothing.
 * @param t the trap
 */
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

/**
 * @brief Type 31: a trap that rolls ahead, bouncing, at its act's speed
 *        from sType1FSpeeds. Act 5 reads the halfword after the table's
 *        five speeds, which differs per overlay.
 * @param t the trap
 */
void RunTrapType1F(StageTrap *t) {
    StepTrapType1F(t, sType1FSpeeds[t->act]);
}

/**
 * @brief Types 13 and 14: hands the overlay's type-13 step a speed of
 *        act * 10 + 70 and 45.
 * @param t   the trap
 * @param dir the side: 2 for type 13, 1 for type 14
 */
void RunTrapType0D(StageTrap *t, s8 dir) {
    StepTrapType0D(t, t->act * 10 + 70, 45, dir);
}

/**
 * @brief Type 27: hands the overlay's type-27 step a speed of
 *        act * 10 + 20 and a range of 5000.
 * @param t the trap
 */
void RunTrapType1B(StageTrap *t) {
    StepTrapType1B(t, t->act * 10 + 20, 5000);
}

/**
 * @brief Type 29: a three-part trap, a body and two swinging parts, that
 *        walks ahead at act * 10 + 10.
 * @param t the trap
 */
void RunTrapType1D(StageTrap *t) {
    StepTrapType1D(t, t->act * 10 + 10);
}

/**
 * @brief Type 48: hands the overlay's type-48 step a speed of
 *        act * 10 + 80 and a range of 7000.
 * @param t the trap
 */
void RunTrapType30(StageTrap *t) {
    StepTrapType30(t, t->act * 10 + 80, 7000);
}

/**
 * @brief Types 24 and 25: hands the overlay's type-24 step a speed of
 *        act * 10 + 10 for acts below 10, or the same counted from 10 and
 *        backwards for the rest, and a range of 1200.
 * @param t   the trap
 * @param dir 0 for type 24, 1 for type 25
 */
void RunTrapType18(StageTrap *t, s8 dir) {
    if (t->act < 10) {
        StepTrapType18(t, t->act * 10 + 10, 1200, dir);
    } else {
        StepTrapType18(t, -((t->act - 10) * 10 + 10), 1200, dir);
    }
}

/**
 * @brief Type 26: hands the overlay's type-26 step a speed of
 *        act * 10 + 10 and a range of 1200.
 * @param t   the trap
 * @param dir the step's mode: 0
 */
void RunTrapType1A(StageTrap *t, s8 dir) {
    StepTrapType1A(t, t->act * 10 + 10, 1200, dir);
}

/**
 * @brief Type 28: a trap that falls over a quarter turn to one side when
 *        the player comes within 1000, 2000 or 3000 units, then bounces;
 *        even acts (0 to 4) fall one way, odd acts (1 to 5) the other.
 * @param t the trap
 */
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

/**
 * @brief Type 30: hands the overlay's type-30 step a speed of
 *        act * 5 + 10.
 * @param t the trap
 */
void RunTrapType1E(StageTrap *t) {
    StepTrapType1E(t, t->act * 5 + 10);
}

/**
 * @brief Types 32 and 33: hands the overlay's type-32 step a speed of 10,
 *        20 or 30 (by the act modulo 3) and a range of 700.
 * @param t   the trap
 * @param dir 0 for type 32, 1 for type 33
 */
void RunTrapType20(StageTrap *t, s8 dir) {
    StepTrapType20(t, (u8)(t->act % 3) * 10 + 10, 700, dir);
}

/**
 * @brief Type 42: a trap that backs away along the floor, at 5 or 10, until
 *        it is 250 or 500 units from where it started (by act, 0 to 3), then
 *        stands.
 * @param t the trap
 */
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

/**
 * @brief Type 41: hands the overlay's type-41 step 2 for acts 0 to 2 and 1
 *        for acts 3 to 5, with the act modulo 3.
 * @param t      the trap
 * @param unused unused
 */
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

/**
 * @brief Type 39: a trap that drops to the floor when the player comes
 *        within act * 1000 + 1000 units (acts 0 to 3), bounces and tips
 *        over.
 * @param t the trap
 */
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

/**
 * @brief Type 49: hands the overlay's type-49 step its act, 0 or 1.
 * @param t the trap
 */
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

/**
 * @brief Type 47: a trap that flies straight ahead at its own height at
 *        act * 10 + 30, with a sound as it starts and as the player first
 *        comes near.
 * @param t the trap
 */
void RunTrapType2F(StageTrap *t) {
    RunTrapAheadAtHeight(t, t->act * 10 + 30);
}

#undef ENGINE_PART
#define ENGINE_PART 13

#elif ENGINE_PART == 13

/**
 * @brief Type 0 act 1: a solid object that never moves; it is drawn and
 *        tested, and touching a player who can be hit plays the hit sound.
 * @param t the trap
 */
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

/**
 * @brief Keeps a waiting trap standing on the floor until the player comes
 *        within `range`, then turns it into `type` and `act` from its first
 *        state.
 * @param t     the trap
 * @param range how near the player must come
 * @param type  the type it becomes
 * @param act   the act it becomes
 */
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

/**
 * @brief Type 19: a trap that turns an eighth of a turn to one side when
 *        the player comes within 500, 1000, 2000 or 3000 units (by act, 0
 *        to 7); even acts turn one way, odd acts the other.
 * @param t the trap
 */
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

/**
 * @brief Type 20: a two-part gate. Acts 0 to 5 swing it half a turn open,
 *        to one side or the other, when the player comes within 1000, 2000
 *        or 3000 units; act 6 swings it to and fro for ever, act 7 once.
 * @param t the trap
 */
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

/**
 * @brief Types 21 and 22: a trap that falls when the player comes within
 *        1000, 2000 or 3000 units (by act, 0 to 8): tilting to one side or
 *        the other as it drops, or dropping straight to the floor.
 * @param t   the trap
 * @param arg how fast it tilts: 0 (type 21) over 12 frames, 1 (type 22)
 *            over 10
 */
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

/**
 * @brief Type 23: a trap that moves 150 units up or down (even acts one
 *        way, odd acts the other) when the player comes within 1000, 2000
 *        or 3000 units (by act, 0 to 7).
 * @param t the trap
 */
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

/**
 * @brief Types 34 to 38 and 40: the stage's own trap, picked by the stage
 *        being played. Act 0 runs stage 1-1's or 1-2's; act 1 (and act 0
 *        in any other stage) runs stage 1-1's, 2-1's, 4-1's or 5-1's
 *        second one. In the stages it does not list it does nothing.
 * @param t the trap
 */
void RunTrapType22(StageTrap *t) {
    switch (t->act) {
        case 0:
            switch (gStage) {
                case 0:
                    StepStage1_1Dropper(t);
                    return;
                case 1:
                    StepStage1_2Dropper(t);
                    break;
            }
        case 1:
            switch (gStage) {
                case 0:
                    StepStage1_1CreepingDropper(t);
                    break;
                case 3:
                    StepStage2_1Dropper(t);
                    break;
                case 9:
                    StepStage4_1Dropper(t);
                    break;
                case 12:
                    StepStage5_1Dropper(t);
                    break;
            }
            break;
    }
}

/**
 * @brief Types 44 and 45: a trap that changes model, with a sound, when
 *        the player comes within 500, 1000 or 1500 units (by act, 0 to 2).
 * @param t   the trap
 * @param arg 0 (type 44) to take the next model, which is then solid; 1
 *            (type 45) the previous, which is only drawn
 */
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

/**
 * @brief Type 46: a trap that hops in place, by its act swaying side to
 *        side or moving in one of four directions.
 * @param t the trap
 */
void RunTrapType2E(StageTrap *t) {
    StepHoppingTrap(t, t->act);
}

#undef ENGINE_PART
#define ENGINE_PART 17

#elif ENGINE_PART == 17

/**
 * @brief Type 0 act 17: a stack of two that bursts as BurstTrapSlow does,
 *        a second burst 150 units above the first while it lasts.
 * @param t the trap
 */
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

/** @brief Steps the goal's events: the overlay's goal hook and goal
 *         sequence, then the goal's flashing lights. */
void UpdateGoalEvents(void) {
    UpdateGoalEventHook();
    StepGoalSequence();
    CycleGoalLights(sGoalLightsSet);
}

#undef ENGINE_PART
#define ENGINE_PART 19

#endif
