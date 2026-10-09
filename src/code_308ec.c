#include "common.h"
#include "libgte.h"
#include "libgpu.h"
#include "libgs.h"
#include "code_a0bc.h"
#include "code_1a098.h"

/** @brief The 0x14-byte records of common.h's NumberedSlot table, as the
 *         free list here hands them out. */
typedef struct {
    s16 unk0;  /**< cleared when the record is taken */
    s16 unk2;  /**< set when the record is taken */
    s16 unk4;  /**< set when the record is taken */
    s16 unk6;  /**< set when the record is taken */
    s16 unk8;  /**< set when the record is taken */
    s16 unkA;  /**< set when the record is taken */
    s16 unkC;  /**< set when the record is taken */
    s16 unkE;  /**< set when the record is taken */
    s16 unk10; /**< set when the record is taken */
    u8 unk12;  /**< set when the record is taken */
    u8 next;   /**< free-list link: the next record's index */
} Slot;

#define sSlots ((Slot *)D_800DFAB0)

u16 func_800FAD08(s16 arg0, s32 arg1, s32 arg2);
u16 func_800FAD04(s16 arg0, s32 arg1, s32 arg2);
u16 func_800FA894(s16 arg0, s32 arg1, s32 arg2);
u16 func_800FA580(s16 arg0, s32 arg1, s32 arg2);
u16 func_800F1A48(s16 arg0, s32 arg1, s32 arg2);
u16 func_800F9ABC(s16 arg0, s32 arg1, s32 arg2);
u16 func_800F9CA4(s16 arg0, s32 arg1, s32 arg2);
u16 func_800FA060(s16 arg0, s32 arg1, s32 arg2);
u16 func_800F922C(s16 arg0, s32 arg1, s32 arg2);

s32 func_800FA6C8(void);
s32 func_800FA804(void);
s32 func_800F6CE4(void);
s32 func_800FA3EC(void);
s32 func_800FA1C0(void);
s32 func_800F6AC8(void);
s32 func_800F1080(void);
s32 func_800F64B0(void);
s32 func_800F9618(void);
s32 func_800F98E4(void);
s32 func_800F475C(void);
s32 func_800F9BC0(void);
s32 func_800F8D34(void);
s32 func_800F3800(void);

void func_800FA5D0(void);

/** @brief A position as three words. */
typedef struct {
    s32 vx; /**< x */
    s32 vy; /**< y */
    s32 vz; /**< z */
} Pos3;

/** @brief The head of the game state as this unit writes it. */
typedef struct {
    u8 unk0;                  /**< set to 1 when a stage starts */
    u8 pad1;                  /**< not yet known */
    u8 unk2;                  /**< set to 1 when a stage starts */
    u8 pad3[3];               /**< not yet known */
    u8 unk6;                  /**< set to 2 when a stage starts */
    u8 pad7[0x348 - 0x7];     /**< not yet known */
    Pos3 unk348;              /**< the start position */
    u8 pad354[0x37C - 0x354]; /**< not yet known */
    s32 unk37C;               /**< the start heading */
    s32 unk380;               /**< the start heading */
    u8 pad384[0x3C8 - 0x384]; /**< not yet known */
    s16 unk3C8;               /**< cleared when a stage starts */
    s16 unk3CA;               /**< cleared when a stage starts */
} StageState;

/* MATCHING: a struct lvalue keeps the base in a register. */
#define sStage (*(StageState *)D_8009EB78)

/** @brief code_1a098's Rec48 with the halfword this unit clears. */
typedef struct {
    u8 unk0[0x26];  /**< not yet known */
    s16 unk26;      /**< cleared when a stage starts */
    u8 unk28[0x20]; /**< not yet known */
} Rec48Clear;

/* The Rec48 table; common.h declares it as words. */
#define sRecs48 ((Rec48Clear *)D_800A9008)

/** @brief A rotation followed by a position. */
typedef struct {
    SVECTOR rot; /**< the rotation */
    Pos3 pos;    /**< the position */
} Placement;

/* MATCHING: code_13068 reads this as an SVECTOR array; the position copy
 * needs the whole record here. */
extern Placement D_800A7680;

extern u8 D_80095900;
extern s16 D_80095760;
extern s8 D_8007AD58[];
extern s16 D_80095858;
extern s32 D_800957F4;

/** @brief The record the start heading is read from. */
typedef struct {
    u8 unk0[6]; /**< not yet known */
    s16 unk6;   /**< the start heading, negated */
} StartRec;

extern StartRec *D_80095840;
extern s32 D_8009578C;
extern u8 D_800958F8;
extern s16 D_800957B0;

/* MATCHING: code_1a098 hands it a Rec48; here it gets a position. */
s32 func_800183B0(Pos3 *pos);
void func_8002C2B4(void);
void func_8002C47C(void);
void func_8003399C(void);
void func_80014C58(u8 idx);
void func_80040998(void);
void func_800412DC(void);
void func_80041118(u8 arg0);

void func_800400EC(void) {
    s32 i;

    for (i = 0; i < 128; i++) {
        D_800DFAB0[i].unk13 = i + 1;
    }
    D_80095AA9 = 0;
    D_80095AA8 = 0xFF;
}

/* MATCHING: arg0 is only stored with sb, yet s32; as u8 it takes the first
 * temporary and shifts every other argument register. */
s32 func_80040130(s32 arg0, s16 arg1, s16 arg2, s16 arg3, s16 arg4, s16 arg5, s16 arg6, s16 arg7,
                  s16 arg8) {
    u8 old;

    if ((s8)D_80095AA9 < 0) {
        return -1;
    }
    old = D_80095AA8;
    D_80095AA8 = D_80095AA9;
    D_80095AA9 = sSlots[D_80095AA8].next;
    sSlots[D_80095AA8].next = old;
    sSlots[D_80095AA8].unk12 = arg0;
    sSlots[D_80095AA8].unkC = arg1;
    sSlots[D_80095AA8].unkE = arg2;
    sSlots[D_80095AA8].unk10 = arg3;
    sSlots[D_80095AA8].unk4 = arg5;
    sSlots[D_80095AA8].unk6 = arg6;
    sSlots[D_80095AA8].unk8 = arg7;
    sSlots[D_80095AA8].unkA = arg8;
    sSlots[D_80095AA8].unk0 = 0;
    sSlots[D_80095AA8].unk2 = arg4;
    return 0;
}

INCLUDE_ASM("asm/nonmatchings/code_308ec", func_800401F0);

void func_80040628(void) {
    SVECTOR pos;
    CVECTOR color;

    pos.vx = -0x40;
    pos.vy = -0x10;
    color.r = 1;
    color.g = color.b = color.cd = 0x80;
    func_8001B354(0x15F, &pos, &color, 0, &D_800ACEA8[D_80095750]);
    func_8001B2F4(0x1FE, 2, 0xA0, 0xF0, 10, 0, 0, 0, 0);
    func_8001B2F4(0x1FF, 2, 0xA0, 0xF0, 12, 0x20, 0, 0, 0);
    color.g = color.b = color.cd = 0x40;
    pos.vx = -0xA0;
    pos.vy = -0x78;
    func_8001B354(0x1FE, &pos, &color, 0, &D_800ACEA8[D_80095750]);
    pos.vx = 0;
    pos.vy = -0x78;
    func_8001B354(0x1FF, &pos, &color, 0, &D_800ACEA8[D_80095750]);
}

/* MATCHING: an unused 8-byte local gives retail's 0x20 frame; the start
 * position is copied field by field, not as a struct. */
void func_8004079C(u8 arg0) {
    s32 unused[2];
    s16 yaw;
    u32 i;

    D_80095900 = 1;
    if (arg0 % 3 == 2) {
        D_80095760 = 2;
        return;
    }
    sStage.unk3C8 = 0;
    sStage.unk2 = 1;
    D_800958A8 = 3;
    sStage.unk348.vx = D_800A7680.pos.vx;
    sStage.unk348.vy = D_800A7680.pos.vy;
    sStage.unk348.vz = D_800A7680.pos.vz;
    D_80095858 = 0;
    sStage.unk3CA = 0;
    if (D_8007AD58[D_80095830] == 1) {
        D_800957F4 = func_800183B0(&sStage.unk348);
    } else {
        D_800957F4 = 0;
    }
    D_8009578C = 0;
    yaw = -D_80095840->unk6;
    D_8009676C[0] = (s16)(D_800A7680.pos.vy - 500);
    D_800A7680.rot.vy = yaw;
    sStage.unk37C = sStage.unk380 = yaw;
    D_800957BC = yaw;
    D_800957B0 = yaw;
    if (D_800958F8 != 1) {
        func_8002C2B4();
    } else {
        func_8003399C();
    }
    func_8002C47C();
    func_80041118(D_80095830);
    sStage.unk6 = 2;
    sStage.unk0 = 1;
    D_800958EC = 0;
    D_800957D6 = 1;
    for (i = 0; i < 200; i++) {
        sRecs48[i].unk26 = 0;
    }
    func_80040998();
    func_800412DC();
    func_80014C58(D_80095830);
}

INCLUDE_ASM("asm/nonmatchings/code_308ec", func_80040998);

/* MATCHING: ret has no default; out-of-range numbers return whatever $a1
 * held. */
s16 func_80040CD0(u8 arg0) {
    s32 ret;

    switch (arg0) {
        case 0:
            ret = func_800FA6C8();
            break;
        case 1:
            ret = func_800FA804();
            break;
        case 2:
            ret = func_800F6CE4();
            break;
        case 3:
            ret = func_800FA3EC();
            break;
        case 4:
            ret = func_800FA1C0();
            break;
        case 5:
            ret = func_800F6AC8();
            break;
        case 6:
            ret = func_800F1080();
            break;
        case 7:
            ret = func_800F1080();
            break;
        case 8:
            ret = func_800F64B0();
            break;
        case 9:
            ret = func_800F9618();
            break;
        case 10:
            ret = func_800F98E4();
            break;
        case 11:
            ret = func_800F475C();
            break;
        case 12:
            ret = func_800F9BC0();
            break;
        case 13:
            ret = func_800F8D34();
            break;
        case 14:
            ret = func_800F3800();
            break;
    }
    return ret;
}

/* MATCHING: arg1 and arg2 go through to every callee untouched; they keep
 * $a1 and $a2 live, which puts ret in $a3. */
u16 func_80040E04(s16 arg0, s32 arg1, s32 arg2) {
    u16 ret = 1;

    switch (D_80095830) {
        case 0:
            ret = func_800FAD08(arg0, arg1, arg2);
            break;
        case 1:
            ret = func_800FAD04(arg0, arg1, arg2);
            break;
        case 3:
            ret = func_800FA894(arg0, arg1, arg2);
            break;
        case 4:
            ret = func_800FA580(arg0, arg1, arg2);
            break;
        case 6:
            ret = func_800F1A48(arg0, arg1, arg2);
            break;
        case 7:
            ret = func_800F1A48(arg0, arg1, arg2);
            break;
        case 9:
            ret = func_800F9ABC(arg0, arg1, arg2);
            break;
        case 10:
            ret = func_800F9CA4(arg0, arg1, arg2);
            break;
        case 12:
            ret = func_800FA060(arg0, arg1, arg2);
            break;
        case 13:
            ret = func_800F922C(arg0, arg1, arg2);
            break;
    }
    return ret;
}

INCLUDE_ASM("asm/nonmatchings/code_308ec", func_80040F14);

void func_80041118(u8 arg0) {
    s32 mode = arg0;

    if (mode != 1) {
        if (mode < 2) {
            if (mode == 0) {
                func_800FA5D0();
            }
        }
    }
}
