/* Stage 5-3's tail: the scene's lights, the object placement, the loader
 * and the reset, from chase_tail.inc. A unit of its own: its rodata
 * starts at the loader's string, which puts the loader's jump table 0x10
 * past it, as in every chase. */

#define CHASE_PLACEMENTS 90
#define CHASE_VAB 6
#define CHASE_LIGHT_BLUE 0x80
#define CHASE_FAR_COLOR 0xB0, 0xB0, 0xB0
#define CHASE_FILE_LOC sFiles2Ovl17Loc
#define CHASE_FILE_SECTORS sFiles2Ovl17Sectors

#include "chase.inc"
#include "chase_tail.inc"
