#include "common.h"

/* Overlay functions, loaded above the executable's code: each overlay's name
 * plus the stage it is for (CopySpriteShapes4_1 is ovl12's CopySpriteShapes;
 * world 3's first two stages share theirs). */
void ResetToStageStart3(void);
void CopySpriteShapes3(void);
void ResetChaseStage5_3(void);
void ResetChaseStage4_3(void);
void ResetChaseStage3_3(void);
void ResetChaseStage2_3(void);
void ResetChaseStage1_3(void);
void ResetToStageStart5_2(void);
void CopySpriteShapes5_2(void);
void ResetToStageStart4_1(void);
void CopySpriteShapes4_1(void);
void ResetToStageStart4_2(void);
void ResetToStageStart5_1(void);
void CopySpriteShapes4_2(void);
void CopySpriteShapes5_1(void);
void ResetToStageStart2_2(void);
void ResetToStageStart2_1(void);
void CopySpriteShapes2_2(void);
void CopySpriteShapes2_1(void);
void ResetToStageStart1_1(void);
void CopySpriteShapes1_1(void);
void ResetToStageStart1_2(void);
void CopySpriteShapes1_2(void);

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
            CopySpriteShapes1_1();
            break;
        case STAGE_1_2:
            CopySpriteShapes1_2();
            break;
        case STAGE_2_1:
            CopySpriteShapes2_1();
            break;
        case STAGE_2_2:
            CopySpriteShapes2_2();
            break;
        case STAGE_3_1:
            CopySpriteShapes3();
            break;
        case STAGE_3_2:
            /* MATCHING: keeps cross-jumping from merging this arm with case 6. */
            CopySpriteShapes3();
            __asm__("");
            break;
        case STAGE_4_1:
            CopySpriteShapes4_1();
            break;
        case STAGE_4_2:
            CopySpriteShapes4_2();
            break;
        case STAGE_5_1:
            CopySpriteShapes5_1();
            break;
        case STAGE_5_2:
            CopySpriteShapes5_2();
            break;
    }
}

void ResetStageOverlay(void) {
    switch (gStage) {
        case STAGE_1_1:
            ResetToStageStart1_1();
            break;
        case STAGE_1_2:
            ResetToStageStart1_2();
            break;
        case STAGE_1_3:
            ResetChaseStage1_3();
            break;
        case STAGE_2_1:
            ResetToStageStart2_1();
            break;
        case STAGE_2_2:
            ResetToStageStart2_2();
            break;
        case STAGE_2_3:
            ResetChaseStage2_3();
            break;
        case STAGE_3_1:
            ResetToStageStart3();
            break;
        case STAGE_3_2:
            /* MATCHING: keeps cross-jumping from merging this arm with case 6. */
            ResetToStageStart3();
            __asm__("");
            break;
        case STAGE_3_3:
            ResetChaseStage3_3();
            break;
        case STAGE_4_1:
            ResetToStageStart4_1();
            break;
        case STAGE_4_2:
            ResetToStageStart4_2();
            break;
        case STAGE_4_3:
            ResetChaseStage4_3();
            break;
        case STAGE_5_1:
            ResetToStageStart5_1();
            break;
        case STAGE_5_2:
            ResetToStageStart5_2();
            break;
        case STAGE_5_3:
            ResetChaseStage5_3();
            break;
    }
}
