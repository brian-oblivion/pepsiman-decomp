#include "common.h"

/* Overlay functions, loaded above the executable's code. */
void func_800F1908(void);
void func_800F1BA4(void);
void func_800F3A28(void);
void func_800F4984(void);
void func_800F66BC(void);
void func_800F6CF0(void);
void func_800F6F0C(void);
void func_800F90F4(void);
void func_800F936C(void);
void func_800F9980(void);
void func_800F9BFC(void);
void func_800F9F18(void);
void func_800F9F28(void);
void func_800FA058(void);
void func_800FA1A0(void);
void func_800FA6C0(void);
void func_800FA754(void);
void func_800FA800(void);
void func_800FA9D4(void);
void func_800FAA30(void);
void func_800FAB9C(void);
void func_800FABC4(void);
void func_800FAE44(void);

/** Each stage's successor: the next stage, and 0 after the last. */
u16 sNextStage[16] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 0};

/** Each stage's VAB bank: one per world of three stages, from bank 2. */
u16 sStageVabBanks[16] = {2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 5, 5, 6, 6, 6, 0};

u16 GetNextStage(u16 i) {
    return sNextStage[i];
}

s32 GetStageVabBank(u16 stage) {
    return sStageVabBanks[stage];
}

s32 GetStageTimeLimit(u8 mode, s32 k) {
    if (gStageTableSet != 1) {
        return gTimeLimits[mode][k] * 30;
    }
    return gTimeLimitsAlt[mode][k] * 30;
}

void LoadStageOverlay(u16 stage) {
    s32 id;

    switch (stage) {
        case STAGE_1_1:
            id = 3;
            break;
        case STAGE_1_2:
            id = 4;
            break;
        case STAGE_1_3:
            id = 5;
            break;
        case STAGE_2_1:
            id = 6;
            break;
        case STAGE_2_2:
            id = 7;
            break;
        case STAGE_2_3:
            id = 8;
            break;
        case STAGE_3_1:
            id = 9;
            break;
        case STAGE_3_2:
            id = 10;
            break;
        case STAGE_3_3:
            id = 11;
            break;
        case STAGE_4_1:
            id = 12;
            break;
        case STAGE_4_2:
            id = 13;
            break;
        case STAGE_4_3:
            id = 14;
            break;
        case STAGE_5_1:
            id = 15;
            break;
        case STAGE_5_2:
            id = 16;
            break;
        case STAGE_5_3:
            id = 17;
            break;
        default:
            return;
    }
    LoadOverlay(id);
}

void StartStageOverlay(void) {
    switch (gStage) {
        case STAGE_1_1:
            func_800FAB9C();
            break;
        case STAGE_1_2:
            func_800FAE44();
            break;
        case STAGE_2_1:
            func_800FA9D4();
            break;
        case STAGE_2_2:
            func_800FA800();
            break;
        case STAGE_3_1:
            func_800F1BA4();
            break;
        case STAGE_3_2:
            /* MATCHING: keeps cross-jumping from merging this arm with case 6. */
            func_800F1BA4();
            __asm__("");
            break;
        case STAGE_4_1:
            func_800F9BFC();
            break;
        case STAGE_4_2:
            func_800FA058();
            break;
        case STAGE_5_1:
            func_800FA1A0();
            break;
        case STAGE_5_2:
            func_800F936C();
            break;
    }
}

void ResetStageOverlay(void) {
    switch (gStage) {
        case STAGE_1_1:
            func_800FAA30();
            break;
        case STAGE_1_2:
            func_800FABC4();
            break;
        case STAGE_1_3:
            func_800F6F0C();
            break;
        case STAGE_2_1:
            func_800FA754();
            break;
        case STAGE_2_2:
            func_800FA6C0();
            break;
        case STAGE_2_3:
            func_800F6CF0();
            break;
        case STAGE_3_1:
            func_800F1908();
            break;
        case STAGE_3_2:
            /* MATCHING: keeps cross-jumping from merging this arm with case 6. */
            func_800F1908();
            __asm__("");
            break;
        case STAGE_3_3:
            func_800F66BC();
            break;
        case STAGE_4_1:
            func_800F9980();
            break;
        case STAGE_4_2:
            func_800F9F18();
            break;
        case STAGE_4_3:
            func_800F4984();
            break;
        case STAGE_5_1:
            func_800F9F28();
            break;
        case STAGE_5_2:
            func_800F90F4();
            break;
        case STAGE_5_3:
            func_800F3A28();
            break;
    }
}
