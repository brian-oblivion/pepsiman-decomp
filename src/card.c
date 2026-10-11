#include "common.h"
#include "libapi.h"
#include "string.h"

extern u8 *gSaveBuf;
extern u8 *D_800959D4;
extern s32 gCardSwIoe;
extern s32 gCardSwError;
extern s32 gCardSwTimeout;
extern s32 gCardSwNew;
extern s32 gCardHwIoe;
extern s32 gCardHwError;
extern s32 gCardHwTimeout;
extern s32 gCardHwNew;

/** @brief A file's size in 8 KiB blocks, stored as a word, summed as a halfword. */
typedef union {
    s32 w;  /**< the block count as written */
    u16 lo; /**< its low half, which the sums read */
} BlockCount;

/** @brief One memory card file found by the directory scan. */
typedef struct {
    char name[20];     /**< file name, from the directory entry */
    BlockCount blocks; /**< size in 8 KiB blocks */
} CardFile;

extern CardFile gCardFiles[];
extern u16 gCardStatus;
extern s32 gCardFileCount;   /**< number of records in use */
extern s16 gCardError;       /**< 11 after a failed format, 12 after a failed erase */
extern char *gCardFileName;  /**< name of the file last erased */
extern char sCardDevice[];   /**< "bu10:" */
extern char sToolCardFile[]; /**< "bu10:BISLPS-12345PEPTOOL" */
extern char sHitCardFile[];  /**< "bu10:BISLPS-67890PEPTOOL" */

/** @brief A loaded record bank file: a count word, then the bank. */
typedef struct {
    s32 count;  /**< entry count, copied to the bank's count global */
    u8 data[4]; /**< the bank itself; real size unknown */
} BankFile;

/** @brief The 27 title bytes copied into a save header in one block move. */
typedef struct {
    u8 b[27]; /**< Shift-JIS title text */
} CardTitle;

/** @brief A 16-colour CLUT, copied as one block. */
typedef struct {
    u8 b[0x20]; /**< 16 halfword colours */
} IconClut;

/** @brief One 16x16 4-bit icon frame, copied as one block. */
typedef struct {
    u8 b[0x80]; /**< pixel data */
} IconBits;

/** @brief A 4-bit 16x16 TIM image as stored in rodata. */
typedef struct {
    u8 head[0x14];  /**< TIM id, flags and CLUT block header */
    IconClut clut;  /**< the CLUT */
    u8 imgHead[12]; /**< image block header */
    IconBits bits;  /**< the pixels */
} IconTim;

/** @brief The start of a memory card file header. */
typedef struct {
    u8 magic[2];     /**< "SC" */
    u8 iconFlag;     /**< icon display flag */
    u8 blocks;       /**< blocks the file uses */
    CardTitle title; /**< Shift-JIS title */
} CardHeader;

extern CardHeader gCardHeader;
extern u8 sToolCardTitle[]; /**< title of the first save file */
extern u8 sHitCardTitle[];  /**< title of the second save file */
extern u8 sToolCardIcon0[]; /**< first save file's icon frames, three TIMs */
extern u8 sToolCardIcon1[];
extern u8 sToolCardIcon2[];
extern u8 sHitCardIcon0[]; /**< second save file's icon frames, three TIMs */
extern u8 sHitCardIcon1[];
extern u8 sHitCardIcon2[];
/* MATCHING: the icon CLUT and frames inside gCardHeader (+0x60, +0x100,
 * +0x180); as fields of a full CardHeader the block copies change. */
extern u8 sHeaderIcon0[]; /**< the card header's icon CLUT and first frame */
extern u8 sHeaderIcon1[]; /**< the card header's second icon frame */
extern u8 sHeaderIcon2[]; /**< the card header's third icon frame */

/** @brief The 0x200-byte header block copied to the save buffer. */
typedef struct {
    u8 b[0x200]; /**< header bytes */
} SaveImage;

extern u16 gSaveSum; /**< checksum of the save buffer */

s16 ProbeCard(void);
s32 ReadSave1(void);
s32 RewriteSave1(void);
s32 CreateSave1(void);
s32 ScanCardFiles(void);
void CopySave1Icon(void);
s32 FormatCard(void);
s32 EraseSave1(void);
s32 FindSave1(void);
s32 CheckSave1Space(void);
u16 SumSaveBuffer(void);
void BuildSave1Header(void);
s32 ReadSave2(void);
s32 RewriteSave2(void);
s32 CreateSave2(void);
s32 EraseSave2(void);
s32 FindSave2(void);
s32 CheckSave2Space(void);
void BuildSave2Header(void);
void EnableCardEvents(void);
void DisableCardEvents(void);
s16 WaitCardSwEvent(void);
s16 WaitCardHwEvent(void);
void ClearCardSwEvents(void);
void ClearCardHwEvents(void);
void CopySave2Icon(void);

void SetHitBanks(BankFile *a, BankFile *b) {
    u8 *bank; /* MATCHING: one local for both bank addresses */
    u32 i;

    bank = a->data;
    gHitPoints = (s32)bank;
    bank = b->data;
    gHitQuads = (s32)bank;
    gHitPointSections = a->count;
    gHitQuadSections = b->count;
    for (i = 0; i < 80; i++) {
        D_800D8D20[i].unk72 = -1;
        D_800D8D20[i].unk74 = 0;
    }
    ApplyHitPoints();
    ApplyHitQuads();
}

/** @brief Runs one memory card operation on the first save file; mode 0 reads, 1 rewrites, 2 creates, 3 formats, 4 erases, 5 checks. */
s32 RunCardOp1(s16 mode) {
    s32 unused[2]; /* MATCHING: retail's frame holds 8 unused bytes */
    s32 ret;
    s16 retry;
    u16 sum;
    s32 off; /* MATCHING: a constant offset is split into %hi + low; a register one is not */

    ret = 0;
    retry = 0;
    gCardStatus = 0;
    gCardError = 0;
    D_800959D4 = (u8 *)0x8018D000;
    gSaveBuf = (u8 *)0x8016D000;
    EnableCardEvents();
again: /* MATCHING: a goto loop; a C loop hoists (s16)mode out of it */
    gCardStatus = ProbeCard();
    switch (gCardStatus) {
        case 0: /* MATCHING: the empty case gives retail's compare tree */
            break;
        case 1:
            if (retry < 16) {
                retry++;
                goto again;
            }
            ret = -2;
            goto end;
        case 2:
            gCardFileCount = 0;
            ret = 2;
            goto end;
        case 3:
            gCardFileCount = 0;
            ret = 4;
            if (mode == 3) {
                goto format;
            }
            goto end;
    }
    if (mode == -1) {
        goto end;
    }
    ScanCardFiles();
    if (mode == 5) {
        if (FindSave1() != -1) {
            ret = 0;
        } else {
            ret = CheckSave1Space() == -1;
        }
    } else if (mode == 0 || mode == 1 || mode == 4) {
        if (FindSave1() == -1) {
            ret = 3;
            goto end;
        }
        switch (mode) {
            case 0:
                ret = ReadSave1();
                break;
            case 1:
                ret = RewriteSave1();
                break;
            case 4:
                ret = EraseSave1();
                break;
        }
    } else if (mode == 2) {
        if (CheckSave1Space() == -1) {
            ret = 1;
        } else {
            BuildSave1Header();
            *(SaveImage *)gSaveBuf = *(SaveImage *)&gCardHeader;
            sum = SumSaveBuffer();
            gSaveSum = sum;
            off = 0x1DFFE;
            *(u16 *)(D_800959D4 + off) = sum;
            ret = CreateSave1();
        }
    } else if (mode == 3) {
    format:
        ret = FormatCard();
    }
end:
    DisableCardEvents();
    return ret;
}

s16 ProbeCard(void) {
    s16 retry;
    s16 r;

    retry = 0;
    do {
        ClearCardSwEvents();
        _card_info(0x10);
        r = WaitCardSwEvent();
        switch (r) {
            case 1: /* MATCHING: the empty case gives retail's compare tree */
                break;
            case 2:
                retry++;
                if (retry >= 11) {
                    return 2;
                }
                break;
            case 3:
                ClearCardHwEvents();
                _card_clear(0x10);
                WaitCardHwEvent();
                D_80095A14 = 1;
                break;
        }
        ClearCardSwEvents();
        _card_load(0x10);
        r = WaitCardSwEvent();
    } while (r == 1 || r == 2);
    if (r == 0) {
        return 0;
    }
    return r;
}

/** @brief Reads the first save file into the save buffer, up to ten tries per step; 0, or -1 with the failed step recorded. */
s32 ReadSave1(void) {
    s32 ret;
    s16 i;
    s32 fd;
    u8 *buf;
    s16 tries;

    ret = 0;
    for (i = 0; i < 10; i++) {
        fd = open(sToolCardFile, 1);
        if (fd != -1) {
            goto opened;
        }
    }
    gCardError = 1;
    ret = -1;
    goto end;
opened:
    for (i = 0; i < 10; i++) {
        if (lseek(fd, 0, 0) != -1) {
            goto seeked;
        }
    }
    close(fd);
    gCardError = 2;
    ret = -1;
    goto end;
seeked:
    buf = gSaveBuf;
    for (tries = 0; tries < 10; tries++) {
        if (read(fd, buf, 0x1E000) == 0x1E000) {
            goto done;
        }
        for (i = 0; i < 10; i++) {
            if (lseek(fd, 0, 0) != -1) {
                goto reseeked;
            }
        }
        close(fd);
        gCardError = 2;
        ret = -1;
        goto end;
    reseeked:;
    }
    close(fd);
    gCardError = 3;
    ret = -1;
    goto end;
done:
    close(fd);
end:
    return ret;
}

/** @brief Rewrites the first save file in place, up to ten tries per step; 0, or -1 with the failed step recorded. */
s32 RewriteSave1(void) {
    s32 ret;
    s16 i;
    s32 fd;
    u8 *buf;

    ret = 0;
    for (i = 0; i < 10; i++) {
        fd = open(sToolCardFile, 2);
        if (fd != -1) {
            goto opened;
        }
    }
    ret = -1;
    gCardError = 5;
    goto end;
opened:
    for (i = 0; i < 10; i++) {
        if (lseek(fd, 0, 0) != -1) {
            goto seeked;
        }
    }
    close(fd);
    ret = -1;
    gCardError = 6;
    goto end;
seeked:
    buf = gSaveBuf;
    for (i = 0; i < 10; i++) {
        if (write(fd, buf, 0x1E000) == 0x1E000) {
            goto written;
        }
        for (i = 0; i < 10; i++) {
            if (lseek(fd, 0, 0) != -1) {
                goto reseeked;
            }
        }
        close(fd);
        ret = -1;
        gCardError = 6;
        goto end;
    reseeked:;
    }
    close(fd);
    ret = -1;
    gCardError = 7;
    goto end;
written:
    close(fd);
end:
    return ret;
}

/** @brief Creates and writes the first save file, up to ten tries per step; 0, or -1 with the failed step recorded. */
s32 CreateSave1(void) {
    s32 ret;
    s16 i;
    s32 fd;

    ret = 0;
    for (i = 0; i < 10; i++) {
        fd = open(sToolCardFile, 0xF0200);
        if (fd != -1) {
            goto created;
        }
    }
    /* MATCHING: ret before the code keeps the three fail tails apart */
    ret = -1;
    gCardError = 8;
    goto end;
created:
    close(fd);
    for (i = 0; i < 10; i++) {
        fd = open(sToolCardFile, 2);
        if (fd != -1) {
            goto opened;
        }
    }
    ret = -1;
    gCardError = 9;
    goto end;
opened:
    for (i = 0; i < 10; i++) {
        if (write(fd, gSaveBuf, 0x1E000) == 0x1E000) {
            goto written;
        }
    }
    close(fd);
    ret = -1;
    gCardError = 10;
    goto end;
written:
    close(fd);
end:
    return ret;
}

s32 ScanCardFiles(void) {
    struct DIRENTRY de;
    CardFile *rec;
    s32 i;

    i = 0;
    gCardFileCount = 0;
    if (firstfile(sCardDevice, &de) == NULL) {
        gCardStatus = 0xFFFF;
        return 0xFFFF;
    }
    rec = gCardFiles;
loop:
    strcpy(rec->name, de.name);
    rec->blocks.w = de.size / 8192;
    gCardFileCount++;
    /* MATCHING: the loop notes keep i++ just above the call. */
    do {
        i++;
    } while (0);
    if (nextfile(&de) == NULL) {
        gCardStatus = 0;
        return 0;
    }
    rec = &gCardFiles[i];
    goto loop;
    /* MATCHING: unreachable; its label keeps jump.c from moving the exit. */
    for (;;) {
    }
}

/** @brief Copies the first save file's three icon TIMs into the card header's CLUT and frames. */
void CopySave1Icon(void) {
    IconTim tim[3];

    tim[0] = *(IconTim *)sToolCardIcon0;
    tim[1] = *(IconTim *)sToolCardIcon1;
    tim[2] = *(IconTim *)sToolCardIcon2;
    *(IconClut *)sHeaderIcon0 = tim[0].clut;
    *(IconBits *)(sHeaderIcon0 + 0x20) = tim[0].bits;
    *(IconBits *)sHeaderIcon1 = tim[1].bits;
    *(IconBits *)sHeaderIcon2 = tim[2].bits;
}

/** @brief Opens the eight memory card events (four software, four hardware) and leaves them disabled. */
void OpenCardEvents(void) {
    EnterCriticalSection();
    gCardSwIoe = OpenEvent(0xF4000001, 4, 0x2000, NULL);
    gCardSwError = OpenEvent(0xF4000001, 0x8000, 0x2000, NULL);
    gCardSwTimeout = OpenEvent(0xF4000001, 0x100, 0x2000, NULL);
    gCardSwNew = OpenEvent(0xF4000001, 0x2000, 0x2000, NULL);
    gCardHwIoe = OpenEvent(0xF0000011, 4, 0x2000, NULL);
    gCardHwError = OpenEvent(0xF0000011, 0x8000, 0x2000, NULL);
    gCardHwTimeout = OpenEvent(0xF0000011, 0x100, 0x2000, NULL);
    gCardHwNew = OpenEvent(0xF0000011, 0x2000, 0x2000, NULL);
    ExitCriticalSection();
    DisableEvent(gCardSwIoe);
    DisableEvent(gCardSwError);
    DisableEvent(gCardSwTimeout);
    DisableEvent(gCardSwNew);
    DisableEvent(gCardHwIoe);
    DisableEvent(gCardHwError);
    DisableEvent(gCardHwTimeout);
    DisableEvent(gCardHwNew);
}

/* MATCHING: s32, not s16: both callers test the result unextended. */
s32 CompareBytes(u8 *a, u8 *b, s16 n) {
    s16 i;

    for (i = 0; i < n; i++) {
        if (*a != *b) {
            return -1;
        }
        a++;
        b++;
    }
    return 0;
}

u16 SumSaveBuffer(void) {
    u8 *p;
    u16 sum;
    s32 i;
    u8 lo;
    u8 hi;

    p = gSaveBuf;
    sum = 0;
    for (i = 0; i < 0xEFFF; i++) {
        lo = *p++;
        hi = *p++;
        sum += lo | (hi << 8);
    }
    return sum;
}

s16 CheckSaveSum(u16 expected) {
    u8 *p;
    u16 sum;
    s32 i;
    u8 lo;
    u8 hi;

    p = gSaveBuf;
    sum = 0;
    for (i = 0; i < 0xEFFF; i++) {
        lo = *p++;
        hi = *p++;
        sum += lo | (hi << 8);
    }
    if (sum != expected) {
        return -1;
    }
    return 0;
}

void SetSave1Buffers(void) {
    D_800959D4 = (u8 *)0x8018D000;
    gSaveBuf = (u8 *)0x8016D000;
}

s16 CheckCard(void) {
    s16 result;

    result = 0;
    EnableCardEvents();
    ClearCardSwEvents();
    _card_info(0x10);
    if (WaitCardSwEvent() == 2) {
        result = -1;
    }
    DisableCardEvents();
    return result;
}

s32 FormatCard(void) {
    s32 ret;
    s16 i;

    ret = 0;
    for (i = 0; i < 10; i++) {
        if (format(sCardDevice) != 0) {
            return ret;
        }
    }
    gCardError = 11;
    return -1;
}

s32 EraseSave1(void) {
    s32 ret;
    s16 i;

    ret = 0;
    gCardFileName = sToolCardFile;
    for (i = 0; i < 10; i++) {
        if (erase(gCardFileName) != 0) {
            return ret;
        }
    }
    gCardError = 12;
    return -1;
}

INCLUDE_RODATA("asm/nonmatchings/card", sToolCardFile);

s32 FindSave1(void) {
    s16 i;

    gCardFileName = "BISLPS-12345PEPTOOL";
    for (i = 0; i < gCardFileCount; i++) {
        if (CompareBytes((u8 *)gCardFiles[i].name, (u8 *)gCardFileName, 0x14) == 0) {
            return 0;
        }
    }
    return -1;
}

s32 CheckSave1Space(void) {
    s16 sum;
    s16 i;

    sum = 0;
    for (i = 0; i < gCardFileCount; i++) {
        sum += gCardFiles[i].blocks.lo;
    }
    if (sum > 0) {
        return -1;
    }
    return 0;
}

void BuildSave1Header(void) {
    gCardHeader.magic[0] = 'S';
    gCardHeader.magic[1] = 'C';
    gCardHeader.iconFlag = 0x13;
    gCardHeader.blocks = 15;
    gCardHeader.title = *(CardTitle *)sToolCardTitle;
    CopySave1Icon();
}

void CloseCardEvents(void) {
    EnterCriticalSection();
    CloseEvent(gCardSwIoe);
    CloseEvent(gCardSwError);
    CloseEvent(gCardSwTimeout);
    CloseEvent(gCardSwNew);
    CloseEvent(gCardHwIoe);
    CloseEvent(gCardHwError);
    CloseEvent(gCardHwTimeout);
    CloseEvent(gCardHwNew);
    ExitCriticalSection();
}

void EnableCardEvents(void) {
    EnableEvent(gCardSwIoe);
    EnableEvent(gCardSwError);
    EnableEvent(gCardSwTimeout);
    EnableEvent(gCardSwNew);
    EnableEvent(gCardHwIoe);
    EnableEvent(gCardHwError);
    EnableEvent(gCardHwTimeout);
    EnableEvent(gCardHwNew);
}

void DisableCardEvents(void) {
    DisableEvent(gCardSwIoe);
    DisableEvent(gCardSwError);
    DisableEvent(gCardSwTimeout);
    DisableEvent(gCardSwNew);
    DisableEvent(gCardHwIoe);
    DisableEvent(gCardHwError);
    DisableEvent(gCardHwTimeout);
    DisableEvent(gCardHwNew);
}

s16 WaitCardSwEvent(void) {
    while (1) {
        if (TestEvent(gCardSwIoe) == 1) {
            return 0;
        }
        if (TestEvent(gCardSwError) == 1) {
            return 1;
        }
        if (TestEvent(gCardSwTimeout) == 1) {
            return 2;
        }
        if (TestEvent(gCardSwNew) == 1) {
            return 3;
        }
    }
}

s16 WaitCardHwEvent(void) {
    while (1) {
        if (TestEvent(gCardHwIoe) == 1) {
            return 0;
        }
        if (TestEvent(gCardHwError) == 1) {
            return 1;
        }
        if (TestEvent(gCardHwTimeout) == 1) {
            return 2;
        }
        if (TestEvent(gCardHwNew) == 1) {
            return 3;
        }
    }
}

void ClearCardSwEvents(void) {
    TestEvent(gCardSwIoe);
    TestEvent(gCardSwError);
    TestEvent(gCardSwTimeout);
    TestEvent(gCardSwNew);
}

void ClearCardHwEvents(void) {
    TestEvent(gCardHwIoe);
    TestEvent(gCardHwError);
    TestEvent(gCardHwTimeout);
    TestEvent(gCardHwNew);
}

void ZeroBytes(u8 *p, s32 n) {
    s32 i;

    for (i = 0; i < n; i++) {
        *p++ = 0;
    }
}

/** @brief Runs one memory card operation on the second save file; mode 0 reads, 1 rewrites, 2 creates, 3 formats, 4 erases, 5 checks. */
s32 RunCardOp2(s16 mode) {
    s32 unused[2]; /* MATCHING: retail's frame holds 8 unused bytes */
    s32 ret;
    s16 retry;
    u16 sum;
    s32 off; /* MATCHING: a constant offset is split into %hi + low; a register one is not */

    ret = 0;
    retry = 0;
    gCardStatus = 0;
    gCardError = 0;
    D_800959D4 = (u8 *)0x80195000;
    gSaveBuf = (u8 *)0x8018D000;
    EnableCardEvents();
again: /* MATCHING: a goto loop; a C loop hoists (s16)mode out of it */
    gCardStatus = ProbeCard();
    switch (gCardStatus) {
        case 0: /* MATCHING: the empty case gives retail's compare tree */
            break;
        case 1:
            if (retry < 16) {
                retry++;
                goto again;
            }
            ret = -2;
            goto end;
        case 2:
            gCardFileCount = 0;
            ret = 2;
            goto end;
        case 3:
            gCardFileCount = 0;
            ret = 4;
            if (mode == 3) {
                goto format;
            }
            goto end;
    }
    if (mode == -1) {
        goto end;
    }
    ScanCardFiles();
    if (mode == 5) {
        if (FindSave2() != -1) {
            ret = 0;
        } else {
            ret = CheckSave2Space() == -1;
        }
    } else if (mode == 0 || mode == 1 || mode == 4) {
        if (FindSave2() == -1) {
            ret = 3;
            goto end;
        }
        switch (mode) {
            case 0:
                ret = ReadSave2();
                break;
            case 1:
                ret = RewriteSave2();
                break;
            case 4:
                ret = EraseSave2();
                break;
        }
    } else if (mode == 2) {
        if (CheckSave2Space() == -1) {
            ret = 1;
        } else {
            BuildSave2Header();
            *(SaveImage *)gSaveBuf = *(SaveImage *)&gCardHeader;
            sum = SumSaveBuffer();
            gSaveSum = sum;
            off = 0x17FFE;
            *(u16 *)(D_800959D4 + off) = sum;
            ret = CreateSave2();
        }
    } else if (mode == 3) {
    format:
        ret = FormatCard();
    }
end:
    DisableCardEvents();
    return ret;
}

/** @brief Reads the second save file into the save buffer, up to ten tries per step; 0, or -1 with the failed step recorded. */
s32 ReadSave2(void) {
    s32 ret;
    s16 i;
    s32 fd;
    u8 *buf;
    s16 tries;

    ret = 0;
    for (i = 0; i < 10; i++) {
        fd = open(sHitCardFile, 1);
        if (fd != -1) {
            goto opened;
        }
    }
    gCardError = 1;
    ret = -1;
    goto end;
opened:
    for (i = 0; i < 10; i++) {
        if (lseek(fd, 0, 0) != -1) {
            goto seeked;
        }
    }
    close(fd);
    gCardError = 2;
    ret = -1;
    goto end;
seeked:
    buf = gSaveBuf;
    for (tries = 0; tries < 10; tries++) {
        if (read(fd, buf, 0x18000) == 0x18000) {
            goto done;
        }
        for (i = 0; i < 10; i++) {
            if (lseek(fd, 0, 0) != -1) {
                goto reseeked;
            }
        }
        close(fd);
        gCardError = 2;
        ret = -1;
        goto end;
    reseeked:;
    }
    close(fd);
    gCardError = 3;
    ret = -1;
    goto end;
done:
    close(fd);
end:
    return ret;
}

/** @brief Rewrites the second save file in place, up to ten tries per step; 0, or -1 with the failed step recorded. */
s32 RewriteSave2(void) {
    s32 ret;
    s16 i;
    s32 fd;
    u8 *buf;

    ret = 0;
    for (i = 0; i < 10; i++) {
        fd = open(sHitCardFile, 2);
        if (fd != -1) {
            goto opened;
        }
    }
    ret = -1;
    gCardError = 5;
    goto end;
opened:
    for (i = 0; i < 10; i++) {
        if (lseek(fd, 0, 0) != -1) {
            goto seeked;
        }
    }
    close(fd);
    ret = -1;
    gCardError = 6;
    goto end;
seeked:
    buf = gSaveBuf;
    for (i = 0; i < 10; i++) {
        if (write(fd, buf, 0x18000) == 0x18000) {
            goto written;
        }
        for (i = 0; i < 10; i++) {
            if (lseek(fd, 0, 0) != -1) {
                goto reseeked;
            }
        }
        close(fd);
        ret = -1;
        gCardError = 6;
        goto end;
    reseeked:;
    }
    close(fd);
    ret = -1;
    gCardError = 7;
    goto end;
written:
    close(fd);
end:
    return ret;
}

/** @brief Creates and writes the second save file, up to ten tries per step; 0, or -1 with the failed step recorded. */
s32 CreateSave2(void) {
    s32 ret;
    s16 i;
    s32 fd;

    ret = 0;
    for (i = 0; i < 10; i++) {
        fd = open(sHitCardFile, 0xC0200);
        if (fd != -1) {
            goto created;
        }
    }
    /* MATCHING: ret before the code keeps the three fail tails apart */
    ret = -1;
    gCardError = 8;
    goto end;
created:
    close(fd);
    for (i = 0; i < 10; i++) {
        fd = open(sHitCardFile, 2);
        if (fd != -1) {
            goto opened;
        }
    }
    ret = -1;
    gCardError = 9;
    goto end;
opened:
    for (i = 0; i < 10; i++) {
        if (write(fd, gSaveBuf, 0x18000) == 0x18000) {
            goto written;
        }
    }
    close(fd);
    ret = -1;
    gCardError = 10;
    goto end;
written:
    close(fd);
end:
    return ret;
}

/** @brief Copies the second save file's three icon TIMs into the card header's CLUT and frames. */
void CopySave2Icon(void) {
    IconTim tim[3];

    tim[0] = *(IconTim *)sHitCardIcon0;
    tim[1] = *(IconTim *)sHitCardIcon1;
    tim[2] = *(IconTim *)sHitCardIcon2;
    *(IconClut *)sHeaderIcon0 = tim[0].clut;
    *(IconBits *)(sHeaderIcon0 + 0x20) = tim[0].bits;
    *(IconBits *)sHeaderIcon1 = tim[1].bits;
    *(IconBits *)sHeaderIcon2 = tim[2].bits;
}

void SetSave2Buffers(void) {
    D_800959D4 = (u8 *)0x80195000;
    gSaveBuf = (u8 *)0x8018D000;
}

s32 EraseSave2(void) {
    s32 ret;
    s16 i;

    ret = 0;
    gCardFileName = sHitCardFile;
    for (i = 0; i < 10; i++) {
        if (erase(gCardFileName) != 0) {
            return ret;
        }
    }
    gCardError = 12;
    return -1;
}

INCLUDE_RODATA("asm/nonmatchings/card", sToolCardTitle);

INCLUDE_RODATA("asm/nonmatchings/card", sToolCardIcon0);

INCLUDE_RODATA("asm/nonmatchings/card", sToolCardIcon1);

INCLUDE_RODATA("asm/nonmatchings/card", sToolCardIcon2);

INCLUDE_RODATA("asm/nonmatchings/card", sHitCardFile);

s32 FindSave2(void) {
    s16 i;

    gCardFileName = "BISLPS-67890PEPTOOL";
    for (i = 0; i < gCardFileCount; i++) {
        if (CompareBytes((u8 *)gCardFiles[i].name, (u8 *)gCardFileName, 0x14) == 0) {
            return 0;
        }
    }
    return -1;
}

s32 CheckSave2Space(void) {
    s16 sum;
    s16 i;

    sum = 0;
    for (i = 0; i < gCardFileCount; i++) {
        sum += gCardFiles[i].blocks.lo;
    }
    if (sum >= 4) {
        return -1;
    }
    return 0;
}

void BuildSave2Header(void) {
    gCardHeader.magic[0] = 'S';
    gCardHeader.magic[1] = 'C';
    gCardHeader.iconFlag = 0x13;
    gCardHeader.blocks = 12;
    gCardHeader.title = *(CardTitle *)sHitCardTitle;
    CopySave2Icon();
}

INCLUDE_RODATA("asm/nonmatchings/card", sHitCardTitle);

INCLUDE_RODATA("asm/nonmatchings/card", sHitCardIcon0);

INCLUDE_RODATA("asm/nonmatchings/card", sHitCardIcon1);

INCLUDE_RODATA("asm/nonmatchings/card", sHitCardIcon2);

INCLUDE_RODATA("asm/nonmatchings/card", sButtonMaps);
