#ifndef PRIM_H
#define PRIM_H

/**
 * @file prim.h
 * @brief Linking a hand-built GPU packet into an ordering table.
 */

/** @brief The sprite packet's code (SPRT); libgs.h's GPU_COM_* name the
 *         polygons' codes. */
#define PRIM_SPRT 0x64

/** @brief Code bit 0: the texture is drawn as is, not shaded by the colour. */
#define PRIM_RAW 0x01

/** @brief Code bit 1: the packet is drawn semi-transparent. */
#define PRIM_SEMI_TRANS 0x02

/** @brief A DR_TPAGE command word for page 0, dithering and drawing to the
 *         display area on, as libgpu's getDrawMode(1, 1, 0); a page is
 *         OR'd in. */
#define PRIM_TPAGE_WORD 0xE1000600

/**
 * @brief Puts packet @p p, a @p type, at the head of ordering-table entry
 *        @p tag: the packet's tag word takes the entry's link and the
 *        packet's length in words after the tag, then the entry points at
 *        the packet. libgpu's setlen and addPrim in one, as two whole-word
 *        stores.
 */
#define linkPrim(tag, p, type) \
    (*(u32 *)(p) = (*(tag) & 0xFFFFFF) | (sizeof(type) / 4 - 1) << 24, *(tag) = (u32)(p) & 0xFFFFFF)

#endif
