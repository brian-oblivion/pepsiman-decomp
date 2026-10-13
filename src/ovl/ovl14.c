/* Stage 4-3's overlay: the act-3 chase of world 4, built from
 * chase.inc with its own objects and frame step. */

#define CHASE_LENGTH 10240000
#define CHASE_PLACEMENTS 53
#define CHASE_VAB 5
#define CHASE_LIGHT_BLUE 0x80
#define CHASE_FAR_COLOR 0xB0, 0xB0, 0xB0

#include "chase.inc"
#include "rand.h"

#define ABS(x) ((x) < 0 ? -(x) : (x))

/* Starts a jump on a face button, unless the play is frozen. */
#ifdef NON_MATCHING
void StartChaseJump(void) {
    if (ClipF == 0 && (gGamePadTrig & (PADRleft | PADRdown | PADRright))) {
        sJumpSpeed = -40;
        gPlayer.slideState = 1;
        gPlayer.unk38C = 0;
        gPlayer.curMotion[0] = MOT_OFF;
        PlaySe(0x17);
    }
}
#else
INCLUDE_ASM("asm/ovl/ovl14/nonmatchings/ovl14", StartChaseJump);
#endif

#include "chase_head.inc"

/* Piece model 0, which comes from behind the player, rolling, and hurts it
 * as it passes. */
s32 func_800F0BE0(Act3Obj *o) {
    s32 unused[2];
    MATRIX m;
    s32 x;

    x = gPlayer.x - 1200;
    if (o->x <= x || o->state != 0) {
        switch (o->state) {
            case 0:
                o->y = -25;
                o->x = x;
                o->state = 1;
                break;
            case 1:
                o->timer++;
                o->x += 60;
                GsInitCoordinate2(WORLD, &sPieceCoords[10]);
                sPieceCoords[10].coord.t[0] = o->x + gWorldShift[0];
                sPieceCoords[10].coord.t[1] = o->y + gWorldShift[1];
                sPieceCoords[10].coord.t[2] = o->z + gWorldShift[2];
                sPieceCoords[10].flg = 0;
                sPieceRots[10].vx = 0;
                sPieceRots[10].vy = 0;
                sPieceRots[10].vz = o->timer * 40960 / 360;
                SetCoordRotation(&sPieceRots[10], &sPieceCoords[10]);
                GsGetLs(&sPieceCoords[10], &m);
                GsSetLsMatrix(&m);
                GsGetLw(&sPieceCoords[10], &m);
                GsSetLightMatrix(&m);
                GsSortObject4(&sPieceObjs[10], &gSubOt[gActiveBuff], 2, (u_long *)0x1F800000);
                if (o->x - 35 < gPlayer.x && gPlayer.x < o->x + 35 && o->z - 140 < gPlayer.z &&
                    gPlayer.z < o->z + 140 && o->y - 25 < gPlayer.y && gPlayer.y < o->y + 205) {
                    PlaySe(0x38);
                    gPlayer.action = 9;
                    EndJump();
                    if (gPlayer.slideState < 2 && sHurtTimer == 0) {
                        sHurtTimer = 60;
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
                if (o->x >= gPlayer.x + 500) {
                    o->state = -1;
                }
                break;
        }
    }
}

/* The same, bouncing as it rolls. */
s32 func_800F0EA8(Act3Obj *o) {
    s32 unused[2];
    MATRIX m;
    s32 x;

    x = gPlayer.x - 1200;
    if (o->x <= x || o->state != 0) {
        switch (o->state) {
            case 0:
                o->y = -25;
                o->x = x;
                o->state = 1;
                break;
            case 1:
                o->timer++;
                o->x += 60;
                o->y = -(ABS(rcos(o->timer * 20480 / 360)) * 300 / 4096) - 25;
                GsInitCoordinate2(WORLD, &sPieceCoords[10]);
                sPieceCoords[10].coord.t[0] = o->x + gWorldShift[0];
                sPieceCoords[10].coord.t[1] = o->y + gWorldShift[1];
                sPieceCoords[10].coord.t[2] = o->z + gWorldShift[2];
                sPieceCoords[10].flg = 0;
                sPieceRots[10].vx = 0;
                sPieceRots[10].vy = 0;
                sPieceRots[10].vz = o->timer * 40960 / 360;
                SetCoordRotation(&sPieceRots[10], &sPieceCoords[10]);
                GsGetLs(&sPieceCoords[10], &m);
                GsSetLsMatrix(&m);
                GsGetLw(&sPieceCoords[10], &m);
                GsSetLightMatrix(&m);
                GsSortObject4(&sPieceObjs[10], &gSubOt[gActiveBuff], 2, (u_long *)0x1F800000);
                if (o->x - 35 < gPlayer.x && gPlayer.x < o->x + 35 && o->z - 140 < gPlayer.z &&
                    gPlayer.z < o->z + 140 && o->y - 25 < gPlayer.y && gPlayer.y < o->y + 205) {
                    PlaySe(0x38);
                    gPlayer.action = 9;
                    EndJump();
                    if (gPlayer.slideState < 2 && sHurtTimer == 0) {
                        sHurtTimer = 60;
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
                if (o->x >= gPlayer.x + 500) {
                    o->state = -1;
                }
                break;
        }
    }
}

/* Draws the four-part model at (@p x, @p y, @p z), its parts swinging with
 * the frame count offset by @p n, and throws up dust under it. */
void func_800F1248(s16 x, s16 y, s16 z, s32 n) {
    MATRIX m;
    SVECTOR rot;
    SVECTOR pos;
    s32 t;
    s32 a;
    s32 b;

    pos.vx = x;
    pos.vy = y;
    pos.vz = z;
    t = gFrameCount + n * 10;
    SpawnEffect(4, pos.vx + rand() % 100 - 50, rand() % 20 - 20, pos.vz + rand() % 100 - 50, 0);
    rot.vx = 0;
    rot.vy = 0;
    sPieceCoords[11].coord.t[0] = pos.vx;
    sPieceCoords[11].coord.t[1] = pos.vy;
    sPieceCoords[11].coord.t[2] = pos.vz;
    a = -t * 122880 / 360;
    rot.vz = rsin(a) / 20;
    SetCoordRotation(&rot, &sPieceCoords[11]);
    GsGetLs(sPieceObjs[11].coord2, &m);
    GsSetLsMatrix(&m);
    GsGetLw(sPieceObjs[11].coord2, &m);
    GsSetLightMatrix(&m);
    GsSortObject4(&sPieceObjs[11], &gSubOt[gActiveBuff], 2, (u_long *)0x1F800000);
    sPieceCoords[12].coord.t[0] = 100;
    sPieceCoords[12].coord.t[1] = -100;
    sPieceCoords[12].coord.t[2] = 0;
    rot.vx = 0;
    rot.vy = 0;
    b = t * 122880 / 360;
    rot.vz = rsin(b) / 10;
    SetCoordRotation(&rot, &sPieceCoords[12]);
    GsGetLs(sPieceObjs[12].coord2, &m);
    GsSetLsMatrix(&m);
    GsGetLw(sPieceObjs[12].coord2, &m);
    GsSetLightMatrix(&m);
    GsSortObject4(&sPieceObjs[12], &gSubOt[gActiveBuff], 2, (u_long *)0x1F800000);
    sPieceCoords[13].coord.t[0] = 33;
    sPieceCoords[13].coord.t[1] = -75;
    sPieceCoords[13].coord.t[2] = 0;
    rot.vx = 0;
    rot.vy = 0;
    rot.vz = rsin(a) / 8;
    SetCoordRotation(&rot, &sPieceCoords[13]);
    GsGetLs(sPieceObjs[13].coord2, &m);
    GsSetLsMatrix(&m);
    GsGetLw(sPieceObjs[13].coord2, &m);
    GsSetLightMatrix(&m);
    GsSortObject4(&sPieceObjs[13], &gSubOt[gActiveBuff], 2, (u_long *)0x1F800000);
    sPieceCoords[14].coord.t[0] = -80;
    sPieceCoords[14].coord.t[1] = -88;
    sPieceCoords[14].coord.t[2] = 0;
    rot.vx = 0;
    rot.vy = 0;
    rot.vz = rsin(b) / 5;
    SetCoordRotation(&rot, &sPieceCoords[14]);
    GsGetLs(sPieceObjs[14].coord2, &m);
    GsSetLsMatrix(&m);
    GsGetLw(sPieceObjs[14].coord2, &m);
    GsSetLightMatrix(&m);
    GsSortObject4(&sPieceObjs[14], &gSubOt[gActiveBuff], 2, (u_long *)0x1F800000);
}

INCLUDE_ASM("asm/ovl/ovl14/nonmatchings/ovl14", func_800F1654);

#include "chase_tail.inc"
