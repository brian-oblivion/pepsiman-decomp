#include "common.h"

INCLUDE_ASM("asm/nonmatchings/code_27bc8", func_800373C8);

INCLUDE_ASM("asm/nonmatchings/code_27bc8", func_80037440);

INCLUDE_ASM("asm/nonmatchings/code_27bc8", func_80037700);

INCLUDE_ASM("asm/nonmatchings/code_27bc8", func_800377E8);

INCLUDE_ASM("asm/nonmatchings/code_27bc8", func_8003796C);

INCLUDE_ASM("asm/nonmatchings/code_27bc8", func_80037AE4);

INCLUDE_ASM("asm/nonmatchings/code_27bc8", func_80037C2C);

INCLUDE_ASM("asm/nonmatchings/code_27bc8", func_80037CF0);

INCLUDE_ASM("asm/nonmatchings/code_27bc8", func_80038124);

INCLUDE_ASM("asm/nonmatchings/code_27bc8", func_8003828C);

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

long TestEvent(long event);

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

INCLUDE_ASM("asm/nonmatchings/code_27bc8", func_8003831C);

void func_80038374(void) {
    D_800959D4 = (u8 *)0x8018D000;
    D_80095A18 = (u8 *)0x8016D000;
}

INCLUDE_ASM("asm/nonmatchings/code_27bc8", func_80038394);

INCLUDE_ASM("asm/nonmatchings/code_27bc8", func_800383F8);

INCLUDE_ASM("asm/nonmatchings/code_27bc8", func_80038468);

INCLUDE_RODATA("asm/nonmatchings/code_27bc8", D_80011998);

INCLUDE_ASM("asm/nonmatchings/code_27bc8", func_800384DC);

INCLUDE_ASM("asm/nonmatchings/code_27bc8", func_80038574);

INCLUDE_ASM("asm/nonmatchings/code_27bc8", func_800385E0);

INCLUDE_ASM("asm/nonmatchings/code_27bc8", func_800386A8);

INCLUDE_ASM("asm/nonmatchings/code_27bc8", func_80038730);

INCLUDE_ASM("asm/nonmatchings/code_27bc8", func_800387A8);

INCLUDE_ASM("asm/nonmatchings/code_27bc8", func_80038820);

INCLUDE_ASM("asm/nonmatchings/code_27bc8", func_80038890);

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
