/* The stage engine's data that all eight engine overlays share, in
 * sections, like engine_code.inc: each #include of this file emits the next
 * section, so an overlay includes it once per section, in address order,
 * around its own data. Overlays have no .bss: their zeroed buffers are
 * initialised definitions, which cc1 writes to .data.
 *
 * What differs per overlay is set before the first #include:
 *   ENGINE_SPEEDS_PAD  the halfword after sType1FSpeeds's five speeds;
 *   ENGINE_HINT_FILE   defined in the overlays (12, 15, 16) whose data holds
 *                      the stage-hint file's own copies of the burst tables. */

#ifndef ENGINE_DATA_PART
#define ENGINE_DATA_PART 1
#endif

#if ENGINE_DATA_PART == 1

/* The first of the overlay's three copies of the disc's file table:
 * sFiles1BootLoc to sFiles1LastSectors. */
#define DISC_FILE(x) sFiles1##x
#include "engine_files.inc"

#undef ENGINE_DATA_PART
#define ENGINE_DATA_PART 2

#elif ENGINE_DATA_PART == 2

/* A burst trap's pieces, eight of them: each one's direction on x and z, ... */
s32 sBurstDirs[8][2] = {
    {0, 0}, {-5, 6}, {4, -3}, {2, 5}, {-3, -1}, {6, -5}, {-1, 7}, {-5, 2},
};

/* ... its spin, of which only [0] is read ... */
s16 sBurstSpins[8][2] = {
    {2, -3}, {5, 2}, {-3, 6}, {-3, -1}, {7, -3}, {1, 2}, {-2, -6}, {3, 3},
};

/* ... and the height of its arc. */
s32 sBurstHeights[8] = {400, 200, 300, 600, 100, 500, 240, 20};

s8 sWalksReady = 0;        /* set once the walks are reset */
s8 sSpawnedTrapsReady = 0; /* set once the spawned traps are reset */
s8 sGoalLightsSet = 0;     /* set once the goal lights are on */

/* A type-1F trap's speed by its act. MATCHING: [5] is not a speed: it is
 * the halfword retail's link padded the file's data with, whatever its
 * buffer held, which differs per overlay. */
u16 sType1FSpeeds[6] = {10, 15, 20, 25, 30, ENGINE_SPEEDS_PAD};

#undef ENGINE_DATA_PART
#define ENGINE_DATA_PART 3

#elif ENGINE_DATA_PART == 3

/* The second copy, sFiles2BootLoc on: the one the stage's loader reads. */
#define DISC_FILE(x) sFiles2##x
#include "engine_files.inc"

/* The third copy, sFiles3BootLoc on. */
#define DISC_FILE(x) sFiles3##x
#include "engine_files.inc"

#ifdef ENGINE_HINT_FILE
/* The stage-hint file's own copies of the burst tables, in its order;
 * nothing reads them. */
s16 sHintBurstSpins[8][2] = {
    {2, -3}, {5, 2}, {-3, 6}, {-3, -1}, {7, -3}, {1, 2}, {-2, -6}, {3, 3},
};

s32 sHintBurstDirs[8][2] = {
    {0, 0}, {-5, 6}, {4, -3}, {2, 5}, {-3, -1}, {6, -5}, {-1, 7}, {-5, 2},
};

s32 sHintBurstHeights[8] = {400, 200, 300, 600, 100, 500, 240, 20};
#endif

#undef ENGINE_DATA_PART
#define ENGINE_DATA_PART 4

#elif ENGINE_DATA_PART == 4

/* The walking sprites' state (WALK_ANGLE, WALK_BASE, WALK_FRAME): three
 * words for each of the 100 stage sprites. */
s32 sWalkPose[300] = {0};

/* Each walking sprite's step, and its timer. Both are longer than the 100
 * sprites they serve; the sizes are retail's. */
u8 sWalkState[112] = {0};
s16 sWalkTimer[104] = {0};

/* The traps spawned during the stage (gSlotTable), and the goal's two. */
StageTrap sSpawnedTraps[200] = {0};
StageTrap sGoalTraps[2] = {0};

#undef ENGINE_DATA_PART
#define ENGINE_DATA_PART 5

#endif
