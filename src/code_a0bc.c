#include "common.h"
#include "code_a0bc.h"
#include "gte.h"

/** @brief A flat, unlit TMD triangle: a header, its colour and three
 *  vertex indices. */
typedef struct {
    u32 hdr;     /**< the TMD primitive header */
    CVECTOR rgb; /**< the colour, depth-cued per frame */
    u16 v0;      /**< first vertex index */
    u16 v1;      /**< second vertex index */
    u16 v2;      /**< third vertex index */
    u16 pad;     /**< padding */
} TmdF3;

/** @brief An eight-byte table entry: two halfwords and four bytes. */
typedef struct {
    u16 unk0; /**< a byte and a bit packed together */
    u16 unk2; /**< a CLUT-style packed position */
    u8 unk4;  /**< not yet known */
    u8 unk5;  /**< not yet known */
    u8 unk6;  /**< not yet known */
    u8 unk7;  /**< not yet known */
} Sprite8;

/* MATCHING: this unit's view of the table code_1a098 copies as halfwords. */
extern Sprite8 D_800DD0A0[];

/** @brief A primitive handler: one entry of the handler table. */
typedef PACKET *(*PrimFunc)();

/* The handler table, eight rows of eight handlers. */
extern PrimFunc D_800E48E8[8][8];

PACKET *func_8001B4BC();
PACKET *func_8001B9A4();
PACKET *func_8001C13C();
PACKET *func_8001C878();
PACKET *func_8001D39C();
PACKET *func_8001DAD4();
PACKET *func_8001E558();
PACKET *func_8001EE30();
PACKET *func_8001FBBC();
PACKET *func_8001FE5C();
PACKET *func_800201BC();
PACKET *func_80020520();
PACKET *func_8002097C();
PACKET *func_80020DD8(TmdF3 *prim, SVECTOR *vtx, POLY_F3 *pkt, s32 n, s32 shift, GsOT *ot);
PACKET *func_80020F24();
PACKET *func_800210B4();
PACKET *func_80021240();
PACKET *func_80021434();

/* Handlers code_11dc4 defines. */
PACKET *func_800215C4();
PACKET *func_800217A8();
PACKET *func_80021958();
PACKET *func_80021B88();
PACKET *func_80021D3C();
PACKET *func_80021F80();
PACKET *func_80022150();
PACKET *func_8002230C();

INCLUDE_ASM("asm/nonmatchings/code_a0bc", func_800198BC);

INCLUDE_ASM("asm/nonmatchings/code_a0bc", func_80019CD8);

INCLUDE_ASM("asm/nonmatchings/code_a0bc", func_80019F24);

INCLUDE_ASM("asm/nonmatchings/code_a0bc", func_8001A3D4);

INCLUDE_ASM("asm/nonmatchings/code_a0bc", func_8001A69C);

INCLUDE_ASM("asm/nonmatchings/code_a0bc", func_8001A950);

INCLUDE_ASM("asm/nonmatchings/code_a0bc", func_8001ACB4);

INCLUDE_ASM("asm/nonmatchings/code_a0bc", func_8001B004);

void func_8001B2F4(u16 id, u8 mode, s32 w, s32 h, s32 page, s32 u, s32 v, u16 clutX, s32 clutY) {
    D_800DD0A0[id].unk0 = (u8)page | (mode << 7);
    D_800DD0A0[id].unk2 = (clutY << 6) | (clutX >> 4);
    D_800DD0A0[id].unk4 = u;
    D_800DD0A0[id].unk5 = v;
    D_800DD0A0[id].unk6 = w;
    D_800DD0A0[id].unk7 = h;
}

INCLUDE_ASM("asm/nonmatchings/code_a0bc", func_8001B354);

INCLUDE_ASM("asm/nonmatchings/code_a0bc", func_8001B4BC);

INCLUDE_ASM("asm/nonmatchings/code_a0bc", func_8001B9A4);

INCLUDE_ASM("asm/nonmatchings/code_a0bc", func_8001C13C);

INCLUDE_ASM("asm/nonmatchings/code_a0bc", func_8001C878);

INCLUDE_ASM("asm/nonmatchings/code_a0bc", func_8001D39C);

INCLUDE_ASM("asm/nonmatchings/code_a0bc", func_8001DAD4);

INCLUDE_ASM("asm/nonmatchings/code_a0bc", func_8001E558);

INCLUDE_ASM("asm/nonmatchings/code_a0bc", func_8001EE30);

INCLUDE_ASM("asm/nonmatchings/code_a0bc", func_8001FBBC);

INCLUDE_ASM("asm/nonmatchings/code_a0bc", func_8001FE5C);

INCLUDE_ASM("asm/nonmatchings/code_a0bc", func_800201BC);

INCLUDE_ASM("asm/nonmatchings/code_a0bc", func_80020520);

INCLUDE_ASM("asm/nonmatchings/code_a0bc", func_8002097C);

void func_80020C14(void) {
    D_800E48E8[0][6] = func_80020DD8;
    D_800E48E8[0][7] = func_8001B4BC;
    D_800E48E8[1][6] = func_800210B4;
    D_800E48E8[1][7] = func_8001C13C;
    D_800E48E8[2][6] = func_80021434;
    D_800E48E8[2][7] = func_8001D39C;
    D_800E48E8[3][0] = func_80021F80;
    D_800E48E8[3][6] = func_800217A8;
    D_800E48E8[3][7] = func_8001E558;
    D_800E48E8[4][6] = func_80020F24;
    D_800E48E8[4][7] = func_8001B9A4;
    D_800E48E8[5][6] = func_80021240;
    D_800E48E8[5][7] = func_8001C878;
    D_800E48E8[6][6] = func_800215C4;
    D_800E48E8[6][7] = func_8001DAD4;
    D_800E48E8[7][0] = func_8002097C;
    D_800E48E8[7][6] = func_80021958;
    D_800E48E8[7][7] = func_8001EE30;
}

s32 func_80020CF8(s32 mode) {
    switch (mode) {
        case 0:
        case 1:
            D_800E48E8[3][0] = func_800201BC;
            D_800E48E8[7][0] = func_80020520;
            break;
        case 2:
            D_800E48E8[3][0] = func_80021F80;
            D_800E48E8[7][0] = func_8002097C;
            break;
        case 3:
            D_800E48E8[3][0] = func_8001FBBC;
            D_800E48E8[7][0] = func_8001FE5C;
            break;
        case 4:
            D_800E48E8[3][0] = func_80022150;
            D_800E48E8[7][0] = func_8002230C;
            break;
        case 5:
            D_800E48E8[3][0] = func_80021B88;
            D_800E48E8[7][0] = func_80021D3C;
            break;
    }
}

PACKET *func_80020DD8(TmdF3 *prim, SVECTOR *vtx, POLY_F3 *pkt, s32 n, s32 shift, GsOT *ot) {
    s32 v;
    s32 dp;
    s32 i;
    u32 *tag;

    for (i = 0; i < n; i++, prim++) {
        gte_ldv3(&vtx[prim->v0], &vtx[prim->v1], &vtx[prim->v2]);
        gte_rtpt();
        gte_stflg(&v);
        if (v & 0x7F85E000) {
            continue;
        }
        gte_nclip();
        gte_stopz(&v);
        if (v <= 0) {
            continue;
        }
        gte_stsxy3_f3(pkt);
        gte_avsz3();
        gte_stotz(&v);
        gte_stdp(&dp);
        gte_ldrgb(&prim->rgb);
        gte_lddp(dp);
        gte_dpcs();
        gte_strgb(&pkt->r0);
        tag = (u32 *)ot->org + (v >> shift);
        *(u32 *)pkt = (*tag & 0xFFFFFF) | 0x04000000;
        *tag = (u32)pkt & 0xFFFFFF;
        pkt++;
    }
    return (PACKET *)pkt;
}

INCLUDE_ASM("asm/nonmatchings/code_a0bc", func_80020F24);

INCLUDE_ASM("asm/nonmatchings/code_a0bc", func_800210B4);

INCLUDE_ASM("asm/nonmatchings/code_a0bc", func_80021240);

INCLUDE_ASM("asm/nonmatchings/code_a0bc", func_80021434);
