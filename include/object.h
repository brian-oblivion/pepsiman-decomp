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
    s16 unk24;     /**< zeroed with touched */
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
