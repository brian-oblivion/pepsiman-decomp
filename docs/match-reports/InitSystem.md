# InitSystem — MATCHED 49/49 (round 4, delta)

> Renamed from `func_80014AC8` on 2026-10-10 (tools/rename.py). Address 0x80014ac8.

Unit `src/main.c`. The boot-time system init: resets callbacks, zeroes
0xFC800 bytes from 0x80101000, brings up the memory card (BIOS and
libmcrd), the pads, the CD and the sound system, clears four byte flags,
then runs the game's own graphics and font setup.

```c
void InitSystem(void) {
    ResetCallback();
    bzero((u8 *)0x80101000, 0xFC800);
    InitCARD(1);
    StartCARD();
    MemCardInit(1);
    MemCardStart();
    ChangeClearPAD(0);
    _bu_init();
    InitPads();
    InitSound();
    CdInit();
    SsStart();
    gClearB = 0;
    gClearG = 0;
    gClearR = 0;
    InitGraphics();
    GsInit3D();
    ClearVram();
    D_80095820 = 0;
    InitDebugFont();
}
```

First build. All Sony prototypes come from Sony's headers; main.c now also
includes `libcd.h`, `libsnd.h`, `libmcrd.h` and `memory.h`. The four byte
flags are plain `u8` externs (`lui $at` stores); three of them were already
declared for `ResetClearColor` and moved up the file. `InitSound` is
defined in sound, so its prototype went into `include/common.h`
(additive; declcheck rejects a local prototype of another unit's
function). The game functions defined later in main.c get local
prototypes.
