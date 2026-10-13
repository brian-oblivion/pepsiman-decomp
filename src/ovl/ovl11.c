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
#include "spad.h"


INCLUDE_ASM("asm/ovl/ovl11/nonmatchings/ovl11", func_800F0234);

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
#ifdef NON_MATCHING
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
            color.g = color.b = color.cd = D_800D39D8.title < 16 ? D_800D39D8.title * 8 : 0x80;
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
            quad[5] = quad[4] = (t << 9) + 0x1000;
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
#else
INCLUDE_ASM("asm/ovl/ovl11/nonmatchings/ovl11", func_800F07E4);
#endif

INCLUDE_ASM("asm/ovl/ovl11/nonmatchings/ovl11", func_800F09D8);

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

INCLUDE_RODATA("asm/ovl/ovl11/nonmatchings/ovl11", D_800F0000);

INCLUDE_ASM("asm/ovl/ovl11/nonmatchings/ovl11", func_800F42E8);

#include "chase_tail.inc"
