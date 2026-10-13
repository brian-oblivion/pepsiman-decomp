/* Stage 3-1's overlay: world 3's free-running stage code, stage3.inc,
 * with this overlay's data. */

#define STAGE3_ASM_04C4 INCLUDE_ASM("asm/ovl/ovl09/nonmatchings/ovl09", func_800F04C4);
#define STAGE3_ASM_1908 INCLUDE_ASM("asm/ovl/ovl09/nonmatchings/ovl09", func_800F1908);
#define STAGE3_RODATA_SHAPES INCLUDE_RODATA("asm/ovl/ovl09/nonmatchings/ovl09", D_800F0074);

#include "stage3.inc"
