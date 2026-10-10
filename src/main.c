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
#include "code_a0bc.h"
#include "code_7d74.h"
#include "code_1a098.h"
#include "spad.h"

INCLUDE_RODATA("asm/nonmatchings/main", D_80010000);

INCLUDE_ASM("asm/nonmatchings/main", main);

extern u16 D_800958A6;
extern s32 D_80072484[];
extern s32 D_800724C4[];
void func_80013CDC(void);

/* MATCHING: code_7d74 types the pack as its own PackEntry. */
void func_8001797C(void *pack);

/* MATCHING: arity-ok, retail passes an argument the empty definition ignores. */
void func_80018DE8(s32 arg);

s8 func_80013B38(void) {
    /* MATCHING: a byte, so the later constant 1 is not copied from it. */
    s8 ret;

    ret = 0;
    switch (D_800958A6) {
        case 0:
            D_80096748[0] = (s32)&D_80072484[0];
            D_8009F090[0] = 0x8014D000;
            D_80096748[1] = (s32)&D_80072484[1];
            D_8009F090[1] = 0x8018D000;
            D_80096748[2] = (s32)&D_80072484[2];
            D_8009F090[2] = 0x80123000;
            D_80096748[3] = (s32)&D_80072484[3];
            D_8009F090[3] = 0x8016D000;
            D_80096748[4] = (s32)&D_80072484[4];
            D_8009F090[4] = 0x80101000;
            D_80095960 = 5;
            D_8009F248[0] = D_800724C4[0];
            D_8009F248[1] = D_800724C4[1];
            D_8009F248[2] = D_800724C4[2];
            D_8009F248[3] = D_800724C4[3];
            D_8009F248[4] = D_800724C4[4];
            D_8009596C = 1;
            D_800958A6 = 1;
            break;
        case 1:
            if (D_8009596C == 6) {
                func_80018DE8(0);
                D_800958A6++;
            }
            break;
        case 2:
            ret = 1;
            func_80013CDC();
            func_8001797C((void *)0x8018D000);
            func_8003E13C((unsigned long *)(*(s32 *)0x8016D000 + 0x8016D000));
            func_8003A84C();
            func_80018DE8(1);
            break;
    }
    return ret;
}

/**
 * @brief A per-channel stepping state (the name is a guess): its stepper
 *        advances channel N through tables at offsets 0x20 and 0x1B0.
 *        Only the bytes this unit touches are named.
 */
typedef struct {
    u8 unk0[4];      /**< not yet known */
    u8 unk4;         /**< passed on as the stepper's third argument */
    u8 unk5;         /**< a kind: 0x60 and 0x61 are tested; cleared at load */
    u8 unk6;         /**< cleared at load */
    u8 unk7;         /**< cleared at load */
    u8 unk8;         /**< set to 0xFF when a 0x61 step returns zero */
    u8 unk9;         /**< set to 0xFF at load */
    u8 unkA[0x16];   /**< not yet known */
    s32 unk20[100];  /**< first word of each loaded entry */
    s32 unk1B0[100]; /**< third word of each loaded entry */
    u8 unk340;       /**< the stepper's last result */
    u8 unk341[7];    /**< not yet known */
    s32 unk348;      /**< a position, x: the model's translation */
    s32 unk34C;      /**< a position, y */
    s32 unk350;      /**< a position, z */
    u8 unk354[0x2C]; /**< not yet known */
    u16 unk380;      /**< a rotation about y */
} Stepper;

/* The stepper's state lives at the head of the game state. */
#define sStepper (*(Stepper *)D_8009EB78)

/** @brief A model object with its own coordinate system (a unit-local
 *         copy of code_1a098's view). */
typedef struct {
    GsDOBJ2 obj;         /**< the object handler */
    GsCOORDINATE2 coord; /**< the object's coordinate system */
    SVECTOR rot;         /**< rotation */
    SVECTOR scale;       /**< scale, 0x1000 = 1 */
} Model70;

/** @brief A 16-byte directory entry of a loaded file. */
typedef struct {
    s32 offset;   /**< byte offset of the entry from the directory */
    u8 unk4[0xA]; /**< not yet known */
    u16 count;    /**< entry count; read from the first entry only */
} DirEnt16;

extern Model70 D_800963A0;

/** @brief A 0x78-byte model slot: a Model70 and 8 bytes not yet known. */
typedef struct {
    Model70 m;   /**< the model */
    u8 unk70[8]; /**< not yet known */
} ModelSlot;

extern ModelSlot D_800D8370[];
extern u8 D_8009EF50[];
extern u8 D_80096418[];

/* MATCHING: code_1a098 defines it on its own Model70 view. */
void func_8002C20C(Model70 *m, unsigned long *tmd, u8 n);
/* MATCHING: code_7d74 types the list, slots and data as its own records. */
void func_80019684(s32 *list, u8 *slots, GsCOORDINATE2 *objs, u8 *data, s32 n);
/* MATCHING: code_308ec passes the game state's head in its own view. */
u8 func_80017F0C(Stepper *obj, u16 index, u8 arg);

void func_80013CDC(void) {
    /* MATCHING: one pointer reused for the pack, its second word and the
     * directory, so all three share a saved register. */
    s32 *p;
    s32 *q;

    p = (s32 *)0x8014D000;
    D_80095904 = (s32)((unsigned long *)((u8 *)p + *p) + 1);
    GsMapModelingData((unsigned long *)D_80095904);
    func_8002C20C(&D_800963A0, (unsigned long *)((u8 *)p + *p), 16);
    p = (s32 *)0x8014D010;
    func_8002C20C(&D_800D8370[0].m, (unsigned long *)(*p + 0x8014D000), 0);
    func_8002C20C(&D_800D8370[1].m, (unsigned long *)(*p + 0x8014D000), 1);
    func_8002C20C(&D_800D8370[2].m, (unsigned long *)(*p + 0x8014D000), 2);
    func_8002C20C(&D_800D8370[3].m, (unsigned long *)(*p + 0x8014D000), 3);
    func_8002C20C(&D_800D8370[4].m, (unsigned long *)(*p + 0x8014D000), 4);
    func_8002C20C(&D_800D8370[5].m, (unsigned long *)(*p + 0x8014D000), 5);
    func_8002C20C(&D_800D8370[6].m, (unsigned long *)(*p + 0x8014D000), 6);
    func_80019684(D_800D8360, D_8009EF50, D_800D86E0, D_80096418, 20);
    p = (s32 *)0x80123000;
    D_800958CC = 0;
    D_800958D0 = ((DirEnt16 *)p)->count;
    for (; D_800958CC < D_800958D0; D_800958CC++) {
        /* MATCHING: assigned inside the store, as in func_8002C044. */
        D_800D81B0[D_800958CC] = (q = (s32 *)((u8 *)0x80123000 + *p)) + 1;
        p += 4;
        sStepper.unk20[D_800958CC] = *D_800D81B0[D_800958CC];
        D_800D81B0[D_800958CC]++;
        sStepper.unk1B0[D_800958CC] = D_800D81B0[D_800958CC][1];
    }
    GsInitCoordinate2(WORLD, &D_800D86E0[0]);
    sStepper.unk5 = 0;
    sStepper.unk6 = 0;
    /* MATCHING: retail stores 8 before 7. */
    sStepper.unk8 = 0xFF;
    sStepper.unk7 = 0;
    sStepper.unk9 = 0xFF;
    func_80017F0C(&sStepper, 0, 0);
}

/* Sony's (libgs, carved as asm): GsInitGraph by its arguments. */
void func_80056774(s32 w, s32 h, s32 intmode, s32 dith, s32 vram);

extern GsOT_TAG D_80096A78[2][0x1000];
extern GsOT_TAG D_8009F278[2][0x1000];
extern PACKET D_800ACF00[2][67200];
extern GsOT *D_80095884;

void func_80013EE4(void) {
    s32 i;

    func_80056774(320, 240, 4, 1, 0);
    GsDefDispBuff(0, 0, 0, 240);
    for (i = 0; i < 2; i++) {
        D_800ACEA8[i].length = 12;
        D_800ACEA8[i].org = D_8009F278[i];
        D_800ACEA8[i].point = 0;
        D_800ACEA8[i].offset = 0;
        D_800A7318[i].length = 12;
        D_800A7318[i].org = D_80096A78[i];
        /* MATCHING: retail stores this field twice. */
        D_800ACEA8[i].point = 300;
        D_800A7318[i].offset = 0;
    }
    D_80095750 = GsGetActiveBuff();
    GsSetWorkBase(D_800ACF00[D_80095750]);
    GsClearOt(0, 300, &D_800A7318[D_80095750]);
    GsClearOt(0, 0, &D_800ACEA8[D_80095750]);
    D_80095884 = &D_800A7318[D_80095750];
}

INCLUDE_ASM("asm/nonmatchings/main", func_80014044);

INCLUDE_ASM("asm/nonmatchings/main", func_800142EC);

void func_80013EE4(void);
void func_800142EC(s32 arg);
void func_80014B8C(s16 frames);

/* MATCHING: declared at most 8 bytes, so each base is one `la` register. */
extern s32 D_80095850[2];
extern s32 D_80095870[2];
extern s32 D_80095848[2];

void func_800148B0(void) {
    s32 x;
    s32 y;
    s32 pad;

    x = 0;
    y = 0;
    func_80014B8C(20);
    for (;;) {
        pad = D_80095848[0];
        if (pad & 0x2000) {
            x += 8;
        }
        if (pad & 0x8000) {
            x -= 8;
        }
        if (pad & 0x1000) {
            y -= 8;
        }
        if (pad & 0x4000) {
            y += 8;
        }
        if (pad & 0x800) {
            break;
        }
        x = x < 0 ? 0 : x > 0x2C0 ? 0x2C0 : x;
        y = y < 0 ? 0 : y > 0x110 ? 0x110 : y;
        GsDefDispBuff(x, y, x, y);
        func_800142EC(0);
        DrawSync(0);
        VSync(0);
        GsSwapDispBuff();
    }
    func_80013EE4();
    GsInit3D();
    func_80018CB4();
    func_80018094();
    func_80014B8C(5);
}

extern char D_800954C8[];
extern char D_800954CC[];
extern char D_800954D0[];
extern char D_800954D8[];
extern char D_800954DC[];
extern s32 D_80095788;

void func_800149D0(u8 *base) {
    u8 *p;
    s32 row;
    s32 col;
    s16 i;
    s16 j;

    D_80095788 = 0;
    FntPrint(D_800954D0);
    p = base + D_80095788;
    FntPrint(D_800954D8, p);
    for (i = 0; i < 1; i++) {
        FntPrint(D_800954CC);
    }
    for (row = 0; row < 16; row++) {
        for (col = 0; col < 8; col++) {
            FntPrint(D_800954DC, *p);
            p++;
        }
        FntPrint(D_800954C8);
        for (j = 0; j < 1; j++) {
            FntPrint(D_800954CC);
        }
    }
}

void func_8001534C(void);
void func_80014D20(void);
void func_80014D6C(void);

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

void func_80014B8C(s16 frames) {
    s16 i;

    for (i = 0; i < frames; i++) {
        func_800142EC(0);
        VSync(0);
    }
}

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

extern u8 D_80010000[];

void func_80017440(s32 a, s32 b);

void func_80014C58(u8 idx) {
    Bytes15 tbl;

    tbl = *(Bytes15 *)D_80010000;
    if (D_80095AF0 != 0) {
        D_80095AF0 = 0;
    }
    func_80017440(tbl.b[idx], 1);
}

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
/* MATCHING: signed, so the zero test in func_80014DB0 is its own `lh`. */
extern s16 D_800957D8;
extern u16 D_800957E0;
extern u8 D_800957DA;

void func_80015328(s32 offset, u8 a, u8 b);

/**
 * @brief Selects record @p index of @p tbl and unpacks it into the record
 *        globals. The same body as the out-of-line selector below; inlined
 *        with the table head as @p tbl, its store of the head drops out.
 */
static __inline__ void setRecord(TableHeader *tbl, u16 index) {
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

/**
 * @brief setRecord's twin for repeating the current record: the same
 *        unpacking, with the repeat count @p count stepped down instead of
 *        reloaded. Inlined, so its parameter conversions survive.
 */
static __inline__ void repeatRecord(TableHeader *tbl, u16 index, u8 count) {
    u8 *rec;

    D_80095930 = tbl;
    tbl += index;
    D_8009576C = index;
    D_8009586A = tbl->unk0;
    D_800958E6 = tbl->unk2;
    rec = (u8 *)tbl;
    D_80095764 = rec[5];
    D_80095766 = rec[4];
    D_800957D8 = *(u16 *)(rec + 8);
    D_800957E0 = count - 1;
}

/* MATCHING: s32 with a bare return, so the `bgtz` keeps its `nop` slot. */
s32 func_80014DB0(void) {
    if ((s16)D_800958E6 > 0 && D_800957DA == 1) {
        func_80015328(0, D_80095764, D_80095766);
        D_800958E6--;
        return;
    }
    D_800958E6 = 0;
    D_80095764 = 0;
    D_80095766 = 0;
    func_80015328(0, 0, 0);
    if ((s16)D_800957E0 > 0) {
        if ((s16)--D_8009586A <= 0) {
            if ((s16)D_800957E0 < 2) {
                D_800957E0 = 0;
                if (D_800957D8 != 0) {
                    setRecord(D_80095930, D_800957D8);
                }
            } else {
                repeatRecord(D_80095930, D_8009576C, D_800957E0);
            }
        }
    } else {
        D_8009586A = 0;
    }
}

INCLUDE_ASM("asm/nonmatchings/main", func_80014FA8);

extern u8 *D_80095700;
extern u8 *D_80095704;
/* MATCHING: a known small size, so its address is one `la` register. */
extern u8 D_800954E4[6];

u32 func_80015180(void) {
    u8 *buf;
    u8 *act;
    s32 k;
    s32 state;
    s32 i;

    buf = D_80095700;
    act = D_80095704;
    for (k = 0; k < 2; k++, act += 0x10) {
        state = PadGetState(k * 16);
        if (state == 1) {
            act[0] = 0;
        }
        if (act[0] == 0) {
            PadSetAct(k * 16, act + 2, 2);
            if (state == 2 || (state == 6 && PadSetActAlign(k * 16, D_800954E4))) {
                act[0] = 1;
            }
        }
        if (buf[k * 0x22] != 0) {
            for (i = 2; i < 8; i++) {
                buf[k * 0x22 + i] = 0xFF;
            }
        } else if ((buf[k * 0x22 + 1] & 0xF0) == 0x70) {
            for (i = 4; i < 8; i++) {
                if ((u32)(buf[k * 0x22 + i] - 0x69) < 0x2F) {
                    buf[k * 0x22 + i] = 0x80;
                }
            }
        }
    }
    return ~((buf[0x24] << 24) | (buf[0x25] << 16) | (buf[2] << 8) | buf[3]);
}

void func_80015328(s32 offset, u8 a, u8 b) {
    D_80095704[offset + 2] = a;
    D_80095704[offset + 3] = b;
}

void func_8001552C(u8 *a, u8 *b);

extern u8 D_80095BA0[];
extern u8 D_80095BE8[];

void func_8001534C(void) {
    s16 i;

    func_8001552C(D_80095BA0, D_80095BE8);
    for (i = 0; i < 6; i++) {
        D_80095850[i] = 0;
        D_80095870[i] = 0;
        D_80095848[i] = 0;
    }
}

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

extern s16 D_800954EC;
extern s32 D_8009582C;
extern u8 D_800957F0;

s8 func_80015584(void) {
    s32 r;

    switch (D_8009596C) {
        case 0:
            break;
        case 1:
            CdControlF(CdlSetloc, (u_char *)D_80096748[D_800954EC]);
            D_8009596C++;
            break;
        case 2:
            r = CdSync(1, NULL);
            if (r == 2) {
                D_8009596C++;
            } else if (r == 5) {
                D_8009596C = 1;
                D_800957F0++;
            }
            break;
        case 3:
            D_8009582C = D_8009F248[D_800954EC] + 1;
            if (CdRead(D_8009582C, (u_long *)D_8009F090[D_800954EC], 0x80) == 0) {
                D_800957F0++;
            } else {
                D_8009596C++;
            }
            break;
        case 4:
            r = CdReadSync(1, NULL);
            if (r == 0) {
                D_8009596C++;
            } else if (r == -1) {
                D_8009596C = 1;
                D_800957F0++;
            }
            break;
        case 5:
            if (++D_800954EC >= D_80095960) {
                D_8009596C = 6;
                D_800954EC = 0;
            } else {
                D_8009596C = 1;
            }
            break;
    }
    return D_8009596C;
}

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

void func_80015A28(void) {
    SVECTOR pos;
    RECT rect;

    func_8001B2F4(0x1FE, 2, 0xA0, 0xF0, 10, 0, 0, 0, 0);
    func_8001B2F4(0x1FF, 2, 0xA0, 0xF0, 12, 0x20, 0, 0, 0);
    rect.x = 0;
    rect.y = D_800E474C * 240;
    rect.w = 320;
    rect.h = 240;
    MoveImage(&rect, 640, 0);
    pos.vx = -160;
    pos.vy = -120;
    func_8001B354(0x1FE, &pos, NULL, 0, &D_800ACEA8[D_80095750]);
    pos.vx = 0;
    pos.vy = -120;
    func_8001B354(0x1FF, &pos, NULL, 0, &D_800ACEA8[D_80095750]);
    D_800958A6 = 0;
    D_80095880 = 0x2A;
}

void func_80015B78(void) {
    SVECTOR pos;
    RECT rect;

    func_8001B2F4(0x1FE, 2, 0xA0, 0xF0, 10, 0, 0, 0, 0);
    func_8001B2F4(0x1FF, 2, 0xA0, 0xF0, 12, 0x20, 0, 0, 0);
    rect.x = 0;
    rect.y = D_800E474C * 240;
    rect.w = 320;
    rect.h = 240;
    MoveImage(&rect, 640, 0);
    pos.vx = -160;
    pos.vy = -120;
    func_8001B354(0x1FE, &pos, NULL, 0, &D_800ACEA8[D_80095750]);
    pos.vx = 0;
    pos.vy = -120;
    func_8001B354(0x1FF, &pos, NULL, 0, &D_800ACEA8[D_80095750]);
    D_800958A6 = 0;
    D_80095880 = 0x2B;
}

INCLUDE_ASM("asm/nonmatchings/main", func_80015CC8);

INCLUDE_ASM("asm/nonmatchings/main", func_800160E8);

INCLUDE_ASM("asm/nonmatchings/main", func_80016D14);

void func_800160E8(void);

extern s32 D_80095980;

void func_80016FC0(void) {
    SVECTOR pos;

    pos.vx = 0x70;
    pos.vy = 0x48;
    func_8001B354(D_8009585C % 10 + 0xFB, &pos, NULL, 0, &D_800ACEA8[D_80095750]);
    pos.vx = -0xA0;
    pos.vy = -0x78;
    func_8001B354(0x1FE, &pos, NULL, 0, &D_800ACEA8[D_80095750]);
    pos.vx = 0;
    pos.vy = -0x78;
    func_8001B354(0x1FF, &pos, NULL, 0, &D_800ACEA8[D_80095750]);
    func_800160E8();
    if (D_800958A6 == 100) {
        D_80095760 = 4;
        D_80095980 = 0;
        D_800958A6 = 0;
        D_80095880 = 14;
        func_80014C58(D_80095830);
    }
}

void func_80015CC8(void);

void func_80017124(void) {
    SVECTOR pos;

    pos.vx = 0x70;
    pos.vy = 0x48;
    func_8001B354(D_8009585C % 10 + 0xFB, &pos, NULL, 0, &D_800ACEA8[D_80095750]);
    pos.vx = -0xA0;
    pos.vy = -0x78;
    func_8001B354(0x1FE, &pos, NULL, 0, &D_800ACEA8[D_80095750]);
    pos.vx = 0;
    pos.vy = -0x78;
    func_8001B354(0x1FF, &pos, NULL, 0, &D_800ACEA8[D_80095750]);
    func_80015CC8();
    if (D_800958A6 == 100) {
        D_80095760 = 4;
        D_800958A6 = 0;
        D_80095880 = 14;
    }
}

/** @brief A list of models drawn together (a unit-local view). */
typedef struct {
    GsDOBJ2 *objs; /**< the models */
    s32 count;     /**< how many */
} ModelSet;

/* MATCHING: a struct lvalue over the shared array declaration. */
#define sModels (*(ModelSet *)D_800D8360)

extern s32 D_80095758;

void func_80017270(Stepper *obj) {
    MATRIX m;
    SVECTOR rot;
    GsDOBJ2 *o;
    s32 i;

    func_80020CF8(D_80095758);
    o = sModels.objs;
    o->coord2->coord.t[0] = obj->unk348;
    o->coord2->coord.t[1] = obj->unk34C;
    o->coord2->coord.t[2] = obj->unk350;
    rot.vx = 0;
    rot.vy = obj->unk380;
    rot.vz = 0;
    func_80018AE0(&rot, o->coord2);
    for (i = 0; i < sModels.count; o++, i++) {
        o->coord2->flg = 0;
        if (o->id != -1 && o->tmd != NULL) {
            GsGetLs(o->coord2, &m);
            GsSetLsMatrix(&m);
            GsGetLw(o->coord2, &m);
            GsSetLightMatrix(&m);
            /* MATCHING: retail runs this sort on the scratchpad stack. */
            SetSpadStack();
            GsSortObject4J(o, &D_800ACEA8[D_80095750], 2, (u_long *)0x1F800000);
            ResetSpadStack();
        }
    }
}

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

extern CdlLOC D_80095FE0[];
extern s32 D_80095710;
extern s32 D_80095718;
/* MATCHING: a known small size, so the byte store is `$gp`-relative. */
extern u8 D_80095AC0[4];

void func_80017440(s32 track, s32 arg) {
    CdlATV atv;

    if (D_800958C9 == 1) {
        func_80017574();
    }
    if (D_80095AEE) {
        atv.val2 = 0x50;
        atv.val0 = 0x50;
        atv.val3 = 0;
        atv.val1 = 0;
    } else {
        atv.val2 = 0x50;
        atv.val0 = 0x50;
        atv.val3 = 0x50;
        atv.val1 = 0x50;
    }
    CdMix(&atv);
    D_80095718 = arg;
    CdGetToc(D_80095FE0);
    D_80095720 = CdPosToInt(&D_80095FE0[track]);
    D_80095710 = CdPosToInt(&D_80095FE0[track + 1]) - 20;
    D_8009571C = 0;
    D_80095714 = D_80095720;
    SsSetSerialAttr(0, 0, 1);
    SsSetSerialVol(0, 0x72, 0x72);
    D_80095AC0[0] = 3;
    CdControlB(CdlSetmode, D_80095AC0, 0);
    VSync(3);
    CdReadyCallback((CdlCB)func_80017614);
    func_800175AC(CdlPlay);
    D_800958C9 = 1;
}

INCLUDE_RODATA("asm/nonmatchings/main", D_80010404);
