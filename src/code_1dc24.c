#include "common.h"
#include "memory.h"
#include "libapi.h"
#include "libgte.h"
#include "libgpu.h"
#include "sys/file.h"
#include "code_1a098.h"
#include "code_13068.h"
#include "code_a0bc.h"

/** @brief A 0x3C-byte record of a 100-entry table; only the halfword at 0 is
 *         known. */
typedef struct {
    s16 unk0;      /**< -1 when the record is free (a guess) */
    u8 unk2[2];    /**< not yet known */
    s32 unk4[3];   /**< a position */
    s32 unk10[3];  /**< a copy of unk4 */
    u8 unk1C[0xA]; /**< not yet known */
    u8 unk26;      /**< bit 7 picks one of two handlers */
    u8 unk27;      /**< sTotals.unk6C when placed */
    u8 unk28;      /**< sTotals.unk6D when placed */
    u8 unk29;      /**< not yet known */
    u16 unk2A;     /**< sTotals.unk6A when placed */
    s16 unk2C;     /**< matched against sTotals.unk2A */
    u8 unk2E[2];   /**< not yet known */
    s32 unk30;     /**< a global stamp when placed */
    s32 unk34;     /**< the current entry when placed */
    u8 unk38[4];   /**< not yet known */
} Rec3C;

/** @brief The tool state block: counts, totals and saved menu values. */
typedef struct {
    u8 unk0[3];     /**< not yet known */
    u8 unk3;        /**< the menu line the tool mode was entered from */
    u8 unk4[8];     /**< not yet known */
    s32 unkC;       /**< a height offset added to the camera's y */
    u8 unk10[2];    /**< not yet known */
    u16 unk12;      /**< the edited value saved for menu line 0 */
    u16 unk14;      /**< the edited value saved for menu line 1 */
    u8 unk16[2];    /**< not yet known */
    u16 unk18;      /**< number of Obj48 records in use */
    u16 unk1A;      /**< number of Obj48 records counted live */
    u16 unk1C;      /**< unk38 of the first record placed */
    u16 unk1E;      /**< matched against a Rec48's unk34 */
    s16 unk20;      /**< an angle in degrees, wrapped to 0..359 */
    u8 unk22[2];    /**< not yet known */
    u16 unk24;      /**< the edited value saved for tool mode 2 */
    u16 unk26;      /**< a sum over the current block's entries */
    u16 unk28;      /**< the Rec3C slot the next record goes to */
    u16 unk2A;      /**< matched against a Rec3C's unk2C */
    u16 unk2C;      /**< a Rec3C kind, less 30 */
    u8 unk2E[2];    /**< not yet known */
    u16 unk30;      /**< number of Rec3C records in use */
    u16 unk32;      /**< picks one of two Rec3C handlers */
    u8 unk34[0x34]; /**< not yet known */
    u8 unk68;       /**< copied into a placed record's unk42 */
    u8 unk69;       /**< copied into a placed record's unk43 */
    u16 unk6A;      /**< copied into a placed Rec3C's unk2A */
    u8 unk6C;       /**< copied into a placed Rec3C's unk27 */
    u8 unk6D;       /**< copied into a placed Rec3C's unk28 */
} Totals28;

/** @brief 64 KiB of the tool buffer, copied whole. */
typedef struct {
    u8 b[0x10000]; /**< not yet known */
} Page64K;

/** @brief The tool state block, copied whole. */
typedef struct {
    s32 w[0x74 / 4]; /**< the block */
} TotalsCopy;

/** @brief The 200-entry Obj48 table, copied whole. */
typedef struct {
    s32 w[0x3840 / 4]; /**< the records */
} Obj48sCopy;

/** @brief The 200-entry 0x5C-byte record table, copied whole. */
typedef struct {
    s32 w[0x47E0 / 4]; /**< the records */
} Rec5CsCopy;

/** @brief The 100-entry Rec3C table, copied whole. */
typedef struct {
    s32 w[0x1770 / 4]; /**< the records */
} Rec3CsCopy;

/** @brief The block header area, copied whole; bytes, so a copy of it
 *         tests the alignment at run time. */
typedef struct {
    u8 b[0x800]; /**< the area */
} BlockAreaCopy;

/** @brief A 0xB774-byte slot of the tool buffer, seen 0x200 bytes early:
 *         the slot's data starts at unk200 and its block header area runs
 *         0x200 bytes past this view's end. */
typedef struct {
    u8 unk0[0x200];     /**< not yet known */
    u8 unk200;          /**< 0x38 when the slot is valid */
    u8 unk201;          /**< not yet known */
    u8 unk202;          /**< the owner; compared with a global */
    u8 unk203[0x71];    /**< the rest of the tool state block */
    Obj48sCopy unk274;  /**< the Obj48 table */
    Rec5CsCopy unk3AB4; /**< the 0x5C-byte record table */
    Rec3CsCopy unk8294; /**< the Rec3C table */
    u8 unk9A04[0x1D70]; /**< not yet known; the block area starts at 0xB174 */
} SaveSlot;

/* MATCHING: D_800A7898 and D_80095B28 are also declared in code_1a098, each
 * unit through its own view (Rec3C, Totals28 here; Rec3C, Progress6E there). */
extern u16 D_80095B4C[];   /**< first of a run of halfwords */
extern Rec3C D_800A7898[]; /**< 100 Rec3C records */
extern u8 D_80095B28[];    /**< a Totals28 */
extern char D_80011260[];  /**< path of the tool file, "sim:\\PS\\PEPSI\\DATA\\TOOL0\\TMP.TL0" */
#define sTotals (*(Totals28 *)D_80095B28)
extern char D_800955DC[]; /**< "\n\n" */
extern char D_800954F4[]; /**< colour code of a highlighted menu line */
extern char D_8009550C[]; /**< colour code of a plain menu line */
extern char D_80095628[]; /**< "  YES\n" */
extern char D_80095630[]; /**< "  NO" */
extern char D_80095638[]; /**< "  SAVE\n" */
extern char D_80095640[]; /**< "  LOAD\n" */
extern char D_8001123C[]; /**< "DATA:%d " */
extern char D_80011248[]; /**< "STAGE %d-" */
extern char D_80011254[]; /**< "NO DATA\n" */
extern char D_80095648[]; /**< "1" */
extern char D_8009564C[]; /**< "2" */
extern char D_80095650[]; /**< "BOSS" */
extern char D_80095524[]; /**< "\n" */

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

/* MATCHING: also declared in code_1a098, which only zeroes it. */
extern s32 D_80095824; /**< the current entry of the block, -1 for none */
extern s32 D_80095950; /**< a pad word; bits step the edited value */
extern s32 D_80095958; /**< a pad word; bits step the highlighted line */

s32 func_80033E98(void);

/** @brief A block entry as its first and count words. */
typedef struct {
    s32 start; /**< index of the entry's first point */
    s32 count; /**< number of points */
} Span8;

/** @brief An eight-byte point record of the block's second part. */
typedef struct {
    s16 x;   /**< x */
    s16 y;   /**< y */
    s16 z;   /**< z */
    u16 tag; /**< one more than the latched halfword */
} Pt8;

/** @brief The current-block pointers, seen as one structure. */
typedef struct {
    u8 *ents; /**< the block's entries */
    u8 *pts;  /**< the block's points */
} BlockCur;

/* MATCHING: reached as members, so a member store through a pointer may
 * alias them (common.h declares them as scalars). */
#define sCur (*(BlockCur *)&D_800959C0)

/** @brief A 0x48-byte record of the 200-entry record table, as
 *         this unit places it (code_1a098.h's Rec48 is the same record). */
typedef struct {
    s32 unk0[3];   /**< a position */
    s32 unkC[3];   /**< a copy of unk0 */
    u8 unk18[0xC]; /**< not yet known */
    s16 unk24;     /**< cleared when placed */
    s16 unk26;     /**< cleared when placed */
    s32 unk28;     /**< the current entry when placed */
    u8 unk2C[8];   /**< not yet known */
    s16 unk34;     /**< sTotals.unk1E when placed */
    s16 unk36;     /**< -1 when free */
    s16 unk38;     /**< sTotals.unk1C for the first placed, else -1 */
    u8 unk3A[2];   /**< not yet known */
    s32 unk3C;     /**< a global stamp when placed */
    u8 unk40;      /**< cleared when placed */
    u8 unk41;      /**< not yet known */
    u8 unk42;      /**< sTotals.unk68 when placed */
    u8 unk43;      /**< sTotals.unk69 when placed */
    u8 unk44[4];   /**< not yet known */
} Obj48;

/* MATCHING: code_1a098 declares D_800959D8 as u8 (it only zeroes it); this
 * unit tests it as s8. */
extern Obj48 D_80096788[]; /**< records waiting to be placed */
extern u16 D_8009596E;     /**< number of records waiting */
extern s8 D_800959D8;      /**< a flag; cleared after placing when 1 */

/* MATCHING: per-unit views while Rec3C is unit-local; the two handlers
 * take an s32 id here (passed unextended), u16 and s16 in code_1a098. */
void func_80032964(s32 a, u8 *buf);
void func_80032C28(s16 a, u8 *buf);
void func_800337E4(u8 *buf);
void func_8003390C(Rec3C *recs);
void func_8002C894(s32 id, Rec3C *r);
void func_8002B8F8(s32 id, Rec3C *r);

/** @brief A rotation followed by a position (code_13068.h declares the
 *         record as an SVECTOR array). */
typedef struct {
    SVECTOR rot; /**< the rotation */
    s32 pos[3];  /**< the position */
} RotPos;

#define sRotPos (*(RotPos *)D_800A7680)

/* MATCHING: declared per unit (code_1a098 and code_308ec give their own
 * views); here the start position is passed as words. */
s32 func_800183B0(s32 *pos);
extern u8 D_800958D8; /**< a flag set by tool modes 4 and 5 */

/* MATCHING: code_1a098 declares it as its Rec5C records. */
extern s32 D_800CF080[]; /**< 200 0x5C-byte records */

extern Rec3C D_800DF818; /**< the Rec3C template that gets placed */
extern u8 D_800959E2;    /**< or-ed into a placed Rec3C's unk26 */

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

/** @brief Runs the menu editor, then on flag bit 5 enters the tool mode
 *         of the highlighted line.
 *  @return nothing; the value is undefined. */
s32 func_8002F6A0(void) {
    /* MATCHING: non-void with no return puts the index sll in the bound
     * check's delay slot. */
    func_800330D4();
    if (D_80095970 & 0x20) {
        switch (D_8009574A) {
            case 0:
                D_800958B0 = 200;
                D_800958B2 = 1;
                D_800958D8 = 0;
                D_800959D8 = 0;
                sTotals.unk3 = D_8009574A;
                D_80095748 = sTotals.unk24;
                D_8009574A = 0;
                D_800958DA = 2;
                break;
            case 1:
                D_800958B0 = 200;
                D_800958B2 = 2;
                D_800958D8 = 0;
                D_800959D8 = 0;
                sTotals.unk3 = D_8009574A;
                D_80095748 = sTotals.unk12;
                D_8009574A = 0;
                D_800958DA = 3;
                break;
            case 2:
                D_800958B0 = 200;
                D_800958B2 = 5;
                D_800958D8 = 0;
                D_800959D8 = 0;
                sTotals.unk3 = D_8009574A;
                D_80095748 = sTotals.unk18;
                D_8009574A = 0;
                D_800958A6 = 0;
                D_800958DA = 4;
                break;
            case 3:
                D_800958B0 = 100;
                D_800958B2 = 9;
                D_800958D8 = 0;
                D_800959D8 = 0;
                sTotals.unk3 = D_8009574A;
                D_80095748 = sTotals.unk28;
                D_8009574A = 0;
                D_800958A6 = 0;
                D_800958DA = 5;
                break;
            case 4:
                D_800958D8 = 1;
                D_800959D8 = 0;
                D_800958A6 = 0;
                break;
            case 5:
                D_800959D8 = 1;
                /* MATCHING: an int-valued 1 (retail rebuilds the constant). */
                D_800958D8 = D_800959D8 != 0;
                D_800958A6 = 0;
                D_800958DA = 0;
                break;
            case 6:
                D_800958DA = 0;
                sGameSave.unk348[0] = sRotPos.pos[0];
                sGameSave.unk348[1] = sRotPos.pos[1];
                sGameSave.unk348[2] = sRotPos.pos[2];
                sRotPos.rot.vy = sRotPos.rot.vz;
                D_800957F4 = func_800183B0(sGameSave.unk348);
                D_8009578C = 0;
                D_8009676C[0] = (s16)(sRotPos.pos[1] - 500);
                break;
            case 7:
                D_800958DA = 6;
                D_800958A6 = 0;
                break;
        }
    }
}

/** @brief Fills the six camera words from the yaw in the first rotation
 *         and the view's orbit angle. */
void func_8002F8FC(void) {
    SVECTOR *rot;

    rot = D_800A7680;
    D_800DB2A0[0] = rsin(rot->vy) * 500 / 4096;
    D_800DB2A0[1] = rsin(D_80095914 * 4096 / 360) * 500 / 4096 + (sGameSave.unk348[1] + sTotals.unkC);
    D_800DB2A0[2] = rcos(rot->vy) * 500 / 4096;
    D_800DB2A0[3] = rsin(rot->vy - 0x800) * 900 / 4096;
    D_800DB2A0[4] = sGameSave.unk348[1];
    D_800DB2A0[5] = rcos(rot->vy - 0x800) * 900 / 4096;
}

INCLUDE_ASM("asm/nonmatchings/code_1dc24", func_8002FA78);

/** @brief On flag bit 5, inserts the game position as a new point at the
 *         end of the current entry, shifting the later points up, then
 *         rebuilds the block header. */
/* MATCHING: `e = base; e += k;` and one points pointer for the shift source
 * and the new point. */
void func_8002FDB4(void) {
    s16 pos[3];
    Pt8 *dst;
    Pt8 *p;
    Span8 *e;
    s32 i;
    u32 j;

    if (D_80095970 & 0x20) {
        if (sTotals.unk26 == 200) {
            D_800958DA = 8;
        } else if (D_80095824 == -1) {
            D_800958DA = 7;
        } else {
            e = (Span8 *)sCur.ents;
            e += D_80095824;
            p = (Pt8 *)sCur.pts;
            dst = p;
            pos[0] = sGameSave.unk348[0];
            pos[1] = sGameSave.unk348[1];
            pos[2] = sGameSave.unk348[2];
            p += 198;
            dst += 199;
            for (i = e->start + e->count; i < 200; i++) {
                *dst = *p;
                dst--;
                p--;
            }
            e = (Span8 *)sCur.ents;
            e += D_80095824;
            p = (Pt8 *)sCur.pts;
            j = e->count;
            e->count = j + 1;
            p += e->start + j;
            e = (Span8 *)sCur.ents;
            e += D_80095824 + 1;
            p->x = pos[0];
            p->y = pos[1];
            p->z = pos[2];
            p->tag = D_80095B4C[0] + 1;
            for (j = D_80095824 + 1; j < D_80095794; j++) {
                e->start++;
                e++;
            }
            func_8002D0C4((BlockHeader *)0x801FD000);
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/code_1dc24", func_8002FF74);

INCLUDE_ASM("asm/nonmatchings/code_1dc24", func_80030278);

INCLUDE_ASM("asm/nonmatchings/code_1dc24", func_80030548);

/** @brief Counts the live Obj48 records; on flag bit 5, places every
 *         waiting record into the table (error 8 when it is full). */
void func_80030984(void) {
    u32 i;
    u16 n;
    Obj48 *recs;
    Obj48 *w;

    recs = (Obj48 *)D_800A9008;
    n = sTotals.unk18;
    sTotals.unk1A = 0;
    for (i = 0; i < 200; i++) {
        if (recs[i].unk36 != -1) {
            sTotals.unk1A++;
        }
    }
    if (D_80095970 & 0x20) {
        /* MATCHING: one counter for both loops, unsigned (sltiu) in the
         * first and compared signed (slt) here. */
        for (i = 0; (s32)i < D_8009596E; i++) {
            w = &D_80096788[i];
            if (n == 200) {
                D_800958DA = 8;
                return;
            }
            w->unkC[0] = w->unk0[0];
            w->unkC[1] = w->unk0[1];
            w->unkC[2] = w->unk0[2];
            w->unk40 = 0;
            w->unk24 = 0;
            w->unk26 = 0;
            w->unk28 = D_80095824;
            w->unk3C = D_8009578C;
            w->unk42 = sTotals.unk68;
            w->unk43 = sTotals.unk69;
            if (i == 0) {
                /* MATCHING: stored through the table base, not w. */
                D_80096788[0].unk38 = sTotals.unk1C;
            } else {
                w->unk38 = -1;
            }
            ((Obj48 *)D_800A9008)[(s16)n] = *w;
            ((Obj48 *)D_800A9008)[(s16)n].unk34 = sTotals.unk1E;
            n++;
            sTotals.unk18 = n;
        }
        D_800958A6 = 0;
        if (D_800959D8 == 1) {
            D_800959D8 = 0;
        }
    }
}

/** @brief Runs the menu editor; on a line change loads the new line's
 *         value and limit, else stores the edited value back; then steps
 *         the angle in sTotals.unk20 on flag bits 3 and 2.
 *  @return nothing; the value is undefined. */
s32 func_80030B6C(void) {
    s16 line;
    s16 val;

    /* MATCHING: non-void with no return keeps the last two branches' delay
     * slots nops. */
    line = D_8009574A;
    val = D_80095748;
    func_800330D4();
    if (D_8009574A == 1 && val != (s16)D_80095748) {
        D_800958A6 = 0;
    }
    if (line != D_8009574A) {
        switch (D_8009574A) {
            case 0:
                D_800958B0 = 200;
                D_80095748 = sTotals.unk18;
                break;
            case 1:
                D_800958B0 = D_8009588E;
                D_80095748 = sTotals.unk1C;
                break;
            case 2:
                D_800958B0 = 200;
                D_80095748 = sTotals.unk1E;
                break;
            case 3:
                D_800958B0 = 0x100;
                D_80095748 = sTotals.unk68;
                break;
            case 4:
                D_800958B0 = 0x100;
                D_80095748 = sTotals.unk69;
                break;
        }
    } else {
        switch (line) {
            case 0:
                sTotals.unk18 = D_80095748;
                break;
            case 1:
                sTotals.unk1C = D_80095748;
                break;
            case 2:
                sTotals.unk1E = D_80095748;
                break;
            case 3:
                sTotals.unk68 = D_80095748;
                break;
            case 4:
                sTotals.unk69 = D_80095748;
                break;
        }
    }
    if (D_80095964 & 8) {
        if (++sTotals.unk20 >= 360) {
            sTotals.unk20 = 0;
        }
    }
    if (D_80095964 & 4) {
        if (--sTotals.unk20 < 0) {
            sTotals.unk20 = 359;
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/code_1dc24", func_80030DA0);

INCLUDE_ASM("asm/nonmatchings/code_1dc24", func_80031064);

INCLUDE_ASM("asm/nonmatchings/code_1dc24", func_8003146C);

/** @brief Fills the Rec3C template at the game position and runs its
 *         handler, counts the used Rec3C records, and on flag bit 5 places
 *         the template at the next slot and moves the slot on to a free
 *         one (error 8 when the table is full).
 *  @return nothing; the value is undefined. */
s32 func_800317D0(void) {
    u32 i;
    s32 f;

    /* MATCHING: non-void with no return value; void moves the block order
     * and the delay slots. */
    D_800DF818.unk4[0] = sGameSave.unk348[0];
    D_800DF818.unk4[1] = sGameSave.unk348[1];
    D_800DF818.unk4[2] = sGameSave.unk348[2];
    if (sTotals.unk32 == 0) {
        func_8002B8F8(sTotals.unk2C + 30, &D_800DF818);
    } else {
        func_8002C894(sTotals.unk2C + 30, &D_800DF818);
    }
    /* MATCHING: the counter is cleared before the store, so each arm above
     * ends with it (one in a jump's delay slot). */
    i = 0;
    sTotals.unk30 = 0;
    for (; i < 100; i++) {
        if (D_800A7898[i].unk0 != -1) {
            sTotals.unk30++;
        }
    }
    if (D_80095970 & 0x20) {
        if (sTotals.unk30 == 100) {
            D_800958DA = 8;
        } else {
            D_800DF818.unk2C = sTotals.unk2A;
            D_800DF818.unk0 = sTotals.unk2C + 30;
            D_800DF818.unk10[0] = D_800DF818.unk4[0];
            D_800DF818.unk10[1] = D_800DF818.unk4[1];
            D_800DF818.unk10[2] = D_800DF818.unk4[2];
            /* MATCHING: an s32 local assigned after the copy, and the flag
             * byte or-ed in by a second store. */
            f = sTotals.unk32 << 7;
            D_800DF818.unk26 = f;
            D_800DF818.unk26 |= D_800959E2;
            D_800DF818.unk27 = sTotals.unk6C;
            D_800DF818.unk28 = sTotals.unk6D;
            D_800DF818.unk2A = sTotals.unk6A;
            D_800DF818.unk30 = D_8009578C;
            D_800DF818.unk34 = D_80095824;
            D_800A7898[sTotals.unk28] = D_800DF818;
            /* MATCHING: the full-table store inside the loop, so its 8 is
             * hoisted and the block is not merged with the one above. */
            i = 0;
            while (1) {
                if (D_800A7898[sTotals.unk28].unk0 == -1) {
                    goto found;
                }
                sTotals.unk28++;
                if (sTotals.unk28 == 100) {
                    sTotals.unk28 = 0;
                }
                if (++i == 100) {
                    D_800958DA = 8;
                    return;
                }
            }
        found:
            if (D_800959D8 == 1) {
                D_800959D8 = 0;
            }
        }
    }
}

/** @brief Runs one of two handlers on every used Rec3C whose unk2C equals
 *         sTotals.unk2A, picked by bit 7 of its unk26. */
void func_80031A48(void) {
    u32 i;
    Rec3C *r;

    for (i = 0; i < 100; i++) {
        r = &D_800A7898[i];
        if (r->unk0 != -1 && sTotals.unk2A == r->unk2C) {
            if (r->unk26 & 0x80) {
                func_8002C894(r->unk0, r);
            } else {
                func_8002B8F8(r->unk0, r);
            }
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/code_1dc24", func_80031AEC);

INCLUDE_ASM("asm/nonmatchings/code_1dc24", func_80031EF4);

INCLUDE_ASM("asm/nonmatchings/code_1dc24", func_8003245C);

/** @brief Prints the three save slots of the tool buffer as a menu, each
 *         with its stage and part, or "NO DATA". */
void func_800327BC(void) {
    s16 i;
    SaveSlot *slots;
    SaveSlot *slot;
    u16 stage;
    u16 part;
    u16 v;

    slots = (SaveSlot *)0x8016D000;
    FntPrint(D_800955DC);
    for (i = 0; i < 3; i++) {
        func_80014BF0(3);
        if (D_8009574A == i) {
            FntPrint(D_800954F4);
        } else {
            FntPrint(D_8009550C);
        }
        slot = &slots[i];
        if (slot->unk200 == 0x38) {
            FntPrint(D_8001123C, i + 1);
            v = slot->unk202;
            stage = v / 3;
            FntPrint(D_80011248, stage + 1);
            part = v % 3;
            switch (part) {
                case 0:
                    FntPrint(D_80095648);
                    break;
                case 1:
                    FntPrint(D_8009564C);
                    break;
                case 2:
                    FntPrint(D_80095650);
                    break;
            }
            FntPrint(D_80095524);
        } else {
            FntPrint(D_80011254);
        }
    }
}

INCLUDE_ASM("asm/nonmatchings/code_1dc24", func_80032964);

/** @brief Restores the record tables, the block header area and the tool
 *         state block from save slot `a` of `buf`. */
void func_80032C28(s16 a, u8 *buf) {
    *(Rec5CsCopy *)D_800CF080 = ((SaveSlot *)buf)[a].unk3AB4;
    *(Obj48sCopy *)D_800A9008 = ((SaveSlot *)buf)[a].unk274;
    /* MATCHING: byte offsets past 0x7FFF build the offset whole (ori). */
    *(Rec3CsCopy *)D_800A7898 = *(Rec3CsCopy *)((u8 *)&((SaveSlot *)buf)[a] + 0x8294);
    *(BlockAreaCopy *)0x801FD000 = *(BlockAreaCopy *)((u8 *)&((SaveSlot *)buf)[a] + 0xB174);
    *(TotalsCopy *)D_80095B28 = *(TotalsCopy *)&((SaveSlot *)buf)[a].unk200;
}

INCLUDE_ASM("asm/nonmatchings/code_1dc24", func_80032EE4);

/** @brief Steps the edited value (bits 0x2000 up, 0x8000 down) or the
 *         highlighted line (0x4000 up, 0x1000 down) from the first pad
 *         word, else from the second and third, then wraps both.
 *  @return 0. */
s32 func_800330D4(void) {
    if (D_80095970 & 0x2000) {
        D_80095748++;
        func_80033E98();
    } else if (D_80095970 & 0x8000) {
        D_80095748--;
        func_80033E98();
    } else if (D_80095970 & 0x4000) {
        D_8009574A++;
        func_80033E98();
    } else if (D_80095970 & 0x1000) {
        D_8009574A--;
        func_80033E98();
    } else if (D_80095950 & 0x2000) {
        D_80095748++;
        func_80033E98();
    } else if (D_80095950 & 0x8000) {
        D_80095748--;
        func_80033E98();
    } else if (D_80095958 & 0x4000) {
        D_8009574A++;
        func_80033E98();
    } else if (D_80095958 & 0x1000) {
        D_8009574A--;
        func_80033E98();
    } else {
        return 0;
    }
    return 0;
}

/** @brief Sorts a pulsing red line from `pos` to 200 units above it into
 *         the current ordering table. */
/* MATCHING: the twin of code_24748's func_800365A0 (an s16 colour local, an
 * unused 8 bytes for the frame), without its mirrored x and z. */
void func_80033224(VECTOR *pos) {
    s32 unused[2];
    VECTOR world;
    SVECTOR screen;
    GsLINE line;
    s16 g;

    g = ((rsin(D_8009585C * 10 % 360 * 4096 / 360) * 50) >> 12) + 160;
    line.attribute = 0;
    line.r = 0xFF;
    line.g = g;
    line.b = g;
    world.vx = pos->vx + D_800A7308[0];
    world.vy = pos->vy;
    world.vz = pos->vz + D_800A7308[2];
    func_800230E0(&world, &screen);
    line.x0 = screen.vx;
    line.y0 = screen.vy;
    world.vy = pos->vy - 200;
    func_800230E0(&world, &screen);
    line.x1 = screen.vx;
    line.y1 = screen.vy;
    GsSortLine(&line, &D_800ACEA8[D_80095750], 50);
}

/** @brief Sorts a pulsing red line from `pos` to 100 units away from it
 *         along heading `deg` (degrees) into the current ordering table. */
/* MATCHING: the unused 8 and 16 bytes place world at 0x18 and screen at
 * 0x38 in the 0x68-byte frame. */
void func_80033388(VECTOR *pos, s16 deg) {
    s32 unused[2];
    VECTOR world;
    s32 unused2[4];
    SVECTOR screen;
    GsLINE line;
    s16 g;
    s32 ang;

    g = ((rsin(D_8009585C * 10 % 360 * 4096 / 360) * 50) >> 12) + 160;
    line.attribute = 0;
    line.r = 0xFF;
    line.g = g;
    line.b = g;
    world.vx = pos->vx + D_800A7308[0];
    world.vy = pos->vy;
    world.vz = pos->vz + D_800A7308[2];
    func_800230E0(&world, &screen);
    line.x0 = screen.vx;
    line.y0 = screen.vy;
    ang = deg * 4096 / 360;
    world.vx += rsin(ang) * 100 >> 12;
    world.vz += rcos(ang) * 100 >> 12;
    func_800230E0(&world, &screen);
    line.x1 = screen.vx;
    line.y1 = screen.vy;
    GsSortLine(&line, &D_800ACEA8[D_80095750], 50);
}

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
    sToolSave.unk44 = D_800A7680[0].vy;
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
    D_800A7680[0].vy = sToolSave.unk44;
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

/** @brief Moves the game position `dist` units along heading `deg`
 *         (degrees) relative to the yaw in the first rotation. */
void func_80033A08(s32 deg, s32 dist) {
    s32 ang;
    SVECTOR *rot;

    rot = D_800A7680;
    ang = (deg << 12) / 360;
    sGameSave.unk348[0] -= rsin(rot->vy + ang) * dist >> 12;
    sGameSave.unk348[2] -= rcos(rot->vy + ang) * dist >> 12;
}

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

/** @brief Runs an update; when the highlighted line moved, loads the
 *         edited value and its limit for the new line, else saves the
 *         edited value for the current one. */
void func_80033B34(void) {
    s16 old;

    old = D_8009574A;
    func_800330D4();
    if (old != D_8009574A) {
        switch (D_8009574A) {
            case 0:
                D_800958B0 = 200;
                D_80095748 = sTotals.unk12;
                break;
            case 1:
                D_800958B0 = 3;
                D_80095748 = sTotals.unk14;
                break;
        }
    } else {
        switch (old) {
            case 0:
                sTotals.unk12 = D_80095748;
                break;
            case 1:
                sTotals.unk14 = D_80095748;
                break;
        }
    }
}

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

/** @brief Prints a two-line YES/NO menu, highlighting the line
 *         the menu cursor selects. */
void func_80033C90(void) {
    FntPrint(D_800955DC);
    func_80014BF0(4);
    if (D_8009574A == 0) {
        FntPrint(D_800954F4);
    } else {
        FntPrint(D_8009550C);
    }
    FntPrint(D_80095628);
    func_80014BF0(4);
    if (D_8009574A == 1) {
        FntPrint(D_800954F4);
    } else {
        FntPrint(D_8009550C);
    }
    FntPrint(D_80095630);
}

/** @brief Prints a two-line SAVE/LOAD menu, highlighting the line
 *         the menu cursor selects. */
void func_80033D3C(void) {
    FntPrint(D_800955DC);
    func_80014BF0(4);
    if (D_8009574A == 0) {
        FntPrint(D_800954F4);
    } else {
        FntPrint(D_8009550C);
    }
    FntPrint(D_80095638);
    func_80014BF0(4);
    if (D_8009574A == 1) {
        FntPrint(D_800954F4);
    } else {
        FntPrint(D_8009550C);
    }
    FntPrint(D_80095640);
}

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

/** @brief Wraps the edited value and the highlighted line into their
 *         ranges: below 0 to the top, at or past the limit to 0.
 *  @return 0. */
s32 func_80033E98(void) {
    /* MATCHING: retail reads the edited value signed (lh); common.h's
     * declaration is u16. */
    if ((s16)D_80095748 < 0) {
        D_80095748 = D_800958B0 - 1;
    }
    if ((s16)D_80095748 >= D_800958B0) {
        D_80095748 = 0;
    }
    if (D_8009574A < 0) {
        D_8009574A = D_800958B2 - 1;
    }
    if (D_8009574A >= D_800958B2) {
        D_8009574A = 0;
    }
    return 0;
}
