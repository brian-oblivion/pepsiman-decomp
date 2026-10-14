/* Stage 2-3's tail: the scene's lights, the object placement, the loader
 * and the reset, from chase_tail.inc. A unit of its own: its rodata
 * starts at the loader's string, which puts the loader's jump table 0x10
 * past it, as in every chase. */

#define CHASE_PLACEMENTS 126
#define CHASE_VAB 3
#define CHASE_LIGHT_BLUE 0x40
#define CHASE_FAR_COLOR 0, 0, 32

#include "chase.inc"
#include "chase_tail.inc"
