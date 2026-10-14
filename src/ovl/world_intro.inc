/* What the five world intro overlays share word for word: the model and
 * animation loader, the animation step, the captions, and the draws of the
 * player, the partner and the scene's prop. Each intro's own unit includes
 * it after defining
 *   INTRO_CAPTIONS         how many captions it shows (0: none),
 *   INTRO_CAPTION_TIMES    their frame numbers, one more than the captions,
 *   INTRO_SORT_MODES       1 if its model draws set the TMD sort mode,
 *   DEFINE_INTRO_SPRITES   a statement defining its caption or text sprites,
 *                          given the loader's counter,
 *   INTRO_WORLD            its world, 1 to 5, which picks its intro pack,
 *   INTRO_XA               1 if it plays an XA track and counts its frames,
 * and then writes its own scene script, the step main calls each frame. */

#include "common.h"
#include "libgte.h"
#include "libgpu.h"
#include "libgs.h"
#include "libcd.h"
#include "libetc.h"
#include "rand.h"
#include "model.h"
#include "draw.h"
#include "sound.h"
#include "spad.h"
#include "world_intro.h"
#include "disc_files.h"

/* MATCHING: model types the table as its own record; here it is the
 * overlay's words, as in main. */
void InitTodObjTable(s32 *list, GsDOBJ2 *objs, GsCOORDINATE2 *coords, GsCOORD2PARAM *params, s32 n);
/* MATCHING: main's s32 view, which keeps sound.h from declaring it. */
s32 LoadVabBank(s32 bank);

#if INTRO_CAPTIONS
/** @brief The caption table: each caption's first frame, and the next one's. */
typedef struct {
    s32 t[INTRO_CAPTIONS + 1]; /**< frame numbers */
} CaptionTimes;
#endif

/* The overlay's data, in retail's order. All of it is initialised, so all
 * of it is .data. */

/* Two copies of the disc's file table; the intro loads from the first. */
#define DISC_FILE_PREFIX sDisc
#include "disc_file_tables.inc"
#define DISC_FILE_PREFIX sDiscCopy
#include "disc_file_tables.inc"

/* The shared file holding the intro's models, animations and sprites. */
#define INTRO_PACK DISC_INTRO_PACK(INTRO_WORLD)

/* The intro's own variables. */
#if INTRO_XA
s32 sIntroFrame = 0;   /* frames since the XA track started */
s32 sScriptPaused = 0; /* never set here: nonzero would freeze the script */
#else
/* MATCHING: never read; retail's value. */
s32 sIntroSpare = 0x02E043FF;
#endif
GsCOORDINATE2 sPropCoord = {0};
PlayerState sPartner = {0};
GsDOBJ2 sPartnerObjs[20] = {{0}};
s32 sPartnerTmd = 0;
s32 sPartnerSpare = 0; /* never read */
/* The partner's TodObjTable (model.c): its objects, how many are in use,
 * how many there are, and a word main's table also has. */
s32 sPartnerObjTable[4] = {0};
GsCOORDINATE2 sPartnerCoords[20] = {{0}};
GsDOBJ2 sProp = {0};
VECTOR sPropPos = {0};
SVECTOR sPropRot = {0};
GsCOORD2PARAM sPartnerCoordParams[20] = {{{0}}};

/* The camera script's step. */
/* MATCHING: kept in the fog fade's word, reached as its own symbol. */
#define sSubStep gFogFade

/** @brief The three flat lights of the scene. */
typedef struct {
    GsF_LIGHT l[3]; /**< one per light index */
} LightTable;

/* MATCHING: a struct lvalue keeps the table's base in a register. */
#define sLights (*(LightTable *)gFlatLights)

#if INTRO_CAPTIONS
const CaptionTimes sCaptionTimes = {{INTRO_CAPTION_TIMES}};
#endif

u8 StepIntroMotion(PlayerState *obj, u16 index, s32 objTable, s32 tmdTable) {
    if (obj->motion[index] != obj->curMotion[index]) {
        obj->motionTime[index] = obj->motionStart[obj->motion[index]];
        obj->motionData[index] = (u8 *)gAnimations[obj->motion[index]];
        obj->curMotion[index] = obj->motion[index];
    }
    if (obj->motionTime[index] >=
        obj->motionLen[obj->curMotion[index]] + obj->motionStart[obj->curMotion[index]]) {
        return 0;
    }
    obj->motionTime[index]++;
    obj->motionData[index] = SetTodFrame(obj->motionTime[index], obj->motionData[index], objTable,
                                         (s32)gTodTmdIds, tmdTable, 0, 0);
    return 1;
}

void LoadIntroPack(s32 *pack) {
    s32 *dir;
    s32 i;
    s32 n;

    dir = (s32 *)(pack[0] + (s32)pack);
    sPartnerTmd = dir[0] + (s32)dir + 4;
    GsMapModelingData((unsigned long *)sPartnerTmd);
    InitTodObjTable(sPartnerObjTable, sPartnerObjs, sPartnerCoords, sPartnerCoordParams, 20);
    GsInitCoordinate2(WORLD, sPartnerCoords);
    GsMapModelingData((unsigned long *)(dir[1] + (s32)dir + 4));
    GsLinkObject4((unsigned long)(dir[1] + (s32)dir + 12), &sProp, 0);
    sProp.coord2 = &sPropCoord;
    sProp.attribute = 0;
    GsInitCoordinate2(WORLD, &sPropCoord);
    dir = (s32 *)(pack[1] + (s32)pack);
    n = dir[0] / 4;
    for (i = 0; i < n; i++) {
        gAnimations[i + 51] = (s32 *)(dir[i] + (s32)dir) + 1;
        gPlayer.motionLen[i + 51] = *gAnimations[i + 51];
        gAnimations[i + 51]++;
        gPlayer.motionStart[i + 51] = gAnimations[i + 51][1];
    }
    LoadTimList((unsigned long *)(pack[2] + (s32)pack));
    DEFINE_INTRO_SPRITES(i);
    DefineSprite(SPR_STAGE + 30, 0, 0xFF, 0x58, 6, 0, 0, 0x10, 0x1FD);
}

#if INTRO_CAPTIONS
/* MATCHING: inline, so that the intro's frame step expands it and the
 * compiler writes this copy after it. */
inline void DrawCaptions(s32 time) {
    SVECTOR pos;
    SColor color;
    CaptionTimes times;
    s32 i;

    times = sCaptionTimes;
    for (i = 0; i < INTRO_CAPTIONS; i++) {
        if (times.t[i] < time && time < times.t[i + 1]) {
            pos.vx = -0x80;
            pos.vy = 0x48;
            color.r = BLEND_OPAQUE;
            color.cd = 0x80;
            color.b = 0x80;
            color.g = 0x80;
            DrawSprite(SPR_STAGE + i, &pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
        }
    }
}
#endif

/* MATCHING: inline, so that the intro's frame step expands it and the
 * compiler writes this copy after it. */
inline void DrawIntroProp(void) {
    MATRIX m;

    sPropCoord.coord.t[0] = sPropPos.vx;
    sPropCoord.coord.t[1] = sPropPos.vy;
    sPropCoord.coord.t[2] = sPropPos.vz;
    SetCoordRotation(&sPropRot, &sPropCoord);
    GsGetLs(sProp.coord2, &m);
    GsSetLsMatrix(&m);
    GsGetLw(sProp.coord2, &m);
    GsSetLightMatrix(&m);
    GsSortObject4(&sProp, &gSubOt[gActiveBuff], 2, (u_long *)0x1F800000);
}

/* Draws a model's objects at its position and yaw. It counts the player's
 * objects whichever list it is handed. */
static inline void DrawIntroModel(PlayerState *p, s32 *list) {
    MATRIX m;
    SVECTOR rot;
    GsDOBJ2 *o;
    s32 i;

    o = ((DrawList *)list)->objs;
    o->coord2->coord.t[0] = p->x;
    o->coord2->coord.t[1] = p->y;
    o->coord2->coord.t[2] = p->z;
    rot.vx = 0;
    rot.vz = 0;
    rot.vy = p->yaw;
    SetCoordRotation(&rot, o->coord2);
    for (i = 0; i < ((DrawList *)gTodObjTable)->count; o++, i++) {
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
        /* MATCHING: three barriers lengthen the loop so that global-alloc
         * ranks the object pointer above the counter (retail's s1 and s2). */
        __asm__("");
        __asm__("");
        __asm__("");
    }
}

/* Steps the player's first animation channel and draws the player. */
static inline void DrawIntroPlayer(void) {
    s8 r;

    gPlayer.motionResult[0] = r = StepIntroMotion(&gPlayer, 0, (s32)gTodObjTable, gModelTmd);
    switch (gPlayer.action) {
        case ACT_MODEL_ONCE:
            break;
        case ACT_MODEL_LOOP:
            if (r == 0) {
                gPlayer.curMotion[0] = MOT_OFF;
            }
            break;
    }
#if INTRO_SORT_MODES
    SetTmdSortMode(0);
#endif
    DrawIntroModel(&gPlayer, gTodObjTable);
}

/* The same for the partner, from its own model list. */
static inline void DrawIntroPartner(void) {
    s8 r;

    sPartner.motionResult[0] = r = StepIntroMotion(&sPartner, 0, (s32)sPartnerObjTable, sPartnerTmd);
    switch (sPartner.action) {
        case ACT_MODEL_ONCE:
            break;
        case ACT_MODEL_LOOP:
            if (r == 0) {
                sPartner.curMotion[0] = MOT_OFF;
            }
            break;
    }
#if INTRO_SORT_MODES
    SetTmdSortMode(4);
#endif
    DrawIntroModel(&sPartner, sPartnerObjTable);
}
