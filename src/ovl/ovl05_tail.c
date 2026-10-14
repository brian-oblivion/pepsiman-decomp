/* Stage 1-3's tail: the scene's lights, the object placement, the loader
 * and the reset, from chase_tail.inc. A unit of its own: its rodata
 * starts at the loader's string, which puts the loader's jump table 0x10
 * past it, as in every chase. */

#define CHASE_PLACEMENTS 103
#define CHASE_VAB 2
#define CHASE_LIGHT_BLUE 0x80
#define CHASE_FAR_COLOR 0xB0, 0xB0, 0xB0
#define CHASE_FILE_LOC sCdData4Loc
#define CHASE_FILE_SECTORS sCdData4Sectors

#include "chase.inc"
#include "chase_tail.inc"
