#include "common.h"
#include "memory.h"
#include "libapi.h"
#include "sys/file.h"

/** @brief One of the 80 entries heading a record bank: where the entry's
 *         records start. */
typedef struct {
    s32 first; /**< index in the bank's records of the entry's first one */
    s32 unk4;  /**< not yet known */
} BankEntry;

/** @brief A 0x2C-byte record of the first of two record banks. */
typedef struct {
    u8 unk0[0x2C]; /**< not yet known */
} Rec2C;

/** @brief A 0x4C-byte record of the second of two record banks. */
typedef struct {
    u8 unk0[0x4C]; /**< not yet known */
} Rec4C;

/** @brief The first record bank: 80 entries, then 0x2C-byte records. */
typedef struct {
    BankEntry entries[80]; /**< per-entry start indices */
    Rec2C recs[1];         /**< the records; real count unknown */
} Bank2C;

/** @brief The second record bank: 80 entries, then 0x4C-byte records. */
typedef struct {
    BankEntry entries[80]; /**< per-entry start indices */
    Rec4C recs[1];         /**< the records; real count unknown */
} Bank4C;

/** @brief A 0x78-byte record of an 80-entry table; two pairs of halfwords
 *         are reset together. */
typedef struct {
    u8 unk0[0x6E]; /**< not yet known */
    s16 unk6E;     /**< zeroed when the second buffer is cleared */
    s16 unk70;     /**< -1 when the second buffer is cleared */
    s16 unk72;     /**< zeroed when the first buffer is cleared */
    s16 unk74;     /**< -1 when the first buffer is cleared */
    u8 unk76[2];   /**< not yet known */
} Rec78;

extern u8 D_800D3CA8[];    /**< 0x44C0-byte buffer, cleared as a whole */
extern u8 D_800DB2C0[];    /**< 0x1DB0-byte buffer, cleared as a whole */
extern Rec78 D_800D8D20[]; /**< 80 records */

extern s32 D_80095970; /**< flag word; bit 5 enables a two-state dispatch */
extern u8 D_80095A29;  /**< state of that dispatch: 0 or 1 */

extern s16 D_80095A30; /**< current index, clamped to the entry count */
extern s16 D_8009588E; /**< number of entries */

void func_80034F38(void);
void func_80036F50(void);

extern char D_80011768[]; /**< path of the tool file, "sim:\\PS\\PEPSI\\DATA\\TOOL1\\TMP.TL1" */

INCLUDE_ASM("asm/nonmatchings/code_24748", func_80033F48);

INCLUDE_RODATA("asm/nonmatchings/code_24748", D_80011528);

INCLUDE_RODATA("asm/nonmatchings/code_24748", D_80011534);

INCLUDE_RODATA("asm/nonmatchings/code_24748", D_80011540);

INCLUDE_RODATA("asm/nonmatchings/code_24748", D_8001154C);

INCLUDE_RODATA("asm/nonmatchings/code_24748", D_80011564);

INCLUDE_RODATA("asm/nonmatchings/code_24748", D_80011570);

INCLUDE_RODATA("asm/nonmatchings/code_24748", D_80011588);

INCLUDE_RODATA("asm/nonmatchings/code_24748", D_80011594);

INCLUDE_RODATA("asm/nonmatchings/code_24748", D_800115A0);

INCLUDE_RODATA("asm/nonmatchings/code_24748", D_800115AC);

INCLUDE_RODATA("asm/nonmatchings/code_24748", D_800115BC);

INCLUDE_RODATA("asm/nonmatchings/code_24748", D_800115D0);

INCLUDE_RODATA("asm/nonmatchings/code_24748", D_800115DC);

INCLUDE_RODATA("asm/nonmatchings/code_24748", D_80011618);

INCLUDE_RODATA("asm/nonmatchings/code_24748", D_80011624);

INCLUDE_RODATA("asm/nonmatchings/code_24748", D_80011634);

INCLUDE_RODATA("asm/nonmatchings/code_24748", D_80011644);

INCLUDE_RODATA("asm/nonmatchings/code_24748", D_80011654);

INCLUDE_RODATA("asm/nonmatchings/code_24748", D_80011664);

INCLUDE_RODATA("asm/nonmatchings/code_24748", D_80011678);

INCLUDE_RODATA("asm/nonmatchings/code_24748", D_80011688);

INCLUDE_RODATA("asm/nonmatchings/code_24748", D_80011698);

INCLUDE_RODATA("asm/nonmatchings/code_24748", D_800116AC);

INCLUDE_RODATA("asm/nonmatchings/code_24748", D_800116BC);

INCLUDE_RODATA("asm/nonmatchings/code_24748", D_800116D4);

INCLUDE_RODATA("asm/nonmatchings/code_24748", D_800116E0);

INCLUDE_RODATA("asm/nonmatchings/code_24748", D_800116F4);

INCLUDE_RODATA("asm/nonmatchings/code_24748", D_8001170C);

INCLUDE_RODATA("asm/nonmatchings/code_24748", D_8001171C);

INCLUDE_RODATA("asm/nonmatchings/code_24748", D_80011728);

INCLUDE_RODATA("asm/nonmatchings/code_24748", D_8001173C);

INCLUDE_RODATA("asm/nonmatchings/code_24748", D_8001174C);

INCLUDE_RODATA("asm/nonmatchings/code_24748", D_8001175C);

INCLUDE_RODATA("asm/nonmatchings/code_24748", D_80011768);

INCLUDE_RODATA("asm/nonmatchings/code_24748", D_8001178C);

INCLUDE_RODATA("asm/nonmatchings/code_24748", D_800117B4);

INCLUDE_RODATA("asm/nonmatchings/code_24748", D_800117DC);

INCLUDE_ASM("asm/nonmatchings/code_24748", func_80034070);

INCLUDE_ASM("asm/nonmatchings/code_24748", func_80034388);

INCLUDE_ASM("asm/nonmatchings/code_24748", func_800345C8);

INCLUDE_ASM("asm/nonmatchings/code_24748", func_80034788);

INCLUDE_ASM("asm/nonmatchings/code_24748", func_80034BCC);

INCLUDE_ASM("asm/nonmatchings/code_24748", func_80034D5C);

INCLUDE_ASM("asm/nonmatchings/code_24748", func_80034F38);

INCLUDE_ASM("asm/nonmatchings/code_24748", func_800350C8);

INCLUDE_ASM("asm/nonmatchings/code_24748", func_80035350);

INCLUDE_ASM("asm/nonmatchings/code_24748", func_800355D8);

INCLUDE_ASM("asm/nonmatchings/code_24748", func_800356FC);

INCLUDE_ASM("asm/nonmatchings/code_24748", func_80035970);

INCLUDE_ASM("asm/nonmatchings/code_24748", func_80035E24);

INCLUDE_ASM("asm/nonmatchings/code_24748", func_80036184);

INCLUDE_ASM("asm/nonmatchings/code_24748", func_8003634C);

INCLUDE_ASM("asm/nonmatchings/code_24748", func_80036478);

INCLUDE_ASM("asm/nonmatchings/code_24748", func_800365A0);

INCLUDE_ASM("asm/nonmatchings/code_24748", func_80036704);

INCLUDE_ASM("asm/nonmatchings/code_24748", func_80036878);

Rec2C *func_80036A50(s32 idx, s32 sub) {
    Bank2C *bank = (Bank2C *)D_80095A50;
    Rec2C *recs = bank->recs;

    return &recs[bank->entries[idx].first + sub];
}

Rec4C *func_80036A84(s32 idx, s32 sub) {
    Bank4C *bank = (Bank4C *)D_80095A4C;
    Rec4C *recs = bank->recs;

    return &recs[bank->entries[idx].first + sub];
}

INCLUDE_ASM("asm/nonmatchings/code_24748", func_80036AB8);

INCLUDE_ASM("asm/nonmatchings/code_24748", func_80036B90);

INCLUDE_ASM("asm/nonmatchings/code_24748", func_80036D50);

void func_80036E50(void) {
    u32 i;

    bzero(D_800D3CA8, 0x44C0);
    for (i = 0; i < 80; i++) {
        D_800D8D20[i].unk72 = 0;
        D_800D8D20[i].unk74 = -1;
    }
}

void func_80036EA0(void) {
    u32 i;

    bzero(D_800DB2C0, 0x1DB0);
    for (i = 0; i < 80; i++) {
        D_800D8D20[i].unk6E = 0;
        D_800D8D20[i].unk70 = -1;
    }
}

void func_80036EF0(void) {
    if (D_80095970 & 0x20) {
        switch (D_80095A29) {
            case 0:
                func_80034F38();
                break;
            case 1:
                func_80036F50();
                break;
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/code_24748", func_80036F50);

INCLUDE_ASM("asm/nonmatchings/code_24748", func_80036FE8);

void func_8003708C(void) {
    s32 flags = D_80095970;

    if (flags & 8) {
        D_80095A30++;
    }
    if (flags & 4) {
        D_80095A30--;
    }
    D_80095A30 = D_80095A30 < 0 ? 0 : D_80095A30 > D_8009588E - 1 ? D_8009588E - 1 : D_80095A30;
}

INCLUDE_ASM("asm/nonmatchings/code_24748", func_80037114);

INCLUDE_ASM("asm/nonmatchings/code_24748", func_800371A0);

INCLUDE_ASM("asm/nonmatchings/code_24748", func_80037280);

s32 func_80037318(void) {
    s32 fd;
    s32 n;

    fd = open(D_80011768, O_CREAT | O_WRONLY);
    n = write(fd, (void *)0x8018D000, 0x13868);
    close(fd);
    return n;
}

s32 func_80037370(void) {
    s32 fd;
    s32 n;

    fd = open(D_80011768, O_RDONLY);
    n = read(fd, (void *)0x8018D000, 0x13868);
    close(fd);
    return n;
}
