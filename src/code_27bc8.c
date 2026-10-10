#include "common.h"
#include "libapi.h"
#include "string.h"

extern u8 *D_80095A18;
extern u8 *D_800959D4;
extern s32 D_800959E8;
extern s32 D_800959EC;
extern s32 D_800959F0;
extern s32 D_800959F4;
extern s32 D_800959FC;
extern s32 D_80095A00;
extern s32 D_80095A04;
extern s32 D_80095A08;

/** @brief A file's size in 8 KiB blocks, stored as a word, summed as a halfword. */
typedef union {
    s32 w;  /**< the block count as written */
    u16 lo; /**< its low half, which the sums read */
} BlockCount;

/** @brief One memory card file found by the directory scan. */
typedef struct {
    char name[20];     /**< file name, from the directory entry */
    BlockCount blocks; /**< size in 8 KiB blocks */
} Rec18;

extern Rec18 D_800DF858[];
extern u16 D_800959DA;
extern s32 D_800959DC;    /**< number of records in use */
extern s16 D_800959D0;    /**< 11 after a failed format, 12 after a failed erase */
extern char *D_80095A1C;  /**< name of the file last erased */
extern char D_800956A8[]; /**< "bu10:" */
extern char D_80011998[]; /**< "bu10:BISLPS-12345PEPTOOL" */
extern char D_80011C24[]; /**< "bu10:BISLPS-67890PEPTOOL" */

/** @brief A loaded record bank file: a count word, then the bank. */
typedef struct {
    s32 count;  /**< entry count, copied to the bank's count global */
    u8 data[4]; /**< the bank itself; real size unknown */
} BankFile;

/** @brief The 27 title bytes copied into a save header in one block move. */
typedef struct {
    u8 b[27]; /**< Shift-JIS title text */
} CardTitle;

/** @brief The start of a memory card file header. */
typedef struct {
    u8 magic[2];     /**< "SC" */
    u8 iconFlag;     /**< icon display flag */
    u8 blocks;       /**< blocks the file uses */
    CardTitle title; /**< Shift-JIS title */
} CardHeader;

extern CardHeader D_800DF5D0;
extern u8 D_800119C8[]; /**< title of the first save file */
extern u8 D_80011C54[]; /**< title of the second save file */

void func_80037CF0(void);
void func_80038730(void);
void func_800387A8(void);
s16 func_80038820(void);
s16 func_80038890(void);
void func_80038900(void);
void func_80038948(void);
void func_800390B8(void);

void func_800373C8(BankFile *a, BankFile *b) {
    u8 *bank; /* MATCHING: one local for both bank addresses */
    u32 i;

    bank = a->data;
    D_80095A50 = (s32)bank;
    bank = b->data;
    D_80095A4C = (s32)bank;
    D_80095780 = a->count;
    D_80095810 = b->count;
    for (i = 0; i < 80; i++) {
        D_800D8D20[i].unk72 = -1;
        D_800D8D20[i].unk74 = 0;
    }
    func_80036704();
    func_80036878();
}

INCLUDE_ASM("asm/nonmatchings/code_27bc8", func_80037440);

s16 func_80037700(void) {
    s16 retry;
    s16 r;

    retry = 0;
    do {
        func_80038900();
        _card_info(0x10);
        r = func_80038820();
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
                func_80038948();
                _card_clear(0x10);
                func_80038890();
                D_80095A14 = 1;
                break;
        }
        func_80038900();
        _card_load(0x10);
        r = func_80038820();
    } while (r == 1 || r == 2);
    if (r == 0) {
        return 0;
    }
    return r;
}

/** @brief Reads the first save file into the save buffer, up to ten tries per step; 0, or -1 with the failed step recorded. */
s32 func_800377E8(void) {
    s32 ret;
    s16 i;
    s32 fd;
    u8 *buf;
    s16 tries;

    ret = 0;
    for (i = 0; i < 10; i++) {
        fd = open(D_80011998, 1);
        if (fd != -1) {
            goto opened;
        }
    }
    D_800959D0 = 1;
    ret = -1;
    goto end;
opened:
    for (i = 0; i < 10; i++) {
        if (lseek(fd, 0, 0) != -1) {
            goto seeked;
        }
    }
    close(fd);
    D_800959D0 = 2;
    ret = -1;
    goto end;
seeked:
    buf = D_80095A18;
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
        D_800959D0 = 2;
        ret = -1;
        goto end;
    reseeked:;
    }
    close(fd);
    D_800959D0 = 3;
    ret = -1;
    goto end;
done:
    close(fd);
end:
    return ret;
}

/** @brief Rewrites the first save file in place, up to ten tries per step; 0, or -1 with the failed step recorded. */
s32 func_8003796C(void) {
    s32 ret;
    s16 i;
    s32 fd;
    u8 *buf;

    ret = 0;
    for (i = 0; i < 10; i++) {
        fd = open(D_80011998, 2);
        if (fd != -1) {
            goto opened;
        }
    }
    ret = -1;
    D_800959D0 = 5;
    goto end;
opened:
    for (i = 0; i < 10; i++) {
        if (lseek(fd, 0, 0) != -1) {
            goto seeked;
        }
    }
    close(fd);
    ret = -1;
    D_800959D0 = 6;
    goto end;
seeked:
    buf = D_80095A18;
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
        D_800959D0 = 6;
        goto end;
    reseeked:;
    }
    close(fd);
    ret = -1;
    D_800959D0 = 7;
    goto end;
written:
    close(fd);
end:
    return ret;
}

/** @brief Creates and writes the first save file, up to ten tries per step; 0, or -1 with the failed step recorded. */
s32 func_80037AE4(void) {
    s32 ret;
    s16 i;
    s32 fd;

    ret = 0;
    for (i = 0; i < 10; i++) {
        fd = open(D_80011998, 0xF0200);
        if (fd != -1) {
            goto created;
        }
    }
    /* MATCHING: ret before the code keeps the three fail tails apart */
    ret = -1;
    D_800959D0 = 8;
    goto end;
created:
    close(fd);
    for (i = 0; i < 10; i++) {
        fd = open(D_80011998, 2);
        if (fd != -1) {
            goto opened;
        }
    }
    ret = -1;
    D_800959D0 = 9;
    goto end;
opened:
    for (i = 0; i < 10; i++) {
        if (write(fd, D_80095A18, 0x1E000) == 0x1E000) {
            goto written;
        }
    }
    close(fd);
    ret = -1;
    D_800959D0 = 10;
    goto end;
written:
    close(fd);
end:
    return ret;
}

#ifdef NON_MATCHING
s32 func_80037C2C(void) {
    struct DIRENTRY de;
    Rec18 *rec;
    s32 i;

    i = 0;
    D_800959DC = 0;
    if (firstfile(D_800956A8, &de) == NULL) {
        D_800959DA = 0xFFFF;
        return 0xFFFF;
    }
    rec = D_800DF858;
loop:
    strcpy(rec->name, de.name);
    rec->blocks.w = de.size / 8192;
    D_800959DC++;
    i++;
    if (nextfile(&de) == NULL) {
        D_800959DA = 0;
        return 0;
    }
    rec = &D_800DF858[i];
    goto loop;
}
#else
INCLUDE_ASM("asm/nonmatchings/code_27bc8", func_80037C2C);
#endif

INCLUDE_ASM("asm/nonmatchings/code_27bc8", func_80037CF0);

/** @brief Opens the eight memory card events (four software, four hardware) and leaves them disabled. */
void func_80038124(void) {
    EnterCriticalSection();
    D_800959E8 = OpenEvent(0xF4000001, 4, 0x2000, NULL);
    D_800959EC = OpenEvent(0xF4000001, 0x8000, 0x2000, NULL);
    D_800959F0 = OpenEvent(0xF4000001, 0x100, 0x2000, NULL);
    D_800959F4 = OpenEvent(0xF4000001, 0x2000, 0x2000, NULL);
    D_800959FC = OpenEvent(0xF0000011, 4, 0x2000, NULL);
    D_80095A00 = OpenEvent(0xF0000011, 0x8000, 0x2000, NULL);
    D_80095A04 = OpenEvent(0xF0000011, 0x100, 0x2000, NULL);
    D_80095A08 = OpenEvent(0xF0000011, 0x2000, 0x2000, NULL);
    ExitCriticalSection();
    DisableEvent(D_800959E8);
    DisableEvent(D_800959EC);
    DisableEvent(D_800959F0);
    DisableEvent(D_800959F4);
    DisableEvent(D_800959FC);
    DisableEvent(D_80095A00);
    DisableEvent(D_80095A04);
    DisableEvent(D_80095A08);
}

/* MATCHING: s32, not s16: both callers test the result unextended. */
s32 func_8003828C(u8 *a, u8 *b, s16 n) {
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

u16 func_800382DC(void) {
    u8 *p;
    u16 sum;
    s32 i;
    u8 lo;
    u8 hi;

    p = D_80095A18;
    sum = 0;
    for (i = 0; i < 0xEFFF; i++) {
        lo = *p++;
        hi = *p++;
        sum += lo | (hi << 8);
    }
    return sum;
}

s16 func_8003831C(u16 expected) {
    u8 *p;
    u16 sum;
    s32 i;
    u8 lo;
    u8 hi;

    p = D_80095A18;
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

void func_80038374(void) {
    D_800959D4 = (u8 *)0x8018D000;
    D_80095A18 = (u8 *)0x8016D000;
}

s16 func_80038394(void) {
    s16 result;

    result = 0;
    func_80038730();
    func_80038900();
    _card_info(0x10);
    if (func_80038820() == 2) {
        result = -1;
    }
    func_800387A8();
    return result;
}

s32 func_800383F8(void) {
    s32 ret;
    s16 i;

    ret = 0;
    for (i = 0; i < 10; i++) {
        if (format(D_800956A8) != 0) {
            return ret;
        }
    }
    D_800959D0 = 11;
    return -1;
}

s32 func_80038468(void) {
    s32 ret;
    s16 i;

    ret = 0;
    D_80095A1C = D_80011998;
    for (i = 0; i < 10; i++) {
        if (erase(D_80095A1C) != 0) {
            return ret;
        }
    }
    D_800959D0 = 12;
    return -1;
}

INCLUDE_RODATA("asm/nonmatchings/code_27bc8", D_80011998);

s16 func_800384DC(void) {
    s16 i;

    D_80095A1C = "BISLPS-12345PEPTOOL";
    for (i = 0; i < D_800959DC; i++) {
        if (func_8003828C((u8 *)D_800DF858[i].name, (u8 *)D_80095A1C, 0x14) == 0) {
            return 0;
        }
    }
    return -1;
}

s16 func_80038574(void) {
    s16 sum;
    s16 i;

    sum = 0;
    for (i = 0; i < D_800959DC; i++) {
        sum += D_800DF858[i].blocks.lo;
    }
    if (sum > 0) {
        return -1;
    }
    return 0;
}

void func_800385E0(void) {
    D_800DF5D0.magic[0] = 'S';
    D_800DF5D0.magic[1] = 'C';
    D_800DF5D0.iconFlag = 0x13;
    D_800DF5D0.blocks = 15;
    D_800DF5D0.title = *(CardTitle *)D_800119C8;
    func_80037CF0();
}

void func_800386A8(void) {
    EnterCriticalSection();
    CloseEvent(D_800959E8);
    CloseEvent(D_800959EC);
    CloseEvent(D_800959F0);
    CloseEvent(D_800959F4);
    CloseEvent(D_800959FC);
    CloseEvent(D_80095A00);
    CloseEvent(D_80095A04);
    CloseEvent(D_80095A08);
    ExitCriticalSection();
}

void func_80038730(void) {
    EnableEvent(D_800959E8);
    EnableEvent(D_800959EC);
    EnableEvent(D_800959F0);
    EnableEvent(D_800959F4);
    EnableEvent(D_800959FC);
    EnableEvent(D_80095A00);
    EnableEvent(D_80095A04);
    EnableEvent(D_80095A08);
}

void func_800387A8(void) {
    DisableEvent(D_800959E8);
    DisableEvent(D_800959EC);
    DisableEvent(D_800959F0);
    DisableEvent(D_800959F4);
    DisableEvent(D_800959FC);
    DisableEvent(D_80095A00);
    DisableEvent(D_80095A04);
    DisableEvent(D_80095A08);
}

s16 func_80038820(void) {
    while (1) {
        if (TestEvent(D_800959E8) == 1) {
            return 0;
        }
        if (TestEvent(D_800959EC) == 1) {
            return 1;
        }
        if (TestEvent(D_800959F0) == 1) {
            return 2;
        }
        if (TestEvent(D_800959F4) == 1) {
            return 3;
        }
    }
}

s16 func_80038890(void) {
    while (1) {
        if (TestEvent(D_800959FC) == 1) {
            return 0;
        }
        if (TestEvent(D_80095A00) == 1) {
            return 1;
        }
        if (TestEvent(D_80095A04) == 1) {
            return 2;
        }
        if (TestEvent(D_80095A08) == 1) {
            return 3;
        }
    }
}

void func_80038900(void) {
    TestEvent(D_800959E8);
    TestEvent(D_800959EC);
    TestEvent(D_800959F0);
    TestEvent(D_800959F4);
}

void func_80038948(void) {
    TestEvent(D_800959FC);
    TestEvent(D_80095A00);
    TestEvent(D_80095A04);
    TestEvent(D_80095A08);
}

void func_80038990(u8 *p, s32 n) {
    s32 i;

    for (i = 0; i < n; i++) {
        *p++ = 0;
    }
}

INCLUDE_ASM("asm/nonmatchings/code_27bc8", func_800389B4);

INCLUDE_ASM("asm/nonmatchings/code_27bc8", func_80038C74);

/** @brief Rewrites the second save file in place, up to ten tries per step; 0, or -1 with the failed step recorded. */
s32 func_80038DF8(void) {
    s32 ret;
    s16 i;
    s32 fd;
    u8 *buf;

    ret = 0;
    for (i = 0; i < 10; i++) {
        fd = open(D_80011C24, 2);
        if (fd != -1) {
            goto opened;
        }
    }
    ret = -1;
    D_800959D0 = 5;
    goto end;
opened:
    for (i = 0; i < 10; i++) {
        if (lseek(fd, 0, 0) != -1) {
            goto seeked;
        }
    }
    close(fd);
    ret = -1;
    D_800959D0 = 6;
    goto end;
seeked:
    buf = D_80095A18;
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
        D_800959D0 = 6;
        goto end;
    reseeked:;
    }
    close(fd);
    ret = -1;
    D_800959D0 = 7;
    goto end;
written:
    close(fd);
end:
    return ret;
}

/** @brief Creates and writes the second save file, up to ten tries per step; 0, or -1 with the failed step recorded. */
s32 func_80038F70(void) {
    s32 ret;
    s16 i;
    s32 fd;

    ret = 0;
    for (i = 0; i < 10; i++) {
        fd = open(D_80011C24, 0xC0200);
        if (fd != -1) {
            goto created;
        }
    }
    /* MATCHING: ret before the code keeps the three fail tails apart */
    ret = -1;
    D_800959D0 = 8;
    goto end;
created:
    close(fd);
    for (i = 0; i < 10; i++) {
        fd = open(D_80011C24, 2);
        if (fd != -1) {
            goto opened;
        }
    }
    ret = -1;
    D_800959D0 = 9;
    goto end;
opened:
    for (i = 0; i < 10; i++) {
        if (write(fd, D_80095A18, 0x18000) == 0x18000) {
            goto written;
        }
    }
    close(fd);
    ret = -1;
    D_800959D0 = 10;
    goto end;
written:
    close(fd);
end:
    return ret;
}

INCLUDE_ASM("asm/nonmatchings/code_27bc8", func_800390B8);

void func_800394EC(void) {
    D_800959D4 = (u8 *)0x80195000;
    D_80095A18 = (u8 *)0x8018D000;
}

s32 func_8003950C(void) {
    s32 ret;
    s16 i;

    ret = 0;
    D_80095A1C = D_80011C24;
    for (i = 0; i < 10; i++) {
        if (erase(D_80095A1C) != 0) {
            return ret;
        }
    }
    D_800959D0 = 12;
    return -1;
}

INCLUDE_RODATA("asm/nonmatchings/code_27bc8", D_800119C8);

INCLUDE_RODATA("asm/nonmatchings/code_27bc8", D_800119E4);

INCLUDE_RODATA("asm/nonmatchings/code_27bc8", D_80011AA4);

INCLUDE_RODATA("asm/nonmatchings/code_27bc8", D_80011B64);

INCLUDE_RODATA("asm/nonmatchings/code_27bc8", D_80011C24);

s16 func_80039580(void) {
    s16 i;

    D_80095A1C = "BISLPS-67890PEPTOOL";
    for (i = 0; i < D_800959DC; i++) {
        if (func_8003828C((u8 *)D_800DF858[i].name, (u8 *)D_80095A1C, 0x14) == 0) {
            return 0;
        }
    }
    return -1;
}

s16 func_80039618(void) {
    s16 sum;
    s16 i;

    sum = 0;
    for (i = 0; i < D_800959DC; i++) {
        sum += D_800DF858[i].blocks.lo;
    }
    if (sum >= 4) {
        return -1;
    }
    return 0;
}

void func_8003968C(void) {
    D_800DF5D0.magic[0] = 'S';
    D_800DF5D0.magic[1] = 'C';
    D_800DF5D0.iconFlag = 0x13;
    D_800DF5D0.blocks = 12;
    D_800DF5D0.title = *(CardTitle *)D_80011C54;
    func_800390B8();
}

INCLUDE_RODATA("asm/nonmatchings/code_27bc8", D_80011C54);

INCLUDE_RODATA("asm/nonmatchings/code_27bc8", D_80011C70);

INCLUDE_RODATA("asm/nonmatchings/code_27bc8", D_80011D30);

INCLUDE_RODATA("asm/nonmatchings/code_27bc8", D_80011DF0);

INCLUDE_RODATA("asm/nonmatchings/code_27bc8", D_80011EB0);
