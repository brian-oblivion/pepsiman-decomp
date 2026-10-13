/* Stage 5-2's overlay: the stage engine (engine.inc, engine_code.inc) with this
 * stage's own routines, and the engine functions still in asm. */

#include "engine.inc"

#include "engine_code.inc" /* UpdateStageSprites to StepStageTrap */

void ResetSpawnedTraps(void) {
    u32 i;
    StageTrap *t;
    ModelSlot *a;
    ModelSlot *b;
    ObjectModel *m;

    if (gSkipLandRumble == 1 || sSpawnedTrapsReady == 0) {
        bzero((u8 *)sSpawnedTraps, 200 * sizeof(StageTrap));
        for (i = 0; i < 200; i++) {
            sSpawnedTraps[i].switchNo = -1;
            sSpawnedTraps[i].model = -1;
            sSpawnedTraps[i].state = 0;
            sSpawnedTraps[i].timer = 0;
        }
        for (i = 0; i < 10; i++) {
            D_800D39D8[i].model = -1;
            D_800D39D8[i].state = 0;
            D_800D39D8[i].timer = 0;
        }
        for (i = 0; i < 2; i++) {
            sGoalTraps[i].model = -1;
            sGoalTraps[i].state = 0;
            sGoalTraps[i].timer = 0;
        }
        sSpawnedTrapsReady = 1;
        gSlotTable = (s32)sSpawnedTraps;
        gSlotTableSize = 200 * sizeof(StageTrap);
        m = gObjectModels;
        t = (StageTrap *)gTraps;
        a = (ModelSlot *)&m[t[194].model];
        b = (ModelSlot *)&m[t[195].model];
        b->scale.vy = a->scale.vy = b->scale.vx = a->scale.vx = 0x1000;
    }
}

#include "engine_code.inc" /* StepCheckpoint */

void StepFlungPiece(StageTrap *t) {
    s32 d;

    switch (t->state) {
        case 0:
            t->x = gPlayer.x;
            t->homeY = gPlayer.groundY - 50;
            t->z = gPlayer.z;
            t->rotY = gCourse.rot.vy + 0x800;
            t->homeRotX = 0;
            t->timer = 0;
            t->state++;
        case 1:
            PolarToXZ(40, t->rotY * 360 / 4096, (Vec3 *)gOffsetVec);
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
                D_800D3358[0] = -1;
            }
            break;
    }
}

/* MATCHING: player's gCourse.rot.vy, reached by its own address here so the
 * loop does not hoist gCourse's base. */
extern s16 D_800A7682;

/* MATCHING: player's gCourse.rot.vy, reached by its own address here so the
 * loop does not hoist gCourse's base. */
extern s16 D_800A7682;

void SpawnMissDebris(void) {
    s16 models[15] = {4, 0, 0, 3, 0, 0, 0, 0, 0, 0, 6, 0, 2, 5, 0};
    s16 i;
    s16 n;
    StageTrap *t;

    for (i = 0; i < 8; i++) {
        n = FindFreeSlot48();
        if (n == -1) {
            break;
        }
        t = &sSpawnedTraps[n];
        t->switchNo = 0xB8;
        t->type = 0xFF;
        t->act = 0x6A;
        gSwitches[0xB8].state = SWITCH_ON;
        t->homeY = gPlayer.groundY - 50;
        t->x = gPlayer.x + (rand() % 200 - 100);
        t->z = gPlayer.z + (rand() % 200 - 100);
        t->model = models[gStage];
        t->homeZ = rand() % 10 + 5;
        t->homeX = sBurstHeights[i] / 2;
        t->rotY = sBurstSpins[i][0] * 40960 / 360 + gCourse.rot.vy + 0x800;
        t->homeRotZ = rand() % 20 + 10;
        t->homeRotX = 0;
        t->timer = 0;
        t->state = 0;
    }
}

INCLUDE_ASM("asm/ovl/ovl16/nonmatchings/ovl16", StepMissDebris);

#include "engine_code.inc" /* RunTrapType00 to RunTrapType07 */

void StepTrapType32(StageTrap *t, s16 speed, s8 mode) {
    s16 dir;

    dir = 1;
    if (mode == 2) {
        dir = -1;
    }
    if (t->state == 0) {
        if (LatchProximity(t, 2000) != 0) {
            PlaySe(82);
        }
        SnapToFloor(t);
        if (t->timer < 150 && speed != 0) {
            t->timer++;
            if (mode == 0 || mode == 2) {
                t->rotY = GetPathDirection(t) + dir * 2048;
            }
            PolarToXZ(speed, t->rotY * 360 / 4096, (Vec3 *)gOffsetVec);
            t->x += gOffsetVec[0] >> 12;
            t->z += gOffsetVec[2] >> 12;
        }
        if (mode == 1 && DistanceSquared((VECTOR *)&t->homeX, (VECTOR *)t) > 250000) {
            t->state++;
        }
    }
    if (gSection - UpdatePathSegment(t) < 2) {
        UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
    }
    if (t->touched != 0 && gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
        PlaySe(0x39);
    }
}

void StepTrapType20(StageTrap *t, u16 speed, s32 arg, s8 dir) {
    switch (t->state) {
        case 0:
            t->homeRotZ = 0;
            t->state++;
        case 1:
            t->homeX = t->x;
            t->homeY = t->y;
            t->homeZ = t->z;
            t->state++;
        case 2:
            if (t->homeRotZ == 0) {
                PolarToXZ((s16)speed, t->rotY * 360 / 4096, (Vec3 *)gOffsetVec);
            } else {
                PolarToXZ(-(s16)speed, t->rotY * 360 / 4096, (Vec3 *)gOffsetVec);
            }
            t->x += gOffsetVec[0] >> 12;
            t->z += gOffsetVec[2] >> 12;
            if (LatchProximity(t, 1500) != 0) {
                PlaySe(0x84);
            }
            if (gSection - UpdatePathSegment(t) < 2) {
                UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
            }
            if (t->touched != 0 && gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
                PlaySe(0x39);
            }
            if (dir == 1 && arg * arg < DistanceSquared((VECTOR *)&t->homeX, (VECTOR *)t)) {
                t->state--;
                t->homeRotZ ^= 1;
            }
            break;
    }
}

void StepTrapType29(StageTrap *t, s32 arg, s8 arg2) {
    switch (t->state) {
        case 0:
            t->homeRotZ = 0;
            t->state++;
        case 1:
            if ((s8)arg != 1) {
                if ((s8)arg == 2) {
                    if (t->homeRotZ != 0) {
                        PolarToXZ(-10, (t->rotY + 0x400) * 360 / 4096, (Vec3 *)gOffsetVec);
                    } else {
                        PolarToXZ(10, (t->rotY + 0x400) * 360 / 4096, (Vec3 *)gOffsetVec);
                    }
                }
            } else if (t->homeRotZ == 0) {
                PolarToXZ(-10, (t->rotY + 0x400) * 360 / 4096, (Vec3 *)gOffsetVec);
            } else {
                PolarToXZ(10, (t->rotY + 0x400) * 360 / 4096, (Vec3 *)gOffsetVec);
            }
            t->x += gOffsetVec[0] >> 12;
            t->z += gOffsetVec[2] >> 12;
            if (++t->timer >= 26) {
                t->timer = 0;
                t->state++;
                PlaySe(0x83);
            }
            break;
        case 2:
            t->timer++;
            switch (arg2) {
                case 0:
                    t->homeRotZ ^= 1;
                    t->timer = 0;
                    t->state--;
                    break;
                case 1:
                    if (t->timer >= 16) {
                        t->homeRotZ ^= 1;
                        t->timer = 0;
                        t->state--;
                    }
                    break;
                case 2:
                    if (t->homeRotZ != 0 || t->timer >= 31) {
                        t->homeRotZ ^= 1;
                        t->timer = 0;
                        t->state--;
                    }
                    break;
            }
            break;
        default:
            return;
    }
    if (gSection - UpdatePathSegment(t) < 2) {
        UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
    }
    if (t->touched != 0 && gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
        PlaySe(0x36);
    }
}

INCLUDE_ASM("asm/ovl/ovl16/nonmatchings/ovl16", StepTrapType27);

INCLUDE_ASM("asm/ovl/ovl16/nonmatchings/ovl16", KnockTrapAway);

INCLUDE_ASM("asm/ovl/ovl16/nonmatchings/ovl16", TumbleTrapAway);

#include "engine_code.inc" /* StepBouncingRoller */

#include "engine/shatter_trap.inc"

#include "engine_code.inc" /* BurstTrapFast to BurstTrapSlow */

#include "engine_code.inc" /* BurstTrapMedium */

#include "engine_code.inc" /* StepFlipPanel */

INCLUDE_ASM("asm/ovl/ovl16/nonmatchings/ovl16", SpinTrapAway);

#include "engine_code.inc" /* TipTrapAway */

#include "engine/burst_raised_trap.inc"

INCLUDE_ASM("asm/ovl/ovl16/nonmatchings/ovl16", ToppleTrapOnApproach);

#include "engine/scatter_trap_dust.inc"

#include "engine/swap_trap_model_on_approach.inc"

#ifdef NON_MATCHING
void StepMissSequence(void) {
    s32 unused[18];
    StageTrap *m;

    m = &D_800AC6E0;
    if (D_800D3638 == 1) {
        switch (m->state) {
            case 0:
                gPlayer.control = 0;
                gPlayer.action = 0;
                gPlayer.motion[0] = 4;
                StopSpeed();
                EndJump();
                gMissState = -1;
                D_800D39D8[0].homeRotZ = 0;
                gFellOff = 1;
                m->timer = 0;
                D_800D39D8[0].homeX = gPlayer.x;
                D_800D39D8[0].homeY = gPlayer.y;
                D_800D39D8[0].homeZ = gPlayer.z;
                gRefView[0] = gView[0];
                gRefView[1] = gView[1];
                gRefView[2] = gView[2];
                m->state++;
            case 1:
                gPlayer.x = Lerp(D_800D39D8[0].homeX, D_8007AD68[gStage].x, m->timer, 20);
                gPlayer.z = Lerp(D_800D39D8[0].homeZ, D_8007AD68[gStage].z, m->timer, 20);
                gRefView[0] -= gPlayer.x + gWorldShift[0];
                gRefView[1] = gView[1];
                gRefView[2] -= gPlayer.z + gWorldShift[2];
                gRefView[3] = gView[3];
                gRefView[4] = gView[4];
                gRefView[5] = gView[5];
                GsSetRefView2((GsRVIEW2 *)gRefView);
                if (++m->timer == 10) {
                    gPlayer.motion[0] = 11;
                    gPlayer.action = 0;
                    gPlayer.jumpPower = 40;
                }
                if (m->timer >= 21) {
                    m->timer = 0;
                    m->state++;
                }
                break;
            case 2:
                if (gPlayer.y >= gPlayer.groundY) {
                    gPlayer.rideMode = (gPlayer.rideMode & 0xF0) | 4;
                    SpawnMissDebris();
                    gSpeedFixed = 800;
                    D_800D3638 = 2;
                    gMissState = 0;
                    gFellOff = 0;
                    gPlayer.control = 1;
                }
                break;
        }
    }
}
#else
INCLUDE_ASM("asm/ovl/ovl16/nonmatchings/ovl16", StepMissSequence);
#endif

/* MATCHING: this stage's goal walk, still in asm below. */
s8 func_800F60CC(StageTrap *t);

void StepGoalSequence(void) {
    StageTrap *t;
    StageTrap *u;

    t = sGoalTraps;
    u = &sGoalTraps[1];
    if (D_800D37A8 == 1) {
        ResetRumble();
        DrawFrozenFrame();
        switch (t->state) {
            case 0:
                gPlayer.control = 0;
                gPlayer.action = 0;
                gPlayer.motion[0] = 4;
                StopSpeed();
                gMissState = -2;
                ClipF = 1;
                t->timer = 0;
                u->homeX = gPlayer.x;
                u->homeY = gPlayer.y;
                u->homeZ = gPlayer.z;
                t->state++;
            case 1:
                if (func_800F60CC(u)) {
                    t->state++;
                    StartActResults();
                }
                break;
            case 2:
                if (IsResultsActive() == 0) {
                    t->state++;
                }
                break;
            case 3:
                t->state++;
                break;
            case 4:
                if (gInEditor != 1) {
                    if (gStageTableSet != 3) {
                        t->timer = 0;
                        t->state++;
                    } else {
                        gGameMode = 6;
                        gModeStep = 0;
                        ClipF = 0;
                    }
                }
                break;
            case 5:
                if (++t->timer >= 9) {
                    gStage = GetNextStage(13);
                    BeginAct3LoadScreen();
                    ClipF = 0;
                }
                break;
        }
    }
}

extern s8 D_800F9A60[];

extern s8 D_800F9A60[];
void func_800F8B4C(DirEnt16 *dir, u16 first, u16 count);

extern s8 D_800F9A60[];
void func_800F8B4C(DirEnt16 *dir, u16 first, u16 count);

void CycleGoalLights(s8 arg) {
    u16 periods[8] = {30, 30, 30, 30, 20, 20, 10, 10};
    s8 lights[8] = {0, 0, 0, 0, 1, 1, 2, 2};
    s16 i;

    if (gSectionEdges[-1] - gSection < 20) {
        for (i = 0; i < 8; i++) {
            if (gFrameCount % periods[i] == 0) {
                func_800F8B4C((DirEnt16 *)0x8018D000, i * 4 + D_800F9A60[lights[i]] * 2, 1);
            } else if (gFrameCount % periods[i] == periods[i] >> 1) {
                func_800F8B4C((DirEnt16 *)0x8018D000, i * 4 + 1 + D_800F9A60[lights[i]] * 2, 1);
            }
        }
    }
}

void func_800F8BD4(void);

/* MATCHING: an unused 32-byte local gives retail's frame. */
void func_800F5E80(void) {
    s32 unused[8];
    s32 v;

    if (*(s8 *)&gFellOff == 0) {
        gCourse.rot.vy = UpdateCamera(gPlayer.x, gPlayer.y, gPlayer.z, gFloorY[0]) + 0x800;
    }
    gPlayer.editorY = gPlayer.groundY;
    v = FindFloorNear(&gPlayer.x, gFloorY, gFloorIndex);
    if (v != -1) {
        gFloorIndex = v;
        gStageParams = *(s32 *)&gFloorY[4];
    }
    gPlayer.groundY = gFloorY[0];
    SetSpadStack();
    UpdatePlayer();
    UpdateStageLights();
    if (*(s8 *)&gFellOff == 0) {
        PushPlayerFromWalls();
    }
    DrawModeObject();
    DrawPlayerShadow();
    TestBoxes();
    PairMarks();
    ResetCollision();
    UpdateStageEvents();
    UpdateStageTraps();
    UpdateStageSprites();
    ReactToHits(&gPlayer);
    if (*(s8 *)&gPlayLive == 1) {
        UpdatePickups(&gSubOt[gActiveBuff]);
    }
    LeaveQuadTop();
    UpdatePlayerAction(&gPlayer);
    ResetSpadStack();
    DrawTodObjects();
    UnusedHook2();
    PopInNextCan();
    if (*(s8 *)&gPlayLive == 1) {
        DrawStage(-gPlayer.x, 0, -gPlayer.z);
        DrawSky(gStageParams);
    }
    UpdateEffects((s32)&gSubOt[gActiveBuff], -gPlayer.x, 0, -gPlayer.z);
    func_800F8BD4();
}

INCLUDE_ASM("asm/ovl/ovl16/nonmatchings/ovl16", func_800F60CC);

void func_800F6FA0(void) {
    s16 v;

    if (gPlayer.control != 0 && (u32)(gPlayer.motion[0] - MOT_RIDE) < 5) {
        v = gPlayer.speed;
        if (v < 100) {
            gSpeedFixed += 16;
        } else if (gPad & 0x1000) { /* PADLup */
            gSpeedFixed += 32;
            if ((s16)gSpeedFixed >> 4 > 100) {
                gSpeedFixed = 0x640;
            }
        } else if (gPad & 0x4000) { /* PADLdown */
            gSpeedFixed -= 32;
            if ((s16)gSpeedFixed >> 4 < 100) {
                gSpeedFixed = 0x640;
            }
        } else if (v > 100) {
            gSpeedFixed -= 16;
        } else {
            gSpeedFixed += 16;
        }
        gPlayer.speed = (s16)gSpeedFixed >> 4;
    }
}

/* MATCHING: player's, declared only where defined. */
void OnJumpStart(PlayerState *g);
void ScatterDust(VECTOR *pos);

/* MATCHING: player's, declared only where defined. */
void OnJumpStart(PlayerState *g);
void ScatterDust(VECTOR *pos);

void func_800F70A8(void) {
    s16 n;

    if (gPlayer.control != 0) {
        if ((gGamePadTrig & 0x20) &&
            (gPlayer.y >= gPlayer.groundY || (gPlayer.onRideQuad == 1 && gPlayer.onQuadTop == 1)) &&
            gPlayer.jumpState == JUMP_NONE) {
            gPlayer.airborne = 1;
            gPlayer.onQuadTop = 0;
            gPlayer.jumpState = JUMP_RISING;
            gPlayer.jumpTime = 0;
            gPlayer.peakY = 0;
            OnJumpStart(&gPlayer);
            gPlayer.motion[0] = MOT_RIDE_JUMP;
            gPlayer.jumpPower = 0x23;
            gPlayer.jumpStartY = gPlayer.y;
            ScatterDust((VECTOR *)&gPlayer.x);
            PlaySe(0x17);
        }
        if (gPlayer.jumpState == JUMP_RISING) {
            n = ++gPlayer.jumpTime;
            if (n < 3) {
                gPlayer.jumpPower = 0x21;
            } else if ((u16)(n - 4) < 3) {
                gPlayer.jumpPower = 0x1A;
            } else if (n >= 9) {
                gPlayer.jumpPower = 0x1C;
                gPlayer.jumpState = JUMP_RELEASED;
                gPlayer.motion[0] = MOT_RIDE_JUMP;
                gPlayer.jumpHeight = 2;
            } else if (gGamePad & 0x20) {
                gPlayer.jumpPower = 0x1C;
            } else {
                gPlayer.jumpState = JUMP_RELEASED;
                switch (n) {
                    case 3:
                        gPlayer.motion[0] = MOT_RIDE_JUMP_2;
                        gPlayer.jumpHeight = 0;
                        break;
                    case 7:
                        gPlayer.motion[0] = MOT_RIDE_JUMP_2;
                        gPlayer.jumpHeight = 1;
                        break;
                    default:
                        gPlayer.motion[0] = MOT_RIDE_JUMP_2;
                        gPlayer.jumpHeight = 2;
                        break;
                }
            }
        }
        if (gPlayer.jumpState == JUMP_RELEASED) {
            gPlayer.jumpState = JUMP_NONE;
        }
    }
}

extern s16 sSteerDrift;
/* MATCHING: player's, declared only where defined. */
void ApplySteer(u8 mode, u16 a, s16 b);
void RecentreSteer(void);

extern s16 sSteerDrift;
/* MATCHING: player's, declared only where defined. */
void ApplySteer(u8 mode, u16 a, s16 b);
void RecentreSteer(void);

void func_800F729C(void) {
    if (gPad & 0x8000) { /* PADLleft */
        ApplySteer(3, 16, 15);
        sSteerDrift -= 3;
        gPlayer.action = 0x35;
    } else if (gPad & 0x2000) { /* PADLright */
        ApplySteer(2, 16, 15);
        sSteerDrift += 3;
        gPlayer.action = 0x37;
    } else {
        RecentreSteer();
        gPlayer.action = 0x33;
        if (sSteerDrift != 0) {
            if (sSteerDrift < 0) {
                sSteerDrift++;
            } else {
                sSteerDrift--;
            }
        }
    }
    if (gPadType == 0) {
        RecentreSteer();
    }
    sSteerDrift = sSteerDrift < -50 ? -50 : sSteerDrift > 50 ? 50 : sSteerDrift;
}

#include "engine_code.inc" /* UpdateStageEvents to RunTrapType02 */
/* MATCHING: the dispatchers call these stage entry points, which in this
 * overlay land inside the functions above. */
FUNC_ALIAS(func_800F74FC, StartSpriteWalk, 0x3C);

INCLUDE_ASM("asm/ovl/ovl16/nonmatchings/ovl16", RunTrapType04);

#include "engine_code.inc" /* RunTrapType09 to RunTrapType0A */
/* MATCHING: the dispatchers call these stage entry points, which in this
 * overlay land inside the functions above. */
FUNC_ALIAS(func_800F7968, RunTrapType0A, 0x58);
FUNC_ALIAS(func_800F79A0, RunTrapType0A, 0x90);

INCLUDE_ASM("asm/ovl/ovl16/nonmatchings/ovl16", RunTrapType0B);

#include "engine_code.inc" /* RunTrapType32 */

void RunTrapThenTurnTimed(StageTrap *t, u16 speed, s16 arg, s8 dir) {}

#include "engine_code.inc" /* RunTrapType0F to RunTrapType2F */

void StepTrapType0F(StageTrap *t, u16 speed, s8 dir, s32 arg) {}

void StepTrapType12(StageTrap *t, u16 speed, s32 arg) {}

void StepTrapType1F(StageTrap *t, u16 arg) {}

#include "engine_code.inc" /* StepSolidTrap */

void StepTrapFFAct67(StageTrap *t) {}

void StepTrapFFAct68(StageTrap *t) {}

void StepTrapAct21(StageTrap *t) {}

void StepTrapAct22(StageTrap *t) {}

void StepTrapAct23(StageTrap *t) {}

void StepTrapAct24(StageTrap *t) {}

void StepTrapAct25(StageTrap *t, s32 arg) {
    u8 unused[0x78];

    if (gSection - UpdatePathSegment(t) < 2) {
        UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
        if (t->touched && gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
            PlaySe(0x37);
        }
    }
}
/* MATCHING: the dispatchers call this stage entry point, which in this
 * overlay lands inside the function above. */
FUNC_ALIAS(func_800F82E0, StepTrapAct25, 0x78);

void StepTrapAct27(StageTrap *t) {}

void StepTrapAct28(StageTrap *t) {}

#include "engine_code.inc" /* ArmTrapByRange */

void RunTrapAhead(StageTrap *t, s16 speed) {}

void RunTrapAheadNonstop(StageTrap *t, s16 speed) {}

void RunTrapAlongPath(StageTrap *t, u16 speed, s8 turn) {}

void StepChargingTrap(StageTrap *t, s16 speed, s32 range) {}

void RunTrapThenTurn(StageTrap *t, u16 speed, s32 range, s8 dir) {}

#include "engine/draw_trap_warning.inc"

void StepTrapType0D(StageTrap *t, u16 speed, s32 arg, s8 dir) {}

void StepTrapType30(StageTrap *t, u16 speed, s32 arg) {}

void StepTrapType18(StageTrap *t, s32 speed, s32 arg, s8 dir) {}

void StepTrapType1A(StageTrap *t, u16 speed, s32 arg, s8 dir) {}

void StepTrapType1C(StageTrap *t, s32 speed, s32 arg, s32 arg2) {}

void StepTrapType1E(StageTrap *t, u16 speed) {}

void StepTrapType2A(StageTrap *t, s32 speed, s32 arg) {}

void StepTrapType31(StageTrap *t, s32 arg) {}

#include "engine_code.inc" /* RunTrapType13 to RunTrapType17 */
/* MATCHING: the dispatchers call these stage entry points, which in this
 * overlay land inside the functions above. */
FUNC_ALIAS(func_800F8454, RunTrapType13, 0x38);
FUNC_ALIAS(func_800F84A8, RunTrapType13, 0x8C);

void StepTrapType1B(StageTrap *t, u16 speed, s32 arg) {}

void StepTrapType1D(StageTrap *t, u16 speed) {}

void RunTrapAheadAtHeight(StageTrap *t, u16 speed) {}

#include "engine_code.inc" /* RunTrapType22 to RunTrapType2E */
/* MATCHING: the dispatchers call these stage entry points, which in this
 * overlay land inside the functions above. */
FUNC_ALIAS(func_800F87E4, RunTrapType2C, 0x24);

#include "engine_code.inc" /* BurstTrapStacked */

void SlideTrapAway(StageTrap *t, s32 arg) {}

#include "engine/tumble_trap_act08.inc"

#include "engine/tumble_trap_act09.inc"

#include "engine/tumble_trap_act0_b.inc"

#include "engine/knock_trap_away_act0_c.inc"

void StepDashBoard(StageTrap *t) {}

#include "engine/tumble_trap_act12.inc"

void StepSprayTrap(StageTrap *t, s8 dir) {}

void StepSmokingTumbleTrap(StageTrap *t) {}

void StepTrapAct1D(StageTrap *t) {}

void StepSmokingTrap(StageTrap *t) {}

void TurnTrapOnApproach(StageTrap *t, s32 speed, s8 arg) {}

void OpenGateOnApproach(StageTrap *t, s32 speed, s8 arg) {}

void SwingGateContinuously(StageTrap *t, s8 arg) {}

void ShiftTrapOnApproach(StageTrap *t, s8 dir, s32 speed) {}

void StepHoppingTrap(StageTrap *t, u8 act) {}

#include "engine/draw_trap_while_linked.inc"

void UpdateStageEventHook(void) {
    StageTrap *m;

    m = &D_800D39D8[1];
    if (gSwitches[192].state == SWITCH_ON) {
        m->state = 0;
        gPlayer.motion[0] = 11;
        gPlayer.jumpPower = 60;
        gPlayer.action = 0;
        gSwitches[192].state = SWITCH_OFF;
        gPlayer.dropRide = 1;
        m->timer = 0;
        m->state = 1;
        gMissState = 0;
        gPlayer.hittable = 0;
        gPlayer.invulnTimer = 30;
        gPlayer.rideMode &= 0xF0;
        PlaySe(0x38);
    }
}

#include "engine_code.inc" /* UpdateGoalEvents */

void UpdateGoalEventHook(void) {}

void func_800F8B4C(DirEnt16 *dir, u16 first, u16 count) {
    DirEnt16 *e;
    u16 i;

    e = dir;
    e += first;
    for (i = first; i < first + count; i++) {
        LoadTim((u8 *)dir + e->offset);
        e++;
        DrawSync(0);
    }
}

void func_800F8BD4(void) {
    RECT r;

    r.x = gFrameCount / 2 % 12 * 16;
    r.y = 0x1F3;
    r.w = 16;
    r.h = 1;
    MoveImage(&r, 0, 0x1E0);
}

#include "engine/reveal_start_cans.inc"

INCLUDE_ASM("asm/ovl/ovl16/nonmatchings/ovl16", EmitTrapParticles);

INCLUDE_ASM("asm/ovl/ovl16/nonmatchings/ovl16", func_800F8D34);

#ifdef NON_MATCHING
void func_800F90F4(void) {
    s16 yaw;

    gCameraYaw = 0;
    sCamHeight = 0;
    sViewSpeed = 100;
    gViewerAngle = 0xD8;
    gFellOff = 0;
    gPlayLive = 1;
    sCamOffset[0] = 0;
    gView[1] = 0;
    sCamOffset[1] = 0;
    yaw = -gStageStart->yaw;
    gPlayer.motion[0] = 2;
    gPlayer.control = 1;
    gPlayer.hittable = 1;
    gCourse.rot.vy = yaw;
    gCourse.rot.vz = yaw;
    gPlayer.startYaw = gPlayer.yaw = yaw;
    gFloorIndex = 0;
    gCourse.x = gPlayer.x = -gStageStart->x;
    gSteerYaw = yaw;
    gCourse.y = gPlayer.y = -gStageStart->y;
    gSteerYawHalf = yaw;
    gCourse.z = gPlayer.z = -gStageStart->z;
    gPlayer.drawX = 0;
    gPlayer.drawY = 0;
    gPlayer.drawZ = 0;
    gBodyRot[0] = 0;
    gBodyRot[1] = 0;
    gBodyRot[2] = 0;
    gView[1] = (s16)(gCourse.y - 500);
    RevealStartCans();
}
#else
INCLUDE_ASM("asm/ovl/ovl16/nonmatchings/ovl16", func_800F90F4);
#endif

/* MATCHING: ret set before the table copy and again after the call, so it
 * stays a register; set once it folds to a constant. */
u16 func_800F922C(s16 id, StageTrap *src, StageTrap *dst) {
    u16 ret = 1;
    u8 models[80] = {0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15,
                     16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
                     32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
                     48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
                     64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79};

    if (id < 80) {
        if (id >= 0) {
            dst->homeX = src->x;
            dst->homeY = src->y;
            dst->homeZ = src->z;
            dst->homeRotX = src->rotX;
            dst->homeRotY = src->rotY;
            dst->homeRotZ = src->rotZ;
            ResetObj34(dst);
            dst->model = models[id];
            ret = 1;
        }
    }
    return ret;
}

/** @brief The shape of each of this stage's own sprites, as one block. */
typedef struct {
    u8 shape[41]; /**< a shape per sprite id from SPR_STAGE on */
} StageShapes;

extern const StageShapes D_800F08F0;

void func_800F936C(void) {
    StageShapes shapes;

    shapes = D_800F08F0;
    *(StageShapes *)sSpriteShapes = shapes;
}

INCLUDE_RODATA("asm/ovl/ovl16/nonmatchings/ovl16", D_800F08F0);
