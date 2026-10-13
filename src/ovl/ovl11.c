/* Stage 3-3's overlay: the act-3 chase of world 3, built on chase.inc and
 * chase_tail.inc with its own player step and draws, and the chase pieces
 * without their sounds or the hurt they do in the other chases. */

#define CHASE_PLACEMENTS 100
#define CHASE_LIGHT_RG 0xFF
#define CHASE_LIGHT_BLUE 0xFF
#define CHASE_FAR_COLOR 0xB0, 0xB0, 0xB0
#define CHASE_REACH 120
/* MATCHING: the loader's jump table sits 4 bytes past an 8-byte boundary;
 * this toolchain aligns jump tables to 8, so the loader stays asm. */
#define CHASE_LOADER INCLUDE_ASM("asm/ovl/ovl11/nonmatchings/ovl11", LoadChaseStage);

#include "chase.inc"
#include "rand.h"
#include "spad.h"
#include "player.h"

#define ABS(x) ((x) < 0 ? -(x) : (x))


/* MATCHING: player's routines, declared only where defined; this unit's
 * ApplySteer view takes the step narrowed to s16, as retail passes it. */
s32 UpdateSpeedBand(void);
s32 ApplyGravity(void);
void UpdateJump(void);
s32 TrySlide(void);
void ApplySteer(s32 mode, s16 a, s32 b);
void RecentreSteer(void);

/* Steps the player: speed, gravity, jump and slide, the move along the
 * course, analog or digital steering held within the course heading, and
 * the model's placement. */
#ifdef NON_MATCHING
void func_800F0234(void) {
    PlayerState *g;
    s32 x;
    s32 v;
    s32 hi;
    u8 locked;
    u32 a;
    u32 stick;

    locked = 0;
    UpdateSpeedBand();
    ApplyGravity();
    if (gPlayer.motion[0] >= 2 && gPlayer.motion[0] <= 4) {
        if (gPlayer.speed >= 30 || (gPlayer.speed += 5) >= 30) {
            gPlayer.speed = 30;
            gPlayer.motion[0] = 4;
        }
    }
    UpdateJump();
    TrySlide();
    gPlayer.unk344 = (90 - gPlayer.speed) / 3 + 1;
    gPlayer.stepX = rsin(gCourse.rot.vy) * gPlayer.speed / 4096;
    gPlayer.stepZ = rcos(gCourse.rot.vy) * gPlayer.speed / 4096;
    gPlayer.x -= gPlayer.stepX;
    gPlayer.z -= gPlayer.stepZ;
    if (gPlayer.control != 0) {
        switch (gPadType) {
            case PAD_TYPE_ANALOG:
                stick = gStickRawX;
                if (stick > 0x90 || (gPad & PADLright)) {
                    if (gPad & PADLright) {
                        a = 16;
                    } else {
                        a = stick - 0x80;
                        if ((s32)a < 0) {
                            a = -a;
                        }
                        a >>= 2;
                    }
                    ApplySteer(3, a, 15);
                } else if (stick < 0x70 || (gPad & PADLleft)) {
                    if (gPad & PADLleft) {
                        a = 16;
                    } else {
                        a = stick;
                        if ((s32)a < 0) {
                            a = -a;
                        }
                        a = (0x80 - a) >> 2;
                    }
                    ApplySteer(2, a, 15);
                } else {
                    RecentreSteer();
                }
                break;
            case PAD_TYPE_DIGITAL:
                if ((gPad & PADLright) && !locked) {
                    ApplySteer(3, 16, 15);
                } else if ((gPad & PADLleft) && !locked) {
                    ApplySteer(2, 16, 15);
                } else {
                    RecentreSteer();
                }
                break;
        }
        if (gPlayer.control != 0) {
            goto clamp;
        }
    }
    gPlayer.yaw = gCourse.rot.vy;
clamp:
    g = &gPlayer;
    x = g->yaw;
    v = gCourse.rot.vy - 0x155;
    if (x >= v) {
        hi = gCourse.rot.vy + 0x155;
        if (x <= hi) {
            v = x;
        } else {
            v = hi;
        }
    }
    g->yaw = v;
    gBodyRot[1] = gPlayer.yaw;
    gBodyRot[2] = gPlayer.lean;
    gTodCoords[0].coord.t[0] = gPlayer.drawX;
    gTodCoords[0].coord.t[1] = gPlayer.y + gPlayer.drawY;
    gTodCoords[0].coord.t[2] = gPlayer.drawZ;
    SetCoordRotation((SVECTOR *)gBodyRot, gTodCoords);
    gWorldShift[1] = 0;
    gWorldShift[0] = -gPlayer.x;
    gWorldShift[2] = -gPlayer.z;
}
#else
INCLUDE_ASM("asm/ovl/ovl11/nonmatchings/ovl11", func_800F0234);
#endif

/* Draws the player's models at its position, sorted on the scratchpad
 * stack. */
void func_800F0590(void) {
    MATRIX m;
    GsDOBJ2 *o;
    s32 i;

    o = ((DrawList *)gTodObjTable)->objs;
    o->coord2->coord.t[0] = gPlayer.x;
    o->coord2->coord.t[1] = gPlayer.y;
    o->coord2->coord.t[2] = gPlayer.z;
    for (i = 0; i < ((DrawList *)gTodObjTable)->count; o++, i++) {
        o->coord2->flg = 0;
        if (o->id != -1 && o->tmd != NULL) {
            GsGetLs(o->coord2, &m);
            GsSetLsMatrix(&m);
            GsGetLw(o->coord2, &m);
            GsSetLightMatrix(&m);
            /* MATCHING: retail runs this sort on the scratchpad stack. */
            SetSpadStack();
            GsSortObject4(o, &gSubOt[gActiveBuff], 2, (u_long *)0x1F800000);
            ResetSpadStack();
        }
    }
}

/* Draws the player's shadow under it, smaller and fainter the higher it
 * is above the ground. */
void func_800F06E0(void) {
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
    d = (gPlayer.groundY - gPlayer.y) / 10;
    size.vx = 100 - d;
    size.vy = size.vx / 2;
    color.g = color.b = color.cd = 0x40 - d;
    DrawBillboard(0x12D, &size, (CVECTOR *)&color, 2, &gSubOt[gActiveBuff]);
}

/* MATCHING: draw takes a Sprite2D *, a type local to that unit; this
 * unit passes the same eight halfwords as an array. */
void DrawSpriteTransformed(u16 id, s16 *quad, CVECTOR *color, u16 otz, GsOT *ot);

/* Draws the stage's title: one sprite fading in, a second dropping into
 * place, then the second growing and fading out. */
void func_800F07E4(void) {
    s16 quad[8];
    SColor color;
    s32 d;
    s32 t;

    if (D_800D39D8.title < 68) {
        if (D_800D39D8.title < 30) {
            quad[0] = -128;
            quad[1] = -32;
            color.r = 1;
            /* MATCHING: a store in each arm keeps the stores above the call's
             * arguments. */
            if (D_800D39D8.title < 16) {
                color.g = color.b = color.cd = D_800D39D8.title * 8;
            } else {
                color.g = color.b = color.cd = 0x80;
            }
            DrawSprite(30, (SVECTOR *)quad, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
        }
        if (D_800D39D8.title >= 30 && D_800D39D8.title < 60) {
            quad[0] = -128;
            d = D_800D39D8.title - 30;
            quad[1] = d < 9 ? d * 22 - 208 : -32;
            color.r = 1;
            color.g = color.b = color.cd = 0x80;
            DrawSprite(31, (SVECTOR *)quad, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
        }
        if (D_800D39D8.title >= 60) {
            t = D_800D39D8.title - 60;
            quad[4] = quad[5] = (t << 9) + 0x1000;
            quad[7] = 1;
            color.r = 1;
            quad[2] = 0;
            quad[0] = 0;
            quad[3] = 0;
            quad[1] = 0;
            quad[6] = 0;
            color.g = color.b = color.cd = -128 - t * 16;
            DrawSpriteTransformed(31, quad, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
        }
        D_800D39D8.title++;
    }
}

/** @brief A flat line packet with its colour and code as one word. */
typedef struct {
    u_long tag;  /**< the link and length */
    u_long rgbc; /**< the colour, then the code in the top byte */
    s16 x0;      /**< start x */
    s16 y0;      /**< start y */
    s16 x1;      /**< end x */
    s16 y1;      /**< end y */
} LineF2;

extern SwitchBox gSwitches[];

/** @brief One row of the closing wipe, kept in the switch table's memory. */
typedef struct {
    s16 x;     /**< how far the row's black line reaches */
    s16 speed; /**< how fast it grows */
    u8 pad[4]; /**< unused */
} WipeRow;

/* The packet being built, in the packet buffer. */
#define sLine ((LineF2 *)GsOUT_PACKET_P)
/* The wipe's rows. */
#define sWipe ((WipeRow *)gSwitches)

/* Draws the stage's closing caption, two sprites sliding in, then wipes
 * the screen with 240 black lines, each growing at its own speed. */
#ifdef NON_MATCHING
void func_800F09D8(void) {
    s16 quad[8];
    SColor color;
    s16 t;
    s32 i;
    WipeRow *w;

    if ((u16)D_800D39D8.title - 60 < 8U) {
        t = D_800D39D8.title - 60;
        quad[0] = (t << 5) - 0x180;
        quad[1] = -64;
        color.r = 1;
        color.g = color.b = color.cd = t * 16;
        DrawSprite(32, (SVECTOR *)quad, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
    }
    if ((u16)D_800D39D8.title - 68 < 8U) {
        quad[0] = -128;
        quad[1] = -64;
        color.r = 1;
        color.g = color.b = color.cd = 0x80;
        DrawSprite(32, (SVECTOR *)quad, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
        quad[1] = 0;
        color.r = 1;
        t = D_800D39D8.title - 68;
        quad[0] = (t << 5) - 0x180;
        color.g = color.b = color.cd = t * 16;
        DrawSprite(33, (SVECTOR *)quad, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
    }
    if (D_800D39D8.title >= 76) {
        quad[0] = -128;
        quad[1] = -64;
        color.r = 1;
        color.g = color.b = color.cd = 0x80;
        DrawSprite(32, (SVECTOR *)quad, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
        quad[0] = -128;
        quad[1] = 0;
        color.r = 1;
        color.g = color.b = color.cd = 0x80;
        DrawSprite(33, (SVECTOR *)quad, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
    }
    if (D_800D39D8.title >= 150) {
        w = sWipe;
        for (i = 0; i < 240; i++, w++) {
            setlen(sLine, 3);
            sLine->rgbc = 0;
            setcode(sLine, 0x40);
            sLine->x0 = -160;
            sLine->y0 = sLine->y1 = i - 120;
            sLine->x1 = w->x;
            w->x += w->speed;
            if (w->x >= 160) {
                w->x = 160;
            }
            addPrim(gSubOt[gActiveBuff].org, sLine);
            GsOUT_PACKET_P += sizeof(LineF2);
        }
    }
    D_800D39D8.title++;
}
#else
INCLUDE_ASM("asm/ovl/ovl11/nonmatchings/ovl11", func_800F09D8);
#endif

s32 StepChasePiece0(Act3Obj *o) {
    s32 unused[2];
    MATRIX m;
    s32 dir;
    s32 y;
    s32 z;
    s32 dz;

    if (o->x <= gPlayer.x + 2000 && o->x >= gPlayer.x - 10000) {
        switch (o->state) {
            case 0:
                GsInitCoordinate2(WORLD, &sPieceCoords[10]);
                sPieceCoords[10].coord.t[0] = o->x + gWorldShift[0];
                sPieceCoords[10].coord.t[1] = o->y + gWorldShift[1];
                sPieceCoords[10].coord.t[2] = o->z + gWorldShift[2];
                sPieceCoords[10].flg = 0;
                GsGetLs(&sPieceCoords[10], &m);
                GsSetLsMatrix(&m);
                GsSortObject4(&sPieceObjs[10], &gSubOt[gActiveBuff], 2, (u_long *)0x1F800000);
                if (o->x < sCan.x + CHASE_REACH) {
                    o->state = 1;
                    o->timer = 0;
                    o->speed = 45;
                }
                if (o->x - 20 < gPlayer.x && gPlayer.x < o->x + 20 && o->z - 80 < gPlayer.z &&
                    gPlayer.z < o->z + 80 && o->y - 80 < gPlayer.y) {
                    gPlayer.action = 9;
                    EndJump();
                    o->state = 1;
                    o->timer = 0;
                    o->speed = gPlayer.speed * 3 / 2;
                }
                break;
            case 1:
                GsInitCoordinate2(WORLD, &sPieceCoords[10]);
                dir = 1;
                if (o->z < 0) {
                    dir = -1;
                }
                sPieceCoords[10].coord.t[0] = o->x + o->timer * o->speed + gWorldShift[0];
                y = o->y - rsin(o->timer * 40960 / 360) * 200 / 4096 + gWorldShift[1];
                sPieceCoords[10].coord.t[1] = y;
                dz = dir * 25;
                z = o->z + o->timer * dz + gWorldShift[2];
                sPieceCoords[10].coord.t[2] = z;
                sPieceCoords[10].flg = 0;
                if ((o->timer & 3) == 0) {
                    SpawnEffect(4, sPieceCoords[10].coord.t[0] - gWorldShift[0], y - gWorldShift[1],
                                z - gWorldShift[2], 0);
                }
                sPieceRots[10].vx = o->timer * 40960 / 360 * dir;
                sPieceRots[10].vy = 0;
                sPieceRots[10].vz = 0;
                SetCoordRotation(&sPieceRots[10], &sPieceCoords[10]);
                GsGetLs(&sPieceCoords[10], &m);
                GsSetLsMatrix(&m);
                GsSortObject4(&sPieceObjs[10], &gSubOt[gActiveBuff], 2, (u_long *)0x1F800000);
                if (++o->timer >= 16) {
                    o->state = -1;
                }
                break;
        }
    }
}

/* The same for piece model 1. */
s32 StepChasePiece1(Act3Obj *o) {
    s32 unused[2];
    MATRIX m;
    s32 dir;
    s32 y;
    s32 z;
    s32 dz;

    if (o->x <= gPlayer.x + 2000 && o->x >= gPlayer.x - 10000) {
        switch (o->state) {
            case 0:
                GsInitCoordinate2(WORLD, &sPieceCoords[11]);
                sPieceCoords[11].coord.t[0] = o->x + gWorldShift[0];
                sPieceCoords[11].coord.t[1] = o->y + gWorldShift[1];
                sPieceCoords[11].coord.t[2] = o->z + gWorldShift[2];
                sPieceCoords[11].flg = 0;
                GsGetLs(&sPieceCoords[11], &m);
                GsSetLsMatrix(&m);
                GsSortObject4(&sPieceObjs[11], &gSubOt[gActiveBuff], 2, (u_long *)0x1F800000);
                if (o->x < sCan.x + CHASE_REACH) {
                    o->state = 1;
                    o->timer = 0;
                    o->speed = 45;
                }
                if (o->x - 20 < gPlayer.x && gPlayer.x < o->x + 20 && o->z - 60 < gPlayer.z &&
                    gPlayer.z < o->z + 60 && o->y - 120 < gPlayer.y) {
                    gPlayer.action = 9;
                    EndJump();
                    o->state = 1;
                    o->timer = 0;
                    o->speed = gPlayer.speed * 3 / 2;
                }
                break;
            case 1:
                GsInitCoordinate2(WORLD, &sPieceCoords[11]);
                dir = 1;
                if (o->z < 0) {
                    dir = -1;
                }
                sPieceCoords[11].coord.t[0] = o->x + o->timer * o->speed + gWorldShift[0];
                y = o->y - rsin(o->timer * 40960 / 360) * 200 / 4096 + gWorldShift[1];
                sPieceCoords[11].coord.t[1] = y;
                dz = dir * 25;
                z = o->z + o->timer * dz + gWorldShift[2];
                sPieceCoords[11].coord.t[2] = z;
                sPieceCoords[11].flg = 0;
                if ((o->timer & 3) == 0) {
                    SpawnEffect(4, sPieceCoords[11].coord.t[0] - gWorldShift[0], y - gWorldShift[1],
                                z - gWorldShift[2], 0);
                }
                sPieceRots[11].vx = o->timer * 40960 / 360 * dir;
                sPieceRots[11].vy = 0;
                sPieceRots[11].vz = 0;
                SetCoordRotation(&sPieceRots[11], &sPieceCoords[11]);
                GsGetLs(&sPieceCoords[11], &m);
                GsSetLsMatrix(&m);
                GsSortObject4(&sPieceObjs[11], &gSubOt[gActiveBuff], 2, (u_long *)0x1F800000);
                if (++o->timer >= 16) {
                    o->state = -1;
                }
                break;
        }
    }
}

/* The same for piece model 2. */
s32 StepChasePiece2(Act3Obj *o) {
    s32 unused[2];
    MATRIX m;
    s32 dir;
    s32 y;
    s32 z;
    s32 dz;

    if (o->x <= gPlayer.x + 2000 && o->x >= gPlayer.x - 10000) {
        switch (o->state) {
            case 0:
                GsInitCoordinate2(WORLD, &sPieceCoords[12]);
                sPieceCoords[12].coord.t[0] = o->x + gWorldShift[0];
                sPieceCoords[12].coord.t[1] = o->y + gWorldShift[1];
                sPieceCoords[12].coord.t[2] = o->z + gWorldShift[2];
                sPieceCoords[12].flg = 0;
                GsGetLs(&sPieceCoords[12], &m);
                GsSetLsMatrix(&m);
                GsSortObject4(&sPieceObjs[12], &gSubOt[gActiveBuff], 2, (u_long *)0x1F800000);
                if (o->x < sCan.x + CHASE_REACH) {
                    o->state = 1;
                    o->timer = 0;
                    o->speed = 45;
                }
                if (o->x - 20 < gPlayer.x && gPlayer.x < o->x + 20 && o->z - 70 < gPlayer.z &&
                    gPlayer.z < o->z + 70 && o->y - 200 < gPlayer.y) {
                    gPlayer.action = 9;
                    EndJump();
                    o->state = 1;
                    o->timer = 0;
                    o->speed = gPlayer.speed * 3 / 2;
                }
                break;
            case 1:
                GsInitCoordinate2(WORLD, &sPieceCoords[12]);
                dir = 1;
                if (o->z < 0) {
                    dir = -1;
                }
                sPieceCoords[12].coord.t[0] = o->x + o->timer * o->speed + gWorldShift[0];
                y = o->y - rsin(o->timer * 40960 / 360) * 200 / 4096 + gWorldShift[1];
                sPieceCoords[12].coord.t[1] = y;
                dz = dir * 25;
                z = o->z + o->timer * dz + gWorldShift[2];
                sPieceCoords[12].coord.t[2] = z;
                sPieceCoords[12].flg = 0;
                if ((o->timer & 3) == 0) {
                    SpawnEffect(4, sPieceCoords[12].coord.t[0] - gWorldShift[0], y - gWorldShift[1],
                                z - gWorldShift[2], 0);
                }
                sPieceRots[12].vx = o->timer * 40960 / 360 * dir;
                sPieceRots[12].vy = 0;
                sPieceRots[12].vz = 0;
                SetCoordRotation(&sPieceRots[12], &sPieceCoords[12]);
                GsGetLs(&sPieceCoords[12], &m);
                GsSetLsMatrix(&m);
                GsSortObject4(&sPieceObjs[12], &gSubOt[gActiveBuff], 2, (u_long *)0x1F800000);
                if (++o->timer >= 16) {
                    o->state = -1;
                }
                break;
        }
    }
}

/* The same for piece model 3, tumbling on two axes. */
s32 StepChasePiece3(Act3Obj *o) {
    s32 unused[2];
    MATRIX m;
    s32 dir;
    s32 y;
    s32 z;
    s32 dz;

    if (o->x <= gPlayer.x + 2000 && o->x >= gPlayer.x - 10000) {
        switch (o->state) {
            case 0:
                GsInitCoordinate2(WORLD, &sPieceCoords[13]);
                sPieceCoords[13].coord.t[0] = o->x + gWorldShift[0];
                sPieceCoords[13].coord.t[1] = o->y + gWorldShift[1];
                sPieceCoords[13].coord.t[2] = o->z + gWorldShift[2];
                sPieceCoords[13].flg = 0;
                GsGetLs(&sPieceCoords[13], &m);
                GsSetLsMatrix(&m);
                GsSortObject4(&sPieceObjs[13], &gSubOt[gActiveBuff], 2, (u_long *)0x1F800000);
                if (o->x < sCan.x + CHASE_REACH) {
                    o->state = 1;
                    o->timer = 0;
                    o->speed = 45;
                }
                if (o->x - 50 < gPlayer.x && gPlayer.x < o->x + 50 && o->z - 60 < gPlayer.z &&
                    gPlayer.z < o->z + 60 && o->y - 80 < gPlayer.y) {
                    gPlayer.action = 9;
                    EndJump();
                    o->state = 1;
                    o->timer = 0;
                    o->speed = gPlayer.speed * 3 / 2;
                }
                break;
            case 1:
                GsInitCoordinate2(WORLD, &sPieceCoords[13]);
                dir = 1;
                if (o->z < 0) {
                    dir = -1;
                }
                sPieceCoords[13].coord.t[0] = o->x + o->timer * o->speed + gWorldShift[0];
                y = o->y - rsin(o->timer * 40960 / 360) * 200 / 4096 + gWorldShift[1];
                sPieceCoords[13].coord.t[1] = y;
                dz = dir * 25;
                z = o->z + o->timer * dz + gWorldShift[2];
                sPieceCoords[13].coord.t[2] = z;
                sPieceCoords[13].flg = 0;
                if ((o->timer & 3) == 0) {
                    SpawnEffect(4, sPieceCoords[13].coord.t[0] - gWorldShift[0], y - gWorldShift[1],
                                z - gWorldShift[2], 0);
                }
                sPieceRots[13].vx = o->timer * 40960 / 360 * dir;
                sPieceRots[13].vy = 0;
                sPieceRots[13].vz = o->timer * 49152 / 360 * dir;
                SetCoordRotation(&sPieceRots[13], &sPieceCoords[13]);
                GsGetLs(&sPieceCoords[13], &m);
                GsSetLsMatrix(&m);
                GsSortObject4(&sPieceObjs[13], &gSubOt[gActiveBuff], 2, (u_long *)0x1F800000);
                if (++o->timer >= 16) {
                    o->state = -1;
                }
                break;
        }
    }
}

/* The same for piece model 4. */
s32 StepChasePiece4(Act3Obj *o) {
    s32 unused[2];
    MATRIX m;
    s32 dir;
    s32 y;
    s32 z;
    s32 dz;

    if (o->x <= gPlayer.x + 2000 && o->x >= gPlayer.x - 10000) {
        switch (o->state) {
            case 0:
                GsInitCoordinate2(WORLD, &sPieceCoords[14]);
                sPieceCoords[14].coord.t[0] = o->x + gWorldShift[0];
                sPieceCoords[14].coord.t[1] = o->y + gWorldShift[1];
                sPieceCoords[14].coord.t[2] = o->z + gWorldShift[2];
                sPieceCoords[14].flg = 0;
                GsGetLs(&sPieceCoords[14], &m);
                GsSetLsMatrix(&m);
                GsSortObject4(&sPieceObjs[14], &gSubOt[gActiveBuff], 2, (u_long *)0x1F800000);
                if (o->x < sCan.x + CHASE_REACH) {
                    o->state = 1;
                    o->timer = 0;
                    o->speed = 45;
                }
                if (o->x - 20 < gPlayer.x && gPlayer.x < o->x + 20 && o->z - 95 < gPlayer.z &&
                    gPlayer.z < o->z + 95 && o->y - 100 < gPlayer.y) {
                    gPlayer.action = 9;
                    EndJump();
                    o->state = 1;
                    o->timer = 0;
                    o->speed = gPlayer.speed * 3 / 2;
                }
                break;
            case 1:
                GsInitCoordinate2(WORLD, &sPieceCoords[14]);
                dir = 1;
                if (o->z < 0) {
                    dir = -1;
                }
                sPieceCoords[14].coord.t[0] = o->x + o->timer * o->speed + gWorldShift[0];
                y = o->y - rsin(o->timer * 40960 / 360) * 200 / 4096 + gWorldShift[1];
                sPieceCoords[14].coord.t[1] = y;
                dz = dir * 25;
                z = o->z + o->timer * dz + gWorldShift[2];
                sPieceCoords[14].coord.t[2] = z;
                sPieceCoords[14].flg = 0;
                if ((o->timer & 3) == 0) {
                    SpawnEffect(4, sPieceCoords[14].coord.t[0] - gWorldShift[0], y - gWorldShift[1],
                                z - gWorldShift[2], 0);
                }
                sPieceRots[14].vx = o->timer * 40960 / 360 * dir;
                sPieceRots[14].vy = 0;
                sPieceRots[14].vz = 0;
                SetCoordRotation(&sPieceRots[14], &sPieceCoords[14]);
                GsGetLs(&sPieceCoords[14], &m);
                GsSetLsMatrix(&m);
                GsSortObject4(&sPieceObjs[14], &gSubOt[gActiveBuff], 2, (u_long *)0x1F800000);
                if (++o->timer >= 16) {
                    o->state = -1;
                }
                break;
        }
    }
}

/* The same for piece model 5, tumbling on two axes. */
s32 StepChasePiece5(Act3Obj *o) {
    s32 unused[2];
    MATRIX m;
    s32 dir;
    s32 y;
    s32 z;
    s32 dz;
    s32 h;

    if (o->x <= gPlayer.x + 2000 && o->x >= gPlayer.x - 10000) {
        switch (o->state) {
            case 0:
                GsInitCoordinate2(WORLD, &sPieceCoords[15]);
                sPieceCoords[15].coord.t[0] = o->x + gWorldShift[0];
                sPieceCoords[15].coord.t[1] = o->y - 60 + gWorldShift[1];
                sPieceCoords[15].coord.t[2] = o->z + gWorldShift[2];
                sPieceCoords[15].flg = 0;
                GsGetLs(&sPieceCoords[15], &m);
                GsSetLsMatrix(&m);
                GsSortObject4(&sPieceObjs[15], &gSubOt[gActiveBuff], 2, (u_long *)0x1F800000);
                if (o->x < sCan.x + CHASE_REACH) {
                    o->state = 1;
                    o->timer = 0;
                    o->speed = 45;
                }
                if (o->x - 35 < gPlayer.x && gPlayer.x < o->x + 35 && o->z - 45 < gPlayer.z &&
                    gPlayer.z < o->z + 45 && o->y - 80 < gPlayer.y) {
                    gPlayer.action = 9;
                    EndJump();
                    o->state = 1;
                    o->timer = 0;
                    o->speed = gPlayer.speed * 3 / 2;
                }
                break;
            case 1:
                GsInitCoordinate2(WORLD, &sPieceCoords[15]);
                dir = 1;
                if (o->z < 0) {
                    dir = -1;
                }
                sPieceCoords[15].coord.t[0] = o->x + o->timer * o->speed + gWorldShift[0];
                h = rsin(o->timer * 40960 / 360) * 200 / 4096 + 60;
                y = o->y - h + gWorldShift[1];
                sPieceCoords[15].coord.t[1] = y;
                dz = dir * 25;
                z = o->z + o->timer * dz + gWorldShift[2];
                sPieceCoords[15].coord.t[2] = z;
                sPieceCoords[15].flg = 0;
                if ((o->timer & 3) == 0) {
                    SpawnEffect(4, sPieceCoords[15].coord.t[0] - gWorldShift[0], y - gWorldShift[1],
                                z - gWorldShift[2], 0);
                }
                sPieceRots[15].vx = o->timer * 40960 / 360 * dir;
                sPieceRots[15].vy = 0;
                sPieceRots[15].vz = o->timer * 32768 / 360 * dir;
                SetCoordRotation(&sPieceRots[15], &sPieceCoords[15]);
                GsGetLs(&sPieceCoords[15], &m);
                GsSetLsMatrix(&m);
                GsSortObject4(&sPieceObjs[15], &gSubOt[gActiveBuff], 2, (u_long *)0x1F800000);
                if (++o->timer >= 16) {
                    o->state = -1;
                }
                break;
        }
    }
}

/* The same for piece model 6. */
s32 StepChasePiece6(Act3Obj *o) {
    s32 unused[2];
    MATRIX m;
    s32 dir;
    s32 y;
    s32 z;
    s32 dz;

    if (o->x <= gPlayer.x + 2000 && o->x >= gPlayer.x - 10000) {
        switch (o->state) {
            case 0:
                GsInitCoordinate2(WORLD, &sPieceCoords[16]);
                sPieceCoords[16].coord.t[0] = o->x + gWorldShift[0];
                sPieceCoords[16].coord.t[1] = o->y + gWorldShift[1];
                sPieceCoords[16].coord.t[2] = o->z + gWorldShift[2];
                sPieceCoords[16].flg = 0;
                GsGetLs(&sPieceCoords[16], &m);
                GsSetLsMatrix(&m);
                GsSortObject4(&sPieceObjs[16], &gSubOt[gActiveBuff], 2, (u_long *)0x1F800000);
                if (o->x < sCan.x + CHASE_REACH) {
                    o->state = 1;
                    o->timer = 0;
                    o->speed = 45;
                }
                if (o->x - 20 < gPlayer.x && gPlayer.x < o->x + 20 && o->z - 105 < gPlayer.z &&
                    gPlayer.z < o->z + 105 && o->y - 90 < gPlayer.y) {
                    gPlayer.action = 9;
                    EndJump();
                    o->state = 1;
                    o->timer = 0;
                    o->speed = gPlayer.speed * 3 / 2;
                }
                break;
            case 1:
                GsInitCoordinate2(WORLD, &sPieceCoords[16]);
                dir = 1;
                if (o->z < 0) {
                    dir = -1;
                }
                sPieceCoords[16].coord.t[0] = o->x + o->timer * o->speed + gWorldShift[0];
                y = o->y - rsin(o->timer * 40960 / 360) * 200 / 4096 + gWorldShift[1];
                sPieceCoords[16].coord.t[1] = y;
                dz = dir * 25;
                z = o->z + o->timer * dz + gWorldShift[2];
                sPieceCoords[16].coord.t[2] = z;
                sPieceCoords[16].flg = 0;
                if ((o->timer & 3) == 0) {
                    SpawnEffect(4, sPieceCoords[16].coord.t[0] - gWorldShift[0], y - gWorldShift[1],
                                z - gWorldShift[2], 0);
                }
                sPieceRots[16].vx = o->timer * 40960 / 360 * dir;
                sPieceRots[16].vy = 0;
                sPieceRots[16].vz = 0;
                SetCoordRotation(&sPieceRots[16], &sPieceCoords[16]);
                GsGetLs(&sPieceCoords[16], &m);
                GsSetLsMatrix(&m);
                GsSortObject4(&sPieceObjs[16], &gSubOt[gActiveBuff], 2, (u_long *)0x1F800000);
                if (++o->timer >= 16) {
                    o->state = -1;
                }
                break;
        }
    }
}

/* Piece model 7, a block that stops the player instead of hurting it. */
s32 StepChasePiece7(Act3Obj *o) {
    s32 unused[2];
    MATRIX m;
    s32 dir;
    s32 y;
    s32 z;
    s32 dz;
    s32 h;

    if (o->x <= gPlayer.x + 2000 && o->x >= gPlayer.x - 10000) {
        switch (o->state) {
            case 0:
                GsInitCoordinate2(WORLD, &sPieceCoords[17]);
                sPieceCoords[17].coord.t[0] = o->x + gWorldShift[0];
                sPieceCoords[17].coord.t[1] = o->y - 60 + gWorldShift[1];
                sPieceCoords[17].coord.t[2] = o->z + gWorldShift[2];
                sPieceCoords[17].flg = 0;
                GsGetLs(&sPieceCoords[17], &m);
                GsSetLsMatrix(&m);
                GsSortObject4(&sPieceObjs[17], &gSubOt[gActiveBuff], 2, (u_long *)0x1F800000);
                if (o->x < sCan.x + CHASE_REACH) {
                    o->state = 1;
                    o->timer = 0;
                    o->speed = 45;
                }
                if (o->x - 60 < gPlayer.x && gPlayer.x < o->x + 60 && o->z - 70 < gPlayer.z &&
                    gPlayer.z < o->z + 70 && o->y - 150 < gPlayer.y) {
                    gPlayer.x = o->x - 60;
                }
                break;
            case 1:
                GsInitCoordinate2(WORLD, &sPieceCoords[17]);
                dir = 1;
                if (o->z < 0) {
                    dir = -1;
                }
                sPieceCoords[17].coord.t[0] = o->x + o->timer * o->speed + gWorldShift[0];
                h = rsin(o->timer * 40960 / 360) * 200 / 4096 + 60;
                y = o->y - h + gWorldShift[1];
                sPieceCoords[17].coord.t[1] = y;
                dz = dir * 25;
                z = o->z + o->timer * dz + gWorldShift[2];
                sPieceCoords[17].coord.t[2] = z;
                sPieceCoords[17].flg = 0;
                if ((o->timer & 3) == 0) {
                    SpawnEffect(4, sPieceCoords[17].coord.t[0] - gWorldShift[0], y - gWorldShift[1],
                                z - gWorldShift[2], 0);
                }
                sPieceRots[17].vx = o->timer * 40960 / 360 * dir;
                sPieceRots[17].vy = 0;
                sPieceRots[17].vz = o->timer * 32768 / 360 * dir;
                SetCoordRotation(&sPieceRots[17], &sPieceCoords[17]);
                GsGetLs(&sPieceCoords[17], &m);
                GsSetLsMatrix(&m);
                GsSortObject4(&sPieceObjs[17], &gSubOt[gActiveBuff], 2, (u_long *)0x1F800000);
                if (++o->timer >= 16) {
                    o->state = -1;
                }
                break;
        }
    }
}

s32 StepChasePiece8(Act3Obj *o) {
    s32 unused[2];
    MATRIX m;
    s32 dir;
    s32 y;
    s32 z;
    s32 dz;
    s32 h;

    if (o->x <= gPlayer.x + 2000 && o->x >= gPlayer.x - 10000) {
        switch (o->state) {
            case 0:
                GsInitCoordinate2(WORLD, &sPieceCoords[18]);
                sPieceCoords[18].coord.t[0] = o->x + gWorldShift[0];
                sPieceCoords[18].coord.t[1] = o->y + gWorldShift[1];
                sPieceCoords[18].coord.t[2] = o->z + gWorldShift[2];
                sPieceCoords[18].flg = 0;
                GsGetLs(&sPieceCoords[18], &m);
                GsSetLsMatrix(&m);
                GsSortObject4(&sPieceObjs[18], &gSubOt[gActiveBuff], 2, (u_long *)0x1F800000);
                if (o->x < sCan.x + CHASE_REACH) {
                    o->state = 1;
                    o->timer = 0;
                    o->speed = 45;
                }
                h = 180;
                if (gPlayer.motion[0] == MOT_SLIDE || gPlayer.motion[0] == MOT_SLIDE_END) {
                    h = 90;
                }
                if (o->x - 20 < gPlayer.x && gPlayer.x < o->x + 20 && o->z - 20 < gPlayer.z &&
                    gPlayer.z < o->z + 220 && o->y - 30 < gPlayer.y && gPlayer.y < o->y + h + 30) {
                    gPlayer.action = 9;
                    EndJump();
                    o->state = 1;
                    o->timer = 0;
                    o->speed = gPlayer.speed * 3 / 2;
                }
                break;
            case 1:
                GsInitCoordinate2(WORLD, &sPieceCoords[18]);
                dir = 1;
                if (o->z < 0) {
                    dir = -1;
                }
                sPieceCoords[18].coord.t[0] = o->x + o->timer * o->speed + gWorldShift[0];
                y = o->y - rsin(o->timer * 40960 / 360) * 200 / 4096 + gWorldShift[1];
                sPieceCoords[18].coord.t[1] = y;
                dz = dir * 25;
                z = o->z + o->timer * dz + gWorldShift[2];
                sPieceCoords[18].coord.t[2] = z;
                sPieceCoords[18].flg = 0;
                if ((o->timer & 3) == 0) {
                    SpawnEffect(4, sPieceCoords[18].coord.t[0] - gWorldShift[0], y - gWorldShift[1],
                                z - gWorldShift[2], 0);
                }
                sPieceRots[18].vx = -(o->timer * 40960) / 360 * dir;
                sPieceRots[18].vy = 0;
                sPieceRots[18].vz = 0;
                SetCoordRotation(&sPieceRots[18], &sPieceCoords[18]);
                GsGetLs(&sPieceCoords[18], &m);
                GsSetLsMatrix(&m);
                GsSortObject4(&sPieceObjs[18], &gSubOt[gActiveBuff], 2, (u_long *)0x1F800000);
                if (++o->timer >= 16) {
                    o->state = -1;
                }
                break;
        }
    }
}

s32 StepChasePiece9(Act3Obj *o) {
    s32 unused[2];
    MATRIX m;
    s32 dir;
    s32 y;
    s32 z;
    s32 dz;
    s32 h;

    if (o->x <= gPlayer.x + 2000 && o->x >= gPlayer.x - 10000) {
        switch (o->state) {
            case 0:
                GsInitCoordinate2(WORLD, &sPieceCoords[19]);
                sPieceCoords[19].coord.t[0] = o->x + gWorldShift[0];
                sPieceCoords[19].coord.t[1] = o->y + gWorldShift[1];
                sPieceCoords[19].coord.t[2] = o->z + gWorldShift[2];
                sPieceCoords[19].flg = 0;
                GsGetLs(&sPieceCoords[19], &m);
                GsSetLsMatrix(&m);
                GsSortObject4(&sPieceObjs[19], &gSubOt[gActiveBuff], 2, (u_long *)0x1F800000);
                if (o->x < sCan.x + CHASE_REACH) {
                    o->state = 1;
                    o->timer = 0;
                    o->speed = 45;
                }
                h = 180;
                if (gPlayer.motion[0] == MOT_SLIDE || gPlayer.motion[0] == MOT_SLIDE_END) {
                    h = 90;
                }
                if (o->x - 20 < gPlayer.x && gPlayer.x < o->x + 20 && o->z - 220 < gPlayer.z &&
                    gPlayer.z < o->z + 20 && o->y - 30 < gPlayer.y && gPlayer.y < o->y + h + 30) {
                    gPlayer.action = 9;
                    EndJump();
                    o->state = 1;
                    o->timer = 0;
                    o->speed = gPlayer.speed * 3 / 2;
                }
                break;
            case 1:
                GsInitCoordinate2(WORLD, &sPieceCoords[19]);
                dir = 1;
                if (o->z < 0) {
                    dir = -1;
                }
                sPieceCoords[19].coord.t[0] = o->x + o->timer * o->speed + gWorldShift[0];
                y = o->y - rsin(o->timer * 40960 / 360) * 200 / 4096 + gWorldShift[1];
                sPieceCoords[19].coord.t[1] = y;
                dz = dir * 25;
                z = o->z + o->timer * dz + gWorldShift[2];
                sPieceCoords[19].coord.t[2] = z;
                sPieceCoords[19].flg = 0;
                if ((o->timer & 3) == 0) {
                    SpawnEffect(4, sPieceCoords[19].coord.t[0] - gWorldShift[0], y - gWorldShift[1],
                                z - gWorldShift[2], 0);
                }
                sPieceRots[19].vx = -(o->timer * 40960) / 360 * dir;
                sPieceRots[19].vy = 0;
                sPieceRots[19].vz = 0;
                SetCoordRotation(&sPieceRots[19], &sPieceCoords[19]);
                GsGetLs(&sPieceCoords[19], &m);
                GsSetLsMatrix(&m);
                GsSortObject4(&sPieceObjs[19], &gSubOt[gActiveBuff], 2, (u_long *)0x1F800000);
                if (++o->timer >= 16) {
                    o->state = -1;
                }
                break;
        }
    }
}

s32 StepChasePiece10(Act3Obj *o) {
    s32 unused[2];
    MATRIX m;
    s32 dir;
    s32 y;
    s32 z;
    s32 dz;
    s32 h;

    if (o->x <= gPlayer.x + 2000 && o->x >= gPlayer.x - 10000) {
        switch (o->state) {
            case 0:
                GsInitCoordinate2(WORLD, &sPieceCoords[20]);
                sPieceCoords[20].coord.t[0] = o->x + gWorldShift[0];
                sPieceCoords[20].coord.t[1] = o->y + gWorldShift[1];
                sPieceCoords[20].coord.t[2] = o->z + gWorldShift[2];
                sPieceCoords[20].flg = 0;
                GsGetLs(&sPieceCoords[20], &m);
                GsSetLsMatrix(&m);
                GsSortObject4(&sPieceObjs[20], &gSubOt[gActiveBuff], 2, (u_long *)0x1F800000);
                if (o->x < sCan.x + CHASE_REACH) {
                    o->state = 1;
                    o->timer = 0;
                    o->speed = 45;
                }
                h = 180;
                if (gPlayer.motion[0] == MOT_SLIDE || gPlayer.motion[0] == MOT_SLIDE_END) {
                    h = 90;
                }
                if (o->x - 20 < gPlayer.x && gPlayer.x < o->x + 20 && o->z - 20 < gPlayer.z &&
                    gPlayer.z < o->z + 520 && o->y - 75 < gPlayer.y && gPlayer.y < o->y + h + 75) {
                    gPlayer.action = 9;
                    EndJump();
                    o->state = 1;
                    o->timer = 0;
                    o->speed = gPlayer.speed * 3 / 2;
                }
                break;
            case 1:
                GsInitCoordinate2(WORLD, &sPieceCoords[20]);
                dir = 1;
                if (o->z < 0) {
                    dir = -1;
                }
                sPieceCoords[20].coord.t[0] = o->x + o->timer * o->speed + gWorldShift[0];
                y = o->y - rsin(o->timer * 40960 / 360) * 200 / 4096 + gWorldShift[1];
                sPieceCoords[20].coord.t[1] = y;
                dz = dir * 25;
                z = o->z + o->timer * dz + gWorldShift[2];
                sPieceCoords[20].coord.t[2] = z;
                sPieceCoords[20].flg = 0;
                if ((o->timer & 3) == 0) {
                    SpawnEffect(4, sPieceCoords[20].coord.t[0] - gWorldShift[0], y - gWorldShift[1],
                                z - gWorldShift[2], 0);
                }
                sPieceRots[20].vx = -(o->timer * 40960) / 360 * dir;
                sPieceRots[20].vy = 0;
                sPieceRots[20].vz = 0;
                SetCoordRotation(&sPieceRots[20], &sPieceCoords[20]);
                GsGetLs(&sPieceCoords[20], &m);
                GsSetLsMatrix(&m);
                GsSortObject4(&sPieceObjs[20], &gSubOt[gActiveBuff], 2, (u_long *)0x1F800000);
                if (++o->timer >= 16) {
                    o->state = -1;
                }
                break;
        }
    }
}

s32 StepChasePiece11(Act3Obj *o) {
    s32 unused[2];
    MATRIX m;
    s32 dir;
    s32 y;
    s32 z;
    s32 dz;
    s32 h;

    if (o->x <= gPlayer.x + 2000 && o->x >= gPlayer.x - 10000) {
        switch (o->state) {
            case 0:
                GsInitCoordinate2(WORLD, &sPieceCoords[21]);
                sPieceCoords[21].coord.t[0] = o->x + gWorldShift[0];
                sPieceCoords[21].coord.t[1] = o->y + gWorldShift[1];
                sPieceCoords[21].coord.t[2] = o->z + gWorldShift[2];
                sPieceCoords[21].flg = 0;
                GsGetLs(&sPieceCoords[21], &m);
                GsSetLsMatrix(&m);
                GsSortObject4(&sPieceObjs[21], &gSubOt[gActiveBuff], 2, (u_long *)0x1F800000);
                if (o->x < sCan.x + CHASE_REACH) {
                    o->state = 1;
                    o->timer = 0;
                    o->speed = 45;
                }
                h = 180;
                if (gPlayer.motion[0] == MOT_SLIDE || gPlayer.motion[0] == MOT_SLIDE_END) {
                    h = 90;
                }
                if (o->x - 20 < gPlayer.x && gPlayer.x < o->x + 20 && o->z - 520 < gPlayer.z &&
                    gPlayer.z < o->z + 20 && o->y - 75 < gPlayer.y && gPlayer.y < o->y + h + 75) {
                    gPlayer.action = 9;
                    EndJump();
                    o->state = 1;
                    o->timer = 0;
                    o->speed = gPlayer.speed * 3 / 2;
                }
                break;
            case 1:
                GsInitCoordinate2(WORLD, &sPieceCoords[21]);
                dir = 1;
                if (o->z < 0) {
                    dir = -1;
                }
                sPieceCoords[21].coord.t[0] = o->x + o->timer * o->speed + gWorldShift[0];
                y = o->y - rsin(o->timer * 40960 / 360) * 200 / 4096 + gWorldShift[1];
                sPieceCoords[21].coord.t[1] = y;
                dz = dir * 25;
                z = o->z + o->timer * dz + gWorldShift[2];
                sPieceCoords[21].coord.t[2] = z;
                sPieceCoords[21].flg = 0;
                if ((o->timer & 3) == 0) {
                    SpawnEffect(4, sPieceCoords[21].coord.t[0] - gWorldShift[0], y - gWorldShift[1],
                                z - gWorldShift[2], 0);
                }
                sPieceRots[21].vx = -(o->timer * 40960) / 360 * dir;
                sPieceRots[21].vy = 0;
                sPieceRots[21].vz = 0;
                SetCoordRotation(&sPieceRots[21], &sPieceCoords[21]);
                GsGetLs(&sPieceCoords[21], &m);
                GsSetLsMatrix(&m);
                GsSortObject4(&sPieceObjs[21], &gSubOt[gActiveBuff], 2, (u_long *)0x1F800000);
                if (++o->timer >= 16) {
                    o->state = -1;
                }
                break;
        }
    }
}

/* Piece model 12, which drops from above once the player is near. */
s32 StepChasePiece12(Act3Obj *o) {
    s32 unused[2];
    MATRIX m;
    s32 dir;
    s32 y;
    s32 z;
    s32 dz;
    s32 h;

    if (o->x <= gPlayer.x + 500 && o->x >= gPlayer.x - 10000) {
        switch (o->state) {
            case 0:
                GsInitCoordinate2(WORLD, &sPieceCoords[22]);
                o->y += 30;
                sPieceCoords[22].coord.t[0] = o->x + gWorldShift[0];
                sPieceCoords[22].coord.t[1] = o->y + gWorldShift[1];
                sPieceCoords[22].coord.t[2] = o->z + gWorldShift[2];
                sPieceCoords[22].flg = 0;
                GsGetLs(&sPieceCoords[22], &m);
                GsSetLsMatrix(&m);
                GsSortObject4(&sPieceObjs[22], &gSubOt[gActiveBuff], 2, (u_long *)0x1F800000);
                if (o->y >= -39) {
                    o->state = 1;
                    o->timer = 0;
                    o->speed = 0;
                }
                if (o->x < sCan.x + CHASE_REACH) {
                    o->state = 1;
                    o->timer = 0;
                    o->speed = 45;
                }
                h = 180;
                if (gPlayer.motion[0] == MOT_SLIDE || gPlayer.motion[0] == MOT_SLIDE_END) {
                    h = 90;
                }
                if (o->x - 20 < gPlayer.x && gPlayer.x < o->x + 20 && o->z - 70 < gPlayer.z &&
                    gPlayer.z < o->z + 70 && o->y - 40 < gPlayer.y && gPlayer.y < o->y + h + 40) {
                    gPlayer.action = 9;
                    EndJump();
                    o->state = 1;
                    o->timer = 0;
                    o->speed = gPlayer.speed * 3 / 2;
                }
                break;
            case 1:
                GsInitCoordinate2(WORLD, &sPieceCoords[22]);
                dir = 1;
                if (o->z < 0) {
                    dir = -1;
                }
                sPieceCoords[22].coord.t[0] = o->x + o->timer * o->speed + gWorldShift[0];
                y = o->y - rsin(o->timer * 40960 / 360) * 200 / 4096 + gWorldShift[1];
                sPieceCoords[22].coord.t[1] = y;
                dz = dir * 25;
                z = o->z + o->timer * dz + gWorldShift[2];
                sPieceCoords[22].coord.t[2] = z;
                sPieceCoords[22].flg = 0;
                if ((o->timer & 3) == 0) {
                    SpawnEffect(4, sPieceCoords[22].coord.t[0] - gWorldShift[0], y - gWorldShift[1],
                                z - gWorldShift[2], 0);
                }
                sPieceRots[22].vx = o->timer * 40960 / 360 * dir;
                sPieceRots[22].vy = 0;
                sPieceRots[22].vz = 0;
                SetCoordRotation(&sPieceRots[22], &sPieceCoords[22]);
                GsGetLs(&sPieceCoords[22], &m);
                GsSetLsMatrix(&m);
                GsSortObject4(&sPieceObjs[22], &gSubOt[gActiveBuff], 2, (u_long *)0x1F800000);
                if (++o->timer >= 16) {
                    o->state = -1;
                }
                break;
        }
    }
}

/** @brief The intro clock's record, as this stage's frame step views it. */
typedef struct {
    s32 unk0;              /**< not yet known */
    s32 fall;              /**< the can's fall speed in the opening scene */
    s32 depth;             /**< the depth scale the player's model is drawn with */
    u8 padC[0x24 - 0xC];   /**< not yet known */
    u16 timer;             /**< the step's clock */
    s16 title;             /**< the title's and the closing caption's clock */
    u8 pad28[0x40 - 0x28]; /**< not yet known */
    u8 canInSub;           /**< 1 sorts the can into the sub ordering table */
} ChaseStepRec;

/* The step's record. */
#define sRec (*(ChaseStepRec *)&D_800D39D8)
/* MATCHING: the step's sub-step, kept in the fog fade's word. */
#define sSubStep gFogFade

extern s32 (*D_800F7280[])(Act3Obj *o);
extern GsCOORDINATE2 sAct3Coords[];
extern GsDOBJ2 sAct3Objs[];
/* MATCHING: model's view of the sky colours, as bytes. */
extern u8 *gSkyColors;

/* This unit's routines the step calls. */
void func_800F0234(void);
void func_800F0590(void);
void func_800F06E0(void);
void func_800F09D8(void);
void SetChaseLights(void);
void PlaceChaseObjects(void);

/* MATCHING: views of routines their units declare only where defined. */
s16 Lerp(s16 from, s16 to, u16 step, u16 steps);
s32 UpdatePlayerAction(PlayerState *state);
s32 DrawTodObjects(void);
void DrawPlayerShadow(void);
void UnusedHook3(void);
void DrawSky(u32 arg);
void DrawRepeatingStage(s32 pos, s32 unused, s32 z, s32 range);
void UpdateEffects(s32 unused, s16 x, s16 y, s16 z);

/* Draws the player's models with their depth scaled by the step's record,
 * each sorted on the scratchpad stack. */
/* MATCHING: inlined, so the matrix's address is not kept across the calls. */
static inline void DrawStepPlayer(void) {
    MATRIX m;
    VECTOR scale;
    GsDOBJ2 *o;
    s32 i;
    s32 sz;

    sz = sRec.depth;
    o = ((DrawList *)gTodObjTable)->objs;
    for (i = 0; i < ((DrawList *)gTodObjTable)->count; o++, i++) {
        o->coord2->flg = 0;
        if (o->id != -1 && o->tmd != NULL) {
            GsGetLs(o->coord2, &m);
            scale.vx = 0x1000;
            scale.vy = 0x1000;
            scale.vz = sz;
            ScaleMatrix(&m, &scale);
            GsSetLsMatrix(&m);
            GsGetLw(o->coord2, &m);
            GsSetLightMatrix(&m);
            SetSpadStack();
            GsSortObject4(o, &gSubOt[gActiveBuff], 2, (u_long *)0x1F800000);
            ResetSpadStack();
        }
    }
}

/* MATCHING: tested `>= 0`, cc1 folds it to an abs that keeps the minuend
 * computed first. */
#define ABS_GE(x) ((x) >= 0 ? (x) : -(x))

/* The stage's frame step: the opening scene, where the can bursts out and
 * rolls after the player, the chase, the escape at the run's end, and a
 * debug view that places the can by pad. */
/* MATCHING: non-void with no return keeps the jump table's bound check
 * delay slot; the unused 32-byte local gives retail's frame. */
s32 func_800F42E8(void) {
    s32 unused[8];
    MATRIX m;
    u8 sky[16] = {0x6B, 0xBC, 0xEB, 0, 0xFF, 0xFF, 0xFF, 0,
                  0xFF, 0xFF, 0xFF, 0, 0xFF, 0xFF, 0xFF, 0};
    Act3Obj *o;
    s32 i;
    s32 t;

    switch (gWorldIntroStep) {
        case 0:
            GsSetProjection(250);
            gSkyColors = sky;
            sCan.x = -0x1676;
            sCan.y = -0x4B0;
            sCan.z = 0x78A;
            sCanRot[0].vx = -0x23A;
            sCanRot[0].vy = -0x596;
            sCanRot[0].vz = 0x1FE;
            sCan.lidX = -0x1130;
            sCan.lidY = 0;
            sCan.lidZ = -300;
            sCanRot[1].vy = 0x400;
            sCanRot[1].vx = 0;
            sCanRot[1].vz = 0;
            gMissState = 0;
            gPlayer.motion[0] = 0x13;
            gPlayer.action = 0x17;
            gPlayer.speed = 0;
            gPlayer.x = -0x15E0;
            gPlayer.y = 0;
            gPlayer.z = -300;
            gPlayer.control = 0;
            gCourse.rot.vy = 0x800;
            sRec.canInSub = 0;
            gPlayer.curMotion[1] = 0;
            sSubStep = 0;
            gWorldIntroStep++;
            break;
        case 1:
            switch (sSubStep) {
                case 0:
                    gView[0] = -0x155E;
                    gView[1] = -0xAA;
                    gView[2] = -0x29E;
                    gView[3] = -0x173E;
                    gView[4] = 0xB4;
                    gView[5] = 0x76C;
                    sSubStep = 1;
                    sRec.timer = 0;
                    break;
                case 1:
                    sCan.lidX -= 50;
                    /* MATCHING: `p + (rand() % n - k)` takes p - k first; the
                     * heights below, `y - k + rand() % n`, the roll less k. */
                    SpawnEffect(4, sCan.lidX - 100, sCan.lidY + (rand() % 50 - 25),
                                sCan.lidZ + (rand() % 200 - 100), 0);
                    if (++sRec.timer == 20) {
                        for (gLoopI = 0; gLoopI < 4; gLoopI++) {
                            SpawnEffect(4, gPlayer.x + (rand() % 50 - 25), gPlayer.y - 150 + rand() % 100,
                                        gPlayer.z + (rand() % 50 - 25), rand() % 4);
                        }
                        gPlayer.action = ACT_MODEL_ONCE;
                        gPlayer.motion[0] = 0x34;
                        sSubStep = 2;
                        sRec.timer = 0;
                    }
                    break;
                case 2:
                    gPlayer.x -= 50;
                    gPlayer.y -= 50;
                    if (++sRec.timer == 20) {
                        gPlayer.x = -6000;
                        gPlayer.y = -1300;
                        gPlayer.z = 1000;
                        sSubStep = 5;
                        sRec.timer = 0;
                    }
                    break;
                case 5:
                    gView[0] = -0x15EA;
                    gView[1] = -0x3DE;
                    gView[2] = 1000;
                    gView[3] = -0x1630;
                    gView[4] = -0x492;
                    gView[5] = 0x64A;
                    gPlayer.z += 50;
                    gPlayer.y += 10;
                    if (gPlayer.z >= 1800) {
                        gPlayer.motion[0] = 0x43;
                        for (gLoopI = 0; gLoopI < 4; gLoopI++) {
                            SpawnEffect(4, gPlayer.x + (rand() % 100 - 50),
                                        gPlayer.y - 150 + rand() % 100,
                                        gPlayer.z + (rand() % 100 - 50), rand() % 4);
                        }
                        gView[0] = -0x15EA;
                        gView[1] = -1000;
                        gView[2] = 1000;
                        sRec.timer = 0;
                        sSubStep = 6;
                    }
                    break;
                case 6:
                    gView[0] = -0x15EA;
                    gView[1] = -0x3DE;
                    gView[2] = 1000;
                    gView[3] = -0x1630;
                    gView[4] = -0x492;
                    gView[5] = 0x64A;
                    gPlayer.x += 50;
                    gPlayer.y += 10;
                    gPlayer.z -= 30;
                    if (++sRec.timer == 15) {
                        sRec.timer = 0;
                        sSubStep = 10;
                    }
                    break;
                case 10:
                    sCanRot[0].vx = rsin(sRec.timer * 245760 / 360) * 30 / 4096 - 0x23A;
                    rand(); /* a roll thrown away */
                    SpawnEffect(4, sCan.x + (rand() % 300 - 150), sCan.y + (rand() % 300 - 150),
                                sCan.z + (rand() % 300 - 150), 0);
                    if (++sRec.timer == 60) {
                        sRec.timer = 0;
                        sRec.fall = 0;
                        sSubStep = 15;
                    }
                    break;
                case 15:
                    sCanRot[0].vx = Lerp(sCanRot[0].vx, 0, sRec.timer, 40);
                    sRec.fall += 2;
                    sCan.x += 10;
                    sCan.y += sRec.fall;
                    sCanRot[0].vz -= 0x44;
                    if (++sRec.timer == 20) {
                        for (gLoopI = 0; gLoopI < 4; gLoopI++) {
                            for (gLoopJ = 0; gLoopJ < 4; gLoopJ++) {
                                SpawnEffect(4, sCan.x + (rand() % 300 - 150),
                                            sCan.y + (rand() % 300 - 150),
                                            sCan.z + (rand() % 300 - 150), gLoopI);
                            }
                        }
                        sSubStep = 20;
                    }
                    break;
                case 20:
                    sCanRot[0].vx = Lerp(sCanRot[0].vx, 0, sRec.timer, 60);
                    sRec.fall -= 2;
                    sCan.x += 20;
                    sCan.y -= sRec.fall;
                    sCanRot[0].vz -= 0x88;
                    if (++sRec.timer == 60) {
                        gCourse.rot.vy = -0x400;
                        gPlayer.x = -1000;
                        gPlayer.y = 0;
                        gPlayer.z = 0;
                        gPlayer.motion[0] = 0x35;
                        sCan.x = -4000;
                        /* MATCHING: the height worked in sCan.y itself. */
                        sCan.y = rsin(gFrameCount * 40960 / 360) * 100 / 4096;
                        sCan.y = ABS(sCan.y) - 140;
                        sCan.z = 0;
                        sRec.canInSub = 1;
                        sRec.timer = 0;
                        sSubStep = 22;
                    }
                    break;
                case 22:
                    gView[0] = gPlayer.x + 300;
                    gView[1] = gPlayer.y - 300;
                    gView[2] = gPlayer.z;
                    gView[3] = gPlayer.x - 100;
                    gView[4] = gPlayer.y;
                    gView[5] = gPlayer.z;
                    SpawnEffect(4, gPlayer.x + (rand() % 100 - 50), gPlayer.y,
                                gPlayer.z + (rand() % 100 - 50), 0);
                    if (++sRec.timer == 5) {
                        gPlayer.motion[0] = 0x36;
                        sRec.timer = 0;
                        sSubStep = 25;
                    }
                    break;
                case 25:
                    if (++sRec.timer == 100) {
                        gPlayer.motion[0] = 0x47;
                        sRec.timer = 0;
                        sSubStep = 26;
                    }
                    break;
                case 26:
                    if (++sRec.timer == 50) {
                        gPlayer.motion[0] = 2;
                        sRec.timer = 0;
                        sSubStep = 30;
                    }
                    break;
                case 30:
                    sCan.x += 30;
                    SpawnEffect(4, sCan.x, -50 - rand() % 50, rand() % 500 - 250, 0);
                    sCan.y = rsin(gFrameCount * 40960 / 360) * 100 / 4096;
                    sCan.y = ABS(sCan.y) - 140;
                    sCanRot[0].vx = 0x800;
                    sCanRot[0].vy = 0;
                    sCanRot[0].vz = -(gFrameCount * 49152 / 360);
                    if (++sRec.timer == 30) {
                        gPlayer.motion[0] = 2;
                        sRec.timer = 0;
                        sSubStep = 100;
                    }
                    break;
                case 100:
                    gWorldIntroStep = 2;
                    break;
            }
            GsSetRefView2((GsRVIEW2 *)gView);
            gPlayer.groundY = gPlayer.y;
            gFloorY[0] = gPlayer.y;
            func_800F0234();
            SetChaseLights();
            UpdateEffects((s32)&gSubOt[gActiveBuff], 0, 0, 0);
            DrawRepeatingStage(0, 0, 0, 30000);
            UpdatePlayerAction(&gPlayer);
            switch (gPlayer.action) {
                case ACT_MODEL_ONCE:
                    break;
                case ACT_MODEL_LOOP:
                    if (gPlayer.motionResult[0] == 0) {
                        gPlayer.curMotion[0] = MOT_OFF;
                    }
                    break;
            }
            func_800F0590();
            func_800F06E0();
            sAct3Coords[0].coord.t[0] = sCan.x;
            sAct3Coords[0].coord.t[1] = sCan.y;
            sAct3Coords[0].coord.t[2] = sCan.z;
            SetCoordRotation(&sCanRot[0], &sAct3Coords[0]);
            GsGetLs(sAct3Objs[0].coord2, &m);
            GsSetLsMatrix(&m);
            GsGetLw(sAct3Objs[0].coord2, &m);
            GsSetLightMatrix(&m);
            if (sRec.canInSub == 0) {
                GsSortObject4(&sAct3Objs[0], gCurOt, 2, (u_long *)0x1F800000);
            } else {
                GsSortObject4(&sAct3Objs[0], &gSubOt[gActiveBuff], 2, (u_long *)0x1F800000);
            }
            sAct3Coords[1].coord.t[0] = sCan.lidX;
            sAct3Coords[1].coord.t[1] = sCan.lidY;
            sAct3Coords[1].coord.t[2] = sCan.lidZ;
            SetCoordRotation(&sCanRot[1], &sAct3Coords[1]);
            GsGetLs(sAct3Objs[1].coord2, &m);
            GsSetLsMatrix(&m);
            GsGetLw(sAct3Objs[1].coord2, &m);
            GsSetLightMatrix(&m);
            GsSortObject4(&sAct3Objs[1], gCurOt, 2, (u_long *)0x1F800000);
            DrawSky(0);
            if (gPadTrig & PADstart) {
                gWorldIntroStep = 2;
            }
            break;
        case 2:
            gCourse.rot.vy = -0x400;
            gView[0] = 700;
            gView[1] = -200;
            gView[2] = 0;
            gView[3] = -500;
            gView[4] = 0;
            gView[5] = 0;
            GsSetRefView2((GsRVIEW2 *)gView);
            sCan.x = -0x974;
            sCan.y = -140;
            sCan.z = 0;
            sCanRot[0].vx = 0;
            sCanRot[0].vy = 0;
            sCanRot[0].vz = 0;
            gPlayer.x = -1800;
            gPlayer.speed = 30;
            gPlayer.motion[0] = 4;
            gPlayer.y = 0;
            gPlayer.z = 0;
            gPlayer.action = 0;
            gPlayer.control = 1;
            gPlayer.groundY = 0;
            gFloorY[0] = 0;
            sRec.depth = 0x1000;
            sRec.timer = 0;
            sRec.title = 0;
            gWorldIntroStep = 3;
            break;
        case 3:
            func_800F07E4();
            GsSetProjection(250);
            func_800F0234();
            gPlayer.z = gPlayer.z < -200 ? -200 : gPlayer.z > 200 ? 200 : gPlayer.z;
            SetChaseLights();
            UpdatePlayerAction(&gPlayer);
            DrawStepPlayer();
            if (sRec.timer == 0) {
                DrawPlayerShadow();
            }
            UnusedHook3();
            UpdateEffects((s32)&gSubOt[gActiveBuff], -gPlayer.x, 0, -gPlayer.z);
            gPlayer.groundY = 0;
            gFloorY[0] = 0;
            DrawRepeatingStage(-gPlayer.x, 0, -gPlayer.z, 30000);
            /* MATCHING: the base taken in a local keeps the -1 in a register. */
            o = sObjs;
            for (i = 0; i < 200; i++) {
                if (o[i].state != -1) {
                    D_800F7280[(s16)o[i].kind](&o[i]);
                }
            }
            sCan.x += 28;
            if (sCan.x + 620 < gPlayer.x) {
                sCan.x = gPlayer.x - 620;
            }
            SpawnEffect(4, sCan.x, -50 - rand() % 50, rand() % 500 - 250, 0);
            sAct3Coords[0].coord.t[0] = sCan.x + gWorldShift[0];
            /* MATCHING: a block-local temporary, as the function's own lands in
             * another register. */
            {
                s32 t;

                t = rsin(gFrameCount * 40960 / 360) * 100;
                sAct3Coords[0].coord.t[1] = sCan.y + gWorldShift[1] - ABS_GE(t / 4096);
            }
            sCanRot[0].vx = 0x800;
            sCanRot[0].vy = 0;
            sAct3Coords[0].coord.t[2] = sCan.z + gWorldShift[2];
            sCanRot[0].vz = -(gFrameCount * 49152 / 360);
            SetCoordRotation(&sCanRot[0], &sAct3Coords[0]);
            GsGetLs(sAct3Objs[0].coord2, &m);
            GsSetLsMatrix(&m);
            GsGetLw(sAct3Objs[0].coord2, &m);
            GsSetLightMatrix(&m);
            GsSortObject4(&sAct3Objs[0], &gSubOt[gActiveBuff], 2, (u_long *)0x1F800000);
            DrawSky(0);
            if (gPlayer.x < sCan.x + 120 && sRec.timer == 0) {
                sRec.timer = 1;
                gPlayer.motion[0] = 0x23;
            }
            if (gPlayer.x > 30000) {
                gWorldIntroStep = 4;
                sSubStep = 0;
                gPlayer.control = 0;
                sRec.title = 0;
                for (gLoopI = 0; gLoopI < 240; gLoopI++) {
                    sWipe[gLoopI].x = -(rand() % 64) - 160;
                    sWipe[gLoopI].speed = rand() % 16 + 16;
                }
            }
            if (sRec.timer != 0) {
                gPlayer.control = 0;
                if (sRec.timer == 20) {
                    sRec.depth = 0;
                    gPlayer.action = ACT_MODEL_ONCE;
                    gPlayer.speed = 0;
                    gPlayer.motion[0] = 0x46;
                }
                if (sRec.timer == 140) {
                    PlaceChaseObjects();
                    gWorldIntroStep = 0;
                }
                sRec.timer++;
            }
            break;
        case 4:
            func_800F09D8();
            switch (sSubStep) {
                case 0:
                    gPlayer.z += 10;
                    if (gPlayer.z > 200) {
                        gPlayer.z = 200;
                    }
                    if (gPlayer.x > 32300) {
                        sSubStep = 5;
                        StopCdTrack();
                        PlaySe(0x10);
                    }
                    break;
                case 5:
                    gView[0] = -(rsin(gCourse.rot.vy) * 700 / 4096);
                    gView[1] = -250;
                    gView[2] = -(rcos(gCourse.rot.vy) * 700 / 4096);
                    gView[3] = rsin(gCourse.rot.vy) * 500 / 4096;
                    gView[4] = 0;
                    gView[5] = rcos(gCourse.rot.vy) * 500 / 4096;
                    GsSetRefView2((GsRVIEW2 *)gView);
                    gCourse.rot.vy -= 11;
                    if ((gCourse.rot.vy & 0xFFF) < 0x9B1) {
                        gCourse.rot.vy = -0x64F;
                        sRec.timer = 0;
                        sSubStep = 10;
                        gRefView[0] = gView[0] + gPlayer.x;
                        gRefView[1] = gView[1] + gPlayer.y;
                        gRefView[2] = gView[2] + gPlayer.z;
                        gRefView[3] = gView[3] - gView[0];
                        gRefView[4] = gView[4] - gView[1];
                        gRefView[5] = gView[5] - gView[2];
                    }
                    break;
                case 10:
                    gView[1] = -250;
                    gView[4] = 0;
                    gView[0] = gRefView[0] - gPlayer.x;
                    gView[2] = gRefView[2] - gPlayer.z;
                    gView[3] = gRefView[3] + gView[0];
                    gView[5] = gRefView[5] + gView[2];
                    GsSetRefView2((GsRVIEW2 *)gView);
                    if (++sRec.timer > 30) {
                        sRec.timer = 0;
                        gPlayer.motion[0] = 0x13;
                        gPlayer.action = 0x17;
                        gPlayer.speed = 0;
                        sSubStep = 15;
                    }
                    break;
                case 15:
                    if (++sRec.timer > 30) {
                        gStage = 9;
                        gGameMode = 0x26;
                    }
                    break;
            }
            gPlayer.control = 0;
            GsSetProjection(250);
            func_800F0234();
            SetChaseLights();
            UpdatePlayerAction(&gPlayer);
            DrawTodObjects();
            DrawPlayerShadow();
            UpdateEffects((s32)&gSubOt[gActiveBuff], -gPlayer.x, 0, -gPlayer.z);
            gPlayer.groundY = 0;
            DrawRepeatingStage(-gPlayer.x, 0, -gPlayer.z, 30000);
            o = sObjs;
            for (i = 0; i < 200; i++) {
                if (o[i].state != -1) {
                    D_800F7280[(s16)o[i].kind](&o[i]);
                }
            }
            SpawnEffect(4, sCan.x, -50 - rand() % 50, rand() % 500 - 250, 0);
            sCan.x += 30;
            sAct3Coords[0].coord.t[0] = sCan.x + gWorldShift[0];
            t = rsin(gFrameCount * 40960 / 360) * 100;
            sAct3Coords[0].coord.t[1] = sCan.y + gWorldShift[1] - ABS_GE(t / 4096);
            sCanRot[0].vx = 0x800;
            sCanRot[0].vy = 0;
            sAct3Coords[0].coord.t[2] = sCan.z + gWorldShift[2];
            sCanRot[0].vz = -(gFrameCount * 49152 / 360);
            SetCoordRotation(&sCanRot[0], &sAct3Coords[0]);
            GsGetLs(sAct3Objs[0].coord2, &m);
            GsSetLsMatrix(&m);
            GsGetLw(sAct3Objs[0].coord2, &m);
            GsSetLightMatrix(&m);
            GsSortObject4(&sAct3Objs[0], &gOt[gActiveBuff], 2, (u_long *)0x1F800000);
            DrawSky(0);
            break;
        case 10:
            GsSetProjection(250);
            DrawRepeatingStage(-gPlayer.x, 0, -gPlayer.z, 30000);
            FntPrint("bp %d %d %d\n", sCan.x, sCan.y, sCan.z);
            FntPrint("br %d %d %d\n", sCanRot[0].vx, sCanRot[0].vy, sCanRot[0].vz);
            if (gPad & PADLup) {
                sCan.x -= 10;
            }
            if (gPad & PADLdown) {
                sCan.x += 10;
            }
            if (gPad & PADLleft) {
                sCan.z -= 10;
            }
            if (gPad & PADLright) {
                sCan.z += 10;
            }
            if (gPad & PADL1) {
                sCan.y -= 10;
            }
            if (gPad & PADL2) {
                sCan.y += 10;
            }
            if (gPad & PADRup) {
                sCanRot[0].vx -= 10;
            }
            if (gPad & PADRdown) {
                sCanRot[0].vx += 10;
            }
            if (gPad & PADRleft) {
                sCanRot[0].vz -= 10;
            }
            if (gPad & PADRright) {
                sCanRot[0].vz += 10;
            }
            if (gPad & PADR1) {
                sCanRot[0].vy -= 10;
            }
            if (gPad & PADR2) {
                sCanRot[0].vy += 10;
            }
            if (gPadTrig & PADselect) {
                gView[0] = -0x15EA;
                gView[1] = -0x3DE;
                gView[2] = 1000;
                gView[3] = -0x1630;
                gView[4] = -0x492;
                gView[5] = 0x64A;
                sCan.x = -0x1676;
                sCan.y = -0x4B0;
                sCan.z = 0x78A;
                sCanRot[0].vx = -0x23A;
                sCanRot[0].vy = -0x596;
                sCanRot[0].vz = 0x1FE;
            }
            GsSetRefView2((GsRVIEW2 *)gView);
            sAct3Coords[0].coord.t[0] = sCan.x + gWorldShift[0];
            sAct3Coords[0].coord.t[1] = sCan.y + gWorldShift[1];
            sAct3Coords[0].coord.t[2] = sCan.z + gWorldShift[2];
            SetCoordRotation(&sCanRot[0], &sAct3Coords[0]);
            GsGetLs(sAct3Objs[0].coord2, &m);
            GsSetLsMatrix(&m);
            GsGetLw(sAct3Objs[0].coord2, &m);
            GsSetLightMatrix(&m);
            GsSortObject4(&sAct3Objs[0], gCurOt, 2, (u_long *)0x1F800000);
            break;
    }
}

#include "chase_tail.inc"
