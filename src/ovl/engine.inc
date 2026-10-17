/* The stage engine's declarations: the records, the routines and data the
 * engine's code refers to. Included once by each of the eight overlays built
 * from the engine (ovl03, 04, 06, 07, 12, 13, 15, 16) before the sections of
 * engine_data.inc (the data the engine defines) and engine_code.inc. */


#include "common.h"
#include "ram.h"
#include "memory.h"
#include "libgte.h"
#include "libgpu.h"
#include "libgs.h"
#include "libcd.h"
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
#define WALK_ANGLE(i) sWalkPose[(i) * 3]
#define WALK_BASE(i) sWalkPose[(i) * 3 + 1]
#define WALK_FRAME(i) sWalkPose[(i) * 3 + 2]

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
    s16 counter;     /**< a count some traps keep */
    u8 unk3A[2];     /**< not yet known */
    s32 section;     /**< the section it was placed in */
    u8 touched;      /**< 1 when the player touched one of its hit points this frame */
    u8 marks;        /**< 0x40: the warning has been shown */
    u8 type;         /**< what it does, the step it runs */
    u8 act;          /**< its act */
    s32 aux;         /**< a word some traps keep: their copy's slot in sSpawnedTraps, or a mode */
} StageTrap;

/** @brief An object model slot as a model with its own transform. */
typedef struct {
    GsDOBJ2 obj;         /**< the object handler */
    GsCOORDINATE2 coord; /**< the object's coordinate system */
    SVECTOR rot;         /**< rotation */
    SVECTOR scale;       /**< scale, 0x1000 = 1 */
    u8 unk70[8];         /**< not yet known */
} ModelSlot;

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
extern s8 gRideSwitchState[];
extern EngSprite gStageSprites[];
extern StageTrap gEventTraps[];

/** @brief Six speeds, copied whole. */
typedef struct {
    s16 v[6]; /**< the speeds */
} Speeds;

extern const Speeds sChargeRanges;

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
void ResetSpriteWalks(void);
void StartSpriteWalk(EngSprite *r, u16 i, s16 angle);
void StepSpriteWalk(EngSprite *r, u16 i, s16 angle, u16 speed, s32 range, s32 limit);
void AdvanceSpriteWalk(EngSprite *r, u16 i, u16 speed);
void ResetSpawnedTraps(void);
/* MATCHING: object's views, with this unit's records. */
s32 UpdatePathSegment(StageTrap *t);
void DrawObjectModel(ObjectModel *rec, StageTrap *t);
s32 CollidePlayer(Query30 *q);
void PolarToXZ(s32 r, s32 deg, Vec3 *out);
s16 FindFreeSlot48(void);
void StepStageTrap(StageTrap *t);
void RunTrapTypeFF(StageTrap *t);
void StepFlungPiece(StageTrap *t);
void StepMissDebris(StageTrap *t);
void RunTrapType01(StageTrap *t, s8 reverse);
void RunTrapType09(StageTrap *t, s8 reverse);
void RunTrapType0B(StageTrap *t, s8 dir);
void RunTrapType0D(StageTrap *t, s8 dir);
void RunTrapType0F(StageTrap *t, s8 dir);
void RunTrapType10(StageTrap *t, s8 dir);
void RunTrapType11(StageTrap *t, s8 dir);
void RunTrapType15(StageTrap *t, u8 arg);
void RunTrapType18(StageTrap *t, s8 dir);
void RunTrapType1A(StageTrap *t, s8 dir);
void RunTrapType20(StageTrap *t, s8 dir);
void RunTrapType29(StageTrap *t, s8 unused);
void RunTrapType2C(StageTrap *t, u8 arg);
void RunTrapType32(StageTrap *t, s8 mode);
void RunTrapType05(StageTrap *t, s8 dir);
void PlaySpriteHitSe(u8 se);
void StepMissSequence(void);
void StepSolidTrap(StageTrap *t);
void ShatterTrap(StageTrap *t);
void BurstTrapFast(StageTrap *t);
void SlideTrapAway(StageTrap *t, s32 arg);
void TumbleTrapAct08(StageTrap *t);
void TumbleTrapAct09(StageTrap *t);
void TumbleTrapAct0B(StageTrap *t);
void KnockTrapAwayAct0C(StageTrap *t);
void StepDashBoard(StageTrap *t);
void SpinTrapAway(StageTrap *t);
void BurstTrapStacked(StageTrap *t);
void TumbleTrapAct12(StageTrap *t);
void TipTrapAway(StageTrap *t);
void StepFlipPanel(StageTrap *t);
void BurstTrapMedium(StageTrap *t);
void StepSprayTrap(StageTrap *t, s8 dir);
void BurstTrapFastAlt(StageTrap *t);
void StepSmokingTumbleTrap(StageTrap *t);
void StepSmokingTrap(StageTrap *t);
void StepTrapAct1D(StageTrap *t);
void BurstRaisedTrap(StageTrap *t);
void StepTrapAct21(StageTrap *t);
void StepTrapAct22(StageTrap *t);
void StepTrapAct23(StageTrap *t);
void StepTrapAct24(StageTrap *t);
void StepTrapAct25(StageTrap *t, s32 arg);
void StepTrapAct27(StageTrap *t);
void StepTrapAct28(StageTrap *t);
void RunTrapThenTurn(StageTrap *t, u16 speed, s32 range, s8 dir);
void ArmTrapByRange(StageTrap *t, s32 range, u8 type, u8 act);
void UpdateStageEventHook(void);
void UpdateGoalEvents(void);
void DrawTrapWhileLinked(StageTrap *t);
void EmitTrapParticles(StageTrap *t);
void StepBouncingRoller(StageTrap *t, s32 arg);
void StepTrapFFAct65(StageTrap *t, s32 arg);
void StepTrapFFAct67(StageTrap *t);
void StepTrapFFAct68(StageTrap *t);
/* Steps a trap forward along its heading, snapped to the floor or at its
 * height. */
void StepTrapForward(StageTrap *t, u16 speed);
void StepTrapForwardLevel(StageTrap *t, u16 speed);
void RunTrapAhead(StageTrap *t, s16 speed);
void RunTrapAheadNonstop(StageTrap *t, s16 speed);
void RunTrapAlongPath(StageTrap *t, u16 speed, s8 turn);
void StepChargingTrap(StageTrap *t, s16 speed, s32 range);
void DrawTrapWarning(StageTrap *t);
void RunTrapThenTurnTimed(StageTrap *t, u16 speed, s32 arg, s8 dir);
void StepTrapType32(StageTrap *t, s16 speed, s8 mode);
void StepTrapType0F(StageTrap *t, u16 speed, s8 dir, s32 arg);
void StepTrapType12(StageTrap *t, u16 speed, s16 arg);
void StepTrapType1F(StageTrap *t, u16 arg);
void StepTrapType0D(StageTrap *t, u16 speed, s32 arg, s8 dir);
void StepTrapType1B(StageTrap *t, u16 speed, s32 arg);
void StepTrapType1D(StageTrap *t, u16 speed);
void StepTrapType30(StageTrap *t, u16 speed, s32 arg);
void StepTrapType18(StageTrap *t, s32 speed, s32 arg, s8 dir);
void StepTrapType1A(StageTrap *t, u16 speed, s32 arg, s8 dir);
void StepTrapType1C(StageTrap *t, s32 speed, s32 tiltSide, s32 unused);
void StepTrapType1E(StageTrap *t, u16 speed);
void StepTrapType20(StageTrap *t, u16 speed, s32 arg, s8 dir);
void StepTrapType2A(StageTrap *t, u16 speed, s32 arg);
void StepTrapType29(StageTrap *t, s32 startSide, s8 pauseMode);
void StepTrapType27(StageTrap *t, s32 arg);
void StepTrapType31(StageTrap *t, s32 arg);
void RunTrapAheadAtHeight(StageTrap *t, u16 speed);
void TurnTrapOnApproach(StageTrap *t, s32 speed, s8 arg);
void OpenGateOnApproach(StageTrap *t, s32 speed, s8 arg);
void SwingGateContinuously(StageTrap *t, s8 arg);
void ToppleTrapOnApproach(StageTrap *t, s32 side, s32 speed, u8 kind);
void ShiftTrapOnApproach(StageTrap *t, s8 dir, s32 speed);
void StepStage1_1Dropper(StageTrap *t);
void StepStage1_2Dropper(StageTrap *t);
void StepStage1_1CreepingDropper(StageTrap *t);
s32 StepStage2_1Dropper(StageTrap *t);
s32 StepStage4_1Dropper(StageTrap *t);
s32 StepStage5_1Dropper(StageTrap *t);
void SwapTrapModelOnApproach(StageTrap *t, s32 speed, u8 arg);
void StepHoppingTrap(StageTrap *t, u8 act);
void BurstTrapSlow(StageTrap *t);
void TumbleTrapAway(StageTrap *t, s32 height, s8 spawn);
void KnockTrapAway(StageTrap *t);
void UpdateGoalEventHook(void);
void StepGoalSequence(void);
void CycleGoalLights(s8 arg);
/* MATCHING: stage_flow's, with param0 taken whole: retail passes it
 * without narrowing. */
s32 SpawnEffectWithParams(s32 kind, s16 x, s16 y, s16 z, s16 delay, s32 param0, s16 param1,
                          s16 param2, s16 param3);
void RunTrapType00(StageTrap *t);
void RunTrapType02(StageTrap *t);
void RunTrapType04(StageTrap *t);
void RunTrapType07(StageTrap *t, s32 range, s8 dir);
void RunTrapType0A(StageTrap *t);
void RunTrapType12(StageTrap *t);
void RunTrapType13(StageTrap *t);
void RunTrapType14(StageTrap *t);
void RunTrapType17(StageTrap *t);
void RunTrapType1B(StageTrap *t);
void RunTrapType1C(StageTrap *t);
void RunTrapType1D(StageTrap *t);
void RunTrapType1E(StageTrap *t);
void RunTrapType1F(StageTrap *t);
void RunTrapType22(StageTrap *t);
void RunTrapType27(StageTrap *t);
void RunTrapType2A(StageTrap *t);
void RunTrapType2B(StageTrap *t);
void RunTrapType2E(StageTrap *t);
void RunTrapType2F(StageTrap *t);
void RunTrapType30(StageTrap *t);
void RunTrapType31(StageTrap *t);

/* MATCHING: object's views, with this unit's trap record. */
s32 LatchProximity(StageTrap *t, s32 range);
s32 SnapToFloor(StageTrap *t);

/* MATCHING: object's view, with the target passed unnarrowed. */
void InterpRange(s16 a, s32 b, u16 t, s32 n, s32 *out);

/* MATCHING: object's view, with this unit's trap record. */
s16 GetPathDirection(StageTrap *t);

/* MATCHING: stage defines it with s16 positions; this unit passes words,
 * narrowing them itself where retail does. */
s32 SpawnEffect(s32 kind, s32 x, s32 y, s32 z, s32 delay);

/* MATCHING: object's view, with this unit's trap record. */
s32 SnapToFloor(StageTrap *t);

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

extern s16 sFloorHit[];
/* MATCHING: object's view: the swing's radius is passed unextended. */
void InterpSwing(s32 r, u16 t, u16 n, s32 *out);
/* MATCHING: player's empty hook, called here with the trap. */
void UnusedHook1(StageTrap *t);

/* MATCHING: object's, with r taken whole: retail passes it unnarrowed. */
void InterpSwing(s32 r, u16 t, u16 n, s32 *out);
/* MATCHING: model's, all s32 where it has s16: the result is stored
 * without re-extension. */
s32 Lerp(s32 a, s32 b, s32 t, s32 n);

extern s16 gPlayerYaw[];

/* MATCHING: object's views, with this unit's records. */
s32 SnapToFloor(StageTrap *t);
/* MATCHING: stage defines x..delay as s16; this call passes them
 * unextended, so the view takes s32. */
s32 SpawnEffect(s32 kind, s32 x, s32 y, s32 z, s32 delay);

void ScatterTrapDust(StageTrap *t, u8 side);

/* MATCHING: player's, which no header declares. */
void ScatterDust(VECTOR *pos);

extern s8 gMissSwitchState;
extern StageTrap gMissTrap;
extern s16 D_800AC6B8;
/* MATCHING: player's, declared only where defined. */
void StopSpeed(void);
void SpawnMissDebris(void);

extern s8 gAct2GoalSwitchState;
/* MATCHING: main's, stage's and stage_overlay's, declared only where
 * defined. */
void ResetRumble(void);
void StartActResults(void);
s32 IsResultsActive(void);
u16 GetNextStage(u16 i);
void BeginAct3LoadScreen(void);
s8 StepGoalCutscene(StageTrap *t);

extern s32 *gSectionEdges;

/* MATCHING: the step's callees, which their units declare only where they
 * define them; this image links against them by address. */
s16 UpdateCamera(s16 x, s16 y, s16 z, s16 floorY);
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
void UpdateStageEvents(void);

/* MATCHING: stage's, declared only where defined. */
void StartScreenWipe(void);
s32 IsScreenWipeActive(void);

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

/* MATCHING: object's view, with this unit's trap record. */
void ResetObj34(StageTrap *t);

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

/** @brief The shape of each of the stage's own sprites, as one block. */
typedef struct {
    u8 shape[100]; /**< a shape per sprite id from SPR_STAGE on */
} SpriteShapes;

extern u8 sSpriteShapes[];

/* An address inside a function that other code calls as if it were one:
 * the shared dispatchers jump to other stages' entry points, which in this
 * overlay land mid-function. */
#if !defined(M2CTX) && !defined(PERMUTER)
#define FUNC_ALIAS(name, base, off) \
    __asm__(".globl " #name "\n.set " #name ", " #base " + " #off "\n")
#else
#define FUNC_ALIAS(name, base, off)
#endif


/** @brief One of a stage's on-screen prompts, in a table ended by a
 *         section of -1: shown when the player reaches its section. */
typedef struct {
    s16 section; /**< the course section that starts it */
    s16 limit;   /**< frames it shows before it is dismissed */
    s16 kind;    /**< which prompt it draws */
    s16 timer;   /**< frames shown so far */
    s16 unk8;    /**< not yet known; cleared with timer */
    s16 state;   /**< 0 waiting, 1 shown, 2 answered, 100 done */
} StageHint;
