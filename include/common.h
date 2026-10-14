#ifndef COMMON_H
#define COMMON_H

/**
 * @file common.h
 * @brief Included first by every unit: the integer types (types.h),
 *        and small C89 helper macros.
 */

#ifdef HOST_BUILD
/* The host build has no PS1 assembly: the macros that include it are empty,
 * and the PS1 linker's labels are left out. */
#define INCLUDE_ASM(FOLDER, NAME)
#define INCLUDE_RODATA(FOLDER, NAME)
#else
#include "include_asm.h"
#endif
#include "types.h"

/** The byte offset of `member` in `type`: C89's <stddef.h> offsetof, which
 * Sony's stddef.h does not provide. A constant expression. */
#ifndef offsetof
#define offsetof(type, member) ((unsigned int)&((type *)0)->member)
#endif

/** Fails to compile unless `cond`, a constant expression, holds: C89's
 * static assertion, a typedef of an array of size -1. `name` keeps the
 * typedef unique in its unit. No code or data. Used for the layouts a file
 * or the SDK fixes, which must hold on the PS1 and on every host. */
#define COMPILE_ASSERT(cond, name) typedef char static_assert_##name[(cond) ? 1 : -1]

/** The number of elements in the array `arr` (an array, never a pointer).
 * Signed, so `i < ARRAY_COUNT(a)` on an s32 compares signed, as a literal
 * would. */
#define ARRAY_COUNT(arr) ((s32)(sizeof(arr) / sizeof((arr)[0])))

/** `deg` degrees as a PlayStation angle, 4096 (libgte's ONE) to the turn.
 * Exact for multiples of 45 degrees; a constant expression. */
#define ANGLE_DEG(deg) ((deg) * 4096 / 360)

#if !defined(M2CTX) && !defined(PERMUTER) && !defined(HOST_BUILD)
/** Places a definition in small data, the `.sdata` section: the game's
 * small initialized globals, reached through one base register. Used as
 * `static s32 sName SDATA = 320;`. The definition must have an initializer,
 * and its unit's yaml range must be `.sdata`. */
#define SDATA __attribute__((section(".sdata")))
/** Places a zero-initialized definition in `.sbss`, the uninitialized half of
 * small data: `static s32 sName SBSS = 0;`. The `= 0` is required; without
 * an initializer the section attribute has no effect. */
#define SBSS __attribute__((section(".sbss")))
#else
#define SDATA
#define SBSS
#endif

/** Read-only data in the image that the game writes all the same: `const`
 * on the PS1, so that it lands in `.rodata` as in the image; writable on a
 * host build, whose `.rodata` is read-only memory. */
#ifdef HOST_BUILD
#define IMAGE_CONST
#else
#define IMAGE_CONST const
#endif

/** 20.12 fixed point: libgte's ONE is 1 << FIX12_SHIFT, so `n << FIX12_SHIFT`
 * is the integer n as a fixed-point value. */
#define FIX12_SHIFT 12

/** GteLong's type: `int` against psyz's libgte, `long` against Sony's. */
#ifdef HOST_BUILD
#define GTE_LONG_BASE int
#else
#define GTE_LONG_BASE long
#endif
/** libgte's 32-bit integer, the type of MATRIX.t[] and of VECTOR's members:
 * `long` in Sony's headers, `int` in psyz's (the host build). A pointer to
 * one of those members is a `GteLong *`. */
typedef GTE_LONG_BASE GteLong;

/** @brief The main loop's states, gGameMode. */
enum GameMode {
    GM_BOOT = 0,            /**< clears the clear colour and starts the boot load */
    GM_BOOT_LOAD = 1,       /**< LoadBootData */
    GM_SOUND_LOAD = 2,      /**< LoadSoundData */
    GM_OVERLAY0_LOAD = 3,   /**< loads overlay 0 */
    GM_OVERLAY0_WAIT = 4,   /**< waits for overlay 0 */
    GM_OVERLAY0 = 5,        /**< runs overlay 0 */
    GM_MENU_LOAD = 6,       /**< loads overlay 2, the menus; game over and quitting return here */
    GM_MENU_WAIT = 7,       /**< waits for overlay 2 */
    GM_MENU = 8,            /**< runs overlay 2 */
    GM_STAGE_LOAD = 12,     /**< loads the stage overlay */
    GM_STAGE_WAIT = 13,     /**< waits for the stage overlay */
    GM_STAGE = 14,          /**< UpdateStagePlay */
    GM_GRID_TEST_INIT = 15, /**< InitGridTest */
    GM_GRID_TEST = 16,      /**< UpdateGridTest */
    GM_MOTION_VIEWER_INIT = 17, /**< starts the motion viewer */
    GM_MOTION_VIEWER = 19,      /**< UpdateMotionViewer */
    GM_TRAP_VIEWER_LOAD = 20,   /**< loads the stage overlay for the trap viewer */
    GM_TRAP_VIEWER_WAIT = 21,   /**< waits for it */
    GM_TRAP_VIEWER_INIT = 22,   /**< InitStageOverlay */
    GM_TRAP_VIEWER = 23,        /**< UpdateTrapViewer */
    GM_TRAP_LAYOUT_LOAD = 24,   /**< loads the stage overlay for the layout viewer */
    GM_TRAP_LAYOUT_WAIT = 25,   /**< waits for it */
    GM_TRAP_LAYOUT_INIT = 26,   /**< InitStageOverlay */
    GM_TRAP_LAYOUT = 27,        /**< UpdateTrapLayoutViewer */
    GM_EDITOR_LOAD = 28,        /**< loads the stage overlay for the editor */
    GM_EDITOR_WAIT = 29,        /**< waits for it */
    GM_EDITOR_INIT = 30,        /**< InitStageOverlay */
    GM_EDITOR_RELOAD = 31,      /**< waits for a load, then the editor */
    GM_EDITOR = 32,             /**< UpdateEditor */
    GM_HIT_TOOL_LOAD = 33,      /**< loads the stage overlay for the hit-data tool */
    GM_HIT_TOOL_WAIT = 34,      /**< waits for it */
    GM_HIT_TOOL_INIT = 35,      /**< InitStageOverlay */
    GM_HIT_TOOL_RELOAD = 36,    /**< waits for a load, then the tool */
    GM_HIT_TOOL = 37,           /**< UpdateTool */
    GM_WORLD_INTRO_LOAD = 38,   /**< loads the world's intro overlay (18 + world) */
    GM_WORLD_INTRO_WAIT = 39,   /**< waits for it */
    GM_WORLD_INTRO = 40,        /**< runs it */
    GM_LOAD_SCREEN_MODEL = 41,  /**< UpdateLoadScreenModel */
    GM_LOAD_SCREEN = 42,        /**< UpdateLoadScreen */
    GM_LOAD_SCREEN_ACT3 = 43,   /**< UpdateAct3LoadScreen */
    GM_MOVIE = 44,              /**< plays the world's movie */
};

/* --- Globals shared across units ------------------------------------------
 *
 * Only what every unit can use unchanged: a global that some units reach as a
 * small object and others as an array of unknown size is declared in each
 * unit instead. Types come from the access widths and the signedness of the
 * loads; a 4-byte one is s32 until a unit shows it is a pointer.
 */

/** @brief Three words, a position. */
typedef struct {
    s32 x; /**< x */
    s32 y; /**< y */
    s32 z; /**< z */
} Vec3;

/** @brief A short-lived effect from the 128-record pool: a sprite or model
 *         drawn at a point for a few frames, then handed back. */
typedef struct {
    s16 frame;  /**< frames drawn so far; the step function ends it */
    s16 delay;  /**< frames to wait before the first one is drawn */
    s16 param0; /**< meaning depends on the kind */
    s16 param1; /**< meaning depends on the kind */
    s16 param2; /**< meaning depends on the kind */
    s16 param3; /**< meaning depends on the kind */
    s16 x;      /**< offset added to the drawing position */
    s16 y;      /**< offset added to the drawing position */
    s16 z;      /**< offset added to the drawing position */
    u8 kind;    /**< an EffectKind: its index into the step-function table */
    u8 next;    /**< the next record on the active or free list */
} Effect;

/** @brief Effect.kind, the step function gEffectSteps runs for it. The game
 *         names none; these say what each step draws. */
enum EffectKind {
    EFFECT_BURST = 0,         /**< StepBurst: the 8 SPR_BURST frames */
    EFFECT_TWINKLE = 1,       /**< StepTwinkle: a puff that swells and shrinks */
    EFFECT_SMOKE = 2,         /**< StepSmoke: a puff rising and fading */
    EFFECT_FLASH = 3,         /**< StepFlash: SPR_BURST's first frame fading */
    EFFECT_DUST = 4,          /**< StepDust: an additive blob, spreading */
    EFFECT_GLINT = 5,         /**< StepGlint: a flat screen-space bar, spreading */
    EFFECT_DARK_DUST = 6,     /**< StepDarkDust: a subtractive blob, spreading */
    EFFECT_CAN_COLLECT = 7,   /**< StepCanCollect: a can flies to the HUD, one can */
    EFFECT_FIVE_CANS = 8,     /**< StepFiveCans: five cans circle to the HUD, five cans */
    EFFECT_FLOAT = 9,         /**< StepFloat: SPR_FLOAT moving straight up or down */
    EFFECT_HURRY = 10,        /**< StepHurry: SPR_HURRY slides across */
    EFFECT_FLYING_PIECE = 11, /**< StepFlyingPiece: a model piece thrown in an arc, trailing dust */
    EFFECT_SKID_DUST = 12     /**< StepSkidDust: a blob that slides and slows */
};

extern Effect gEffects[]; /**< the 128-record pool */

/** @brief An object's model slot (gObjectModels, 80 of them): object's
 *         Model70 (GsDOBJ2, coordinate system, rotation, scale), then the
 *         object's ranges in the two hit-data banks. */
typedef struct {
    u8 obj[0x10];   /**< the GsDOBJ2 */
    u8 coord[0x5E]; /**< the GsCOORDINATE2, then the rotation and scale */
    s16 quadFirst;  /**< its first HitQuad, in the scale's pad halfword; 0 when cleared */
    s16 quadCount;  /**< its HitQuad count; -1 when cleared */
    s16 pointFirst; /**< its first HitPoint; 0 when cleared, -1 when a card's banks go in */
    s16 pointCount; /**< its HitPoint count; -1 when cleared, 0 when a card's banks go in */
    u8 unk76[2];    /**< not yet known */
} ObjectModel;

extern ObjectModel gObjectModels[]; /**< the 80 object model slots */

/** @brief PlayerState.rideMode. The low nibble (RIDE_MASK) is the ride a
 *         stage gives: each is drawn from its own mode object, and when a
 *         stage ends with one, it is left behind as a stage object. The game
 *         names none; these say what the code does. The high nibble
 *         (RIDE_ITEM_MASK) is RIDE_ITEM while ACT_STOP_ITEM's object shows. */
enum RideMode {
    RIDE_NONE = 0,        /**< on foot */
    RIDE_BOARD = 1,       /**< STAGE_1_2: a board at the feet; the legs cannot be hit */
    RIDE_ROLLER = 2,      /**< STAGE_2_1 and STAGE_4_2: an object turning with the speed, with a
                        *   rolling sound; steered by heading, a wide hit at the legs */
    RIDE_COVER = 3,       /**< STAGE_1_1 and STAGE_5_1: on the body, hiding most of it; lost on
                        *   a high fall, and hits react as on foot */
    RIDE_BOARD_2 = 4,     /**< STAGE_5_2: a second board, with its own overlay code */
    RIDE_ITEM = 5,        /**< in the high nibble: the held object is drawn */
    RIDE_MASK = 0x0F,     /**< the ride */
    RIDE_ITEM_MASK = 0xF0 /**< the held object */
};

/** @brief PlayerState.motion: the TOD motion a channel plays. The game
 *         names none; these follow the PlayerAction that asks for each. */
enum PlayerMotion {
    MOT_DEFAULT = 0x00,         /**< the motion viewer's first */
    MOT_NONE = 0x01,            /**< StepAnimation does nothing */
    MOT_RUN_SLOW = 0x02,        /**< speed band 0 */
    MOT_RUN = 0x03,             /**< speed band 1 */
    MOT_RUN_FAST = 0x04,        /**< speed band 2 */
    MOT_SLIDE_START = 0x05,     /**< the slide: its length set by the speed */
    MOT_SLIDE = 0x06,           /**< sliding, raising dust */
    MOT_SLIDE_END = 0x07,       /**< back to the run when it ends */
    MOT_JUMP = 0x08,            /**< rising while the button is held */
    MOT_JUMP_HIGH = 0x09,       /**< the fall after a full-height jump */
    MOT_JUMP_MID = 0x0A,        /**< the fall after a mid-height jump */
    MOT_JUMP_LOW = 0x0B,        /**< the fall after a short jump */
    MOT_FALL_OFF = 0x0C,        /**< far below the course after falling off */
    MOT_TRIP = 0x0D,            /**< ACT_TRIP */
    MOT_TIME_UP = 0x0E,         /**< ACT_TIME_UP: slows to a halt */
    MOT_STOP_WAIT = 0x0F,       /**< ACT_STOP_WAIT's wait */
    MOT_STOP = 0x10,            /**< slows to a halt over 10 frames */
    MOT_STOP_ITEM = 0x11,       /**< ACT_STOP_ITEM's object and sounds */
    MOT_STOP_POSE = 0x12,       /**< ACT_STOP_POSE's pose */
    MOT_STAND = 0x13,           /**< standing still, out of control */
    MOT_LAUNCH = 0x14,          /**< ACT_LAUNCH in the air */
    MOT_SPIN = 0x15,            /**< ACT_SPIN, first phase */
    MOT_SPIN_2 = 0x16,          /**< second phase */
    MOT_SPIN_3 = 0x17,          /**< third phase */
    MOT_BRAKE = 0x18,           /**< ACT_BRAKE */
    MOT_BRAKE_RISE = 0x19,      /**< ACT_BRAKE_RISE */
    MOT_DASH = 0x1A,            /**< ACT_DASH, and the landing while dashing */
    MOT_KNOCKDOWN = 0x1B,       /**< thrown up and forward */
    MOT_KNOCKDOWN_SKID = 0x1C,  /**< skids to a halt */
    MOT_KNOCKDOWN_LIE = 0x1D,   /**< lies still: up or a miss */
    MOT_DOWN = 0x1E,            /**< lies still; then a get-up chosen by the action */
    MOT_GET_UP = 0x1F,          /**< gets up, then ACT_RUN */
    MOT_BOUNCE = 0x20,          /**< two bounces on landing */
    MOT_SKID = 0x21,            /**< skids to a halt and lies still: up or a miss */
    MOT_GET_UP_2 = 0x22,        /**< gets up another way, then ACT_RUN */
    MOT_THROWN_BACK = 0x23,     /**< flies back (or, for ACT_SCREEN_HIT_FLY, at the camera) */
    MOT_THROWN_LAND = 0x24,     /**< lands from a throw */
    MOT_THROWN_LAND_END = 0x25, /**< then MOT_SKID */
    MOT_GET_UP_3 = 0x26,        /**< gets up a third way, then ACT_RUN */
    MOT_BOUNCE_BACK = 0x27,     /**< bounces back: up or a miss */
    MOT_REEL = 0x28,            /**< rocks from side to side; also the screen hit's impact */
    MOT_REEL_END = 0x29,        /**< then MOT_BOUNCE_BACK */
    MOT_FALL_LIE = 0x2A,        /**< lies after a fall: up or a miss */
    MOT_FALL_RISE = 0x2B,       /**< then MOT_GET_UP */
    MOT_HEAD_HIT = 0x2C,        /**< ACT_HEAD_HIT */
    MOT_STAGGER = 0x2D,         /**< staggers sideways, then ACT_RUN */
    MOT_FALL_SKID = 0x2E,       /**< ACT_FALL_SKID's fall */
    MOT_FALL_FLAT = 0x2F,       /**< ACT_FALL_FLAT's fall */
    MOT_TWIRL = 0x30,           /**< turning, or turning toward the camera */
    MOT_TWIRL_FALL = 0x31,      /**< falls after a twirl or a spin, then MOT_SKID */
    MOT_FLATTENED = 0x32,       /**< ACT_FLATTENED */
    MOT_RIDE = 0x33,            /**< on a ride, centred */
    MOT_RIDE_LEAN_A_IN = 0x34,  /**< leans one way */
    MOT_RIDE_LEAN_A_OUT = 0x35, /**< back from it */
    MOT_RIDE_LEAN_B_IN = 0x36,  /**< leans the other way */
    MOT_RIDE_LEAN_B_OUT = 0x37, /**< back from it */
    MOT_RIDE_JUMP = 0x38,       /**< a ride in the air */
    MOT_RIDE_JUMP_2 = 0x39,     /**< a ride in the air, second */
    MOT_OFF = 0xFF              /**< no motion; as curMotion, restarts the one asked for */
};

/** @brief PlayerState.action, what UpdatePlayerAction runs. Most come in
 *         pairs: a request (`_START`), which sets the motion and the
 *         rumble, then the state it runs in. A hit picks its action from
 *         the HitReaction table. The knock-downs end with the player lying
 *         still: up again while gBonusLevel is above 0, else a miss.
 *         Values no code here sets are set by the stage overlays. */
enum PlayerAction {
    ACT_RUN = 0x00,                      /**< running under control */
    ACT_TRIP_START = 0x07,               /**< a light hit while running */
    ACT_TRIP = 0x08,                     /**< slows down; runs on when the motion ends */
    ACT_TRIP_HARD_START = 0x09,          /**< as ACT_TRIP_START, with a stronger rumble */
    ACT_TRIP_HARD = 0x0A,                /**< as ACT_TRIP */
    ACT_TOD_FILTER_START = 0x0F,         /**< sets the motion's packet filter to 0x80 */
    ACT_TOD_FILTER = 0x10,               /**< runs with it */
    ACT_LOOK_RESET_START = 0x11,         /**< clears the look point */
    ACT_LOOK_RESET = 0x12,               /**< eases the look yaws back to 0, then ACT_RUN */
    ACT_SPIN_START = 0x13,               /**< a spinning hit */
    ACT_SPIN = 0x14,                     /**< spins and bounces for 30 frames, then up or a fall */
    ACT_STOP_START = 0x17,               /**< from a run: stops */
    ACT_STOP = 0x18,                     /**< slows to a halt and stands */
    ACT_STOP_WAIT_START = 0x19,          /**< stops, then waits */
    ACT_STOP_WAIT = 0x1A,                /**< 46 frames with object 10's other model, then on */
    ACT_STOP_ITEM_START = 0x1B,          /**< stops, then holds an object */
    ACT_STOP_ITEM = 0x1C,                /**< the object appears at frame 40, three sounds
                                      *   follow (likely the Pepsi drink) */
    ACT_STOP_POSE_START = 0x1F,          /**< stops, then a pose */
    ACT_STOP_POSE = 0x20,                /**< the pose, then ACT_RUN */
    ACT_TIME_UP_START = 0x23,            /**< the clock ran out */
    ACT_TIME_UP = 0x24,                  /**< slows to a halt over 60 frames */
    ACT_LAUNCH_START = 0x27,             /**< knocked into the air */
    ACT_LAUNCH = 0x28,                   /**< lands, skids, lies still */
    ACT_RIDE_CENTRE_START = 0x33,        /**< a ride: back from a lean */
    ACT_RIDE_CENTRE = 0x34,              /**< a ride: centred */
    ACT_RIDE_LEAN_A_START = 0x35,        /**< a ride: leans one way */
    ACT_RIDE_LEAN_A = 0x36,              /**< leaning that way */
    ACT_RIDE_LEAN_B_START = 0x37,        /**< a ride: leans the other way */
    ACT_RIDE_LEAN_B = 0x38,              /**< leaning that way */
    ACT_DASH_START = 0x39,               /**< up and the slide button at top speed */
    ACT_DASH = 0x3A,                     /**< a burst of speed for 30 frames */
    ACT_BRAKE_START = 0x3B,              /**< down and the slide button */
    ACT_BRAKE = 0x3C,                    /**< slows to a halt within 16 frames */
    ACT_BRAKE_RISE = 0x3D,               /**< gets up, then ACT_RUN */
    ACT_KNOCKDOWN_START = 0x3E,          /**< thrown up and forward */
    ACT_KNOCKDOWN = 0x3F,                /**< lands, skids, lies still */
    ACT_KNOCKDOWN_BOUNCE_START = 0x40,   /**< thrown up and forward */
    ACT_KNOCKDOWN_BOUNCE = 0x41,         /**< lands with two bounces, skids, lies still */
    ACT_THROWN_BACK_START = 0x42,        /**< thrown back: every hit on a ride */
    ACT_THROWN_BACK = 0x43,              /**< flies back, lands, skids, lies still */
    ACT_THROWN_BACK_BOUNCE_START = 0x44, /**< thrown back */
    ACT_THROWN_BACK_BOUNCE = 0x45,       /**< flies back, lands with bounces, lies still */
    ACT_REEL_START = 0x46,               /**< stopped dead */
    ACT_REEL = 0x47,                     /**< rocks from side to side, falls back, lies still */
    ACT_HEAD_HIT_START = 0x48,           /**< a hit on the head; ACT_THROWN_BACK_START with
                                      *   no bonus level left */
    ACT_HEAD_HIT = 0x49,                 /**< staggers sideways, then runs on */
    ACT_FALL_SKID_START = 0x4A,          /**< falls at speed */
    ACT_FALL_SKID = 0x4B,                /**< skids to a halt and lies still */
    ACT_FALL_FLAT_START = 0x4C,          /**< stopped dead */
    ACT_FALL_FLAT = 0x4D,                /**< drops flat and lies still */
    ACT_TWIRL_START = 0x4E,              /**< a turning hit */
    ACT_TWIRL = 0x4F,                    /**< turns five and a half times, falls, lies still */
    ACT_SCREEN_HIT_START = 0x50,         /**< a miss: one of the two below, at random */
    ACT_SCREEN_HIT_TWIRL = 0x51,         /**< turns while flying at the camera, hits the
                                      *   screen, shakes the view and slides down */
    ACT_FLATTENED_START = 0x52,          /**< a miss: crushed */
    ACT_FLATTENED = 0x53,                /**< drawn flat; MISS_RETRY after 90 frames */
    ACT_SCREEN_HIT_FLY = 0x55,           /**< flies straight at the camera and hits the screen */
    ACT_BLOWN_AWAY_START = 0x56,         /**< a miss: thrown high */
    ACT_BLOWN_AWAY = 0x57,               /**< falls, lies still, MISS_RETRY; STAGE_2_2 drops
                                      *   the player in with it and recovers */
    ACT_ROLL_STRAIGHT = 0x58,            /**< ride mode 2 with no steering */
    ACT_RIDE_LEAN_A_IN = 0x5A,           /**< holds the first lean's motion */
    ACT_RIDE_LEAN_A_OUT = 0x5C,          /**< holds its return motion */
    ACT_RIDE_STRAIGHT = 0x5E,            /**< ride mode 3 on the ground with no steering */
    ACT_MODEL_ONCE = 0x60,               /**< a demo model: plays its motion once */
    ACT_MODEL_LOOP = 0x61                /**< a demo model: loops its motion */
};

/** @brief PlayerState.slideState: what the slide button last started. */
enum SlideState {
    SLIDE_NONE = 0,    /**< running */
    SLIDE_BRAKING = 1, /**< braking, from down and the slide button */
    SLIDE_DASHING = 2  /**< dashing, from up and the button at top speed */
};

/** @brief PlayerState.jumpState. */
enum JumpState {
    JUMP_NONE = 0,    /**< on the ground, or falling: a jump can start */
    JUMP_RISING = 1,  /**< the button is held and the jump still rises */
    JUMP_RELEASED = 2 /**< for one step after the rise ends */
};

/** @brief The controller type byte, gPadType. */
enum PadType {
    PAD_TYPE_DIGITAL = 0x41, /**< a digital pad */
    PAD_TYPE_ANALOG = 0x73   /**< an analog pad in analog mode */
};

/** @brief The player's state (gPlayer), 0x3D8 bytes. Its head is the
 *         animation state StepAnimation runs: two channels, each asking
 *         for a motion (a TOD sequence) and playing one. The load screen
 *         keeps a whole copy as its turning model (gLoadScreenModel). */
typedef struct {
    u8 control;    /**< 1 while the pad steers the player; cleared by a miss or the goal */
    u8 lastSteer;  /**< the steering mode last applied; a repeat only resets yaw */
    u8 hittable;   /**< 1 when a hit can be reacted to; 0 while invulnTimer runs,
                               *   when the player is drawn every other frame pair */
    u8 twoTracks;  /**< nonzero steps both animation channels instead of one */
    u8 animFilter; /**< StepAnimation's last argument for channel 0: SetTodFrame's packet filter; 0x80 in ACT_TOD_FILTER */
    u8 action;     /**< what UpdatePlayerAction runs, a PlayerAction */
    u8 motion[2];  /**< the PlayerMotion each channel is asked to play */
    u8 curMotion[2];      /**< the PlayerMotion each channel is playing; MOT_OFF forces a restart */
    s16 motionTime[2];    /**< each channel's clock */
    u8 padE[0x18 - 0xE];  /**< not yet known */
    u8 *motionData[2];    /**< each channel's position in its sequence */
    s32 motionLen[100];   /**< each motion's length */
    s32 motionStart[100]; /**< each motion's start time */
    s8 motionResult[2];   /**< StepAnimation's result for each channel; 2 at a loop's end */
    u8 pad342[0x344 - 0x342]; /**< not yet known */
    s32 unk344;               /**< (40 - speed) / 3 + 1 */
    s32 x;                    /**< position x */
    s32 y;                    /**< position y, down positive; held at groundY on the ground */
    s32 z;                    /**< position z */
    s32 drawX;                /**< added to x for the drawn position */
    s32 drawY;                /**< added to y for the drawn position */
    s32 drawZ;                /**< added to z for the drawn position */
    s16 bodyRotX;             /**< added to the body's x rotation: ACT_REEL's rocking */
    s16 bodyYaw;              /**< added to the body's yaw: the spins and twirls */
    s16 unk364;               /**< cleared on a reset */
    u8 pad366[0x368 - 0x366]; /**< not yet known */
    s32 lookX;      /**< x of the point the models turn to face (gLookYaw*); cleared by player */
    s32 lookY;      /**< y of that point */
    s32 lookZ;      /**< z of that point; cleared by player */
    s16 actTimer;   /**< the current action's first counter, mostly its frames */
    s16 actTimer2;  /**< the current action's second counter */
    s16 actTimer3;  /**< the current action's third counter */
    s16 actTimer4;  /**< the current action's fourth counter */
    s32 startYaw;   /**< the start heading, set with yaw */
    s32 yaw;        /**< the body heading; follows the course heading in bounded steps */
    s32 lean;       /**< the body roll while steering, -15..15; decays to 0 */
    s32 lookYaw;    /**< object 2's eased turn toward the look point (gLookYawNarrow) */
    s16 unk38C;     /**< cleared whenever slideState changes */
    u8 slideState;  /**< a SlideState */
    u8 airborne;    /**< 1 from a jump's start to its landing */
    u16 jumpState;  /**< a JumpState */
    s16 jumpTime;   /**< steps the button has been held while jumpState is 1 */
    s32 jumpStartY; /**< y when the jump began */
    s16 jumpPower;  /**< the jump's strength, set by how long the button is held */
    u8 pad39A[0x39C - 0x39A]; /**< not yet known */
    s32 peakY;                /**< the jump's highest point (lowest y); 0 on landing */
    s32 jumpHeight;           /**< the jump just released: 0 low, 1 mid, 2 high */
    s16 slideTimer;           /**< frames left in MOT_SLIDE */
    s16 fallSpeed;            /**< added (times 5) to y each step in the air; reset on landing */
    s16 speed;                /**< the running speed; 40 is cruising, 50 after a reset */
    s16 startSpeed;           /**< the speed when the action began, eased down from */
    u16 speedBand;            /**< 0 below 13 in speed, 1 below 26, else 2 */
    s16 stepX;                /**< speed along the sine of the heading: this step's x move */
    s16 stepZ;                /**< speed along the cosine of the heading: this step's z move */
    u8 pad3B2[0x3B4 - 0x3B2]; /**< not yet known */
    s32 steerSum;             /**< moved by 50 a step while steering */
    s32 onRideQuad;           /**< 1 while over a ride-hit quad */
    s32 onQuadTop;            /**< 1 while standing on it */
    s32 groundY;              /**< the ground height under the player */
    s32 editorY;              /**< the editor keeps y here before snapping to the floor */
    s16 invulnTimer;          /**< counts down after a hit; the player is hittable again at 0 */
    u16 falling;   /**< 1 once CheckFall finds the player falling off: drawn into gOt, not gSubOt */
    s32 rideQuadY; /**< that quad's top */
    u8 rideMode;   /**< the ride and the held object, RideMode values */
    u8 dropRide;   /**< 1 leaves the ride behind as a stage object */
    s8 flattened;  /**< 1 draws the player with no depth (ACT_FLATTENED) */
    s8 showShadow; /**< nonzero draws the shadow under the player */
    s8 altModel;   /**< 1 while MOT_STOP_WAIT runs: object 10 draws its other model */
    s8 bonusLocked;           /**< 1 holds the bonus level at 0 and stops the clock */
    u8 pad3D6[0x3D8 - 0x3D6]; /**< not yet known */
} PlayerState;

extern PlayerState gPlayer; /**< the player */

/** @brief A 10-byte hit record, one per group of the player's collision
 *         points; collision fills it, player reacts to it. */
typedef struct {
    s8 hit;   /**< the hit flag; 0 when free */
    u8 point; /**< the nearest point of the group */
    u8 kind;  /**< what hit, a row of the HitReaction table */
    u8 unk3;  /**< from the query */
    s16 unk4; /**< from the query */
    s16 unk6; /**< from the query */
    s16 unk8; /**< from the query */
} PlayerHit;

extern PlayerHit gPlayerHits[3]; /**< one per point group */

/** @brief A stage, gStage: STAGE_<world>_<act>, counted from 1; the world
 *         is stage / 3 and the act stage % 3. */
enum Stage {
    STAGE_1_1,
    STAGE_1_2,
    STAGE_1_3,
    STAGE_2_1,
    STAGE_2_2,
    STAGE_2_3,
    STAGE_3_1,
    STAGE_3_2,
    STAGE_3_3,
    STAGE_4_1,
    STAGE_4_2,
    STAGE_4_3,
    STAGE_5_1,
    STAGE_5_2,
    STAGE_5_3
};

/** @brief A world, gStage / 3. */
enum World { WORLD_1, WORLD_2, WORLD_3, WORLD_4, WORLD_5 };

/* Small: a plain extern. */
extern s32 gTimInfo;        /**< 2 units, 4 functions */
extern u8 gClearB;          /**< the screen clear and fog colour, blue */
extern u8 gClearG;          /**< the screen clear and fog colour, green */
extern u8 gClearR;          /**< the screen clear and fog colour, red */
extern u8 gRepeatButtonsOn; /**< 3 units, 3 functions */
extern s32 gCdTrackPos;     /**< 2 units, 2 functions */
extern s32 gCdTrackPlays;   /**< 2 units, 2 functions */
extern s32 gCdTrackStart;   /**< 2 units, 2 functions */
extern s32 gLookYawNarrow; /**< object 2's turn toward the look point, eased by 8 within +-60 degrees */
extern s32 gLookYawWide; /**< the other objects' turn toward the look point, eased by 50 within +-80 degrees */
extern u16 gEditValue;      /**< the value the highlighted menu line edits */
extern s16 gMenuLine;       /**< the highlighted line of a menu */
extern u32 gStageObjCount;  /**< an entry count */
extern s32 gActiveBuff;     /**< the display buffer being drawn this frame, 0 or 1 */
extern s32 gPad;            /**< port 1's held buttons this frame (PAD* bits), after ReadPad */
extern s32 gGamePadTrig;    /**< remapped button flags; 2 units */
extern u16 gModeStep;       /**< the model viewer's state; 2 units */
extern u16 gWorldIntroStep; /**< the world intro overlay's step; main and stage_flow clear it */
extern u8 gAllTargetsBeaten; /**< 1 once every act's best time beats its target; set by the act results */
extern u8 gPlayLive; /**< 1 from a stage's start or restart until a time-up miss freezes the play */
extern u8 gFellOff;  /**< set when the player falls below the floor; never read here */
extern s16 gMissState;     /**< the play state, a MissState */
extern s32 gLoopI;         /**< a loop counter kept in a global */
extern s32 gSlotTableSize; /**< the slot table's size in bytes */
extern s32 gSlotTable;     /**< the address of the 72-byte slot records */
extern s32 gStickTrigY; /**< a pad word; its bits step a position or a highlighted line; 2 units */
extern s32 gVSyncTime;  /**< what EndFrame's VSync returned; printed first on the status panel */
extern s32 gRetries;    /**< set to 0xFF on leaving the tool; 2 units */
extern u8 gInEditor;    /**< 2 units */
extern s32 gElapsed;    /**< 2 units */
extern s32 gTimeLeft;   /**< 2 units */
extern s16 gSteerYawHalf;    /**< moved by half of gSteerYaw's steering steps */
extern s32 gGamePad;         /**< 2 units */
extern s8 gViewerMotion;     /**< a level, kept within 0..120; 2 units */
extern u8 gPadType;          /**< port 1's controller type byte, a PadType */
extern u32 gStickRawY;       /**< pad byte 7, an analogue stick axis, 0x80 at rest */
extern u32 gStickRawX;       /**< pad byte 6, the axis gStickX is read from, 0x80 at rest */
extern s16 gSelectedSection; /**< the selected ObjectModel entry; 2 units */
extern u16 gCameraYaw;       /**< 2 units */
extern s8 gButtonConfig;     /**< 2 units */
extern s8 *gStageRunObjs;    /**< the group list the zones index; 2 units */
extern s32 gCpuMarkCount;    /**< 2 units */
extern u8 gUnlocks;          /**< 2 units */
extern s32 gFloorData;       /**< the address of the floor data a probe tests; 2 units */
extern u8 gLivePoints[];     /**< a 0x44C0-byte buffer of 0x2C-byte placed records */
extern u8 gLiveQuads[];      /**< a 0x1DB0-byte buffer of 0x4C-byte records */
extern u8 gCdTrackPlaying;   /**< 1 while CD audio is playing; 2 units */
extern u16 gGameMode;        /**< the main loop's state, a GameMode */
extern s16 gCdLoadCount;     /**< files in the current CD load */
extern u8 gLoading; /**< 1 while a load runs: a fixed view and a black screen; 0 the game's view */
extern s8 gPauseState;        /**< nonzero stops the frame counter; 2 units */
extern s32 ClipF;             /**< a word inside libgte's clipf object, as a variable; 2 units */
extern s32 gModelTmd;         /**< the mapped model data's TMD address; 2 units */
extern s8 gCdLoadState;       /**< UpdateCdLoad's step, a CdLoadState */
extern u8 gStereo;            /**< nonzero for stereo: the CD mix has no cross-feed; 2 units */
extern s16 gMenuChoice;       /**< what RunMenu returned for the card menus */
extern u16 gToolCardStep;     /**< the card data screen's step, a CardStep */
extern s16 gToolSlot;         /**< 2 units, 4 functions */
extern u16 gCardNew;          /**< 1 after ProbeCard clears a new card; the tools reset it */
extern s32 gHitQuads;         /**< 2 units, 7 functions */
extern s32 gHitPoints;        /**< 2 units, 7 functions */
extern s32 gHitPointSections; /**< entry count of the first record bank; 2 units */
extern s32 gHitQuadSections;  /**< entry count of the second record bank; 2 units */
extern u16 gVabLoadStep;      /**< 2 units, 3 functions */
extern u8 gStage;             /**< the stage being played, a Stage */
extern s32 gStageParams;      /**< 2-bit stage settings, read at bits 4, 8, 12, 20 and 22 */
extern u8 gEffectActive;      /**< 2 units, 2 functions */
extern u8 gEffectFree;        /**< 2 units, 2 functions */
extern u16 gCans;             /**< 5 units, 7 functions */
extern s16 gEditMax;          /**< limit of the value being edited */
extern s16 gMenuLineCount;    /**< the count a menu line wraps at */
extern s32 gPadTrig;  /**< port 1's newly pressed buttons this frame, auto-repeat included */
extern u8 *gCanSpans; /**< each stage block's cans: first point and count (after the BlockHeader) */
extern u8 *gCanPoints;    /**< the cans: x, y, z and group, eight bytes each */
extern s32 gCanSpanCount; /**< how many blocks have a span (the BlockHeader's first word) */
extern s16 gSoundFade;    /**< the fade running, a SoundFade */
extern s32 gBonusLevel;   /**< 2 units; 3 when a stage starts */
extern s16 gSteerYaw;  /**< the heading steering builds; held within 0x100 of the course heading */
extern u8 *gFogColors; /**< 2 units; far (fog) colours, four bytes each */
extern u16 gFogFade;   /**< 2 units; fog fade-in step: 0..31, then 100 */
extern s32 gVSyncMode; /**< EndFrame's VSync argument: the frames to wait */
extern s32 gStageTableSet; /**< 1 picks the second set of stage tables; -1 at boot */
extern s8 gHitRecover; /**< set by a hit reaction; the next ResetPlayerAction gives 60 frames of invulnerability */
extern u16 gSpeedFixed;   /**< 2 units */
extern u32 gFrameCount;   /**< frames drawn while not paused */
extern s32 gLoopJ;        /**< a second loop counter, or a count, kept in a global */
extern s16 gSectionCount; /**< number of entries; 2 units */
extern s16 gViewerAngle;  /**< the view's orbit angle, in degrees; 2 units */
extern s32 gSection;      /**< stamped on placed records; 2 units */
extern u16 gSubStep;      /**< a state switched on in main; cleared after placing; 2 units */
extern s16 gToolMode;     /**< the editor's EditorMode or the hit tool's ToolMode */
extern s32 gFloorIndex; /**< the floor under the player, from FindFloor; the hint FindFloorNear starts from */
extern s32 gTmdSortMode; /**< SetTmdSortMode's argument for the models */

/* Per-stage tables, three words per stage; a global flag picks the set. */
extern s32 gTimeLimits[][3];    /**< 2 units */
extern s32 gTimeLimitsAlt[][3]; /**< the second set; 2 units */

/* Three positions per stage that stage_flow defines and only the stage
 * overlays read: where their scripted sequences move the player. */
extern Vec3 D_8007AD68[]; /**< after a miss */
extern Vec3 D_8007AE1C[]; /**< at the goal, first */
extern Vec3 D_8007AED0[]; /**< at the goal, then */

/* Arrays of unknown size, each reached only at its first element: most are
 * probably members of larger structures, still to be found. */
extern u16 gRumblePatterns[]; /**< 3 units, 6 functions */
extern s32 gCdLoadLoc[];      /**< each file's CdlLOC in the current CD load */
extern s32 gView[];           /**< 4 units, 4 functions */
extern u16 gBodyRot[]; /**< the player model's rotation (an SVECTOR: 0, yaw, lean) for gTodCoords[0] */
extern s16 gBodyYaw[];       /**< gBodyRot[1], read signed */
extern s32 gCdLoadDest[];    /**< each file's load address in the current CD load */
extern s32 gCdLoadSectors[]; /**< each file's length in sectors, less one */
extern s32 gWorldShift[];    /**< 5 units, 22 functions */
extern s32 gTraps[];         /**< the 200 traps (Trap records; units view them their own way) */
extern s16 gFloorY[];        /**< 2 units, 6 functions */
extern s32 gOffsetVec[];     /**< PolarOffset's result, x y z */
extern s32 gTodObjTable[];   /**< 2 units, 5 functions */
extern s32 gSavedStart[]; /**< the state a stage can resume from (stage_flow's SavedStart); [0] is its valid flag */
extern s32 gRefView[];    /**< the game's GsRVIEW2, as words */
extern s32 gFlatLights[]; /**< 3 units, 4 functions */

extern s32 *gAnimations[]; /**< per-sequence data pointers; 2 units */

/* The debug lines' formats collision defines, which the stage editor and
 * the object view print. */
extern const char gTrapNoFmt[];   /**< "TRAP NO  (%2d / %2d)" */
extern const char gSyncFmt[];     /**< "SYNC %3d" */
extern const char gGridFmt[];     /**< " GRID %4d" */
extern const char gNowBlockFmt[]; /**< " NOW BLOCK (%2d/%2d)" */

/* --- Functions called from more than one unit, with the same prototype ---- */

/** @brief Defined in sound.
 *  @param id what to start, by number
 *  @return -1 when a sound bank failed to open; otherwise nothing is
 *          returned (callers ignore the result) */
s32 PlaySe(s32 id);

/** @brief PlaySe's ids, the sound effects. The game names none; these say
 *         where each is played. Ids below 70 are in the common VAB bank;
 *         from 70 on, sSeVabBanks picks a stage's own. */
enum SoundEffect {
    SE_STEP = 0x13,  /**< every 8 frames of MOT_RUN_SLOW */
    SE_SLIDE = 0x16, /**< a slide or a brake starts */
    SE_JUMP = 0x17,  /**< leaving the ground */
    SE_WIPE = 0x19,  /**< StartScreenWipe */
    SE_SPIN = 0x1A,  /**< the spin's first frame */
    SE_FALL = 0x1B,  /**< falling off the course (MISS_FELL) */
    SE_LAND = 0x1E,  /**< LandPlayer: down on the ground or a ride quad */
    SE_POWER_UP = 0x22, /**< a retry won on the results screen, or every tenth can raising the bonus level */
    SE_NEW_RECORD = 0x25, /**< the results screen beats a best can count or time */
    SE_HURRY = 0x28,      /**< 11 seconds left */
    SE_TALLY = 0x2A,      /**< every tenth can counted on the results screen */
    SE_ITEM_TICK = 0x2B,  /**< ACT_STOP_ITEM's three repeated sounds */
    SE_ALL_CANS = 0x2F,   /**< the results screen counts all 100 cans */
    SE_ITEM_1 = 0x30,     /**< ACT_STOP_ITEM, frame 13 */
    SE_ITEM_2 = 0x31,     /**< ACT_STOP_ITEM, frame 20 */
    SE_MISS = 0x34,       /**< lying down with no bonus level left: the miss */
    SE_CAN = 0x35,        /**< a can collected */
    SE_LAUNCH = 0x3C,     /**< ACT_LAUNCH starts */
    SE_GET_UP = 0x50,     /**< getting up after a fall */
    SE_DASH = 0x71,       /**< the dash starts */
    SE_ROLLER = 0x81      /**< every 20 frames of STAGE_2_1's roller */
};

/** @brief StartRumble's patterns: each a single RumbleStep of
 *         gRumblePatterns. The game names none; these follow the action
 *         that starts each (several share one). */
enum RumblePattern {
    RUMBLE_NONE = 0,       /**< all zero: stops the motors */
    RUMBLE_TRIP = 1,       /**< large motor 170, 10 frames */
    RUMBLE_KNOCKDOWN = 2,  /**< both motors, 10 frames: a hard trip or a knockdown */
    RUMBLE_BOUNCE = 3,     /**< large 200, three 5-frame pulses: a bounce */
    RUMBLE_HIT = 4,        /**< both motors, 10 frames: thrown back, a head hit, a fall */
    RUMBLE_REEL = 5,       /**< both motors, four 5-frame pulses */
    RUMBLE_HARD_LAND = 6,  /**< large 255, 10 frames: LandPlayer, or the skid after a throw */
    RUMBLE_SPIN = 7,       /**< large 255, three times */
    RUMBLE_TWIRL = 8,      /**< large 130, four 5-frame pulses */
    RUMBLE_FLATTENED = 9,  /**< both motors, weak, 15 frames */
    RUMBLE_DASH = 10,      /**< both motors, weak, 30 frames */
    RUMBLE_SKID = 11,      /**< the small motor alone, 5 frames */
    RUMBLE_BRAKE = 12,     /**< both motors, four 5-frame pulses */
    RUMBLE_JUMP_LAND = 13, /**< large 130, 10 frames: landing from a jump */
    RUMBLE_SLIDE = 14      /**< both motors, faint, 20 frames */
};

/** @brief Defined in sound; called from main. */
void InitSound(void);

/** @brief Defined in model; called from main. */
void InitView(void);

/** @brief Defined in model: updates an index from a position, then
 *         checks the position against the data blob.
 *  @param pos a position, three words
 *  @param arg a result buffer for the check
 *  @param cur the current index
 *  @return the updated index, or -1 when the check fails */
s32 FindFloorNear(void *pos, void *arg, s32 cur);

/** @brief Defined in model; called when a stage is reset. */
void StopCdTrack(void);

/** @brief Defined in main; starts a CD-DA track.
 *  @param track the track number
 *  @param loop  nonzero to loop it */
void PlayCdTrack(s32 track, s32 loop);

/** @brief Defined in main; steps a model's animation (the title's player).
 *  @param obj the model's state */
void StepStepper(PlayerState *obj);

/** @brief Defined in main; draws a model (the title's player).
 *  @param obj the model's state */
void DrawStepper(PlayerState *obj);

/** @brief Defined in stage; starts the screen wipe and its sound. */
void StartScreenWipe(void);

/** @brief Defined in stage; whether the screen wipe runs.
 *  @return nonzero while it runs */
s32 IsScreenWipeActive(void);

/** @brief Defined in model; called from main: sets up the three flat
 *         lights, the ambient light and the fog. */
void InitLights(void);

/** @brief Defined in main.
 *  @param count how many times it prints its fixed string */
void PrintTabs(s16 count);

/** @brief Defined in main: freezes the screen into two sprites and starts
 *         the load screen, with a copy of the player running on it. */
void BeginLoadScreenModel(void);

/** @brief Defined in main: starts the load screen for the next stage. */
void BeginLoadScreen(void);

/** @brief Defined in main: freezes the screen into two sprites and starts
 *         the third act's load screen. */
void BeginAct3LoadScreen(void);

/** @brief Defined in player: on every tenth pickup, plays a cue
 *         (unless the stage forbids it) and updates the pickup display. */
void CheckCanBonus(void);

/** @brief Defined in stage; loads a run of TIM images (each starting
 *         with the word 0x10) into VRAM.
 *  @param p the first TIM */
void LoadTimList(unsigned long *p);

/** @brief Defined in stage (still asm); called from main. */
void DefineHudSprites(void);

/** @brief Defined in stage: queues the draw-mode and clipped
 *         draw-environment packets that split the screen.
 *  @param clip nonzero to queue them; zero does nothing */
void SetLetterbox(s32 clip);

/** @brief Defined in model; issues one CD command.
 *  @param com the CdlXXX command
 *  @return always 0 */
s32 SendCdTrackCommand(u8 com);

/** @brief Defined in model; the CD ready callback main installs.
 *  @param mode the interrupt status */
void OnCdTrackReady(u8 mode);

/** @brief Defined in draw: fills one 8-byte record of a sprite table.
 *  @param id    which record
 *  @param mode  stored at bit 7 of the record's first halfword
 *  @param w     stored in byte 6
 *  @param h     stored in byte 7
 *  @param page  the low byte of the first halfword (read as a byte)
 *  @param u     stored in byte 4
 *  @param v     stored in byte 5
 *  @param clutX its top 12 bits form the low bits of the second halfword
 *  @param clutY stored from bit 6 of the second halfword */
void DefineSprite(u16 id, u8 mode, s32 w, s32 h, s32 page, s32 u, s32 v, u16 clutX, s32 clutY);

/** @brief Sprite ids, the records DefineSprite fills and the Draw* calls
 *         draw. The game names none; these say where each is drawn. A
 *         run of frames or glyphs is named by its first id. */
enum SpriteId {
    SPR_STAGE = 0x1E,  /**< the first of a stage's own sprites (LoadSpritePack); act 3
                                  *   puts four 255x64 strips here */
    SPR_CAN = 0xFA,    /**< the can: the HUD icon, the billboard, the collected can */
    SPR_RUNNER = 0xFB, /**< 10 frames of a running figure: the bonus level, the loading screen */
    SPR_DIGIT = 0x105, /**< 10 16x16 digits: the cans and retries counts */
    SPR_SMALL_DIGIT = 0x10F,     /**< 10 8x8 digits: the HUD time */
    SPR_SMALL_COLON = 0x119,     /**< the HUD time's separator */
    SPR_BURST = 0x11A,           /**< 8 frames of a burst; frame 5 is also a puff of its own */
    SPR_HUD_TIME = 0x122,        /**< the HUD time's label */
    SPR_BIG_DIGIT = 0x123,       /**< 10 32x32 digits: the seconds left */
    SPR_BLOB = 0x12D,            /**< a soft round blob: the player's shadow, dust */
    SPR_PROGRESS_BAR = 0x12E,    /**< the course progress bar */
    SPR_PROGRESS_MARK = 0x12F,   /**< 4 frames of the marker on it */
    SPR_UNDRAWN = 0x133,         /**< 88x40, defined but not drawn by this executable */
    SPR_FLOAT = 0x134,           /**< StepFloat's billboard, drifting up or down */
    SPR_LIVES = 0x135,           /**< the icon beside the retries count */
    SPR_HURRY = 0x136,           /**< the banner that slides across with 11 seconds left */
    SPR_RESULT_DIGIT = 0x137,    /**< 10 results-screen digits */
    SPR_RESULT_COLON = 0x141,    /**< the results time's separator */
    SPR_RESULT_SLASH = 0x142,    /**< the slash of "n/100" */
    SPR_RESULT_SHADOW = 0x143,   /**< the 12 glyphs above again, drawn as their shadow */
    SPR_ACT_LABEL = 0x14F,       /**< one label per act, by rank % 3 */
    SPR_RESULT_BANNER = 0x151,   /**< the act results' banner, zoomed in */
    SPR_WORLD_LABEL = 0x152,     /**< the world results' title */
    SPR_WORLD_DIGIT = 0x153,     /**< 4 world numbers, by rank / 3 */
    SPR_CANS_LABEL = 0x157,      /**< results: the cans row */
    SPR_BEST_CANS_LABEL = 0x158, /**< results: the best cans row */
    SPR_TIME_LABEL = 0x159,      /**< results: the time row */
    SPR_BEST_TIME_LABEL = 0x15A, /**< results: the best time row */
    SPR_ALL_CANS = 0x15B,        /**< slides in when all 100 cans are counted */
    SPR_NEW_RECORD = 0x15C,      /**< slides in for a new best */
    SPR_MARKER = 0x15D,          /**< the tools' point marker billboard */
    SPR_POINTER = 0x15E,         /**< the hit tool's bobbing pointer */
    SPR_PAUSE = 0x15F,           /**< the pause screen's caption */
    SPR_SCREEN_LEFT = 0x1FE,     /**< the left half of a frozen frame in VRAM */
    SPR_SCREEN_RIGHT = 0x1FF     /**< its right half */
};

/** @brief Defined in editor.
 *  @return 0. */
s32 StepMenuCursor(void);

/** @brief Defined in stage: remaps three buttons of the two button
 *         words by a row of a remap table. */
void RemapButtons(void);

/** @brief Defined in collision. */
void ResetCollision(void);

/** @brief Defined in collision. */
void LeaveQuadTop(void);

/** @brief Defined in hit_tool; called from card. */
void ApplyHitPoints(void);

/** @brief Defined in hit_tool; called from card. */
void ApplyHitQuads(void);

/** @brief Defined in player; called from stage_flow. Resets the
 *         stepped value two mode handlers steer, to 40 whole units. */
void ResetSpeed(void);

/** @brief Defined in editor. */
void PrintYesNo(void);

/** @brief Defined in editor. */
void PrintSaveLoad(void);

/** @brief Defined in editor: wraps the highlighted line at `n` lines
 *         and runs an update.
 *  @param n the number of lines
 *  @return the highlighted line when flag bit 5 is set, else -1. */
s32 RunMenu(s16 n);

/** @brief UpdateCdLoad's steps, gCdLoadState: each file of the load is
 *         sought, then read; an error goes back to the seek. */
enum CdLoadState {
    CD_LOAD_IDLE = 0,      /**< nothing to do */
    CD_LOAD_SEEK = 1,      /**< sets the file's position; a load starts here */
    CD_LOAD_SEEK_WAIT = 2, /**< waits for the seek */
    CD_LOAD_READ = 3,      /**< starts the read */
    CD_LOAD_READ_WAIT = 4, /**< waits for the read */
    CD_LOAD_NEXT = 5,      /**< the next file, or CD_LOAD_DONE */
    CD_LOAD_DONE = 6       /**< every file has been read */
};

/** @brief The play state, gMissState: playing, or a miss and its retry.
 *         -2 and 110 are not set by this executable. */
enum MissState {
    MISS_HALT = -2,     /**< nothing runs, not even the HUD */
    MISS_DYING = -1,    /**< a miss animation plays: the HUD, but no clock */
    MISS_NONE = 0,      /**< playing: the clock runs; at zero, MISS_TIME_UP */
    MISS_TIME_UP = 1,   /**< out of time: a frozen frame, then MISS_RETRY */
    MISS_FELL = 2,      /**< fell off the course: stops the music */
    MISS_FELL_WAIT = 3, /**< waits 60 frames, then MISS_RETRY */
    MISS_RETRY = 100,   /**< restarts the stage, or the menus with no retries left */
    MISS_TO_MENU = 110  /**< 0x180 frames, then the menus */
};

/** @brief A memory card operation, RunCardOp1's and RunCardOp2's `mode`. */
enum CardOp {
    CARD_OP_PROBE = -1,  /**< probes the card and stops */
    CARD_OP_READ = 0,    /**< reads the save file */
    CARD_OP_REWRITE = 1, /**< rewrites the save file in place */
    CARD_OP_CREATE = 2,  /**< creates the save file and writes it */
    CARD_OP_FORMAT = 3,  /**< formats the card */
    CARD_OP_ERASE = 4,   /**< erases the save file */
    CARD_OP_CHECK = 5    /**< tests for the file, then for room for it */
};

/** @brief What RunCardOp1 and RunCardOp2 return. */
enum CardResult {
    CARD_PROBE_FAILED = -2, /**< the card answered with an error 16 times */
    CARD_FAILED = -1,       /**< a file step failed; gCardError says which */
    CARD_OK = 0,            /**< done */
    CARD_FULL = 1,          /**< no room for the save file */
    CARD_ABSENT = 2,        /**< no card answered */
    CARD_NO_FILE = 3,       /**< the card has no save file */
    CARD_UNFORMATTED = 4    /**< a new card, not formatted */
};

/** @brief The card data screen's steps, gToolCardStep: the editor's and
 *         the hit-data tool's share them. */
enum CardStep {
    CS_CHECK = 0,        /**< reads the card */
    CS_MODE_SELECT = 1,  /**< SAVE or LOAD */
    CS_SLOT_SELECT = 2,  /**< one of the three slots */
    CS_SAVE_CHECK = 3,   /**< checks the slot before writing it */
    CS_SAVE_ASK = 4,     /**< "OVER WRITE OK ?" */
    CS_LOAD_CHECK = 5,   /**< checks the slot before loading it */
    CS_LOAD_ASK = 6,     /**< "LOAD OK ?" */
    CS_NO_DATA = 7,      /**< "CAN NOT OPEN DATA" */
    CS_WRITE = 8,        /**< stores the slot and rewrites the file */
    CS_LOAD = 9,         /**< restores the slot */
    CS_COMPLETE = 10,    /**< "COMPLATE" */
    CS_ERROR = 11,       /**< "ERROR" */
    CS_NO_MEMORY = 12,   /**< "NOT ENOUGH MEMORY"; no way out */
    CS_NO_CARD = 13,     /**< "NO CONNECT CARD" */
    CS_NO_FILE = 14,     /**< "NO FILE", "CREATE NEW FILE ?" */
    CS_UNFORMATTED = 15, /**< "UN FORMAT", "FORMAT OK ?" */
    CS_CREATE = 16,      /**< creates the save file */
    CS_FORMAT = 17,      /**< formats the card */
    CS_EXIT = 18         /**< leaves the data screen */
};

/** @brief Defined in card. */
void OpenCardEvents(void);

/** @brief Defined in card. */
void CloseCardEvents(void);

/** @brief Defined in card: runs one memory card operation on the
 *         second save file.
 *  @param mode a CardOp
 *  @return a CardResult */
s32 RunCardOp2(s16 mode);

/** @brief Defined in collision: tests a 0x4C-byte record.
 *  @param rec the record */
void PlaceCollisionQuad(void *rec);

/** @brief Defined in draw_tmd: selects one of 23 data sets by `id`
 *         and points three globals at it.
 *  @param id the set, 0 to 22 (read as a byte) */
void LoadOverlay(s32 id);

/** @brief Defined in stage_flow: one frame of the stage-start sequence.
 *  @param stage the stage whose overlay runs the frame */
void UpdateStagePlay(u8 stage);

/** @brief Defined in stage_overlay: reads a stage's halfword from a table.
 *  @param stage the stage index
 *  @return the table entry */
s32 GetStageVabBank(u16 stage);

/** @brief Defined in stage_overlay. */
void ResetStageOverlay(void);

/** @brief Defined in main.
 *  @param arg not yet known */
void ReadPad(s32 arg);

/** @brief Defined in stage. */
void DrawFrozenFrame(void);

/** @brief Defined in stage_flow. */
void FadeInFog(void);

/** @brief What UpdateSoundFade fades out over 50 frames, gSoundFade. */
enum SoundFade {
    FADE_NONE = 0, /**< no fade: resets the volumes */
    FADE_BGM = 1,  /**< the BGM sequence, then stops it */
    FADE_ALL = 3,  /**< the BGM and the sound effects; falls into FADE_CD */
    FADE_CD = 4    /**< the CD audio, then stops the track */
};

/** @brief Starts a fade that UpdateSoundFade runs over 50 frames.
 *  @param mode a SoundFade */
void SetSoundFade(u8 mode);

/** @brief Defined in player. */
void EndJump(void);

/** @brief Defined in model. */
void UpdateMotionViewer(void);

/** @brief Defined in draw. */
void InitTmdSortFuncs(void);

/** @brief Defined in player. */
void InitGridTest(void);

/** @brief Defined in player. */
void UpdateGridTest(void);

/** @brief Defined in object. */
void UpdateTrapViewer(void);

/** @brief Defined in object. */
void InitEntryModels(void);

/** @brief Defined in hit_tool. */
void UpdateTool(void);

/** @brief Defined in editor.
 *  @return not yet known */
s32 UpdateEditor(void);

/** @brief Defined in stage_flow: a stage's number.
 *  @param stage the stage
 *  @return not yet known */
s16 InitStageOverlay(u8 stage);

/** @brief Defined in stage_overlay.
 *  @param sel not yet known */
void LoadStageOverlay(u16 sel);


/** @brief Defined in stage. */
void DrawScreenWipe(void);

/** @brief Defined in stage. */
void UpdateActResults(void);

/** @brief Defined in player.
 *  @return not yet known */
s32 UpdatePlayState(void);

/** @brief Defined in main.
 *  @return not yet known */
s32 UpdateRumble(void);

/** @brief Defined in main.
 *  @param offset not yet known
 *  @param a      not yet known
 *  @param b      not yet known */
void SetPadVibration(s32 offset, u8 a, u8 b);

/** @brief Defined in sound: sets a volume.
 *  @param vol the volume */
void SetCdVolume(s32 vol);

#endif
