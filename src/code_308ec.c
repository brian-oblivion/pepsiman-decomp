#include "common.h"

#ifdef NON_MATCHING
/** @brief A record of a 128-entry table. Only one byte is known: the
 *         record's 1-based number, written at start-up. */
typedef struct {
    u8 unk0[0x13]; /**< not yet known */
    u8 unk13;      /**< set to the record's index + 1 */
} Unk800DFAB0;

extern Unk800DFAB0 D_800DFAB0[];
extern u8 D_80095AA8;
extern u8 D_80095AA9;

void func_800400EC(void) {
    s32 i;

    for (i = 0; i < 128; i++) {
        D_800DFAB0[i].unk13 = i + 1;
    }
    D_80095AA9 = 0;
    D_80095AA8 = 0xFF;
}
#else
INCLUDE_ASM("asm/nonmatchings/code_308ec", func_800400EC);
#endif

INCLUDE_ASM("asm/nonmatchings/code_308ec", func_80040130);

INCLUDE_ASM("asm/nonmatchings/code_308ec", func_800401F0);

INCLUDE_ASM("asm/nonmatchings/code_308ec", func_80040628);

INCLUDE_ASM("asm/nonmatchings/code_308ec", func_8004079C);

INCLUDE_ASM("asm/nonmatchings/code_308ec", func_80040998);

INCLUDE_ASM("asm/nonmatchings/code_308ec", func_80040CD0);

INCLUDE_ASM("asm/nonmatchings/code_308ec", func_80040E04);

INCLUDE_ASM("asm/nonmatchings/code_308ec", func_80040F14);

INCLUDE_ASM("asm/nonmatchings/code_308ec", func_80041118);
