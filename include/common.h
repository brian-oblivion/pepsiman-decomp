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
extern s32 D_800956D4;  /**< 2 units, 4 functions */
extern u8 D_8009574C;   /**< fog colour, red; 2 units */
extern u8 D_80095754;   /**< fog colour, green; 2 units */
extern u8 D_8009575C;   /**< fog colour, blue; 2 units */
extern u8 D_800956F7;   /**< 3 units, 3 functions */
extern s32 D_80095714;  /**< 2 units, 2 functions */
extern s32 D_8009571C;  /**< 2 units, 2 functions */
extern s32 D_80095720;  /**< 2 units, 2 functions */
extern u16 D_80095748;  /**< 2 units, 2 functions */
extern s16 D_8009574A;  /**< the highlighted line of a three-line menu */
extern u32 D_80095794;  /**< an entry count */
extern s32 D_80095750;  /**< 2 units, 3 functions */
extern s32 D_80095964;  /**< button flags; 2 units */
extern s32 D_800957EC;  /**< remapped button flags; 2 units */
extern u16 D_80095760;  /**< the model viewer's state; 2 units */
extern u16 D_80095768;  /**< 2 units, 2 functions; never loaded, sign unknown */
extern u8 D_800957D5;   /**< 2 units, 2 functions; never loaded, sign unknown */
extern u8 D_800957D6;   /**< 2 units, 3 functions; never loaded, sign unknown */
extern u8 D_800958EC;   /**< 2 units, 4 functions; never loaded, sign unknown */
extern s16 D_80095858;  /**< 2 units; set to 2 at the goal when 0 */
extern s32 D_800958CC;  /**< 2 units, 2 functions; a loop counter kept in a global */
extern s32 D_800959B4;  /**< 2 units, 2 functions */
extern s32 D_800959B8;  /**< 2 units, 2 functions */
extern s32 D_80095958;  /**< a pad word; its bits step a position or a highlighted line; 2 units */
extern s32 D_800959A0;  /**< printed first on the status panel; 2 units */
extern s32 D_800958B4;  /**< the address of the floor data a probe tests; 2 units */
extern u8 D_800D3CA8[]; /**< a 0x44C0-byte buffer of 0x2C-byte placed records */
extern u8 D_800DB2C0[]; /**< a 0x1DB0-byte buffer of 0x4C-byte records */
extern u8 D_800958C9;   /**< 1 while CD audio is playing; 2 units */
extern u16 D_80095880;  /**< 2 units */
extern s16 D_80095960;  /**< 2 units */
extern u8 D_8009586C;   /**< the view mode: 0 the game's reference view, 1 a fixed one; 2 units */
extern s8 D_80095974;   /**< nonzero stops the frame counter; 2 units */
extern s32 ClipF;       /**< a word inside libgte's clipf object, as a variable; 2 units */
extern s32 D_80095904;  /**< the mapped model data's TMD address; 2 units */
extern s8 D_8009596C;   /**< 2 units */
extern u8 D_80095AEE;   /**< nonzero for stereo: the CD mix has no cross-feed; 2 units */
extern s16 D_800959E0;  /**< 2 units, 2 functions */
extern u16 D_800959E4;  /**< 2 units, 4 functions */
extern s16 D_80095A0C;  /**< 2 units, 4 functions */
extern u16 D_80095A14;  /**< 3 units, 3 functions; never loaded, sign unknown */
extern s32 D_80095A4C;  /**< 2 units, 7 functions */
extern s32 D_80095A50;  /**< 2 units, 7 functions */
extern s32 D_80095780;  /**< entry count of the first record bank; 2 units */
extern s32 D_80095810;  /**< entry count of the second record bank; 2 units */
extern u16 D_80095B0E;  /**< 2 units, 3 functions */
extern u8 D_80095830;   /**< 8 units, 24 functions */
extern s32 D_80095864;  /**< 3 units, 5 functions */
extern u8 D_80095AA8;   /**< 2 units, 2 functions */
extern u8 D_80095AA9;   /**< 2 units, 2 functions */
extern u16 D_800958E8;  /**< 5 units, 7 functions */
extern s16 D_800958B0;  /**< limit of the value being edited */
extern s16 D_800958B2;  /**< the count a menu line wraps at */
extern s32 D_80095970;  /**< flag word; bit 5 enables a two-state dispatch */
extern s16 D_800E474C;  /**< libgs's PSDIDX; 2 units */
extern u8 *D_800959C0;  /**< the bytes after a BlockHeader */
extern u8 *D_800959C4;  /**< the BlockHeader's second part */
extern u8 *D_800E48D0;  /**< the next free byte of the primitive buffer; 5 units */
extern s32 D_800959C8;  /**< the BlockHeader's first word */
extern s16 D_80095AF0;  /**< 2 units, 2 functions */
extern s32 D_800958A8;  /**< 2 units; 3 when a stage starts */
extern s16 D_800957BC;  /**< 2 units; a heading */
extern u8 *D_800958FC;  /**< 2 units; far (fog) colours, four bytes each */
extern u16 D_8009576A;  /**< 2 units; fog fade-in step: 0..31, then 100 */
extern s32 D_800957A8;  /**< 2 units */
extern s32 D_800958AC;  /**< 2 units; 1 picks the second set of stage tables */
extern s8 D_8009599C;   /**< 2 units */
extern u16 D_800957D2;  /**< 2 units */
extern u32 D_8009585C;  /**< a frame counter; 2 units */
extern s32 D_800958D0;  /**< a loop counter or count kept in a global; 2 units */
extern s16 D_8009588E;  /**< number of entries; 2 units */
extern s16 D_80095914;  /**< the view's orbit angle, in degrees; 2 units */
extern s32 D_8009578C;  /**< stamped on placed records; 2 units */
extern u16 D_800958A6;  /**< a state switched on in main; cleared after placing; 2 units */
extern s16 D_800958DA;  /**< an error code; cleared when flag bit 6 is set; 2 units */
extern s32 D_800957F4;  /**< the result of the start-position lookup; 2 units */

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

extern s32 *D_800D81B0[]; /**< per-sequence data pointers; 2 units */

/* --- Functions called from more than one unit, with the same prototype ---- */

/** @brief Defined in code_31cec.
 *  @param id what to start, by number */
void func_80042538(s32 id);

/** @brief Defined in code_31cec; called from main. */
void func_800428B0(void);

/** @brief Defined in code_7d74; called from main. */
void func_80018CB4(void);

/** @brief Defined in code_7d74: updates an index from a position, then
 *         checks the position against the data blob.
 *  @param pos a position, three words
 *  @param arg a result buffer for the check
 *  @param cur the current index
 *  @return the updated index, or -1 when the check fails */
s32 func_80018D70(void *pos, void *arg, s32 cur);

/** @brief Defined in code_7d74; called when a stage is reset. */
void func_80017574(void);

/** @brief Defined in code_7d74; called from main: sets up the three flat
 *         lights, the ambient light and the fog. */
void func_80018094(void);

/** @brief Defined in main.
 *  @param count how many times it prints its fixed string */
void func_80014BF0(s16 count);

/** @brief Defined in code_13068: on every tenth pickup, plays a cue
 *         (unless the stage forbids it) and updates the pickup display. */
void func_80028448(void);

/** @brief Defined in code_29f54; loads a run of TIM images (each starting
 *         with the word 0x10) into VRAM.
 *  @param p the first TIM */
void func_8003E13C(unsigned long *p);

/** @brief Defined in code_29f54 (still asm); called from main. */
void func_8003A84C(void);

/** @brief Defined in code_29f54: queues the draw-mode and clipped
 *         draw-environment packets that split the screen.
 *  @param clip nonzero to queue them; zero does nothing */
void func_80039754(s32 clip);

/** @brief Defined in code_7d74; issues one CD command.
 *  @param com the CdlXXX command
 *  @return always 0 */
s32 func_800175AC(u8 com);

/** @brief Defined in code_7d74; the CD ready callback main installs.
 *  @param mode the interrupt status */
void func_80017614(u8 mode);

/** @brief Defined in code_a0bc: fills one 8-byte record of a sprite table.
 *  @param id    which record
 *  @param mode  stored at bit 7 of the record's first halfword
 *  @param w     stored in byte 6
 *  @param h     stored in byte 7
 *  @param page  the low byte of the first halfword (read as a byte)
 *  @param u     stored in byte 4
 *  @param v     stored in byte 5
 *  @param clutX its top 12 bits form the low bits of the second halfword
 *  @param clutY stored from bit 6 of the second halfword */
void func_8001B2F4(u16 id, u8 mode, s32 w, s32 h, s32 page, s32 u, s32 v, u16 clutX, s32 clutY);

/** @brief Defined in code_1dc24.
 *  @return 0. */
s32 func_800330D4(void);

/** @brief Defined in code_29f54: remaps three buttons of the two button
 *         words by a row of a remap table. */
void func_8003DE34(void);

/** @brief Defined in code_1902c. */
void func_8002980C(void);

/** @brief Defined in code_1902c. */
void func_80029838(void);

/** @brief Defined in code_24748; called from code_27bc8. */
void func_80036704(void);

/** @brief Defined in code_24748; called from code_27bc8. */
void func_80036878(void);

/** @brief Defined in code_13068; called from code_308ec. Resets the
 *         stepped value two mode handlers steer, to 40 whole units. */
void func_800287C0(void);

/** @brief Defined in code_1dc24. */
void func_80033C90(void);

/** @brief Defined in code_1dc24. */
void func_80033D3C(void);

/** @brief Defined in code_1dc24: wraps the highlighted line at `n` lines
 *         and runs an update.
 *  @param n the number of lines
 *  @return the highlighted line when flag bit 5 is set, else -1. */
s32 func_8003356C(s16 n);

/** @brief Defined in code_27bc8. */
void func_80038124(void);

/** @brief Defined in code_27bc8. */
void func_800386A8(void);

/** @brief Defined in code_27bc8: runs one memory card operation on the
 *         second save file.
 *  @param mode 0 reads, 1 rewrites, 2 creates, 3 formats, 4 erases, 5 checks
 *  @return 0 or -1 */
s32 func_800389B4(s16 mode);

/** @brief Defined in code_1902c: tests a 0x4C-byte record.
 *  @param rec the record */
void func_80028888(void *rec);

#endif
