/* Stage 2-2's overlay: the stage engine the eight free-running stages
 * share, and the stage's own objects. */

#include "common.h"
#include "memory.h"
#include "libgte.h"
#include "libgpu.h"
#include "libgs.h"
#include "object.h"
#include "collision.h"
#include "player.h"
#include "model.h"
#include "rand.h"
#include "spad.h"
#include "draw.h"

/** @brief A stage sprite (gStageSprites, 100 of them), as the stage
 *         engine reads it. */
typedef struct {
    s16 id;         /**< its sprite; -1 when free */
    u8 unk2[2];     /**< not yet known */
    s32 x;          /**< x */
    s32 y;          /**< y */
    s32 z;          /**< z */
    s32 driftCount; /**< zeroed when hit: StepDrift's count */
    s32 driftBaseY; /**< the player's y when hit: StepDrift's base */
    s32 driftPhase; /**< StepDrift's phase */
    u16 width;      /**< its drawn width */
    u16 height;     /**< its drawn height */
    s32 radius;     /**< the query's range */
    u8 queried;     /**< set before the query */
    u8 hit;         /**< 1 when the player hit it */
    u8 flags;       /**< SpriteFlag bits and a SpriteAct */
    u8 walk;        /**< 1 for a sprite that walks towards the player */
    u8 se;          /**< the sound it makes when hit */
    u8 unk29;       /**< not yet known */
    s16 walkArg;    /**< handed to the walk's start */
    s16 switchNo;   /**< the switch that shows it */
    u8 unk2E[0xE];  /**< not yet known */
} EngSprite;

/* A walking sprite's state, three words per sprite: its heading in
 * degrees, the sprite of its frame 0 and its frame, 0 to 3. */
#define WALK_ANGLE(i) D_800FBA1C[(i) * 3]
#define WALK_BASE(i) D_800FBA1C[(i) * 3 + 1]
#define WALK_FRAME(i) D_800FBA1C[(i) * 3 + 2]

/** @brief A trap as the stage engine moves it: its position, its placed
 *         copy and angles, a clock, a state and a type. */
typedef struct {
    s32 x;           /**< position x */
    s32 y;           /**< position y */
    s32 z;           /**< position z */
    s32 homeX;       /**< placed x */
    s32 homeY;       /**< placed y */
    s32 homeZ;       /**< placed z */
    s16 rotX;        /**< rotation x */
    s16 rotY;        /**< rotation y */
    s16 rotZ;        /**< rotation z */
    s16 homeRotX;    /**< placed rotation x */
    s16 homeRotY;    /**< placed rotation y */
    s16 homeRotZ;    /**< placed rotation z */
    u16 timer;       /**< frames in the current state */
    s16 state;       /**< its state */
    s32 block;       /**< the block it stands in */
    s32 lightAngle;  /**< from the floor lookup */
    s32 lightAngle2; /**< from the floor lookup */
    s16 switchNo;    /**< the switch that triggers it */
    s16 model;       /**< its ObjectModel entry; -1 when free */
    u8 unk38[4];     /**< not yet known */
    s32 section;     /**< the section it was placed in */
    u8 touched;      /**< 1 when the player touched one of its hit points this frame */
    u8 unk41;        /**< not yet known */
    u8 type;         /**< what it does, the step it runs */
    u8 act;          /**< its act */
    u8 unk44[4];     /**< not yet known */
} StageTrap;

/** @brief An object model slot as a model with its own transform. */
typedef struct {
    GsDOBJ2 obj;         /**< the object handler */
    GsCOORDINATE2 coord; /**< the object's coordinate system */
    SVECTOR rot;         /**< rotation */
    SVECTOR scale;       /**< scale, 0x1000 = 1 */
    u8 unk70[8];         /**< not yet known */
} ModelSlot;

/** @brief Three words, a position. */
typedef struct {
    s32 x; /**< x */
    s32 y; /**< y */
    s32 z; /**< z */
} Vec3;

/** @brief The 0x30-byte argument block of a position query: collision's
 *         Sphere. */
typedef struct {
    Vec3 pos;       /**< the position asked about */
    u8 unkC[8];     /**< not set here */
    s32 radius;     /**< the query's radius, summed with the other's */
    u8 kind;        /**< the hit kind it reports */
    u8 unk19[0x17]; /**< not set here */
} Query30;

/** @brief The saved state a stage can resume from (stage_flow's). */
typedef struct {
    s32 valid;      /**< non-zero when there is one: the checkpoint's number */
    Vec3 pos;       /**< the position */
    s32 section;    /**< gSection */
    s32 floorIndex; /**< gFloorIndex */
    u16 cameraYaw;  /**< gCameraYaw */
    u16 cans;       /**< gCans */
    u8 pad1C[4];    /**< not yet known */
    s32 viewY;      /**< gView[1], the camera height */
} SavedStart;

/* MATCHING: a struct lvalue keeps the base in a register. */
#define sSaved (*(SavedStart *)gSavedStart)

#define ABS(x) ((x) < 0 ? -(x) : (x))

extern SwitchBox gSwitches[];
extern ModelSlot gModeObjectModels[];
/* MATCHING: player's view, signed here: reached through a split lui/%lo pair. */
extern s8 D_800D3358[];
extern EngSprite gStageSprites[];
extern s32 D_800FBA1C[];
extern u8 D_800FBECC[];
extern s16 D_800FBF3C[];
extern StageTrap D_800FC00C[];
extern StageTrap D_800D39D8[];
extern StageTrap D_800FF84C[];
extern s8 D_800FAF4D;
extern s16 D_800FAF0C[][2];
extern s32 D_800FAF2C[];
extern s8 D_800FAF4C;
extern s8 D_800FAF4E;
extern u16 D_800FAF50[];

/** @brief Six speeds, copied whole. */
typedef struct {
    s16 v[6]; /**< the speeds */
} Speeds;

extern const Speeds D_800F014C;

/** @brief A 16-byte directory entry of a loaded file. */
typedef struct {
    s32 offset;   /**< byte offset of the entry from the directory */
    u8 unk4[0xC]; /**< not yet known */
} DirEnt16;

extern s8 gSkipLandRumble; /**< 1 after a start or restart */

/* MATCHING: object's views, with this unit's sprite record. */
void StepSprite(s32 id, EngSprite *r);
void DrawObjectSprite(s32 id, EngSprite *r);
s32 StepDrift(EngSprite *r);
void func_800F8AE4(void);
void func_800F8B44(EngSprite *r, u16 i, s16 arg);
void func_800F0AA0(EngSprite *r, u16 i, s16 arg, u16 speed, s32 range, s32 limit);
void func_800F0C94(EngSprite *r, u16 i, u16 speed);
void func_800F13AC(void);
/* MATCHING: object's views, with this unit's records. */
s32 UpdatePathSegment(StageTrap *t);
void DrawObjectModel(ObjectModel *rec, StageTrap *t);
s32 CollidePlayer(Query30 *q);
void PolarToXZ(s32 r, s32 deg, Vec3 *out);
s16 FindFreeSlot48(void);
void func_800F0F0C(StageTrap *t);
void func_800F8C08(StageTrap *t);
void func_800F1838(StageTrap *t);
void func_800F1D5C(StageTrap *t);
void func_800F8D10(StageTrap *t, s8 reverse);
void func_800F8F04(StageTrap *t, s8 reverse);
void func_800F9044(StageTrap *t, s8 dir);
void func_800F93A8(StageTrap *t, s8 dir);
void func_800F9158(StageTrap *t, s8 dir);
void func_800F91C0(StageTrap *t, s8 dir);
void func_800F9230(StageTrap *t, s8 dir);
void func_800F9B44(StageTrap *t, u8 arg);
void func_800F9480(StageTrap *t, s8 dir);
void func_800F94F8(StageTrap *t, s8 dir);
void func_800F95F0(StageTrap *t, s8 dir);
void func_800F96D0(StageTrap *t, s8 unused);
void func_800F9D9C(StageTrap *t, u8 arg);
void func_800F90AC(StageTrap *t, s8 mode);
void func_800F228C(StageTrap *t, s8 dir);
void func_800F8A7C(u8 se);
void func_800F7890(void);
void func_800F9820(StageTrap *t);
void func_800F404C(StageTrap *t);
void func_800F4354(StageTrap *t);
void func_800F4C80(StageTrap *t, s32 arg);
void func_800F9EB0(StageTrap *t);
void func_800F9ED4(StageTrap *t);
void func_800F9EF8(StageTrap *t);
void func_800F9F1C(StageTrap *t);
void func_800F516C(StageTrap *t);
void func_800F557C(StageTrap *t);
void func_800F9E24(StageTrap *t);
void func_800F9F3C(StageTrap *t);
void func_800F57C0(StageTrap *t);
void func_800F5324(StageTrap *t);
void func_800F4E50(StageTrap *t);
void func_800F5A24(StageTrap *t, s8 dir);
void func_800F4650(StageTrap *t);
void func_800F3CFC(StageTrap *t);
void func_800F3E7C(StageTrap *t);
void func_800F9F60(StageTrap *t);
void func_800F5D88(StageTrap *t);
void func_800F98CC(StageTrap *t);
void func_800F98D4(StageTrap *t);
void func_800F98DC(StageTrap *t);
void func_800F98E4(StageTrap *t);
void func_800F98EC(StageTrap *t, s32 arg);
void func_800F98F4(StageTrap *t);
void func_800F98FC(StageTrap *t);
void func_800F2A5C(StageTrap *t, u16 speed, s32 range, s8 dir);
void func_800F9904(StageTrap *t, s32 range, u8 type, u8 act);
void func_800FA008(void);
void func_800FA010(void);
void func_800F9F68(StageTrap *t);
void func_800FA110(StageTrap *t);
void func_800F3B08(StageTrap *t, s32 arg);
void func_800F79A0(StageTrap *t, s32 arg);
void func_800F98BC(StageTrap *t);
void func_800F98C4(StageTrap *t);
void func_800F24DC(StageTrap *t, s16 speed);
void func_800F28FC(StageTrap *t, s16 speed);
void func_800F2C24(StageTrap *t, u16 speed, s8 turn);
void func_800F2DCC(StageTrap *t, s16 speed, s32 range);
void func_800F9990(StageTrap *t);
void func_800F9150(StageTrap *t, u16 speed, s16 arg, s8 dir);
void func_800F99F8(StageTrap *t, s16 speed, s8 mode);
void func_800F9808(StageTrap *t, u16 speed, s8 dir, s32 arg);
void func_800F9810(StageTrap *t, u16 speed, s32 arg);
void func_800F9818(StageTrap *t, u16 arg);
void func_800F99A0(StageTrap *t, u16 speed, s32 arg, s8 dir);
void func_800F9C90(StageTrap *t, u16 speed, s32 arg);
void func_800F9C98(StageTrap *t, u16 speed);
void func_800F99A8(StageTrap *t, u16 speed, s32 arg);
void func_800F99B0(StageTrap *t, s32 speed, s32 arg, s8 dir);
void func_800F99B8(StageTrap *t, u16 speed, s32 arg, s8 dir);
void func_800F99C0(StageTrap *t, s32 speed, s32 arg, s32 arg2);
void func_800F99C8(StageTrap *t, u16 speed);
void func_800F99D0(StageTrap *t, u16 speed, s32 arg, s8 dir);
void func_800F99D8(StageTrap *t, s32 speed, s32 arg);
void func_800F99E0(StageTrap *t, s32 arg, s8 arg2);
void func_800F99E8(StageTrap *t, s32 arg);
void func_800F99F0(StageTrap *t, s32 arg);
void func_800F326C(StageTrap *t, u16 speed);
void func_800F6098(StageTrap *t, s32 speed, s8 arg);
void func_800F645C(StageTrap *t, s32 speed, s8 arg);
void func_800F67E4(StageTrap *t, s8 arg);
void func_800F6B0C(StageTrap *t, s32 side, s32 speed, u8 kind);
void func_800F6FF4(StageTrap *t, s8 dir, s32 speed);
void func_800F82E0(StageTrap *t);
void func_800F8454(StageTrap *t);
void func_800F87E4(StageTrap *t);
void func_800F84A8(StageTrap *t);
void func_800F74FC(StageTrap *t);
void func_800F7968(StageTrap *t);
void func_800F76DC(StageTrap *t, s32 speed, u8 arg);
void func_800F7278(StageTrap *t, u8 act);
void func_800F495C(StageTrap *t);
void func_800F3644(StageTrap *t, s32 height, s8 spawn);
void func_800F33B8(StageTrap *t);
void func_800FA048(void);
void func_800F7C44(void);
void func_800F7E08(s8 arg);
/* MATCHING: stage_flow's, with param0 taken whole: retail passes it
 * without narrowing. */
s32 SpawnEffectWithParams(s32 kind, s16 x, s16 y, s16 z, s16 delay, s32 param0, s16 param1,
                          s16 param2, s16 param3);
void func_800F1F84(StageTrap *t);
void func_800F8DF4(StageTrap *t);
void func_800F8E88(StageTrap *t);
void func_800F23C4(StageTrap *t, s32 range, s8 dir);
void func_800F8F94(StageTrap *t);
void func_800F9280(StageTrap *t);
void func_800F9A00(StageTrap *t);
void func_800F9AA0(StageTrap *t);
void func_800F9BF8(StageTrap *t);
void func_800F93E4(StageTrap *t);
void func_800F9534(StageTrap *t);
void func_800F9418(StageTrap *t);
void func_800F95C0(StageTrap *t);
void func_800F9370(StageTrap *t);
void func_800F9CA0(StageTrap *t);
void func_800F9740(StageTrap *t);
void func_800F9650(StageTrap *t);
void func_800F92EC(StageTrap *t);
void func_800F9E00(StageTrap *t);
void func_800F97D4(StageTrap *t);
void func_800F944C(StageTrap *t);
void func_800F9790(StageTrap *t);

void func_800F094C(void) {
    u32 i;
    EngSprite *r;
    s8 k;

    func_800F8AE4();
    for (i = 0; i < 100; i++) {
        r = &gStageSprites[i];
        if (r->id != -1 && (k = gSwitches[r->switchNo].state) == SWITCH_ON) {
            if (r->walk != 0 && r->walk == k) {
                func_800F0AA0(r, i, r->walkArg, 5, 2500, 180);
            }
            if (r->flags & SPRITE_HITTABLE) {
                StepSprite(r->id, r);
                if (r->hit != 0) {
                    func_800F8A7C(r->se);
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

void func_800F0AA0(EngSprite *r, u16 i, s16 arg, u16 speed, s32 range, s32 limit) {
    switch ((s8)D_800FBECC[i]) {
        case 0:
            func_800F8B44(r, i, arg);
            D_800FBECC[i]++;
        case 1:
            D_800FBF3C[i]++;
            if (range * range > DistanceSquared((VECTOR *)&gPlayer.x, (VECTOR *)&r->x)) {
                D_800FBECC[i]++;
                D_800FBF3C[i] = 0;
            }
            break;
        case 2:
            func_800F0C94(r, i, speed);
            if (++D_800FBF3C[i] > limit) {
                D_800FBECC[i]++;
                D_800FBF3C[i] = 0;
            }
        case 3:
            if (r->hit == 1) {
                r->driftCount = 0;
                r->flags = (r->flags & SPRITE_MIRROR) | SPRITE_ACT_DRIFT;
                r->driftBaseY = r->y;
                D_800FBECC[i] += 2;
                r->id = WALK_BASE(i) += 4;
            }
            break;
    }
}

void func_800F0C94(EngSprite *r, u16 i, u16 speed) {
    if (gFrameCount % 5 == 0) {
        WALK_FRAME(i) = (WALK_FRAME(i) + 1) % 4;
        r->id = WALK_FRAME(i) + WALK_BASE(i);
    }
    r->x += rsin((WALK_ANGLE(i) << 12) / 360) * speed >> 12;
    r->z += rcos((WALK_ANGLE(i) << 12) / 360) * speed >> 12;
}

void func_800F0E04(void) {
    u32 i;
    StageTrap *t;

    func_800F13AC();
    for (i = 0; i < 200; i++) {
        t = &((StageTrap *)gTraps)[i];
        if (t->model != -1 && gSwitches[t->switchNo].state == SWITCH_ON) {
            func_800F0F0C(t);
        }
    }
    for (i = 0; i < 200; i++) {
        t = &D_800FC00C[i];
        if (t->model != -1 && gSwitches[t->switchNo].state == SWITCH_ON) {
            func_800F0F0C(t);
        }
    }
}

void func_800F0F0C(StageTrap *t) {
    switch (t->type) {
        case 255:
            func_800F8C08(t);
            break;
        case 0:
            func_800F1F84(t);
            break;
        case 1:
            func_800F8D10(t, 0);
            break;
        case 2:
            func_800F8DF4(t);
            break;
        case 3:
            func_800F8D10(t, 1);
            break;
        case 4:
            func_800F8E88(t);
            break;
        case 5:
            func_800F228C(t, 2);
            break;
        case 6:
            func_800F228C(t, 1);
            break;
        case 7:
            func_800F23C4(t, 3000, 2);
            break;
        case 8:
            func_800F23C4(t, 3000, 1);
            break;
        case 9:
            func_800F8F04(t, 0);
            break;
        case 10:
            func_800F8F94(t);
            break;
        case 11:
            func_800F9044(t, 2);
            break;
        case 12:
            func_800F9044(t, 1);
            break;
        case 13:
            func_800F93A8(t, 2);
            break;
        case 14:
            func_800F93A8(t, 1);
            break;
        case 15:
            func_800F9158(t, 0);
            break;
        case 16:
            func_800F91C0(t, 1);
            break;
        case 17:
            func_800F9230(t, 3);
            break;
        case 18:
            func_800F9280(t);
            break;
    }
    switch (t->type) {
        case 19:
            func_800F9A00(t);
            break;
        case 20:
            func_800F9AA0(t);
            break;
        case 21:
            func_800F9B44(t, 0);
            break;
        case 22:
            func_800F9B44(t, 1);
            break;
        case 23:
            func_800F9BF8(t);
            break;
        case 24:
            func_800F9480(t, 0);
            break;
        case 25:
            func_800F9480(t, 1);
            break;
        case 26:
            func_800F94F8(t, 0);
            break;
        case 27:
            func_800F93E4(t);
            break;
        case 28:
            func_800F9534(t);
            break;
        case 29:
            func_800F9418(t);
            break;
        case 30:
            func_800F95C0(t);
            break;
        case 31:
            func_800F9370(t);
            break;
        case 32:
            func_800F95F0(t, 0);
            break;
        case 33:
            func_800F95F0(t, 1);
            break;
        case 34:
        case 35:
        case 36:
        case 37:
        case 38:
        case 40:
            func_800F9CA0(t);
            break;
        case 39:
            func_800F9740(t);
            break;
        case 41:
            func_800F96D0(t, 0);
            break;
        case 42:
            func_800F9650(t);
            break;
        case 43:
            func_800F92EC(t);
            break;
        case 44:
            func_800F9D9C(t, 0);
            break;
        case 45:
            func_800F9D9C(t, 1);
            break;
        case 46:
            func_800F9E00(t);
            break;
        case 47:
            func_800F97D4(t);
            break;
        case 48:
            func_800F944C(t);
            break;
        case 49:
            func_800F9790(t);
            break;
        case 50:
            func_800F90AC(t, 0);
            break;
        case 51:
            func_800F90AC(t, 1);
            break;
    }
}

#ifdef NON_MATCHING
void func_800F13AC(void) {
    u32 i;
    StageTrap *t;
    ModelSlot *a;
    ModelSlot *b;

    if (gSkipLandRumble == 1 || D_800FAF4D == 0) {
        bzero((u8 *)D_800FC00C, 200 * sizeof(StageTrap));
        for (i = 0; i < 200; i++) {
            D_800FC00C[i].switchNo = -1;
            D_800FC00C[i].model = -1;
            D_800FC00C[i].state = 0;
            D_800FC00C[i].timer = 0;
        }
        for (i = 0; i < 10; i++) {
            D_800D39D8[i].model = -1;
            D_800D39D8[i].state = 0;
            D_800D39D8[i].timer = 0;
        }
        for (i = 0; i < 2; i++) {
            D_800FF84C[i].model = -1;
            D_800FF84C[i].state = 0;
            D_800FF84C[i].timer = 0;
        }
        D_800FAF4D = 1;
        gSlotTable = (s32)D_800FC00C;
        gSlotTableSize = 200 * sizeof(StageTrap);
        t = (StageTrap *)gTraps;
        a = (ModelSlot *)&gObjectModels[t[194].model];
        b = (ModelSlot *)&gObjectModels[t[195].model];
        a->scale.vy = b->scale.vy = a->scale.vx = b->scale.vx = 0x1000;
    }
}
#else
INCLUDE_ASM("asm/ovl/ovl07/nonmatchings/ovl07", func_800F13AC);
#endif

void func_800F14F4(StageTrap *t, s32 id, s8 near) {
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

#ifdef NON_MATCHING
void func_800F1838(StageTrap *t) {
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
            if (d < 0) {
                d = -d;
            }
            t->y = t->homeY - d;
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
#else
INCLUDE_ASM("asm/ovl/ovl07/nonmatchings/ovl07", func_800F1838);
#endif

#ifdef NON_MATCHING
void func_800F1A84(void) {
    s16 models[15] = {4, 0, 0, 3, 0, 0, 0, 0, 0, 0, 6, 0, 2, 5, 0};
    s16 i;
    s16 n;
    StageTrap *t;

    for (i = 0; i < 8; i++) {
        n = FindFreeSlot48();
        if (n == -1) {
            break;
        }
        t = &D_800FC00C[n];
        t->switchNo = 0xB8;
        t->type = 0xFF;
        t->act = 0x6A;
        gSwitches[0xB8].state = SWITCH_ON;
        t->homeY = gPlayer.groundY - 50;
        t->x = rand() % 200 + gPlayer.x - 100;
        t->z = rand() % 200 + gPlayer.z - 100;
        t->model = models[gStage];
        t->homeZ = rand() % 10 + 5;
        t->homeX = D_800FAF2C[i] / 2;
        t->rotY = gCourse.rot.vy + D_800FAF0C[i][0] * 40960 / 360 + 0x800;
        t->homeRotX = 0;
        t->timer = 0;
        t->state = 0;
        t->homeRotZ = rand() % 20 + 10;
    }
}
#else
INCLUDE_ASM("asm/ovl/ovl07/nonmatchings/ovl07", func_800F1A84);
INCLUDE_RODATA("asm/ovl/ovl07/nonmatchings/ovl07", D_800F0084);
#endif

#ifdef NON_MATCHING
void func_800F1D5C(StageTrap *t) {
    s32 d;

    if (t->state == 0) {
        PolarToXZ(t->homeRotZ, t->rotY * 360 / 4096, (Vec3 *)gOffsetVec);
        t->x += gOffsetVec[0] >> 12;
        t->z += gOffsetVec[2] >> 12;
        t->y = t->homeY;
        t->timer = (t->timer + t->homeZ) % 360;
        d = rsin((t->timer << 12) / 360) * t->homeX >> 12;
        t->homeRotX++;
        if (d < 0) {
            d = -d;
        }
        t->y = t->homeY - d;
        t->rotX -= (t->homeRotZ << 12) / 360;
        t->rotZ += (t->homeZ << 12) / 360;
        gModeObjectModels[t->model].coord.super = NULL;
        if (gFrameCount & 3) {
            DrawObjectModel((ObjectModel *)&gModeObjectModels[t->model], t);
        }
        if (t->homeRotX > 60) {
            t->model = -1;
            t->state++;
        }
    }
}
#else
INCLUDE_ASM("asm/ovl/ovl07/nonmatchings/ovl07", func_800F1D5C);
#endif

void func_800F1F84(StageTrap *t) {
    switch (t->act) {
        case 0:
        default:
            if (gSection - UpdatePathSegment(t) < 2) {
                DrawObjectModel(&gObjectModels[t->model], t);
            }
            break;
        case 1:
            func_800F9820(t);
            break;
        case 2:
            func_800F33B8(t);
            break;
        case 3:
            func_800F3644(t, 70, 0);
            break;
        case 4:
            func_800F404C(t);
            break;
        case 5:
            func_800F4354(t);
            break;
        case 6:
            func_800F495C(t);
            break;
        case 7:
            func_800F4C80(t, 0);
            break;
        case 8:
            func_800F9EB0(t);
            break;
        case 9:
            func_800F9ED4(t);
            break;
        case 11:
            func_800F9EF8(t);
            break;
        case 12:
            func_800F9F1C(t);
            break;
        case 13:
            func_800F516C(t);
            break;
        case 15:
        case 16:
            func_800F557C(t);
            break;
        case 17:
            func_800F9E24(t);
            break;
        case 18:
            func_800F9F3C(t);
            break;
        case 19:
            func_800F57C0(t);
            break;
        case 14:
        case 20:
            func_800F5324(t);
            break;
        case 10:
        case 21:
            func_800F4E50(t);
            break;
        case 22:
            func_800F3644(t, 50, 0);
            break;
        case 23:
            func_800F5A24(t, 2);
            break;
        case 24:
            func_800F5A24(t, 1);
            break;
        case 25:
            func_800F4650(t);
            break;
        case 26:
            func_800F3644(t, 20, 0);
            break;
        case 27:
            func_800F3CFC(t);
            break;
        case 28:
            func_800F3E7C(t);
            break;
        case 29:
            func_800F9F60(t);
            break;
        case 30:
            func_800F5D88(t);
            break;
        case 31:
            func_800F3644(t, 30, 1);
            break;
        case 32:
            func_800F3644(t, 10, 2);
            break;
        case 33:
            func_800F98CC(t);
            break;
        case 34:
            func_800F98D4(t);
            break;
        case 35:
            func_800F98DC(t);
            break;
        case 36:
            func_800F98E4(t);
            break;
        case 37:
            func_800F98EC(t, 2);
            break;
        case 38:
            func_800F98EC(t, 1);
            break;
        case 39:
            func_800F98F4(t);
            break;
        case 40:
            func_800F98FC(t);
            break;
    }
}

const Speeds D_800F014C = {{1500, 2000, 2500, 3000, 4000, 5000}};

void func_800F228C(StageTrap *t, s8 dir) {
    switch (t->act) {
        case 1:
            func_800F2A5C(t, 20, 1000, dir);
            break;
        case 2:
            func_800F2A5C(t, 30, 1000, dir);
            break;
        case 3:
            func_800F2A5C(t, 40, 1000, dir);
            break;
        case 4:
            func_800F2A5C(t, 50, 1000, dir);
            break;
        case 5:
            func_800F2A5C(t, 60, 1000, dir);
            break;
        case 6:
            func_800F2A5C(t, 20, 1500, dir);
            break;
        case 7:
            func_800F2A5C(t, 30, 1500, dir);
            break;
        case 8:
            func_800F2A5C(t, 40, 1500, dir);
            break;
        case 9:
            func_800F2A5C(t, 50, 1500, dir);
            break;
        case 10:
            func_800F2A5C(t, 60, 1500, dir);
            break;
        case 11:
            func_800F2A5C(t, 20, 2000, dir);
            break;
        case 12:
            func_800F2A5C(t, 30, 2000, dir);
            break;
        case 13:
            func_800F2A5C(t, 40, 2000, dir);
            break;
        case 14:
            func_800F2A5C(t, 50, 2000, dir);
            break;
        case 15:
            func_800F2A5C(t, 60, 2000, dir);
            break;
        case 16:
            func_800F2A5C(t, 20, 2500, dir);
            break;
        case 17:
            func_800F2A5C(t, 30, 2500, dir);
            break;
        case 18:
            func_800F2A5C(t, 40, 2500, dir);
            break;
        case 19:
            func_800F2A5C(t, 50, 2500, dir);
            break;
        case 20:
            func_800F2A5C(t, 60, 2500, dir);
            break;
    }
}

void func_800F23C4(StageTrap *t, s32 range, s8 dir) {
    switch (t->act) {
        case 101:
            func_800F2A5C(t, 10, 500, dir);
            break;
        case 102:
            func_800F2A5C(t, 20, 500, dir);
            break;
        case 103:
            func_800F2A5C(t, 30, 500, dir);
            break;
        case 104:
            func_800F2A5C(t, 40, 500, dir);
            break;
        case 105:
            func_800F2A5C(t, 50, 500, dir);
            break;
        case 106:
            func_800F2A5C(t, 10, 1000, dir);
            break;
        case 107:
            func_800F2A5C(t, 20, 1000, dir);
            break;
        case 108:
            func_800F2A5C(t, 30, 1000, dir);
            break;
        case 109:
            func_800F2A5C(t, 40, 1000, dir);
            break;
        case 110:
            func_800F2A5C(t, 50, 1000, dir);
            break;
        case 111:
            func_800F2A5C(t, 10, 2000, dir);
            break;
        case 112:
            func_800F2A5C(t, 20, 2000, dir);
            break;
        case 113:
            func_800F2A5C(t, 30, 2000, dir);
            break;
        case 114:
            func_800F2A5C(t, 40, 2000, dir);
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
            func_800F9904(t, range, t->type, t->act + 100);
            break;
        case 115:
            func_800F2A5C(t, 50, 2000, dir);
            break;
    }
}

/* MATCHING: object's views, with this unit's trap record. */
s32 LatchProximity(StageTrap *t, s32 range);
s32 SnapToFloor(StageTrap *t);

void func_800F24DC(StageTrap *t, s16 speed) {
    if (LatchProximity(t, 2000) != 0) {
        PlaySe(82);
    }
    if (SnapToFloor(t) == -1) {
        t->y = t->homeY;
    }
    t->y -= 20;
    if (t->timer < 150 && speed != 0) {
        t->timer++;
        PolarToXZ(speed, t->rotY * 360 / 4096, (Vec3 *)gOffsetVec);
        t->x += gOffsetVec[0] >> 12;
        t->z += gOffsetVec[2] >> 12;
    }
    if (gSection - UpdatePathSegment(t) < 2) {
        UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
    }
    if (t->touched != 0 && gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
        PlaySe(55);
    }
}

void func_800F2658(StageTrap *t, u16 speed) {
    if (LatchProximity(t, 2000) != 0) {
        PlaySe(82);
    }
    if (SnapToFloor(t) == -1) {
        t->y = t->homeY;
    }
    t->y -= 20;
    PolarToXZ(speed, t->rotY * 360 / 4096, (Vec3 *)gOffsetVec);
    t->x += gOffsetVec[0] >> 12;
    t->z += gOffsetVec[2] >> 12;
    if (gSection - UpdatePathSegment(t) < 2) {
        UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
    }
    if (t->touched != 0 && gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
        PlaySe(55);
    }
}

void func_800F27B0(StageTrap *t, u16 speed) {
    if (LatchProximity(t, 2000) != 0) {
        PlaySe(82);
    }
    t->homeY = t->y;
    t->y -= 20;
    PolarToXZ(speed, t->rotY * 360 / 4096, (Vec3 *)gOffsetVec);
    t->x += gOffsetVec[0] >> 12;
    t->z += gOffsetVec[2] >> 12;
    if (gSection - UpdatePathSegment(t) < 2) {
        UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
    }
    if (t->touched != 0 && gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
        PlaySe(55);
    }
    t->y = t->homeY;
}

void func_800F28FC(StageTrap *t, s16 speed) {
    if (LatchProximity(t, 2000) != 0) {
        PlaySe(82);
    }
    if (SnapToFloor(t) == -1) {
        t->y = t->homeY;
    }
    t->y -= 20;
    PolarToXZ(speed, t->rotY * 360 / 4096, (Vec3 *)gOffsetVec);
    t->x += gOffsetVec[0] >> 12;
    t->z += gOffsetVec[2] >> 12;
    if (gSection - UpdatePathSegment(t) < 2) {
        UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
    }
    if (t->touched != 0 && gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
        PlaySe(55);
    }
}

/* MATCHING: object's view, with the target passed unnarrowed. */
void InterpRange(s16 a, s32 b, u16 t, s32 n, s32 *out);

void func_800F2A5C(StageTrap *t, u16 speed, s32 range, s8 dir) {
    switch (t->state) {
        case 0:
            t->homeX = t->x;
            t->homeY = t->y;
            t->homeZ = t->z;
            t->state++;
        case 1:
            func_800F2658(t, speed);
            if (range * range < DistanceSquared((VECTOR *)&t->homeX, (VECTOR *)t)) {
                t->timer = 0;
                t->state++;
                t->homeRotY = t->rotY;
                if (speed > 30) {
                    PlaySe(88);
                }
            }
            if (LatchProximity(t, 2000) != 0) {
                PlaySe(82);
            }
            break;
        case 2:
            switch (dir) {
                case 2:
                    InterpRange(t->homeRotY, t->homeRotY - 0x400, t->timer, 30, gOffsetVec);
                    t->rotY = gOffsetVec[1];
                    break;
                case 1:
                    InterpRange(t->homeRotY, t->homeRotY + 0x400, t->timer, 30, gOffsetVec);
                    t->rotY = gOffsetVec[1];
                    break;
            }
            func_800F2658(t, speed);
            if (++t->timer > 30) {
                t->timer = 0;
                t->state++;
            }
            break;
        case 3:
            func_800F27B0(t, speed);
            break;
    }
}

/* MATCHING: object's view, with this unit's trap record. */
s16 GetPathDirection(StageTrap *t);

void func_800F2C24(StageTrap *t, u16 speed, s8 turn) {
    if (LatchProximity(t, 2000) != 0) {
        PlaySe(82);
    }
    t->state++;
    SnapToFloor(t);
    if (t->timer < 150 && speed != 0) {
        t->timer++;
        t->y -= 20;
        t->rotY = GetPathDirection(t) + (s8)(turn % 2) * 0x800;
        PolarToXZ(speed, t->rotY * 360 / 4096, (Vec3 *)gOffsetVec);
        t->x += gOffsetVec[0] >> 12;
        t->z += gOffsetVec[2] >> 12;
    }
    if (gSection - UpdatePathSegment(t) < 2) {
        UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
    }
    if (t->touched != 0 && gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
        PlaySe(55);
    }
}

/* MATCHING: stage defines it with s16 positions; this unit passes words,
 * narrowing them itself where retail does. */
s32 SpawnEffect(s32 kind, s32 x, s32 y, s32 z, s32 delay);

/* MATCHING: an unused 16-byte local sizes retail's frame. */
void func_800F2DCC(StageTrap *t, s16 speed, s32 range) {
    s32 unused[4];

    switch (t->state) {
        case 0:
            t->homeX = t->x;
            t->homeY = t->y;
            t->homeZ = t->z;
            t->state++;
        case 1:
            func_800F2658(t, speed);
            if (range * range < DistanceSquared((VECTOR *)&t->homeX, (VECTOR *)t)) {
                t->timer = 0;
                t->state++;
                t->homeRotY = t->rotY;
                PlaySe(62);
            }
            break;
        case 2:
            SnapToFloor(t);
            if (++t->timer < 5) {
                t->y -= 20;
                PolarToXZ(-(s16)(speed / 3), t->rotY * 360 / 4096, (Vec3 *)gOffsetVec);
                t->x += gOffsetVec[0] >> 12;
                t->z += gOffsetVec[2] >> 12;
            } else {
                t->timer = 0;
                t->state++;
                PlaySe(62);
            }
            break;
        case 3:
            SnapToFloor(t);
            t->y -= 20;
            if (gFrameCount % 5 == 0) {
                SpawnEffect(1, (s16)(t->x + (rand() % 200 - 100)), (s16)t->y,
                            (s16)(t->z + (rand() % 200 - 100)), (s16)(rand() % 3));
            }
            break;
    }
    if (gSection - UpdatePathSegment(t) < 2) {
        UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
    }
    if (t->touched != 0 && gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
        PlaySe(55);
    }
}

void func_800F3134(StageTrap *t, u16 speed) {
    SnapToFloor(t);
    PolarToXZ(speed, t->rotY * 360 / 4096, (Vec3 *)gOffsetVec);
    t->x += gOffsetVec[0] >> 12;
    t->z += gOffsetVec[2] >> 12;
    if (LatchProximity(t, 500) != 0) {
        PlaySe(120);
    }
    if (gSection - UpdatePathSegment(t) < 2) {
        UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
    }
    if (t->touched != 0 && gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
        PlaySe(55);
    }
}

void func_800F326C(StageTrap *t, u16 speed) {
    switch (t->state) {
        case 0:
            t->timer = 0;
            t->state++;
            PlaySe(109);
        case 1:
            if (LatchProximity(t, 2000) != 0) {
                PlaySe(110);
            }
            PolarToXZ(speed, t->rotY * 360 / 4096, (Vec3 *)gOffsetVec);
            t->x += gOffsetVec[0] >> 12;
            t->z += gOffsetVec[2] >> 12;
            UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
            if (t->touched != 0 && gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
                PlaySe(57);
            }
            break;
    }
}

#ifdef NON_MATCHING
/* MATCHING: declared [] so its load stays below the stores to t. */
extern s16 D_8009EEF8[];

void func_800F33B8(StageTrap *t) {
    s32 d;

    switch (t->state) {
        case 0:
            t->homeY = t->y;
            if (gSection - UpdatePathSegment(t) < 2) {
                UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
            }
            if (t->touched != 0) {
                if (gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
                    PlaySe(58);
                }
                t->homeX = 0;
                t->homeY = t->y;
                t->homeRotY = D_8009EEF8[0];
                t->state++;
                t->timer = 0;
            }
            t->y = t->homeY;
            break;
        case 1:
            PolarToXZ(40, t->homeRotY * 360 / 4096, (Vec3 *)gOffsetVec);
            t->x -= gOffsetVec[0] >> 12;
            t->z -= gOffsetVec[2] >> 12;
            t->timer = (t->timer + 10) % 360;
            t->rotZ -= 11;
            d = rsin((t->timer << 12) / 360) * 150 >> 12;
            if (d < 0) {
                d = -d;
            }
            t->y = t->homeY - d;
            if (++t->homeX == 25) {
                t->state++;
            }
            if (gFrameCount % 3 != 0) {
                DrawObjectModel(&gObjectModels[t->model], t);
            }
            break;
    }
}
#else
INCLUDE_ASM("asm/ovl/ovl07/nonmatchings/ovl07", func_800F33B8);
#endif

/* MATCHING: object's view, with this unit's trap record. */
s32 SnapToFloor(StageTrap *t);

#ifdef NON_MATCHING
void func_800F3644(StageTrap *t, s32 height, s8 spawn) {
    s16 n;
    s32 d;

    switch (t->state) {
        case 0:
            t->homeY = t->y;
            t->y -= height;
            if (t->touched != 0) {
                if (gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
                    PlaySe(0x37);
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
                n = FindFreeSlot48();
                D_800FC00C[n] = *t;
                D_800FC00C[n].rotZ = 0;
                D_800FC00C[n].rotX = 0;
                D_800FC00C[n].rotY = D_800FC00C[n].homeRotY = gCourse.rot.vy;
                D_800FC00C[n].x -= 25 - rand() % 50;
                D_800FC00C[n].z -= 25 - rand() % 50;
                D_800FC00C[n].state = 0;
                SnapToFloor(&D_800FC00C[n]);
                D_800FC00C[n].model = rand() % 3 + 17;
                D_800FC00C[n].type = 0;
                D_800FC00C[n].act = 1;
                t->homeRotZ++;
            }
            break;
    }
}
#else
INCLUDE_ASM("asm/ovl/ovl07/nonmatchings/ovl07", func_800F3644);
#endif

void func_800F3B08(StageTrap *t, s32 arg) {
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

/** @brief object's 0x2C-byte placed point: a world position worked out
 *         from a local one in a coordinate system. */
typedef struct {
    VECTOR pos;           /**< world position, written from the local one */
    u8 unk10[0xC];        /**< not yet known */
    s32 lx;               /**< local x */
    s32 ly;               /**< local y */
    s32 lz;               /**< local z */
    GsCOORDINATE2 *coord; /**< the coordinate system lx..lz are in */
} PlacedPos;

/* MATCHING: object's view, with this unit's copy of its placed point. */
void UpdatePlacedPos(PlacedPos *p);

void func_800F3CFC(StageTrap *t) {
    PlacedPos p;
    s32 x;
    s32 z;

    p.lx = 0;
    p.ly = -70;
    p.lz = 0;
    p.coord = (GsCOORDINATE2 *)gObjectModels[t->model].coord;
    UpdatePlacedPos(&p);
    t->homeX = p.pos.vx;
    t->homeY = p.pos.vy;
    t->homeZ = p.pos.vz;
    if (gFrameCount % 5 == 0) {
        /* MATCHING: x - (k - r), not x - k + r: retail subtracts k from x first. */
        x = (s16)p.pos.vx - (20 - rand() % 40);
        z = (s16)p.pos.vz - (20 - rand() % 40);
        SpawnEffect(1, x, (s16)p.pos.vy, z, (s16)(rand() % 3));
    }
    func_800F3644(t, 70, 0);
}

void func_800F3E7C(StageTrap *t) {
    PlacedPos p;
    s32 x;
    s32 z;

    p.lx = 0;
    p.ly = -50;
    p.lz = -100;
    p.coord = (GsCOORDINATE2 *)gObjectModels[t->model].coord;
    UpdatePlacedPos(&p);
    t->homeX = p.pos.vx;
    t->homeY = p.pos.vy;
    t->homeZ = p.pos.vz;
    if (gFrameCount % 5 == 0) {
        x = (s16)p.pos.vx - (100 - rand() % 200);
        z = (s16)p.pos.vz - (200 - rand() % 400);
        SpawnEffect(1, x, (s16)p.pos.vy, z, (s16)(rand() % 3));
    }
    if (gSection - UpdatePathSegment(t) < 2) {
        UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
    }
}

extern s16 sFloorHit[];
extern s32 D_800FAECC[][2];
/* MATCHING: object's view: the swing's radius is passed unextended. */
void InterpSwing(s32 r, u16 t, u16 n, s32 *out);
/* MATCHING: player's empty hook, called here with the trap. */
void UnusedHook1(StageTrap *t);

void func_800F404C(StageTrap *t) {
    u8 pad[0x28];
    StageTrap c;
    s16 i;

    switch (t->state) {
        case 0:
            SnapToFloor(t);
            t->timer++;
            if (gSection - UpdatePathSegment(t) < 2) {
                UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
            }
            if (t->touched == 1) {
                if (gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
                    PlaySe(0x38);
                }
                t->homeY = sFloorHit[0];
                t->timer = 0;
                t->model++;
                t->state++;
                UnusedHook1(t);
            }
            break;
        case 1:
            if (t->timer % 3 != 0) {
                for (i = 0; i < 6; i++) {
                    c = *t;
                    InterpSwing(D_800FAF2C[i], t->timer, 30, gOffsetVec);
                    c.y -= gOffsetVec[1] >> 12;
                    /* MATCHING: a shift, not * 4: it loads the timer before the table. */
                    c.x += D_800FAECC[i][0] * (t->timer << 2);
                    c.z += D_800FAECC[i][1] * (t->timer << 2);
                    c.rotZ += (t->timer * (D_800FAF0C[i][0] * 2 + 10) << 12) / 360;
                    DrawObjectModel(&gObjectModels[t->model], &c);
                }
            }
            if (++t->timer == 30) {
                t->state = -1;
            }
            break;
    }
}

void func_800F4354(StageTrap *t) {
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
                InterpSwing(D_800FAF2C[i], t->timer, 30, gOffsetVec);
                c.y -= gOffsetVec[1] >> 12;
                /* MATCHING: * 3 << 1, not * 6: it loads the timer before the table. */
                c.x += D_800FAECC[i][0] * (t->timer * 3 << 1);
                c.z += D_800FAECC[i][1] * (t->timer * 3 << 1);
                c.rotZ += (t->timer * (D_800FAF0C[i][0] * 2 + 10) << 12) / 360;
                DrawObjectModel(&gObjectModels[c.model], &c);
            }
            if (t->timer == 30) {
                t->state++;
            }
            break;
    }
}

void func_800F4650(StageTrap *t) {
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
                InterpSwing(D_800FAF2C[i], t->timer, 30, gOffsetVec);
                c.y -= gOffsetVec[1] >> 12;
                c.x += D_800FAECC[i][0] * (t->timer * 3 << 1);
                c.z += D_800FAECC[i][1] * (t->timer * 3 << 1);
                c.rotZ += (t->timer * (D_800FAF0C[i][0] * 2 + 10) << 12) / 360;
                DrawObjectModel(&gObjectModels[c.model], &c);
            }
            if (t->timer == 30) {
                t->state++;
            }
            break;
    }
}

/* MATCHING: object's, with r taken whole: retail passes it unnarrowed. */
void InterpSwing(s32 r, u16 t, u16 n, s32 *out);
extern s32 D_800FAECC[][2];

void func_800F495C(StageTrap *t) {
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
                InterpSwing(D_800FAF2C[i], t->timer, 30, gOffsetVec);
                c.y -= gOffsetVec[1] >> 12;
                /* MATCHING: timer * 3 is reassociated onto the table's word. */
                c.x += D_800FAECC[i][0] * (t->timer * 2 + t->timer);
                c.z += D_800FAECC[i][1] * (t->timer * 2 + t->timer);
                c.rotZ += (t->timer * (D_800FAF0C[i][0] * 2 + 10) << 12) / 360;
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

void func_800F4C80(StageTrap *t, s32 arg) {
    switch (t->state) {
        case 0:
            if (gSection - UpdatePathSegment(t) < 2) {
                UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
            }
            if (t->touched != 0) {
                t->homeRotY = gPlayer.yaw;
                t->timer = 0;
                t->state++;
                if (gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
                    PlaySe(0x3A);
                }
            }
            break;
        case 1:
            PolarToXZ(40, t->homeRotY * 360 / 4096, (Vec3 *)gOffsetVec);
            t->x -= gOffsetVec[0] >> 12;
            t->z -= gOffsetVec[2] >> 12;
            t->rotZ += 11;
            if (++t->timer == 15) {
                t->state = -1;
                t->timer = 0;
            }
            if (!(gFrameCount & 1) && t->state != -1) {
                DrawObjectModel(&gObjectModels[t->model], t);
            }
            break;
    }
}

void func_800F4E50(StageTrap *t) {
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
                InterpSwing(D_800FAF2C[i], t->timer, 30, gOffsetVec);
                c.y -= gOffsetVec[1] >> 12;
                /* MATCHING: as in func_800F495C, timer * 5 spelled out. */
                c.x += D_800FAECC[i][0] * (t->timer * 4 + t->timer);
                c.z += D_800FAECC[i][1] * (t->timer * 4 + t->timer);
                c.rotZ += (t->timer * (D_800FAF0C[i][0] * 2 + 10) << 12) / 360;
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

/* MATCHING: model's, all s32 where it has s16: the result is stored
 * without re-extension. */
s32 Lerp(s32 a, s32 b, s32 t, s32 n);

void func_800F516C(StageTrap *t) {
    switch (t->state) {
        case 0:
            if (t->touched != 0) {
                gPlayer.action = ACT_DASH_START;
                t->timer = 0;
                t->state++;
                if (gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
                    PlaySe(0x71);
                }
            }
            if (gSection - UpdatePathSegment(t) < 2) {
                UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
            }
            break;
        case 1:
            t->timer++;
            t->rotX = Lerp(0, -0x4E3, t->timer, 3);
            if (t->timer == 3) {
                t->homeRotY = (t->rotY - 0x800) * 360 / 4096;
                t->timer = 0;
                t->state++;
            }
        case 2:
            if (gSection - UpdatePathSegment(t) < 2) {
                DrawObjectModel(&gObjectModels[t->model], t);
            }
            break;
    }
}

void func_800F5324(StageTrap *t) {
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

extern s16 D_8009EEF8[];

#ifdef NON_MATCHING
void func_800F557C(StageTrap *t) {
    s32 d;

    switch (t->state) {
        case 0:
            UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
            if (t->touched == 1) {
                t->homeX = 0;
                t->homeY = t->y;
                /* MATCHING: read as a member at a fixed address, which keeps
                 * the load below the stores into t. */
                t->homeRotY = ((SVECTOR *)D_8009EEF8)->vx;
                t->state++;
                t->timer = 0;
            }
            break;
        case 1:
            PolarToXZ(40, t->homeRotY * 360 / 4096, (Vec3 *)gOffsetVec);
            t->x -= gOffsetVec[0] >> 12;
            t->z -= gOffsetVec[2] >> 12;
            t->timer = (t->timer + 10) % 360;
            t->rotX += 113;
            t->rotZ += 113;
            d = rsin((t->timer << 12) / 360) * 200 >> 12;
            if (d < 0) {
                d = -d;
            }
            t->y = t->homeY - d;
            if (++t->homeX == 20) {
                t->state++;
            }
            if ((gFrameCount & 1) && gSection - UpdatePathSegment(t) < 2) {
                DrawObjectModel(&gObjectModels[t->model], t);
            }
            break;
    }
}
#else
INCLUDE_ASM("asm/ovl/ovl07/nonmatchings/ovl07", func_800F557C);
#endif

void func_800F57C0(StageTrap *t) {
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

/* MATCHING: object's views, with this unit's records. */
s32 SnapToFloor(StageTrap *t);
/* MATCHING: stage defines x..delay as s16; this call passes them
 * unextended, so the view takes s32. */
s32 SpawnEffect(s32 kind, s32 x, s32 y, s32 z, s32 delay);

#ifdef NON_MATCHING
void func_800F5A24(StageTrap *t, s8 dir) {
    PlacedPos p;
    StageTrap c;
    s32 d;

    if (gSection - UpdatePathSegment(t) < 2) {
        UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
        if (t->touched != 0 && gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
            PlaySe(0x37);
        }
        if (dir != 0) {
            switch (t->state) {
                case 0:
                    switch (dir) {
                        case 2:
                            p.lx = -140;
                            break;
                        case 1:
                            p.lx = 140;
                            break;
                        case 3:
                            p.lx = 0;
                            break;
                    }
                    p.ly = 50;
                    p.lz = 0;
                    p.coord = (GsCOORDINATE2 *)gObjectModels[t->model].coord;
                    UpdatePlacedPos(&p);
                    t->homeX = p.pos.vx;
                    t->homeY = p.pos.vy;
                    t->homeZ = p.pos.vz;
                    t->state++;
                case 1:
                    c = *t;
                    SnapToFloor(&c);
                    if (!(gFrameCount & 3)) {
                        SpawnEffectWithParams(9, t->homeX, t->homeY, t->homeZ, 0, 20,
                                              (d = c.y - t->homeY) < 0 ? -d : d, 0, 0);
                    }
                    if (gFrameCount % 10 == 0) {
                        PlaySe(0x6A);
                    }
                    if (!(gFrameCount & 1)) {
                        SpawnEffect(4, (s16)t->homeX - 50 + rand() % 100, (s16)c.y + 20 - rand() % 50,
                                    (s16)t->homeZ - 50 + rand() % 100, 0);
                    }
                    break;
            }
        }
    }
}
#else
INCLUDE_ASM("asm/ovl/ovl07/nonmatchings/ovl07", func_800F5A24);
#endif

void func_800F5D88(StageTrap *t) {
    StageTrap c;
    u16 i;

    switch (t->state) {
        case 0:
            t->homeY = t->y;
            t->y -= 50;
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
            t->y = t->homeY;
            break;
        case 1:
            t->timer++;
            if (t->timer % 3) {
                for (i = 0; i < 8; i++) {
                    c = *t;
                    c.model = 0;
                    InterpSwing(D_800FAF2C[i], t->timer, 30, gOffsetVec);
                    c.y -= gOffsetVec[1] >> 12;
                    /* MATCHING: a shift, not * 2, keeps the timer load first. */
                    c.x += D_800FAECC[i][0] * (t->timer << 1);
                    c.z += D_800FAECC[i][1] * (t->timer << 1);
                    c.rotZ += (t->timer * (D_800FAF0C[i][0] * 2 + 10) << 12) / 360;
                    if (gSection - UpdatePathSegment(t) < 2) {
                        DrawObjectModel(&gObjectModels[c.model], &c);
                    }
                }
            }
            if (t->timer == 30) {
                t->state++;
            }
            break;
    }
}

void func_800F6098(StageTrap *t, s32 speed, s8 arg) {
    s32 v;
    s32 w;

    switch (t->state) {
        case 0:
            if (speed * speed > DistanceSquared((VECTOR *)&gPlayer.x, (VECTOR *)t)) {
                t->timer = 0;
                t->state++;
                t->homeRotY = t->rotY;
            }
            if (gSection - UpdatePathSegment(t) < 2) {
                DrawObjectModel(&gObjectModels[t->model], t);
            }
            break;
        case 1:
            t->timer++;
            v = Lerp(0, 0x200, t->timer, 10);
            switch (arg) {
                case 2:
                    t->rotY = t->homeRotY - v;
                    break;
                case 1:
                    t->rotY = t->homeRotY + v;
                    break;
            }
            if (gSection - UpdatePathSegment(t) < 2) {
                UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
            }
            if (t->timer == 10) {
                t->timer = 0;
                t->state++;
                t->model++;
                switch (arg) {
                    case 2:
                        t->homeRotY -= 0x200;
                        break;
                    case 1:
                        t->homeRotY += 0x200;
                        break;
                }
            }
            if (t->touched != 0 && gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
                PlaySe(0x37);
            }
            break;
        case 2:
            t->timer++;
            w = Lerp(0, 0x200, t->timer, 10);
            switch (arg) {
                case 2:
                    t->rotY = t->homeRotY - w;
                    break;
                case 1:
                    t->rotY = t->homeRotY + w;
                    break;
            }
            if (t->timer == 10) {
                t->timer = 0;
                t->state++;
            }
            if (gSection - UpdatePathSegment(t) < 2) {
                UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
            }
            if (t->touched != 0 && gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
                PlaySe(0x37);
            }
            break;
        case 3:
            if (gSection - UpdatePathSegment(t) < 2) {
                UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
            }
            if (t->touched != 0 && gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
                PlaySe(0x37);
            }
            break;
    }
}

void func_800F645C(StageTrap *t, s32 speed, s8 arg) {
    StageTrap c;
    s32 v;

    c = *t;
    c.model++;
    switch (t->state) {
        case 0:
            if (speed * speed > DistanceSquared((VECTOR *)&gPlayer.x, (VECTOR *)t)) {
                t->timer = 0;
                t->state++;
                t->homeRotY = t->rotY;
            }
            if (gSection - UpdatePathSegment(t) < 2) {
                UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
                UpdateObjectEntry(&gObjectModels[c.model], (Trap *)&c);
            }
            if ((t->touched != 0 || c.touched != 0) && gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
                PlaySe(0x37);
            }
            break;
        case 1:
            t->timer++;
            v = Lerp(0, 0x800, t->timer, 40);
            switch (arg) {
                case 2:
                    c.rotY = t->homeRotY - v;
                    break;
                case 1:
                    c.rotY = t->homeRotY + v;
                    break;
            }
            if (gSection - UpdatePathSegment(t) < 2) {
                UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
                UpdateObjectEntry(&gObjectModels[c.model], (Trap *)&c);
            }
            if (t->timer == 40) {
                t->timer = 0;
                t->state++;
            }
            if ((t->touched != 0 || c.touched != 0) && gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
                PlaySe(0x37);
            }
            break;
        case 2:
            if (gSection - UpdatePathSegment(t) < 2) {
                UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
                c.rotY += 0x800;
                UpdateObjectEntry(&gObjectModels[c.model], (Trap *)&c);
            }
            if ((t->touched != 0 || c.touched != 0) && gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
                PlaySe(0x37);
            }
            break;
    }
}

void func_800F67E4(StageTrap *t, s8 arg) {
    StageTrap c;

    c = *t;
    c.model++;
    switch (t->state) {
        case 0:
            t->homeX = 0;
            t->homeRotY = t->rotY;
            t->state++;
        case 1:
            t->timer++;
            switch (t->homeX) {
                case 0:
                    c.rotY = t->homeRotY - Lerp(0, 0x800, t->timer, 40);
                    break;
                case 1:
                    c.rotY = t->homeRotY - Lerp(0x800, 0, t->timer, 40);
                    break;
            }
            if (gSection - UpdatePathSegment(t) < 2) {
                UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
                UpdateObjectEntry(&gObjectModels[c.model], (Trap *)&c);
            }
            if (t->timer == 40) {
                t->timer = 0;
                t->homeX ^= 1;
                if (arg == 1) {
                    t->homeZ = 0;
                    t->state++;
                }
            }
            if ((t->touched != 0 || c.touched != 0) && gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
                PlaySe(0x37);
            }
            break;
        case 2:
            t->timer++;
            switch (t->homeX) {
                case 0:
                    c.rotY = 0;
                    break;
                case 1:
                    c.rotY = 0x800;
                    break;
            }
            if (gSection - UpdatePathSegment(t) < 2) {
                UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
                UpdateObjectEntry(&gObjectModels[c.model], (Trap *)&c);
            }
            if ((t->touched != 0 || c.touched != 0) && gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
                PlaySe(0x37);
            }
            if (t->timer == 20) {
                t->timer = 0;
                t->state--;
            }
            break;
    }
}

void func_800F6ED0(StageTrap *t, u8 side);

void func_800F6B0C(StageTrap *t, s32 side, s32 speed, u8 kind) {
    s16 n;

    switch (kind) {
        case 0:
            n = 12;
            break;
        case 1:
            n = 10;
            break;
    }
    switch (t->state) {
        case 0:
            if (speed * speed > DistanceSquared((VECTOR *)&gPlayer.x, (VECTOR *)t)) {
                t->timer = 0;
                t->state++;
                t->homeRotY = t->rotY;
            }
            if (gSection - UpdatePathSegment(t) < 2) {
                DrawObjectModel(&gObjectModels[t->model], t);
            }
            break;
        case 1:
            t->timer++;
            switch ((u8)side) {
                case 1:
                case 2:
                    t->y += t->timer * 4;
                    break;
                case 3:
                    t->y += t->timer * 12;
                    if (t->y > gFloorY[0] - 30) {
                        t->y = gFloorY[0] - 30;
                    }
                    break;
            }
            switch ((u8)side) {
                case 2:
                    t->rotZ = -Lerp(0, 227, t->timer, n);
                    break;
                case 1:
                    t->rotZ = Lerp(0, 227, t->timer, n);
                    break;
            }
            if (t->timer == n) {
                t->timer = 0;
                t->state++;
                func_800F6ED0(t, side);
                if (gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
                    PlaySe(0x37);
                }
            }
            if (gSection - UpdatePathSegment(t) < 2) {
                UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
            }
            break;
        case 2:
            t->timer++;
            t->y -= 10;
            if (t->timer == 2) {
                t->timer = 0;
                t->state++;
            }
            if (gSection - UpdatePathSegment(t) < 2) {
                UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
            }
            break;
        case 3:
            t->timer++;
            t->y += 10;
            if (t->timer == 4) {
                t->timer = 0;
                t->state++;
                func_800F6ED0(t, side);
                if (gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
                    PlaySe(0x37);
                }
            }
        case 4:
            if (gSection - UpdatePathSegment(t) < 2) {
                UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
            }
            if (t->touched != 0 && gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
                PlaySe(0x37);
            }
            break;
    }
}

/* MATCHING: player's, which no header declares. */
void ScatterDust(VECTOR *pos);

void func_800F6ED0(StageTrap *t, u8 side) {
    VECTOR v;
    u16 i;

    switch (side) {
        case 2:
            v.vx = t->x + 280;
            v.vz = t->z - 200;
            v.vy = gFloorY[0];
            ScatterDust(&v);
            break;
        case 1:
            v.vx = t->x - 280;
            v.vz = t->z + 200;
            v.vy = gFloorY[0];
            ScatterDust(&v);
            break;
        case 3:
            for (i = 0; i < 10; i++) {
                v.vx = t->x + 300;
                v.vx -= i * 50;
                v.vz = t->z - 200;
                v.vz += i * 50;
                v.vy = gFloorY[0];
                ScatterDust(&v);
            }
            break;
    }
}

void func_800F6FF4(StageTrap *t, s8 dir, s32 speed) {
    s32 step;

    switch (dir) {
        case 0:
            step = 10;
            break;
        case 1:
            step = -10;
            break;
    }
    switch (t->state) {
        case 0:
            t->homeX = t->x;
            t->homeY = t->y;
            t->homeZ = t->z;
            t->state++;
        case 1:
            if (gSection - UpdatePathSegment(t) < 2) {
                UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
            }
            if (speed * speed > DistanceSquared((VECTOR *)&gPlayer.x, (VECTOR *)t)) {
                t->timer = 0;
                t->state++;
            }
            if (t->touched != 0 && gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
                PlaySe(0x37);
            }
            break;
        case 2:
            if (gSection - UpdatePathSegment(t) < 2) {
                UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
            }
            t->y += step;
            if (DistanceSquared((VECTOR *)&t->homeX, (VECTOR *)t) > 22500) {
                t->timer = 0;
                t->state++;
            }
            if (t->touched != 0 && gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
                PlaySe(0x37);
            }
            break;
        case 3:
            if (gSection - UpdatePathSegment(t) < 2) {
                UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
            }
            if (t->touched != 0 && gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
                PlaySe(0x37);
            }
            break;
    }
}

#ifdef NON_MATCHING
void func_800F7278(StageTrap *t, u8 act) {
    VECTOR v;
    s32 off;
    s32 amp;
    s16 r;
    s16 ang;
    s32 d;
    s32 e;

    switch (act) {
        case 0:
            off = 0x400;
            amp = 125;
            break;
        case 1:
            off = 0xC00;
            amp = 250;
            break;
        case 2:
            off = 0x400;
            break;
        case 3:
            off = 0xC00;
            break;
        case 4:
            off = 0;
            break;
        case 5:
            off = 0x800;
            break;
    }
    if (t->state == 0) {
        t->timer++;
        d = rsin((t->timer * 20 << 12) / 360) * 25 >> 12;
        if (d < 0) {
            d = -d;
        }
        t->y = t->homeY - d;
        switch (act) {
            case 0:
            case 1:
                r = rsin((t->timer * 3 << 12) / 360) * amp >> 12;
                ang = t->rotY + off;
                t->x = t->homeX - (rsin(ang) * r >> 12);
                t->z = t->homeZ - (rcos(ang) * r >> 12);
                break;
            case 2:
            case 3:
            case 4:
            case 5:
                ang = t->rotY + off;
                t->x += rsin(ang) * 20 >> 12;
                t->z += rcos(ang) * 20 >> 12;
                break;
        }
        d = t->x + gWorldShift[0];
        if (d < 0) {
            d = -d;
        }
        if (d < 2000) {
            e = t->z + gWorldShift[2];
            if (e < 0) {
                e = -e;
            }
            if (e < 2000 && t->timer % 3 == 0) {
                v.vx = t->x + (rsin((t->rotY + 0x800) * 10 * (rand() % 10)) >> 12);
                v.vz = t->z + (rcos((t->rotY + 0x800) * 10 * (rand() % 10)) >> 12);
                v.vy = t->y;
                ScatterDust(&v);
            }
        }
        if (gSection - UpdatePathSegment(t) < 2) {
            UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
            if (t->timer * 20 % 180 == 0) {
                PlaySe(0x4E);
            }
        }
        if (t->touched != 0 && gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
            PlaySe(0x36);
        }
    }
}
#else
INCLUDE_ASM("asm/ovl/ovl07/nonmatchings/ovl07", func_800F7278);
#endif

INCLUDE_ASM("asm/ovl/ovl07/nonmatchings/ovl07", func_800F74FC);

void func_800F76DC(StageTrap *t, s32 speed, u8 arg) {
    switch (t->state) {
        case 0:
            if (speed * speed > DistanceSquared((VECTOR *)&gPlayer.x, (VECTOR *)t)) {
                switch (arg) {
                    case 0:
                        t->homeRotX = 0;
                        t->model++;
                        break;
                    case 1:
                        t->homeRotX = 1;
                        t->model--;
                        break;
                }
                PlaySe(0x3B);
                t->state++;
            }
            if (gSection - UpdatePathSegment(t) < 2) {
                DrawObjectModel(&gObjectModels[t->model], t);
            }
            break;
        case 1:
            if (gSection - UpdatePathSegment(t) < 2) {
                if (t->homeRotX == 0) {
                    UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
                } else {
                    DrawObjectModel(&gObjectModels[t->model], t);
                }
            }
            break;
        default:
            return;
    }
    if (t->touched != 0 && gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
        PlaySe(0x36);
    }
}

/* An address inside a function that other code calls as if it were one:
 * the shared dispatchers jump to other stages' entry points, which in this
 * overlay land mid-function. */
#if !defined(M2CTX) && !defined(PERMUTER)
#define FUNC_ALIAS(name, base, off) \
    __asm__(".globl " #name "\n.set " #name ", " #base " + " #off "\n")
#else
#define FUNC_ALIAS(name, base, off)
#endif

extern s8 D_800D3638;
extern StageTrap D_800AC6E0;
extern Vec3 D_8007AD68[];
extern s16 D_800AC6B8;
/* MATCHING: player's, declared only where defined. */
void StopSpeed(void);
void func_800F1A84(void);

void func_800F7890(void) {
    s32 unused[18];
    StageTrap *m;
    u16 i;

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
                gRefView[3] = gView[3];
                gRefView[4] = gView[4];
                gRefView[5] = gView[5];
                m->state++;
            case 1:
                gPlayer.x = Lerp(D_800D39D8[0].homeX, D_8007AD68[gStage].x, m->timer, 25);
                gPlayer.z = Lerp(D_800D39D8[0].homeZ, D_8007AD68[gStage].z, m->timer, 25);
                gView[1] = gRefView[1] + Lerp(0, 65, m->timer, 25);
                gView[4] = gRefView[4] + Lerp(0, 50, m->timer, 25);
                GsSetRefView2((GsRVIEW2 *)gView);
                if (++m->timer >= 26) {
                    m->timer = 0;
                    m->state++;
                }
                break;
            case 2:
                gPlayer.motion[0] = 0;
                if (++m->timer != 0) {
                    D_800AC6B8 = 90;
                    m->timer = 0;
                    m->state++;
                }
                break;
            case 3:
                if (++m->timer < 8 && m->timer % 2 == 0) {
                    PlaySe(0x38);
                }
                if (m->timer >= 9) {
                    m->timer = 0;
                    m->state++;
                    gOffsetVec[0] = gPlayer.x;
                    gOffsetVec[2] = gPlayer.z;
                    for (i = 0; i < 40; i++) {
                        gOffsetVec[1] = gPlayer.y - i * 10;
                        ScatterDust((VECTOR *)gOffsetVec);
                    }
                    func_800F1A84();
                    gMissState = 0;
                    gFellOff = 0;
                    D_800D39D8[1].state = 0;
                    D_800D39D8[1].timer = 0;
                    gPlayer.control = 1;
                }
                gView[1] = gRefView[1] + 100;
                GsSetRefView2((GsRVIEW2 *)gView);
                break;
            case 4:
                DrawFrozenFrame();
                if (++m->timer == 30) {
                    m->state++;
                }
                gView[1] = gRefView[1] + 100;
                GsSetRefView2((GsRVIEW2 *)gView);
                break;
        }
    }
}

/* MATCHING: the type dispatchers call these two addresses inside the
 * function above (other stages' entry points in their own overlays). */
FUNC_ALIAS(func_800F7968, func_800F7890, 0xD8);
FUNC_ALIAS(func_800F79A0, func_800F7890, 0x110);

extern s8 D_800D37A8;
/* MATCHING: main's, stage's and stage_overlay's, declared only where
 * defined. */
void ResetRumble(void);
void StartActResults(void);
s32 IsResultsActive(void);
u16 GetNextStage(u16 i);
void BeginAct3LoadScreen(void);
s8 func_800F82CC(StageTrap *t);

void func_800F7C44(void) {
    StageTrap *t;
    StageTrap *u;

    t = D_800FF84C;
    u = &D_800FF84C[1];
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
                if (func_800F82CC(u)) {
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
                    gStage = GetNextStage(4);
                    BeginAct3LoadScreen();
                    ClipF = 0;
                }
                break;
        }
    }
}

extern s32 *gSectionEdges;

#ifdef NON_MATCHING
void func_800F7E08(s8 arg) {
    u16 periods[8] = {20, 40, 30, 40, 20, 30, 20, 35};
    s16 i;

    if (gSectionEdges[-1] - gSection < 20) {
        for (i = 0; i < 8; i++) {
            if (gFrameCount % periods[i] == 0) {
                func_800FA050((u8 *)0x8018D000, i * 4 + arg * 2, 1);
            } else if (gFrameCount % periods[i] == periods[i] >> 1) {
                func_800FA050((u8 *)0x8018D000, i * 4 + arg * 2 + 1, 1);
            }
        }
    }
}
#else
INCLUDE_ASM("asm/ovl/ovl07/nonmatchings/ovl07", func_800F7E08);
INCLUDE_RODATA("asm/ovl/ovl07/nonmatchings/ovl07", D_800F04F0);
#endif

/* MATCHING: the step's callees, which their units declare only where they
 * define them; this image links against them by address. */
s16 UpdateCamera(s16 x, s16 target, s16 y, s16 height);
void UpdatePlayer(void);
void UpdateStageLights(void);
void PushPlayerFromWalls(void);
void DrawModeObject(void);
void DrawPlayerShadow(void);
s32 PairMarks(void);
s32 UpdatePlayerAction(PlayerState *state);
s32 DrawTodObjects(void);
/* MATCHING: more of the step's callees, declared only where defined. */
void UnusedHook2(void);
void PopInNextCan(void);
void DrawStage(s16 x, s16 y, s16 z);
void DrawSky(u32 arg);
void UpdateEffects(s32 unused, s16 x, s16 y, s16 z);
/* MATCHING: object defines it with no parameters; retail's call passes the
 * ordering table it once took. */
void UpdatePickups(GsOT *ot);
void func_800F8A40(void);

/* MATCHING: an unused 32-byte local gives retail's frame. */
void func_800F7F40(void) {
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
    func_800F8A40();
    func_800F0E04();
    func_800F094C();
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
}

#ifdef NON_MATCHING
void func_800F8184(u8 *dir, u16 id, u16 first, u16 count) {
    u16 i;
    s32 tp;
    DirEnt16 *e;
    TimInfo *t;

    e = (DirEnt16 *)dir + first;
    for (i = first; i < first + count; i++) {
        LoadTim(dir + e->offset);
        DrawSync(0);
        tp = GetTPage(0, 0, ((TimInfo *)gTimInfo)->pixRect.x, ((TimInfo *)gTimInfo)->pixRect.y);
        t = (TimInfo *)gTimInfo;
        DefineSprite(id, 0, (u8)t->width, (u8)t->pixRect.h, (u8)tp,
                     (u8)((t->pixRect.x - t->pixRect.x / 64 * 64) * t->perUnit),
                     (u8)(t->pixRect.y + 1 - t->pixRect.y / 256), t->clutRect.x, (u16)t->clutRect.y);
        id++;
        e++;
    }
}
#else
INCLUDE_ASM("asm/ovl/ovl07/nonmatchings/ovl07", func_800F8184);
#endif

extern GsRVIEW2 D_800FF8DC;
/* MATCHING: stage's, declared only where defined. */
void StartScreenWipe(void);
s32 IsScreenWipeActive(void);

s8 func_800F82CC(StageTrap *t) {
    s8 ret = 0;

    gPlayer.showShadow = 0;
    DrawFrozenFrame();
    switch (t->state) {
        case 0:
            StopCdTrack();
            gPlayer.action = 0;
            gPlayer.motion[0] = 0x36;
            gFellOff = 1;
            gRefView[0] = rsin(gCourse.rot.vy + 0x5C7) * 300 >> 12;
            gRefView[2] = rcos(gCourse.rot.vy + 0x5C7) * 300 >> 12;
            gRefView[1] = gPlayer.y - 150;
            gRefView[3] = rsin(gCourse.rot.vy + 0x800) * 150 >> 12;
            gRefView[5] = rcos(gCourse.rot.vy + 0x800) * 150 >> 12;
            D_800FF8DC = *(GsRVIEW2 *)gView;
            gRefView[4] = gPlayer.y - 100;
            t->homeRotZ = 0;
            t->timer = 0;
            t->state++;
            SetSoundFade(4);
        case 1:
            if (t->homeRotZ == 30) {
                PlaySe(0x22);
            }
            gPlayer.fallSpeed = 0;
            gPlayer.drawX += rsin(gCourse.rot.vy + 0x800) * 3 >> 12;
            gPlayer.drawZ += rcos(gCourse.rot.vy + 0x800) * 3 >> 12;
            gTodCoords[0].coord.t[0] = gPlayer.drawX;
            gTodCoords[0].coord.t[1] = t->homeY + gPlayer.drawY;
            gTodCoords[0].coord.t[2] = gPlayer.drawZ;
            SetCoordRotation((SVECTOR *)gBodyRot, gTodCoords);
            if (t->timer < 60) {
                t->timer++;
            }
            gView[0] = Lerp(((s32 *)&D_800FF8DC)[0], gRefView[0], t->timer, 60);
            gView[1] = Lerp(((s32 *)&D_800FF8DC)[1], gRefView[1], t->timer, 60);
            gView[2] = Lerp(((s32 *)&D_800FF8DC)[2], gRefView[2], t->timer, 60);
            gView[3] = Lerp(((s32 *)&D_800FF8DC)[3], gRefView[3], t->timer, 60);
            gView[4] = Lerp(((s32 *)&D_800FF8DC)[4], gRefView[4], t->timer, 60);
            gView[5] = Lerp(((s32 *)&D_800FF8DC)[5], gRefView[5], t->timer, 60);
            GsSetRefView2((GsRVIEW2 *)gView);
            if (++t->homeRotZ > 60) {
                gRefView[0] = rsin(gCourse.rot.vy + 0x471) * 800 >> 12;
                gRefView[2] = rcos(gCourse.rot.vy + 0x471) * 800 >> 12;
                gRefView[1] = gPlayer.y - 450;
                gRefView[3] = rsin(gCourse.rot.vy + 0x800) * 700 >> 12;
                gRefView[5] = rcos(gCourse.rot.vy + 0x800) * 700 >> 12;
                t->homeRotZ = 0;
                t->homeRotX = 0;
                t->timer = 0;
                t->state++;
            }
            break;
        case 2:
            gPlayer.fallSpeed = 0;
            if (t->homeRotZ == 10) {
                PlaySe(0x1A);
            }
            t->homeRotZ++;
            t->homeRotX++;
            gPlayer.drawY += t->homeRotZ * 2;
            gPlayer.drawX += rsin(gCourse.rot.vy + 0x800) * 25 >> 12;
            gPlayer.drawZ += rcos(gCourse.rot.vy + 0x800) * 25 >> 12;
            gTodCoords[0].coord.t[0] = gPlayer.drawX;
            gPlayer.groundY = gTodCoords[0].coord.t[1] = t->homeY + gPlayer.drawY;
            gTodCoords[0].coord.t[2] = gPlayer.drawZ;
            SetCoordRotation((SVECTOR *)gBodyRot, gTodCoords);
            if (t->timer < 60) {
                t->timer++;
            }
            gView[0] = Lerp(gView[0], gRefView[0], t->timer, 80);
            gView[1] = Lerp(gView[1], gRefView[1], t->timer, 80);
            gView[2] = Lerp(gView[2], gRefView[2], t->timer, 80);
            gView[3] = Lerp(gView[3], gRefView[3], t->timer, 80);
            gView[4] = Lerp(gView[4], gRefView[4], t->timer, 80);
            gView[5] = Lerp(gView[5], gRefView[5], t->timer, 80);
            GsSetRefView2((GsRVIEW2 *)gView);
            if (t->homeRotX > 40) {
                t->timer = 0;
                t->state++;
                StartScreenWipe();
                D_800FAF4E = 1;
            }
            break;
        case 3:
            if (++t->timer == 15) {
                PlaySe(0x33);
            }
            if (IsScreenWipeActive() == 0) {
                t->state++;
                gPlayer.x = 0x1EC0;
                gPlayer.z = 0x18AA;
                gView[0] = rsin(gCourse.rot.vy + 0x800) * 500 >> 12;
                gView[2] = rcos(gCourse.rot.vy + 0x800) * 500 >> 12;
                gView[1] = -0x992;
                gView[3] = rsin(gCourse.rot.vy + 0x800) * 700 >> 12;
                gView[5] = rcos(gCourse.rot.vy + 0x800) * 700 >> 12;
                gView[4] = -0x960;
                GsSetRefView2((GsRVIEW2 *)gView);
            }
            break;
        case 4:
            ret = 1;
            break;
    }
    return ret;
}

/* MATCHING: the type-34 dispatcher calls these addresses inside the
 * function above (other stages' entry points in their own overlays). */
FUNC_ALIAS(func_800F82E0, func_800F82CC, 0x14);
FUNC_ALIAS(func_800F8454, func_800F82CC, 0x188);
FUNC_ALIAS(func_800F84A8, func_800F82CC, 0x1DC);
FUNC_ALIAS(func_800F87E4, func_800F82CC, 0x518);

void func_800F8A40(void) {
    if (gMissState <= 0) {
        func_800F7890();
        func_800FA008();
        func_800FA010();
    }
}

void func_800F8A7C(u8 se) {
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

void func_800F8AE4(void) {
    if (gSkipLandRumble == 1 || D_800FAF4C == 0) {
        bzero(D_800FBECC, 100);
        D_800FAF4C = 1;
        D_800FAF4E = 0;
    }
}

void func_800F8B44(EngSprite *r, u16 i, s16 angle) {
    r->driftBaseY = 0;
    r->driftCount = r->x;
    r->driftPhase = r->z;
    WALK_ANGLE(i) = angle;
    WALK_BASE(i) = r->id;
    WALK_FRAME(i) = rand() % 3;
    D_800FBF3C[i] = 0;
}

void func_800F8C08(StageTrap *t) {
    switch (t->act) {
        case 0:
            break;
        case 1:
            func_800F9F68(t);
            break;
        case 5:
            func_800FA110(t);
            break;
        case 10:
            func_800F14F4(t, 1, 0);
            break;
        case 11:
            func_800F14F4(t, 2, 0);
            break;
        case 12:
            func_800F14F4(t, 1, 1);
            break;
        case 13:
            func_800F14F4(t, 2, 1);
            break;
        case 100:
            func_800F3B08(t, 80);
            break;
        case 101:
            func_800F79A0(t, 50);
            break;
        case 102:
            func_800F79A0(t, 180);
            break;
        case 103:
            func_800F98BC(t);
            break;
        case 104:
            func_800F98C4(t);
            break;
        case 105:
            func_800F1838(t);
            break;
        case 106:
            func_800F1D5C(t);
            break;
    }
}

void func_800F8D10(StageTrap *t, s8 reverse) {
    s16 dir;

    dir = 1;
    if (reverse == 1) {
        dir = -1;
    }
    switch (t->act) {
        case 0:
        default:
            func_800F24DC(t, 0);
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
            func_800F24DC(t, t->act * 10 * dir);
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
            func_800F28FC(t, (t->act - 10) * 10 * dir);
            break;
    }
}

void func_800F8DF4(StageTrap *t) {
    switch (t->act) {
        case 0:
        default:
            func_800F24DC(t, 0);
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
            func_800F2C24(t, t->act * 10, 0);
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
            func_800F2C24(t, (t->act - 10) * 10, 1);
            break;
    }
}

#ifdef NON_MATCHING
void func_800F8E88(StageTrap *t) {
    Speeds sp = D_800F014C;

    func_800F2DCC(t, t->act * 10 + 40, sp.v[t->act / 4]);
}
#else
INCLUDE_ASM("asm/ovl/ovl07/nonmatchings/ovl07", func_800F8E88);
#endif

void func_800F8F04(StageTrap *t, s8 reverse) {
    s16 dir;

    dir = 1;
    if (reverse == 1) {
        dir = -1;
    }
    func_800F2C24(t, t->act * 10 + dir * 60, 0);
    func_800F9990(t);
    if (t->state == 0) {
        PlaySe(0x11);
    }
}

void func_800F8F94(StageTrap *t) {
    Speeds sp = D_800F014C;

    func_800F2DCC(t, t->act * 10 + 60, sp.v[t->act / 5]);
    func_800F9990(t);
    if (t->state == 0) {
        PlaySe(0x11);
    }
}

#ifdef NON_MATCHING
void func_800F9044(StageTrap *t, s8 dir) {
    s16 v[2] = {60, 90};

    func_800F9150(t, t->act * 10 + 60, v[t->act / 5], dir);
}
#else
INCLUDE_ASM("asm/ovl/ovl07/nonmatchings/ovl07", func_800F9044);
#endif

void func_800F90AC(StageTrap *t, s8 mode) {
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
                func_800F99F8(t, t->act * 10, 0);
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
                func_800F99F8(t, -((t->act - 10) * 10), 2);
                break;
        }
    } else {
        switch (t->act) {
            case 0:
            case 1:
            case 2:
            case 3:
                func_800F99F8(t, (t->act + 1) * 10, mode);
                break;
        }
    }
}

void func_800F9150(StageTrap *t, u16 speed, s16 arg, s8 dir) {}

void func_800F9158(StageTrap *t, s8 dir) {
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
            func_800F9808(t, t->act * 10, dir, 0);
            break;
        case 11:
            func_800F9808(t, 60, 5, 0);
            break;
    }
}

void func_800F91C0(StageTrap *t, s8 dir) {
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
            func_800F9808(t, t->act * 10, dir, 0);
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
            func_800F9808(t, (t->act - 10) * 10, dir + 1, 0);
            break;
    }
}

void func_800F9230(StageTrap *t, s8 dir) {
    switch (t->act) {
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
            func_800F9808(t, (t->act + 6) * 10, dir, 0);
            break;
    }
}

void func_800F9280(StageTrap *t) {
    switch (t->act) {
        case 0:
        case 1:
        case 2:
            func_800F9810(t, (t->act + 1) * 10, 60);
            break;
        case 3:
        case 4:
        case 5:
            func_800F9810(t, (t->act - 2) * 10, 90);
            break;
    }
}

void func_800F92EC(StageTrap *t) {
    switch (t->act) {
        case 0:
            func_800F9808(t, 5, 4, 250);
            break;
        case 1:
            func_800F9808(t, 10, 4, 250);
            break;
        case 2:
            func_800F9808(t, 5, 4, 500);
            break;
        case 3:
            func_800F9808(t, 10, 4, 500);
            break;
        case 4:
            break;
    }
}

void func_800F9370(StageTrap *t) {
    func_800F9818(t, D_800FAF50[t->act]);
}

void func_800F93A8(StageTrap *t, s8 dir) {
    func_800F99A0(t, t->act * 10 + 70, 45, dir);
}

void func_800F93E4(StageTrap *t) {
    func_800F9C90(t, t->act * 10 + 20, 5000);
}

void func_800F9418(StageTrap *t) {
    func_800F9C98(t, t->act * 10 + 10);
}

void func_800F944C(StageTrap *t) {
    func_800F99A8(t, t->act * 10 + 80, 7000);
}

void func_800F9480(StageTrap *t, s8 dir) {
    if (t->act < 10) {
        func_800F99B0(t, t->act * 10 + 10, 1200, dir);
    } else {
        func_800F99B0(t, -((t->act - 10) * 10 + 10), 1200, dir);
    }
}

void func_800F94F8(StageTrap *t, s8 dir) {
    func_800F99B8(t, t->act * 10 + 10, 1200, dir);
}

void func_800F9534(StageTrap *t) {
    switch (t->act) {
        case 0:
            func_800F99C0(t, 1000, 1, 0);
            break;
        case 1:
            func_800F99C0(t, 1000, 2, 0);
            break;
        case 2:
            func_800F99C0(t, 2000, 1, 0);
            break;
        case 3:
            func_800F99C0(t, 2000, 2, 0);
            break;
        case 4:
            func_800F99C0(t, 3000, 1, 0);
            break;
        case 5:
            func_800F99C0(t, 3000, 2, 0);
            break;
    }
}

void func_800F95C0(StageTrap *t) {
    func_800F99C8(t, t->act * 5 + 10);
}

void func_800F95F0(StageTrap *t, s8 dir) {
    func_800F99D0(t, (u8)(t->act % 3) * 10 + 10, 700, dir);
}

void func_800F9650(StageTrap *t) {
    switch (t->act) {
        case 0:
            func_800F99D8(t, 5, 250);
            break;
        case 1:
            func_800F99D8(t, 10, 250);
            break;
        case 2:
            func_800F99D8(t, 5, 500);
            break;
        case 3:
            func_800F99D8(t, 10, 500);
            break;
    }
}

void func_800F96D0(StageTrap *t, s8 unused) {
    switch (t->act) {
        case 0:
        case 1:
        case 2:
            func_800F99E0(t, 2, t->act);
            break;
        case 3:
        case 4:
        case 5:
            func_800F99E0(t, 1, t->act % 3);
            break;
    }
}

void func_800F9740(StageTrap *t) {
    switch (t->act) {
        case 0:
        case 1:
        case 2:
        case 3:
            func_800F99E8(t, t->act * 1000 + 1000);
            break;
    }
}

void func_800F9790(StageTrap *t) {
    switch (t->act) {
        case 0:
            func_800F99F0(t, 0);
            break;
        case 1:
            func_800F99F0(t, 1);
            break;
    }
}

void func_800F97D4(StageTrap *t) {
    func_800F326C(t, t->act * 10 + 30);
}

void func_800F9808(StageTrap *t, u16 speed, s8 dir, s32 arg) {}

void func_800F9810(StageTrap *t, u16 speed, s32 arg) {}

void func_800F9818(StageTrap *t, u16 arg) {}

void func_800F9820(StageTrap *t) {
    if (gSection - UpdatePathSegment(t) < 2) {
        UpdateObjectEntry(&gObjectModels[t->model], (Trap *)t);
    }
    if (t->touched != 0 && gPlayer.invulnTimer == 0 && gPlayer.hittable == 1) {
        PlaySe(0x39);
    }
}

void func_800F98BC(StageTrap *t) {}

void func_800F98C4(StageTrap *t) {}

void func_800F98CC(StageTrap *t) {}

void func_800F98D4(StageTrap *t) {}

void func_800F98DC(StageTrap *t) {}

void func_800F98E4(StageTrap *t) {}

void func_800F98EC(StageTrap *t, s32 arg) {}

void func_800F98F4(StageTrap *t) {}

void func_800F98FC(StageTrap *t) {}

void func_800F9904(StageTrap *t, s32 range, u8 type, u8 act) {
    if (t->state == 0) {
        func_800F24DC(t, 0);
        if (range * range > DistanceSquared((VECTOR *)&gPlayer.x, (VECTOR *)t)) {
            t->state = 0;
            t->timer = 0;
            t->type = type;
            t->act = act;
        }
    }
}

void func_800F9990(StageTrap *t) {
    s32 unused[4];
}

void func_800F99A0(StageTrap *t, u16 speed, s32 arg, s8 dir) {}

void func_800F99A8(StageTrap *t, u16 speed, s32 arg) {}

void func_800F99B0(StageTrap *t, s32 speed, s32 arg, s8 dir) {}

void func_800F99B8(StageTrap *t, u16 speed, s32 arg, s8 dir) {}

void func_800F99C0(StageTrap *t, s32 speed, s32 arg, s32 arg2) {}

void func_800F99C8(StageTrap *t, u16 speed) {}

void func_800F99D0(StageTrap *t, u16 speed, s32 arg, s8 dir) {}

void func_800F99D8(StageTrap *t, s32 speed, s32 arg) {}

void func_800F99E0(StageTrap *t, s32 arg, s8 arg2) {}

void func_800F99E8(StageTrap *t, s32 arg) {}

void func_800F99F0(StageTrap *t, s32 arg) {}

void func_800F99F8(StageTrap *t, s16 speed, s8 mode) {}

void func_800F9A00(StageTrap *t) {
    switch (t->act) {
        case 0:
            func_800F6098(t, 500, 1);
            break;
        case 1:
            func_800F6098(t, 500, 2);
            break;
        case 2:
            func_800F6098(t, 1000, 1);
            break;
        case 3:
            func_800F6098(t, 1000, 2);
            break;
        case 4:
            func_800F6098(t, 2000, 1);
            break;
        case 5:
            func_800F6098(t, 2000, 2);
            break;
        case 6:
            func_800F6098(t, 3000, 1);
            break;
        case 7:
            func_800F6098(t, 3000, 2);
            break;
    }
}

void func_800F9AA0(StageTrap *t) {
    switch (t->act) {
        case 0:
            func_800F645C(t, 1000, 1);
            break;
        case 1:
            func_800F645C(t, 1000, 2);
            break;
        case 2:
            func_800F645C(t, 2000, 1);
            break;
        case 3:
            func_800F645C(t, 2000, 2);
            break;
        case 4:
            func_800F645C(t, 3000, 1);
            break;
        case 5:
            func_800F645C(t, 3000, 2);
            break;
        case 6:
            func_800F67E4(t, 0);
            break;
        case 7:
            func_800F67E4(t, 1);
            break;
    }
}

void func_800F9B44(StageTrap *t, u8 arg) {
    switch (t->act) {
        case 0:
            func_800F6B0C(t, 2, 1000, arg);
            break;
        case 1:
            func_800F6B0C(t, 3, 1000, arg);
            break;
        case 2:
            func_800F6B0C(t, 1, 1000, arg);
            break;
        case 3:
            func_800F6B0C(t, 2, 2000, arg);
            break;
        case 4:
            func_800F6B0C(t, 3, 2000, arg);
            break;
        case 5:
            func_800F6B0C(t, 1, 2000, arg);
            break;
        case 6:
            func_800F6B0C(t, 2, 3000, arg);
            break;
        case 7:
            func_800F6B0C(t, 3, 3000, arg);
            break;
        case 8:
            func_800F6B0C(t, 1, 3000, arg);
            break;
    }
}

void func_800F9BF8(StageTrap *t) {
    switch (t->act) {
        case 0:
            func_800F6FF4(t, 0, 1000);
            break;
        case 1:
            func_800F6FF4(t, 1, 1000);
            break;
        case 2:
            func_800F6FF4(t, 0, 2000);
            break;
        case 3:
            func_800F6FF4(t, 1, 2000);
            break;
        case 4:
            func_800F6FF4(t, 0, 3000);
            break;
        case 5:
            func_800F6FF4(t, 1, 3000);
            break;
        case 6:
            func_800F6FF4(t, 0, 3000);
            break;
        case 7:
            func_800F6FF4(t, 1, 3000);
            break;
    }
}

void func_800F9C90(StageTrap *t, u16 speed, s32 arg) {}

void func_800F9C98(StageTrap *t, u16 speed) {}

void func_800F9CA0(StageTrap *t) {
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

void func_800F9D9C(StageTrap *t, u8 arg) {
    switch (t->act) {
        case 0:
            func_800F76DC(t, 500, arg);
            break;
        case 1:
            func_800F76DC(t, 1000, arg);
            break;
        case 2:
            func_800F76DC(t, 1500, arg);
            break;
    }
}

void func_800F9E00(StageTrap *t) {
    func_800F7278(t, t->act);
}

void func_800F9E24(StageTrap *t) {
    StageTrap copy;

    func_800F495C(t);
    if (t->state == 1) {
        copy = *t;
        copy.y -= 150;
        func_800F495C(&copy);
    }
}

void func_800F9EB0(StageTrap *t) {
    func_800F3644(t, 70, 0);
}

void func_800F9ED4(StageTrap *t) {
    func_800F3644(t, 30, 0);
}

void func_800F9EF8(StageTrap *t) {
    func_800F3644(t, 30, 0);
}

void func_800F9F1C(StageTrap *t) {
    func_800F33B8(t);
}

void func_800F9F3C(StageTrap *t) {
    func_800F3644(t, 10, 0);
}

void func_800F9F60(StageTrap *t) {}

void func_800F9F68(StageTrap *t) {
    Query30 q;
    StageTrap *k;

    k = &((StageTrap *)gTraps)[195];
    if (k->state >= 0 && (k->state < 4 || (k->state == 4 && k->timer < 4))) {
        if (gSection - UpdatePathSegment(t) < 2) {
            DrawObjectModel(&gObjectModels[t->model], t);
        }
    }
}

void func_800FA008(void) {}

void func_800FA010(void) {
    func_800FA048();
    func_800F7C44();
    func_800F7E08(D_800FAF4E);
}

void func_800FA048(void) {}

#ifdef NON_MATCHING
void func_800FA050(u8 *dir, u16 first, u16 count) {
    u16 i;
    DirEnt16 *e;

    e = (DirEnt16 *)dir + first;
    for (i = first; i < first + count; i++) {
        LoadTim(dir + e->offset);
        e++;
        DrawSync(0);
    }
}
#else
INCLUDE_ASM("asm/ovl/ovl07/nonmatchings/ovl07", func_800FA050);
#endif

void func_800FA0D8(void) {
    ClearCanGroups();
    gCanGroupOn[1] = 1;
    gCanGroupOn[2] = 1;
    ShowRevealedCans();
}

#ifdef NON_MATCHING
void func_800FA110(StageTrap *t) {
    s32 r;

    if ((gFrameCount & 3) == 0) {
        r = rand();
        SpawnEffectWithParams(12, t->x, t->y, t->z, 0, t->rotY + 0x800, ABS(r % 10 + 15), 20, 0);
    }
}
#else
INCLUDE_ASM("asm/ovl/ovl07/nonmatchings/ovl07", func_800FA110);
#endif

extern s32 D_800FB1A0[];
extern s32 D_800FB1C0[];

/* MATCHING: main's views of the loaders. */
void SetBootReady(s32 arg);
void LoadTimPack(void *pack);
s32 LoadVabBank(s32 bank);
void LoadStageData(unsigned long *p);
void SetHitBanks(void *a, void *b);
void LoadSpritePack(void *dir, u16 id);
void ResetStageObjects(void);
void StartStageOverlay(void);
void RestoreStageObjects(void);
s32 LoadStageSprites(void);

s32 func_800FA1C0(void) {
    /* MATCHING: an unused 8-byte local gives retail's frame. */
    s32 ret = 0;
    s32 unused[2];

    switch (gSubStep) {
        case 0:
            SetBootReady(0);
            gCdLoadLoc[0] = (s32)&D_800FB1A0[0];
            gCdLoadDest[0] = 0x8019D000;
            gCdLoadCount = 1;
            gCdLoadState = CD_LOAD_SEEK;
            gCdLoadSectors[0] = D_800FB1C0[1];
            gSubStep++;
            break;
        case 1:
            if (gCdLoadState == CD_LOAD_DONE) {
                LoadTimList((unsigned long *)0x8019D000);
                gVabLoadStep = 0;
                gSubStep++;
            }
            break;
        case 2:
            if (LoadVabBank(3) == 1) {
                gSubStep++;
            }
            break;
        case 3:
            gCdLoadLoc[0] = (s32)&D_800FB1A0[1];
            gCdLoadDest[0] = 0x8019D000;
            gCdLoadCount = 1;
            gCdLoadState = CD_LOAD_SEEK;
            gCdLoadSectors[0] = D_800FB1C0[2];
            gSubStep++;
            break;
        case 4:
            if (gCdLoadState == CD_LOAD_DONE) {
                LoadTimPack((void *)0x8019D000);
                gSubStep++;
            }
            break;
        case 5:
            gCdLoadLoc[0] = (s32)&D_800FB1A0[2];
            gCdLoadDest[0] = 0x8019D000;
            gCdLoadLoc[1] = (s32)&D_800FB1A0[3];
            gCdLoadDest[1] = 0x80155000;
            gCdLoadCount = 2;
            gCdLoadState = CD_LOAD_SEEK;
            gCdLoadSectors[0] = D_800FB1C0[3];
            gCdLoadSectors[1] = D_800FB1C0[4];
            gSubStep++;
            break;
        case 6:
            if (gCdLoadState == CD_LOAD_DONE) {
                LoadStageData((unsigned long *)0x8019D000);
                gSubStep++;
            }
            break;
        case 7:
            gCdLoadLoc[0] = (s32)&D_800FB1A0[4];
            gCdLoadDest[0] = 0x8016D000;
            gCdLoadCount = 1;
            gCdLoadState = CD_LOAD_SEEK;
            gCdLoadSectors[0] = D_800FB1C0[5];
            gSubStep++;
            break;
        case 8:
            if (gCdLoadState == CD_LOAD_DONE) {
                LoadSpritePack((void *)0x8016D000, SPR_STAGE);
                gSubStep++;
            }
            break;
        case 9:
            InitEntryModels();
            gSubStep++;
            break;
        case 10:
            gCdLoadLoc[0] = (s32)&D_800FB1A0[5];
            gCdLoadDest[0] = 0x8016D000;
            gCdLoadCount = 1;
            gCdLoadState = CD_LOAD_SEEK;
            gCdLoadSectors[0] = D_800FB1C0[6];
            gSubStep++;
            break;
        case 11:
            if (gCdLoadState == CD_LOAD_DONE) {
                SetHitBanks((void *)0x8016D000, (void *)0x80171744);
                RestoreStageObjects();
                LoadStageSprites();
                gSubStep++;
            }
            break;
        case 12:
            gCdLoadLoc[0] = (s32)&D_800FB1A0[6];
            gCdLoadDest[0] = 0x8018D000;
            gCdLoadCount = 1;
            gCdLoadState = CD_LOAD_SEEK;
            gCdLoadSectors[0] = D_800FB1C0[7];
            gSubStep++;
            break;
        case 13:
            if (gCdLoadState == CD_LOAD_DONE) {
                ResetStageObjects();
                StartStageOverlay();
                SetBootReady(1);
                ret = 1;
            }
            break;
    }
    FntPrint("CDEXEC %d", gSubStep);
    return ret;
}

/* MATCHING: object's view, with this unit's trap record. */
void ResetObj34(StageTrap *t);

/* MATCHING: ret set before the table copy and again after the call, so it
 * stays a register; set once it folds to a constant. */
u16 func_800FA580(s16 id, StageTrap *src, StageTrap *dst) {
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

/** @brief The stage's start record: the start point, negated, and the
 *         start heading, negated. */
typedef struct {
    s16 x;   /**< the start x, negated */
    s16 y;   /**< the start y, negated */
    s16 z;   /**< the start z, negated */
    s16 yaw; /**< the start heading, negated */
} StartRec;

/* MATCHING: a per-unit view; stage stores it as an s32. */
extern StartRec *gStageStart;
extern s16 sCamOffsetX;
/* MATCHING: stage's sCamOffsetX and the sCamOffsetZ after it, reached as
 * one pair. */
#define sCamOffset (&sCamOffsetX)
extern s16 sCamHeight;
extern u16 sViewSpeed;

#ifdef NON_MATCHING
void func_800FA6C0(void) {
    s16 yaw;

    /* MATCHING: stored as the s16 it is read as. */
    *(s16 *)&gCameraYaw = -0x201;
    sCamHeight = -150;
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
    gPlayer.drawX = 0;
    gPlayer.drawY = 0;
    gPlayer.drawZ = 0;
    gCourse.z = gPlayer.z = -gStageStart->z;
    gBodyRot[0] = 0;
    gBodyRot[1] = 0;
    gBodyRot[2] = 0;
    gView[1] = (s16)(gCourse.y - 150);
    func_800FA0D8();
}
#else
INCLUDE_ASM("asm/ovl/ovl07/nonmatchings/ovl07", func_800FA6C0);
#endif

/** @brief The shape of each of the stage's own sprites, as one block. */
typedef struct {
    u8 shape[100]; /**< a shape per sprite id from SPR_STAGE on */
} SpriteShapes;

extern u8 sSpriteShapes[];
extern const SpriteShapes D_800F08E8;

void func_800FA800(void) {
    SpriteShapes shapes;

    shapes = D_800F08E8;
    *(SpriteShapes *)sSpriteShapes = shapes;
}

INCLUDE_RODATA("asm/ovl/ovl07/nonmatchings/ovl07", D_800F08E8);
