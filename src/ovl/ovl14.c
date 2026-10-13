/* Stage 4-3's overlay: the act-3 chase of world 4, built from
 * chase.inc with its own objects and frame step. */

#define CHASE_LENGTH 10240000
#define CHASE_PLACEMENTS 53
#define CHASE_VAB 5
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
INCLUDE_ASM("asm/ovl/ovl14/nonmatchings/ovl14", StartChaseJump);
#endif

#include "chase_head.inc"

INCLUDE_ASM("asm/ovl/ovl14/nonmatchings/ovl14", func_800F0BE0);

INCLUDE_ASM("asm/ovl/ovl14/nonmatchings/ovl14", func_800F0EA8);

INCLUDE_ASM("asm/ovl/ovl14/nonmatchings/ovl14", func_800F1248);

INCLUDE_ASM("asm/ovl/ovl14/nonmatchings/ovl14", func_800F1654);

#include "chase_tail.inc"
