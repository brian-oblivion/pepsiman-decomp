#include "common.h"
#include "libgte.h"
#include "libgpu.h"
#include "libgs.h"
#include "code_a0bc.h"
#include "code_13068.h"
#include "code_1a098.h"
#include "code_7d74.h"
#include "rand.h"

/** @brief The game-wide state record, as far as this unit reads it. The
 *         shared header declares it as a byte array; the rest of the layout
 *         is still unknown. */
typedef struct {
    u8 unk0; /**< set to 1 on a reset; not yet known */
    u8 unk1; /**< the steering mode last applied; a repeat only resets unk380 */
    u8 unk2; /**< cleared on a reset when the flag byte is 1 */
    u8 unk3; /**< nonzero steps two animation tracks instead of one */
    u8 unk4; /**< the argument of the single-track step */
    u8 unk5; /**< cleared on a reset */
    u8 unk6; /**< 2 on a reset, 0x33 when unk3D0's low nibble is 3 */
    u8 unk7; /**< 0x13 or 0xFF; not yet known */
    u8 unk8; /**< set to 0xFF; not yet known */
    u8 unk9; /**< set to 0xFF; not yet known */
    u8 padA[0x340 - 0xA];
    s8 unk340; /**< 2 lets the band in unk3AC pick the next mode */
    s8 unk341; /**< the second animation track's step result */
    u8 pad342[0x344 - 0x342];
    s32 unk344; /**< (40 - unk3A8) / 3 + 1 */
    s32 unk348; /**< pushed back along the sine of an angle */
    s32 unk34C; /**< raised to a cap: unk3C0, or a global one when unk3B8 is 1 */
    s32 unk350; /**< pushed back along the cosine of an angle */
    s32 unk354; /**< added to unk348 for a drawn position */
    s32 unk358; /**< added to unk34C for the drawn position */
    s32 unk35C; /**< added to unk350 for a drawn position */
    s16 unk360; /**< an angle copied to the camera rotation's x */
    s16 unk362; /**< an angle added to unk380 for the camera rotation's y */
    u8 pad364[0x368 - 0x364];
    s32 unk368; /**< cleared; not yet known */
    u8 pad36C[0x370 - 0x36C];
    s32 unk370; /**< cleared; not yet known */
    s16 unk374; /**< cleared when unk34C passes the goal line */
    s16 unk376; /**< a step counter of the current mode */
    s16 unk378; /**< a second step counter of the current mode */
    s16 unk37A; /**< cleared with unk378 */
    u8 pad37C[0x380 - 0x37C];
    s32 unk380; /**< an angle that follows the camera's yaw in bounded steps */
    s32 unk384; /**< decays towards 0 by one a step */
    u8 pad388[0x38C - 0x388];
    s16 unk38C; /**< cleared whenever unk38E changes */
    u8 unk38E;  /**< cleared on a reset; a small state (0, 1, 2) */
    u8 unk38F;  /**< 1 also gates a check on unk398 */
    u16 unk390; /**< set to 2 together with clearing unk398 */
    s16 unk392; /**< counts the steps while unk390 is 1 */
    s32 unk394; /**< takes unk34C when unk390 goes to 1 */
    s16 unk398; /**< cleared together with setting unk390 */
    u8 pad39A[0x39C - 0x39A];
    s32 unk39C; /**< with unk3CC, picks which cap unk34C gets */
    s32 unk3A0; /**< 0, 1 or 2, set as unk390 goes to 2 */
    s16 unk3A4; /**< a countdown set to 1 by some unk3A8 ranges */
    s16 unk3A6; /**< raised by 2 when unk34C passes the goal line */
    s16 unk3A8; /**< a count that unk3AC grades in three bands */
    s16 unk3AA; /**< unk3A8 saved when a mode starts */
    u16 unk3AC; /**< 0 below 13 in unk3A8, 1 below 26, else 2 */
    s16 unk3AE; /**< unk3A8 along the sine of the heading */
    s16 unk3B0; /**< unk3A8 along the cosine of the heading */
    u8 pad3B2[0x3B4 - 0x3B2];
    s32 unk3B4; /**< moved by 50 a step while steering */
    s32 unk3B8; /**< 1 selects the global cap for unk34C */
    s32 unk3BC; /**< 1 with unk3B8 also starts the unk390 sequence */
    s32 unk3C0; /**< the usual cap for unk34C */
    u8 pad3C4[0x3C8 - 0x3C4];
    s16 unk3C8; /**< set to 60 on a reset when the flag byte is 1 */
    s16 unk3CA; /**< set to 1 when the stage ends */
    s32 unk3CC; /**< with unk39C, picks which cap unk34C gets */
    u8 unk3D0;  /**< low nibble read on a reset */
    u8 unk3D1;  /**< set to 1 when a stage ends with a mode in unk3D0 */
    u8 unk3D2;  /**< set to 1 by one mode */
    s8 unk3D3;  /**< nonzero draws the gauge sprite */
    u8 unk3D4;  /**< 1 while mode 0xF runs */
    u8 unk3D5;  /**< set to 1 by some pickups */
} GameState;

/* MATCHING: a struct lvalue keeps the base in a register; array offsets fold into %lo. */
#define sGame (*(GameState *)D_8009EB78)

/* MATCHING: retail reaches these through a split lui/%lo pair, so each is
 * an array of unknown size here. */
extern s32 D_800D8440[];
extern u8 D_800D8960[];
extern s16 D_800D38DE[];
extern s8 D_8009EF4D[];

extern s32 D_800AC860;
extern s16 D_800959B2;
/* MATCHING: cc1 splits this load (its lui sits in a branch delay slot, away
 * from the lw), so it is an array here, though both halves use one register. */
extern s32 D_8009EF44[];

/** @brief A point of a 21-by-16 grid spanning the 320x240 screen, 16
 *         pixels apart, stored row by row. */
typedef struct {
    s32 x; /**< column offset from the screen centre */
    s32 y; /**< row offset from the screen centre */
    s32 z; /**< always 0 */
} GridPoint;

extern s32 D_800959A8;
extern GridPoint D_800DE5E0[];
extern DVECTOR D_800DE0A0[];
extern s32 D_800DED50[];
extern s32 D_800957F8;
extern s32 D_80095800;
extern s32 D_80095804;
extern s32 D_8009580C;

extern u8 *D_80095790;

/** @brief The three flat lights of the scene: one overhead-front, two behind to the sides. */
typedef struct {
    GsF_LIGHT l[3]; /**< one per light index */
} LightTable;

/* MATCHING: a struct lvalue keeps the table's base in a register. */
#define sLights (*(LightTable *)D_800DD070)
extern s32 D_8009EEF8[];

/** @brief A 72-byte record of the shared slot table, as the item
 *         spawner fills it; code_1902c's Slot48 is the same record. */
typedef struct {
    s32 unk0; /**< set to minus the view offset's x when a stage object is placed */
    s32 unk4; /**< cleared, or 0x3C or 0x32, when a stage object is placed */
    s32 unk8; /**< set to minus the view offset's z when a stage object is placed */
    u8 padC[0x18 - 0xC];
    s16 unk18; /**< -0x88, or turned by half of unk3A8 degrees */
    s16 unk1A; /**< 0xAA when a stage object is placed */
    s16 unk1C; /**< -0x88 for one stage object */
    u8 pad1E[0x24 - 0x1E];
    s16 unk24; /**< cleared on a spawn */
    s16 unk26; /**< cleared on a spawn */
    u8 pad28[0x34 - 0x28];
    s16 unk34; /**< 0xBA on a spawn */
    s16 unk36; /**< -1 when the record is free, else its kind */
    u8 pad38[0x42 - 0x38];
    u8 unk42; /**< 0xFF on a spawn */
    u8 unk43; /**< 0x69 on a spawn */
    u8 pad44[0x48 - 0x44];
} SlotRec;

/** @brief A 0x78-byte model slot, paired with the SlotRec of the same
 *         index; main's ModelSlot is the same record. */
typedef struct {
    u8 pad0[0x58];
    s32 unk58; /**< the address of the coordinate system the model hangs from */
    u8 pad5C[0x78 - 0x5C];
} ObjRec;

extern SlotRec D_800D3860[];
/* MATCHING: main declares this as its ModelSlot; this unit's view names the word at 0x58. */
extern ObjRec D_800D8370[];
/* MATCHING: code_1a098 types these Rec78 and Rec48; this unit's records are views of them. */
void func_8002A5B0(Rec78 *rec, Rec48 *r);

/* MATCHING: retail reaches these through a split lui/%lo pair. */
extern u8 D_8009EF49[];
extern s32 D_800D85A8[];
extern s32 D_800D83C8[];
extern s32 D_800D8530[];
extern s32 D_800D8698[];
extern s32 D_800D84B8[];
extern s32 D_800D8620[];
extern u8 D_800D3358[];
extern s16 D_800D3896[];
extern s16 D_800D3926[];
extern s16 D_800D396E[];
extern s16 D_800D39B6[];
extern s16 D_800D39FE[];
extern s16 D_800D3A46[];

void func_80023834(u8 mode, u16 a, s16 b);
void func_80023764(void);
s32 func_80023BFC(void);
s32 func_80023D68(void);
s32 func_80026548(void);
s32 func_8002670C(void);
void func_80026848(void);
s32 func_80027714(void);
void func_800278B0(void);
void func_80027A00(void);
void func_80027BEC(void);
void func_80027D04(void);
void func_80028008(void);
s32 func_800283A0(void);
void func_800285C8(s32 deg, s32 radius, VECTOR *out);
void func_800287F4(void);
s32 func_80028508(void);
/* MATCHING: code_1dc24 stores it as u8; this unit's tests load it lb. */
extern s8 D_80095962;

/** @brief A 10-byte pickup slot; three live ones are checked each frame. */
typedef struct {
    s8 unk0; /**< nonzero when the slot is live */
    u8 pad1;
    u8 unk2; /**< the pickup's kind, a row of the PickupKind table */
    u8 pad3[7];
} Pickup;

/* MATCHING: common.h declares the table as bytes; this unit reads its records. */
#define sPickups ((Pickup *)D_8009F0B0)

/** @brief Eight bytes of a pickup kind's effects. */
typedef struct {
    u8 b[8]; /**< per stage mode and flag */
} PickupKind;

extern PickupKind D_8007714C[];
extern s8 D_8009575F;
/* Overlay entry points: the stage modes' own code. */
void func_800F8A58(void);
void func_800F8B60(void);
void func_800F6FA0(void);
void func_800F70A8(void);
void func_800F8D64(void);
void func_800F729C(void);
void func_80023B20(void);
void func_80028650(void);
void func_80028500(void);

/* MATCHING: code_29f54 defines x..n as s16; this unit's calls pass them
 * unextended, so its prototype takes s32. */
s32 func_8003F834(s32 id, s32 x, s32 y, s32 z, s32 n);
void func_80015450(u16 *table, s32 index);
extern s32 D_800957B4;
extern u16 D_8009587E;
extern s16 D_800957B0;
extern s32 D_800956E4;
extern s32 D_800956E0;
extern s32 D_800DF5A0[];
extern s32 D_800DF5B0[];

/** @brief Three words moved as one, for a block copy. */
typedef struct {
    s32 w[3]; /**< the words */
} Words3;

extern s32 D_800DF5C0[];
/* MATCHING: a per-unit view; code_1a098 declares it the same way, code_1dc24
 * as words. */
extern Rec5C D_800CF080[];
extern u8 D_800959B0;
extern s16 D_800957DC;
void func_8003AFAC(void);
/* MATCHING: code_308ec defines this with a u8 parameter; this unit's call passes none. */
void func_8004079C();
void func_8003E360(s32 level);
/* MATCHING: code_308ec stores this as u8; this unit's test loads it lb. */
extern s8 D_80095900;
s32 func_800297A4(VECTOR *a, VECTOR *b);
/* MATCHING: code_7d74 defines this as an empty void(void); this unit's call
 * passes the state block in $a0. */
void func_80018CA4(GameState *g);

void func_80022868(void) {
    GsCOORDINATE2 coord;
    MATRIX ls;
    SVECTOR size;
    s32 unused[2];
    CVECTOR color;
    s32 d;

    if (sGame.unk3D3 != 0) {
        GsInitCoordinate2(WORLD, &coord);
        coord.coord.t[0] = sGame.unk348 + sGame.unk354 + D_800A7308[0];
        coord.coord.t[1] = D_800AC858[0];
        coord.coord.t[2] = sGame.unk350 + sGame.unk35C + D_800A7308[2];
        GsGetLs(&coord, &ls);
        GsSetLsMatrix(&ls);
        d = (sGame.unk3C0 - sGame.unk34C) / 10;
        color.r = 2;
        size.vx = 100 - d;
        size.vy = size.vx / 2;
        color.g = color.b = color.cd = 0x40 - d;
        func_8001A3D4(0x12D, &size, &color, 2, &D_800ACEA8[D_80095750]);
    }
}

void func_800229A8(void) {
    for (D_800958D0 = 0; D_800958D0 < 16; D_800958D0++) {
        for (D_800958CC = 0; D_800958CC < 21; D_800958CC++) {
            D_800DE5E0[D_800958D0 * 21 + D_800958CC].x = D_800958CC * 16 - 160;
            D_800DE5E0[D_800958D0 * 21 + D_800958CC].y = D_800958D0 * 16 - 120;
            D_800DE5E0[D_800958D0 * 21 + D_800958CC].z = 0;
        }
    }
    D_800DB2A0[0] = 0;
    D_800959A8 = 0;
    D_800DB2A0[1] = 0;
    D_800DB2A0[2] = 1000;
    D_800DB2A0[3] = 0;
    D_800DB2A0[4] = 0;
    D_800DB2A0[5] = 0;
}

/* MATCHING: the parity mask through y, the first loop's local, keeps 0xFF out of the preheader. */
void func_80022A74(void) {
    DVECTOR sxy;
    SVECTOR v;
    MATRIX ls;
    GsCOORDINATE2 coord;
    GsLINE line;
    POLY_F4 poly;
    long p;
    long flag;
    s32 x;
    s32 y;
    POLY_F4 *pp;

    if (D_80095970 & 0x10) {
        D_800959A8 = 0;
    }
    if (D_80095964 & 4) {
        D_800959A8 += 2;
    }
    if (D_80095964 & 8) {
        D_800959A8 -= 2;
    }
    if (D_80095970 & 0x800) {
        D_80095880 = 3;
    }
    if (D_80095964 & 0x8000) {
        D_800DB2A0[0]++;
    }
    if (D_80095964 & 0x2000) {
        D_800DB2A0[0]--;
    }
    if (D_80095964 & 0x1000) {
        D_800DB2A0[1]++;
    }
    if (D_80095964 & 0x4000) {
        D_800DB2A0[1]--;
    }
    line.attribute = 0;
    line.b = line.g = line.r = 0xFF;
    for (D_800958D0 = 0; D_800958D0 < 16; D_800958D0++) {
        for (D_800958CC = 0; D_800958CC < 21; D_800958CC++) {
            x = D_800DE5E0[D_800958D0 * 21 + D_800958CC].x;
            y = D_800DE5E0[D_800958D0 * 21 + D_800958CC].y;
            GsInitCoordinate2(WORLD, &coord);
            coord.coord.t[2] = 0;
            coord.flg = 0;
            coord.coord.t[0] = x;
            coord.coord.t[1] = y;
            GsGetLs(&coord, &ls);
            GsSetLsMatrix(&ls);
            v.vx = v.vy = v.vz = 0;
            D_800DE5E0[D_800958D0 * 21 + D_800958CC].z = RotTransPers(&v, (long *)&sxy, &p, &flag);
            D_800DE0A0[D_800958D0 * 21 + D_800958CC].vx = line.x0 = line.x1 = sxy.vx;
            D_800DE0A0[D_800958D0 * 21 + D_800958CC].vy = line.y0 = line.y1 = sxy.vy;
            GsSortLine(&line, &D_800A7318[D_80095750], 0);
        }
    }
    pp = &poly;
    for (D_800958D0 = 0; D_800958D0 < 15; D_800958D0++) {
        for (D_800958CC = 0; D_800958CC < 20; D_800958CC++) {
            SetPolyF4(pp);
            pp->x0 = D_800DE0A0[D_800958D0 * 21 + D_800958CC].vx;
            pp->y0 = D_800DE0A0[D_800958D0 * 21 + D_800958CC].vy;
            pp->x1 = D_800DE0A0[D_800958D0 * 21 + (D_800958CC + 1)].vx;
            pp->y1 = D_800DE0A0[D_800958D0 * 21 + (D_800958CC + 1)].vy;
            pp->x3 = D_800DE0A0[(D_800958D0 + 1) * 21 + (D_800958CC + 1)].vx;
            pp->y3 = D_800DE0A0[(D_800958D0 + 1) * 21 + (D_800958CC + 1)].vy;
            pp->x2 = D_800DE0A0[(D_800958D0 + 1) * 21 + D_800958CC].vx;
            pp->y2 = D_800DE0A0[(D_800958D0 + 1) * 21 + D_800958CC].vy;
            y = 1;
            if (D_800958D0 & y) {
                if (D_800958CC & y) {
                    pp->r0 = 0x70;
                    pp->g0 = 0x70;
                    pp->b0 = 0x80;
                } else {
                    pp->r0 = 0xFF;
                    pp->g0 = 0xFF;
                    pp->b0 = 0xFF;
                }
            } else if (D_800958CC & 1) {
                pp->r0 = 0xFF;
                pp->g0 = 0xFF;
                pp->b0 = 0xFF;
            } else {
                pp->r0 = 0x70;
                pp->g0 = 0x70;
                pp->b0 = 0x80;
            }
            GsSortPoly(pp, &D_800A7318[D_80095750], D_800DE5E0[D_800958D0 * 21 + D_800958CC].z >> 6);
        }
    }
    D_800957F8 = D_800959A8;
    D_80095800 = D_800DED50[0];
    D_80095804 = D_800DE0A0[0].vx;
    D_8009580C = D_800DE0A0[0].vy;
}

void func_80022F58(void) {}

void func_80022F60(void) {}

void func_80022F68(VECTOR *pos) {
    func_8003F834(4, (s16)pos->vx + (rand() % 160 - 80), (s16)pos->vy,
                  (s16)pos->vz + (rand() % 160 - 80), 1);
}

void func_80023020(VECTOR *pos) {
    func_8003F834(6, (s16)pos->vx + (rand() % 160 - 80), (s16)pos->vy,
                  (s16)pos->vz + (rand() % 160 - 80), 1);
}

void func_800230D8(void) {}

/* MATCHING: the unused 88 bytes put p and flag at sp+0xE8 and the frame at 0x100. */
void func_800230E0(VECTOR *pos, SVECTOR *out) {
    DVECTOR sxy;
    SVECTOR v;
    MATRIX ls;
    GsCOORDINATE2 coord;
    s32 unused[22];
    long p;
    long flag;

    GsInitCoordinate2(WORLD, &coord);
    coord.coord.t[0] = pos->vx;
    coord.coord.t[1] = pos->vy;
    coord.coord.t[2] = pos->vz;
    coord.flg = 0;
    GsGetLs(&coord, &ls);
    GsSetLsMatrix(&ls);
    v.vz = 0;
    v.vy = 0;
    v.vx = 0;
    out->vz = RotTransPers(&v, (long *)&sxy, &p, &flag);
    out->vx = sxy.vx;
    out->vy = sxy.vy;
}

/* MATCHING: the unused pair puts flag at sp+0x70 and the frame at 0x88. */
void func_80023194(GsCOORDINATE2 *coord, SVECTOR *pos, VECTOR *out) {
    MATRIX world;
    MATRIX local;
    SVECTOR v;
    VECTOR t;
    s32 unused[2];
    long flag;

    v.vx = pos->vx;
    v.vy = pos->vy;
    v.vz = pos->vz;
    GsGetLws(coord, &local, &world);
    GsSetLsMatrix(&local);
    RotTrans(&v, &t, &flag);
    out->vx = t.vx;
    out->vy = t.vy;
    out->vz = t.vz;
    GsSetLsMatrix(&world);
}

/* MATCHING: g keeps the sGame base in a register for the clamp's store at the join;
 * cases 1 and 2 of the second switch are separate arms, merged by cross-jumping. */
void func_80023228(void) {
    s32 t;
    s32 x;
    s32 v;
    s32 hi;
    GameState *g;

    if (D_80095962 != 1) {
        func_80023BFC();
    }
    func_800283A0();
    func_80023D68();
    if (sGame.unk0 == 1) {
        func_80027714();
    }
    func_800287F4();
    func_800278B0();
    switch (sGame.unk3D0 & 0xF) {
        case 0:
            func_80026548();
            func_8002670C();
            func_80026848();
            func_80028508();
            break;
        case 1:
            func_800F8A58();
            func_800F8B60();
            break;
        case 4:
            func_800F6FA0();
            func_800F70A8();
            break;
        case 2:
            func_80027BEC();
            break;
        case 3:
            func_80027D04();
            break;
    }
    sGame.unk344 = (40 - sGame.unk3A8) / 3 + 1;
    t = sGame.unk3A8;
    switch (((u32)D_80095864 >> 22) & 3) {
        case 1:
            if (t != 0) {
                t = 70;
            }
            break;
        case 2:
            if (t != 0) {
                t = 90;
            }
            break;
        case 3:
            if (sGame.unk34C >= sGame.unk3C0) {
                func_800285C8(D_800A7680->vy * 360 / 4096 - 180, 30, (VECTOR *)D_800D39C8);
                sGame.unk348 += D_800D39C8[0];
                sGame.unk350 += D_800D39C8[2];
            }
            break;
    }
    switch (((u32)D_80095864 >> 20) & 3) {
        case 1:
            if (D_8009EF20[0] != 0) {
                t = 30;
            }
            break;
        case 2:
            if (D_8009EF20[0] != 0) {
                t = 30;
            }
            break;
        case 3:
            if (sGame.unk34C >= sGame.unk3C0) {
                func_800285C8(D_800A7680->vy * 360 / 4096, 30, (VECTOR *)D_800D39C8);
                sGame.unk348 += D_800D39C8[0];
                sGame.unk350 += D_800D39C8[2];
            }
            break;
    }
    D_800957D2 = t << 4;
    switch (D_8009EF48[0] & 0xF) {
        case 0:
        case 1:
        case 3:
        case 4:
            sGame.unk3AE = rsin(D_800A7680->vy) * sGame.unk3A8 >> 12;
            sGame.unk3B0 = rcos(D_800A7680->vy) * sGame.unk3A8 >> 12;
            sGame.unk348 -= sGame.unk3AE;
            sGame.unk350 -= sGame.unk3B0;
            break;
        case 2:
            sGame.unk3AE = rsin(sGame.unk380) * sGame.unk3A8 >> 12;
            sGame.unk3B0 = rcos(sGame.unk380) * sGame.unk3A8 >> 12;
            sGame.unk348 -= sGame.unk3AE;
            sGame.unk350 -= sGame.unk3B0;
            break;
    }
    switch (D_8009EF48[0] & 0xF) {
        case 0:
            func_80023764();
            break;
        case 1:
            func_800F8D64();
            break;
        case 4:
            func_800F729C();
            break;
        case 2:
            func_80027A00();
            break;
        case 3:
            func_80028008();
            break;
    }
    g = &sGame;
    x = g->unk380;
    v = D_800A7680->vy - 0x155;
    if (x >= v) {
        hi = D_800A7680->vy + 0x155;
        if (x <= hi) {
            v = x;
        } else {
            v = hi;
        }
    }
    g->unk380 = v;
    D_8009EAB8[1] = sGame.unk380;
    D_8009EAB8[2] = sGame.unk384;
    D_800D86E0[0].coord.t[0] = sGame.unk354;
    D_800D86E0[0].coord.t[1] = sGame.unk34C + sGame.unk358;
    D_800D86E0[0].coord.t[2] = sGame.unk35C;
    func_80018AE0((SVECTOR *)D_8009EAB8, D_800D86E0);
    D_800A7308[1] = 0;
    D_800A7308[0] = -sGame.unk348;
    D_800A7308[2] = -sGame.unk350;
}

void func_80023764(void) {
    if (sGame.unk0 != 0) {
        switch (D_80095784) {
            case 0x73:
                if (D_80095964 & 0x8000) {
                    func_80023834(3, 16, 15);
                } else if (D_80095964 & 0x2000) {
                    func_80023834(2, 16, 15);
                } else {
                    func_80023B20();
                }
                break;
            case 0x41:
                if (D_80095964 & 0x8000) {
                    func_80023834(3, 16, 15);
                } else if (D_80095964 & 0x2000) {
                    func_80023834(2, 16, 15);
                } else {
                    func_80023B20();
                }
                break;
            default:
                func_80023B20();
                break;
        }
        if (sGame.unk0 != 0) {
            return;
        }
    }
    D_8009EEF8[0] = D_800A7680[0].vy;
}

static __inline__ void pushBack(s32 deg, s32 dist) {
    s32 angle;
    SVECTOR *rot;

    rot = D_800A7680;
    angle = ANGLE_DEG(deg);
    sGame.unk348 -= rsin(rot->vy + angle) * dist >> FIX12_SHIFT;
    sGame.unk350 -= rcos(rot->vy + angle) * dist >> FIX12_SHIFT;
}

/* MATCHING: the (u16) read is the separate lhu retail stores when the heading is in range. */
void func_80023834(u8 mode, u16 a, s16 b) {
    s16 yaw;
    s32 w;

    if (sGame.unk1 != mode) {
        switch (mode) {
            case 3:
                yaw = D_800A7680->vy;
                if (yaw - 0x2D < sGame.unk380) {
                    sGame.unk380 = yaw - 0x2D;
                }
                D_800957BC -= a * 2;
                D_800957B0 -= a;
                sGame.unk380 = D_800957BC;
                if ((sGame.unk3D0 & 0xF) != 2 && yaw < sGame.unk380) {
                    sGame.unk380 = yaw;
                }
                if (sGame.unk384 < 0) {
                    sGame.unk384 += 0x22;
                }
                if (sGame.unk384 < 16) {
                    w = sGame.unk384;
                } else {
                    w = 15;
                }
                sGame.unk384 = w;
                sGame.unk3B4 -= 50;
                pushBack(-90, b);
                break;
            case 2:
                yaw = D_800A7680->vy;
                if (sGame.unk380 < yaw + 0x4000) {
                    sGame.unk380 = yaw + 0x2D;
                }
                D_800957BC += a * 2;
                D_800957B0 += a;
                sGame.unk380 = D_800957BC;
                if ((sGame.unk3D0 & 0xF) != 2 && sGame.unk380 < yaw) {
                    sGame.unk380 = yaw;
                }
                if (sGame.unk384 > 0) {
                    sGame.unk384 -= 0x22;
                }
                if (sGame.unk384 >= -15) {
                    w = sGame.unk384;
                } else {
                    w = -15;
                }
                sGame.unk384 = w;
                sGame.unk3B4 += 50;
                pushBack(90, b);
                break;
        }
        D_800957BC = D_800957BC < D_800A7680->vy - 0x100   ? D_800A7680->vy - 0x100
                     : D_800A7680->vy + 0x100 < D_800957BC ? D_800A7680->vy + 0x100
                                                           : (u16)D_800957BC;
        if (sGame.unk3B8 == 1 && sGame.unk3BC == 1 && sGame.unk38F == 0) {
            sGame.unk34C = sGame.unk3C0;
        }
    } else {
        sGame.unk380 = D_800A7680->vy;
    }
}

/* MATCHING: the inline ternary abs (not an if on d) lets the first +56 test reuse the
 * loaded value when it skips the store. */
void func_80023B20(void) {
    u16 i;
    s32 yaw;
    s32 d;

    for (i = 0; i < 3; i++) {
        if (sGame.unk384 < 0) {
            sGame.unk384++;
        }
        if (sGame.unk384 > 0) {
            sGame.unk384--;
        }
        yaw = D_800A7680->vy;
        d = sGame.unk380 - yaw;
        if ((d < 0 ? -d : d) > 56) {
            if (sGame.unk380 < yaw) {
                sGame.unk380 += 56;
            }
            if (sGame.unk380 > yaw) {
                sGame.unk380 -= 56;
            }
        } else {
            if (sGame.unk380 < yaw) {
                sGame.unk380 += 11;
            }
            if (sGame.unk380 > yaw) {
                sGame.unk380 -= 11;
            }
        }
        D_800957BC = sGame.unk380;
    }
}

/* MATCHING: non-void with no return keeps the last beqz delay slot a nop. */
s32 func_80023BFC(void) {
    s32 unused[2];

    if ((D_800AC860 & 0x40) && sGame.unk6 != 0xE) {
        if (sGame.unk3D0 & 0xF) {
            sGame.unk3CA = 1;
            if ((sGame.unk3D0 & 0xF) != 3) {
                sGame.unk398 = 20;
                sGame.unk3D0 &= 0xF0;
            }
            func_800287C0();
            sGame.unk0 = 0;
        }
        sGame.unk3C0 = 30000;
        if (sGame.unk34C >= D_800AC858[0] + 100) {
            if (D_80095858 == 0) {
                D_80095858 = 2;
            }
            D_800958EC = 1;
            sGame.unk0 = 0;
            sGame.unk3A6 += 2;
            func_800287C0();
            sGame.unk374 = 0;
            if (sGame.unk34C > D_800AC858[0] + 0x8C && (sGame.unk3D0 & 0xF) == 3) {
                sGame.unk3D0 &= 0xF0;
            }
            sGame.unk3CA = 1;
            if (sGame.unk34C > D_800AC858[0] + 1000) {
                sGame.unk6 = 12;
                sGame.unk3A6 = 0;
                sGame.unk3D0 &= 0xF0;
            }
        }
    }
}

#ifdef NON_MATCHING
/* MATCHING: 129/134; retail copies unk34C to a second register ($a0) for the unk39C and
 * closeness tests and keeps the first ($a2) for the step. */
s32 func_80023D68(void) {
    s32 x;
    s32 d;
    s32 y;

    x = sGame.unk34C;
    if (x < D_800956E4) {
        D_800956E4 = x;
    }
    if (x < sGame.unk39C) {
        sGame.unk39C = x;
    }
    if ((u32)(sGame.unk6 - 8) < 4) {
        goto end;
    }
    if ((u32)(sGame.unk6 - 0x38) < 2 && (sGame.unk3D0 & 0xF) != 3) {
        goto end;
    }
    d = sGame.unk3C0 - x;
    if ((d >= 0 ? d : -d) < 50) {
        sGame.unk34C = sGame.unk3C0;
    } else {
        sGame.unk34C = x + (u16)sGame.unk3A6 * 5;
    }
    if (sGame.unk3B8 == 1 && sGame.unk3BC == 1) {
        sGame.unk3A6 = 0;
        sGame.unk39C = 0;
        sGame.unk34C = sGame.unk3CC;
        goto end;
    }
    if (sGame.unk39C <= sGame.unk3CC && sGame.unk3B8 == 1) {
        if (sGame.unk34C >= sGame.unk3CC) {
            sGame.unk34C = sGame.unk3CC;
            sGame.unk3A6 = 0;
            sGame.unk39C = 0;
            sGame.unk3BC = sGame.unk3B8;
        } else {
            sGame.unk3A6++;
        }
        goto end;
    }
    if (sGame.unk34C >= sGame.unk3C0) {
        y = D_800AC858[0];
        sGame.unk3A6 = 0;
        sGame.unk39C = 0;
        sGame.unk3BC = 0;
        sGame.unk34C = y;
        if (D_800956E0 == 0) {
            D_800DF5B0[0] = sGame.unk348;
            D_800DF5B0[1] = y;
            D_800DF5B0[2] = sGame.unk350;
            D_800956E0 = SquareRoot0(func_800297A4((VECTOR *)D_800DF5A0, (VECTOR *)D_800DF5B0));
            if (D_80095900 == 0) {
                func_80015450(D_800734AC, 13);
            }
        }
    } else {
        sGame.unk3A6++;
    }
end:;
}
#else
INCLUDE_ASM("asm/nonmatchings/code_13068", func_80023D68);
#endif

/* MATCHING: the volatile read keeps the later unk2 reads as reloads, as retail. */
void func_80023F80(u8 *state) {
    u8 v;
    s16 i;
    u8 m;
    Pickup *p;

    v = 0;
    if (((GameState *)state)->unk2 == 0) {
        return;
    }
    if (D_80095962 == 1) {
        return;
    }
    for (i = 0; i < 3; i++) {
        p = &sPickups[i];
        if (p->unk0 == 0) {
            continue;
        }
        D_8009599C = 0;
        if ((u32)(*(volatile u8 *)&p->unk2 - 15) >= 4) {
            m = ((GameState *)state)->unk6;
            switch (((GameState *)state)->unk5) {
                case 0:
                case 0x5E:
                    switch (m) {
                        case 0x33:
                            if ((sGame.unk3D0 & 0xF) != 3) {
                                break;
                            }
                        case 2:
                        case 3:
                        case 4:
                            v = D_8007714C[sPickups[i].unk2].b[0];
                            break;
                        case 8:
                        case 9:
                        case 10:
                        case 11:
                            v = D_8007714C[sPickups[i].unk2].b[1];
                            break;
                        case 5:
                        case 6:
                        case 7:
                            v = D_8007714C[sPickups[i].unk2].b[2];
                            break;
                    }
                    break;
                case 0x39:
                case 0x3A:
                    v = D_8007714C[p->unk2].b[3];
                    break;
                case 0x3B:
                case 0x3C:
                    v = D_8007714C[p->unk2].b[0];
                    break;
                default:
                    switch (sGame.unk3D0 & 0xF) {
                        case 1:
                        case 2:
                        case 4:
                            v = 0x42;
                            break;
                        case 3:
                        default:
                            v = D_8007714C[sPickups[i].unk2].b[4];
                            break;
                    }
                    break;
            }
            switch (sGame.unk3D0 & 0xF) {
                case 1:
                case 2:
                case 4:
                    v = 0x42;
                    break;
            }
        } else {
            v = D_8007714C[p->unk2].b[2 - i];
            if (sGame.unk3D0 & 0xF) {
                v = 0x42;
            }
        }
        switch (sGame.unk3D0 & 0xF) {
            case 1:
            case 2:
            case 4:
                sGame.unk3D1 = 1;
                sGame.unk3D5 = 1;
                D_800958A8 = 0;
                sGame.unk3D0 &= 0xF0;
                ((GameState *)state)->unk2 = 0;
                D_8009599C = 1;
                ((GameState *)state)->unk3C8 = 750;
                ((GameState *)state)->unk5 = v;
                if (sPickups[i].unk2 != 0) {
                    func_800285B0();
                }
                D_8009575F = sPickups[0].unk0 | sPickups[1].unk0 | sPickups[2].unk0;
                return;
            case 0:
            default:
                if (v == 0) {
                    continue;
                }
                if (D_8007714C[sPickups[i].unk2].b[5] == 1) {
                    ((GameState *)state)->unk2 = 0;
                    ((GameState *)state)->unk3C8 = 150;
                    D_8009599C = 1;
                } else {
                    ((GameState *)state)->unk2 = 0;
                    ((GameState *)state)->unk3C8 = 18;
                }
                if (D_8007714C[sPickups[i].unk2].b[6] == 1) {
                    sGame.unk3D5 = 1;
                    D_800958A8 = 0;
                }
                if (D_8007714C[sPickups[i].unk2].b[7] == 1 &&
                    (s8)(sPickups[0].unk0 | sPickups[1].unk0 | sPickups[2].unk0) != D_8009575F) {
                    func_80028260(-1);
                }
                ((GameState *)state)->unk5 = v;
                if (sPickups[i].unk2 != 0) {
                    func_800285B0();
                }
                D_8009575F = sPickups[0].unk0 | sPickups[1].unk0 | sPickups[2].unk0;
                return;
        }
    }
    D_8009575F = 0;
}

/* MATCHING: code_7d74 types the object as its own view; this unit passes the state block. */
u8 func_80017F0C(GameState *obj, u16 index, u8 arg);
/* MATCHING: code_7d74 defines this with s16/u16 parameters and an s16 result; this unit's
 * calls pass halfwords unextended and store the result as a word. */
s32 func_80018D04(s32 from, s32 to, s32 step, s32 steps);
/* MATCHING: code_7d74 defines this as an empty void(void); this unit's calls pass the state
 * block and the step in $a0 and $a1. */
void func_80018CAC(GameState *g, s32 step);

s32 func_800281B8(GameState *g);
void func_800283E4(void);
extern s32 D_80095740;
extern s32 D_80095744;

#ifdef NON_MATCHING
/* MATCHING: s32 with no return keeps the delay slots before the exit nops; d is one pseudo for
 * every step so global-alloc gives it $a1, the step's argument register. */
s32 func_80024450(GameState *state) {
    u8 m;
    s16 t;
    s32 v;
    s8 r;
    s32 d;

    m = state->unk6;
    t = --sGame.unk3C8;
    if (t < 0) {
        sGame.unk3C8 = 0;
        sGame.unk2 = 1;
    } else {
        if (sPickups[0].unk0 | sPickups[1].unk0 | sPickups[2].unk0) {
            if (t == 1) {
                sGame.unk3C8 = 2;
            }
        }
        sGame.unk2 = 0;
    }
    if (state->unk3 == 0) {
        state->unk340 = func_80017F0C(state, 0, state->unk4);
    } else {
        state->unk340 = func_80017F0C(state, 0, 1);
        state->unk341 = func_80017F0C(state, 1, 2);
    }
top:
    switch (state->unk5) {
        case 0x7:
            state->unk5 = 8;
            state->unk6 = 0xD;
            state->unk0 = 0;
            func_80015450(D_800734AC, 1);
            break;
        case 0x9:
            state->unk5 = 0xA;
            state->unk6 = 0xD;
            state->unk0 = 0;
            state->unk376 = 0;
            state->unk374 = 0;
            func_80015450(D_800734AC, 2);
            break;
        case 0xF:
            state->unk5 = 0x10;
            state->unk4 = 0x80;
            break;
        case 0x11:
            state->unk5 = 0x12;
            state->unk374 = 0;
            state->unk368 = 0;
            state->unk370 = 0;
            break;
        case 0x12:
            if (state->unk374 < 60) {
                state->unk374++;
            }
            D_80095740 = func_80018D04(D_80095740, 0, state->unk374, 60);
            /* MATCHING: a byte-pointer read keeps the load below the store to D_80095740. */
            D_80095744 = func_80018D04(D_80095744, 0, *(s16 *)((u8 *)state + 0x374), 60);

            if (D_80095740 == 0) {
                state->unk4 = 0;
                state->unk5 = 0;
            }
            break;
        case 0x13:
            state->unk5 = 0x14;
            state->unk6 = 0x15;
            state->unk0 = 0;
            state->unk376 = 0;
            state->unk374 = 0;
            func_80015450(D_800734AC, 7);
            break;
        case 0x17:
            if ((u8)(m - 2) < 3) {
                state->unk5 = 0x18;
                state->unk6 = 0x10;
                state->unk374 = 0;
                state->unk0 = 0;
                state->unk3AA = state->unk3A8;
            }
            break;
        case 0x23:
            state->unk5 = 0x24;
            state->unk6 = 0xE;
            state->unk374 = 0;
            state->unk0 = 0;
            state->unk3AA = state->unk3A8;
            break;
        case 0x19:
            state->unk5 = 0x1A;
            state->unk0 = 0;
            state->unk374 = 0;
            if (state->unk3A8 == 0) {
                state->unk6 = 0xF;
            } else {
                state->unk6 = 0x10;
                state->unk374 = 0;
                state->unk3AA = state->unk3A8;
            }
            break;
        case 0x1B:
            state->unk5 = 0x1C;
            state->unk0 = 0;
            if (state->unk3A8 == 0) {
                state->unk340 = 1;
                state->unk374 = 0;
                state->unk6 = 0x11;
            } else {
                state->unk6 = 0x10;
                state->unk374 = 0;
                state->unk3AA = state->unk3A8;
            }
            break;
        case 0x1F:
            state->unk5 = 0x20;
            state->unk0 = 0;
            if (state->unk3A8 != 0) {
                state->unk6 = 0x10;
                state->unk374 = 0;
                state->unk3AA = state->unk3A8;
            } else {
                state->unk340 = 1;
                state->unk6 = 0x12;
            }
            break;
        case 0x27:
            func_80042538(0x3C);
            state->unk5 = 0x28;
            state->unk6 = 0x14;
            state->unk398 = 70;
            state->unk376 = 0;
            state->unk374 = 0;
            state->unk0 = 0;
            break;
        case 0x39:
            state->unk5 = 0x3A;
            state->unk6 = 0x1A;
            state->unk376 = 0;
            state->unk3AA = state->unk3A8;
            func_80015450(D_800734AC, 10);
        case 0x3A:
            if (state->unk376 < 5) {
                func_8003C014();
                D_800957D2 = 0x5A0;
            } else {
                D_800957D2 = 0x460;
            }
            state->unk376++;
            state->unk3A8 = (s16)D_800957D2 >> 4;
            func_80022F68((VECTOR *)&state->unk348);
            if (state->unk376 >= 30) {
                state->unk5 = 0;
                if (sGame.unk34C >= sGame.unk3C0 && (u32)(sGame.unk6 - 5) >= 3) {
                    state->unk6 = 4;
                }
            }
            break;
        case 0x3B:
            func_80042538(0x16);
            state->unk5 = 0x3C;
            state->unk0 = 0;
            state->unk6 = 0x18;
            state->unk378 = 0;
            state->unk376 = 0;
            state->unk374 = 0;
            state->unk3AA = state->unk3A8;
            func_80015450(D_800734AC, 12);
        case 0x3C:
            D_800957D2 -= 0x20;
            if (++state->unk378 >= 16 || (s16)D_800957D2 < 0) {
                D_800957D2 = 0;
                state->unk5 = 0x3D;
                state->unk6 = 0x19;
                state->unk0 = 1;
            }
            state->unk3A8 = (s16)D_800957D2 >> 4;
            func_80022F68((VECTOR *)&state->unk348);
            break;
        case 0x3D:
            if (state->unk340 == 1) {
                state->unk6 = 4;
                state->unk5 = 0;
                state->unk38E = 0;
                D_800957D2 = 0x320;
            }
            break;
        case 0x3E:
            state->unk5 = 0x3F;
            state->unk6 = 0x1B;
            state->unk398 = 70;
            state->unk376 = 0;
            state->unk374 = 0;
            state->unk0 = 0;
            func_80015450(D_800734AC, 2);
            break;
        case 0x40:
            state->unk5 = 0x41;
            state->unk6 = 0x1B;
            state->unk398 = 70;
            state->unk376 = 0;
            state->unk374 = 0;
            state->unk0 = 0;
            func_80015450(D_800734AC, 2);
            break;
        case 0x42:
            state->unk5 = 0x43;
            state->unk6 = 0x23;
            if (state->unk3A8 < 90) {
                state->unk3A8 = 70;
            }
            state->unk0 = 0;
            state->unk398 = state->unk3A8;
            /* MATCHING: *= -1 reloads the halfword with lh. */
            state->unk3A8 *= -1;
            func_80015450(D_800734AC, 4);
            break;
        case 0x44:
            state->unk5 = 0x45;
            state->unk6 = 0x23;
            if (state->unk3A8 < 90) {
                state->unk3A8 = 70;
            }
            state->unk0 = 0;
            state->unk398 = state->unk3A8;
            state->unk3A8 *= -1;
            func_80015450(D_800734AC, 4);
            break;
        case 0x46:
            state->unk5 = 0x47;
            state->unk6 = 0x28;
            func_800287C0();
            state->unk374 = 10;
            state->unk376 = 0;
            state->unk0 = 0;
            func_80015450(D_800734AC, 5);
            break;
        case 0x48:
            state->unk5 = 0x49;
            state->unk6 = 0x2C;
            state->unk376 = 0;
            state->unk374 = 0;
            state->unk0 = 0;
            if (func_80028260(0) <= 0) {
                state->unk5 = 0x42;
                goto top;
            }
            func_80015450(D_800734AC, 4);
            func_800287C0();
            break;
        case 0x4A:
            state->unk5 = 0x4B;
            state->unk6 = 0x2E;
            state->unk376 = 0;
            state->unk374 = 0;
            state->unk0 = 0;
            func_80015450(D_800734AC, 4);
            break;
        case 0x4C:
            state->unk5 = 0x4D;
            state->unk6 = 0x2F;
            func_800287C0();
            state->unk0 = 0;
            func_80015450(D_800734AC, 4);
            break;
        case 0x4E:
            state->unk5 = 0x4F;
            state->unk6 = 0x30;
            func_800287C0();
            state->unk37A = 0;
            state->unk378 = 0;
            state->unk376 = 0;
            state->unk374 = 0;
            state->unk0 = 0;
            func_80015450(D_800734AC, 8);
            break;
        case 0x50:
            if (rand() & 1) {
                state->unk5 = 0x51;
                state->unk6 = 0x30;
            } else {
                state->unk5 = 0x55;
                state->unk6 = 0x23;
            }
            func_800287C0();
            state->unk37A = 0;
            state->unk378 = 0;
            state->unk376 = 0;
            state->unk374 = 0;
            state->unk0 = 0;
            state->unk2 = 0;
            state->unk3C8 = 150;
            func_80015450(D_800734AC, 4);
            break;
        case 0x52:
            state->unk5 = 0x53;
            state->unk6 = 0x32;
            state->unk3D2 = 1;
            func_800287C0();
            state->unk37A = 0;
            state->unk378 = 0;
            state->unk376 = 0;
            state->unk374 = 0;
            state->unk0 = 0;
            func_80015450(D_800734AC, 9);
            break;
        case 0x56:
            D_80095858 = -1;
            state->unk5 = 0x57;
            state->unk6 = 0x23;
            func_800287C0();
            state->unk398 = 300;
            state->unk37A = 0;
            state->unk378 = 0;
            state->unk376 = 0;
            state->unk374 = 0;
            state->unk0 = 0;
            func_80015450(D_800734AC, 4);
            break;
        case 0x33:
            switch (state->unk6) {
                case 0x33:
                    break;
                case 0x36:
                    if (state->unk340 == 0) {
                        state->unk6 = 0x37;
                    }
                    break;
                case 0x34:
                    if (state->unk340 == 0) {
                        state->unk6 = 0x35;
                    }
                    break;
                case 0x35:
                case 0x37:
                    if (state->unk340 == 0) {
                        state->unk5 = 0x34;
                        state->unk6 = 0x33;
                    }
                    break;
            }
            break;
        case 0x35:
            switch (state->unk6) {
                case 0x35:
                case 0x36:
                    break;
                case 0x33:
                case 0x37:
                    if (state->unk340 == 0) {
                        state->unk5 = 0x36;
                        state->unk6 = 0x34;
                    }
                    break;
            }
            break;
        case 0x37:
            switch (state->unk6) {
                case 0x34:
                    break;
                case 0x33:
                case 0x35:
                    if (state->unk340 == 0) {
                        state->unk5 = 0x38;
                        state->unk6 = 0x36;
                    }
                    break;
            }
            break;
        case 0x5A:
            state->unk6 = 0x34;
            break;
        case 0x5C:
            state->unk6 = 0x35;
            break;
        case 0x58:
        case 0x5E:
            state->unk6 = 0x33;
            break;
    }
    switch (state->unk6) {
        case 2:
            if (D_80095880 == 0xE && D_8009586C == 0 && (D_8009585C & 7) == 0) {
                func_80042538(0x13);
            }
            break;
        case 3:
            if (D_80095880 == 0xE && D_8009586C == 0 && D_8009585C % 6 == 0) {
                func_80042538(0x13);
            }
            break;
        case 4:
            if (D_80095880 == 0xE && D_8009586C == 0 && (D_8009585C & 3) == 0) {
                func_80042538(0x13);
            }
            break;
        case 8:
        case 9:
        case 0xA:
        case 0xB:
        case 0x38:
        case 0x39:
            state->unk398 -= 5;
            d = state->unk398;
            state->unk34C -= d;
            if (state->unk39C <= sGame.unk3CC && state->unk3B8 == 1) {
                if (state->unk34C >= sGame.unk3CC) {
                    state->unk34C = sGame.unk3CC;
                    func_80018CAC(&sGame, d);
                    /* MATCHING: case 0 shares the default body, which the compare tree reaches first. */
                    switch ((s8)sGame.unk38E) {
                        case 0:
                        default:
                            state->unk6 = 3;
                            break;
                        case 2:
                            state->unk6 = 0x1A;
                            break;
                    }
                    func_80022F68((VECTOR *)&state->unk348);
                    state->unk38F = 0;
                    state->unk3BC = 1;
                    func_80015450(D_800734AC, 13);
                }
            } else if (state->unk34C >= state->unk3C0) {
                state->unk34C = D_800AC858[0];
                func_80018CAC(&sGame, d);
                switch ((s8)sGame.unk38E) {
                    case 0:
                    default:
                        state->unk6 = 3;
                        break;
                    case 2:
                        state->unk6 = 0x1A;
                        break;
                }
                func_80022F68((VECTOR *)&state->unk348);
                state->unk38F = 0;
                state->unk3BC = 0;
                func_80015450(D_800734AC, 13);
                D_800DF5B0[0] = state->unk348;
                D_800DF5B0[1] = state->unk34C;
                D_800DF5B0[2] = state->unk350;
                D_800956E0 = SquareRoot0(func_800297A4((VECTOR *)D_800DF5A0, (VECTOR *)D_800DF5B0));
            }
            break;
        case 0xD:
            state->unk3A8 -= 2;
            if (state->unk3A8 <= 0) {
                func_800287C0();
            }
            /* MATCHING: cases 7, 0x1F and 0x26 branch here; their own calls would be the copies
             * cross-jumping keeps. */
            if (state->unk340 == 0 && (state->unk5 == 8 || state->unk5 == 0xA)) {
            reset:
                func_800283E4();
            }
            break;
        case 0x15:
        case 0x16:
        case 0x17:
            state->unk3A8 -= 4;
            if (state->unk3A8 <= 0) {
                func_800287C0();
            }
            if (sGame.unk374 == 0) {
                func_80042538(0x1A);
            }
            state->unk0 = 0;
            sGame.unk374++;
            sGame.unk362 += ANGLE_DEG(sGame.unk374 / 2);
            if (state->unk340 == 0) {
                sGame.unk376 = (sGame.unk376 + 20) % 360;
                if (sGame.unk376 == 80 || sGame.unk376 == 260) {
                    state->unk6 = 0x16;
                }
                if (sGame.unk376 == 160 || sGame.unk376 == 340) {
                    state->unk6 = 0x17;
                }
                v = rsin(ANGLE_DEG(sGame.unk376));
                /* MATCHING: one expression, so the table's %hi is built before the abs. */
                sGame.unk34C =
                    D_800AC858[0] - ((v * 100 >> 12) >= 0 ? (v * 100 >> 12) : -(v * 100 >> 12));

                if (sGame.unk374 >= 30) {
                    state->unk362 = 0;
                    func_800287C0();
                    if (func_80028260(0) != 0) {
                        func_800283E4();
                    } else {
                        sGame.unk374 = 0;
                        state->unk6 = 0x31;
                    }
                }
            }
            D_8009EAB8[1] = sGame.unk380 + sGame.unk362;
            D_800D86E0[0].coord.t[0] = sGame.unk354;
            D_800D86E0[0].coord.t[1] = sGame.unk34C + sGame.unk358;
            D_800D86E0[0].coord.t[2] = sGame.unk35C;
            func_80018AE0((SVECTOR *)D_8009EAB8, D_800D86E0);
            break;
        case 0x10:
            if (state->unk374 < 10) {
                state->unk374++;
            }
            state->unk3A8 = func_80018D04(state->unk3AA, 0, state->unk374, 10);
            switch (state->unk5) {
                case 0x18:
                    if (state->unk340 == 0) {
                        state->unk6 = 0x13;
                    }
                    break;
                case 0x1A:
                    if (state->unk340 == 0) {
                        state->unk6 = 0xF;
                    }
                    break;
                case 0x1C:
                    if (state->unk340 == 0) {
                        state->unk6 = 0x11;
                    }
                    break;
                case 0x20:
                    if (state->unk340 == 0) {
                        state->unk6 = 0x12;
                    }
                    break;
            }
            break;
        case 0xE:
            if (state->unk374 < 60) {
                state->unk374++;
            }
            state->unk3A8 = func_80018D04(state->unk3AA, 0, state->unk374, 60);
            break;
        case 0x13:
            state->unk0 = 0;
            break;
        case 0xF:
            sGame.unk3D4 = 1;
            if (state->unk340 == 0) {
                if (++state->unk374 >= 46) {
                    state->unk5 = 0;
                    state->unk6 = 0x13;
                    sGame.unk3D4 = 0;
                }
            }
            break;
        case 0x11:
            if (++state->unk374 == 13) {
                func_80042538(0x30);
            }
            if (state->unk374 == 20) {
                func_80042538(0x31);
            }
            if (state->unk374 == 40) {
                state->unk3D0 = (state->unk3D0 & 0xF) | 0x50;
            }
            if (state->unk374 == 90) {
                func_80042538(0x2B);
            }
            if (state->unk374 == 110) {
                func_80042538(0x2B);
            }
            if (state->unk374 == 130) {
                func_80042538(0x2B);
            }
            if (state->unk374 == 90) {
                state->unk3 = 1;
                state->unk7 = 0x13;
            }
            if (state->unk341 == 0) {
                state->unk3 = 0;
                state->unk7 = 0xFF;
                state->unk9 = 0xFF;
            }
        case 0x12:
            if (state->unk340 == 0) {
                state->unk5 = 0;
                state->unk6 = 0x13;
                state->unk3D0 &= 0xF;
            }
            break;
        case 5:
            state->unk3 = 0;
            if (state->unk340 == 0) {
                state->unk6 = 6;
                if (state->unk3A8 >= 0xB0) {
                    state->unk3A4 = 1;
                }
                if ((u32)((u16)state->unk3A8 - 0x97) < 0x19) {
                    state->unk3A4 = 1;
                }
                if ((u32)((u16)state->unk3A8 - 0x7E) < 0x19) {
                    state->unk3A4 = 1;
                }
                if (state->unk3A8 < 0x7E) {
                    state->unk3A4 = 1;
                }
            }
            if (sGame.unk34C == sGame.unk3C0) {
                func_80022F68((VECTOR *)&state->unk348);
            }
            func_80015450(D_800734AC, 14);
            break;
        case 6:
            if (D_8009585C % 3 == 0) {
                func_80022F68((VECTOR *)&state->unk348);
            }
            if (state->unk3A4 == 0) {
                state->unk6 = 7;
            }
            state->unk3A4--;
            break;
        case 0x14:
            state->unk3A8 -= 4;
            if (state->unk3A8 <= 0) {
                func_800287C0();
            }
            if (state->unk398 > 0) {
                state->unk398 -= 7;
                d = state->unk398;
                state->unk34C -= d;
            }
            r = func_800281B8(state);
            /* MATCHING: three nested tests; one condition folds into a range check. */
            if (r != 0)
                if (r >= 0)
                    if (r < 3) {
                        state->unk6 = 0x24;
                    }
            break;
        case 0x1B:
            state->unk3A8 -= 2;
            if (state->unk3A8 <= 0) {
                func_800287C0();
            }
            state->unk398 -= 7;
            d = state->unk398;
            state->unk34C -= d;
            r = func_800281B8(state);
            if (r != 0)
                if (r >= 0)
                    if (r < 3) {
                        switch (state->unk5) {
                            case 0x41:
                                func_80015450(D_800734AC, 3);
                                state->unk6 = 0x20;
                                break;
                            case 0x3F:
                                state->unk6 = 0x1C;
                                break;
                        }
                    }
            break;
        case 0x1C:
            state->unk3A8 -= 2;
            if (state->unk3A8 >= 0x15) {
                func_80022F68((VECTOR *)D_8009EEC0);
            }
            if (state->unk3A8 <= 0) {
                func_800287C0();
                if (++state->unk374 >= 16) {
                    state->unk374 = 0;
                    state->unk6 = 0x1D;
                }
            }
            break;
        case 0x1D:
            state->unk374++;
            if (state->unk340 == 0) {
                if (func_80028260(0) != 0) {
                    state->unk376 = 0;
                    state->unk6 = 0x1F;
                } else {
                    if (state->unk374 == 30) {
                        func_80042538(0x34);
                    }
                    if (state->unk374 >= 61) {
                        D_80095858 = 100;
                    }
                }
            }
            break;
        case 0x1E:
            if (state->unk340 == 0) {
                switch (state->unk5) {
                    case 0x14:
                    case 0x28:
                    case 0x3F:
                    case 0x43:
                        state->unk376 = 0;
                        state->unk6 = 0x1F;
                        break;
                    case 0x41:
                    case 0x45:
                        state->unk376 = 0;
                        state->unk6 = 0x22;
                        break;
                }
            }
            break;
        case 0x20:
            if (state->unk340 == 0) {
                state->unk374++;
                func_80022F68((VECTOR *)D_8009EEC0);
                state->unk6 = 0x20;
                state->unk8 = 0xFF;
            }
            if (state->unk374 == 2) {
                state->unk374 = 0;
                state->unk6 = 0x21;
            }
            break;
        case 0x21:
            state->unk3A8 -= 1;
            if (state->unk3A8 >= 6) {
                func_80022F68((VECTOR *)D_8009EEC0);
                func_80015450(D_800734AC, 11);
            }
            if (state->unk3A8 <= 0) {
                func_800287C0();
                if (++state->unk374 >= 16) {
                    switch (state->unk5) {
                        default:
                            if (func_80028260(0) != 0) {
                                state->unk374 = 0;
                                state->unk376 = 0;
                                state->unk6 = 0x1F;
                            } else {
                                if (state->unk374 == 30) {
                                    func_80042538(0x34);
                                }
                                if (state->unk374 >= 61) {
                                    D_80095858 = 100;
                                }
                            }
                            break;
                        case 0x49:
                            if (func_80028260(0) != 0) {
                                state->unk376 = 0;
                                state->unk6 = 0x2D;
                            } else {
                                if (state->unk374 == 30) {
                                    func_80042538(0x34);
                                }
                                if (state->unk374 >= 61) {
                                    D_80095858 = 100;
                                }
                            }
                            break;
                        case 0x4F:
                            if (func_80028260(0) != 0) {
                                state->unk6 = 0x22;
                            } else {
                                if (state->unk374 == 30) {
                                    func_80042538(0x34);
                                }
                                if (state->unk374 >= 61) {
                                    D_80095858 = 100;
                                }
                            }
                            break;
                        case 0x57:
                            D_80095858 = -1;
                            if (state->unk374 >= 61) {
                                if (D_80095830 == 4 && D_8009578C == 0) {
                                    state->unk6 = 0x22;
                                    D_80095858 = 0;
                                } else {
                                    D_80095858 = 100;
                                }
                            }
                            if ((D_80095830 != 4 || D_8009578C != 0) && state->unk374 == 30) {
                                func_80042538(0x34);
                            }
                            break;
                    }
                }
            }
            break;
        case 0x23:
            if (state->unk5 == 0x55) {
                sGame.unk354 = func_80018D04(0, D_80096768[0], state->unk378, 5);
                sGame.unk358 = func_80018D04(0, -50, state->unk378, 5);
                sGame.unk35C = func_80018D04(0, D_80096768[2], state->unk378, 5);
                if (++state->unk378 >= 5) {
                    func_80015450(D_800734AC, 5);
                    state->unk6 = 0x28;
                    state->unk37A = 0;
                    state->unk378 = 0;
                    state->unk376 = 0;
                    state->unk374 = 0;
                }
                D_800D86E0[0].coord.t[0] = sGame.unk354;
                D_800D86E0[0].coord.t[1] = sGame.unk34C + sGame.unk358;
                D_800D86E0[0].coord.t[2] = sGame.unk35C;
                func_80018AE0((SVECTOR *)D_8009EAB8, D_800D86E0);
                break;
            }
            state->unk3A8 += 4;
            if (state->unk3A8 >= 0) {
                func_800287C0();
            }
            state->unk398 -= 7;
            d = state->unk398;
            state->unk34C -= d;
            r = func_800281B8(state);
            if (r != 0)
                if (r >= 0)
                    if (r < 3) {
                        state->unk6 = 0x24;
                    }
            break;
        case 0x24:
            func_80022F68((VECTOR *)D_8009EEC0);
            if (state->unk340 == 0) {
                switch (state->unk5) {
                    case 0x28:
                    case 0x43:
                    case 0x57:
                        state->unk6 = 0x25;
                        break;
                    case 0x45:
                        func_80015450(D_800734AC, 3);
                        state->unk6 = 0x27;
                        break;
                }
            }
            break;
        case 0x25:
            if (state->unk340 == 0) {
                state->unk376 = 0;
                state->unk374 = 0;
                state->unk6 = 0x21;
            }
            break;
        case 0x27:
            if (state->unk340 == 0) {
                state->unk374++;
                func_80022F68((VECTOR *)D_8009EEC0);
                state->unk6 = 0x27;
                state->unk8 = 0xFF;
            }
            state->unk376++;
            if (state->unk374 >= 2) {
                if (func_80028260(0) > 0) {
                    switch (state->unk5) {
                        case 0x45:
                            state->unk374 = 0;
                            state->unk6 = 0x21;
                            break;
                        case 0x47:
                            state->unk374 = 0;
                            state->unk376 = 0;
                            state->unk6 = 0x1F;
                            state->unk3A8 = 0;
                            state->unk374 = 0;
                            break;
                    }
                } else {
                    func_80015450(D_800734AC, 6);
                    state->unk6 = 0x21;
                    state->unk374 = 0;
                    if (state->unk376 == 30) {
                        func_80042538(0x34);
                    }
                    if (state->unk376 >= 61) {
                        D_80095858 = 100;
                    }
                }
            }
            break;
        case 0x28:
            switch (state->unk5) {
                case 0x51:
                    D_8009EAB8[1] = sGame.unk362 = D_800A7680[0].vy + 0x800;
                case 0x55:
                    D_800D86E0[0].coord.t[0] = sGame.unk354;
                    D_800D86E0[0].coord.t[1] = sGame.unk34C + sGame.unk358;
                    D_800D86E0[0].coord.t[2] = sGame.unk35C;
                    func_80018AE0((SVECTOR *)D_8009EAB8, D_800D86E0);
                    if (++state->unk378 < 10) {
                        /* MATCHING: x - (5 - r) keeps the -5 on the loaded word. */
                        D_80096768[0] = D_80096768[0] - (5 - rand() % 10);
                        D_80096768[1] = D_80096768[1] - (5 - rand() % 10);
                        D_80096768[2] = D_80096768[2] - (5 - rand() % 10);
                        GsSetRefView2((GsRVIEW2 *)D_80096768);
                    }
                    if (state->unk378 >= 31) {
                        sGame.unk358 += 2;
                    }
                    D_80095858 = -1;
                    if (state->unk378 >= 121) {
                        D_80095858 = 100;
                    }
                    break;
                default:
                    state->unk374--;
                    state->unk376++;
                    if (state->unk374 <= 0) {
                        state->unk374 = 1;
                    }
                    if (state->unk376 & 1) {
                        sGame.unk360 = -(state->unk374 * 0x5000 / 360);
                    } else {
                        sGame.unk360 = state->unk374 * 0x5000 / 360;
                    }
                    D_8009EAB8[0] = sGame.unk360;
                    D_800D86E0[0].coord.t[0] = sGame.unk354;
                    D_800D86E0[0].coord.t[1] = sGame.unk34C + sGame.unk358;
                    D_800D86E0[0].coord.t[2] = sGame.unk35C;
                    func_80018AE0((SVECTOR *)D_8009EAB8, D_800D86E0);
                    if (state->unk340 == 0) {
                        sGame.unk360 = 0;
                        D_8009EAB8[0] = 0;
                        state->unk6 = 0x29;
                    }
                    break;
            }
            break;
        case 0x29:
            if (state->unk340 == 0) {
                state->unk374 = 0;
                state->unk376 = 0;
                state->unk3A8 = -5;
                func_80015450(D_800734AC, 3);
                state->unk6 = 0x27;
            }
            break;
        case 0x2A:
            switch (state->unk5) {
                default:
                    if (state->unk340 == 0 && ++state->unk374 >= 16) {
                        if (func_80028260(0) != 0) {
                            state->unk6 = 0x2B;
                        } else {
                            if (state->unk374 == 30) {
                                func_80042538(0x34);
                            }
                            if (state->unk374 >= 61) {
                                D_80095858 = 100;
                            }
                        }
                    }
                    break;
                case 0x4B:
                    if (state->unk3A8 >= 0x15) {
                        func_80022F68((VECTOR *)D_8009EEC0);
                    }
                    state->unk3A8 -= 2;
                    if (state->unk3A8 <= 0) {
                        func_800287C0();
                    }
                    if (state->unk340 == 0 && state->unk3A8 == 0 && ++state->unk374 >= 16) {
                        if (func_80028260(0) != 0) {
                            state->unk6 = 0x2B;
                        } else {
                            if (state->unk374 == 30) {
                                func_80042538(0x34);
                            }
                            if (state->unk374 >= 61) {
                                D_80095858 = 100;
                            }
                        }
                    }
                    break;
                case 0x4D:
                    if (state->unk340 == 0 && ++state->unk374 >= 16) {
                        if (func_80028260(0) != 0) {
                            state->unk376 = 0;
                            state->unk6 = 0x26;
                            state->unk374 = 0;
                        } else {
                            if (state->unk374 == 30) {
                                func_80042538(0x34);
                            }
                            if (state->unk374 >= 61) {
                                D_80095858 = 100;
                            }
                        }
                    }
                    break;
            }
            break;
        case 0x2B:
            if (state->unk340 == 0) {
                state->unk376 = 0;
                state->unk6 = 0x1F;
            }
            break;
        case 0x2C:
            if (state->unk340 == 0) {
                state->unk376 = 0;
                state->unk6 = 0x2D;
            }
            break;
        case 0x2D:
            sGame.unk354 = rsin(D_800A7680[0].vy + 0x400) * 20 >> 12;
            sGame.unk35C = rcos(D_800A7680[0].vy + 0x400) * 20 >> 12;
            D_800D86E0[0].coord.t[2] = sGame.unk35C;
            D_800D86E0[0].coord.t[0] = sGame.unk354;
            D_800D86E0[0].coord.t[1] = sGame.unk34C + sGame.unk358;
            func_80018AE0((SVECTOR *)D_8009EAB8, D_800D86E0);
            sGame.unk35C = 0;
            sGame.unk354 = 0;
        case 0x22:
            if (++state->unk376 == 1) {
                func_80042538(0x50);
            }
        case 0x7:
            if (state->unk340 == 0) {
                goto reset;
            }
            break;
        case 0x2E:
            state->unk3A8 -= 2;
            if (state->unk3A8 <= 0) {
                func_800287C0();
            }
            if (state->unk340 == 0) {
                state->unk6 = 0x2A;
            }
            break;
        case 0x2F:
            if (state->unk340 == 0) {
                r = func_800281B8(state);
                if (r != 0)
                    if (r >= 0)
                        if (r < 3) {
                            state->unk6 = 0x2A;
                        }
            }
            break;
        case 0x1F:
        case 0x26:
            if (++state->unk376 == 1) {
                func_80042538(0x50);
            }
            if (state->unk340 == 0 && ++state->unk374 >= 16) {
                goto reset;
            }

            break;
        case 0x30:
            switch (state->unk5) {
                case 0x51:
                    sGame.unk362 = state->unk376 =
                        func_80018D04(state->unk376, 0x6000, state->unk374, 30);
                    sGame.unk354 = func_80018D04(0, D_80096768[0], state->unk378, 15);
                    sGame.unk358 = func_80018D04(0, -50, state->unk378, 15);
                    sGame.unk35C = func_80018D04(0, D_80096768[2], state->unk378, 15);
                    state->unk374++;
                    if (++state->unk378 >= 14) {
                        func_80015450(D_800734AC, 5);
                        state->unk6 = 0x28;
                        state->unk37A = 0;
                        state->unk378 = 0;
                        state->unk376 = 0;
                        state->unk374 = 0;
                    }
                    break;
                case 0x4F:
                    if (++state->unk378 >= 41) {
                        state->unk378 = 60;
                        state->unk6 = 0x31;
                        state->unk374 = 0;
                    }
                    sGame.unk362 = state->unk376 =
                        func_80018D04(state->unk376, 0x5800, state->unk378, 60);
                    break;
            }
            D_8009EAB8[1] = sGame.unk362;
            D_800D86E0[0].coord.t[0] = sGame.unk354;
            D_800D86E0[0].coord.t[1] = sGame.unk34C + sGame.unk358;
            D_800D86E0[0].coord.t[2] = sGame.unk35C;
            func_80018AE0((SVECTOR *)D_8009EAB8, D_800D86E0);
            break;
        case 0x31:
            if (state->unk340 == 0) {
                func_80015450(D_800734AC, 6);
                state->unk374 = 0;
                state->unk6 = 0x21;
            }
            break;
        case 0x32:
            D_80095858 = -1;
            if (++state->unk374 >= 91) {
                D_80095858 = 100;
            }
            break;
        case 0x33:
        case 0x34:
        case 0x35:
            if ((u32)((D_8009EF48[0] & 0xF) - 2) < 2 && state->unk340 == 0) {
                state->unk8 = 0xFF;
            }
            break;
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/code_13068", func_80024450);
#endif

/* MATCHING: non-void with no return keeps two bnez delay slots nops. */
s32 func_80026548(void) {
    s16 v;

    if (sGame.unk0 != 0 && (u32)(sGame.unk6 - 8) >= 4) {
        if (sGame.unk5 != 0x3A) {
            v = sGame.unk3A8;
            if (v < 40) {
                D_800957D2 += 16;
            } else if (D_80095964 & 0x1000) {
                D_800957D2 += 32;
                if ((s16)D_800957D2 >> 4 > 40) {
                    D_800957D2 = 640;
                }
            } else if (D_80095964 & 0x4000) {
                D_800957D2 -= 32;
                if ((s16)D_800957D2 >> 4 < 40) {
                    D_800957D2 = 640;
                }
            } else if (v > 40) {
                D_800957D2 -= 16;
            } else {
                D_800957D2 += 16;
            }
            sGame.unk3A8 = (s16)D_800957D2 >> 4;
            if (sGame.unk3AC == 2 && sGame.unk340 == 2) {
                sGame.unk6 = 4;
            }
            if (sGame.unk3AC == 1 && sGame.unk340 == 2) {
                sGame.unk6 = 3;
            }
            if (sGame.unk3AC == 0 && sGame.unk340 == 2) {
                sGame.unk6 = 2;
            }
        } else {
            D_800957D2 = 0x460;
            sGame.unk3A8 = 0x46;
        }
    }
}

/* MATCHING: non-void with no return keeps li 0x71 first in the call block. */
s32 func_8002670C(void) {
    s32 btn;

    if (sGame.unk0 != 0) {
        switch ((s8)sGame.unk38E) {
            case 0:
                sGame.unk38E = 0;
                sGame.unk38C = 0;
                if ((u32)(sGame.unk6 - 2) < 3 && sGame.unk34C >= sGame.unk3C0 && (D_800957EC & 0x40)) {
                    btn = D_80095964;
                    if (btn & 0x4000) {
                        sGame.unk38E = 1;
                        sGame.unk38C = 0;
                        sGame.unk5 = 0x3B;
                    }
                    if (btn & 0x1000) {
                        if ((u32)((u16)sGame.unk3A8 - 0x26) < 5) {
                            sGame.unk38E = 2;
                            sGame.unk5 = 0x39;
                            func_80042538(0x71);
                            sGame.unk38C = 0;
                        }
                    }
                }
                break;
            case 1:
                func_80028500();
                break;
            case 2:
                if (sGame.unk5 == 0) {
                    sGame.unk38E = 0;
                    sGame.unk38C = 0;
                }
                break;
        }
    }
}

/* MATCHING: as func_80027E14 for the switch; the volatile read keeps the unk348 and
 * unk350 loads below the stores to D_800956E0, D_800DF5A0[1] and D_800956E4. */
void func_80026848(void) {
    s16 n;

    if (sGame.unk0 != 0) {
        if ((D_800957EC & 0x20) &&
            (sGame.unk34C >= sGame.unk3C0 || (sGame.unk3B8 == 1 && sGame.unk3BC == 1)) &&
            sGame.unk390 == 0) {
            D_800956E0 = 0;
            D_800DF5A0[1] = sGame.unk34C;
            D_800956E4 = sGame.unk34C;
            D_800DF5A0[0] = *(volatile s32 *)&sGame.unk348;
            D_800DF5A0[2] = sGame.unk350;
            sGame.unk38F = 1;
            sGame.unk3BC = 0;
            sGame.unk390 = 1;
            sGame.unk392 = 0;
            sGame.unk39C = 0;
            func_80018CA4(&sGame);
            sGame.unk6 = 8;
            sGame.unk398 = 0x23;
            sGame.unk394 = sGame.unk34C;
            func_80022F68((VECTOR *)&sGame.unk348);
            func_80042538(0x17);
        }
        if (sGame.unk390 == 1) {
            n = ++sGame.unk392;
            if (n < 3) {
                sGame.unk398 = 0x19;
            } else if ((u16)(n - 4) < 3) {
                sGame.unk398 = 0x12;
            } else if (n >= 9) {
                sGame.unk398 = 0x14;
                sGame.unk390 = 2;
                sGame.unk6 = 9;
                sGame.unk3A0 = 2;
            } else if (D_800957B4 & 0x20) {
                sGame.unk398 = 0x14;
            } else {
                sGame.unk390 = 2;
                switch (n) {
                    case 3:
                        sGame.unk6 = 0xB;
                        sGame.unk3A0 = 0;
                        break;
                    case 7:
                        sGame.unk6 = 0xA;
                        sGame.unk3A0 = 1;
                        break;
                    default:
                        sGame.unk6 = 9;
                        sGame.unk3A0 = 2;
                        break;
                }
            }
        }
        if (sGame.unk390 == 2) {
            sGame.unk390 = 0;
        }
    }
}

/* MATCHING: non-void with no return keeps the last branch's delay slot a nop. */
s32 func_80026A60(void) {
    D_800DB2A0[0] = 0;
    D_800DB2A0[1] = rsin((D_80095914 << 12) / 360) * 500 / 4096 + sGame.unk3C0;
    D_800DB2A0[2] = rcos((D_80095914 << 12) / 360) * 600 / 4096;
    D_800DB2A0[4] = sGame.unk34C;
    D_800DB2A0[0] = rsin(D_800A7680->vy) * 400 / 4096;
    D_800DB2A0[2] = rcos(D_800A7680->vy) * 400 / 4096;
    D_800DB2A0[3] = rsin(D_800A7680->vy - 0x800) * 900 / 4096;
    D_800DB2A0[5] = rcos(D_800A7680->vy - 0x800) * 900 / 4096;
    if (D_8009587E < sGame.unk3A8) {
        D_8009587E++;
    }
    if (D_8009587E > sGame.unk3A8) {
        D_8009587E--;
    }
}

/* MATCHING: the index-first integer sum gives retail's addu operand order. */
void func_80026C70(void) {
    u8 *c;

    c = (u8 *)((((u32)D_80095864 >> 12) & 3) * 4 + (s32)D_80095790);
    sLights.l[0].r = sLights.l[1].r = sLights.l[2].r = c[0];
    sLights.l[0].g = sLights.l[1].g = sLights.l[2].g = c[1];
    sLights.l[0].b = sLights.l[1].b = sLights.l[2].b = c[2];
    sLights.l[0].vx = 0;
    sLights.l[0].vy = 100;
    sLights.l[0].vz = 100;
    GsSetFlatLight(0, &sLights.l[0]);
    sLights.l[1].vx = 86;
    sLights.l[1].vy = 100;
    sLights.l[1].vz = -50;
    GsSetFlatLight(1, &sLights.l[1]);
    sLights.l[2].vx = -86;
    sLights.l[2].vy = 100;
    sLights.l[2].vz = -50;
    GsSetFlatLight(2, &sLights.l[2]);
    if (D_8009576A == 100 || D_80095830 % 3 == 2) {
        func_80028650();
    }
}

/* MATCHING: the empty case -2 sets the switch's compare tree; non-void with no return keeps
 * the guards' delay slots nops; the unused words give retail's frame. */
s32 func_80026D9C(void) {
    s32 unused[4];
    u16 i;
    s32 x;
    s16 t;

    if (D_80095830 % 3 == 2) {
        goto end;
    }
top:
    switch (D_80095858) {
        case -2:
            break;
        case -1:
            func_8003AFAC();
            break;
        case 0:
            x = D_800958A8;
            if (D_800958AC == 1) {
                if (x >= 2) {
                    D_800958A8 = 2;
                }
            } else if (x >= 3) {
                D_800958A8 = 3;
            }
            if (D_800958A8 <= 0) {
                D_800958A8 = 0;
            }
            if (D_8009EF4D[0] == 1) {
                D_800958A8 = 0;
            }
            if (D_800958A8 != 0 && D_8009EF4D[0] != 1 && (D_80095830 != 4 || D_8009578C != 0)) {
                D_80095980++;
                D_80095988--;
            }
            if (D_80095830 % 3 == 2) {
                break;
            }
            func_80040F14();
            func_8003AFAC();
            if (D_800958F8 == 1) {
                break;
            }
            if (D_80095988 > 0) {
                break;
            }
            D_80095988 = 0;
            sGame.unk0 = 0;
            sGame.unk2 = 0;
            sGame.unk3C8 = 500;
            sGame.unk38E = 0;
            sGame.unk38C = 0;
            if (sGame.unk3D0 & 0xF) {
                sGame.unk5 = 0;
                sGame.unk3D0 &= 0xF0;
                sGame.unk3D1 = 1;
                sGame.unk398 = 0;
                sGame.unk390 = 0;
            }
            if (D_800CF080[196].unk0 == 1) {
                break;
            }
            if (D_800CF080[198].unk0 == 1) {
                break;
            }
            if (sGame.unk5 == 0 || sGame.unk5 == 0x3A || sGame.unk5 == 0x3C) {
                if ((s16)D_800957D2 >> 4 < 15) {
                    D_800957D2 = 0xF0;
                }
                sGame.unk5 = 0x23;
                D_80095858 = 1;
                D_800959B0 = 0;
                D_800957DC = 0;
                sGame.unk3A8 = (s16)D_800957D2 >> 4;
                func_80042958(4);
                goto top;
            }
            break;
        case 1:
            D_800957DC++;
            func_8003C014();
            if (D_800957DC == 10) {
                for (i = 0; i < 200; i++) {
                    D_800CF080[i].unk0 = 2;
                }
                D_800958EC = 0;
                D_800957D6 = 0;
                *(Words3 *)D_800DF5C0 = *(Words3 *)D_8009EEC0;
            }
            if (D_800957DC >= 10) {
                sGame.unk3A8 = 0;
                D_800957D2 = 0;
                *(Words3 *)&sGame.unk348 = *(Words3 *)D_800DF5C0;
            }
            if (D_800957DC > 100) {
                t = D_800957DC - 100;
                if (t > 60) {
                    D_80095858 = 100;
                }
            }
            break;
        case 2:
            func_80017574();
            func_80042538(0x1B);
            D_800957DC = 0;
            D_80095858 = 3;
            break;
        case 3:
            t = ++D_800957DC;
            if (t > 60) {
                D_80095858 = 100;
            }
            break;
        case 100:
            ClipF = 0;
            if (D_80095770 == 0) {
                D_8009EF4A[0] = 0;
                D_80095880 = 6;
                D_80095760 = 0xFFFF;
            } else {
                func_8004079C();
                D_80095770--;
            }
            break;
        case 110:
            t = ++D_800957DC;
            func_8003E360(t);
            if (t == 0x180) {
                D_80095880 = 6;
            }
            break;
    }
end:;
}

void func_800272D4(void) {
    switch (sGame.unk3D0 & 0xF) {
        case 1:
            D_800D3860[0].unk18 = -0x88;
            D_800D3860[0].unk1A = 0xAA;
            D_800D3860[0].unk4 = 0;
            D_800D8370[0].unk58 = (s32)&D_800D86E0[16];
            D_800D3860[0].unk0 = -D_800A7308[0];
            D_800D3860[0].unk8 = -D_800A7308[2];
            func_8002A5B0((Rec78 *)&D_800D8370[D_800D3860[0].unk36], (Rec48 *)&D_800D3860[0]);
            break;
        case 4:
            D_800D3860[5].unk18 = -0x88;
            D_800D3860[5].unk1C = -0x88;
            D_800D3860[5].unk1A = 0xAA;
            D_800D3860[5].unk4 = 0;
            D_800D8370[5].unk58 = (s32)&D_800D86E0[16];
            D_800D3860[5].unk0 = -D_800A7308[0];
            D_800D3860[5].unk8 = -D_800A7308[2];
            func_8002A5B0((Rec78 *)&D_800D8370[D_800D3860[5].unk36], (Rec48 *)&D_800D3860[5]);
            break;
        case 2:
            switch (D_80095830) {
                case 3:
                    D_800D3860[3].unk4 = 0x3C;
                    D_800D8370[3].unk58 = (s32)&D_800D86E0[0];
                    D_800D3860[3].unk0 = -D_800A7308[0];
                    D_800D3860[3].unk8 = -D_800A7308[2];
                    D_800D3860[3].unk18 += ANGLE_DEG(sGame.unk3A8 / 2);
                    func_8002A5B0((Rec78 *)&D_800D8370[D_800D3860[3].unk36], (Rec48 *)&D_800D3860[3]);
                    if (D_8009585C % 20 == 0) {
                        func_80042538(0x81);
                    }
                    break;
                case 10:
                    D_800D3860[6].unk4 = 0x32;
                    D_800D8370[6].unk58 = (s32)&D_800D86E0[0];
                    D_800D3860[6].unk0 = -D_800A7308[0];
                    D_800D3860[6].unk8 = -D_800A7308[2];
                    D_800D3860[6].unk18 += ANGLE_DEG(sGame.unk3A8 / 2);
                    func_8002A5B0((Rec78 *)&D_800D8370[D_800D3860[6].unk36], (Rec48 *)&D_800D3860[6]);
                    if (D_8009585C % 20 == 0) {
                        func_80042538(0x81);
                    }
                    break;
            }
            break;
        case 3:
            switch (D_80095830) {
                case 0:
                    D_800D3860[4].unk4 = 0;
                    D_800D8370[4].unk58 = (s32)&D_800D86E0[1];
                    D_800D3860[4].unk0 = -D_800A7308[0];
                    D_800D3860[4].unk8 = -D_800A7308[2];
                    func_8002A5B0((Rec78 *)&D_800D8370[D_800D3860[4].unk36], (Rec48 *)&D_800D3860[4]);
                    break;
                case 12:
                    D_800D3860[2].unk4 = 0;
                    D_800D8370[2].unk58 = (s32)&D_800D86E0[1];
                    D_800D3860[2].unk0 = -D_800A7308[0];
                    D_800D3860[2].unk8 = -D_800A7308[2];
                    func_8002A5B0((Rec78 *)&D_800D8370[D_800D3860[2].unk36], (Rec48 *)&D_800D3860[2]);
                    break;
            }
            break;
    }
    if ((D_8009EF48[0] >> 4) == 5) {
        D_800D3860[1].unk4 = 0;
        D_800D3860[1].unk0 = -D_800A7308[0];
        D_800D3860[1].unk8 = -D_800A7308[2];
        func_8002A5B0((Rec78 *)&D_800D8370[D_800D3860[1].unk36], (Rec48 *)&D_800D3860[1]);
    }
}

/* MATCHING: non-void with no return makes the guards fill their delay slots with the lui. */
s32 func_80027714(void) {
    switch (sGame.unk3D0 & 0xF) {
        case 1:
            if ((u32)(sGame.unk6 - 0x33) >= 7) {
                sGame.unk6 = 0x33;
                D_800D83C8[0] = (s32)&D_800D86E0[16];
                D_800D3896[0] = 0;
            }
            break;
        case 4:
            if ((u32)(sGame.unk6 - 0x33) >= 7) {
                sGame.unk6 = 0x33;
                D_800D8620[0] = (s32)&D_800D86E0[16];
                D_800D39FE[0] = 5;
            }
            break;
        case 2:
            if ((u32)(sGame.unk6 - 0x33) >= 3) {
                sGame.unk6 = 0x33;
                switch (D_80095830) {
                    case 3:
                        D_800D8530[0] = (s32)&D_800D86E0[0];
                        D_800D396E[0] = 3;
                        break;
                    case 10:
                        D_800D8698[0] = (s32)&D_800D86E0[0];
                        D_800D3A46[0] = 6;
                        break;
                }
            }
            break;
        case 3:
            switch (D_80095830) {
                case 0:
                    D_800D85A8[0] = (s32)&D_800D86E0[1];
                    D_800D39B6[0] = 4;
                    break;
                case 12:
                    D_800D84B8[0] = (s32)&D_800D86E0[1];
                    D_800D3926[0] = 2;
                    break;
            }
            break;
    }
}

static __inline__ s16 findFreeSlot(SlotRec *slot) {
    u32 i;

    for (i = 0; i < D_800959B4 / sizeof(SlotRec); i++) {
        if (slot->unk36 == -1) {
            return i;
        }
        slot++;
    }
    return -1;
}

void func_800278B0(void) {
    SlotRec *slot;
    s16 idx;

    slot = (SlotRec *)D_800959B8;
    if (D_8009EF49[0] == 1) {
        idx = findFreeSlot(slot);
        if (idx != -1) {
            slot += idx;
            switch (D_80095830) {
                case 0:
                    D_800D85A8[0] = 0;
                    slot->unk36 = 4;
                    break;
                case 1:
                    D_800D83C8[0] = 0;
                    slot->unk36 = 0;
                    break;
                case 3:
                    D_800D8530[0] = 0;
                    slot->unk36 = 3;
                    break;
                case 10:
                    D_800D8698[0] = 0;
                    slot->unk36 = 6;
                    break;
                case 12:
                    D_800D84B8[0] = 0;
                    slot->unk36 = 2;
                    break;
                case 13:
                    D_800D8620[0] = 0;
                    slot->unk36 = 5;
                    break;
            }
            slot->unk34 = 0xBA;
            slot->unk42 = 0xFF;
            slot->unk43 = 0x69;
            slot->unk26 = 0;
            slot->unk24 = 0;
            D_800D3358[0] = 1;
        }
        D_8009EF49[0] = 0;
    }
}

/* MATCHING: func_800282F0's body as an inline helper; calling it with the s16 step
 * keeps the step's sign extension ahead of the rsin call. */
void func_80027A00(void) {
    s16 step;
    s32 v;
    s32 x;
    GameState *g;
    s32 hi;

    if (sGame.unk0 != 0) {
        if (D_80095964 & 0x8000) {
            func_80023834(3, 16, 5);
            D_800959B2 -= 4;
        } else if (D_80095964 & 0x2000) {
            func_80023834(2, 16, 5);
            D_800959B2 += 4;
        } else {
            sGame.unk5 = 0x58;
            if (D_800959B2 != 0) {
                if (D_800959B2 < 0) {
                    D_800959B2 += 2;
                } else {
                    D_800959B2 -= 2;
                }
            }
        }
        switch (D_80095830) {
            case 3:
                sGame.unk34C = sGame.unk3C0 - 0x5A;
                break;
            case 10:
                sGame.unk34C = sGame.unk3C0 - 0x50;
                break;
        }
        D_800959B2 = D_800959B2 < -10 ? -10 : D_800959B2 > 10 ? 10 : D_800959B2;
        step = D_800959B2 / 10;
        pushBack(90, step);
        /* MATCHING: as in func_80028008. */
        __asm__("");
        g = &sGame;
        x = g->unk380;
        v = D_800A7680->vy - 0x71;
        if (x >= v) {
            hi = D_800A7680->vy + 0x71;
            if (x <= hi) {
                v = x;
            } else {
                v = hi;
            }
        }
        g->unk380 = v;
    }
}

/* MATCHING: the unsigned range test is the single subtract-and-compare retail does. */
void func_80027BEC(void) {
    s16 v;

    if (sGame.unk0 != 0 && (u32)(sGame.unk6 - 0x33) < 3) {
        v = sGame.unk3A8;
        if (v < 35) {
            D_800957D2 += 16;
        } else if (D_80095964 & 0x1000) {
            D_800957D2 += 32;
            if ((s16)D_800957D2 >> 4 > 35) {
                D_800957D2 = 560;
            }
        } else if (D_80095964 & 0x4000) {
            D_800957D2 -= 32;
            if ((s16)D_800957D2 >> 4 < 35) {
                D_800957D2 = 560;
            }
        } else if (v > 35) {
            D_800957D2 -= 16;
        } else {
            D_800957D2 += 16;
        }
        D_8009EF20[0] = (s16)D_800957D2 >> 4;
    }
}

void func_80027D04(void) {
    s16 v;

    if (sGame.unk0 != 0 && sGame.unk6 == 0x33) {
        v = sGame.unk3A8;
        if (v < 30) {
            D_800957D2 += 16;
        } else if (D_80095964 & 0x1000) {
            D_800957D2 += 32;
            if ((s16)D_800957D2 >> 4 > 30) {
                D_800957D2 = 480;
            }
        } else if (D_80095964 & 0x4000) {
            D_800957D2 -= 32;
            if ((s16)D_800957D2 >> 4 < 30) {
                D_800957D2 = 480;
            }
        } else if (v > 30) {
            D_800957D2 -= 16;
        } else {
            D_800957D2 += 16;
        }
        D_8009EF20[0] = (s16)D_800957D2 >> 4;
    }
}

/* MATCHING: the switch lays the default arm out last, after its own base reload, as retail. */
void func_80027E14(void) {
    s16 n;

    if (sGame.unk0 != 0) {
        if ((D_800957EC & 0x20) &&
            (sGame.unk34C >= sGame.unk3C0 || (sGame.unk3B8 == 1 && sGame.unk3BC == 1)) &&
            sGame.unk390 == 0) {
            sGame.unk38F = 1;
            sGame.unk3BC = 0;
            sGame.unk390 = 1;
            sGame.unk392 = 0;
            sGame.unk39C = 0;
            func_80018CA4(&sGame);
            sGame.unk6 = 8;
            sGame.unk398 = 0x23;
            sGame.unk394 = sGame.unk34C;
            func_80022F68((VECTOR *)&sGame.unk348);
            func_80042538(0x17);
        }
        if (sGame.unk390 == 1) {
            n = ++sGame.unk392;
            if (n < 3) {
                sGame.unk398 = 0x19;
            } else if ((u16)(n - 4) < 3) {
                sGame.unk398 = 0x12;
            } else if (n >= 9) {
                sGame.unk398 = 0x14;
                sGame.unk390 = 2;
                sGame.unk6 = 8;
                sGame.unk3A0 = 2;
            } else if (D_800957B4 & 0x20) {
                sGame.unk398 = 0x14;
            } else {
                sGame.unk390 = 2;
                switch (n) {
                    case 3:
                        sGame.unk6 = 9;
                        sGame.unk3A0 = 0;
                        break;
                    case 7:
                        sGame.unk6 = 9;
                        sGame.unk3A0 = 1;
                        break;
                    default:
                        sGame.unk6 = 9;
                        sGame.unk3A0 = 2;
                        break;
                }
            }
        }
        if (sGame.unk390 == 2) {
            sGame.unk390 = 0;
        }
    }
}

void func_80028008(void) {
    s16 step;
    s32 v;
    s32 x;
    GameState *g;
    s32 hi;

    if (sGame.unk0 != 0) {
        if (D_80095964 & 0x2000) {
            func_80023834(3, 16, 8);
            D_800959B2 -= 4;
        } else if (D_80095964 & 0x8000) {
            func_80023834(2, 16, 8);
            D_800959B2 += 4;
        } else if (sGame.unk34C >= sGame.unk3C0) {
            sGame.unk5 = 0x5E;
            if (D_800959B2 != 0) {
                if (D_800959B2 < 0) {
                    D_800959B2 += 2;
                } else {
                    D_800959B2 -= 2;
                }
            }
        }
        D_800959B2 = D_800959B2 < -7 ? -7 : D_800959B2 > 7 ? 7 : D_800959B2;
        step = D_800959B2 / 10;
        pushBack(90, step);
        /* MATCHING: the barrier keeps the yaw and unk380 loads below the unk350 store;
         * g keeps the sGame base in a register for the store at the join. */
        __asm__("");
        g = &sGame;
        x = g->unk380;
        v = D_800A7680->vy - 0x71;
        if (x >= v) {
            hi = D_800A7680->vy + 0x71;
            if (x <= hi) {
                v = x;
            } else {
                v = hi;
            }
        }
        g->unk380 = v;
    }
}

s32 func_800281B8(GameState *g) {
    s32 ret;

    ret = 0;
    if (g->unk39C <= g->unk3CC && g->unk3B8 == 1) {
        if (g->unk34C >= D_8009EF44[0]) {
            g->unk34C = D_8009EF44[0];
            ret = 2;
            func_80042538(30);
        }
    } else if (g->unk34C >= g->unk3C0) {
        g->unk34C = g->unk3C0;
        ret = 1;
        func_80042538(30);
    }
    if (ret != 0) {
        func_80015450(D_800734AC, 6);
    }
    return ret;
}

s32 func_80028260(s32 n) {
    D_800958A8 += n;
    if (D_800958AC == 1) {
        if (D_800958A8 >= 2) {
            D_800958A8 = 2;
        }
    } else if (D_800958A8 >= 3) {
        D_800958A8 = 3;
    }
    if (D_800958A8 <= 0) {
        D_800958A8 = 0;
    }
    if (D_8009EF4D[0] == 1) {
        D_800958A8 = 0;
    }
    return D_800958A8;
}

/* MATCHING: the local pointer lets angle and the sGame base share $s0. */
void func_800282F0(s32 deg, s32 dist) {
    s32 angle;
    SVECTOR *rot;

    rot = D_800A7680;
    angle = ANGLE_DEG(deg);
    sGame.unk348 -= rsin(rot->vy + angle) * dist >> FIX12_SHIFT;
    sGame.unk350 -= rcos(rot->vy + angle) * dist >> FIX12_SHIFT;
}

/* MATCHING: non-void with no return keeps the first delay slot a nop. */
s32 func_800283A0(void) {
    s16 v;

    v = sGame.unk3A8;
    if (v < 13) {
        sGame.unk3AC = 0;
    } else if (v >= 26) {
        sGame.unk3AC = 2;
    } else {
        sGame.unk3AC = 1;
    }
}

void func_800283E4(void) {
    sGame.unk6 = 2;
    sGame.unk5 = 0;
    sGame.unk0 = 1;
    sGame.unk38E = 0;
    if (D_8009599C == 1) {
        sGame.unk2 = 0;
        sGame.unk3C8 = 60;
        D_8009599C = 0;
    }
    if ((sGame.unk3D0 & 0xF) == 3) {
        sGame.unk6 = 0x33;
    }
}

/* MATCHING: `one` is the 1 retail keeps in $a0 for both compares. */
void func_80028448(void) {
    s32 one;

    if (D_800958E8 != 0 && D_800958E8 % 10 == 0) {
        one = 1;
        if (D_800958AC == one) {
            if (D_800958A8 == 2) {
                goto skip;
            }
        } else if (D_800958A8 == 3) {
            goto skip;
        }
        if (D_8009EF4D[0] != one) {
            func_80042538(34);
        }
    skip:
        func_80028260(1);
    }
}

void func_800284F8(void) {}

void func_80028500(void) {}

/* MATCHING: non-void with no return keeps the second bnez's delay slot a nop. */
s32 func_80028508(void) {
    if (sGame.unk0 != 0 && (sGame.unk6 < 5 || sGame.unk6 >= 8) &&
        (sGame.unk38F != 1 || sGame.unk398 < 6) && !(D_80095964 & 0x1000) && (D_800957EC & 0x40)) {
        func_80042538(22);
        sGame.unk6 = 5;
    }
}

void func_800285B0(void) {
    sGame.unk398 = 0;
    sGame.unk390 = 2;
}

void func_800285C8(s32 deg, s32 radius, VECTOR *out) {
    s32 angle;

    angle = ANGLE_DEG(deg);
    out->vx = rsin(angle) * radius >> FIX12_SHIFT;
    out->vz = rcos(angle) * radius >> FIX12_SHIFT;
}

void func_80028650(void) {
    s32 i;

    i = ((u32)D_80095864 >> 8) & 3;
    SetFogNearFar(3000, 8000, 250);
    SetFarColor(D_800958FC[i * 4], D_800958FC[i * 4 + 1], D_800958FC[i * 4 + 2]);
}

/* MATCHING: the red channel through its own local moves the spill to y2, as retail. */
void func_800286B0(u16 col, s16 x0, s16 y0, s16 x1, s16 y1, s16 x2, s16 y2, s16 x3, s16 y3, u16 pri) {
    POLY_F4 poly;
    POLY_F4 *p;
    s32 r;

    p = &poly;
    SetPolyF4(p);
    r = col & 0x1F;
    p->r0 = r << 3;
    p->g0 = ((col >> 5) & 0x1F) << 3;
    p->b0 = ((col >> 10) & 0x1F) << 3;
    p->x0 = x0;
    p->y0 = y0;
    p->x1 = x1;
    p->y1 = y1;
    p->x2 = x3;
    p->y2 = y3;
    p->x3 = x2;
    p->y3 = y2;
    GsSortPoly(p, &D_800ACEA8[D_80095750], pri);
}

void func_800287C0(void) {
    D_8009EF20[0] = 0;
    D_800957D2 = 0x280;
}

void func_800287DC(void) {
    D_8009EF20[0] = 0;
    D_800957D2 = 0;
}

void func_800287F4(void) {
    if ((D_8009EF48[0] >> 4) == 5) {
        D_800D8440[0] = (s32)D_800D8960;
        D_800D38DE[0] = 1;
    }
}

INCLUDE_RODATA("asm/nonmatchings/code_13068", D_80010950);

INCLUDE_RODATA("asm/nonmatchings/code_13068", D_80010AE0);

INCLUDE_RODATA("asm/nonmatchings/code_13068", D_80010B10);
