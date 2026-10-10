#ifndef CODE_1A098_H
#define CODE_1A098_H

/**
 * @file code_1a098.h
 * @brief Record types code_1a098 shares with code_1dc24, the unit that
 *        follows it.
 */

#include "common.h"
#include "libgte.h"
#include "libgpu.h"
#include "libgs.h"

/** @brief A 0x5C-byte record of a 200-entry table: a box whose corners
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
} Rec5C;

/** @brief A 0x48-byte record of a 200-entry table; a few fields known. */
typedef struct {
    u8 unk0[0x24]; /**< not yet known */
    s16 unk24;     /**< zeroed with unk40 */
    u8 unk26[2];   /**< not yet known */
    s32 unk28;     /**< set from a per-record lookup */
    u8 unk2C[8];   /**< not yet known */
    s16 unk34;     /**< -1 when reset */
    s16 unk36;     /**< -1 when reset */
    s16 unk38;     /**< -1 when fully reset */
    u8 unk3A[6];   /**< not yet known */
    u8 unk40;      /**< cleared with unk24 */
    u8 unk41;      /**< 1 when reset; only bit 0 survives a clear */
    u8 unk42[6];   /**< not yet known */
} Rec48;

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

extern GsCOORDINATE2 D_800D86E0[]; /**< coordinate systems; code_13068 points records at [0], [1] and [16] */

/** @brief Points the current-block globals at the block `hdr` heads.
 *  @param hdr the header */
void func_8002D0C4(BlockHeader *hdr);

/** @brief Updates a Rec78 entry and the Rec48 record it belongs to.
 *  @param rec the Rec78 entry
 *  @param r the Rec48 record; its byte at 0x40 collects a result bit
 *  @return nothing; the value is undefined (non-void for the match) */
s32 func_8002A7D8(Rec78 *rec, Rec48 *r);

extern SVECTOR D_800957E4; /**< a local position to transform to world */
extern VECTOR D_8009F268;  /**< the world position of that local one */
extern u8 D_800A74D0[];    /**< 128 byte flags; cleared together */

/** @brief Marks with 2 every block entry whose flag is 1.
 *  @return nothing; the value is undefined. */
s32 func_8002D16C(void);

/** @brief Resets the game state: clears the 0x30000-byte block at `recs`
 *         and frees its 200 Rec48 records, and resets the tables.
 *  @param recs the block */
void func_8002D2C0(Rec48 *recs);

#endif
