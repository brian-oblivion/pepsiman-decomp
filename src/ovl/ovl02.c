#include "common.h"
#include "libgte.h"
#include "libgpu.h"
#include "libgs.h"
#include "libetc.h"
#include "model.h"
#include "draw.h"
#include "spad.h"
#include "world_intro.h"

/** @brief The three flat lights of the scene. */
typedef struct {
    GsF_LIGHT l[3]; /**< one per light index */
} LightTable;

/* MATCHING: a struct lvalue keeps the table's base in a register. */
#define sLights (*(LightTable *)gFlatLights)

extern s32 D_800FA220;
extern s32 D_800FA28C;
void func_800F83E0(void);
void func_800F8898(void);
void func_800F88B0(void);

void func_800F01EC(void) {
    SVECTOR pos;
    SColor color;

    switch (gSubStep) {
        case 50:
            DefineSprite(0x1E, 0, 0x80, 0xF0, 0x15, 0, 0, 0, 0x1F3);
            DefineSprite(0x1F, 0, 0x80, 0xF0, 0x15, 0x80, 0, 0, 0x1F3);
            DefineSprite(0x20, 0, 0x40, 0xF0, 0x16, 0, 0, 0, 0x1F3);
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
            gPlayer.action = 0x61;
            gPlayer.motion[0] = 3;
            gPlayer.z = -600;
            gPlayer.speed = 0;
            gPlayer.x = 0;
            gPlayer.y = 100;
            gPlayer.yaw = 0x800;
            sView.vpx = 500;
            sView.vpy = -100;
            sView.vpz = 0;
            sView.vrx = 0;
            sView.vry = 0;
            sView.vrz = 0;
            D_800FA220 = 0;
            gSubStep = 100;
            func_800F8898();
            break;
        case 100:
            func_800F83E0();
            color.r = -1;
            color.g = color.b = color.cd = 0x80;
            pos.vx = -160;
            pos.vy = -120;
            DrawSprite(0x1E, &pos, (CVECTOR *)&color, 1, gCurOt);
            pos.vx += 0x80;
            DrawSprite(0x1F, &pos, (CVECTOR *)&color, 1, gCurOt);
            pos.vx += 0x80;
            DrawSprite(0x20, &pos, (CVECTOR *)&color, 1, gCurOt);
            if (D_800FA28C == 1) {
                D_800FA220++;
                gPlayer.z += 20;
                if (D_800FA220 >= 60) {
                    func_800F88B0();
                }
            }
            if (D_800FA28C == 0) {
                gModeStep = 5;
            }
            if (gPadTrig & PADstart) {
                gModeStep = 5;
            }
            break;
    }
}

INCLUDE_ASM("asm/ovl/ovl02/nonmatchings/ovl02", func_800F04EC);

INCLUDE_ASM("asm/ovl/ovl02/nonmatchings/ovl02", func_800F0AD4);

INCLUDE_ASM("asm/ovl/ovl02/nonmatchings/ovl02", func_800F0E58);

extern GsCOORDINATE2 sAct3Coords[];
extern GsDOBJ2 sAct3Objs[];
extern VECTOR D_8009EA78[];
extern SVECTOR D_8009EB58[];

void func_800F1494(s32 i) {
    MATRIX m;

    sAct3Coords[i].coord.t[0] = D_8009EA78[i].vx;
    sAct3Coords[i].coord.t[1] = D_8009EA78[i].vy;
    sAct3Coords[i].coord.t[2] = D_8009EA78[i].vz;
    SetCoordRotation(&D_8009EB58[i], &sAct3Coords[i]);
    GsGetLs(sAct3Objs[i].coord2, &m);
    GsSetLsMatrix(&m);
    GsGetLw(sAct3Objs[i].coord2, &m);
    GsSetLightMatrix(&m);
    SetTmdSortMode(0);
    GsSortObject4J(&sAct3Objs[i], &gSubOt[gActiveBuff], 2, (u_long *)0x1F800000);
}

INCLUDE_RODATA("asm/ovl/ovl02/nonmatchings/ovl02", D_800F0000);

INCLUDE_ASM("asm/ovl/ovl02/nonmatchings/ovl02", func_800F1570);

INCLUDE_ASM("asm/ovl/ovl02/nonmatchings/ovl02", func_800F6670);

INCLUDE_ASM("asm/ovl/ovl02/nonmatchings/ovl02", func_800F69B0);

INCLUDE_ASM("asm/ovl/ovl02/nonmatchings/ovl02", func_800F7A4C);

void func_800F80F8(void) {
    GsCOORDINATE2 c;
    MATRIX m;
    SVECTOR size;
    SVECTOR unused;
    SColor color;

    GsInitCoordinate2(WORLD, &c);
    c.coord.t[1] = 0;
    c.coord.t[0] = gPlayer.x;
    c.coord.t[2] = gPlayer.z;
    GsGetLs(&c, &m);
    GsSetLsMatrix(&m);
    size.vx = 100;
    size.vy = 50;
    color.r = BLEND_SUB;
    color.g = color.b = color.cd = 0x40;
    DrawBillboard(0x12D, &size, (CVECTOR *)&color, 2, &gSubOt[gActiveBuff]);
}

void func_800F81B4(void) {
    GsCOORDINATE2 c;
    MATRIX m;
    SVECTOR size;
    SVECTOR unused;
    SColor color;

    GsInitCoordinate2(WORLD, &c);
    c.coord.t[0] = 0x2B5C;
    c.coord.t[1] = 0;
    c.coord.t[2] = 0;
    GsGetLs(&c, &m);
    GsSetLsMatrix(&m);
    size.vx = 800;
    size.vy = 50;
    color.r = BLEND_SUB;
    color.g = color.b = color.cd = 0x60;
    DrawBillboard(0x12D, &size, (CVECTOR *)&color, 2, &gSubOt[gActiveBuff]);
}

void func_800F8264(void) {
    MATRIX m;
    SVECTOR rot;
    GsDOBJ2 *o;
    s32 i;

    SetTmdSortMode(1);
    o = ((DrawList *)gTodObjTable)->objs;
    o->coord2->coord.t[0] = gPlayer.x;
    o->coord2->coord.t[1] = gPlayer.y;
    o->coord2->coord.t[2] = gPlayer.z;
    rot.vx = 0;
    rot.vz = 0;
    rot.vy = gPlayer.yaw;
    SetCoordRotation(&rot, o->coord2);
    for (i = 0; i < ((DrawList *)gTodObjTable)->count; i++, o++) {
        o->coord2->flg = 0;
        if (o->id != -1 && o->tmd != NULL) {
            GsGetLs(o->coord2, &m);
            GsSetLsMatrix(&m);
            GsGetLw(o->coord2, &m);
            GsSetLightMatrix(&m);
            SetSpadStack();
            GsSortObject4J(o, &gSubOt[gActiveBuff], 2, (u_long *)0x1F800000);
            ResetSpadStack();
        }
        /* MATCHING: as in world_intro.inc's model draw, three barriers rank
         * the object pointer above the counter (retail's s1 and s2). */
        __asm__("");
        __asm__("");
        __asm__("");
    }
}

extern s32 D_800FA288;

void func_800F83E0(void) {
    GsOT *ot;
    POLY_F4 *p;
    DR_TPAGE *t;

    ot = &gSubOt[gActiveBuff];
    switch (D_800FA28C) {
        case 1:
            break;
        case 0:
            p = (POLY_F4 *)GsOUT_PACKET_P;
            setPolyF4(p);
            p->r0 = p->g0 = p->b0 = 0;
            p->x0 = p->x2 = -160;
            p->x1 = p->x3 = 160;
            p->y0 = p->y1 = -120;
            p->y2 = p->y3 = 120;
            addPrim(ot->org, p);
            p++;
            GsOUT_PACKET_P = (PACKET *)p;
            break;
        case 2:
            p = (POLY_F4 *)GsOUT_PACKET_P;
            setPolyF4(p);
            setSemiTrans(p, 1);
            p->x0 = p->x2 = -160;
            p->x1 = p->x3 = 160;
            p->y0 = p->y1 = -120;
            p->y2 = p->y3 = 120;
            p->r0 = p->g0 = p->b0 = ~(D_800FA288 * 8);
            addPrim(ot->org, p);
            p++;
            t = (DR_TPAGE *)p;
            setlen(t, 1);
            t->code[0] = 0xE1000640;
            addPrim(ot->org, t);
            t++;
            GsOUT_PACKET_P = (PACKET *)t;
            D_800FA288++;
            if (D_800FA288 >= 32) {
                D_800FA28C = 1;
            }
            break;
        case 3:
            p = (POLY_F4 *)GsOUT_PACKET_P;
            setPolyF4(p);
            setSemiTrans(p, 1);
            p->x0 = p->x2 = -160;
            p->x1 = p->x3 = 160;
            p->y0 = p->y1 = -120;
            p->y2 = p->y3 = 120;
            p->r0 = p->g0 = p->b0 = D_800FA288 * 8;
            addPrim(ot->org, p);
            p++;
            t = (DR_TPAGE *)p;
            setlen(t, 1);
            t->code[0] = 0xE1000640;
            addPrim(ot->org, t);
            t++;
            GsOUT_PACKET_P = (PACKET *)t;
            D_800FA288++;
            if (D_800FA288 >= 32) {
                D_800FA28C = 0;
            }
            break;
    }
}

void func_800F8708(SVECTOR *pos, s32 kind) {
    switch (kind) {
        case 1:
            DrawSprite(0x53, pos, NULL, 0, &gSubOt[gActiveBuff]);
            break;
        case 2:
            DrawSprite(0x54, pos, NULL, 0, &gSubOt[gActiveBuff]);
            break;
        case 3:
            DrawSprite(0x55, pos, NULL, 0, &gSubOt[gActiveBuff]);
            pos->vx += 0x90;
            DrawSprite(0x56, pos, NULL, 0, &gSubOt[gActiveBuff]);
            break;
    }
}

void func_800F8840(PlayerState *obj) {
    obj->motionResult[0] = StepAnimation(obj, 0, obj->animFilter);
    switch (obj->action) {
        case 0x60:
            break;
        case 0x61:
            if (obj->motionResult[0] == 0) {
                obj->curMotion[0] = 0xFF;
            }
            break;
    }
}

void func_800F8898(void) {
    D_800FA288 = 0;
    D_800FA28C = 2;
}

void func_800F88B0(void) {
    D_800FA288 = 0;
    D_800FA28C = 3;
}

void func_800F88C8(SVECTOR *rot) {
    MATRIX m;
    SVECTOR v;
    VECTOR out;

    v.vx = 300;
    v.vy = 0;
    v.vz = 0;
    m = GsIDMATRIX;
    RotMatrix(rot, &m);
    SetRotMatrix(&m);
    SetTransMatrix(&m);
    RotTrans(&v, &out, 0);
    sView.vrx = 0;
    sView.vry = -100;
    sView.vrz = 0;
    sView.vpx = out.vx;
    sView.vpy = out.vy - 100;
    sView.vpz = out.vz;
    GsSetRefView2(&sView);
}
