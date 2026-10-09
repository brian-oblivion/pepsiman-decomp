#include "common.h"
#include "libgte.h"
#include "libgpu.h"
#include "libgs.h"
#include "code_a0bc.h"

/** @brief The 0x14-byte records of common.h's NumberedSlot table, as the
 *         free list here hands them out. */
typedef struct {
    s16 unk0;  /**< cleared when the record is taken */
    s16 unk2;  /**< set when the record is taken */
    s16 unk4;  /**< set when the record is taken */
    s16 unk6;  /**< set when the record is taken */
    s16 unk8;  /**< set when the record is taken */
    s16 unkA;  /**< set when the record is taken */
    s16 unkC;  /**< set when the record is taken */
    s16 unkE;  /**< set when the record is taken */
    s16 unk10; /**< set when the record is taken */
    u8 unk12;  /**< set when the record is taken */
    u8 next;   /**< free-list link: the next record's index */
} Slot;

#define sSlots ((Slot *)D_800DFAB0)

u16 func_800FAD08(s16 arg0, s32 arg1, s32 arg2);
u16 func_800FAD04(s16 arg0, s32 arg1, s32 arg2);
u16 func_800FA894(s16 arg0, s32 arg1, s32 arg2);
u16 func_800FA580(s16 arg0, s32 arg1, s32 arg2);
u16 func_800F1A48(s16 arg0, s32 arg1, s32 arg2);
u16 func_800F9ABC(s16 arg0, s32 arg1, s32 arg2);
u16 func_800F9CA4(s16 arg0, s32 arg1, s32 arg2);
u16 func_800FA060(s16 arg0, s32 arg1, s32 arg2);
u16 func_800F922C(s16 arg0, s32 arg1, s32 arg2);

s32 func_800FA6C8(void);
s32 func_800FA804(void);
s32 func_800F6CE4(void);
s32 func_800FA3EC(void);
s32 func_800FA1C0(void);
s32 func_800F6AC8(void);
s32 func_800F1080(void);
s32 func_800F64B0(void);
s32 func_800F9618(void);
s32 func_800F98E4(void);
s32 func_800F475C(void);
s32 func_800F9BC0(void);
s32 func_800F8D34(void);
s32 func_800F3800(void);

void func_800FA5D0(void);

void func_800400EC(void) {
    s32 i;

    for (i = 0; i < 128; i++) {
        D_800DFAB0[i].unk13 = i + 1;
    }
    D_80095AA9 = 0;
    D_80095AA8 = 0xFF;
}

INCLUDE_ASM("asm/nonmatchings/code_308ec", func_80040130);

INCLUDE_ASM("asm/nonmatchings/code_308ec", func_800401F0);

INCLUDE_ASM("asm/nonmatchings/code_308ec", func_80040628);

INCLUDE_ASM("asm/nonmatchings/code_308ec", func_8004079C);

INCLUDE_ASM("asm/nonmatchings/code_308ec", func_80040998);

INCLUDE_ASM("asm/nonmatchings/code_308ec", func_80040CD0);

INCLUDE_ASM("asm/nonmatchings/code_308ec", func_80040E04);

INCLUDE_ASM("asm/nonmatchings/code_308ec", func_80040F14);

void func_80041118(u8 arg0) {
    s32 mode = arg0;

    if (mode != 1) {
        if (mode < 2) {
            if (mode == 0) {
                func_800FA5D0();
            }
        }
    }
}
