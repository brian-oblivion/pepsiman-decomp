/* Stage 5-3's overlay: the act-3 chase of world 5, built from
 * chase.inc with its own objects and frame step. */

#define CHASE_LENGTH 12800000
#define CHASE_PLACEMENTS 90
#define CHASE_VAB 6
#define CHASE_LIGHT_BLUE 0x80
#define CHASE_FAR_COLOR 0xB0, 0xB0, 0xB0

#include "chase.inc"

/* Starts a jump on a face button, unless the play is frozen. */
#ifdef NON_MATCHING
void StartChaseJump(void) {
    if (ClipF == 0 && (gGamePadTrig & (PADRleft | PADRdown | PADRright))) {
        sJumpSpeed = -40;
        gPlayer.slideState = 1;
        gPlayer.unk38C = 0;
        gPlayer.curMotion[0] = MOT_OFF;
        PlaySe(0x17);
    }
}
#else
INCLUDE_ASM("asm/ovl/ovl17/nonmatchings/ovl17", StartChaseJump);
#endif

#include "chase_head.inc"

INCLUDE_ASM("asm/ovl/ovl17/nonmatchings/ovl17", func_800F0BB8);

INCLUDE_ASM("asm/ovl/ovl17/nonmatchings/ovl17", func_800F0E80);

INCLUDE_ASM("asm/ovl/ovl17/nonmatchings/ovl17", func_800F1220);

INCLUDE_ASM("asm/ovl/ovl17/nonmatchings/ovl17", func_800F14E8);

INCLUDE_ASM("asm/ovl/ovl17/nonmatchings/ovl17", func_800F17B0);

#include "chase_tail.inc"
