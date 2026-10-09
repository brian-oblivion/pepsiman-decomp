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

/* --- Globals shared across units ------------------------------------------
 *
 * Only what every unit can use unchanged: a global that some units reach as a
 * small object and others as an array of unknown size is declared in each
 * unit instead. Types come from the access widths and the signedness of the
 * loads; a 4-byte one is s32 until a unit shows it is a pointer.
 */

/** @brief A record of a 128-entry table. Only one byte is known: the
 *         record's 1-based number, written at start-up. */
typedef struct {
    u8 unk0[0x13]; /**< not yet known */
    u8 unk13;      /**< set to the record's index + 1 */
} NumberedSlot;

extern NumberedSlot D_800DFAB0[]; /**< the 128 records; 2 units */

/** @brief A 0x78-byte record of an 80-entry table; two pairs of halfwords
 *         are reset together. */
typedef struct {
    u8 unk0[0x10];  /**< not yet known */
    u8 unk10[0x5E]; /**< handed to the dispatch's object on entering state 1 */
    s16 unk6E;      /**< zeroed when the second buffer is cleared */
    s16 unk70;      /**< -1 when the second buffer is cleared */
    s16 unk72;      /**< zeroed when the first buffer is cleared, -1 when banks are installed */
    s16 unk74;      /**< -1 when the first buffer is cleared, zeroed when banks are installed */
    u8 unk76[2];    /**< not yet known */
} Rec78;

extern Rec78 D_800D8D20[]; /**< the 80 records; 2 units */

/* Small: a plain extern. */
extern s32 D_800956D4; /**< 2 units, 4 functions */
extern u8 D_8009574C;  /**< fog colour, red; 2 units */
extern u8 D_80095754;  /**< fog colour, green; 2 units */
extern u8 D_8009575C;  /**< fog colour, blue; 2 units */
extern u8 D_800956F7;  /**< 3 units, 3 functions */
extern s32 D_80095714; /**< 2 units, 2 functions */
extern s32 D_8009571C; /**< 2 units, 2 functions */
extern s32 D_80095720; /**< 2 units, 2 functions */
extern u16 D_80095748; /**< 2 units, 2 functions */
extern s16 D_8009574A; /**< the highlighted line of a three-line menu */
extern u32 D_80095794; /**< an entry count */
extern s32 D_80095750; /**< 2 units, 3 functions */
extern u16 D_80095768; /**< 2 units, 2 functions; never loaded, sign unknown */
extern u8 D_800957D5;  /**< 2 units, 2 functions; never loaded, sign unknown */
extern u8 D_800957D6;  /**< 2 units, 3 functions; never loaded, sign unknown */
extern u8 D_800958EC;  /**< 2 units, 4 functions; never loaded, sign unknown */
extern s32 D_800958CC; /**< 2 units, 2 functions; a loop counter kept in a global */
extern s32 D_800959B4; /**< 2 units, 2 functions */
extern s32 D_800959B8; /**< 2 units, 2 functions */
extern s16 D_800959E0; /**< 2 units, 2 functions */
extern u16 D_800959E4; /**< 2 units, 4 functions */
extern s16 D_80095A0C; /**< 2 units, 4 functions */
extern u16 D_80095A14; /**< 3 units, 3 functions; never loaded, sign unknown */
extern s32 D_80095A4C; /**< 2 units, 7 functions */
extern s32 D_80095A50; /**< 2 units, 7 functions */
extern s32 D_80095780; /**< entry count of the first record bank; 2 units */
extern s32 D_80095810; /**< entry count of the second record bank; 2 units */
extern u16 D_80095B0E; /**< 2 units, 3 functions */
extern u8 D_80095830;  /**< 8 units, 24 functions */
extern s32 D_80095864; /**< 3 units, 5 functions */
extern u8 D_80095AA8;  /**< 2 units, 2 functions */
extern u8 D_80095AA9;  /**< 2 units, 2 functions */
extern u16 D_800958E8; /**< 5 units, 7 functions */
extern s16 D_800958B0; /**< limit of the value being edited */
extern s16 D_800958B2; /**< the count a menu line wraps at */
extern s32 D_80095970; /**< flag word; bit 5 enables a two-state dispatch */
extern u8 *D_800959C0; /**< the bytes after a BlockHeader */
extern u8 *D_800959C4; /**< the BlockHeader's second part */
extern s32 D_800959C8; /**< the BlockHeader's first word */
extern s16 D_80095AF0; /**< 2 units, 2 functions */
extern u32 D_8009585C; /**< a frame counter; 2 units */

/* Arrays of unknown size, each reached only at its first element: most are
 * probably members of larger structures, still to be found. */
extern u16 D_800734AC[]; /**< 3 units, 6 functions */
extern s32 D_80096748[]; /**< 3 units, 7 functions */
extern s32 D_80096768[]; /**< 4 units, 4 functions */
extern s32 D_8009676C[]; /**< 2 units, 3 functions */
extern u16 D_8009EAB8[]; /**< 2 units, 3 functions; never loaded, sign unknown */
extern s16 D_8009EABA[]; /**< 2 units, 2 functions */
extern u8 D_8009EB78[];  /**< 8 units, 69 functions */
extern u8 D_8009EB7E[];  /**< 2 units, 2 functions */
extern u8 D_8009EEC0[];  /**< an object with a position; passed to
                          *   distance tests */
extern s16 D_8009EF20[]; /**< 2 units, 6 functions */
extern u8 D_8009EF48[];  /**< 4 units, 7 functions */
extern s8 D_8009EF4A[];  /**< 2 units, 2 functions */
extern s32 D_8009F090[]; /**< 3 units, 7 functions */
extern u8 D_8009F0B0[];  /**< 2 units, 4 functions */
extern s32 D_8009F248[]; /**< 3 units, 7 functions */
extern s32 D_800A7308[]; /**< 5 units, 22 functions */
extern s32 D_800A9008[]; /**< 3 units, 17 functions */
extern s16 D_800AC858[]; /**< 2 units, 6 functions */
extern s32 D_800D39C8[]; /**< 2 units, 2 functions */
extern s32 D_800D8360[]; /**< 2 units, 5 functions */
extern s32 D_800D86B8[]; /**< 2 units, 3 functions */
extern s32 D_800DB2A0[]; /**< 5 units, 15 functions */
extern s32 D_800DD070[]; /**< 3 units, 4 functions */

/* --- Functions called from more than one unit, with the same prototype ---- */

/** @brief Defined in code_31cec.
 *  @param id what to start, by number */
void func_80042538(s32 id);

/** @brief Defined in code_31cec; called from main. */
void func_800428B0(void);

/** @brief Defined in code_7d74; called from main. */
void func_80018CB4(void);

/** @brief Defined in code_7d74; called from main: sets up the three flat
 *         lights, the ambient light and the fog. */
void func_80018094(void);

/** @brief Defined in main.
 *  @param count how many times it prints its fixed string */
void func_80014BF0(s16 count);

/** @brief Defined in code_1a098. */
void func_800330D4(void);

/** @brief Defined in code_1902c. */
void func_8002980C(void);

/** @brief Defined in code_1902c. */
void func_80029838(void);

/** @brief Defined in code_24748; called from code_27bc8. */
void func_80036704(void);

/** @brief Defined in code_24748; called from code_27bc8. */
void func_80036878(void);

#endif
