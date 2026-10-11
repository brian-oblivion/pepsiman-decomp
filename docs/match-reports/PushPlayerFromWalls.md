# PushPlayerFromWalls — MATCHED 140/140 (round 9, charlie)

> Renamed from `func_8002A328` on 2026-10-11 (tools/rename.py). Address 0x8002a328.

Unit `src/object.c`. Two wall probes: a point 50 units from the
player (`sGameHead` 0x348/0x34C/0x350) at the heading `D_800A7680[0].vy`
plus and minus 0x200 (45 degrees); UpdateSection finds the path segment
from `D_800957F4`, and when QueryWall reports a hit the player moves
25 units back along heading ±0x400. Then func_8002A558.

```c
/* MATCHING: stage defines x and y as s16; this unit passes them
 * unextended. */
void UpdateSection(s32 *index, s32 x, s32 y);

s32 QueryWall(s32 index, VECTOR *pos, u8 *data);
s32 func_8002A558(void);
extern s32 D_800957F4; /**< a path index the probe starts from */
extern s32 gFloorData; /**< the address of the data the probe tests */

/** @brief Probes 50 units ahead of the player, 45 degrees either side of
 *         the heading; each probe that hits pushes the player 25 units
 *         back from that side. Then runs the mode check. */
void PushPlayerFromWalls(void) {
    VECTOR pos;
    s32 idx;

    pos.vx = sGameHead.unk348 + (rsin(D_800A7680[0].vy + 0x200) * 50 >> 12);
    pos.vy = sGameHead.unk34C;
    pos.vz = sGameHead.unk350 + (rcos(D_800A7680[0].vy + 0x200) * 50 >> 12);
    idx = D_800957F4;
    UpdateSection(&idx, pos.vx, pos.vz);
    if (QueryWall(idx, &pos, (u8 *)gFloorData)) {
        sGameHead.unk348 -= rsin(D_800A7680[0].vy + 0x400) * 25 >> 12;
        sGameHead.unk350 -= rcos(D_800A7680[0].vy + 0x400) * 25 >> 12;
    }
    pos.vx = sGameHead.unk348 + (rsin(D_800A7680[0].vy - 0x200) * 50 >> 12);
    pos.vy = sGameHead.unk34C;
    pos.vz = sGameHead.unk350 + (rcos(D_800A7680[0].vy - 0x200) * 50 >> 12);
    idx = D_800957F4;
    UpdateSection(&idx, pos.vx, pos.vz);
    if (QueryWall(idx, &pos, (u8 *)gFloorData)) {
        sGameHead.unk348 -= rsin(D_800A7680[0].vy - 0x400) * 25 >> 12;
        sGameHead.unk350 -= rcos(D_800A7680[0].vy - 0x400) * 25 >> 12;
    }
    func_8002A558();
}```

First build. `GameHead` (unit-local) gained `unk348`, `unk34C`, `unk350`;
no offset moved.

UpdateSection's x and y arrive unextended (`lw` straight into `$a1`), so
this unit's view takes `s32` (LEARNINGS, "a callee's prototype is a
per-unit view"). QueryWall is still asm; its prototype here is a
guess from this call (index, position, data base). `D_800957F4` and
`gFloorData` are declared here as stage_flow and model declare them
(`s32`); declcheck lists them MULTI until a merge moves them to
`common.h`.
