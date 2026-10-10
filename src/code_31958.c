#include "common.h"

INCLUDE_ASM("asm/nonmatchings/code_31958", func_80041158);

INCLUDE_ASM("asm/nonmatchings/code_31958", func_80041178);

INCLUDE_ASM("asm/nonmatchings/code_31958", func_80041198);

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
