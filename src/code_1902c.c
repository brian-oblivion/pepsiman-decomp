#include "common.h"
#include "libgte.h"
#include "libgpu.h"
#include "libgs.h"

/* MATCHING: retail reaches these through a split lui/%lo pair, so each is
 * an array of unknown size here. */
extern s32 D_8009EF30[];

extern GsOT D_800ACEA8[];
extern s32 D_80095750;

/** @brief A 72-byte record in the slot table the free-slot search walks;
 *         only the free marker is known. */
typedef struct {
    u8 pad0[0x36];
    s16 unk36; /**< -1 when the record is free */
    u8 pad38[0x48 - 0x38];
} Slot48;

/** @brief A position with a radius further in, as the collision test
 *         reads it. */
typedef struct {
    s32 x; /**< position */
    s32 y; /**< position */
    s32 z; /**< position */
    u8 padC[8];
    s32 radius; /**< summed with the other body's radius */
} Body;

void func_8001A3D4(s32 id, SVECTOR *size, CVECTOR *color, s32 mode, GsOT *ot);
void func_80028984(void);
void func_8002985C(void);
void func_8002988C(void);

s16 func_8002882C(void) {
    Slot48 *slot = (Slot48 *)D_800959B8;
    u32 i;

    for (i = 0; i < D_800959B4 / sizeof(Slot48); i++) {
        if (slot->unk36 == -1) {
            return i;
        }
        slot++;
    }
    return -1;
}

INCLUDE_ASM("asm/nonmatchings/code_1902c", func_80028888);

INCLUDE_ASM("asm/nonmatchings/code_1902c", func_80028984);

INCLUDE_ASM("asm/nonmatchings/code_1902c", func_80028AE4);

INCLUDE_ASM("asm/nonmatchings/code_1902c", func_80028DBC);

INCLUDE_ASM("asm/nonmatchings/code_1902c", func_80028F0C);

INCLUDE_ASM("asm/nonmatchings/code_1902c", func_8002964C);

s32 func_8002971C(Body *a, Body *b) {
    VECTOR d;
    s32 dist;

    d.vx = (b->x - a->x) * (b->x - a->x);
    d.vy = (b->y - a->y) * (b->y - a->y);
    d.vz = (b->z - a->z) * (b->z - a->z);
    dist = d.vx + d.vy + d.vz;
    d.vx = (b->radius + a->radius) * (b->radius + a->radius);
    return dist < d.vx;
}

s32 func_800297A4(VECTOR *a, VECTOR *b) {
    VECTOR d;

    d.vx = (b->vx - a->vx) * (b->vx - a->vx);
    d.vy = (b->vy - a->vy) * (b->vy - a->vy);
    d.vz = (b->vz - a->vz) * (b->vz - a->vz);
    return d.vx + d.vy + d.vz;
}

void func_8002980C(void) {
    D_8009EB78[1] = 0;
    func_8002985C();
    func_8002988C();
}

void func_80029838(void) {
    s32 *p = (s32 *)D_8009EB78;

    if (p[0x3B8 / 4] == 0) {
        p[0x3BC / 4] = 0;
    }
}

void func_8002985C(void) {
    D_8009F0B0[0] = 0;
    D_8009F0B0[10] = 0;
    D_8009F0B0[20] = 0;
    func_80028984();
}

void func_8002988C(void) {
    D_8009EF30[0] = 0;
}

INCLUDE_RODATA("asm/nonmatchings/code_1902c", D_80010B34);

INCLUDE_RODATA("asm/nonmatchings/code_1902c", D_80010B4C);

INCLUDE_RODATA("asm/nonmatchings/code_1902c", D_80010B58);

INCLUDE_RODATA("asm/nonmatchings/code_1902c", D_80010B64);
