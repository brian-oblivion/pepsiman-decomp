/* Stage 1-3's overlay: the act-3 chase, where the player runs toward the
 * camera, jumps and steps sideways, and the stage's own objects come at
 * it. */

#define CHASE_LENGTH 7680000
#define CHASE_PLACEMENTS 103
#define CHASE_VAB 2
#define CHASE_REACH 120
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
INCLUDE_ASM("asm/ovl/ovl05/nonmatchings/ovl05", StartChaseJump);
#endif

#include "chase_head.inc"

#include "chase_pieces.inc"

INCLUDE_ASM("asm/ovl/ovl05/nonmatchings/ovl05", func_800F4794);


#include "chase_tail.inc"
