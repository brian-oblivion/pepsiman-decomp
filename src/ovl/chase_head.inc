/* The act-3 chase stages' shared functions that follow StartChaseJump:
 * the player's step, its model and shadow draws, and the HUD. */

void StartChaseJump(void);

/* The frame step's state, defined by chase_data.inc. MATCHING: cc1 writes
 * a unit's tentative definitions at its end in the order of their FIRST
 * declarations, so the zeroed ones are declared here in .bss order. */
extern s32 sObjCount;
extern Act3Step sPieceSteps[];
extern CVECTOR sChaseSky[4];
extern s32 sChaseUnused[3];
extern MATRIX sChaseMatrix;
extern s32 sCaughtTimer;
extern s32 sEscapeTimer;
extern s32 sSceneTimer;
extern POLY_F4 *sFadePoly;
extern DR_MODE *sFadeMode;

/* The player's step in the chase: gravity, the jump, the hit and the
 * sidestep. */
/* MATCHING: non-void with no return keeps six delay slots nops. */
s32 StepChasePlayer(void) {
    if (sHurtTimer != 0) {
        sHurtTimer--;
    }
    gPlayer.y += sJumpSpeed;
    sJumpSpeed += 5;
    if (gPlayer.y >= 0) {
        gPlayer.y = 0;
        sJumpSpeed = 5;
        gPlayer.airborne = 0;
    } else {
        gPlayer.airborne = 1;
    }
    switch ((s8)gPlayer.slideState) {
        case 0:
            gPlayer.speed = 30;
            gPlayer.motion[0] = 0x33;
            gPlayer.action = ACT_MODEL_LOOP;
            StartChaseJump();
            if (ClipF == 0) {
                if (gPad & PADLleft) {
                    gPlayer.z -= 15;
                    if (gPlayer.z < -200) {
                        gPlayer.z = -200;
                    }
                } else if (gPad & PADLright) {
                    gPlayer.z += 15;
                    if (gPlayer.z > 200) {
                        gPlayer.z = 200;
                    }
                }
            }
            break;
        case 1:
            gPlayer.speed = 30;
            gPlayer.motion[0] = 0x34;
            gPlayer.action = ACT_MODEL_ONCE;
            if (gPlayer.airborne == 0) {
                gPlayer.slideState = 0;
                gPlayer.unk38C = 0;
                gPlayer.curMotion[0] = MOT_OFF;
            }
            if (ClipF == 0) {
                if (gPad & PADLleft) {
                    gPlayer.z -= 15;
                    if (gPlayer.z < -200) {
                        gPlayer.z = -200;
                    }
                } else if (gPad & PADLright) {
                    gPlayer.z += 15;
                    if (gPlayer.z > 200) {
                        gPlayer.z = 200;
                    }
                }
            }
            break;
        case 2:
            gPlayer.speed = 24;
            gPlayer.motion[0] = 0x35;
            gPlayer.action = ACT_MODEL_ONCE;
            if (++gPlayer.unk38C > 32) {
                gPlayer.slideState = 0;
                gPlayer.unk38C = 0;
                gPlayer.curMotion[0] = MOT_OFF;
            }
            break;
        case 3:
            gPlayer.motion[0] = 0x36;
            gPlayer.speed = 0;
            gPlayer.action = ACT_MODEL_ONCE;
            if (++gPlayer.unk38C > 16) {
                sChaseLive = 0;
            }
            break;
    }
}

/* Draws the player's model at its position and yaw. */
void DrawChasePlayer(void) {
    MATRIX m;
    SVECTOR rot;
    GsDOBJ2 *o;
    s32 i;

    o = ((DrawList *)gTodObjTable)->objs;
    o->coord2->coord.t[0] = gPlayer.x;
    o->coord2->coord.t[1] = gPlayer.y;
    o->coord2->coord.t[2] = gPlayer.z;
    rot.vx = 0;
    rot.vz = 0;
    rot.vy = gPlayer.yaw;
    SetCoordRotation(&rot, o->coord2);
    for (i = 0; i < ((DrawList *)gTodObjTable)->count; o++, i++) {
        o->coord2->flg = 0;
        if (o->id != -1 && o->tmd != NULL) {
            GsGetLs(o->coord2, &m);
            GsSetLsMatrix(&m);
            GsGetLw(o->coord2, &m);
            GsSetLightMatrix(&m);
            GsSortObject4J(o, &gSubOt[gActiveBuff], 2, (u_long *)0x1F800000);
        }
    }
}

/* Draws the player's model at the run's centre, its depth scaled by @p sz;
 * every other pair of frames while it flashes after a hit. */
void DrawChasePlayerScaled(s32 sz) {
    MATRIX m;
    SVECTOR rot;
    VECTOR scale;
    GsDOBJ2 *o;
    s32 i;

    if (sHurtTimer / 2 % 2 != 1) {
        o = ((DrawList *)gTodObjTable)->objs;
        o->coord2->coord.t[0] = 0;
        o->coord2->coord.t[1] = gPlayer.y;
        o->coord2->coord.t[2] = 0;
        rot.vx = 0;
        rot.vz = 0;
        rot.vy = gPlayer.yaw;
        SetCoordRotation(&rot, o->coord2);
        for (i = 0; i < ((DrawList *)gTodObjTable)->count; o++, i++) {
            o->coord2->flg = 0;
            if (o->id != -1 && o->tmd != NULL) {
                scale.vx = 0x1000;
                scale.vy = 0x1000;
                scale.vz = sz;
                GsGetLs(o->coord2, &m);
                ScaleMatrix(&m, &scale);
                GsSetLsMatrix(&m);
                GsGetLw(o->coord2, &m);
                GsSetLightMatrix(&m);
                GsSortObject4J(o, &gSubOt[gActiveBuff], 2, (u_long *)0x1F800000);
            }
        }
    }
}

/* Draws the player's shadow at the run's centre, smaller and fainter the
 * higher the player is. */
void DrawChaseShadowCentre(void) {
    GsCOORDINATE2 c;
    MATRIX m;
    SVECTOR size;
    SVECTOR unused;
    SColor color;
    s32 d;

    GsInitCoordinate2(WORLD, &c);
    c.coord.t[0] = 0;
    c.coord.t[1] = 0;
    c.coord.t[2] = 0;
    GsGetLs(&c, &m);
    GsSetLsMatrix(&m);
    color.r = BLEND_SUB;
    d = -gPlayer.y / 10;
    size.vx = 100 - d;
    size.vy = size.vx / 2;
    color.g = color.b = color.cd = 0x40 - d;
    DrawBillboard(0x12D, &size, (CVECTOR *)&color, 2, &gSubOt[gActiveBuff]);
}

/* The same at the player's position. */
void DrawChaseShadow(void) {
    GsCOORDINATE2 c;
    MATRIX m;
    SVECTOR size;
    SVECTOR unused;
    SColor color;
    s32 d;

    GsInitCoordinate2(WORLD, &c);
    c.coord.t[1] = 0;
    c.coord.t[0] = gPlayer.x;
    c.coord.t[2] = gPlayer.z;
    GsGetLs(&c, &m);
    GsSetLsMatrix(&m);
    color.r = BLEND_SUB;
    d = -gPlayer.y / 10;
    size.vx = 100 - d;
    size.vy = size.vx / 2;
    color.g = color.b = color.cd = 0x40 - d;
    DrawBillboard(0x12D, &size, (CVECTOR *)&color, 2, &gSubOt[gActiveBuff]);
}

/* Draws the retry count, its label, and the run's progress bar. */
void DrawChaseHud(void) {
    SVECTOR pos;
    SColor color;
    s32 n;
    s32 div;

    n = gRetries;
    pos.vx = 0x68;
    pos.vy = 0x4C;
    for (div = 1; div < 10001; div *= 10) {
        if (n / div != 0) {
            DrawSprite(n / div % 10 + 0x105, &pos, NULL, 0, &gSubOt[gActiveBuff]);
            pos.vx -= 16;
        }
    }
    color.r = BLEND_OPAQUE;
    color.g = color.b = color.cd = 0x80;
    pos.vx = 0x70;
    pos.vy = 0x38;
    DrawSprite(0x135, &pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
    n = gPlayer.x;
    if (n < 0) {
        n = 0;
    }
    color.r = BLEND_ADD;
    color.g = color.b = color.cd = 0x80;
    pos.vy = -0x69;
    pos.vx = n * 0x5800 / CHASE_LENGTH + 0x34;
    DrawSprite((gFrameCount >> 2 & 3) + 0x12F, &pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
    pos.vx = 0x34;
    pos.vy = -100;
    DrawSprite(0x12E, &pos, NULL, 0, &gSubOt[gActiveBuff]);
}
