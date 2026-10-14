#include "common.h"
#include "libapi.h"
#include "string.h"

extern u8 *gSaveBuf;
extern u8 *sSaveData; /**< the save image being built or checked */
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

/** @brief A memory card event, as WaitCardSwEvent and WaitCardHwEvent
 *         return it: which of the four events of a kind fired. */
enum CardEvent {
    CARD_EV_IOE = 0,     /**< the operation completed */
    CARD_EV_ERROR = 1,   /**< the card reported an error */
    CARD_EV_TIMEOUT = 2, /**< no card answered */
    CARD_EV_NEW = 3      /**< a new card: unformatted, or swapped */
};

/** @brief The file step that failed, gCardError. */
enum CardError {
    CARD_ERR_NONE = 0,
    CARD_ERR_READ_OPEN = 1,     /**< opening the file to read it */
    CARD_ERR_READ_SEEK = 2,     /**< seeking to its start */
    CARD_ERR_READ = 3,          /**< reading it */
    CARD_ERR_REWRITE_OPEN = 5,  /**< opening the file to rewrite it */
    CARD_ERR_REWRITE_SEEK = 6,  /**< seeking to its start */
    CARD_ERR_REWRITE = 7,       /**< writing it */
    CARD_ERR_CREATE = 8,        /**< creating the file */
    CARD_ERR_CREATE_OPEN = 9,   /**< opening the new file */
    CARD_ERR_CREATE_WRITE = 10, /**< writing it */
    CARD_ERR_FORMAT = 11,       /**< formatting the card */
    CARD_ERR_ERASE = 12         /**< erasing the file */
};

extern CardFile gCardFiles[];
extern u16 gCardStatus; /**< ProbeCard's CardEvent; 0xFFFF when the directory scan finds no file */
extern s32 gCardFileCount;         /**< number of records in use */
extern s16 gCardError;             /**< the step that failed, a CardError */
extern char *gCardFileName;        /**< name of the file last erased */
extern char sCardDevice[];         /**< "bu10:" */
extern const char sToolCardFile[]; /**< "bu10:BISLPS-12345PEPTOOL" */
extern const char sHitCardFile[];  /**< "bu10:BISLPS-67890PEPTOOL" */

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

/** @brief A TIM block header: the CLUT's or the image's. */
typedef struct {
    u32 len; /**< the block's bytes, this header included */
    u16 x;   /**< VRAM x */
    u16 y;   /**< VRAM y */
    u16 w;   /**< width in VRAM halfwords */
    u16 h;   /**< height */
} TimBlockHead;

/** @brief A memory card icon frame as its TIM file: 16x16 pixels of 4 bits
 *         with a 16-colour CLUT (the layout IconTim copies as bytes). */
typedef struct {
    u32 id;                /**< 0x10, a TIM */
    u32 flags;             /**< 8: 4-bit pixels, a CLUT follows */
    TimBlockHead clutHead; /**< the CLUT's block, at (0, 480) */
    u16 clut[16];          /**< 15-bit colours, bit 15 the STP bit */
    TimBlockHead imgHead;  /**< the pixels' block, 4 halfwords by 16 rows */
    u8 pixels[16][8];      /**< rows of 16 CLUT indices, low nibble first */
} CardIconTim;

/** @brief The start of a memory card file header. */
typedef struct {
    u8 magic[2];     /**< "SC" */
    u8 iconFlag;     /**< icon display flag */
    u8 blocks;       /**< blocks the file uses */
    CardTitle title; /**< Shift-JIS title */
} CardHeader;

extern CardHeader gCardHeader;
extern const CardTitle sToolCardTitle;   /**< title of the first save file */
extern const CardTitle sHitCardTitle;    /**< title of the second save file */
extern const CardIconTim sToolCardIcon0; /**< first save file's icon frames */
extern const CardIconTim sToolCardIcon1;
extern const CardIconTim sToolCardIcon2;
extern const CardIconTim sHitCardIcon0; /**< second save file's icon frames */
extern const CardIconTim sHitCardIcon1;
extern const CardIconTim sHitCardIcon2;
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
        gObjectModels[i].pointFirst = -1;
        gObjectModels[i].pointCount = 0;
    }
    ApplyHitPoints();
    ApplyHitQuads();
}

/** @brief Runs one memory card operation, a CardOp, on the first save file; returns a CardResult. */
s32 RunCardOp1(s16 mode) {
    s32 unused[2]; /* MATCHING: retail's frame holds 8 unused bytes */
    s32 ret;
    s16 retry;
    u16 sum;
    s32 off; /* MATCHING: a constant offset is split into %hi + low; a register one is not */

    ret = CARD_OK;
    retry = 0;
    gCardStatus = CARD_EV_IOE;
    gCardError = CARD_ERR_NONE;
    sSaveData = (u8 *)0x8018D000;
    gSaveBuf = (u8 *)0x8016D000;
    EnableCardEvents();
again: /* MATCHING: a goto loop; a C loop hoists (s16)mode out of it */
    gCardStatus = ProbeCard();
    switch (gCardStatus) {
        case CARD_EV_IOE: /* MATCHING: the empty case gives retail's compare tree */
            break;
        case CARD_EV_ERROR:
            if (retry < 16) {
                retry++;
                goto again;
            }
            ret = CARD_PROBE_FAILED;
            goto end;
        case CARD_EV_TIMEOUT:
            gCardFileCount = 0;
            ret = CARD_ABSENT;
            goto end;
        case CARD_EV_NEW:
            gCardFileCount = 0;
            ret = CARD_UNFORMATTED;
            if (mode == CARD_OP_FORMAT) {
                goto format;
            }
            goto end;
    }
    if (mode == CARD_OP_PROBE) {
        goto end;
    }
    ScanCardFiles();
    if (mode == CARD_OP_CHECK) {
        if (FindSave1() != -1) {
            ret = CARD_OK;
        } else {
            ret = CheckSave1Space() == -1;
        }
    } else if (mode == CARD_OP_READ || mode == CARD_OP_REWRITE || mode == CARD_OP_ERASE) {
        if (FindSave1() == -1) {
            ret = CARD_NO_FILE;
            goto end;
        }
        switch (mode) {
            case CARD_OP_READ:
                ret = ReadSave1();
                break;
            case CARD_OP_REWRITE:
                ret = RewriteSave1();
                break;
            case CARD_OP_ERASE:
                ret = EraseSave1();
                break;
        }
    } else if (mode == CARD_OP_CREATE) {
        if (CheckSave1Space() == -1) {
            ret = CARD_FULL;
        } else {
            BuildSave1Header();
            *(SaveImage *)gSaveBuf = *(SaveImage *)&gCardHeader;
            sum = SumSaveBuffer();
            gSaveSum = sum;
            off = 0x1DFFE;
            *(u16 *)(sSaveData + off) = sum;
            ret = CreateSave1();
        }
    } else if (mode == CARD_OP_FORMAT) {
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
            case CARD_EV_ERROR: /* MATCHING: the empty case gives retail's compare tree */
                break;
            case CARD_EV_TIMEOUT:
                retry++;
                if (retry >= 11) {
                    return CARD_EV_TIMEOUT;
                }
                break;
            case CARD_EV_NEW:
                ClearCardHwEvents();
                _card_clear(0x10);
                WaitCardHwEvent();
                gCardNew = 1;
                break;
        }
        ClearCardSwEvents();
        _card_load(0x10);
        r = WaitCardSwEvent();
    } while (r == CARD_EV_ERROR || r == CARD_EV_TIMEOUT);
    if (r == CARD_EV_IOE) {
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

    ret = CARD_OK;
    for (i = 0; i < 10; i++) {
        fd = open((char *)sToolCardFile, 1);
        if (fd != -1) {
            goto opened;
        }
    }
    gCardError = CARD_ERR_READ_OPEN;
    ret = CARD_FAILED;
    goto end;
opened:
    for (i = 0; i < 10; i++) {
        if (lseek(fd, 0, 0) != -1) {
            goto seeked;
        }
    }
    close(fd);
    gCardError = CARD_ERR_READ_SEEK;
    ret = CARD_FAILED;
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
        gCardError = CARD_ERR_READ_SEEK;
        ret = CARD_FAILED;
        goto end;
    reseeked:;
    }
    close(fd);
    gCardError = CARD_ERR_READ;
    ret = CARD_FAILED;
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

    ret = CARD_OK;
    for (i = 0; i < 10; i++) {
        fd = open((char *)sToolCardFile, 2);
        if (fd != -1) {
            goto opened;
        }
    }
    ret = CARD_FAILED;
    gCardError = CARD_ERR_REWRITE_OPEN;
    goto end;
opened:
    for (i = 0; i < 10; i++) {
        if (lseek(fd, 0, 0) != -1) {
            goto seeked;
        }
    }
    close(fd);
    ret = CARD_FAILED;
    gCardError = CARD_ERR_REWRITE_SEEK;
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
        ret = CARD_FAILED;
        gCardError = CARD_ERR_REWRITE_SEEK;
        goto end;
    reseeked:;
    }
    close(fd);
    ret = CARD_FAILED;
    gCardError = CARD_ERR_REWRITE;
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

    ret = CARD_OK;
    for (i = 0; i < 10; i++) {
        fd = open((char *)sToolCardFile, 0xF0200);
        if (fd != -1) {
            goto created;
        }
    }
    /* MATCHING: ret before the code keeps the three fail tails apart */
    ret = CARD_FAILED;
    gCardError = CARD_ERR_CREATE;
    goto end;
created:
    close(fd);
    for (i = 0; i < 10; i++) {
        fd = open((char *)sToolCardFile, 2);
        if (fd != -1) {
            goto opened;
        }
    }
    ret = CARD_FAILED;
    gCardError = CARD_ERR_CREATE_OPEN;
    goto end;
opened:
    for (i = 0; i < 10; i++) {
        if (write(fd, gSaveBuf, 0x1E000) == 0x1E000) {
            goto written;
        }
    }
    close(fd);
    ret = CARD_FAILED;
    gCardError = CARD_ERR_CREATE_WRITE;
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
        gCardStatus = CARD_EV_IOE;
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

    tim[0] = *(IconTim *)&sToolCardIcon0;
    tim[1] = *(IconTim *)&sToolCardIcon1;
    tim[2] = *(IconTim *)&sToolCardIcon2;
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
    sSaveData = (u8 *)0x8018D000;
    gSaveBuf = (u8 *)0x8016D000;
}

s16 CheckCard(void) {
    s16 result;

    result = 0;
    EnableCardEvents();
    ClearCardSwEvents();
    _card_info(0x10);
    if (WaitCardSwEvent() == CARD_EV_TIMEOUT) {
        result = -1;
    }
    DisableCardEvents();
    return result;
}

s32 FormatCard(void) {
    s32 ret;
    s16 i;

    ret = CARD_OK;
    for (i = 0; i < 10; i++) {
        if (format(sCardDevice) != 0) {
            return ret;
        }
    }
    gCardError = CARD_ERR_FORMAT;
    return -1;
}

s32 EraseSave1(void) {
    s32 ret;
    s16 i;

    ret = CARD_OK;
    gCardFileName = (char *)sToolCardFile;
    for (i = 0; i < 10; i++) {
        if (erase(gCardFileName) != 0) {
            return ret;
        }
    }
    gCardError = CARD_ERR_ERASE;
    return -1;
}

const char sToolCardFile[] = "bu10:BISLPS-12345PEPTOOL";

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
    gCardHeader.title = sToolCardTitle;
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
            return CARD_EV_IOE;
        }
        if (TestEvent(gCardSwError) == 1) {
            return CARD_EV_ERROR;
        }
        if (TestEvent(gCardSwTimeout) == 1) {
            return CARD_EV_TIMEOUT;
        }
        if (TestEvent(gCardSwNew) == 1) {
            return CARD_EV_NEW;
        }
    }
}

s16 WaitCardHwEvent(void) {
    while (1) {
        if (TestEvent(gCardHwIoe) == 1) {
            return CARD_EV_IOE;
        }
        if (TestEvent(gCardHwError) == 1) {
            return CARD_EV_ERROR;
        }
        if (TestEvent(gCardHwTimeout) == 1) {
            return CARD_EV_TIMEOUT;
        }
        if (TestEvent(gCardHwNew) == 1) {
            return CARD_EV_NEW;
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

/** @brief Runs one memory card operation, a CardOp, on the second save file; returns a CardResult. */
s32 RunCardOp2(s16 mode) {
    s32 unused[2]; /* MATCHING: retail's frame holds 8 unused bytes */
    s32 ret;
    s16 retry;
    u16 sum;
    s32 off; /* MATCHING: a constant offset is split into %hi + low; a register one is not */

    ret = CARD_OK;
    retry = 0;
    gCardStatus = CARD_EV_IOE;
    gCardError = CARD_ERR_NONE;
    sSaveData = (u8 *)0x80195000;
    gSaveBuf = (u8 *)0x8018D000;
    EnableCardEvents();
again: /* MATCHING: a goto loop; a C loop hoists (s16)mode out of it */
    gCardStatus = ProbeCard();
    switch (gCardStatus) {
        case CARD_EV_IOE: /* MATCHING: the empty case gives retail's compare tree */
            break;
        case CARD_EV_ERROR:
            if (retry < 16) {
                retry++;
                goto again;
            }
            ret = CARD_PROBE_FAILED;
            goto end;
        case CARD_EV_TIMEOUT:
            gCardFileCount = 0;
            ret = CARD_ABSENT;
            goto end;
        case CARD_EV_NEW:
            gCardFileCount = 0;
            ret = CARD_UNFORMATTED;
            if (mode == CARD_OP_FORMAT) {
                goto format;
            }
            goto end;
    }
    if (mode == CARD_OP_PROBE) {
        goto end;
    }
    ScanCardFiles();
    if (mode == CARD_OP_CHECK) {
        if (FindSave2() != -1) {
            ret = CARD_OK;
        } else {
            ret = CheckSave2Space() == -1;
        }
    } else if (mode == CARD_OP_READ || mode == CARD_OP_REWRITE || mode == CARD_OP_ERASE) {
        if (FindSave2() == -1) {
            ret = CARD_NO_FILE;
            goto end;
        }
        switch (mode) {
            case CARD_OP_READ:
                ret = ReadSave2();
                break;
            case CARD_OP_REWRITE:
                ret = RewriteSave2();
                break;
            case CARD_OP_ERASE:
                ret = EraseSave2();
                break;
        }
    } else if (mode == CARD_OP_CREATE) {
        if (CheckSave2Space() == -1) {
            ret = CARD_FULL;
        } else {
            BuildSave2Header();
            *(SaveImage *)gSaveBuf = *(SaveImage *)&gCardHeader;
            sum = SumSaveBuffer();
            gSaveSum = sum;
            off = 0x17FFE;
            *(u16 *)(sSaveData + off) = sum;
            ret = CreateSave2();
        }
    } else if (mode == CARD_OP_FORMAT) {
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

    ret = CARD_OK;
    for (i = 0; i < 10; i++) {
        fd = open((char *)sHitCardFile, 1);
        if (fd != -1) {
            goto opened;
        }
    }
    gCardError = CARD_ERR_READ_OPEN;
    ret = CARD_FAILED;
    goto end;
opened:
    for (i = 0; i < 10; i++) {
        if (lseek(fd, 0, 0) != -1) {
            goto seeked;
        }
    }
    close(fd);
    gCardError = CARD_ERR_READ_SEEK;
    ret = CARD_FAILED;
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
        gCardError = CARD_ERR_READ_SEEK;
        ret = CARD_FAILED;
        goto end;
    reseeked:;
    }
    close(fd);
    gCardError = CARD_ERR_READ;
    ret = CARD_FAILED;
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

    ret = CARD_OK;
    for (i = 0; i < 10; i++) {
        fd = open((char *)sHitCardFile, 2);
        if (fd != -1) {
            goto opened;
        }
    }
    ret = CARD_FAILED;
    gCardError = CARD_ERR_REWRITE_OPEN;
    goto end;
opened:
    for (i = 0; i < 10; i++) {
        if (lseek(fd, 0, 0) != -1) {
            goto seeked;
        }
    }
    close(fd);
    ret = CARD_FAILED;
    gCardError = CARD_ERR_REWRITE_SEEK;
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
        ret = CARD_FAILED;
        gCardError = CARD_ERR_REWRITE_SEEK;
        goto end;
    reseeked:;
    }
    close(fd);
    ret = CARD_FAILED;
    gCardError = CARD_ERR_REWRITE;
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

    ret = CARD_OK;
    for (i = 0; i < 10; i++) {
        fd = open((char *)sHitCardFile, 0xC0200);
        if (fd != -1) {
            goto created;
        }
    }
    /* MATCHING: ret before the code keeps the three fail tails apart */
    ret = CARD_FAILED;
    gCardError = CARD_ERR_CREATE;
    goto end;
created:
    close(fd);
    for (i = 0; i < 10; i++) {
        fd = open((char *)sHitCardFile, 2);
        if (fd != -1) {
            goto opened;
        }
    }
    ret = CARD_FAILED;
    gCardError = CARD_ERR_CREATE_OPEN;
    goto end;
opened:
    for (i = 0; i < 10; i++) {
        if (write(fd, gSaveBuf, 0x18000) == 0x18000) {
            goto written;
        }
    }
    close(fd);
    ret = CARD_FAILED;
    gCardError = CARD_ERR_CREATE_WRITE;
    goto end;
written:
    close(fd);
end:
    return ret;
}

/** @brief Copies the second save file's three icon TIMs into the card header's CLUT and frames. */
void CopySave2Icon(void) {
    IconTim tim[3];

    tim[0] = *(IconTim *)&sHitCardIcon0;
    tim[1] = *(IconTim *)&sHitCardIcon1;
    tim[2] = *(IconTim *)&sHitCardIcon2;
    *(IconClut *)sHeaderIcon0 = tim[0].clut;
    *(IconBits *)(sHeaderIcon0 + 0x20) = tim[0].bits;
    *(IconBits *)sHeaderIcon1 = tim[1].bits;
    *(IconBits *)sHeaderIcon2 = tim[2].bits;
}

void SetSave2Buffers(void) {
    sSaveData = (u8 *)0x80195000;
    gSaveBuf = (u8 *)0x8018D000;
}

s32 EraseSave2(void) {
    s32 ret;
    s16 i;

    ret = CARD_OK;
    gCardFileName = (char *)sHitCardFile;
    for (i = 0; i < 10; i++) {
        if (erase(gCardFileName) != 0) {
            return ret;
        }
    }
    gCardError = CARD_ERR_ERASE;
    return -1;
}

/** The save file's title, Shift-JIS for "ペプシマン開発ツールデータ". */
const CardTitle sToolCardTitle = {"\x83\x79\x83\x76\x83\x56\x83\x7D\x83\x93\x8A\x4A\x94\xAD"
                                  "\x83\x63\x81\x5B\x83\x8B\x83\x66\x81\x5B\x83\x5E"};

/** The first save file's icon, frame 0: a 16x16 4-bit TIM. Each pixel row's
 * comment spells its CLUT indices left to right. */
const CardIconTim sToolCardIcon0 = {
    0x10, /* a TIM */
    8,    /* 4-bit pixels, with a CLUT */
    {0x2C, 0, 480, 16, 1},
    {0x7FFF, 0x0842, 0x1084, 0x18C6, 0x2108, 0x318C, 0x39CE, 0x4210, 0x4E73, 0x56B5, 0x5EF7, 0x6739,
     0x6F7B, 0x77BD, 0x8000, 0x10C7},
    {0x8C, 0, 0, 4, 16},
    {
        {0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE}, /* eeeeeeeeeeeeeeee */
        {0x41, 0x44, 0x44, 0xE3, 0x44, 0x44, 0x44, 0xE1}, /* 1444443e4444441e */
        {0x04, 0x00, 0x00, 0xEA, 0x00, 0x00, 0x00, 0xE4}, /* 400000ae0000004e */
        {0x04, 0x38, 0x04, 0xEA, 0xB0, 0x43, 0x08, 0xE3}, /* 408340ae0b34803e */
        {0x04, 0x00, 0x00, 0xEA, 0x00, 0x00, 0x00, 0xE4}, /* 400000ae0000004e */
        {0x03, 0x48, 0x04, 0xEA, 0xB0, 0x44, 0x08, 0xE4}, /* 308440ae0b44804e */
        {0x04, 0x00, 0x00, 0xEA, 0x00, 0x00, 0x00, 0xE4}, /* 400000ae0000004e */
        {0x04, 0x37, 0x44, 0x44, 0x44, 0x14, 0x07, 0xE3}, /* 407344444441703e */
        {0x04, 0xA6, 0x00, 0x00, 0x00, 0x70, 0x07, 0xE4}, /* 406a00000007704e */
        {0x03, 0xE7, 0xAE, 0xE0, 0x0A, 0xEE, 0x06, 0xE4}, /* 307eea0ea0ee604e */
        {0x04, 0xA9, 0xDA, 0xA0, 0x0D, 0xAA, 0x07, 0xE4}, /* 409aad0ad0aa704e */
        {0x04, 0x78, 0xC7, 0x70, 0x0C, 0x77, 0x07, 0xE3}, /* 40877c07c077703e */
        {0x04, 0xE7, 0xDE, 0xED, 0x0A, 0xEE, 0x06, 0xE4}, /* 407eeddea0ee604e */
        {0x03, 0x27, 0x0B, 0xE7, 0x0A, 0xEE, 0x07, 0xE4}, /* 3072b07ea0ee704e */
        {0x04, 0x97, 0x80, 0xEE, 0x0A, 0xDE, 0x00, 0xE4}, /* 407908eea0ed004e */
        {0x72, 0x23, 0xE3, 0xEE, 0xEE, 0x5E, 0x67, 0xEE}, /* 27323eeeeee576ee */
    },
};

/** Frame 1 of the first save file's icon. */
const CardIconTim sToolCardIcon1 = {
    0x10, /* a TIM */
    8,    /* 4-bit pixels, with a CLUT */
    {0x2C, 0, 480, 16, 1},
    {0x7FFF, 0x0842, 0x1084, 0x18C6, 0x2108, 0x294A, 0x318C, 0x39CE, 0x4631, 0x4E73, 0x56B5, 0x5EF7,
     0x6739, 0x6F7B, 0x77BD, 0x8000},
    {0x8C, 0, 0, 4, 16},
    {
        {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}, /* ffffffffffffffff */
        {0xFF, 0x41, 0x44, 0x75, 0xF1, 0xFF, 0xFF, 0xFF}, /* ff1444571fffffff */
        {0xFF, 0x04, 0x00, 0x00, 0x18, 0xEC, 0xF2, 0xFF}, /* ff40000081ce2fff */
        {0xFF, 0xAB, 0x94, 0x00, 0xD0, 0xA0, 0x6B, 0xFF}, /* ffba49000d0ab6ff */
        {0x3F, 0x0E, 0x0B, 0x5E, 0x00, 0xCA, 0x90, 0xFF}, /* f3e0b0e500ac09ff */
        {0xFF, 0xE2, 0xE0, 0xF3, 0xE6, 0x00, 0xF7, 0xFF}, /* ff2e0e3f6e007fff */
        {0x6F, 0x0D, 0xD0, 0xBB, 0xDB, 0x00, 0x8E, 0xF3}, /* f6d00dbbbd00e83f */
        {0x0D, 0xA0, 0x0B, 0xBE, 0xE0, 0x8B, 0x0E, 0xF8}, /* d00ab0eb0eb8e08f */
        {0x87, 0xF2, 0x0F, 0xFB, 0xB0, 0xFF, 0x4F, 0xFF}, /* 782ff0bf0bfff4ff */
        {0x8F, 0xBB, 0x0B, 0xBE, 0xE0, 0xBB, 0x6B, 0xFF}, /* f8bbb0eb0ebbb6ff */
        {0x8F, 0xBB, 0x0C, 0xBD, 0xE0, 0xBB, 0x6B, 0xFF}, /* f8bbc0db0ebbb6ff */
        {0xFF, 0xFF, 0x06, 0xF8, 0xB0, 0xFF, 0x22, 0xFF}, /* ffff608f0bff22ff */
        {0xFF, 0x4F, 0x0E, 0xF2, 0xB0, 0xFF, 0x08, 0xF4}, /* fff4e02f0bff804f */
        {0x62, 0x0A, 0x80, 0xFF, 0xE0, 0x78, 0x0E, 0xF3}, /* 26a008ff0e87e03f */
        {0x07, 0xC0, 0xF5, 0xFF, 0x0C, 0x00, 0x90, 0xFF}, /* 700c5fffc00009ff */
        {0x4F, 0xF2, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}, /* f42fffffffffffff */
    },
};

/** Frame 2 of the first save file's icon. */
const CardIconTim sToolCardIcon2 = {
    0x10, /* a TIM */
    8,    /* 4-bit pixels, with a CLUT */
    {0x2C, 0, 480, 16, 1},
    {0x7FFF, 0x0842, 0x18C6, 0x2108, 0x294A, 0x318C, 0x39CE, 0x4210, 0x4631, 0x4E73, 0x5EF7, 0x6739,
     0x6F7B, 0x8000, 0x6B7B, 0x6F7B},
    {0x8C, 0, 0, 4, 16},
    {
        {0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD}, /* dddddddddddddddd */
        {0x1D, 0x33, 0x33, 0x33, 0x33, 0x33, 0x23, 0xDD}, /* d1333333333332dd */
        {0x3D, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA0, 0xDD}, /* d300000000000add */
        {0x3D, 0x90, 0x23, 0x06, 0x39, 0x32, 0xA0, 0xDD}, /* d309326093230add */
        {0x3D, 0x70, 0xDD, 0x03, 0xD7, 0xDD, 0xA0, 0xDD}, /* d307dd307ddd0add */
        {0x2D, 0xC0, 0xAA, 0x0B, 0xAC, 0xAA, 0xA0, 0xDD}, /* d20caab0caaa0add */
        {0x3D, 0xC0, 0xAA, 0x0B, 0xAC, 0xAA, 0xA0, 0xDD}, /* d30caab0caaa0add */
        {0x3D, 0x70, 0xDD, 0x03, 0xD7, 0xDD, 0xA0, 0xDD}, /* d307dd307ddd0add */
        {0x3D, 0x90, 0x33, 0x06, 0x39, 0x33, 0xA0, 0xDD}, /* d309336093330add */
        {0x2D, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA0, 0xDD}, /* d200000000000add */
        {0x3D, 0x90, 0x33, 0x06, 0x39, 0x33, 0xA0, 0xDD}, /* d309336093330add */
        {0x6D, 0x70, 0xDD, 0x02, 0xD7, 0xDD, 0xA0, 0xDD}, /* d607dd207ddd0add */
        {0xAD, 0x20, 0xDD, 0x03, 0xD6, 0xDD, 0xA0, 0xDD}, /* da02dd306ddd0add */
        {0x05, 0xDC, 0xDD, 0x03, 0xD7, 0xA8, 0xA0, 0xDD}, /* 50cddd307d8a0add */
        {0x08, 0xD4, 0xDD, 0xA2, 0xD5, 0x08, 0x80, 0xDD}, /* 804ddd2a5d8008dd */
        {0x2D, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD}, /* d2dddddddddddddd */
    },
};

const char sHitCardFile[] = "bu10:BISLPS-67890PEPTOOL";

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
    gCardHeader.title = sHitCardTitle;
    CopySave2Icon();
}

/** The save file's title, Shift-JIS for "ペプシマン開発ヒットデータ". */
const CardTitle sHitCardTitle = {"\x83\x79\x83\x76\x83\x56\x83\x7D\x83\x93\x8A\x4A\x94\xAD"
                                 "\x83\x71\x83\x62\x83\x67\x83\x66\x81\x5B\x83\x5E"};

/** The second save file's icon, frame 0 (the same images as the first's). */
const CardIconTim sHitCardIcon0 = {
    0x10, /* a TIM */
    8,    /* 4-bit pixels, with a CLUT */
    {0x2C, 0, 480, 16, 1},
    {0x7FFF, 0x0842, 0x1084, 0x18C6, 0x2108, 0x318C, 0x39CE, 0x4210, 0x4E73, 0x56B5, 0x5EF7, 0x6739,
     0x6F7B, 0x77BD, 0x8000, 0x10C7},
    {0x8C, 0, 0, 4, 16},
    {
        {0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE}, /* eeeeeeeeeeeeeeee */
        {0x41, 0x44, 0x44, 0xE3, 0x44, 0x44, 0x44, 0xE1}, /* 1444443e4444441e */
        {0x04, 0x00, 0x00, 0xEA, 0x00, 0x00, 0x00, 0xE4}, /* 400000ae0000004e */
        {0x04, 0x38, 0x04, 0xEA, 0xB0, 0x43, 0x08, 0xE3}, /* 408340ae0b34803e */
        {0x04, 0x00, 0x00, 0xEA, 0x00, 0x00, 0x00, 0xE4}, /* 400000ae0000004e */
        {0x03, 0x48, 0x04, 0xEA, 0xB0, 0x44, 0x08, 0xE4}, /* 308440ae0b44804e */
        {0x04, 0x00, 0x00, 0xEA, 0x00, 0x00, 0x00, 0xE4}, /* 400000ae0000004e */
        {0x04, 0x37, 0x44, 0x44, 0x44, 0x14, 0x07, 0xE3}, /* 407344444441703e */
        {0x04, 0xA6, 0x00, 0x00, 0x00, 0x70, 0x07, 0xE4}, /* 406a00000007704e */
        {0x03, 0xE7, 0xAE, 0xE0, 0x0A, 0xEE, 0x06, 0xE4}, /* 307eea0ea0ee604e */
        {0x04, 0xA9, 0xDA, 0xA0, 0x0D, 0xAA, 0x07, 0xE4}, /* 409aad0ad0aa704e */
        {0x04, 0x78, 0xC7, 0x70, 0x0C, 0x77, 0x07, 0xE3}, /* 40877c07c077703e */
        {0x04, 0xE7, 0xDE, 0xED, 0x0A, 0xEE, 0x06, 0xE4}, /* 407eeddea0ee604e */
        {0x03, 0x27, 0x0B, 0xE7, 0x0A, 0xEE, 0x07, 0xE4}, /* 3072b07ea0ee704e */
        {0x04, 0x97, 0x80, 0xEE, 0x0A, 0xDE, 0x00, 0xE4}, /* 407908eea0ed004e */
        {0x72, 0x23, 0xE3, 0xEE, 0xEE, 0x5E, 0x67, 0xEE}, /* 27323eeeeee576ee */
    },
};

/** Frame 1 of the second save file's icon. */
const CardIconTim sHitCardIcon1 = {
    0x10, /* a TIM */
    8,    /* 4-bit pixels, with a CLUT */
    {0x2C, 0, 480, 16, 1},
    {0x7FFF, 0x0842, 0x1084, 0x18C6, 0x2108, 0x294A, 0x318C, 0x39CE, 0x4631, 0x4E73, 0x56B5, 0x5EF7,
     0x6739, 0x6F7B, 0x77BD, 0x8000},
    {0x8C, 0, 0, 4, 16},
    {
        {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}, /* ffffffffffffffff */
        {0xFF, 0x41, 0x44, 0x75, 0xF1, 0xFF, 0xFF, 0xFF}, /* ff1444571fffffff */
        {0xFF, 0x04, 0x00, 0x00, 0x18, 0xEC, 0xF2, 0xFF}, /* ff40000081ce2fff */
        {0xFF, 0xAB, 0x94, 0x00, 0xD0, 0xA0, 0x6B, 0xFF}, /* ffba49000d0ab6ff */
        {0x3F, 0x0E, 0x0B, 0x5E, 0x00, 0xCA, 0x90, 0xFF}, /* f3e0b0e500ac09ff */
        {0xFF, 0xE2, 0xE0, 0xF3, 0xE6, 0x00, 0xF7, 0xFF}, /* ff2e0e3f6e007fff */
        {0x6F, 0x0D, 0xD0, 0xBB, 0xDB, 0x00, 0x8E, 0xF3}, /* f6d00dbbbd00e83f */
        {0x0D, 0xA0, 0x0B, 0xBE, 0xE0, 0x8B, 0x0E, 0xF8}, /* d00ab0eb0eb8e08f */
        {0x87, 0xF2, 0x0F, 0xFB, 0xB0, 0xFF, 0x4F, 0xFF}, /* 782ff0bf0bfff4ff */
        {0x8F, 0xBB, 0x0B, 0xBE, 0xE0, 0xBB, 0x6B, 0xFF}, /* f8bbb0eb0ebbb6ff */
        {0x8F, 0xBB, 0x0C, 0xBD, 0xE0, 0xBB, 0x6B, 0xFF}, /* f8bbc0db0ebbb6ff */
        {0xFF, 0xFF, 0x06, 0xF8, 0xB0, 0xFF, 0x22, 0xFF}, /* ffff608f0bff22ff */
        {0xFF, 0x4F, 0x0E, 0xF2, 0xB0, 0xFF, 0x08, 0xF4}, /* fff4e02f0bff804f */
        {0x62, 0x0A, 0x80, 0xFF, 0xE0, 0x78, 0x0E, 0xF3}, /* 26a008ff0e87e03f */
        {0x07, 0xC0, 0xF5, 0xFF, 0x0C, 0x00, 0x90, 0xFF}, /* 700c5fffc00009ff */
        {0x4F, 0xF2, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}, /* f42fffffffffffff */
    },
};

/** Frame 2 of the second save file's icon. */
const CardIconTim sHitCardIcon2 = {
    0x10, /* a TIM */
    8,    /* 4-bit pixels, with a CLUT */
    {0x2C, 0, 480, 16, 1},
    {0x7FFF, 0x0842, 0x18C6, 0x2108, 0x294A, 0x318C, 0x39CE, 0x4210, 0x4631, 0x4E73, 0x5EF7, 0x6739,
     0x6F7B, 0x8000, 0x6B7B, 0x6F7B},
    {0x8C, 0, 0, 4, 16},
    {
        {0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD}, /* dddddddddddddddd */
        {0x1D, 0x33, 0x33, 0x33, 0x33, 0x33, 0x23, 0xDD}, /* d1333333333332dd */
        {0x3D, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA0, 0xDD}, /* d300000000000add */
        {0x3D, 0x90, 0x23, 0x06, 0x39, 0x32, 0xA0, 0xDD}, /* d309326093230add */
        {0x3D, 0x70, 0xDD, 0x03, 0xD7, 0xDD, 0xA0, 0xDD}, /* d307dd307ddd0add */
        {0x2D, 0xC0, 0xAA, 0x0B, 0xAC, 0xAA, 0xA0, 0xDD}, /* d20caab0caaa0add */
        {0x3D, 0xC0, 0xAA, 0x0B, 0xAC, 0xAA, 0xA0, 0xDD}, /* d30caab0caaa0add */
        {0x3D, 0x70, 0xDD, 0x03, 0xD7, 0xDD, 0xA0, 0xDD}, /* d307dd307ddd0add */
        {0x3D, 0x90, 0x33, 0x06, 0x39, 0x33, 0xA0, 0xDD}, /* d309336093330add */
        {0x2D, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA0, 0xDD}, /* d200000000000add */
        {0x3D, 0x90, 0x33, 0x06, 0x39, 0x33, 0xA0, 0xDD}, /* d309336093330add */
        {0x6D, 0x70, 0xDD, 0x02, 0xD7, 0xDD, 0xA0, 0xDD}, /* d607dd207ddd0add */
        {0xAD, 0x20, 0xDD, 0x03, 0xD6, 0xDD, 0xA0, 0xDD}, /* da02dd306ddd0add */
        {0x05, 0xDC, 0xDD, 0x03, 0xD7, 0xA8, 0xA0, 0xDD}, /* 50cddd307d8a0add */
        {0x08, 0xD4, 0xDD, 0xA2, 0xD5, 0x08, 0x80, 0xDD}, /* 804ddd2a5d8008dd */
        {0x2D, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD}, /* d2dddddddddddddd */
    },
};

INCLUDE_RODATA("asm/nonmatchings/card", sButtonMaps);
