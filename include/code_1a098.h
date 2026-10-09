#ifndef CODE_1A098_H
#define CODE_1A098_H

/**
 * @file code_1a098.h
 * @brief Record types code_1a098 shares with code_1dc24, the unit that
 *        follows it.
 */

#include "common.h"

/** @brief A 0x5C-byte record of a 200-entry table; only byte 0 is known. */
typedef struct {
    s8 unk0;       /**< -1 when the record is free (a guess) */
    u8 unk1[0x5B]; /**< not yet known */
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

/** @brief Points the current-block globals at the block `hdr` heads.
 *  @param hdr the header */
void func_8002D0C4(BlockHeader *hdr);

/** @brief Applies a Rec48 to the Rec78 it names.
 *  @param rec the Rec78
 *  @param r the Rec48 */
void func_8002A7D8(Rec78 *rec, Rec48 *r);

#endif
