#include "common.h"
#include "libsnd.h"
#include "libcd.h"
#include "libetc.h"
#include "libpress.h"
#include "libapi.h"
#include "stdio.h"
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
void func_80041F28(s16 arg0);

extern s16 D_800E0570[];
extern char D_800E0588[];
extern s32 D_80095AB4;
extern u16 D_80095B1A;
extern s32 D_8009579C;
extern s16 D_80095B14;
extern s16 D_80095B18;
extern s16 D_80095B16;
extern s16 D_80095AEA;
extern u16 D_80095AFC;
extern s32 D_8007B6A4[];
extern s32 D_8007B6CC[];
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

void func_800142EC(s32 arg);
void func_80041964(void);
void func_80041C7C(DECENV *dec, s32 x0, s32 y0, s32 x1, s32 y1);
void func_80041D18(CdlLOC *loc, void (*callback)());
s32 func_80041D88(DECENV *dec);
void func_80041E20(DECENV *dec, s32 mode);

extern CdlLOC D_80095AB0;
extern s32 D_80095AD8;
extern RECT D_80095AE0;
extern DECENV D_800E04B0;
extern CdlFILE D_800E04E0;
extern DRAWENV D_800E04F8;
extern DISPENV D_800E0558;

s32 func_80041534(char *name, s16 skip) {
    CdlATV atv;
    s16 one;
    s32 id;
    s32 ret;

    ret = 0;
    if (*(s8 *)name != 0) {
        while (CdSearchFile(&D_800E04E0, name) == 0) {
        }
        D_80095AB0.minute = D_800E04E0.pos.minute;
        D_80095AB0.second = D_800E04E0.pos.second;
        D_80095AB0.sector = D_800E04E0.pos.sector;
    }
    func_80041C7C(&D_800E04B0, 0, 0, 0, 240);
    func_80041D18(&D_80095AB0, func_80041964);
    func_80041D88(&D_800E04B0);
    SetDefDispEnv(&D_800E0558, 0, 0, 0, 0);
    SetDefDrawEnv(&D_800E04F8, 0, 0, 0, 0);
    one = 1;
    D_800E0558.disp.w = D_800E0558.disp.w * 2 / 3;
    D_800E0558.isrgb24 = one;
    PutDispEnv(&D_800E0558);
    PutDrawEnv(&D_800E04F8);
    SetDispMask(0);
    while (1) {
        DecDCTin(D_800E04B0.vlcbuf[D_800E04B0.vlcid], 3);
        DecDCTout((u_long *)D_800E04B0.imgbuf, D_800E04B0.slice.w * D_800E04B0.slice.h / 2);
        func_80041D88(&D_800E04B0);
        func_80041E20(&D_800E04B0, 0);
        VSync(0);
        id = D_800E04B0.rectid == 0;
        SetDefDispEnv(&D_800E0558, D_800E04B0.rect[id].x, D_800E04B0.rect[id].y,
                      D_800E04B0.rect[id].w, D_800E04B0.rect[id].h);
        SetDefDrawEnv(&D_800E04F8, D_800E04B0.rect[id].x, D_800E04B0.rect[id].y,
                      D_800E04B0.rect[id].w, D_800E04B0.rect[id].h);
        D_800E0558.disp.w = D_800E0558.disp.w * 2 / 3;
        D_800E0558.isrgb24 = 1;
        PutDispEnv(&D_800E0558);
        PutDrawEnv(&D_800E04F8);
        SetDispMask(1);
        if (D_80095AB4 == one) {
            atv.val3 = 0;
            atv.val2 = 0;
            atv.val1 = 0;
            atv.val0 = 0;
            CdMix(&atv);
            VSync(0);
            while (CdControl(CdlPause, 0, 0) == 0) {
            }
            setRECT(&D_80095AE0, 0, 0, 480, 480);
            ClearImage(&D_80095AE0, 0, 0, 0);
            DrawSync(0);
            break;
        }
        func_800142EC(0);
        if ((D_80095970 & 0x800) && skip == one) {
            atv.val3 = 0;
            atv.val2 = 0;
            atv.val1 = 0;
            atv.val0 = 0;
            CdMix(&atv);
            VSync(0);
            while (CdControl(CdlPause, 0, 0) == 0) {
            }
            setRECT(&D_80095AE0, 0, 0, 480, 480);
            ClearImage(&D_80095AE0, 0, 0, 0);
            DrawSync(0);
            ret = 1;
            break;
        }
    }
    MoveImage(&D_800E04B0.rect[D_800E04B0.rectid], D_800E04B0.rect[id].x, D_800E04B0.rect[id].y);
    DrawSync(0);
    DecDCToutCallback(0);
    StUnSetRing();
    while (CdControlB(CdlPause, 0, 0) == 0) {
    }
    ChangeClearPAD(0);
    /* MATCHING: retail keeps this load and drops the store. */
    D_800E0558.disp.w = D_800E0558.disp.w;
    D_800E0558.isrgb24 = 0;
    SetDefDispEnv(&D_800E0558, D_800E04B0.rect[id].x, D_800E04B0.rect[id].y, D_800E04B0.rect[id].w,
                  D_800E04B0.rect[id].h);
    SetDefDrawEnv(&D_800E04F8, D_800E04B0.rect[id].x, D_800E04B0.rect[id].y, D_800E04B0.rect[id].w,
                  D_800E04B0.rect[id].h);
    return ret;
}

extern s32 D_800E1F1C;

void func_80041964(void) {
    if (D_800E1F1C) {
        StCdInterrupt();
        D_800E1F1C = 0;
    }
    LoadImage(&D_800E04B0.slice, (u_long *)D_800E04B0.imgbuf);
    D_800E04B0.slice.x += D_800E04B0.slice.w;
    if (D_800E04B0.slice.x <
        D_800E04B0.rect[D_800E04B0.rectid].x + D_800E04B0.rect[D_800E04B0.rectid].w) {
        DecDCTout((u_long *)D_800E04B0.imgbuf, D_800E04B0.slice.w * D_800E04B0.slice.h / 2);
    } else {
        D_800E04B0.isdone = 1;
        D_800E04B0.rectid = D_800E04B0.rectid == 0;
        D_800E04B0.slice.x = D_800E04B0.rect[D_800E04B0.rectid].x + D_800956C2;
        D_800E04B0.slice.y = D_800E04B0.rect[D_800E04B0.rectid].y + (240 - D_80095AC8) / 2;
    }
}

extern u_long D_80095ABC;
extern StHEADER *D_80095ACC;
extern u_long *D_80095AD4;
extern s32 D_800956C4;
extern s32 D_800956C8;

u_long *func_80041A6C(DECENV *dec) {
    D_80095AD0 = 0x800000;
    while (StGetNext(&D_80095AD4, (u_long **)&D_80095ACC)) {
        if (--D_80095AD0 == 0) {
            return NULL;
        }
    }
    D_80095ABC = D_80095ACC->frameCount;
    if (D_80095ACC->frameCount >= D_80095AD8) {
        D_80095AB4 = 1;
    }
    if (D_800956C4 != D_80095ACC->width || D_800956C8 != D_80095ACC->height) {
        setRECT(&D_80095AE0, 0, 0, 480, 480);
        ClearImage(&D_80095AE0, 0, 0, 0);
        DrawSync(0);
        D_800956C4 = D_80095ACC->width;
        D_800956C8 = D_80095ACC->height;
    }
    dec->rect[0].w = dec->rect[1].w = D_800956C4 * 3 / 2;
    dec->slice.h = dec->rect[0].h = dec->rect[1].h = D_800956C8;
    return D_80095AD4;
}

void func_80041BAC(char *name, CdlLOC *loc, s32 arg2, s16 arg3, s16 arg4) {
    CdlATV atv;

    D_80095AD8 = arg2;
    D_80095AC8 = arg3;
    D_80095AB0.minute = loc->minute;
    D_80095AB0.second = loc->second;
    D_80095AB0.sector = loc->sector;
    setRECT(&D_80095AE0, 0, 0, 480, 480);
    ClearImage(&D_80095AE0, 0, 0, 0);
    DrawSync(0);
    /* MATCHING: D_80095AEE by address; retail reaches it `lui` only here. */
    if (*(u8 *)0x80095AEE) {
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
    func_80041534(name, arg4);
}

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

void func_80041E20(DECENV *dec, s32 mode) {
    D_80095AD0 = 0x800000;
    while (dec->isdone == 0) {
        if (--D_80095AD0 == 0) {
            printf("time out in decoding !\n");
            dec->isdone = 1;
            dec->rectid = dec->rectid == 0;
            dec->slice.x = dec->rect[dec->rectid].x;
            dec->slice.y = dec->rect[dec->rectid].y;
        }
    }
    dec->isdone = 0;
}

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

s32 func_80042150(u16 song) {
    u32 *hdr;

    SsSeqStop(D_800E0570[0]);
    SsSeqClose(D_800E0570[0]);
    if (D_8009579C != 0) {
        return -1;
    }
    hdr = (u32 *)0x80101000;
    hdr += song * 4;
    D_800E0570[0] = SsSeqOpen((unsigned long *)(*hdr + 0x80101000), D_80095B14);
    SsSeqPlay(D_800E0570[0], 1, 0);
    D_80095B18 = 100;
    SsSeqSetVol(D_800E0570[0], 100, 100);
    D_80095AF0 = 0;
}

/* MATCHING: code_7d74 defines it with s16 arguments and result; this unit
 * passes and reads them as s32. */
s32 func_80018D04(s32 from, s32 to, s32 step, s32 steps);
void func_80042968(s32 vol);

extern s16 D_80095724;
extern s16 D_80095AE8;
extern s16 D_80095B0C;

void func_80042208(void) {
    s16 i;

    switch (D_80095AF0) {
        case 0:
            D_80095AE8 = 50;
            D_80095B0C = 127;
            D_80095B18 = 127;
            D_80095724 = 80;
            break;
        case 1:
            D_80095B18 = func_80018D04(0, 100, --D_80095AE8, 50);
            D_80095B18 = D_80095B18 < 0 ? 0 : D_80095B18 > 127 ? 127 : D_80095B18;
            if (D_80095AE8 == 0) {
                SsSeqStop(D_800E0570[0]);
                SsSeqClose(D_800E0570[0]);
                D_80095AF0 = 0;
                D_80095B0C = 127;
                D_80095B18 = 127;
            }
            SsSeqSetVol(D_800E0570[0], D_80095B18, D_80095B18);
            break;
        case 3:
            D_80095B18 = func_80018D04(0, 100, --D_80095AE8, 50);
            D_80095B0C = D_80095B18 = D_80095B18 < 0 ? 0 : D_80095B18 > 127 ? 127 : D_80095B18;
            SsSeqSetVol(D_800E0570[0], D_80095B18, D_80095B18);
            for (i = 2; i < 10; i++) {
                SsSeqSetVol(D_800E0570[i], D_80095B0C, D_80095B0C);
            }
            if (D_80095AE8 == 0) {
                SsSeqStop(D_800E0570[0]);
                SsSeqClose(D_800E0570[0]);
                for (i = 2; i < 10; i++) {
                    SsSeqStop(D_800E0570[i]);
                }
                for (i = 0; i < 10; i++) {
                    SsSeqSetVol(D_800E0570[i], 0, 0);
                }
                D_80095B0C = 127;
                D_80095B18 = 127;
                D_80095AF0 = 0;
                D_80095724 = 80;
            }
        case 4:
            D_80095724 = func_80018D04(0, 80, --D_80095AE8, 50);
            D_80095724 = D_80095724 < 0 ? 0 : D_80095724 > 80 ? 80 : D_80095724;
            func_80042968(D_80095724);
            if (D_80095AE8 == 0) {
                D_80095724 = 80;
                func_80017574();
                D_80095AF0 = 0;
            }
            break;
    }
    if (D_80095AE8 < 0) {
        D_80095AF0 = 0;
    }
}

extern u8 D_8007BBEC[];

s32 func_80042538(s32 id) {
    s16 *seq;
    s16 i;
    u8 bank;
    s16 *vabs;
    u32 *hdr;

    for (D_80095B1A = 2; D_80095B1A < 10; D_80095B1A++) {
        i = D_80095B1A;
        seq = &D_800E0570[(u16)i];
        if (SsIsEos(*seq, 0) == 0) {
            break;
        }
    }
    SsSeqClose(*seq);
    if (D_8009579C != 0) {
        return -1;
    }
    hdr = (u32 *)0x80101000;
    hdr += (u16)id * 4;
    vabs = &D_80095B14;
    if ((u16)id < 70) {
        bank = 0;
    } else {
        bank = D_8007BBEC[(u16)id - 70];
    }
    D_800E0570[(u16)i] = SsSeqOpen((unsigned long *)(*hdr + 0x80101000), vabs[bank]);
    SsSeqPlay(D_800E0570[(u16)i], 1, 1);
    SsSeqSetVol(D_800E0570[(u16)i], 127, 127);
    if (++D_80095B1A >= 10) {
        D_80095B1A = 2;
    }
}

extern s32 D_8007B69C[];

s32 func_800426A4(void) {
    s32 ret = 0;

    switch (D_800958A6) {
        case 0:
            D_80096748[0] = (s32)&D_8007B69C[0];
            D_8009F090[0] = 0x80101000;
            D_80096748[1] = (s32)&D_8007B69C[2];
            D_8009F090[1] = 0x8016D000;
            D_80095960 = 2;
            D_8009596C = 1;
            D_8009F248[0] = D_8007B6CC[4];
            D_8009F248[1] = D_8007B6CC[6];
            D_800958A6++;
            break;
        case 1:
            if (D_8009596C == 6) {
                func_80041F28(0);
                D_800958A6++;
            }
            break;
        case 2:
            D_80096748[0] = (s32)&D_8007B69C[3];
            D_8009F090[0] = 0x8016D000;
            D_80095960 = 1;
            D_8009596C = 1;
            D_8009F248[0] = D_8007B6CC[7];
            D_800958A6++;
            break;
        case 3:
            if (D_8009596C == 6) {
                func_80041F28(1);
                func_80042058();
                D_80095AFC = 1;
                D_800958A6++;
            }
            break;
        case 4:
            SsSetMVol(0, 0);
            SsSetRVol(0, 0);
            SsUtSetReverbType(SS_REV_TYPE_STUDIO_B);
            VSync(0);
            SsSetMVol(127, 127);
            SsSetRVol(120, 120);
            SsUtReverbOn();
            SsUtSetReverbDepth(48, 48);
            ret = 1;
            break;
    }
    return ret;
}

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

void func_80042968(s32 vol) {
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

s32 func_80042A88(u16 bank) {
    s32 ret = 0;

    if (bank == 0) {
        return 1;
    }
    if (bank == D_80095AFC) {
        return 1;
    }
    switch (D_80095B0E) {
        case 0:
            D_80096748[0] = (s32)&D_8007B6A4[bank];
            D_8009F248[0] = D_8007B6CC[bank + 6];
            D_8009F090[0] = 0x8016D000;
            D_80095960 = 1;
            D_8009596C = 1;
            D_80095B0E++;
            break;
        case 1:
            if (D_8009596C == 6) {
                SsVabClose(D_80095B16);
                D_80095AFC = bank;
                func_80041F28(1);
                ret = 1;
            }
            break;
    }
    return ret;
}

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
