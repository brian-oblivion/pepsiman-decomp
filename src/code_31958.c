#include "common.h"

/* Overlay functions, loaded above the executable's code. */
void func_800F1BA4(void);
void func_800F936C(void);
void func_800F9BFC(void);
void func_800FA058(void);
void func_800FA1A0(void);
void func_800FA800(void);
void func_800FA9D4(void);
void func_800FAB9C(void);
void func_800FAE44(void);

extern u16 D_8007B0EC[];
extern u16 D_8007B10C[];

u16 func_80041158(u16 i) {
    return D_8007B0EC[i];
}

s32 func_80041178(u16 stage) {
    return D_8007B10C[stage];
}

s32 func_80041198(u8 mode, s32 k) {
    if (D_800958AC != 1) {
        return D_8007AF84[mode][k] * 30;
    }
    return D_8007B038[mode][k] * 30;
}

void func_8004121C(u16 sel) {
    s32 id;

    switch (sel) {
        case 0:
            id = 3;
            break;
        case 1:
            id = 4;
            break;
        case 2:
            id = 5;
            break;
        case 3:
            id = 6;
            break;
        case 4:
            id = 7;
            break;
        case 5:
            id = 8;
            break;
        case 6:
            id = 9;
            break;
        case 7:
            id = 10;
            break;
        case 8:
            id = 11;
            break;
        case 9:
            id = 12;
            break;
        case 10:
            id = 13;
            break;
        case 11:
            id = 14;
            break;
        case 12:
            id = 15;
            break;
        case 13:
            id = 16;
            break;
        case 14:
            id = 17;
            break;
        default:
            return;
    }
    func_80022554(id);
}

void func_800412DC(void) {
    switch (D_80095830) {
        case 0:
            func_800FAB9C();
            break;
        case 1:
            func_800FAE44();
            break;
        case 3:
            func_800FA9D4();
            break;
        case 4:
            func_800FA800();
            break;
        case 6:
            func_800F1BA4();
            break;
        case 7:
            /* MATCHING: keeps cross-jumping from merging this arm with case 6. */
            func_800F1BA4();
            __asm__("");
            break;
        case 9:
            func_800F9BFC();
            break;
        case 10:
            func_800FA058();
            break;
        case 12:
            func_800FA1A0();
            break;
        case 13:
            func_800F936C();
            break;
    }
}

INCLUDE_ASM("asm/nonmatchings/code_31958", func_800413BC);
