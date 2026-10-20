#ifndef OBJECT_H
#define OBJECT_H

/**
 * @file object.h
 * @brief Record types object shares with editor, the unit that
 *        follows it.
 */

#include "common.h"
#include "libgte.h"
#include "libgpu.h"
#include "libgs.h"

/** @brief The editor state's first byte after a reset: a save slot holding
 *         anything else is empty. */
#define EDITOR_MAGIC 0x38

/** @brief A StageSprite's flags byte. */
enum SpriteFlag {
    SPRITE_ACT_MASK = 0x3F, /**< the act, a SpriteAct */
    SPRITE_MIRROR = 0x40,   /**< drawn mirrored */
    SPRITE_HITTABLE = 0x80  /**< the player can hit it ("HIT." ON) */
};

/** @brief A StageSprite's act ("ACT."), the low bits of its flags. */
enum SpriteAct {
    SPRITE_ACT_NONE = 0, /**< "NO ACT" */
    SPRITE_ACT_NEAR = 1, /**< "NEAR ACT": the next sprite while the player is near */
    SPRITE_ACT_HIT = 2,  /**< "HIT ACT": the next sprite once hit, then it drifts */
    SPRITE_ACT_DRIFT = 3 /**< drifting away after a hit */
};

/** @brief A switch's state, SwitchBox.state: the traps and sprites a
 *         switch owns run while it is SWITCH_ON. */
enum SwitchState {
    SWITCH_FREE = -1, /**< no switch in this slot */
    SWITCH_ARMED = 0, /**< waiting for the player */
    SWITCH_ON = 1,    /**< the player has passed through it */
    SWITCH_OFF = 2    /**< spent: after its pair fires, or when a miss starts */
};

/** @brief A trap's type, the dispatcher StepStageTrap runs it by. Paired
 *         types share a dispatcher and differ in a direction or mode;
 *         "left" turns the heading by -90 degrees, "right" by +90. */
enum TrapType {
    TRAP_OBJECT = 0,                 /**< a standing object; its act picks how it reacts */
    TRAP_RUN_AHEAD = 1,              /**< runs straight ahead on the floor */
    TRAP_RUN_ALONG_PATH = 2,         /**< runs along the course */
    TRAP_RUN_AHEAD_BACKWARD = 3,     /**< runs straight back on the floor */
    TRAP_CHARGE = 4,                 /**< charges, recoils, stands in dust */
    TRAP_RUN_THEN_TURN_LEFT = 5,     /**< runs ahead, then turns left */
    TRAP_RUN_THEN_TURN_RIGHT = 6,    /**< runs ahead, then turns right */
    TRAP_WAIT_RUN_TURN_LEFT = 7,     /**< waits for the player, runs, turns left */
    TRAP_WAIT_RUN_TURN_RIGHT = 8,    /**< waits for the player, runs, turns right */
    TRAP_RUN_ALONG_PATH_WARNED = 9,  /**< runs along the course, with a warning */
    TRAP_CHARGE_WARNED = 10,         /**< charges, with a warning */
    TRAP_RUN_TURN_TIMED_LEFT = 11,   /**< runs ahead for a time, then turns left */
    TRAP_RUN_TURN_TIMED_RIGHT = 12,  /**< runs ahead for a time, then turns right */
    TRAP_BIKER_LEFT = 13,            /**< a biker who swerves left and skids */
    TRAP_BIKER_RIGHT = 14,           /**< a biker who swerves right and skids */
    TRAP_SEMI_TRUCK = 15,            /**< a semi truck driving straight ahead */
    TRAP_SEMI_TRUCK_ALONG_PATH = 16, /**< a semi truck driving along the course */
    TRAP_SEMI_TRUCK_WARNED = 17,     /**< a semi truck along the course, with a warning */
    TRAP_JACKKNIFING_TRUCK = 18,     /**< a semi truck that drives, then jackknifes */
    TRAP_PLANK_WORKER = 19,          /**< a worker carrying a plank, turning as the player nears */
    TRAP_GATE = 20,                  /**< a two-part gate that swings open */
    TRAP_TOPPLE = 21,                /**< falls when the player nears, in 12 frames */
    TRAP_TOPPLE_FAST = 22,           /**< falls when the player nears, in 10 frames */
    TRAP_SHIFT = 23,                 /**< moves up or down when the player nears */
    TRAP_MINE_CART = 24,             /**< a mine cart rolling a distance, then coasting */
    TRAP_MINE_CART_ALONG_PATH = 25,  /**< a mine cart rolling along the course */
    TRAP_ROLL_THEN_HOP = 26,         /**< rolls a distance, then hops to a stop; unused */
    TRAP_FALLING_CAN = 27,           /**< a giant can that drops, then rolls */
    TRAP_TIP_OVER = 28,              /**< tips a quarter turn when the player nears; unused */
    TRAP_BISON = 29,                 /**< a running bison */
    TRAP_BOULDER = 30,               /**< a bouncing boulder */
    TRAP_TUMBLEWEED = 31,            /**< a tumbleweed */
    TRAP_CRATE_ONE_WAY = 32,         /**< a crate sliding one way; unused */
    TRAP_SHUTTLING_CRATE = 33,       /**< a crate sliding back and forth */
    TRAP_STAGE_TRUCK_1 = 34,         /**< the stage's own truck (34 to 38 and 40 alike) */
    TRAP_STAGE_TRUCK_2 = 35,         /**< the stage's own truck */
    TRAP_STAGE_TRUCK_3 = 36,         /**< the stage's own truck */
    TRAP_STAGE_TRUCK_4 = 37,         /**< the stage's own truck */
    TRAP_STAGE_TRUCK_5 = 38,         /**< the stage's own truck */
    TRAP_FALLING_SIGN = 39,          /**< a sign that drops, bounces and tips flat */
    TRAP_STAGE_TRUCK_6 = 40,         /**< the stage's own truck */
    TRAP_SLIDING_DOOR = 41,          /**< one panel of a pair of sliding doors */
    TRAP_BACK_AWAY = 42,             /**< backs away a distance; unused */
    TRAP_SEMI_TRUCK_REVERSING = 43,  /**< a semi truck backing up */
    TRAP_MANHOLE_WORKER_UP = 44,     /**< a manhole worker who pops up */
    TRAP_MANHOLE_WORKER_DOWN = 45,   /**< a manhole worker who ducks back down */
    TRAP_JACKHAMMER_WORKER = 46,     /**< a worker with a jackhammer */
    TRAP_TRAIN = 47,                 /**< a train */
    TRAP_RUN_AND_SHRINK = 48,        /**< runs ahead with a warning, then shrinks away; unused */
    TRAP_SPINNING_BRUSH = 49,        /**< a spinning brush */
    TRAP_SLIDING_CRATE = 50,         /**< a crate sliding along the floor */
    TRAP_SLIDING_CRATE_SHORT = 51,   /**< a crate sliding 500 units, then stopping */
    TRAP_SPECIAL = 255               /**< a special object; its act is a SpecialAct */
};

/** @brief The act of a TRAP_SPECIAL trap. */
enum SpecialAct {
    SPECIAL_NONE = 0,                 /**< does nothing */
    SPECIAL_LINKED = 1,               /**< drawn while a linked trap is in its early states */
    SPECIAL_PARTICLES = 5,            /**< gives off particles */
    SPECIAL_CHECKPOINT_1 = 10,        /**< checkpoint 1 */
    SPECIAL_CHECKPOINT_2 = 11,        /**< checkpoint 2 */
    SPECIAL_RISING_CHECKPOINT_1 = 12, /**< checkpoint 1, rising as the player nears */
    SPECIAL_RISING_CHECKPOINT_2 = 13, /**< checkpoint 2, rising as the player nears */
    SPECIAL_BOUNCING_ROLLER = 100,    /**< a barrel or can rolling off a truck */
    SPECIAL_BOUNCING_LOG = 101,       /**< a log off the log truck, bouncing low */
    SPECIAL_BOUNCING_LOG_HIGH = 102,  /**< a log off the log truck, bouncing high */
    SPECIAL_TOSSED_LITTER = 103,      /**< litter thrown from a truck */
    SPECIAL_BOUNCING_FURNITURE = 104, /**< furniture bouncing out of a van */
    SPECIAL_FLUNG_PIECE = 105,        /**< a piece flung from the player */
    SPECIAL_MISS_DEBRIS = 106         /**< a miss's debris */
};

/** @brief A switch (gSwitches, 200 of them): a box whose corners
 *         are tested against the player or placed in the world. */
typedef struct {
    s8 state;             /**< a SwitchState */
    u8 unk1[3];           /**< not yet known */
    s16 v[4][3];          /**< local corners, around the box centre */
    s32 w[4][3];          /**< world corners */
    GsCOORDINATE2 *coord; /**< the coordinate system of the corners */
    s32 x;                /**< x of the box */
    s32 y;                /**< y */
    s32 z;                /**< z */
} SwitchBox;

/** @brief A trap, as the editor calls it (gTraps, 200 of them): a stage
 *         object with a model, a switch, a type and an act. */
typedef struct {
    u8 unk0[0x24]; /**< not yet known */
    s16 timer;     /**< zeroed with touched */
    u8 unk26[2];   /**< not yet known */
    s32 block;     /**< the block it was placed in; set from a per-record lookup */
    u8 unk2C[8];   /**< not yet known */
    s16 switchNo;  /**< the switch that triggers it; -1 when reset */
    s16 model;     /**< its ObjectModel entry; -1 when free */
    s16 hook;      /**< the stage hook of the first record of a placement, else -1 */
    u8 unk3A[6];   /**< not yet known */
    u8 touched;    /**< 1 when the player touched one of its hit points this frame */
    u8 marks;      /**< bit 0 set when reset and kept by ClearObjectHits; bit 1 LatchProximity's */
    u8 unk42[6];   /**< not yet known */
} Trap;

/** @brief The header of a stage block: its can spans follow it, and its
 *         can points start at a byte offset it gives. */
typedef struct {
    s32 spanCount; /**< how many can spans follow, gCanSpanCount */
    s32 offset;    /**< byte offset of the can points (gCanPoints) from the header */
} BlockHeader;

/** @brief CanPoint.group's bit for a can already collected. */
#define CAN_COLLECTED 0x8000

/** @brief A Pepsi can (gCanPoints), eight bytes. */
typedef struct {
    s16 x;     /**< x */
    s16 y;     /**< y */
    u16 z;     /**< z, read signed */
    s16 group; /**< its group, an index into gCanGroupOn; bit 15 once collected */
} CanPoint;

extern GsCOORDINATE2 gTodCoords[]; /**< the 20 TOD objects' coordinate systems; player points records at [0], [1] and [16] */

/** @brief Points the current-block globals at the block `hdr` heads.
 *  @param hdr the header */
void SetCurrentBlock(BlockHeader *hdr);

/** @brief Updates a ObjectModel entry and the Trap record it belongs to.
 *  @param rec the ObjectModel entry
 *  @param r the Trap record; its byte at 0x40 collects a result bit
 *  @return nothing; the value is undefined (non-void for the match) */
s32 UpdateObjectEntry(ObjectModel *rec, Trap *r);

extern SVECTOR gLocalPos; /**< a local position to transform to world */
extern VECTOR gWorldPos;  /**< the world position of that local one */
extern u8 gCanGroupOn[];  /**< each can group's revealed flag, 128 of them */

/** @brief Defined in object: hides every can group.
 *  @return nothing; the value is undefined */
s32 ClearCanGroups(void);

/** @brief Marks with 2 every block entry whose flag is 1.
 *  @return nothing; the value is undefined. */
s32 ShowRevealedCans(void);

/** @brief Resets the game state: clears the 0x30000-byte block at `recs`
 *         and frees its 200 Trap records, and resets the tables.
 *  @param recs the block */
void ResetGameState(Trap *recs);

/** @brief Defined in object.
 *  @return not yet known */
s32 TestBoxes(void);

#endif
