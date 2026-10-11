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

/** @brief A switch (gSwitches, 200 of them): a box whose corners
 *         are tested against the player or placed in the world. */
typedef struct {
    s8 unk0;              /**< -1 when free (a guess); 1 once the box is hit */
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
    s16 unk24;     /**< zeroed with unk40 */
    u8 unk26[2];   /**< not yet known */
    s32 block;     /**< the block it was placed in; set from a per-record lookup */
    u8 unk2C[8];   /**< not yet known */
    s16 switchNo;  /**< the switch that triggers it; -1 when reset */
    s16 model;     /**< its ObjectModel entry; -1 when free */
    s16 hook;      /**< the stage hook of the first record of a placement, else -1 */
    u8 unk3A[6];   /**< not yet known */
    u8 unk40;      /**< cleared with unk24 */
    u8 unk41;      /**< 1 when reset; only bit 0 survives a clear */
    u8 unk42[6];   /**< not yet known */
} Trap;

/** @brief The header of a block whose second part starts at a byte offset
 *         the header gives. */
typedef struct {
    s32 unk0;   /**< not yet known; kept in a global */
    s32 offset; /**< byte offset of the second part from the header */
} BlockHeader;

/** @brief An eight-byte entry of the BlockHeader's first part. */
typedef struct {
    s16 unk0; /**< not yet known */
    s16 unk2; /**< not yet known */
    u16 unk4; /**< summed over the entries */
    s16 unk6; /**< an index into the 128 byte flags */
} Ent8;

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
