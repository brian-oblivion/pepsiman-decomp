#include "common.h"
#include "memory.h"
#include "libapi.h"
#include "sys/file.h"
#include "libgte.h"
#include "libgpu.h"
#include "libgs.h"
#include "code_a0bc.h"
#include "code_13068.h"

/** @brief One of the 80 entries heading a record bank: where the entry's
 *         records start. */
typedef struct {
    s32 first; /**< index in the bank's records of the entry's first one */
    s32 unk4;  /**< not yet known */
} BankEntry;

/** @brief A 0x2C-byte record of the first of two record banks. */
typedef struct {
    u8 unk0[0x2C]; /**< not yet known */
} Rec2C;

/** @brief A 0x4C-byte record of the second of two record banks. */
typedef struct {
    u8 unk0[0x4C]; /**< not yet known */
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

/** @brief A 0x78-byte record of an 80-entry table; two pairs of halfwords
 *         are reset together. */
typedef struct {
    u8 unk0[0x10];  /**< not yet known */
    u8 unk10[0x5E]; /**< handed to the dispatch's object on entering state 1 */
    s16 unk6E;      /**< zeroed when the second buffer is cleared */
    s16 unk70;      /**< -1 when the second buffer is cleared */
    s16 unk72;      /**< zeroed when the first buffer is cleared */
    s16 unk74;      /**< -1 when the first buffer is cleared */
    u8 unk76[2];    /**< not yet known */
} Rec78;

extern u8 D_800D3CA8[];    /**< 0x44C0-byte buffer, cleared as a whole */
extern u8 D_800DB2C0[];    /**< 0x1DB0-byte buffer, cleared as a whole */
extern Rec78 D_800D8D20[]; /**< 80 records */

extern s32 D_80095970; /**< flag word; bit 5 enables a two-state dispatch */
extern u8 D_80095A29;  /**< state of that dispatch: 0 or 1 */

extern s16 D_80095A30; /**< current index, clamped to the entry count */
extern s16 D_8009588E; /**< number of entries */

void func_80034F38(void);
void func_80036F50(void);

/** @brief An object reset when the two-state dispatch enters state 1. */
typedef struct {
    u8 unk0[0x48]; /**< not yet known */
    void *unk48;   /**< points into the current entry's record */
} Obj48;

extern Obj48 D_800DF9C0; /**< reset by the dispatch's state 0 */
extern u8 D_8009EEC0[];  /**< passed with it */
extern u8 D_80095A59;    /**< state of the second dispatch: 0 or 1 */
extern s16 D_80095A54;   /**< set to 100 on entering state 1 */
extern s16 D_80095A56;   /**< set to 100 on entering state 1 */

void func_80034BCC(void);
void func_8002A98C(Obj48 *obj, u8 *p, s32 a, s32 b);

extern s16 D_8009574A;    /**< the highlighted line of a three-line menu */
extern char D_800956A4[]; /**< the menu's title */
extern char D_80095668[]; /**< marker of the highlighted line */
extern char D_80095670[]; /**< marker of the other lines */
extern char D_8001175C[]; /**< format of one numbered line */


extern u16 D_80095A3C; /**< saved value of menu line 0 */
extern u8 D_80095A48;  /**< saved value of menu line 1 */
extern s16 D_800958B0; /**< limit of the value being edited */
extern s16 D_800958DA; /**< cleared when flag bit 6 is set */

/** @brief The three words of the game state this unit resets. */
typedef struct {
    u8 pad0[0x348]; /**< not reached here */
    s32 unk348;     /**< cleared on a reset */
    s32 unk34C;     /**< cleared on a reset */
    s32 unk350;     /**< cleared on a reset */
} GameStatePos;

/* MATCHING: a struct lvalue keeps the base in a register. */
#define sGamePos (*(GameStatePos *)D_8009EB78)

extern s32 D_80095780; /**< entry count of the first record bank */
extern s32 D_80095810; /**< entry count of the second record bank */
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

extern u32 D_8009585C; /**< a frame counter driving the marker's bob */

void func_8001B354(u16 id, SVECTOR *pos, s32 a, s32 b, GsOT *ot);

extern u8 D_80095774; /**< set while the reset below runs */

void func_8002A7D8(Rec78 *rec);
void func_80023F80(u8 *state);

extern char D_80011768[]; /**< path of the tool file, "sim:\\PS\\PEPSI\\DATA\\TOOL1\\TMP.TL1" */

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

INCLUDE_ASM("asm/nonmatchings/code_24748", func_80034070);

INCLUDE_ASM("asm/nonmatchings/code_24748", func_80034388);

INCLUDE_ASM("asm/nonmatchings/code_24748", func_800345C8);

INCLUDE_ASM("asm/nonmatchings/code_24748", func_80034788);

INCLUDE_ASM("asm/nonmatchings/code_24748", func_80034BCC);

INCLUDE_ASM("asm/nonmatchings/code_24748", func_80034D5C);

INCLUDE_ASM("asm/nonmatchings/code_24748", func_80034F38);

INCLUDE_ASM("asm/nonmatchings/code_24748", func_800350C8);

INCLUDE_ASM("asm/nonmatchings/code_24748", func_80035350);

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

INCLUDE_ASM("asm/nonmatchings/code_24748", func_800356FC);

INCLUDE_ASM("asm/nonmatchings/code_24748", func_80035970);

INCLUDE_ASM("asm/nonmatchings/code_24748", func_80035E24);

INCLUDE_ASM("asm/nonmatchings/code_24748", func_80036184);

INCLUDE_ASM("asm/nonmatchings/code_24748", func_8003634C);

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

INCLUDE_ASM("asm/nonmatchings/code_24748", func_800365A0);

INCLUDE_ASM("asm/nonmatchings/code_24748", func_80036704);

INCLUDE_ASM("asm/nonmatchings/code_24748", func_80036878);

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

INCLUDE_ASM("asm/nonmatchings/code_24748", func_80036B90);

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

INCLUDE_ASM("asm/nonmatchings/code_24748", func_80036FE8);

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
    func_8002A7D8(&D_800D8D20[D_80095A30]);
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
