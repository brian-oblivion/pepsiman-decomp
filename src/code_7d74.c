#include "common.h"
#include "rand.h"
#include "libgte.h"
#include "libgpu.h"
#include "libgs.h"
#include "libcd.h"

typedef struct {
    s32 offset;
    u8 unk4[0xA];
    u16 count;
} PackEntry;

typedef struct {
    s32 unk0;
    s32 unk4;
    s32 unk8;
    s32 unkC;
} Slot;

typedef struct {
    Slot *slots;
    s32 count;
    s32 capacity;
} SlotList;

extern u8 D_800958C9;
extern CdlLOC D_80095728;

void func_80017774(void *data);
s32 func_800175AC(u8 com);


INCLUDE_ASM("asm/nonmatchings/code_7d74", func_80017574);

INCLUDE_ASM("asm/nonmatchings/code_7d74", func_800175AC);

void func_80017614(u8 mode) {
    if (mode == 4) {
        func_800175AC(CdlPlay);
    }
}

INCLUDE_ASM("asm/nonmatchings/code_7d74", func_80017640);

INCLUDE_ASM("asm/nonmatchings/code_7d74", func_80017774);

INCLUDE_ASM("asm/nonmatchings/code_7d74", func_80017880);

INCLUDE_ASM("asm/nonmatchings/code_7d74", func_8001797C);

INCLUDE_ASM("asm/nonmatchings/code_7d74", func_800179F8);

s32 func_80017B18(void) {
    return rand();
}

INCLUDE_ASM("asm/nonmatchings/code_7d74", func_80017B38);

INCLUDE_ASM("asm/nonmatchings/code_7d74", func_80017DD4);

INCLUDE_ASM("asm/nonmatchings/code_7d74", func_80017F0C);

INCLUDE_ASM("asm/nonmatchings/code_7d74", func_80018094);

INCLUDE_ASM("asm/nonmatchings/code_7d74", func_8001819C);

INCLUDE_ASM("asm/nonmatchings/code_7d74", func_800183B0);

INCLUDE_ASM("asm/nonmatchings/code_7d74", func_800184BC);

INCLUDE_ASM("asm/nonmatchings/code_7d74", func_80018AE0);

INCLUDE_ASM("asm/nonmatchings/code_7d74", func_80018BD8);

void func_80018CA4(void) {}

void func_80018CAC(void) {}

INCLUDE_ASM("asm/nonmatchings/code_7d74", func_80018CB4);

#ifdef NON_MATCHING
s16 func_80018D04(s16 from, s16 to, u16 step, u16 steps) {
    if (step == steps) {
        return to;
    }
    return from + ((((to - from) << 16) / steps) * step) / 0x10000;
}
#else
INCLUDE_ASM("asm/nonmatchings/code_7d74", func_80018D04);
#endif

INCLUDE_ASM("asm/nonmatchings/code_7d74", func_80018D70);

void func_80018DE8(void) {}

INCLUDE_ASM("asm/nonmatchings/code_7d74", func_80018DF0);

INCLUDE_ASM("asm/nonmatchings/code_7d74", func_800195CC);

INCLUDE_ASM("asm/nonmatchings/code_7d74", func_80019684);

INCLUDE_ASM("asm/nonmatchings/code_7d74", func_800196E4);

INCLUDE_ASM("asm/nonmatchings/code_7d74", func_80019730);

INCLUDE_ASM("asm/nonmatchings/code_7d74", func_800197E4);

s32 *func_80019874(s32 *table, s32 *keys, s32 key) {
    s32 n;

    table++;
    n = *table++;
    while (n > 0) {
        if (key == *keys) {
            break;
        }
        table += 7;
        n--;
        keys++;
    }
    if (n == 0) {
        return NULL;
    }
    return table;
}

INCLUDE_RODATA("asm/nonmatchings/code_7d74", D_80010454);
