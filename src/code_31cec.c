#include "common.h"
#include "libsnd.h"
#include "libcd.h"
#include "libetc.h"
#include "libpress.h"
#include "libgte.h"
#include "libgpu.h"

/** @brief Movie decode state: double VLC buffers, image buffer, VRAM targets. */
typedef struct {
    u_long *vlcbuf[2]; /**< VLC buffers, used alternately */
    s32 vlcid;         /**< index of the VLC buffer being decoded */
    u_short *imgbuf;   /**< decoded image slice buffer */
    RECT rect[2];      /**< VRAM frame areas, used alternately */
    s32 rectid;        /**< index of the frame area being filled */
    RECT slice;        /**< area one DecDCTout() call fills */
    s32 isdone;        /**< set when a whole frame has been decoded */
} DECENV;

/* MATCHING: every access reloads after the store, as a volatile does. */
extern volatile s32 D_80095AD0;

u_long *func_80041A6C(DECENV *dec);

extern s16 D_800E0570[];
extern char D_800E0588[];
extern s32 D_80095AB4;
extern u8 D_80095AEE;
extern u16 D_80095B1A;
extern s16 D_80095AF0;
extern s32 D_8009579C;
extern s16 D_80095B14;
extern s16 D_80095B18;
extern s16 D_80095B16;
extern s16 D_80095AEA;
extern s16 D_800956C2;
extern s16 D_80095AC8;

void func_800FA610(s16 arg0);
void func_80041EE0(CdlLOC *loc);

void func_800414EC(s16 arg0) {
    s32 mode = D_80095830;

    if (mode != 1) {
        if (mode < 2) {
            if (mode == 0) {
                func_800FA610(arg0);
            }
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/code_31cec", func_80041534);

INCLUDE_ASM("asm/nonmatchings/code_31cec", func_80041964);

INCLUDE_ASM("asm/nonmatchings/code_31cec", func_80041A6C);

INCLUDE_ASM("asm/nonmatchings/code_31cec", func_80041BAC);

void func_80041C7C(DECENV *dec, s32 x0, s32 y0, s32 x1, s32 y1) {
    dec->vlcbuf[0] = (u_long *)0x8015D000;
    dec->vlcbuf[1] = (u_long *)0x80171000;
    dec->vlcid = 0;
    dec->imgbuf = (u_short *)0x80195000;
    dec->rectid = 0;
    dec->isdone = 0;
    setRECT(&dec->rect[0], x0, y0, 480, 240);
    setRECT(&dec->rect[1], x1, y1, 480, 240);
    setRECT(&dec->slice, x0 + D_800956C2, y0 + (240 - D_80095AC8) / 2, 24, D_80095AC8);
}

void func_80041D18(CdlLOC *loc, void (*callback)()) {
    DecDCTReset(0);
    D_80095AB4 = 0;
    DecDCToutCallback(callback);
    StSetRing((u_long *)0x80185000, 32);
    StSetStream(1, 1, -1, 0, 0);
    func_80041EE0(loc);
}

s32 func_80041D88(DECENV *dec) {
    u_long *next;

    D_80095AD0 = 0x800000;
    while ((next = func_80041A6C(dec)) == NULL) {
        if (--D_80095AD0 == 0) {
            return -1;
        }
    }
    dec->vlcid = dec->vlcid == 0;
    DecDCTvlc(next, dec->vlcbuf[dec->vlcid]);
    StFreeRing(next);
    return 0;
}

INCLUDE_ASM("asm/nonmatchings/code_31cec", func_80041E20);

void func_80041EE0(CdlLOC *loc) {
    do {
        while (CdControl(CdlSetloc, (u_char *)loc, 0) == 0) {
        }
    } while (CdRead2(CdlModeStream | CdlModeSpeed | CdlModeRT) == 0);
}

INCLUDE_ASM("asm/nonmatchings/code_31cec", func_80041F28);

s32 func_80042058(void) {
    s16 i;
    u32 *hdr;

    SsSetSerialAttr(SS_SERIAL_A, SS_MIX, SS_SON);
    SsSetSerialVol(SS_SERIAL_A, 127, 127);
    D_80095AEA = 0;
    D_80095B1A = 0;
    hdr = (u32 *)0x80101000;
    for (i = 2; i < 10; i++) {
        D_800E0570[i] = SsSeqOpen((unsigned long *)(*hdr + 0x80101000), D_80095B16);
    }
    for (i = 0; i < 10; i++) {
        SsSeqSetVol(D_800E0570[i], 0, 0);
    }
    return 0;
}

INCLUDE_ASM("asm/nonmatchings/code_31cec", func_80042150);

INCLUDE_ASM("asm/nonmatchings/code_31cec", func_80042208);

INCLUDE_ASM("asm/nonmatchings/code_31cec", func_80042538);

INCLUDE_ASM("asm/nonmatchings/code_31cec", func_800426A4);

void func_800428B0(void) {
    SsInit();
    SsSetTableSize(D_800E0588, 9, 1);
    SsSetTickMode(1);
}

void func_800428EC(void) {
    SsSetMVol(0, 0);
    SsSetRVol(0, 0);
    SsUtSetReverbType(SS_REV_TYPE_STUDIO_B);
    VSync(0);
    SsSetMVol(127, 127);
    SsSetRVol(120, 120);
    SsUtReverbOn();
    SsUtSetReverbDepth(48, 48);
}

void func_80042958(u8 value) {
    D_80095AF0 = value;
}

void func_80042968(u8 vol) {
    CdlATV atv;

    if (D_80095AEE) {
        atv.val2 = vol;
        atv.val0 = vol;
        atv.val3 = 0;
        atv.val1 = 0;
    } else {
        atv.val2 = vol;
        atv.val0 = vol;
        atv.val3 = vol;
        atv.val1 = vol;
    }
    CdMix(&atv);
}

void func_800429B4(void) {
    SsSeqStop(D_800E0570[0]);
    SsSeqClose(D_800E0570[0]);
}

void func_800429EC(void) {
    s16 *seq;

    for (D_80095B1A = 2; D_80095B1A < 10; D_80095B1A++) {
        seq = &D_800E0570[D_80095B1A];
        if (*seq != 0) {
            if (SsIsEos(*seq, 0) == 1) {
                SsSeqStop(*seq);
            }
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/code_31cec", func_80042A88);

void func_80042B80(u8 chan) {
    CdlFILTER filter;
    u_char mode[4];
    CdlATV atv;

    if (D_80095AEE) {
        atv.val2 = 80;
        atv.val0 = 80;
        atv.val3 = 0;
        atv.val1 = 0;
    } else {
        atv.val2 = 80;
        atv.val0 = 80;
        atv.val3 = 80;
        atv.val1 = 80;
    }
    CdMix(&atv);
    filter.file = 1;
    filter.chan = chan;
    CdControlF(CdlSetfilter, (u_char *)&filter);
    mode[0] = CdlModeSpeed | CdlModeRT | CdlModeSF | CdlModeDA;
    CdControlF(CdlSetmode, mode);
    CdControlF(CdlReadS, 0);
}

void func_80042C14(void) {
    CdlATV vol;

    vol.val3 = 0;
    vol.val2 = 0;
    vol.val1 = 0;
    vol.val0 = 0;
    CdMix(&vol);
    CdControlF(CdlPause, 0);
}

void func_80042C50(void) {}
