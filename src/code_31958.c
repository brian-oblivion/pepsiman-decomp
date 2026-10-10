#include "common.h"

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

INCLUDE_ASM("asm/nonmatchings/code_31958", func_800412DC);

INCLUDE_ASM("asm/nonmatchings/code_31958", func_800413BC);
