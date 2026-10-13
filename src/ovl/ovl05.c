/* Stage 1-3's overlay: the act-3 chase, where the player runs toward the
 * camera, jumps and steps sideways, and the stage's own objects come at
 * it. */

#include "common.h"
#include "libgte.h"
#include "libgpu.h"
#include "libgs.h"
#include "libcd.h"
#include "libetc.h"
#include "model.h"
#include "draw.h"
#include "sound.h"
#include "object.h"
#include "world_intro.h"

/** @brief An act-3 object, as this overlay views gTraps: 20 bytes. */
typedef struct {
    s32 x;     /**< position x */
    s32 y;     /**< position y */
    s32 z;     /**< position z, along the run */
    s16 state; /**< -1 when free, 0 placed */
    s16 timer; /**< frames in its state */
    u16 kind;  /**< which object it is */
    s16 speed; /**< how fast it flies once hit */
} Act3Obj;

/** @brief A placed object of the stage's object list: 16 bytes. */
typedef struct {
    u16 kind;  /**< which object */
    u8 pad[2]; /**< alignment */
    s32 x;     /**< position x */
    s32 y;     /**< position y */
    s32 z;     /**< position z */
} Act3Place;

#define sObjs ((Act3Obj *)gTraps)

/* MATCHING: views of routines their units declare only where defined. */
void SetBootReady(s32 arg);
s32 LoadVabBank(s32 bank);
void LoadAct3Data(unsigned long *p);

extern GsCOORDINATE2 sPieceCoords[];
extern GsDOBJ2 sPieceObjs[];
extern SVECTOR sPieceRots[];
extern s32 D_8009EA78; /* the near edge of the drawn run */
extern u8 gRumbleEnabled;
extern s16 gRumbleTime;
extern s16 gRumbleSmall[];
/* MATCHING: stage defines it with s16 positions; this unit passes words. */
s32 SpawnEffect(s32 kind, s32 x, s32 y, s32 z, s32 delay);
extern s32 D_800F7B04; /* the jump's vertical speed */
extern s32 D_800F7B08;
extern s32 D_800F80B0; /* frames the player flashes after a hit */
extern Act3Place D_800F7490[103];
extern CdlLOC D_800F7D04[];
extern s32 D_800F7D28[];

/** @brief The three flat lights of the scene. */
typedef struct {
    GsF_LIGHT l[3]; /**< one per light index */
} LightTable;

/* MATCHING: a struct lvalue keeps the table's base in a register. */
#define sLights (*(LightTable *)gFlatLights)


/* Starts a jump on a face button, unless the play is frozen. */
#ifdef NON_MATCHING
void func_800F01F0(void) {
    if (ClipF == 0 && (gGamePadTrig & (PADRleft | PADRdown | PADRright))) {
        D_800F7B04 = -40;
        gPlayer.slideState = 1;
        gPlayer.unk38C = 0;
        gPlayer.curMotion[0] = MOT_OFF;
        PlaySe(0x17);
    }
}
#else
INCLUDE_ASM("asm/ovl/ovl05/nonmatchings/ovl05", func_800F01F0);
#endif

void func_800F01F0(void);

/* The player's step in the chase: gravity, the jump, the hit and the
 * sidestep. */
/* MATCHING: non-void with no return keeps six delay slots nops. */
s32 func_800F0258(void) {
    if (D_800F80B0 != 0) {
        D_800F80B0--;
    }
    gPlayer.y += D_800F7B04;
    D_800F7B04 += 5;
    if (gPlayer.y >= 0) {
        gPlayer.y = 0;
        D_800F7B04 = 5;
        gPlayer.airborne = 0;
    } else {
        gPlayer.airborne = 1;
    }
    switch ((s8)gPlayer.slideState) {
        case 0:
            gPlayer.speed = 30;
            gPlayer.motion[0] = 0x33;
            gPlayer.action = ACT_MODEL_LOOP;
            func_800F01F0();
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
                D_800F7B08 = 0;
            }
            break;
    }
}

/* Draws the player's model at its position and yaw. */
void func_800F04A0(void) {
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
void func_800F05F0(s32 sz) {
    MATRIX m;
    SVECTOR rot;
    VECTOR scale;
    GsDOBJ2 *o;
    s32 i;

    if (D_800F80B0 / 2 % 2 != 1) {
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
void func_800F0790(void) {
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
void func_800F087C(void) {
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
void func_800F097C(void) {
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
    pos.vx = n * 0x5800 / 7680000 + 0x34;
    DrawSprite((gFrameCount >> 2 & 3) + 0x12F, &pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
    pos.vx = 0x34;
    pos.vy = -100;
    DrawSprite(0x12E, &pos, NULL, 0, &gSubOt[gActiveBuff]);
}

/* An object of piece model 0: drawn while it is near the player; it is
 * knocked away as the run reaches it, and hits the player it touches. */
s32 func_800F0BB8(Act3Obj *o) {
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
                if (o->x < D_8009EA78 + 120) {
                    PlaySe(0x38);
                    o->state = 1;
                    o->timer = 0;
                    o->speed = 45;
                }
                if (o->x - 20 < gPlayer.x && gPlayer.x < o->x + 20 && o->z - 80 < gPlayer.z &&
                    gPlayer.z < o->z + 80 && o->y - 80 < gPlayer.y) {
                    PlaySe(0x38);
                    gPlayer.action = 9;
                    EndJump();
                    if (gPlayer.slideState < 2) {
                        if (D_800F80B0 == 0) {
                            D_800F80B0 = 60;
                            if (gRumbleEnabled) {
                                gRumbleTime = 15;
                                gRumbleSmall[0] = 200;
                                gRumbleSmall[1] = 200;
                            }
                            gPlayer.slideState = 2;
                            gPlayer.unk38C = 0;
                            gPlayer.curMotion[0] = MOT_OFF;
                        }
                    }
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
s32 func_800F103C(Act3Obj *o) {
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
                if (o->x < D_8009EA78 + 120) {
                    PlaySe(0x3B);
                    o->state = 1;
                    o->timer = 0;
                    o->speed = 45;
                }
                if (o->x - 20 < gPlayer.x && gPlayer.x < o->x + 20 && o->z - 60 < gPlayer.z &&
                    gPlayer.z < o->z + 60 && o->y - 120 < gPlayer.y) {
                    PlaySe(0x3B);
                    gPlayer.action = 9;
                    EndJump();
                    if (gPlayer.slideState < 2) {
                        if (D_800F80B0 == 0) {
                            D_800F80B0 = 60;
                            if (gRumbleEnabled) {
                                gRumbleTime = 15;
                                gRumbleSmall[0] = 200;
                                gRumbleSmall[1] = 200;
                            }
                            gPlayer.slideState = 2;
                            gPlayer.unk38C = 0;
                            gPlayer.curMotion[0] = MOT_OFF;
                        }
                    }
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
s32 func_800F14C0(Act3Obj *o) {
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
                if (o->x < D_8009EA78 + 120) {
                    PlaySe(0x3B);
                    o->state = 1;
                    o->timer = 0;
                    o->speed = 45;
                }
                if (o->x - 20 < gPlayer.x && gPlayer.x < o->x + 20 && o->z - 70 < gPlayer.z &&
                    gPlayer.z < o->z + 70 && o->y - 200 < gPlayer.y) {
                    PlaySe(0x3B);
                    gPlayer.action = 9;
                    EndJump();
                    if (gPlayer.slideState < 2) {
                        if (D_800F80B0 == 0) {
                            D_800F80B0 = 60;
                            if (gRumbleEnabled) {
                                gRumbleTime = 15;
                                gRumbleSmall[0] = 200;
                                gRumbleSmall[1] = 200;
                            }
                            gPlayer.slideState = 2;
                            gPlayer.unk38C = 0;
                            gPlayer.curMotion[0] = MOT_OFF;
                        }
                    }
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
s32 func_800F1944(Act3Obj *o) {
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
                if (o->x < D_8009EA78 + 120) {
                    PlaySe(0x38);
                    o->state = 1;
                    o->timer = 0;
                    o->speed = 45;
                }
                if (o->x - 50 < gPlayer.x && gPlayer.x < o->x + 50 && o->z - 60 < gPlayer.z &&
                    gPlayer.z < o->z + 60 && o->y - 80 < gPlayer.y) {
                    PlaySe(0x38);
                    gPlayer.action = 9;
                    EndJump();
                    if (gPlayer.slideState < 2) {
                        if (D_800F80B0 == 0) {
                            D_800F80B0 = 60;
                            if (gRumbleEnabled) {
                                gRumbleTime = 15;
                                gRumbleSmall[0] = 200;
                                gRumbleSmall[1] = 200;
                            }
                            gPlayer.slideState = 2;
                            gPlayer.unk38C = 0;
                            gPlayer.curMotion[0] = MOT_OFF;
                        }
                    }
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
s32 func_800F1DFC(Act3Obj *o) {
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
                if (o->x < D_8009EA78 + 120) {
                    PlaySe(0x3A);
                    o->state = 1;
                    o->timer = 0;
                    o->speed = 45;
                }
                if (o->x - 20 < gPlayer.x && gPlayer.x < o->x + 20 && o->z - 95 < gPlayer.z &&
                    gPlayer.z < o->z + 95 && o->y - 100 < gPlayer.y) {
                    PlaySe(0x3A);
                    gPlayer.action = 9;
                    EndJump();
                    if (gPlayer.slideState < 2) {
                        if (D_800F80B0 == 0) {
                            D_800F80B0 = 60;
                            if (gRumbleEnabled) {
                                gRumbleTime = 15;
                                gRumbleSmall[0] = 200;
                                gRumbleSmall[1] = 200;
                            }
                            gPlayer.slideState = 2;
                            gPlayer.unk38C = 0;
                            gPlayer.curMotion[0] = MOT_OFF;
                        }
                    }
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
s32 func_800F2280(Act3Obj *o) {
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
                if (o->x < D_8009EA78 + 120) {
                    PlaySe(0x39);
                    o->state = 1;
                    o->timer = 0;
                    o->speed = 45;
                }
                if (o->x - 35 < gPlayer.x && gPlayer.x < o->x + 35 && o->z - 45 < gPlayer.z &&
                    gPlayer.z < o->z + 45 && o->y - 80 < gPlayer.y) {
                    PlaySe(0x39);
                    gPlayer.action = 9;
                    EndJump();
                    if (gPlayer.slideState < 2) {
                        if (D_800F80B0 == 0) {
                            D_800F80B0 = 60;
                            if (gRumbleEnabled) {
                                gRumbleTime = 15;
                                gRumbleSmall[0] = 200;
                                gRumbleSmall[1] = 200;
                            }
                            gPlayer.slideState = 2;
                            gPlayer.unk38C = 0;
                            gPlayer.curMotion[0] = MOT_OFF;
                        }
                    }
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
s32 func_800F2738(Act3Obj *o) {
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
                if (o->x < D_8009EA78 + 120) {
                    PlaySe(0x3A);
                    o->state = 1;
                    o->timer = 0;
                    o->speed = 45;
                }
                if (o->x - 20 < gPlayer.x && gPlayer.x < o->x + 20 && o->z - 105 < gPlayer.z &&
                    gPlayer.z < o->z + 105 && o->y - 90 < gPlayer.y) {
                    PlaySe(0x3A);
                    gPlayer.action = 9;
                    EndJump();
                    if (gPlayer.slideState < 2) {
                        if (D_800F80B0 == 0) {
                            D_800F80B0 = 60;
                            if (gRumbleEnabled) {
                                gRumbleTime = 15;
                                gRumbleSmall[0] = 200;
                                gRumbleSmall[1] = 200;
                            }
                            gPlayer.slideState = 2;
                            gPlayer.unk38C = 0;
                            gPlayer.curMotion[0] = MOT_OFF;
                        }
                    }
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
s32 func_800F2BBC(Act3Obj *o) {
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
                if (o->x < D_8009EA78 + 120) {
                    PlaySe(0x39);
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

/* The same for piece model 8, a wall the player passes by sliding
 * under it. */
s32 func_800F2FC8(Act3Obj *o) {
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
                if (o->x < D_8009EA78 + 120) {
                    PlaySe(0x3B);
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
                    PlaySe(0x3B);
                    gPlayer.action = 9;
                    EndJump();
                    if (gPlayer.slideState < 2) {
                        if (D_800F80B0 == 0) {
                            D_800F80B0 = 60;
                            if (gRumbleEnabled) {
                                gRumbleTime = 15;
                                gRumbleSmall[0] = 200;
                                gRumbleSmall[1] = 200;
                            }
                            gPlayer.slideState = 2;
                            gPlayer.unk38C = 0;
                            gPlayer.curMotion[0] = MOT_OFF;
                        }
                    }
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

/* The same for piece model 9, a wall the player passes by sliding
 * under it. */
s32 func_800F3480(Act3Obj *o) {
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
                if (o->x < D_8009EA78 + 120) {
                    PlaySe(0x3B);
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
                    PlaySe(0x3B);
                    gPlayer.action = 9;
                    EndJump();
                    if (gPlayer.slideState < 2) {
                        if (D_800F80B0 == 0) {
                            D_800F80B0 = 60;
                            if (gRumbleEnabled) {
                                gRumbleTime = 15;
                                gRumbleSmall[0] = 200;
                                gRumbleSmall[1] = 200;
                            }
                            gPlayer.slideState = 2;
                            gPlayer.unk38C = 0;
                            gPlayer.curMotion[0] = MOT_OFF;
                        }
                    }
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

/* The same for piece model 10, a wall the player passes by sliding
 * under it. */
s32 func_800F3938(Act3Obj *o) {
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
                if (o->x < D_8009EA78 + 120) {
                    PlaySe(0x3B);
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
                    PlaySe(0x3B);
                    gPlayer.action = 9;
                    EndJump();
                    if (gPlayer.slideState < 2) {
                        if (D_800F80B0 == 0) {
                            D_800F80B0 = 60;
                            if (gRumbleEnabled) {
                                gRumbleTime = 15;
                                gRumbleSmall[0] = 200;
                                gRumbleSmall[1] = 200;
                            }
                            gPlayer.slideState = 2;
                            gPlayer.unk38C = 0;
                            gPlayer.curMotion[0] = MOT_OFF;
                        }
                    }
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

/* The same for piece model 11, a wall the player passes by sliding
 * under it. */
s32 func_800F3DF0(Act3Obj *o) {
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
                if (o->x < D_8009EA78 + 120) {
                    PlaySe(0x3B);
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
                    PlaySe(0x3B);
                    gPlayer.action = 9;
                    EndJump();
                    if (gPlayer.slideState < 2) {
                        if (D_800F80B0 == 0) {
                            D_800F80B0 = 60;
                            if (gRumbleEnabled) {
                                gRumbleTime = 15;
                                gRumbleSmall[0] = 200;
                                gRumbleSmall[1] = 200;
                            }
                            gPlayer.slideState = 2;
                            gPlayer.unk38C = 0;
                            gPlayer.curMotion[0] = MOT_OFF;
                        }
                    }
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
s32 func_800F42A8(Act3Obj *o) {
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
                    PlaySe(0x3B);
                    o->state = 1;
                    o->timer = 0;
                    o->speed = 0;
                }
                if (o->x < D_8009EA78 + 120) {
                    PlaySe(0x3B);
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
                    PlaySe(0x3B);
                    gPlayer.action = 9;
                    EndJump();
                    if (gPlayer.slideState < 2) {
                        if (D_800F80B0 == 0) {
                            D_800F80B0 = 60;
                            if (gRumbleEnabled) {
                                gRumbleTime = 15;
                                gRumbleSmall[0] = 200;
                                gRumbleSmall[1] = 200;
                            }
                            gPlayer.slideState = 2;
                            gPlayer.unk38C = 0;
                            gPlayer.curMotion[0] = MOT_OFF;
                        }
                    }
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

INCLUDE_ASM("asm/ovl/ovl05/nonmatchings/ovl05", func_800F4794);

/* Sets the scene's three lights and its fog. */
void func_800F6B8C(void) {
    sLights.l[0].r = 0x80;
    sLights.l[0].g = 0x80;
    sLights.l[0].b = 0x80;
    sLights.l[1].r = 0x80;
    sLights.l[1].g = 0x80;
    sLights.l[1].b = 0x80;
    sLights.l[2].r = 0x80;
    sLights.l[2].g = 0x80;
    sLights.l[2].b = 0x80;
    sLights.l[0].vx = 0;
    sLights.l[0].vy = 100;
    sLights.l[0].vz = 100;
    GsSetFlatLight(0, &sLights.l[0]);
    sLights.l[1].vx = 0x56;
    sLights.l[1].vy = 100;
    sLights.l[1].vz = -50;
    GsSetFlatLight(1, &sLights.l[1]);
    sLights.l[2].vx = -0x56;
    sLights.l[2].vy = 100;
    sLights.l[2].vz = -50;
    GsSetFlatLight(2, &sLights.l[2]);
    SetFogNearFar(3000, 8000, 250);
    SetFarColor(0xB0, 0xB0, 0xB0);
}

/* Frees every object, then places the stage's 103. */
void func_800F6C60(void) {
    Act3Obj *o;
    s32 i;

    o = sObjs;
    for (i = 0; i < 200; i++) {
        o[i].state = -1;
        o[i].timer = 0;
    }
    for (i = 0; i < sizeof(D_800F7490) / sizeof(D_800F7490[0]); i++) {
        o[i].kind = D_800F7490[i].kind;
        o[i].x = D_800F7490[i].x;
        o[i].y = D_800F7490[i].y;
        o[i].z = D_800F7490[i].z;
        o[i].state = 0;
    }
}

/* Loads the stage: its textures, its act-3 data and sound bank. */
s32 func_800F6CE4(void) {
    /* MATCHING: an s16 result keeps gSubStep's increment off its register;
     * an unused 8-byte local gives retail's frame. */
    s16 ret = 0;
    s32 unused[2];
    u32 i;

    switch (gSubStep) {
        case 0:
            SetBootReady(0);
            gCdLoadLoc[0] = (s32)&D_800F7D04[0];
            gCdLoadDest[0] = 0x8019D000;
            gCdLoadCount = 1;
            gCdLoadState = CD_LOAD_SEEK;
            gCdLoadSectors[0] = D_800F7D28[0];
            gSubStep++;
            break;
        case 1:
            if (gCdLoadState == CD_LOAD_DONE) {
                LoadTimList((unsigned long *)0x8019D000);
                gSubStep++;
            }
            break;
        case 2:
            gCdLoadLoc[0] = (s32)&D_800F7D04[1];
            gCdLoadDest[0] = 0x8019D000;
            gCdLoadCount = 1;
            gCdLoadState = CD_LOAD_SEEK;
            gCdLoadSectors[0] = D_800F7D28[1];
            gSubStep++;
            break;
        case 3:
            if (gCdLoadState == CD_LOAD_DONE) {
                LoadAct3Data((unsigned long *)0x8019D000);
                gSubStep++;
            }
            break;
        case 4:
            if (LoadVabBank(2) == 1) {
                gCdLoadState = CD_LOAD_DONE;
                gSubStep++;
            }
            break;
        case 5:
        case 8:
            if (gCdLoadState == CD_LOAD_DONE) {
                gSubStep++;
            }
            break;
        case 6:
            gSectionCount = 0;
            gSubStep++;
            break;
        case 7:
            gCdLoadState = CD_LOAD_DONE;
            gSubStep++;
            break;
        case 9:
            for (i = 0; i < 200; i++) {
                ((Trap *)gTraps)[i].model = -1;
                ((Trap *)gTraps)[i].switchNo = -1;
                ((Trap *)gTraps)[i].marks = 1;
            }
            gCdLoadState = CD_LOAD_DONE;
            gSubStep++;
            break;
        case 10:
            gSubStep++;
            break;
        case 11:
            ret = 1;
            func_800F6C60();
            SetBootReady(1);
            break;
    }
    FntPrint("CDEXEC %d", gSubStep);
    return ret;
}

/* Resets the stage: places its objects again. */
void func_800F6F0C(void) {
    func_800F6C60();
    gWorldIntroStep = 0;
}
