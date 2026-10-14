/**
 * @file sound.c
 * @brief The game's sound and its movie player: the VAB banks, the BGM and
 *        sound-effect sequences, their fades, reverb, XA audio and the
 *        streamed STR movies.
 *
 * LoadSoundData runs at boot, a step a frame: it reads the sound pack (the
 * SEQ files, opened in place) and the first VAB, which stays open as
 * gBgmVab, then the first swappable VAB, opened as gSeVab; LoadVabBank later
 * swaps another numbered bank into that second slot. Sequence slot 0 plays
 * the background music (PlayBgm), slots 2 to 9 the sound effects (PlaySe),
 * and UpdateSoundFade, run every frame, fades the music, the effects or the
 * CD audio out over 50 frames. The CD mix follows the stereo setting
 * (SetCdVolume, PlayXa). PlayMovie streams an STR file from the disc and
 * decodes it a frame at a time: the frame's data is VLC-decoded into one of
 * two buffers while the MDEC turns the other into 24-bit slices that its
 * callback loads into one of two VRAM frame areas, the other being shown.
 */

#include "common.h"
#include "ram.h"
#include "sbss.h"
#include "memory.h"
#include "libsnd.h"
#include "libcd.h"
#include "libetc.h"
#include "libpress.h"
#include "libapi.h"
#include "stdio.h"
#include "libgte.h"
#include "libgpu.h"
#include "sound.h"

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
extern volatile s32 gMovieTimeout; /**< the movie waits' countdown: each gives up when it reaches 0 */

u_long *GetNextMovieFrame(DECENV *dec);
s32 LoadVab(s16 bank);

extern s16 gSeqIds[]; /**< the open SEQ ids: 0 the BGM, 2 to 9 the sound-effect slots; 1 never opens */
extern char sSeqAttrTable[]; /**< SsSetTableSize's SEQ attribute table */
extern s32 gMovieEnded;      /**< set once the movie's last frame has been read */
extern u16 gSeSlot;          /**< the sound-effect slot, 2 to 9, PlaySe tries first */
extern s16 gBgmVab;          /**< VAB id of the first bank, open for good; gSeVab follows it */
extern u8 *sVabBody;   /**< the sound VAB body address: worked out for bank 0, reused for bank 1 */
extern s16 gBgmVolume; /**< the BGM sequence's volume, as the fade sets it */
extern s16 gSeVab;     /**< VAB id of the swappable bank, the one gVabBank names */
extern s16 D_80095AEA; /**< cleared by InitSeSeqs; read by no code */
extern u16 gVabBank;   /**< the numbered VAB bank open as gSeVab, 1 on */
extern s16 gMovieOffsetX;       /**< where the picture starts across a frame area, in VRAM units */
extern s16 gMovieDisplayHeight; /**< the movie picture's height in lines, centred on 240 */

/* The disc file table's 26th and 27th copies (disc_files.inc), the
 * executable's last: LoadSoundData and LoadVabBank read the SEQ pack and
 * the VAB banks from the second, at sFiles27BootLoc[4] on. */
#define DISC_FILE(x) sFiles26##x /**< names the 26th copy's array x */
#include "disc_files.inc"
#define DISC_FILE(x) sFiles27##x /**< names the 27th copy's array x */
#include "disc_files.inc"

/** @brief Stage 0's overlay hook for a collected can group; the overlay
 *         defines it empty.
 *  @param index the can group */
void func_800FA610(s16 index);
void StartMovieRead(CdlLOC *loc);

/** @brief Runs the stage overlay's hook for a collected can of group
 *         `index`; only STAGE_1_1 has one. */
void OnCanCollected(s16 index) {
    s32 mode = gStage;

    if (mode != 1) {
        if (mode < 2) {
            if (mode == 0) {
                func_800FA610(index);
            }
        }
    }
}

void OnMovieSliceDecoded(void);
void InitMovieDecoder(DECENV *dec, s32 x0, s32 y0, s32 x1, s32 y1);
void InitMovieStream(CdlLOC *loc, void (*callback)());
s32 DecodeNextMovieFrame(DECENV *dec);
void WaitMovieFrameDecoded(DECENV *dec, s32 mode);

extern CdlLOC gMovieLoc;      /**< where the movie being played starts on the disc */
extern s32 gMovieLastFrame;   /**< the frame number the movie ends at */
extern RECT gMovieClearRect;  /**< the VRAM area a movie clears to black: both frame areas */
extern DECENV gMovieDec;      /**< the movie decoder */
extern CdlFILE gMovieFile;    /**< the movie file CdSearchFile found */
extern DRAWENV gMovieDrawEnv; /**< the drawing environment while a movie plays */
extern DISPENV gMovieDispEnv; /**< the display environment while a movie plays, 24-bit */

/** @brief Plays the movie at gMovieLoc, or the file @p name, showing each
 *         frame in 24-bit colour until the last frame or START; then clears
 *         the screen, stops the stream and goes back to 16-bit display.
 *  @param name      the movie file's name; empty to play from gMovieLoc
 *  @param skippable 1 when START may end the movie early
 *  @return 1 when START skipped the movie, 0 when it played to the end */
s32 PlayMovieFile(char *name, s16 skippable) {
    CdlATV atv;
    s16 one;
    s32 id;
    s32 ret;

    ret = 0;
    if (*(s8 *)name != 0) {
        while (CdSearchFile(&gMovieFile, name) == 0) {
        }
        gMovieLoc.minute = gMovieFile.pos.minute;
        gMovieLoc.second = gMovieFile.pos.second;
        gMovieLoc.sector = gMovieFile.pos.sector;
    }
    InitMovieDecoder(&gMovieDec, 0, 0, 0, 240);
    InitMovieStream(&gMovieLoc, OnMovieSliceDecoded);
    DecodeNextMovieFrame(&gMovieDec);
    SetDefDispEnv(&gMovieDispEnv, 0, 0, 0, 0);
    SetDefDrawEnv(&gMovieDrawEnv, 0, 0, 0, 0);
    one = 1;
    gMovieDispEnv.disp.w = gMovieDispEnv.disp.w * 2 / 3;
    gMovieDispEnv.isrgb24 = one;
    PutDispEnv(&gMovieDispEnv);
    PutDrawEnv(&gMovieDrawEnv);
    SetDispMask(0);
    while (1) {
        DecDCTin(gMovieDec.vlcbuf[gMovieDec.vlcid], 3);
        DecDCTout((u_long *)gMovieDec.imgbuf, gMovieDec.slice.w * gMovieDec.slice.h / 2);
        DecodeNextMovieFrame(&gMovieDec);
        WaitMovieFrameDecoded(&gMovieDec, 0);
        VSync(0);
        id = gMovieDec.rectid == 0;
        SetDefDispEnv(&gMovieDispEnv, gMovieDec.rect[id].x, gMovieDec.rect[id].y,
                      gMovieDec.rect[id].w, gMovieDec.rect[id].h);
        SetDefDrawEnv(&gMovieDrawEnv, gMovieDec.rect[id].x, gMovieDec.rect[id].y,
                      gMovieDec.rect[id].w, gMovieDec.rect[id].h);
        gMovieDispEnv.disp.w = gMovieDispEnv.disp.w * 2 / 3;
        gMovieDispEnv.isrgb24 = 1;
        PutDispEnv(&gMovieDispEnv);
        PutDrawEnv(&gMovieDrawEnv);
        SetDispMask(1);
        if (gMovieEnded == one) {
            atv.val3 = 0;
            atv.val2 = 0;
            atv.val1 = 0;
            atv.val0 = 0;
            CdMix(&atv);
            VSync(0);
            while (CdControl(CdlPause, 0, 0) == 0) {
            }
            setRECT(&gMovieClearRect, 0, 0, 480, 480);
            ClearImage(&gMovieClearRect, 0, 0, 0);
            DrawSync(0);
            break;
        }
        ReadPad(0);
        if ((gPadTrig & PADstart) && skippable == one) {
            atv.val3 = 0;
            atv.val2 = 0;
            atv.val1 = 0;
            atv.val0 = 0;
            CdMix(&atv);
            VSync(0);
            while (CdControl(CdlPause, 0, 0) == 0) {
            }
            setRECT(&gMovieClearRect, 0, 0, 480, 480);
            ClearImage(&gMovieClearRect, 0, 0, 0);
            DrawSync(0);
            ret = 1;
            break;
        }
    }
    MoveImage(&gMovieDec.rect[gMovieDec.rectid], gMovieDec.rect[id].x, gMovieDec.rect[id].y);
    DrawSync(0);
    DecDCToutCallback(0);
    StUnSetRing();
    while (CdControlB(CdlPause, 0, 0) == 0) {
    }
    ChangeClearPAD(0);
    /* MATCHING: retail keeps this load and drops the store. */
    gMovieDispEnv.disp.w = gMovieDispEnv.disp.w;
    gMovieDispEnv.isrgb24 = 0;
    SetDefDispEnv(&gMovieDispEnv, gMovieDec.rect[id].x, gMovieDec.rect[id].y, gMovieDec.rect[id].w,
                  gMovieDec.rect[id].h);
    SetDefDrawEnv(&gMovieDrawEnv, gMovieDec.rect[id].x, gMovieDec.rect[id].y, gMovieDec.rect[id].w,
                  gMovieDec.rect[id].h);
    return ret;
}

extern s32 StCdIntrFlag; /**< libcd streaming's flag: set when StCdInterrupt is owed */

/** @brief The MDEC's output callback: loads the slice just decoded into
 *         VRAM and starts the next, or, once the frame area is full, marks
 *         the frame done and moves the slice to the other frame area. */
void OnMovieSliceDecoded(void) {
    if (StCdIntrFlag) {
        StCdInterrupt();
        StCdIntrFlag = 0;
    }
    LoadImage(&gMovieDec.slice, (u_long *)gMovieDec.imgbuf);
    gMovieDec.slice.x += gMovieDec.slice.w;
    if (gMovieDec.slice.x < gMovieDec.rect[gMovieDec.rectid].x + gMovieDec.rect[gMovieDec.rectid].w) {
        DecDCTout((u_long *)gMovieDec.imgbuf, gMovieDec.slice.w * gMovieDec.slice.h / 2);
    } else {
        gMovieDec.isdone = 1;
        gMovieDec.rectid = gMovieDec.rectid == 0;
        gMovieDec.slice.x = gMovieDec.rect[gMovieDec.rectid].x + gMovieOffsetX;
        gMovieDec.slice.y = gMovieDec.rect[gMovieDec.rectid].y + (240 - gMovieDisplayHeight) / 2;
    }
}

extern u_long gMovieFrameNo;   /**< the number of the movie frame last read; read by no code */
extern StHEADER *gMovieHeader; /**< the sector header of the movie frame last read */
extern u_long *gMovieAddr;     /**< the data of the movie frame last read */
extern s32 gMovieWidth;        /**< the movie frame's width in pixels */
extern s32 gMovieHeight;       /**< the movie frame's height in lines */

/* sound's small data, in address order. */
s16 D_800956C0 SDATA = 320; /**< read by no code */
s16 gMovieOffsetX SDATA = 0;
s32 gMovieWidth SDATA = 0;
s32 gMovieHeight SDATA = 0;
s32 D_800956CC SDATA = 0; /**< read by no code */

/** @brief Waits for the next movie frame in the stream's ring buffer, notes
 *         when it is the last, and sizes the decoder's frame areas to it,
 *         clearing VRAM when the size changes.
 *  @param dec the movie decoder
 *  @return the frame's data, or NULL when none arrived in time */
u_long *GetNextMovieFrame(DECENV *dec) {
    gMovieTimeout = 0x800000;
    while (StGetNext(&gMovieAddr, (u_long **)&gMovieHeader)) {
        if (--gMovieTimeout == 0) {
            return NULL;
        }
    }
    gMovieFrameNo = gMovieHeader->frameCount;
    if (gMovieHeader->frameCount >= gMovieLastFrame) {
        gMovieEnded = 1;
    }
    if (gMovieWidth != gMovieHeader->width || gMovieHeight != gMovieHeader->height) {
        setRECT(&gMovieClearRect, 0, 0, 480, 480);
        ClearImage(&gMovieClearRect, 0, 0, 0);
        DrawSync(0);
        gMovieWidth = gMovieHeader->width;
        gMovieHeight = gMovieHeader->height;
    }
    dec->rect[0].w = dec->rect[1].w = gMovieWidth * 3 / 2;
    dec->slice.h = dec->rect[0].h = dec->rect[1].h = gMovieHeight;
    return gMovieAddr;
}

void PlayMovie(char *name, CdlLOC *loc, s32 lastFrame, s16 height, s16 skippable) {
    CdlATV atv;

    gMovieLastFrame = lastFrame;
    gMovieDisplayHeight = height;
    gMovieLoc.minute = loc->minute;
    gMovieLoc.second = loc->second;
    gMovieLoc.sector = loc->sector;
    setRECT(&gMovieClearRect, 0, 0, 480, 480);
    ClearImage(&gMovieClearRect, 0, 0, 0);
    DrawSync(0);
    /* MATCHING: gStereo by address; retail reaches it `lui` only here. */
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
    PlayMovieFile(name, skippable);
}

/** @brief Sets up the movie decoder: its buffers, its two 480 by 240 frame
 *         areas and its first slice, which starts gMovieOffsetX across and
 *         centres gMovieDisplayHeight lines.
 *  @param dec the movie decoder
 *  @param x0  the first frame area's left edge in VRAM
 *  @param y0  the first frame area's top edge
 *  @param x1  the second frame area's left edge
 *  @param y1  the second frame area's top edge */
void InitMovieDecoder(DECENV *dec, s32 x0, s32 y0, s32 x1, s32 y1) {
    dec->vlcbuf[0] = (u_long *)RAM_MOVIE_VLC0;
    dec->vlcbuf[1] = (u_long *)RAM_MOVIE_VLC1;
    dec->vlcid = 0;
    dec->imgbuf = (u_short *)RAM_MOVIE_IMAGE;
    dec->rectid = 0;
    dec->isdone = 0;
    setRECT(&dec->rect[0], x0, y0, 480, 240);
    setRECT(&dec->rect[1], x1, y1, 480, 240);
    setRECT(&dec->slice, x0 + gMovieOffsetX, y0 + (240 - gMovieDisplayHeight) / 2, 24,
            gMovieDisplayHeight);
}

/** @brief Resets the MDEC, sets up a 32-sector ring buffer for a 24-bit
 *         stream of every frame, and starts reading the movie.
 *  @param loc      where the movie file starts on the disc
 *  @param callback the MDEC's output callback, run after each slice */
void InitMovieStream(CdlLOC *loc, void (*callback)()) {
    DecDCTReset(0);
    gMovieEnded = 0;
    DecDCToutCallback(callback);
    StSetRing((u_long *)RAM_MOVIE_RING, 32);
    StSetStream(1, 1, -1, 0, 0);
    StartMovieRead(loc);
}

/** @brief Takes the next movie frame from the stream, VLC-decodes it into
 *         the decoder's other VLC buffer and frees its ring sectors.
 *  @param dec the movie decoder
 *  @return 0, or -1 when no frame arrived in time */
s32 DecodeNextMovieFrame(DECENV *dec) {
    u_long *next;

    gMovieTimeout = 0x800000;
    while ((next = GetNextMovieFrame(dec)) == NULL) {
        if (--gMovieTimeout == 0) {
            return -1;
        }
    }
    dec->vlcid = dec->vlcid == 0;
    DecDCTvlc(next, dec->vlcbuf[dec->vlcid]);
    StFreeRing(next);
    return 0;
}

/** @brief Waits until the MDEC has finished the frame being decoded; on a
 *         time-out prints a message and treats the frame as done.
 *  @param dec  the movie decoder
 *  @param mode unused */
void WaitMovieFrameDecoded(DECENV *dec, s32 mode) {
    gMovieTimeout = 0x800000;
    while (dec->isdone == 0) {
        if (--gMovieTimeout == 0) {
            printf("time out in decoding !\n");
            dec->isdone = 1;
            dec->rectid = dec->rectid == 0;
            dec->slice.x = dec->rect[dec->rectid].x;
            dec->slice.y = dec->rect[dec->rectid].y;
        }
    }
    dec->isdone = 0;
}

/** @brief Seeks the drive to a movie and starts the double-speed streamed
 *         read, retrying each command until the drive takes it.
 *  @param loc where the movie file starts on the disc */
void StartMovieRead(CdlLOC *loc) {
    do {
        while (CdControl(CdlSetloc, (u_char *)loc, 0) == 0) {
        }
    } while (CdRead2(CdlModeStream | CdlModeSpeed | CdlModeRT) == 0);
}

/** @brief Opens the VAB file just read to the work buffer in one of the two
 *         VAB slots and sends its waveforms to the sound RAM.
 *
 *  Bank 0's header is copied to just past the sound pack's last SEQ;
 *  bank 1's goes straight after bank 0's, at sVabBody.
 *  @param bank 0 for gBgmVab's slot, 1 for gSeVab's
 *  @return 0, or -1 when the VAB header did not open (gVabError is set) */
s32 LoadVab(s16 bank) {
    u32 *vh = (u32 *)RAM_WORK_A;
    u32 *rec;
    u8 *dst;
    u32 src;
    s16 *slot;
    s16 vab;

    gVabError = 0;
    switch (bank) {
        case 0:
            rec = (u32 *)RAM_SOUND_PACK;
            rec += (u16)(((u16 *)rec)[7] - 1) * 4;
            dst = (u8 *)rec[0];
            dst += rec[1];
            dst += RAM_SOUND_PACK;
            sVabBody = dst + vh[1];
            break;
        case 1:
            dst = sVabBody;
            break;
    }
    src = RAM_WORK_A;
    memcpy(dst, vh[0] + src, vh[1]);
    src = 0; /* MATCHING: a dead store, so cse rebuilds RAM_WORK_A below. */
    vab = SsVabOpenHead(dst, -1);
    slot = &(&gBgmVab)[bank];
    *slot = vab;
    if (vab != -1) {
        *slot = SsVabTransBody((u8 *)(vh[4] + RAM_WORK_A), vab);
        SsVabTransCompleted(1);
        return 0;
    }
    gVabError = vab;
    return -1;
}

/** @brief Sets up the CD audio input to the sound mix and the eight
 *         sound-effect slots: opens the pack's first SEQ in each, on gSeVab,
 *         and silences every slot.
 *  @return 0 */
s32 InitSeSeqs(void) {
    s16 i;
    u32 *hdr;

    SsSetSerialAttr(SS_SERIAL_A, SS_MIX, SS_SON);
    SsSetSerialVol(SS_SERIAL_A, 127, 127);
    D_80095AEA = 0;
    gSeSlot = 0;
    hdr = (u32 *)RAM_SOUND_PACK;
    for (i = 2; i < 10; i++) {
        gSeqIds[i] = SsSeqOpen((unsigned long *)(*hdr + RAM_SOUND_PACK), gSeVab);
    }
    for (i = 0; i < 10; i++) {
        SsSeqSetVol(gSeqIds[i], 0, 0);
    }
    return 0;
}

/** @brief Stops the music and starts SEQ @p song of the sound pack in its
 *         place, looping, on gBgmVab, at volume 100, ending any fade.
 *  @param song the SEQ's index in the sound pack
 *  @return -1 when a sound bank failed to load; no value otherwise */
s32 PlayBgm(u16 song) {
    u32 *hdr;

    SsSeqStop(gSeqIds[0]);
    SsSeqClose(gSeqIds[0]);
    if (gVabError != 0) {
        return -1;
    }
    hdr = (u32 *)RAM_SOUND_PACK;
    hdr += song * 4;
    gSeqIds[0] = SsSeqOpen((unsigned long *)(*hdr + RAM_SOUND_PACK), gBgmVab);
    SsSeqPlay(gSeqIds[0], 1, 0);
    gBgmVolume = 100;
    SsSeqSetVol(gSeqIds[0], 100, 100);
    gSoundFade = FADE_NONE;
}

/* MATCHING: model defines it with s16 arguments and result; this unit
 * passes and reads them as s32. */
/** @brief Model's linear interpolation: the value @p step of @p steps of
 *         the way from @p from to @p to.
 *  @param from  the value at step 0
 *  @param to    the value at step @p steps
 *  @param step  how far along
 *  @param steps the number of steps
 *  @return the interpolated value */
s32 Lerp(s32 from, s32 to, s32 step, s32 steps);

extern s16 gSoundFadeTimer; /**< the frames a fade has left, from 50 */
extern s16 gSeVolume;       /**< the sound effects' volume, as the fade sets it */

void UpdateSoundFade(void) {
    s16 i;

    switch (gSoundFade) {
        case FADE_NONE:
            gSoundFadeTimer = 50;
            gSeVolume = 127;
            gBgmVolume = 127;
            gCdVolume = 80;
            break;
        case FADE_BGM:
            gBgmVolume = Lerp(0, 100, --gSoundFadeTimer, 50);
            gBgmVolume = gBgmVolume < 0 ? 0 : gBgmVolume > 127 ? 127 : gBgmVolume;
            if (gSoundFadeTimer == 0) {
                SsSeqStop(gSeqIds[0]);
                SsSeqClose(gSeqIds[0]);
                gSoundFade = FADE_NONE;
                gSeVolume = 127;
                gBgmVolume = 127;
            }
            SsSeqSetVol(gSeqIds[0], gBgmVolume, gBgmVolume);
            break;
        case FADE_ALL:
            gBgmVolume = Lerp(0, 100, --gSoundFadeTimer, 50);
            gSeVolume = gBgmVolume = gBgmVolume < 0 ? 0 : gBgmVolume > 127 ? 127 : gBgmVolume;
            SsSeqSetVol(gSeqIds[0], gBgmVolume, gBgmVolume);
            for (i = 2; i < 10; i++) {
                SsSeqSetVol(gSeqIds[i], gSeVolume, gSeVolume);
            }
            if (gSoundFadeTimer == 0) {
                SsSeqStop(gSeqIds[0]);
                SsSeqClose(gSeqIds[0]);
                for (i = 2; i < 10; i++) {
                    SsSeqStop(gSeqIds[i]);
                }
                for (i = 0; i < 10; i++) {
                    SsSeqSetVol(gSeqIds[i], 0, 0);
                }
                gSeVolume = 127;
                gBgmVolume = 127;
                gSoundFade = FADE_NONE;
                gCdVolume = 80;
            }
            /* falls through: FADE_ALL fades the CD audio too */
        case FADE_CD:
            gCdVolume = Lerp(0, 80, --gSoundFadeTimer, 50);
            gCdVolume = gCdVolume < 0 ? 0 : gCdVolume > 80 ? 80 : gCdVolume;
            SetCdVolume(gCdVolume);
            if (gSoundFadeTimer == 0) {
                gCdVolume = 80;
                StopCdTrack();
                gSoundFade = FADE_NONE;
            }
            break;
    }
    if (gSoundFadeTimer < 0) {
        gSoundFade = FADE_NONE;
    }
}

/** @brief Which open VAB plays each sound from id 70 on: 0 gBgmVab's, 1 the next
 *  one (gSeVab's). Ids below 70 all play from gBgmVab's. */
u8 sSeVabBanks[64] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1,
    1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0,
};

/** @brief Plays SEQ @p id of the sound pack once as a sound effect, in the
 *         first effect slot not playing (the last slot when all are), on
 *         the VAB sSeVabBanks picks, at full volume. @p id is the SEQ's
 *         index in the sound pack.
 *  @return -1 when a sound bank failed to load; no value otherwise */
s32 PlaySe(s32 id) {
    s16 *seq;
    s16 i;
    u8 bank;
    s16 *vabs;
    u32 *hdr;

    for (gSeSlot = 2; gSeSlot < 10; gSeSlot++) {
        i = gSeSlot;
        seq = &gSeqIds[(u16)i];
        if (SsIsEos(*seq, 0) == 0) {
            break;
        }
    }
    SsSeqClose(*seq);
    if (gVabError != 0) {
        return -1;
    }
    hdr = (u32 *)RAM_SOUND_PACK;
    hdr += (u16)id * 4;
    vabs = &gBgmVab;
    if ((u16)id < 70) {
        bank = 0;
    } else {
        bank = sSeVabBanks[(u16)id - 70];
    }
    gSeqIds[(u16)i] = SsSeqOpen((unsigned long *)(*hdr + RAM_SOUND_PACK), vabs[bank]);
    SsSeqPlay(gSeqIds[(u16)i], 1, 1);
    SsSeqSetVol(gSeqIds[(u16)i], 127, 127);
    if (++gSeSlot >= 10) {
        gSeSlot = 2;
    }
}

/** @brief Loads the game's sound at boot, one gSubStep a call: reads the
 *         sound pack and the first VAB, opens it, reads and opens VAB bank 1
 *         with the effect slots, then turns on the Studio B reverb.
 *  @return 1 once everything is loaded, else 0 */
s32 LoadSoundData(void) {
    s32 ret = 0;

    switch (gSubStep) {
        case 0:
            gCdLoadLoc[0] = (s32)&sFiles27BootLoc[4];
            gCdLoadDest[0] = RAM_SOUND_PACK;
            gCdLoadLoc[1] = (s32)&sFiles27BootLoc[6];
            gCdLoadDest[1] = RAM_WORK_A;
            gCdLoadCount = 2;
            gCdLoadState = CD_LOAD_SEEK;
            gCdLoadSectors[0] = sFiles27BootSectors[4];
            gCdLoadSectors[1] = sFiles27BootSectors[6];
            gSubStep++;
            break;
        case 1:
            if (gCdLoadState == CD_LOAD_DONE) {
                LoadVab(0);
                gSubStep++;
            }
            break;
        case 2:
            gCdLoadLoc[0] = (s32)&sFiles27BootLoc[7];
            gCdLoadDest[0] = RAM_WORK_A;
            gCdLoadCount = 1;
            gCdLoadState = CD_LOAD_SEEK;
            gCdLoadSectors[0] = sFiles27BootSectors[7];
            gSubStep++;
            break;
        case 3:
            if (gCdLoadState == CD_LOAD_DONE) {
                LoadVab(1);
                InitSeSeqs();
                gVabBank = 1;
                gSubStep++;
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

void InitSound(void) {
    SsInit();
    SsSetTableSize(sSeqAttrTable, 9, 1);
    SsSetTickMode(1);
}

/** @brief Sets the master volume and the Studio B reverb as LoadSoundData's
 *         last step does, silencing both for a frame first; no code calls it. */
void InitReverb(void) {
    SsSetMVol(0, 0);
    SsSetRVol(0, 0);
    SsUtSetReverbType(SS_REV_TYPE_STUDIO_B);
    VSync(0);
    SsSetMVol(127, 127);
    SsSetRVol(120, 120);
    SsUtReverbOn();
    SsUtSetReverbDepth(48, 48);
}

void SetSoundFade(u8 mode) {
    gSoundFade = mode;
}

void SetCdVolume(s32 vol) {
    CdlATV atv;

    if (gStereo) {
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

/** @brief Stops the music and closes its SEQ. */
void StopBgm(void) {
    SsSeqStop(gSeqIds[0]);
    SsSeqClose(gSeqIds[0]);
}

/** @brief Stops every sound-effect slot whose sequence is still playing. */
void StopFinishedSe(void) {
    s16 *seq;

    for (gSeSlot = 2; gSeSlot < 10; gSeSlot++) {
        seq = &gSeqIds[gSeSlot];
        if (*seq != 0) {
            if (SsIsEos(*seq, 0) == 1) {
                SsSeqStop(*seq);
            }
        }
    }
}

/** @brief Swaps VAB bank @p bank in as gSeVab, one gVabLoadStep a call:
 *         reads it from the disc to the work buffer, then closes the old
 *         bank and opens it.
 *  @param bank the numbered bank, 1 on; 0 asks for nothing
 *  @return 1 once the bank is in place (at once when it already is), else 0 */
s32 LoadVabBank(u16 bank) {
    s32 ret = 0;

    if (bank == 0) {
        return 1;
    }
    if (bank == gVabBank) {
        return 1;
    }
    switch (gVabLoadStep) {
        case 0:
            gCdLoadLoc[0] = (s32)&sFiles27BootLoc[bank + 6];
            gCdLoadSectors[0] = sFiles27BootSectors[bank + 6];
            gCdLoadDest[0] = RAM_WORK_A;
            gCdLoadCount = 1;
            gCdLoadState = CD_LOAD_SEEK;
            gVabLoadStep++;
            break;
        case 1:
            if (gCdLoadState == CD_LOAD_DONE) {
                SsVabClose(gSeVab);
                gVabBank = bank;
                LoadVab(1);
                ret = 1;
            }
            break;
    }
    return ret;
}

void PlayXa(u8 chan) {
    CdlFILTER filter;
    u_char mode[4];
    CdlATV atv;

    if (gStereo) {
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

void StopXa(void) {
    CdlATV vol;

    vol.val3 = 0;
    vol.val2 = 0;
    vol.val1 = 0;
    vol.val0 = 0;
    CdMix(&vol);
    CdControlF(CdlPause, 0);
}

/** @brief The C runtime hook main calls first; empty in this program. */
void __main(void) {}
