/* What the act-3 chase stages (1-3, 2-3, 4-3 and 5-3) share word for word:
 * the record views, the player's jump and step, its model and shadow draws
 * and the HUD. Each stage's unit defines
 *   CHASE_LENGTH       the run's length, which scales the progress bar,
 *   CHASE_PLACEMENTS   how many objects its placement list holds,
 * includes this, defines its own StartChaseJump, includes chase_head.inc,
 * and includes chase_tail.inc after its own objects, step and lights. */

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

/** @brief The giant can that rolls after the player: its body and its lid. */
typedef struct {
    s32 x;    /**< the body's x; objects are knocked away as it reaches them */
    s32 y;    /**< the body's y */
    s32 z;    /**< the body's z */
    s32 pad;  /**< not yet known */
    s32 lidX; /**< the lid's x, in the opening scene */
    s32 lidY; /**< the lid's y */
    s32 lidZ; /**< the lid's z */
} ChaseCan;

extern ChaseCan D_8009EA78;
extern SVECTOR D_8009EB58[2]; /* the body's and the lid's rotations */
#define sCan D_8009EA78
#define sCanRot D_8009EB58
extern u8 gRumbleEnabled;
extern s16 gRumbleTime;
extern s16 gRumbleSmall[];
/* MATCHING: stage defines it with s16 positions; this unit passes words. */
s32 SpawnEffect(s32 kind, s32 x, s32 y, s32 z, s32 delay);
extern s32 sJumpSpeed; /* the jump's vertical speed */
extern s32 sChaseLive;
extern s32 sHurtTimer; /* frames the player flashes after a hit */
extern Act3Place sPlacements[CHASE_PLACEMENTS];
extern CdlLOC sChaseFileLoc[];
extern s32 sChaseFileSectors[];

/** @brief The three flat lights of the scene. */
typedef struct {
    GsF_LIGHT l[3]; /**< one per light index */
} LightTable;

/* MATCHING: a struct lvalue keeps the table's base in a register. */
#define sLights (*(LightTable *)gFlatLights)
