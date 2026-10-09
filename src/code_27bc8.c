#include "common.h"
#include "libapi.h"

INCLUDE_ASM("asm/nonmatchings/code_27bc8", func_800373C8);

INCLUDE_ASM("asm/nonmatchings/code_27bc8", func_80037440);

INCLUDE_ASM("asm/nonmatchings/code_27bc8", func_80037700);

INCLUDE_ASM("asm/nonmatchings/code_27bc8", func_800377E8);

INCLUDE_ASM("asm/nonmatchings/code_27bc8", func_8003796C);

INCLUDE_ASM("asm/nonmatchings/code_27bc8", func_80037AE4);

INCLUDE_ASM("asm/nonmatchings/code_27bc8", func_80037C2C);

INCLUDE_ASM("asm/nonmatchings/code_27bc8", func_80037CF0);

INCLUDE_ASM("asm/nonmatchings/code_27bc8", func_80038124);

extern u8 *D_80095A18;
extern u8 *D_800959D4;
extern s32 D_800959E8;
extern s32 D_800959EC;
extern s32 D_800959F0;
extern s32 D_800959F4;
extern s32 D_800959FC;
extern s32 D_80095A00;
extern s32 D_80095A04;
extern s32 D_80095A08;

/** @brief A 0x18-byte record; the halfword at 0x14 is summed. */
typedef struct {
    u8 unk0[0x14]; /**< not yet known */
    u16 unk14;     /**< summed over the records in use */
    u8 unk16[2];   /**< not yet known */
} Rec18;

extern Rec18 D_800DF858[];
extern s32 D_800959DC;    /**< number of records in use */
extern s16 D_800959D0;    /**< 11 after a failed format, 12 after a failed erase */
extern char *D_80095A1C;  /**< name of the file last erased */
extern char D_800956A8[]; /**< "bu10:" */
extern char D_80011998[]; /**< "bu10:BISLPS-12345PEPTOOL" */
extern char D_80011C24[]; /**< "bu10:BISLPS-67890PEPTOOL" */

void func_80038730(void);
void func_800387A8(void);
s16 func_80038820(void);
void func_80038900(void);

s16 func_8003828C(u8 *a, u8 *b, s16 n) {
    s16 i;

    for (i = 0; i < n; i++) {
        if (*a != *b) {
            return -1;
        }
        a++;
        b++;
    }
    return 0;
}

u16 func_800382DC(void) {
    u8 *p;
    u16 sum;
    s32 i;
    u8 lo;
    u8 hi;

    p = D_80095A18;
    sum = 0;
    for (i = 0; i < 0xEFFF; i++) {
        lo = *p++;
        hi = *p++;
        sum += lo | (hi << 8);
    }
    return sum;
}

s16 func_8003831C(u16 expected) {
    u8 *p;
    u16 sum;
    s32 i;
    u8 lo;
    u8 hi;

    p = D_80095A18;
    sum = 0;
    for (i = 0; i < 0xEFFF; i++) {
        lo = *p++;
        hi = *p++;
        sum += lo | (hi << 8);
    }
    if (sum != expected) {
        return -1;
    }
    return 0;
}

void func_80038374(void) {
    D_800959D4 = (u8 *)0x8018D000;
    D_80095A18 = (u8 *)0x8016D000;
}

s16 func_80038394(void) {
    s16 result;

    result = 0;
    func_80038730();
    func_80038900();
    _card_info(0x10);
    if (func_80038820() == 2) {
        result = -1;
    }
    func_800387A8();
    return result;
}

s32 func_800383F8(void) {
    s32 ret;
    s16 i;

    ret = 0;
    for (i = 0; i < 10; i++) {
        if (format(D_800956A8) != 0) {
            return ret;
        }
    }
    D_800959D0 = 11;
    return -1;
}

INCLUDE_ASM("asm/nonmatchings/code_27bc8", func_80038468);

INCLUDE_RODATA("asm/nonmatchings/code_27bc8", D_80011998);

INCLUDE_ASM("asm/nonmatchings/code_27bc8", func_800384DC);

s16 func_80038574(void) {
    s16 sum;
    s16 i;

    sum = 0;
    for (i = 0; i < D_800959DC; i++) {
        sum += D_800DF858[i].unk14;
    }
    if (sum > 0) {
        return -1;
    }
    return 0;
}

INCLUDE_ASM("asm/nonmatchings/code_27bc8", func_800385E0);

INCLUDE_ASM("asm/nonmatchings/code_27bc8", func_800386A8);

INCLUDE_ASM("asm/nonmatchings/code_27bc8", func_80038730);

INCLUDE_ASM("asm/nonmatchings/code_27bc8", func_800387A8);

s16 func_80038820(void) {
    while (1) {
        if (TestEvent(D_800959E8) == 1) {
            return 0;
        }
        if (TestEvent(D_800959EC) == 1) {
            return 1;
        }
        if (TestEvent(D_800959F0) == 1) {
            return 2;
        }
        if (TestEvent(D_800959F4) == 1) {
            return 3;
        }
    }
}

s16 func_80038890(void) {
    while (1) {
        if (TestEvent(D_800959FC) == 1) {
            return 0;
        }
        if (TestEvent(D_80095A00) == 1) {
            return 1;
        }
        if (TestEvent(D_80095A04) == 1) {
            return 2;
        }
        if (TestEvent(D_80095A08) == 1) {
            return 3;
        }
    }
}

void func_80038900(void) {
    TestEvent(D_800959E8);
    TestEvent(D_800959EC);
    TestEvent(D_800959F0);
    TestEvent(D_800959F4);
}

void func_80038948(void) {
    TestEvent(D_800959FC);
    TestEvent(D_80095A00);
    TestEvent(D_80095A04);
    TestEvent(D_80095A08);
}

void func_80038990(u8 *p, s32 n) {
    s32 i;

    for (i = 0; i < n; i++) {
        *p++ = 0;
    }
}

INCLUDE_ASM("asm/nonmatchings/code_27bc8", func_800389B4);

INCLUDE_ASM("asm/nonmatchings/code_27bc8", func_80038C74);

INCLUDE_ASM("asm/nonmatchings/code_27bc8", func_80038DF8);

INCLUDE_ASM("asm/nonmatchings/code_27bc8", func_80038F70);

INCLUDE_ASM("asm/nonmatchings/code_27bc8", func_800390B8);

void func_800394EC(void) {
    D_800959D4 = (u8 *)0x80195000;
    D_80095A18 = (u8 *)0x8018D000;
}

INCLUDE_ASM("asm/nonmatchings/code_27bc8", func_8003950C);

INCLUDE_RODATA("asm/nonmatchings/code_27bc8", D_800119C8);

INCLUDE_RODATA("asm/nonmatchings/code_27bc8", D_800119E4);

INCLUDE_RODATA("asm/nonmatchings/code_27bc8", D_80011AA4);

INCLUDE_RODATA("asm/nonmatchings/code_27bc8", D_80011B64);

INCLUDE_RODATA("asm/nonmatchings/code_27bc8", D_80011C24);

INCLUDE_ASM("asm/nonmatchings/code_27bc8", func_80039580);

INCLUDE_ASM("asm/nonmatchings/code_27bc8", func_80039618);

INCLUDE_ASM("asm/nonmatchings/code_27bc8", func_8003968C);

INCLUDE_RODATA("asm/nonmatchings/code_27bc8", D_80011C54);

INCLUDE_RODATA("asm/nonmatchings/code_27bc8", D_80011C70);

INCLUDE_RODATA("asm/nonmatchings/code_27bc8", D_80011D30);

INCLUDE_RODATA("asm/nonmatchings/code_27bc8", D_80011DF0);

INCLUDE_RODATA("asm/nonmatchings/code_27bc8", D_80011EB0);
