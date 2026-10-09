#include "common.h"
#include "libgte.h"
#include "libgpu.h"
#include "libetc.h"
#include "libpad.h"
#include "libgs.h"
#include "libapi.h"
#include "libcd.h"
#include "libsnd.h"
#include "libmcrd.h"
#include "memory.h"

INCLUDE_RODATA("asm/nonmatchings/main", D_80010000);

INCLUDE_ASM("asm/nonmatchings/main", main);

INCLUDE_ASM("asm/nonmatchings/main", func_80013B38);

INCLUDE_ASM("asm/nonmatchings/main", func_80013CDC);

INCLUDE_ASM("asm/nonmatchings/main", func_80013EE4);

INCLUDE_ASM("asm/nonmatchings/main", func_80014044);

INCLUDE_ASM("asm/nonmatchings/main", func_800142EC);

INCLUDE_ASM("asm/nonmatchings/main", func_800148B0);

INCLUDE_ASM("asm/nonmatchings/main", func_800149D0);

void func_80013EE4(void);
void func_8001534C(void);
void func_80014D20(void);
void func_80014D6C(void);

extern s32 D_800957A8;
extern u8 D_8009574C;
extern u8 D_80095754;
extern u8 D_8009575C;
extern u8 D_80095820;

void func_80014AC8(void) {
    ResetCallback();
    bzero((u8 *)0x80101000, 0xFC800);
    InitCARD(1);
    StartCARD();
    MemCardInit(1);
    MemCardStart();
    ChangeClearPAD(0);
    _bu_init();
    func_8001534C();
    func_800428B0();
    CdInit();
    SsStart();
    D_8009574C = 0;
    D_80095754 = 0;
    D_8009575C = 0;
    func_80013EE4();
    GsInit3D();
    func_80014D20();
    D_80095820 = 0;
    func_80014D6C();
}

void func_800142EC(s32 arg);

void func_80014B8C(s16 frames) {
    s16 i;

    for (i = 0; i < frames; i++) {
        func_800142EC(0);
        VSync(0);
    }
}

extern char D_800954CC[];

void func_80014BF0(s16 count) {
    s16 i;

    for (i = 0; i < count; i++) {
        FntPrint(D_800954CC);
    }
}

/** @brief 15 bytes copied as one block, for a local copy of a table. */
typedef struct {
    u8 b[15]; /**< the bytes */
} Bytes15;

#ifdef NON_MATCHING
extern u8 D_80010000[];
extern s16 D_80095AF0;

void func_80017440(s32 a, s32 b);

void func_80014C58(u8 idx) {
    Bytes15 tbl;

    tbl = *(Bytes15 *)D_80010000;
    /* MATCHING: retail reaches this through `lui`, other units through $gp. */
    if (D_80095AF0 != 0) {
        D_80095AF0 = 0;
    }
    func_80017440(tbl.b[idx], 1);
}
#else
INCLUDE_ASM("asm/nonmatchings/main", func_80014C58);
#endif

void func_80014CF0(void) {
    /* MATCHING: retail reserves an 8-byte frame it never touches. */
    s32 unused[2];

    D_800957A8 = 2;
    D_8009574C = 0;
    D_80095754 = 0;
    D_8009575C = 0;
}

void func_80014D20(void) {
    RECT rect;

    rect.x = 0;
    rect.y = 0;
    rect.w = 0x400;
    rect.h = 0x200;
    ClearImage(&rect, 0, 0, 0);
    DrawSync(0);
}

void func_80014D6C(void) {
    FntLoad(0x3C0, 0);
    FntOpen(-0x9A, -0x74, 0x140, 0x100, 0, 0x400);
}

INCLUDE_ASM("asm/nonmatchings/main", func_80014DB0);

INCLUDE_ASM("asm/nonmatchings/main", func_80014FA8);

INCLUDE_ASM("asm/nonmatchings/main", func_80015180);

extern u8 *D_80095704;

void func_80015328(s32 offset, u8 a, u8 b) {
    D_80095704[offset + 2] = a;
    D_80095704[offset + 3] = b;
}

void func_8001552C(u8 *a, u8 *b);

extern u8 D_80095BA0[];
extern u8 D_80095BE8[];
/* MATCHING: declared at most 8 bytes, so each base is one `la` register. */
extern s32 D_80095850[2];
extern s32 D_80095870[2];
extern s32 D_80095848[2];

void func_8001534C(void) {
    s16 i;

    func_8001552C(D_80095BA0, D_80095BE8);
    for (i = 0; i < 6; i++) {
        D_80095850[i] = 0;
        D_80095870[i] = 0;
        D_80095848[i] = 0;
    }
}

extern s32 D_800958CC;

void func_800153CC(s32 mode) {
    mode &= 1;
    D_800958CC = 0;
    do {
        VSync(0);
        if (PadGetState(0) == 6) {
            PadSetMainMode(0, mode, 0);
            return;
        }
        D_800958CC++;
    } while (D_800958CC < 10);
}

/**
 * @brief A 10-byte record of a table in initialized data (the table
 *        starts with one), unpacked field by field into small globals.
 *        Meanings not yet known.
 */
typedef struct {
    u16 unk0; /**< not yet known */
    u16 unk2; /**< not yet known */
    u8 unk4;  /**< not yet known */
    u8 unk5;  /**< not yet known */
    u8 unk6;  /**< not yet known */
    u8 unk7;  /**< not yet known */
    u16 unk8; /**< not yet known */
} TableHeader;

extern TableHeader *D_80095930;
extern u16 D_8009576C;
extern u16 D_8009586A;
extern u16 D_800958E6;
extern u16 D_80095764;
extern u16 D_80095766;
extern u16 D_800957D8;
extern u16 D_800957E0;

#ifdef NON_MATCHING
void func_80015450(TableHeader *tbl, u16 index) {
    u8 *rec;

    D_80095930 = tbl;
    tbl += index;
    D_8009576C = index;
    D_8009586A = tbl->unk0;
    D_800958E6 = tbl->unk2;
    /* MATCHING: byte-pointer reads keep each load below the prior store. */
    rec = (u8 *)tbl;
    D_80095764 = rec[5];
    D_80095766 = rec[4];
    D_800957D8 = *(u16 *)(rec + 8);
    D_800957E0 = rec[6];
}
#else
INCLUDE_ASM("asm/nonmatchings/main", func_80015450);
#endif

void func_800154C4(void) {
    TableHeader *hdr;

    hdr = (TableHeader *)D_800734AC;
    D_80095930 = hdr;
    D_8009576C = 0;
    D_8009586A = hdr->unk0;
    D_800958E6 = hdr->unk2;
    D_80095764 = hdr->unk5;
    D_80095766 = hdr->unk4;
    D_800957D8 = hdr->unk8;
    D_800957E0 = hdr->unk6;
}

extern u8 *D_80095700;

void func_8001552C(u8 *a, u8 *b) {
    s32 i;

    D_80095700 = a;
    D_80095704 = b;
    for (i = 0; i < 32; i++) {
        D_80095704[i] = 0;
    }
    PadInitDirect(D_80095700, D_80095700 + 0x22);
    PadStartCom();
}

INCLUDE_RODATA("asm/nonmatchings/main", D_80010148);

INCLUDE_ASM("asm/nonmatchings/main", func_80015584);

extern char D_80010148[];

void func_80015754(char *name, void *buf) {
    s32 fd;

    fd = open(name, 1);
    if (fd == -1) {
        for (;;) {
            GsSwapDispBuff();
            FntPrint(D_80010148, name);
            FntFlush(-1);
        }
    }
    read(fd, buf, 0x100000);
    close(fd);
}

INCLUDE_ASM("asm/nonmatchings/main", func_800157DC);

INCLUDE_ASM("asm/nonmatchings/main", func_80015A28);

INCLUDE_ASM("asm/nonmatchings/main", func_80015B78);

INCLUDE_ASM("asm/nonmatchings/main", func_80015CC8);

INCLUDE_ASM("asm/nonmatchings/main", func_800160E8);

INCLUDE_ASM("asm/nonmatchings/main", func_80016D14);

INCLUDE_ASM("asm/nonmatchings/main", func_80016FC0);

INCLUDE_ASM("asm/nonmatchings/main", func_80017124);

INCLUDE_ASM("asm/nonmatchings/main", func_80017270);

/**
 * @brief A per-channel stepping state (the name is a guess): its stepper
 *        advances channel N through tables at offsets 0x20 and 0x1B0.
 *        Only the bytes this unit touches are named.
 */
typedef struct {
    u8 unk0[4];     /**< not yet known */
    u8 unk4;        /**< passed on as the stepper's third argument */
    u8 unk5;        /**< a kind: 0x60 and 0x61 are tested */
    u8 unk6[2];     /**< not yet known */
    u8 unk8;        /**< set to 0xFF when a 0x61 step returns zero */
    u8 unk9[0x337]; /**< not yet known */
    u8 unk340;      /**< the stepper's last result */
} Stepper;

u8 func_80017F0C(Stepper *obj, u16 index, u8 arg);

void func_800173E8(Stepper *obj) {
    /* MATCHING: signed, so the zero test is `sll 24`, not `andi 0xFF`. */
    s8 result;

    result = func_80017F0C(obj, 0, obj->unk4);
    obj->unk340 = result;
    switch (obj->unk5) {
        case 0x60:
            break;
        case 0x61:
            if (result == 0) {
                obj->unk8 = 0xFF;
            }
            break;
    }
}

INCLUDE_ASM("asm/nonmatchings/main", func_80017440);

INCLUDE_RODATA("asm/nonmatchings/main", D_80010404);
