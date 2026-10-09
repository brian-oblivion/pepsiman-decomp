#include "common.h"
#include "memory.h"
#include "libapi.h"
#include "libgte.h"
#include "sys/file.h"
#include "code_1a098.h"

/** @brief A 0x3C-byte record of a 100-entry table; only the halfword at 0 is
 *         known. */
typedef struct {
    s16 unk0;      /**< -1 when the record is free (a guess) */
    u8 unk2[0x3A]; /**< not yet known */
} Rec3C;

/** @brief A state block with a halfword total at 0x26. */
typedef struct {
    u8 unk0[0x1E]; /**< not yet known */
    u16 unk1E;     /**< matched against a Rec48's unk34 */
    u8 unk20[6];   /**< not yet known */
    u16 unk26;     /**< a sum over the current block's entries */
} Totals28;

/** @brief 64 KiB of the tool buffer, copied whole. */
typedef struct {
    u8 b[0x10000]; /**< not yet known */
} Page64K;

/** @brief A 0xB774-byte slot of the tool buffer; a tag byte and an owner
 *         byte known. */
typedef struct {
    u8 unk0[0x200];    /**< not yet known */
    u8 unk200;         /**< 0x38 when the slot is valid */
    u8 unk201;         /**< not yet known */
    u8 unk202;         /**< the owner; compared with a global */
    u8 unk203[0xB571]; /**< not yet known */
} SaveSlot;

extern u16 D_80095B4C[];   /**< first of a run of halfwords */
extern Rec3C D_800A7898[]; /**< 100 Rec3C records */
extern u8 D_80095B28[];    /**< a Totals28 */
extern char D_80011260[];  /**< path of the tool file, "sim:\\PS\\PEPSI\\DATA\\TOOL0\\TMP.TL0" */
#define sTotals (*(Totals28 *)D_80095B28)

/** @brief The tool state block, seen as the save area past its totals. */
typedef struct {
    u8 unk0[0x38]; /**< not yet known */
    s32 unk38[3];  /**< a copy of sGameSave.unk348 */
    s16 unk44;     /**< a saved halfword of game state */
    u8 unk46[2];   /**< not yet known */
    s32 unk48[6];  /**< six saved words of game state */
} ToolSave;

#define sToolSave (*(ToolSave *)D_80095B28)

/** @brief The game state, seen as the part the tool state saves. */
typedef struct {
    u8 unk0[0x348]; /**< not yet known */
    s32 unk348[3];  /**< saved into sToolSave.unk38 */
} GameSave;

#define sGameSave (*(GameSave *)D_8009EB78)

void func_80032964(s32 a, u8 *buf);
void func_80032C28(s32 a, u8 *buf);
void func_800337E4(u8 *buf);
void func_8003390C(Rec3C *recs);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_80010B7C);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_80010B8C);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_80010BA4);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_80010BB8);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_80010BCC);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_80010BD8);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_80010BE4);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_80010BF0);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_80010BFC);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_80010C08);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_80010C14);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_80010C20);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_80010C2C);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_80010C38);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_80010C48);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_80010C54);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_80010C68);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_80010C78);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_80010C88);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_80010C94);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_80010CA4);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_80010CB0);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_80010CC4);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_80010CD4);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_80010CE4);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_80010CF0);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_80010D04);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_80010D18);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_80010D24);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_80010D30);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_80010D3C);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_80010D54);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_80010D64);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_80010D74);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_80010D84);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_80010D9C);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_80010DAC);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_80010DC4);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_80010DD0);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_80010DDC);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_80010DE8);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_80010DF4);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_80010E00);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_80010E0C);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_80010E18);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_80010E28);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_80010E3C);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_80010E50);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_80010E60);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_80010E74);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_80010E8C);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_80010EA4);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_80010EBC);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_80010EC8);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_80010EE0);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_80010EEC);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_80010F00);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_80010F0C);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_80010F24);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_80010F38);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_80010F50);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_80010F60);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_80010F6C);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_80010F80);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_80010F98);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_80010FAC);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_80010FBC);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_80010FCC);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_80010FDC);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_80010FEC);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_80010FFC);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_80011010);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_80011020);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_8001102C);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_8001103C);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_80011048);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_80011054);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_80011064);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_80011074);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_80011080);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_8001108C);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_80011098);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_800110A4);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_800110B0);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_800110BC);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_800110C8);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_800110D4);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_800110E0);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_800110EC);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_800110F8);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_80011104);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_80011114);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_80011124);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_80011134);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_80011144);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_80011158);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_80011168);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_80011178);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_8001118C);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_8001119C);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_800111B4);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_800111C0);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_800111D4);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_800111EC);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_800111FC);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_80011208);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_8001121C);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_8001122C);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_8001123C);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_80011248);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_80011254);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_80011260);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_80011284);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_800112A8);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_800112CC);

INCLUDE_ASM("asm/nonmatchings/code_1dc24", func_8002D424);

INCLUDE_ASM("asm/nonmatchings/code_1dc24", func_8002DC44);

INCLUDE_ASM("asm/nonmatchings/code_1dc24", func_8002F270);

INCLUDE_ASM("asm/nonmatchings/code_1dc24", func_8002F6A0);

INCLUDE_ASM("asm/nonmatchings/code_1dc24", func_8002F8FC);

INCLUDE_ASM("asm/nonmatchings/code_1dc24", func_8002FA78);

INCLUDE_ASM("asm/nonmatchings/code_1dc24", func_8002FDB4);

INCLUDE_ASM("asm/nonmatchings/code_1dc24", func_8002FF74);

INCLUDE_ASM("asm/nonmatchings/code_1dc24", func_80030278);

INCLUDE_ASM("asm/nonmatchings/code_1dc24", func_80030548);

INCLUDE_ASM("asm/nonmatchings/code_1dc24", func_80030984);

INCLUDE_ASM("asm/nonmatchings/code_1dc24", func_80030B6C);

INCLUDE_ASM("asm/nonmatchings/code_1dc24", func_80030DA0);

INCLUDE_ASM("asm/nonmatchings/code_1dc24", func_80031064);

INCLUDE_ASM("asm/nonmatchings/code_1dc24", func_8003146C);

INCLUDE_ASM("asm/nonmatchings/code_1dc24", func_800317D0);

INCLUDE_ASM("asm/nonmatchings/code_1dc24", func_80031A48);

INCLUDE_ASM("asm/nonmatchings/code_1dc24", func_80031AEC);

INCLUDE_ASM("asm/nonmatchings/code_1dc24", func_80031EF4);

INCLUDE_ASM("asm/nonmatchings/code_1dc24", func_8003245C);

INCLUDE_ASM("asm/nonmatchings/code_1dc24", func_800327BC);

INCLUDE_ASM("asm/nonmatchings/code_1dc24", func_80032964);

INCLUDE_ASM("asm/nonmatchings/code_1dc24", func_80032C28);

INCLUDE_ASM("asm/nonmatchings/code_1dc24", func_80032EE4);

INCLUDE_ASM("asm/nonmatchings/code_1dc24", func_800330D4);

INCLUDE_ASM("asm/nonmatchings/code_1dc24", func_80033224);

INCLUDE_ASM("asm/nonmatchings/code_1dc24", func_80033388);

/** @brief Wraps the highlighted line at `n` lines, resets the edited value
 *         and runs an update.
 *  @return the highlighted line when flag bit 5 is set, else -1. */
s32 func_8003356C(s16 n) {
    D_80095748 = 0;
    D_800958B2 = n;
    D_800958B0 = 1;
    D_8009574A = D_8009574A % n;
    func_800330D4();
    if (D_80095970 & 0x20) {
        return D_8009574A;
    }
    return -1;
}

/** @brief Saves three pieces of game state into the tool state block,
 *         then runs a step on the tool buffer. */
void func_800335E8(s16 a) {
    sToolSave.unk38[0] = sGameSave.unk348[0];
    sToolSave.unk38[1] = sGameSave.unk348[1];
    sToolSave.unk38[2] = sGameSave.unk348[2];
    sToolSave.unk44 = D_800A7682[0];
    sToolSave.unk48[0] = D_800DB2A0[0];
    sToolSave.unk48[1] = D_800DB2A0[1];
    sToolSave.unk48[2] = D_800DB2A0[2];
    sToolSave.unk48[3] = D_800DB2A0[3];
    sToolSave.unk48[4] = D_800DB2A0[4];
    sToolSave.unk48[5] = D_800DB2A0[5];
    func_80032964(a, (u8 *)0x8016D000);
}

/** @brief Checks that save slot `i` of the tool buffer is valid and
 *         belongs to the current owner.
 *  @return 0 when it does, -1 when not. */
s32 func_80033680(s16 i) {
    SaveSlot *slot;

    slot = &((SaveSlot *)0x8016D000)[i];
    /* MATCHING: two guards, each returning -1; an && test lays the success
     * path out as the branch target. */
    if (slot->unk200 != 0x38) {
        return -1;
    }
    if (slot->unk202 != D_80095830) {
        return -1;
    }
    return 0;
}

/** @brief Runs a step on the tool buffer, then restores the game state
 *         the save step put in the tool state block. */
void func_800336F8(s16 a) {
    func_80032C28(a, (u8 *)0x8016D000);
    sGameSave.unk348[0] = sToolSave.unk38[0];
    sGameSave.unk348[1] = sToolSave.unk38[1];
    sGameSave.unk348[2] = sToolSave.unk38[2];
    D_800A7682[0] = sToolSave.unk44;
    D_800DB2A0[0] = sToolSave.unk48[0];
    D_800DB2A0[1] = sToolSave.unk48[1];
    D_800DB2A0[2] = sToolSave.unk48[2];
    D_800DB2A0[3] = sToolSave.unk48[3];
    D_800DB2A0[4] = sToolSave.unk48[4];
    D_800DB2A0[5] = sToolSave.unk48[5];
}

/** @brief Clears and sets up a fixed 0x800-byte block near the top of RAM,
 *         then clears and resets the Rec3C table. */
void func_80033790(void) {
    bzero((u8 *)0x801FD000, 0x800);
    func_800337E4((u8 *)0x801FD000);
    bzero((u8 *)D_800A7898, sizeof(Rec3C) * 100);
    func_8003390C(D_800A7898);
}

/** @brief Builds an empty block header in `buf` for the current entry
 *         count and points the current-block globals at it. */
void func_800337E4(u8 *buf) {
    s32 n;

    func_8002D0C4((BlockHeader *)buf);
    n = D_80095794;
    D_800959C0 = buf + 8;
    D_800959C8 = n;
    D_800959C4 = buf + (n * 8 + 8);
    /* MATCHING: byte-pointer stores, not BlockHeader members: a member
     * store does not alias the globals, so the reload below would go. */
    *(s32 *)buf = n;
    *(s32 *)(buf + 4) = D_800959C8 * 8 + 8;
}

/** @brief Sets byte 0 of all 200 records of a Rec5C table to -1. */
void func_80033854(Rec5C *recs) {
    u32 i;

    for (i = 0; i < 200; i++) {
        recs->unk0 = -1;
        recs++;
    }
}

/** @brief Sets byte 0 of records 100 to 199 of a 200-entry table to -1. */
void func_80033878(Rec5C *recs) {
    u32 i;

    recs += 100;
    for (i = 100; i < 200; i++) {
        recs->unk0 = -1;
        recs++;
    }
}

/** @brief Resets all 200 records of a Rec48 table. */
void func_800338A0(Rec48 *recs) {
    u32 i;

    for (i = 0; i < 200; i++) {
        recs->unk36 = -1;
        recs->unk38 = -1;
        recs->unk34 = -1;
        recs->unk41 = 1;
        recs++;
    }
}

/** @brief Resets records 150 to 199 of a 200-entry Rec48 table. */
void func_800338D8(Rec48 *recs) {
    u32 i;
    Rec48 *r;

    r = &recs[150];
    for (i = 150; i < 200; i++) {
        r->unk36 = -1;
        r->unk34 = -1;
        r->unk41 = 1;
        r++;
    }
}

/** @brief Sets the halfword at 0 of all 100 records of a Rec3C table to -1. */
void func_8003390C(Rec3C *recs) {
    u32 i;

    for (i = 0; i < 100; i++) {
        recs->unk0 = -1;
        recs++;
    }
}

/** @brief Runs a step on the tool buffer, then copies its first 64 KiB to
 *         the next 64 KiB. */
void func_80033930(void) {
    func_80032964(0, (u8 *)0x8016D000);
    *(Page64K *)0x8017D000 = *(Page64K *)0x8016D000;
}

/** @brief Copies the second 64 KiB of the tool buffer back over the first,
 *         then runs a step on it. */
void func_8003399C(void) {
    *(Page64K *)0x8016D000 = *(Page64K *)0x8017D000;
    func_80032C28(0, (u8 *)0x8016D000);
}

INCLUDE_ASM("asm/nonmatchings/code_1dc24", func_80033A08);

/** @brief Totals unk4 of the current block's entries into sTotals.unk26. */
void func_80033AB8(void) {
    s32 i;
    Ent8 *e;

    e = (Ent8 *)D_800959C0;
    sTotals.unk26 = 0;
    for (i = 0; i < D_800959C8; i++) {
        sTotals.unk26 += e->unk4;
        e++;
    }
}

/** @brief Runs an update, then latches a halfword into the first slot of a
 *         halfword run. */
void func_80033B08(void) {
    func_800330D4();
    D_80095B4C[0] = D_80095748;
}

INCLUDE_ASM("asm/nonmatchings/code_1dc24", func_80033B34);

/** @brief Applies every used Rec48 whose unk34 equals sTotals.unk1E to the
 *         Rec78 its unk36 names. */
void func_80033BF8(void) {
    u32 i;
    Rec48 *r;

    for (i = 0; i < 200; i++) {
        r = &((Rec48 *)D_800A9008)[i];
        if (r->unk36 != -1 && sTotals.unk1E == r->unk34) {
            func_8002A7D8(&D_800D8D20[r->unk36], r);
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/code_1dc24", func_80033C90);

INCLUDE_ASM("asm/nonmatchings/code_1dc24", func_80033D3C);

/** @brief Writes the tool buffer to the tool file on the host.
 *  @return the count written. */
s32 func_80033DE8(void) {
    s32 fd;
    s32 n;

    fd = open(D_80011260, O_CREAT | O_WRONLY);
    n = write(fd, (void *)0x8016D000, 0x2285C);
    close(fd);
    return n;
}

/** @brief Reads the tool file on the host into the tool buffer.
 *  @return the count read. */
s32 func_80033E40(void) {
    s32 fd;
    s32 n;

    fd = open(D_80011260, O_RDONLY);
    n = read(fd, (void *)0x8016D000, 0x2285C);
    close(fd);
    return n;
}

INCLUDE_ASM("asm/nonmatchings/code_1dc24", func_80033E98);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_800114DC);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_800114E8);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_800114F4);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_80011500);

INCLUDE_RODATA("asm/nonmatchings/code_1dc24", D_8001151C);
