#include "common.h"
#include "memory.h"
#include "libapi.h"
#include "sys/file.h"
#include "libgte.h"
#include "libgpu.h"
#include "libgs.h"
#include "code_a0bc.h"
#include "code_13068.h"
#include "code_1a098.h"

/** @brief One of the 80 entries heading a record bank: where the entry's
 *         records start. */
typedef struct {
    s32 first; /**< index in the bank's records of the entry's first one */
    s32 unk4;  /**< not yet known */
} BankEntry;

/** @brief A 0x2C-byte record of the first of two record banks. */
typedef struct {
    u8 unk0[0x14]; /**< not yet known */
    s32 unk14;     /**< ten times the menu's line-0 value */
    u8 unk18;      /**< the menu's line-1 value */
    u8 pad19[3];   /**< not yet known */
    s32 unk1C;     /**< negated x of the game state's position */
    s32 unk20;     /**< y of the game state's position */
    s32 unk24;     /**< negated z of the game state's position */
    s16 unk28;     /**< index of the owning entry */
    u8 pad2A[2];   /**< not yet known */
} Rec2C;

/** @brief Three halfwords copied as one 6-byte unit. */
typedef struct {
    s16 x; /**< not yet known */
    s16 y; /**< not yet known */
    s16 z; /**< not yet known */
} Pt6;

/** @brief A 0x4C-byte record of the second of two record banks. */
typedef struct {
    Pt6 pts[4];     /**< copied from the dispatch's object */
    u8 unk18[0x30]; /**< not yet known */
    void *unk48;    /**< the owning 0x78-byte record's unk10 */
} Rec4C;

/** @brief The first record bank: 80 entries, then 0x2C-byte records. */
typedef struct {
    BankEntry entries[80]; /**< per-entry start indices */
    Rec2C recs[1];         /**< the records; real count unknown */
} Bank2C;

/** @brief The second record bank: 80 entries, then 0x4C-byte records. */
typedef struct {
    BankEntry entries[80]; /**< per-entry start indices */
    Rec4C recs[1];         /**< the records; real count unknown */
} Bank4C;

Rec2C *func_80036A50(s32 idx, s32 sub);
Rec4C *func_80036A84(s32 idx, s32 sub);

extern u8 D_800D3CA8[]; /**< 0x44C0-byte buffer, cleared as a whole */
extern u8 D_800DB2C0[]; /**< 0x1DB0-byte buffer, cleared as a whole */

extern u8 D_80095A29; /**< state of that dispatch: 0 or 1 */

extern s16 D_80095A30; /**< current index, clamped to the entry count */

void func_80034F38(void);
void func_80036F50(void);

/** @brief Three words, of which the drawing reads the low halves. */
typedef struct {
    s32 x; /**< not yet known */
    s32 y; /**< not yet known */
    s32 z; /**< not yet known */
} Vec3W;

/** @brief An object reset when the two-state dispatch enters state 1. */
typedef struct {
    Pt6 pts[4];     /**< copied into a new second-bank record */
    Vec3W unk18[4]; /**< corner offsets drawn around the owner */
    void *unk48;    /**< points into the current entry's record */
} Obj48;

/** @brief What an Obj48's unk48 points at, as the drawing reads it. */
typedef struct {
    u8 pad0[0x18]; /**< not reached here */
    VECTOR pos;    /**< the owner's position */
} ObjOwner;

extern Obj48 D_800DF9C0;  /**< reset by the dispatch's state 0 */
extern Rec4C *D_80095A34; /**< the second-bank record inserted last */
extern Rec2C *D_80095A44; /**< the first-bank record inserted last */
extern u8 D_80095A59;     /**< state of the second dispatch: 0 or 1 */
extern s16 D_80095A54;    /**< set to 100 on entering state 1 */
extern s16 D_80095A56;    /**< set to 100 on entering state 1 */

void func_80034BCC(void);
/* MATCHING: a per-unit view; code_1a098 defines it as (s16 (*)[3], VECTOR *,
 * s16, s16), a box's corners around a position. */
void func_8002A98C(Obj48 *obj, u8 *p, s32 a, s32 b);

extern char D_800956A4[]; /**< the menu's title */
extern char D_80095668[]; /**< marker of the highlighted line */
extern char D_80095670[]; /**< marker of the other lines */
extern char D_8001175C[]; /**< format of one numbered line */


extern u16 D_80095A3C; /**< saved value of menu line 0 */
extern u8 D_80095A48;  /**< saved value of menu line 1 */

/** @brief The three words of the game state this unit resets. */
typedef struct {
    u8 pad0[0x348]; /**< not reached here */
    s32 unk348;     /**< cleared on a reset */
    s32 unk34C;     /**< cleared on a reset */
    s32 unk350;     /**< cleared on a reset */
} GameStatePos;

/* MATCHING: a struct lvalue keeps the base in a register. */
#define sGamePos (*(GameStatePos *)D_8009EB78)

extern u8 D_80095A61;
extern u8 D_80095A24;
extern s16 D_80095A26;
extern u8 D_80095A60;
extern s16 D_80095A22;
extern s32 D_80095A38;
extern s32 D_80095A5C;
extern s32 D_80095A2C;
extern s32 D_80095A40;
extern u8 D_80095A58;
extern u8 D_80095A28;

void func_80036E50(void);
void func_80036EA0(void);

extern u8 D_80095774; /**< set while the reset below runs */

void func_80023F80(u8 *state);

extern char D_80011768[]; /**< path of the tool file, "sim:\\PS\\PEPSI\\DATA\\TOOL1\\TMP.TL1" */
extern char D_8001178C[]; /**< path of the first hit-data file, HITDATA0.T1D */
extern char D_800117B4[]; /**< path of the second hit-data file, HITDATA1.T1D */
extern char D_800117DC[]; /**< path of the third hit-data file, HITDATA2.T1D */

void func_80034388(void);
void func_800345C8(void);
void func_80034788(void);
void func_80034D5C(Obj48 *obj);
void func_800350C8(void);
void func_80035350(void);
void func_800355D8(void);
void func_800356FC(void);
void func_80035970(void);
void func_80035E24(void);
void func_80036478(VECTOR *pos);
void func_800365A0(VECTOR *pos);
void func_80036AB8(VECTOR *pos, u16 scale);
void func_80036EF0(void);
s32 func_80036FE8(void);
void func_8003708C(void);
void func_80037114(void);
void func_800371A0(void);
void func_80037280(void);
/* Defined by code_7d74, whose header does not declare it yet. */
void func_800179F8(u16 col, s16 x0, s16 y0, s16 x1, s16 y1, s16 x2, s16 y2, s16 x3, s16 y3, u16 pri);

/* MATCHING: the bob gets its own statement, or cc1 adds -200 to pos->vy. */
void func_80033F48(VECTOR *pos) {
    SVECTOR size;
    VECTOR world;
    SVECTOR screen;
    s32 bob;

    world.vx = pos->vx + D_800A7308[0];
    bob = ((rsin(D_8009585C * 10 % 360 * 4096 / 360) * 10) >> 12) - 200;
    world.vy = pos->vy + bob;
    world.vz = pos->vz + D_800A7308[2];
    func_800230E0(&world, &screen);
    size.vx = screen.vx - 7;
    size.vy = screen.vy - 32;
    func_8001B354(0x15E, &size, 0, 5, &D_800ACEA8[D_80095750]);
}

INCLUDE_RODATA("asm/nonmatchings/code_24748", D_800114DC);

INCLUDE_RODATA("asm/nonmatchings/code_24748", D_800114E8);

INCLUDE_RODATA("asm/nonmatchings/code_24748", D_800114F4);

INCLUDE_RODATA("asm/nonmatchings/code_24748", D_80011500);

INCLUDE_RODATA("asm/nonmatchings/code_24748", D_8001151C);

INCLUDE_RODATA("asm/nonmatchings/code_24748", D_80011528);

INCLUDE_RODATA("asm/nonmatchings/code_24748", D_80011534);

INCLUDE_RODATA("asm/nonmatchings/code_24748", D_80011540);

INCLUDE_RODATA("asm/nonmatchings/code_24748", D_8001154C);

INCLUDE_RODATA("asm/nonmatchings/code_24748", D_80011564);

INCLUDE_RODATA("asm/nonmatchings/code_24748", D_80011570);

INCLUDE_RODATA("asm/nonmatchings/code_24748", D_80011588);

INCLUDE_RODATA("asm/nonmatchings/code_24748", D_80011594);

INCLUDE_RODATA("asm/nonmatchings/code_24748", D_800115A0);

INCLUDE_RODATA("asm/nonmatchings/code_24748", D_800115AC);

INCLUDE_RODATA("asm/nonmatchings/code_24748", D_800115BC);

INCLUDE_RODATA("asm/nonmatchings/code_24748", D_800115D0);

INCLUDE_RODATA("asm/nonmatchings/code_24748", D_800115DC);

INCLUDE_RODATA("asm/nonmatchings/code_24748", D_80011618);

INCLUDE_RODATA("asm/nonmatchings/code_24748", D_80011624);

INCLUDE_RODATA("asm/nonmatchings/code_24748", D_80011634);

INCLUDE_RODATA("asm/nonmatchings/code_24748", D_80011644);

INCLUDE_RODATA("asm/nonmatchings/code_24748", D_80011654);

INCLUDE_RODATA("asm/nonmatchings/code_24748", D_80011664);

INCLUDE_RODATA("asm/nonmatchings/code_24748", D_80011678);

INCLUDE_RODATA("asm/nonmatchings/code_24748", D_80011688);

INCLUDE_RODATA("asm/nonmatchings/code_24748", D_80011698);

INCLUDE_RODATA("asm/nonmatchings/code_24748", D_800116AC);

INCLUDE_RODATA("asm/nonmatchings/code_24748", D_800116BC);

INCLUDE_RODATA("asm/nonmatchings/code_24748", D_800116D4);

INCLUDE_RODATA("asm/nonmatchings/code_24748", D_800116E0);

INCLUDE_RODATA("asm/nonmatchings/code_24748", D_800116F4);

INCLUDE_RODATA("asm/nonmatchings/code_24748", D_8001170C);

INCLUDE_RODATA("asm/nonmatchings/code_24748", D_8001171C);

INCLUDE_RODATA("asm/nonmatchings/code_24748", D_80011728);

INCLUDE_RODATA("asm/nonmatchings/code_24748", D_8001173C);

INCLUDE_RODATA("asm/nonmatchings/code_24748", D_8001174C);

INCLUDE_RODATA("asm/nonmatchings/code_24748", D_8001175C);

INCLUDE_RODATA("asm/nonmatchings/code_24748", D_80011768);

INCLUDE_RODATA("asm/nonmatchings/code_24748", D_8001178C);

INCLUDE_RODATA("asm/nonmatchings/code_24748", D_800117B4);

INCLUDE_RODATA("asm/nonmatchings/code_24748", D_800117DC);

void func_80034070(void) {
    switch (D_80095760) {
        case 0:
            D_800A7308[0] = 0;
            D_800A7308[2] = 0;
            D_8009588E = 80;
            bzero((u8 *)0x8016D000, 0x10000);
            *(s32 *)0x8016D000 = 80;
            D_80095780 = 80;
            D_80095A50 = 0x8016D004;
            func_80036E50();
            bzero((u8 *)0x8017D000, 0x10000);
            *(s32 *)0x8017D000 = 80;
            D_80095810 = 80;
            D_80095A4C = 0x8017D004;
            func_80036EA0();
            sGamePos.unk348 = 0;
            sGamePos.unk34C = 0;
            sGamePos.unk350 = 0;
            D_80095A61 = 10;
            D_80095A24 = 2;
            D_80095A26 = 10;
            D_80095A3C = 5;
            D_80095A60 = 0;
            D_80095A22 = 0;
            D_80095A30 = 0;
            D_80095A29 = 0;
            D_80095A38 = 0;
            D_80095A5C = 0;
            D_80095A2C = 0;
            D_80095A40 = 0;
            D_80095A58 = 0;
            D_80095A28 = 0;
            D_800958DA = 0;
            D_8009574A = 0;
            D_80095748 = 0;
            D_800956F7 = 1;
            D_80095760++;
            break;
        case 1:
            switch ((u16)D_800958DA) {
                case 0:
                    func_80034388();
                    func_80036EF0();
                    if (D_80095A29 == 1 && (D_80095970 & 0x40)) {
                        D_80095A59 = 0;
                    }
                    break;
                case 1:
                    func_800345C8();
                    break;
                case 2:
                    func_800371A0();
                    break;
                case 3:
                    func_800350C8();
                    break;
                case 4:
                    D_80095A59 = 0;
                    D_800958DA = 0;
                    break;
                case 5:
                    func_80035350();
                    break;
                case 6:
                    break;
                case 7:
                    func_80035970();
                    break;
            }
            func_80034788();
            func_80036FE8();
            func_80037114();
            if ((u16)(D_800958DA - 6) >= 2) {
                func_800355D8();
            }
            func_800356FC();
            if (D_80095A29 == 1 && D_80095A59 == 1) {
                func_80034D5C(&D_800DF9C0);
                func_8002A98C(&D_800DF9C0, D_8009EEC0, D_80095A54, D_80095A56);
            }
            if (D_80095A58 == 0) {
                func_8003708C();
                func_80036478((VECTOR *)D_8009EEC0);
                func_800365A0((VECTOR *)D_8009EEC0);
                if (D_80095A29 == 0) {
                    func_80036AB8((VECTOR *)D_8009EEC0, (s16)D_80095A3C * 10);
                }
            }
            break;
    }
}

extern s32 D_80095958; /**< a pad word; bits 12 and 14 step the position's y */

void func_80034388(void) {
    s32 flags;

    if (D_80095A59 == 1) {
        flags = D_80095970;
        if (flags & 0x1000) {
            D_80095A56 += D_80095A26;
        }
        if (flags & 0x4000) {
            D_80095A56 -= D_80095A26;
        }
        if (flags & 0x2000) {
            D_80095A54 += D_80095A26;
        }
        if (flags & 0x8000) {
            D_80095A54 -= D_80095A26;
        }
        D_80095A54 = D_80095A54 < 0 ? 0 : D_80095A54 > 1000 ? 1000 : D_80095A54;
        D_80095A56 = D_80095A56 < 0 ? 0 : D_80095A56 > 1000 ? 1000 : D_80095A56;
    } else {
        if (D_80095970 & 0x1000) {
            sGamePos.unk350 -= D_80095A26;
        }
        if (D_80095970 & 0x4000) {
            sGamePos.unk350 += D_80095A26;
        }
        if (D_80095970 & 0x2000) {
            sGamePos.unk348 -= D_80095A26;
        }
        if (D_80095970 & 0x8000) {
            sGamePos.unk348 += D_80095A26;
        }
    }
    if (D_80095958 & 0x1000) {
        sGamePos.unk34C -= D_80095A26;
    }
    if (D_80095958 & 0x4000) {
        sGamePos.unk34C += D_80095A26;
    }
    if (D_80095970 & 0x10) {
        D_80095A59 = 0;
        D_800958DA = 1;
    }
}

void func_800345C8(void) {
    D_800958B0 = 1;
    D_800958B2 = 5;
    func_800330D4();
    if (D_80095970 & 0x20) {
        switch (D_8009574A) {
            case 0:
                D_80095A29 = D_8009574A;
                D_800958B0 = 0x33;
                D_800958B2 = 2;
                D_8009574A = 0;
                D_800958DA = 2;
                D_80095A58 = 0;
                D_80095A28 = 0;
                D_80095748 = D_80095A3C;
                break;
            case 1:
                D_80095A29 = D_8009574A;
                D_8009574A = 0;
                D_80095748 = 0;
                D_800958DA = 4;
                D_80095A58 = 0;
                D_80095A28 = 0;
                break;
            case 2:
                D_80095A28 = 0;
                D_80095A58 ^= 1;
            case 3:
                if (D_8009574A == 3) {
                    D_80095A58 = D_80095A28 = D_80095A28 ^ 1;
                }
                D_800958A6 = 0;
                if (D_80095A58 == 1) {
                    switch (D_80095A29) {
                        case 0:
                            D_800958DA = 3;
                            break;
                        case 1:
                            D_800958DA = 5;
                            break;
                    }
                }
                break;
            case 4:
                D_800958A6 = 0;
                D_800958DA = 7;
                break;
        }
    }
    if (D_80095970 & 0x40) {
        D_800958DA = 0;
    }
}

extern s32 D_800959A0; /**< printed first on the status panel */
extern char D_800114DC[];
extern char D_800114E8[];
extern char D_800114F4[];
extern char D_80011500[];
extern char D_8001151C[];
extern char D_80011528[];
extern char D_80011534[];
extern char D_80011540[];
extern char D_8001154C[];
extern char D_80011564[];
extern char D_80011570[];
extern char D_80011588[];
extern char D_80011594[];
extern char D_800115A0[];
extern char D_800115AC[];
extern char D_800115BC[];
extern char D_800115D0[];
extern char D_80095658[];
extern char D_80095660[];
extern char D_80095678[];
extern char D_80095680[];
extern char D_80095688[];
extern char D_80095690[];

void func_80034788(void) {
    FntPrint(D_800114DC, D_800959A0);
    FntPrint(D_800114E8, D_80095A30);
    FntPrint(D_800114F4, D_80095A26);
    FntPrint(D_80011500, sGamePos.unk348, sGamePos.unk34C, sGamePos.unk350);
    FntPrint(D_8001151C);
    switch (D_80095A29) {
        case 0:
            FntPrint(D_80011528);
            break;
        case 1:
            FntPrint(D_80011534);
            break;
    }
    if (D_80095A28 == 1) {
        FntPrint(D_80011540);
    } else if (D_80095A58 == 1) {
        FntPrint(D_8001154C);
    }
    FntPrint(D_80095658);
    if (D_80095A28 == 1) {
        switch (D_80095A29) {
            case 0:
                FntPrint(D_80095660, D_80095A44->unk14);
                FntPrint(D_80011564, D_80095A44->unk18);
                FntPrint(D_80011570, -D_80095A44->unk1C, D_80095A44->unk20, -D_80095A44->unk24);
                break;
            case 1:
                FntPrint(D_80011588);
                break;
        }
    }
    switch ((u16)D_800958DA) {
        case 1:
            D_80095A58 = 0;
            D_80095A28 = 0;
            if (D_8009574A == 0) {
                FntPrint(D_80095668);
            } else {
                FntPrint(D_80095670);
            }
            func_80014BF0(7);
            FntPrint(D_80011594);
            if (D_8009574A == 1) {
                FntPrint(D_80095668);
            } else {
                FntPrint(D_80095670);
            }
            func_80014BF0(7);
            FntPrint(D_800115A0);
            if (D_8009574A == 2) {
                FntPrint(D_80095668);
            } else {
                FntPrint(D_80095670);
            }
            func_80014BF0(7);
            FntPrint(D_80095678);
            if (D_8009574A == 3) {
                FntPrint(D_80095668);
            } else {
                FntPrint(D_80095670);
            }
            func_80014BF0(7);
            FntPrint(D_80095680);
            if (D_8009574A == 4) {
                FntPrint(D_80095668);
            } else {
                FntPrint(D_80095670);
            }
            func_80014BF0(7);
            FntPrint(D_80095688);
            func_800179F8(0x2821, 60, -68, 140, -68, 140, 12, 60, 12, 0);
            break;
        case 2:
            FntPrint(D_80095690);
            func_80014BF0(3);
            FntPrint(D_800115AC);
            func_80014BF0(3);
            if (D_8009574A == 0) {
                FntPrint(D_80095668);
            } else {
                FntPrint(D_80095670);
            }
            FntPrint(D_800115BC, (s16)D_80095A3C * 10);
            func_80014BF0(3);
            if (D_8009574A == 1) {
                FntPrint(D_80095668);
            } else {
                FntPrint(D_80095670);
            }
            FntPrint(D_800115D0, D_80095A48);
            func_800179F8(0x2821, -80, -30, 80, -30, 80, 30, -80, 30, 0);
            break;
        case 7:
            func_80035E24();
            break;
    }
}

/* MATCHING: the last loop enters at its test (a for or while is rotated). */
void func_80034BCC(void) {
    Bank4C *bank;
    BankEntry *e;
    Rec4C *recs;
    Rec4C *src;
    Rec4C *dst;
    Rec4C *rec;
    s32 k;
    s32 n;

    bank = (Bank4C *)D_80095A4C;
    e = bank->entries;
    e += D_80095A30;
    recs = (Rec4C *)&bank->entries[D_8009588E];
    src = &recs[98];
    dst = &recs[99];
    for (k = e->first + e->unk4; k < 100; k++) {
        *dst = *src;
        dst--;
        src--;
    }
    bank = (Bank4C *)D_80095A4C;
    e = bank->entries;
    e += D_80095A30;
    rec = func_80036A84(D_80095A30, e->unk4);
    D_80095A34 = rec;
    for (k = 0; k < 4; k++) {
        rec->pts[k].x = D_800DF9C0.pts[k].x;
        rec->pts[k].y = D_800DF9C0.pts[k].y;
        rec->pts[k].z = D_800DF9C0.pts[k].z;
    }
    D_80095A34->unk48 = D_800DF9C0.unk48;
    e = (BankEntry *)D_80095A4C;
    e += D_80095A30;
    e->unk4++;
    n = D_8009588E;
    k = D_80095A30 + 1;
    goto test;
    do {
        e++;
        e->first++;
        k++;
    test:;
    } while (k < n);
    D_80095A2C++;
    func_80036878();
}

void func_80028888(Obj48 *obj);

void func_80034D5C(Obj48 *obj) {
    VECTOR tmp;
    Pt6 scr[4];
    VECTOR world;
    Pt6 pts[4];
    s32 unused[6];
    s16 i;

    func_80028888(obj);
    world.vx = ((ObjOwner *)obj->unk48)->pos.vx + D_800A7308[0];
    world.vy = ((ObjOwner *)obj->unk48)->pos.vy;
    world.vz = ((ObjOwner *)obj->unk48)->pos.vz + D_800A7308[2];
    for (i = 0; i < 4; i++) {
        pts[i].x = obj->unk18[i].x;
        pts[i].y = obj->unk18[i].y;
        pts[i].z = obj->unk18[i].z;
    }
    tmp.vx = world.vx + pts[0].x;
    tmp.vy = pts[0].y;
    tmp.vz = world.vz + pts[0].z;
    func_800230E0(&tmp, (SVECTOR *)&scr[0]);
    tmp.vx = world.vx + pts[1].x;
    tmp.vy = pts[1].y;
    tmp.vz = world.vz + pts[1].z;
    func_800230E0(&tmp, (SVECTOR *)&scr[1]);
    tmp.vx = world.vx + pts[3].x;
    tmp.vy = pts[3].y;
    tmp.vz = world.vz + pts[3].z;
    func_800230E0(&tmp, (SVECTOR *)&scr[2]);
    tmp.vx = world.vx + pts[2].x;
    tmp.vy = pts[2].y;
    tmp.vz = world.vz + pts[2].z;
    func_800230E0(&tmp, (SVECTOR *)&scr[3]);
    func_800179F8(0x3E0, scr[0].x, scr[0].y, scr[1].x, scr[1].y, scr[2].x, scr[2].y, scr[3].x,
                  scr[3].y, 0x32);
}

/* MATCHING: the last loop enters at its test (a for or while is rotated). */
void func_80034F38(void) {
    Bank2C *bank;
    BankEntry *e;
    Rec2C *recs;
    Rec2C *src;
    Rec2C *dst;
    Rec2C *rec;
    s32 k;
    s32 n;

    bank = (Bank2C *)D_80095A50;
    e = bank->entries;
    e += D_80095A30;
    recs = (Rec2C *)&bank->entries[D_8009588E];
    src = &recs[398];
    dst = &recs[399];
    for (k = e->first + e->unk4; k < 400; k++) {
        *dst = *src;
        dst--;
        src--;
    }
    bank = (Bank2C *)D_80095A50;
    e = bank->entries;
    e += D_80095A30;
    rec = func_80036A50(D_80095A30, e->unk4);
    rec->unk1C = -sGamePos.unk348;
    rec->unk20 = sGamePos.unk34C;
    D_80095A44 = rec;
    rec->unk24 = -sGamePos.unk350;
    rec->unk14 = (s16)D_80095A3C * 10;
    rec->unk28 = D_80095A30;
    rec->unk18 = D_80095A48;
    e = (BankEntry *)D_80095A50;
    e += D_80095A30;
    e->unk4++;
    n = D_8009588E;
    k = D_80095A30 + 1;
    goto test;
    do {
        e++;
        e->first++;
        k++;
    test:;
    } while (k < n);
    D_80095A38++;
    func_80036704();
}

#ifdef NON_MATCHING
void func_800350C8(void) {
    VECTOR pos;
    Bank2C *bank;
    BankEntry *e;
    Rec2C *src;
    Rec2C *dst;
    Rec2C *rec;
    s32 k;
    s32 n;

    switch (D_800958A6) {
        case 0:
            D_80095A38 = 0;
            D_800958A6 = 1;
            break;
        case 1:
            bank = (Bank2C *)D_80095A50;
            e = bank->entries;
            e += D_80095A30;
            if (e->unk4 == 0) {
                D_800958DA = 0;
                D_80095A58 = 0;
            }
            if (D_80095970 & 8) {
                D_80095A38++;
            }
            if (D_80095970 & 4) {
                D_80095A38--;
            }
            e = bank->entries;
            e += D_80095A30;
            D_80095A38 = D_80095A38<0 ? 0 : D_80095A38>(u32)(e->unk4 - 1) ? e->unk4 - 1 : D_80095A38;
            rec = func_80036A50(D_80095A30, D_80095A38);
            pos.vx = -rec->unk1C;
            pos.vy = rec->unk20;
            D_80095A44 = rec;
            pos.vz = -rec->unk24;
            if (D_80095970 & 0x40) {
                D_800958DA = 0;
                D_80095A58 = 0;
            }
            if (D_80095A28 != 1 && (D_80095970 & 0x20)) {
                bank = (Bank2C *)D_80095A50;
                e = bank->entries;
                e += D_80095A30;
                dst = (Rec2C *)&bank->entries[D_8009588E];
                src = &dst[e->first + D_80095A38 + 1];
                dst += e->first + D_80095A38;
                for (k = e->first + D_80095A38; k < 399; k++) {
                    *dst = *src;
                    dst++;
                    src++;
                }
                n = D_8009588E;
                /* MATCHING: a byte-pointer store keeps the index load below it. */
                *(s32 *)((u8 *)e + 4) -= 1;
                k = D_80095A30 + 1;
                for (; k < n; k++) {
                    e++;
                    e->first--;
                }
                func_80036704();
            }
            func_80036478(&pos);
            func_800365A0(&pos);
            break;
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/code_24748", func_800350C8);
#endif

#ifdef NON_MATCHING
void func_80035350(void) {
    VECTOR pos;
    Bank4C *bank;
    BankEntry *e;
    Rec4C *src;
    Rec4C *dst;
    Rec4C *rec;
    s32 k;
    s32 n;

    switch (D_800958A6) {
        case 0:
            D_80095A2C = 0;
            D_800958A6 = 1;
            break;
        case 1:
            bank = (Bank4C *)D_80095A4C;
            e = bank->entries;
            e += D_80095A30;
            if (e->unk4 == 0) {
                D_800958DA = 0;
                D_80095A58 = 0;
            }
            if (D_80095970 & 8) {
                D_80095A2C++;
            }
            if (D_80095970 & 4) {
                D_80095A2C--;
            }
            e = bank->entries;
            e += D_80095A30;
            D_80095A2C = D_80095A2C<0 ? 0 : D_80095A2C>(u32)(e->unk4 - 1) ? e->unk4 - 1 : D_80095A2C;
            rec = func_80036A84(D_80095A30, D_80095A2C);
            pos.vx = -rec->pts[0].x;
            pos.vy = rec->pts[0].y;
            D_80095A34 = rec;
            pos.vz = -rec->pts[0].z;
            if (D_80095970 & 0x40) {
                D_800958DA = 0;
                D_80095A58 = 0;
            }
            if (D_80095A28 != 1 && (D_80095970 & 0x20)) {
                bank = (Bank4C *)D_80095A4C;
                e = bank->entries;
                e += D_80095A30;
                dst = (Rec4C *)&bank->entries[D_8009588E];
                src = &dst[e->first + D_80095A2C + 1];
                dst += e->first + D_80095A2C;
                for (k = e->first + D_80095A2C; k < 99; k++) {
                    *dst = *src;
                    dst++;
                    src++;
                }
                n = D_8009588E;
                /* MATCHING: a byte-pointer store keeps the index load below it. */
                *(s32 *)((u8 *)e + 4) -= 1;
                k = D_80095A30 + 1;
                for (; k < n; k++) {
                    e++;
                    e->first--;
                }
                func_80036878();
            }
            func_80036478(&pos);
            func_800365A0(&pos);
            break;
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/code_24748", func_80035350);
#endif

/* MATCHING: the first clamp is a preloaded local with nested ifs; a nested
 * ternary like the second one moves the value through another register. */
void func_800355D8(void) {
    s32 flags = D_80095970;
    u8 v;

    if (flags & 1) {
        D_80095A60++;
        D_80095A22 = D_800958DA;
        D_800958DA = 6;
        D_80095A24 = 0;
    }
    if (flags & 2) {
        D_80095A60--;
        D_80095A22 = D_800958DA;
        D_800958DA = 6;
        D_80095A24 = 0;
    }
    if (flags & 0x80) {
        D_80095A61++;
        D_80095A22 = D_800958DA;
        D_800958DA = 6;
        D_80095A24 = 0;
    }
    if (flags & 0x40) {
        D_80095A61--;
        D_80095A22 = D_800958DA;
        D_800958DA = 6;
        D_80095A24 = 0;
    }
    v = D_80095A61;
    if ((s8)D_80095A61 > 0) {
        if ((s8)D_80095A61 > 20) {
            v = 20;
        }
    } else {
        v = 1;
    }
    D_80095A61 = v;
    D_80095A60 = (s8)D_80095A60 < 0 ? 4 : (s8)D_80095A60 > 4 ? 0 : D_80095A60;
}

/** @brief Five rows of three words, copied from rodata as one block. */
typedef struct {
    s32 v[5][3]; /**< per-row x, y, z */
} Rows5;

extern Rows5 D_800115DC; /**< the rows the step below scales */
extern s32 D_800CEBA0[]; /**< start of the interpolation */
extern u8 D_80095A20;    /**< interpolation step, 0..30 */
extern s32 D_800DB2B8[]; /**< cleared on every step */

/* MATCHING: a per-unit view; code_7d74 defines it as (s16, s16, u16, u16)
 * returning s16. */
s32 func_80018D04(s32 a, s32 b, u16 t, u16 n);

#ifdef NON_MATCHING
/* MATCHING: case 2 stores the switch value ($v1); retail stores the compare
 * constant ($v0). */
void func_800356FC(void) {
    Rows5 rows;

    rows = D_800115DC;
    switch (D_80095A24) {
        case 0:
            D_80095A24 = 1;
            D_80095A20 = 0;
            D_800CEBA0[0] = D_800DB2A0[0];
            D_800CEBA0[1] = D_800DB2A0[1];
            D_800CEBA0[2] = D_800DB2A0[2];
            break;
        case 1:
            D_800DB2A0[0] = func_80018D04(D_800CEBA0[0], rows.v[(s8)D_80095A60][0] * (s8)D_80095A61,
                                          D_80095A20, 30);
            D_800DB2A0[1] = func_80018D04(D_800CEBA0[1], rows.v[(s8)D_80095A60][1] * (s8)D_80095A61,
                                          D_80095A20, 30);
            D_800DB2A0[2] = func_80018D04(D_800CEBA0[2], rows.v[(s8)D_80095A60][2] * (s8)D_80095A61,
                                          D_80095A20, 30);
            if (++D_80095A20 == 30) {
                D_80095A24++;
            }
            break;
        case 2:
            D_800958DA = D_80095A22;
            D_80095A24 = 2;
            break;
        case 3:
            D_800DB2A0[0] = rows.v[(s8)D_80095A60][0] * (s8)D_80095A61;
            D_800DB2A0[1] = rows.v[(s8)D_80095A60][1] * (s8)D_80095A61;
            D_800DB2A0[2] = rows.v[(s8)D_80095A60][2] * (s8)D_80095A61;
            D_800DB2A0[3] = 0;
            D_800DB2A0[4] = 0;
            D_800DB2A0[5] = 0;
            break;
    }
    D_800DB2B8[0] = 0;
}
#else
INCLUDE_ASM("asm/nonmatchings/code_24748", func_800356FC);
#endif

/* Defined by code_27bc8 and code_1dc24, which have no header. */
void func_80038124(void);
/* MATCHING: a per-unit view; code_27bc8 defines it returning s16, and
 * func_80035970 compares the result with -1 unextended. */
s32 func_80038394(void);
void func_800386A8(void);
s32 func_800389B4(s32 slot);
s32 func_8003356C(s16 n);
s32 func_8003634C(void);
s32 func_80037318(void);
s32 func_80037370(void);
void func_80036184(s16 n);
void func_80036B90(s16 n);

#ifdef NON_MATCHING
void func_80035970(void) {
    s32 err = 0;
    s32 r;

    switch (D_800958A6) {
        case 0:
            func_80038124();
            D_800959E4 = 0;
            D_8009574A = 0;
            D_80095748 = 0;
            D_800958A6++;
            break;
        case 1:
            switch (D_800959E4) {
                case 0:
                    D_80095A14 = 0;
                    err = func_800389B4(0);
                    if (err == 0) {
                        D_800959E4 = 1;
                    }
                    break;
                case 1:
                    if (func_80038394() == -1 || (D_80095970 & 0x40)) {
                        D_800959E4 = 18;
                        break;
                    }
                    D_800959E0 = func_8003356C(2);
                    switch (D_800959E0) {
                        case 2:
                            if (func_80037318() == -1) {
                                D_800959E4 = 11;
                            } else if (func_8003634C() == -1) {
                                D_800959E4 = 11;
                            } else {
                                D_800959E4 = 10;
                            }
                            break;
                        case 3:
                            if (func_80037370() == -1) {
                                D_800959E4 = 11;
                            } else {
                                D_800959E0 = 1;
                                D_800959E4 = 2;
                            }
                            break;
                        case -1:
                            break;
                        default:
                            D_8009574A = 0;
                            D_800959E4 = 2;
                            break;
                    }
                    break;
                case 2:
                    if (D_80095970 & 0x40) {
                        D_800959E4 = 1;
                        break;
                    }
                    D_80095A0C = func_8003356C(3);
                    if (D_80095A0C == -1) {
                        break;
                    }
                    switch (D_800959E0) {
                        case 0:
                            D_800959E4 = 3;
                            break;
                        case 1:
                            D_800959E4 = 5;
                            break;
                    }
                    break;
                case 10:
                    if (D_80095970 & 0x60) {
                        D_800959E4 = 1;
                    }
                    break;
                case 4:
                    r = func_8003356C(2);
                    switch (r) {
                        case 0:
                            D_800959E4 = 8;
                            break;
                        case 1:
                            D_800959E4 = 1;
                            break;
                    }
                    break;
                case 3:
                    D_800959E4 = 8;
                    break;
                case 8:
                    func_80036184(D_80095A0C);
                    err = func_800389B4(1);
                    if (err == 0) {
                        D_800959E4 = 10;
                    }
                    break;
                case 5:
                    D_800959E4 = 6;
                    D_8009574A = 0;
                    break;
                case 6:
                    r = func_8003356C(2);
                    if (r == 0) {
                        D_800959E4 = 9;
                    } else if (r == 1) {
                        D_800959E4 = 1;
                    }
                    break;
                case 7:
                    if (D_80095970 & 0x20) {
                        D_800959E4 = 5;
                    }
                    break;
                case 9:
                    func_80036B90(D_80095A0C);
                    func_80036704();
                    func_80036878();
                    D_800959E4 = 10;
                    break;
                case 11:
                case 13:
                    if (D_80095970 & 0x40) {
                        D_800959E4 = 18;
                    }
                    break;
                case 14:
                    r = func_8003356C(2);
                    switch (r) {
                        case 0:
                            D_800959E4 = 16;
                            break;
                        case 1:
                            D_800959E4 = 18;
                            break;
                        case -1:
                            break;
                    }
                    break;
                case 15:
                    r = func_8003356C(2);
                    switch (r) {
                        case 0:
                            D_800959E4 = 17;
                            break;
                        case 1:
                            D_800959E4 = 18;
                            break;
                        case -1:
                            break;
                    }
                    break;
                case 16:
                    err = func_800389B4(2);
                    if (err == 0) {
                        D_800959E4 = 0;
                    }
                    break;
                case 17:
                    err = func_800389B4(3);
                    if (err == 0) {
                        D_800959E4 = 0;
                    }
                    break;
                case 18:
                    D_800958A6++;
                    break;
            }
            switch (err) {
                case -2:
                case -1:
                    D_800959E4 = 11;
                    break;
                case 1:
                    D_800959E4 = 12;
                    break;
                case 2:
                    D_800959E4 = 13;
                    break;
                case 3:
                    D_800959E4 = 14;
                    break;
                case 4:
                    D_800959E4 = 15;
                    break;
            }
            break;
        case 2:
            func_800386A8();
            D_800958DA = 0;
            break;
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/code_24748", func_80035970);
#endif

extern char D_80095698[]; /**< a line prefix */
extern char D_8009569C[]; /**< not yet known */
extern char D_80011618[];
extern char D_80011624[];
extern char D_80011634[];
extern char D_80011644[];
extern char D_80011654[];
extern char D_80011664[];
extern char D_80011678[];
extern char D_80011688[];
extern char D_80011698[];
extern char D_800116AC[];
extern char D_800116BC[];
extern char D_800116D4[];
extern char D_800116E0[];
extern char D_800116F4[];
extern char D_8001170C[];
extern char D_8001171C[];
extern char D_80011728[];
extern char D_8001173C[];
extern char D_8001174C[];

/* Defined by code_1dc24, which has no header. */
void func_80033C90(void);
void func_80033D3C(void);

void func_80035E24(void) {
    func_80014BF0(5);
    FntPrint(D_80011618);
    FntPrint(D_80095698);
    switch (D_800959E4) {
        case 0:
            FntPrint(D_80095698);
            func_80014BF0(4);
            FntPrint(D_8009569C);
            break;
        case 1:
            FntPrint(D_80095698);
            func_80014BF0(3);
            FntPrint(D_80011624);
            func_80033D3C();
            break;
        case 2:
            FntPrint(D_80095698);
            func_80014BF0(3);
            FntPrint(D_80011634);
            func_80037280();
            break;
        case 16:
            FntPrint(D_80095698);
            func_80014BF0(3);
            FntPrint(D_80011644);
            break;
        case 17:
            FntPrint(D_80095698);
            func_80014BF0(3);
            FntPrint(D_80011654);
            break;
        case 4:
            FntPrint(D_80095698);
            func_80014BF0(3);
            FntPrint(D_80011664, D_80095A0C + 1);
            func_80014BF0(3);
            FntPrint(D_80011678);
            func_80033C90();
            break;
        case 8:
            FntPrint(D_80095698);
            func_80014BF0(3);
            FntPrint(D_80011688);
            break;
        case 6:
            FntPrint(D_80095698);
            func_80014BF0(3);
            FntPrint(D_80011698, D_80095A0C + 1);
            func_80014BF0(3);
            FntPrint(D_800116AC);
            func_80033C90();
            break;
        case 7:
            FntPrint(D_80095698);
            func_80014BF0(2);
            FntPrint(D_800116BC, D_80095A0C + 1);
            break;
        case 10:
            FntPrint(D_80095698);
            func_80014BF0(3);
            FntPrint(D_800116D4);
            break;
        case 11:
            FntPrint(D_80095698);
            func_80014BF0(2);
            FntPrint(D_800116E0);
            break;
        case 12:
            FntPrint(D_80095698);
            func_80014BF0(2);
            FntPrint(D_800116F4);
            break;
        case 13:
            FntPrint(D_80095698);
            func_80014BF0(3);
            FntPrint(D_8001170C);
            break;
        case 14:
            FntPrint(D_80095698);
            func_80014BF0(4);
            FntPrint(D_8001171C);
            func_80014BF0(3);
            FntPrint(D_80011728);
            func_80033C90();
            break;
        case 15:
            FntPrint(D_80095698);
            func_80014BF0(3);
            FntPrint(D_8001173C);
            func_80014BF0(3);
            FntPrint(D_8001174C);
            func_80033C90();
            break;
    }
    func_800179F8(0x421, -100, -60, 100, -60, 100, 60, -100, 60, 0);
}

/** @brief The first block of one hit-data slot. */
typedef struct {
    u8 b[0x4744]; /**< not yet known */
} HitBlockA;

/** @brief The second block of one hit-data slot. */
typedef struct {
    u8 b[0x2034]; /**< not yet known */
} HitBlockB;

/** @brief One hit-data slot: the two blocks a HITDATA file holds. */
typedef struct {
    HitBlockA a; /**< the first block */
    HitBlockB b; /**< the second block */
} HitSlot;

/** @brief The tool buffer: a header, then the three hit-data slots. */
typedef struct {
    u8 hdr[0x200];    /**< not yet known */
    HitSlot slots[3]; /**< one per HITDATA file */
} ToolBuf;

void func_80036184(s16 n) {
    ToolBuf *tool = (ToolBuf *)0x8018D000;

    tool->slots[n].a = *(HitBlockA *)0x8016D000;
    (&tool->slots[n])->b = *(HitBlockB *)0x8017D000;
}

s32 func_8003634C(void) {
    s32 fd;

    fd = open(D_8001178C, O_CREAT | O_WRONLY);
    if (write(fd, (void *)0x8018D200, 0x4744) == -1) {
        goto fail;
    }
    if (write(fd, (void *)0x80191944, 0x2034) == -1) {
        goto fail;
    }
    close(fd);
    fd = open(D_800117B4, O_CREAT | O_WRONLY);
    if (write(fd, (void *)0x80193978, 0x4744) == -1) {
        goto fail;
    }
    if (write(fd, (void *)0x801980BC, 0x2034) == -1) {
        goto fail;
    }
    close(fd);
    fd = open(D_800117DC, O_CREAT | O_WRONLY);
    if (write(fd, (void *)0x8019A0F0, 0x4744) == -1) {
        goto fail;
    }
    if (write(fd, (void *)0x8019E834, 0x2034) == -1) {
        goto fail;
    }
    close(fd);
    return 0;
fail:
    close(fd);
    return -1;
}

/* MATCHING: -pos->vx + ... loads the parameter's word before the global's. */
void func_80036478(VECTOR *pos) {
    SVECTOR size;
    VECTOR world;
    SVECTOR screen;
    s32 bob;

    world.vx = -pos->vx + D_800A7308[0];
    bob = ((rsin(D_8009585C * 10 % 360 * 4096 / 360) * 10) >> 12) - 200;
    world.vy = pos->vy + bob;
    world.vz = -pos->vz + D_800A7308[2];
    func_800230E0(&world, &screen);
    size.vx = screen.vx - 7;
    size.vy = screen.vy - 32;
    func_8001B354(0x15E, &size, 0, 5, &D_800ACEA8[D_80095750]);
}

/* MATCHING: an s16 colour local keeps 0xFF loaded after the sine. */
void func_800365A0(VECTOR *pos) {
    s32 unused[2];
    VECTOR world;
    SVECTOR screen;
    GsLINE line;
    s16 g;

    g = ((rsin(D_8009585C * 10 % 360 * 4096 / 360) * 50) >> 12) + 160;
    line.attribute = 0;
    line.r = 0xFF;
    line.g = g;
    line.b = g;
    world.vx = -pos->vx + D_800A7308[0];
    world.vy = pos->vy;
    world.vz = -pos->vz + D_800A7308[2];
    func_800230E0(&world, &screen);
    line.x0 = screen.vx;
    line.y0 = screen.vy;
    world.vy = pos->vy - 200;
    func_800230E0(&world, &screen);
    line.x1 = screen.vx;
    line.y1 = screen.vy;
    GsSortLine(&line, &D_800ACEA8[D_80095750], 50);
}

#ifdef NON_MATCHING
/* MATCHING: retail's 0x2A-byte copy is word-aligned; no type found for it. */
/** @brief The first 0x2A bytes of a 0x2C-byte record, copied as a block. */
typedef struct {
    s16 h[21]; /**< not yet known */
} Copy2A;

/** @brief A record of the first bank as the bank installer reads it. */
typedef struct {
    u8 unk0[0x28]; /**< copied as a block */
    s16 unk28;     /**< index of the owning 0x78-byte record */
    u8 pad[2];     /**< not copied */
} Bank2CRec;

/** @brief A record of the live buffer the first bank is copied into. */
typedef struct {
    u8 unk0[0x28]; /**< copied from the bank record */
    void *unk28;   /**< the owning record's unk10 */
} Live2C;

void func_80036704(void) {
    s16 i;
    BankEntry *e = ((Bank2C *)D_80095A50)->entries;
    Bank2CRec *rec;

    for (i = 0; i < (u32)D_80095780; i++) {
        D_800D8D20[i].unk72 = e->first;
        D_800D8D20[i].unk74 = e->unk4;
        e++;
    }
    for (i = 0; i < 400; i++) {
        rec = (Bank2CRec *)func_80036A50(0, i);
        *(Copy2A *)&((Live2C *)D_800D3CA8)[i] = *(Copy2A *)rec;
        ((Live2C *)D_800D3CA8)[i].unk28 = D_800D8D20[rec->unk28].unk10;
    }
}
#else
INCLUDE_ASM("asm/nonmatchings/code_24748", func_80036704);
#endif

void func_80036878(void) {
    s16 i;
    s16 j;
    BankEntry *e = ((Bank4C *)D_80095A4C)->entries;

    for (i = 0; i < (u32)D_80095810; i++) {
        D_800D8D20[i].unk6E = e->first;
        D_800D8D20[i].unk70 = e->unk4;
        e++;
    }
    for (i = 0; i < 80; i++) {
        for (j = D_800D8D20[i].unk6E; j < D_800D8D20[i].unk70 + D_800D8D20[i].unk6E; j++) {
            func_80036A84(0, j)->unk48 = D_800D8D20[i].unk10;
        }
    }
    for (i = 0; i < 100; i++) {
        ((Rec4C *)D_800DB2C0)[i] = *func_80036A84(0, i);
    }
}

Rec2C *func_80036A50(s32 idx, s32 sub) {
    Bank2C *bank = (Bank2C *)D_80095A50;
    Rec2C *recs = bank->recs;

    return &recs[bank->entries[idx].first + sub];
}

Rec4C *func_80036A84(s32 idx, s32 sub) {
    Bank4C *bank = (Bank4C *)D_80095A4C;
    Rec4C *recs = bank->recs;

    return &recs[bank->entries[idx].first + sub];
}

/* MATCHING: the chained assignment stores its rightmost target first. */
void func_80036AB8(VECTOR *pos, u16 scale) {
    SVECTOR size;
    CVECTOR color;
    GsCOORDINATE2 coord;
    MATRIX ls;

    GsInitCoordinate2(WORLD, &coord);
    coord.coord.t[0] = -pos->vx;
    coord.coord.t[1] = pos->vy;
    coord.coord.t[2] = -pos->vz;
    GsGetLs(&coord, &ls);
    GsSetLsMatrix(&ls);
    size.vx = size.vy = scale * 2;
    color.r = 0;
    color.g = color.b = color.cd = 0x80;
    func_8001A3D4(0x15D, &size, &color, 2, &D_800ACEA8[D_80095750]);
}

/* MATCHING: (&slot)->b puts the second address sum offset-first. */
void func_80036B90(s16 n) {
    ToolBuf *tool = (ToolBuf *)0x8018D000;

    *(HitBlockA *)0x8016D000 = tool->slots[n].a;
    *(HitBlockB *)0x8017D000 = (&tool->slots[n])->b;
}

void func_80036D50(void) {
    D_800A7308[0] = 0;
    D_800A7308[2] = 0;
    D_8009588E = 80;
    bzero((u8 *)0x8016D000, 0x10000);
    *(s32 *)0x8016D000 = 80;
    D_80095780 = 80;
    D_80095A50 = 0x8016D004;
    func_80036E50();
    bzero((u8 *)0x8017D000, 0x10000);
    *(s32 *)0x8017D000 = 80;
    D_80095810 = 80;
    D_80095A4C = 0x8017D004;
    func_80036EA0();
    sGamePos.unk348 = 0;
    sGamePos.unk34C = 0;
    sGamePos.unk350 = 0;
    D_80095A61 = 10;
    D_80095A24 = 2;
    D_80095A26 = 10;
    D_80095A60 = 0;
    D_80095A22 = 0;
    D_80095A30 = 0;
    D_80095A29 = 0;
    D_80095A38 = 0;
    D_80095A5C = 0;
    D_80095A2C = 0;
    D_80095A40 = 0;
    D_80095A3C = 5;
    D_80095A58 = 0;
    D_80095A28 = 0;
}

void func_80036E50(void) {
    u32 i;

    bzero(D_800D3CA8, 0x44C0);
    for (i = 0; i < 80; i++) {
        D_800D8D20[i].unk72 = 0;
        D_800D8D20[i].unk74 = -1;
    }
}

void func_80036EA0(void) {
    u32 i;

    bzero(D_800DB2C0, 0x1DB0);
    for (i = 0; i < 80; i++) {
        D_800D8D20[i].unk6E = 0;
        D_800D8D20[i].unk70 = -1;
    }
}

void func_80036EF0(void) {
    if (D_80095970 & 0x20) {
        switch (D_80095A29) {
            case 0:
                func_80034F38();
                break;
            case 1:
                func_80036F50();
                break;
        }
    }
}

void func_80036F50(void) {
    switch (D_80095A59) {
        case 0:
            D_80095A54 = 100;
            D_80095A56 = 100;
            D_800DF9C0.unk48 = D_800D8D20[D_80095A30].unk10;
            func_8002A98C(&D_800DF9C0, D_8009EEC0, 100, 100);
            D_80095A59 = 1;
            break;
        case 1:
            func_80034BCC();
            D_80095A59 = 0;
            break;
    }
}

/* MATCHING: s32 with no return; the jump table lands 4 bytes late. */
s32 func_80036FE8(void) {
    if (D_80095970 & 0x100) {
        switch (D_80095A26) {
            case 10:
                D_80095A26 = 20;
                break;
            case 20:
                D_80095A26 = 30;
                break;
            case 30:
                D_80095A26 = 40;
                break;
            case 40:
                D_80095A26 = 50;
                break;
            case 50:
                D_80095A26 = 10;
                break;
        }
    }
}

void func_8003708C(void) {
    s32 flags = D_80095970;

    if (flags & 8) {
        D_80095A30++;
    }
    if (flags & 4) {
        D_80095A30--;
    }
    D_80095A30 = D_80095A30 < 0 ? 0 : D_80095A30 > D_8009588E - 1 ? D_8009588E - 1 : D_80095A30;
}

#ifdef NON_MATCHING
void func_80037114(void) {
    D_80095774 = 1;
    func_8002980C();
    D_800A9008[0] = 0;
    D_800A9008[1] = 0;
    D_800A9008[2] = 0;
    ((s16 *)D_800A9008)[12] = 0;
    ((s16 *)D_800A9008)[13] = 0;
    ((s16 *)D_800A9008)[14] = 0;
    D_800A9008[11] = 0;
    D_800A9008[12] = 0;
    func_8002A7D8(&D_800D8D20[D_80095A30], (Rec48 *)D_800A9008);
    func_80023F80(D_8009EB78);
    func_80029838();
    D_80095774 = 0;
}
#else
INCLUDE_ASM("asm/nonmatchings/code_24748", func_80037114);
#endif

/* MATCHING: the unused 8 bytes give the 0x20-byte frame. */
void func_800371A0(void) {
    s32 unused[2];
    s16 line = D_8009574A;

    func_800330D4();
    if (line != D_8009574A) {
        switch (D_8009574A) {
            case 0:
                D_800958B0 = 0x33;
                D_80095748 = D_80095A3C;
                break;
            case 1:
                D_800958B0 = 0x100;
                D_80095748 = D_80095A48;
                break;
        }
    } else {
        switch (line) {
            case 0:
                D_80095A3C = D_80095748;
                break;
            case 1:
                D_80095A48 = D_80095748;
                break;
        }
    }
    if (D_80095970 & 0x40) {
        D_800958DA = 0;
    }
}

/* MATCHING: one call per arm; a ternary argument shares one %hi register. */
void func_80037280(void) {
    s16 i;

    FntPrint(D_800956A4);
    for (i = 0; i < 3; i++) {
        func_80014BF0(4);
        if (D_8009574A == i) {
            FntPrint(D_80095668);
        } else {
            FntPrint(D_80095670);
        }
        FntPrint(D_8001175C, i + 1);
    }
}

s32 func_80037318(void) {
    s32 fd;
    s32 n;

    fd = open(D_80011768, O_CREAT | O_WRONLY);
    n = write(fd, (void *)0x8018D000, 0x13868);
    close(fd);
    return n;
}

s32 func_80037370(void) {
    s32 fd;
    s32 n;

    fd = open(D_80011768, O_RDONLY);
    n = read(fd, (void *)0x8018D000, 0x13868);
    close(fd);
    return n;
}
