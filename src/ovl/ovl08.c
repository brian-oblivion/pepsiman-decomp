/* Stage 2-3's overlay: the act-3 chase of world 2, built like stage 1-3's
 * from chase.inc with its own objects. */

#define CHASE_LENGTH 10240000
#define CHASE_PLACEMENTS 126
#define CHASE_VAB 3
#define CHASE_REACH 550
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

/* Sets the scene's three lights and its fog. */
void SetChaseLights(void) {
    sLights.l[0].r = 0x80;
    sLights.l[0].g = 0x80;
    sLights.l[0].b = 0x40;
    sLights.l[1].r = 0x80;
    sLights.l[1].g = 0x80;
    sLights.l[1].b = 0x40;
    sLights.l[2].r = 0x80;
    sLights.l[2].g = 0x80;
    sLights.l[2].b = 0x40;
    sLights.l[0].vx = 0;
    sLights.l[0].vy = 100;
    sLights.l[0].vz = 100;
    GsSetFlatLight(0, &sLights.l[0]);
    sLights.l[1].vx = 0x56;
    sLights.l[1].vy = 100;
    sLights.l[1].vz = -50;
    GsSetFlatLight(1, &sLights.l[1]);
    sLights.l[2].vx = -0x56;
    sLights.l[2].vy = 100;
    sLights.l[2].vz = -50;
    GsSetFlatLight(2, &sLights.l[2]);
    SetFogNearFar(3000, 8000, 250);
    SetFarColor(0, 0, 32);
}

#include "chase_tail.inc"
