/* Stage 2-3's overlay: the act-3 chase of world 2, built like stage 1-3's
 * from chase.inc with its own objects. */

#define CHASE_LENGTH 10240000
#define CHASE_PLACEMENTS 126
#define CHASE_VAB 3
#define CHASE_REACH 550
#define CHASE_LIGHT_BLUE 0x40
#define CHASE_FAR_COLOR 0, 0, 32
/* MATCHING: the loader's jump table sits 4 bytes past an 8-byte boundary;
 * this toolchain aligns jump tables to 8, so the loader stays asm. */
#define CHASE_LOADER INCLUDE_ASM("asm/ovl/ovl08/nonmatchings/ovl08", LoadChaseStage);

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
INCLUDE_ASM("asm/ovl/ovl08/nonmatchings/ovl08", StartChaseJump);
#endif

#include "chase_head.inc"

#include "chase_pieces.inc"

INCLUDE_ASM("asm/ovl/ovl08/nonmatchings/ovl08", func_800F4758);


#include "chase_tail.inc"
