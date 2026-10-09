#include "common.h"
#include "memory.h"

/** @brief A 0x5C-byte record of a 200-entry table; only byte 0 is known. */
typedef struct {
    s8 unk0;       /**< -1 when the record is free (a guess) */
    u8 unk1[0x5B]; /**< not yet known */
} Rec5C;

/** @brief A 0x3C-byte record of a 100-entry table; only the halfword at 0 is
 *         known. */
typedef struct {
    s16 unk0;      /**< -1 when the record is free (a guess) */
    u8 unk2[0x3A]; /**< not yet known */
} Rec3C;

/** @brief A 0x48-byte record of a 200-entry table; a few fields known. */
typedef struct {
    u8 unk0[0x24]; /**< not yet known */
    s16 unk24;     /**< zeroed with unk40 */
    u8 unk26[0xE]; /**< not yet known */
    s16 unk34;     /**< -1 when reset */
    s16 unk36;     /**< -1 when reset */
    s16 unk38;     /**< -1 when fully reset */
    u8 unk3A[6];   /**< not yet known */
    u8 unk40;      /**< cleared with unk24 */
    u8 unk41;      /**< 1 when reset; only bit 0 survives a clear */
    u8 unk42[6];   /**< not yet known */
} Rec48;

/** @brief An object whose current position and halfword triple are reset
 *         from a stored copy. */
typedef struct {
    s32 unk0;    /**< current; reset from unkC */
    s32 unk4;    /**< current; reset from unk10 */
    s32 unk8;    /**< current; reset from unk14 */
    s32 unkC;    /**< stored */
    s32 unk10;   /**< stored */
    s32 unk14;   /**< stored */
    u16 unk18;   /**< current; reset from unk1E */
    u16 unk1A;   /**< current; reset from unk20 */
    u16 unk1C;   /**< current; reset from unk22 */
    u16 unk1E;   /**< stored */
    u16 unk20;   /**< stored */
    u16 unk22;   /**< stored */
    u8 unk24[8]; /**< not yet known */
    s32 unk2C;   /**< zeroed on a reset */
    s32 unk30;   /**< zeroed on a reset */
} Obj34;

/** @brief An object with a word at 0x28 that a lookup updates. */
typedef struct {
    u8 unk0[0x28]; /**< not yet known */
    s32 unk28;     /**< passed to the lookup and updated from it */
} Obj2C;

/** @brief Three words, a position. */
typedef struct {
    s32 x; /**< x */
    s32 y; /**< y */
    s32 z; /**< z */
} Vec3;

/** @brief The 0x30-byte argument block of a position query. */
typedef struct {
    Vec3 pos;       /**< the position asked about */
    u8 unkC[8];     /**< not set by the range-50 caller */
    s32 unk14;      /**< 50 from the range-50 caller (a range?) */
    u8 unk18;       /**< 0 from the range-50 caller */
    u8 unk19[0x17]; /**< not set by the range-50 caller */
} Query30;

/** @brief Eight bytes of four halfwords, copied whole. */
typedef struct {
    s16 v[4]; /**< not yet known */
} Quad16;

/** @brief The header of a block whose second part starts at a byte offset
 *         the header gives. */
typedef struct {
    s32 unk0;   /**< not yet known; kept in a global */
    s32 offset; /**< byte offset of the second part from the header */
} BlockHeader;

extern u8 *D_800959C0;      /**< the bytes after a BlockHeader */
extern u8 *D_800959C4;      /**< the BlockHeader's second part */
extern s32 D_800959C8;      /**< the BlockHeader's first word */
extern u8 D_800A74D0[];     /**< 128 byte flags; cleared together */
extern u16 D_80095748;      /**< a halfword copied into the run below */
extern u16 D_80095B4C[];    /**< first of a run of halfwords */
extern u8 D_80096738[];     /**< passed to the lookup */
extern Quad16 D_800DD0A0[]; /**< a table of eight-byte entries */
extern Rec3C D_800A7898[];  /**< 100 Rec3C records */

s32 func_80018D70(void *pos, void *arg, s32 cur);
s32 func_80028AE4(Query30 *q);
/* MATCHING: s32, though the callee returns a sign-extended s16: retail
 * stores the result with no re-extension. */
s32 func_80018D04(s16 a, s16 b, u16 t, u16 n);
void func_800330D4(void);
void func_800337E4(u8 *buf);
void func_8003390C(Rec3C *recs);

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_80029898);

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_80029930);

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_800299D8);

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_80029E74);

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_8002A328);

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_8002A558);

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_8002A5B0);

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_8002A7D8);

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_8002A98C);

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_8002AA58);

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_8002AEB8);

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_8002AF6C);

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_8002B04C);

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_8002B220);

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_8002B5FC);

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_8002B7C8);

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_8002B8F8);

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_8002BC4C);

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_8002BD00);

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_8002BEC0);

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_8002C044);

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_8002C0EC);

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_8002C188);

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_8002C20C);

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_8002C2B4);

/** @brief Updates `p->unk28` from a lookup unless the lookup fails (-1). */
void func_8002C438(Obj2C *p) {
    s32 v;

    v = func_80018D70(p, D_80096738, p->unk28);
    if (v != -1) {
        p->unk28 = v;
    }
}

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_8002C47C);

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_8002C4D8);

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_8002C540);

/** @brief Clears byte 0 of the first `count` records of a Rec5C table. */
void func_8002C5A4(Rec5C *recs, u16 count) {
    u16 i;

    for (i = 0; i < count; i++) {
        recs->unk0 = 0;
        recs++;
    }
}

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_8002C5D0);

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_8002C650);

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_8002C6A4);

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_8002C724);

/** @brief Resets an object's current values from its stored copy. */
void func_8002C820(Obj34 *p) {
    p->unk2C = 0;
    p->unk30 = 0;
    p->unk0 = p->unkC;
    p->unk4 = p->unk10;
    p->unk8 = p->unk14;
    p->unk18 = p->unk1E;
    p->unk1A = p->unk20;
    p->unk1C = p->unk22;
}

/** @brief Copies entry `i` of the eight-byte table to `out`. */
void func_8002C85C(u16 i, Quad16 *out) {
    Quad16 *src;

    /* MATCHING: the base in its own local, then advanced by i; indexing
     * gives the sum the index register. */
    src = D_800DD0A0;
    src += i;
    *out = *src;
}

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_8002C894);

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_8002C994);

/** @brief Interpolates from 0 towards `b` by `t`/`n`, into out[1]. */
void func_8002CAA4(s16 b, u16 t, u16 n, s32 *out) {
    out[1] = func_80018D04(0, b, t, n);
}

/** @brief Interpolates from `a` towards `b` by `t`/`n`, into out[1]. */
void func_8002CAE4(s16 a, s16 b, u16 t, u16 n, s32 *out) {
    out[1] = func_80018D04(a, b, t, n);
}

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_8002CB24);

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_8002CC24);

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_8002CCEC);

/** @brief Points the current-block globals at the block `hdr` heads. */
void func_8002D0C4(BlockHeader *hdr) {
    D_800959C0 = (u8 *)hdr + 8;
    D_800959C8 = hdr->unk0;
    /* MATCHING: offset read through a byte pointer, not hdr->offset: the
     * member access lets the load rise above the D_800959C8 store. */
    D_800959C4 = (u8 *)hdr + *(s32 *)((u8 *)hdr + 4);
    D_800958E8 = 0;
}

/** @brief Runs the position query for `pos` with a range of 50. */
s8 func_8002D0F0(Vec3 *pos) {
    Query30 q;

    q.pos.x = pos->x;
    q.pos.y = pos->y;
    q.pos.z = pos->z;
    q.unk18 = 0;
    q.unk14 = 50;
    return func_80028AE4(&q);
}

/** @brief Clears a 128-byte table of flags.
 *  @return nothing; the value is undefined. */
s32 func_8002D140(void) {
    u32 i;

    /* MATCHING: non-void with no return; it keeps $v0 live at the exit, so
     * the loop's delay slot stays a nop. */
    for (i = 0; i < 128; i++) {
        D_800A74D0[i] = 0;
    }
}

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_8002D16C);

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_8002D1CC);

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_8002D230);

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_8002D2C0);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_80010B7C);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_80010B8C);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_80010BA4);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_80010BB8);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_80010BCC);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_80010BD8);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_80010BE4);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_80010BF0);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_80010BFC);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_80010C08);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_80010C14);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_80010C20);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_80010C2C);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_80010C38);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_80010C48);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_80010C54);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_80010C68);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_80010C78);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_80010C88);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_80010C94);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_80010CA4);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_80010CB0);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_80010CC4);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_80010CD4);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_80010CE4);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_80010CF0);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_80010D04);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_80010D18);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_80010D24);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_80010D30);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_80010D3C);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_80010D54);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_80010D64);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_80010D74);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_80010D84);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_80010D9C);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_80010DAC);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_80010DC4);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_80010DD0);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_80010DDC);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_80010DE8);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_80010DF4);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_80010E00);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_80010E0C);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_80010E18);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_80010E28);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_80010E3C);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_80010E50);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_80010E60);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_80010E74);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_80010E8C);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_80010EA4);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_80010EBC);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_80010EC8);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_80010EE0);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_80010EEC);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_80010F00);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_80010F0C);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_80010F24);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_80010F38);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_80010F50);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_80010F60);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_80010F6C);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_80010F80);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_80010F98);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_80010FAC);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_80010FBC);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_80010FCC);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_80010FDC);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_80010FEC);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_80010FFC);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_80011010);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_80011020);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_8001102C);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_8001103C);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_80011048);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_80011054);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_80011064);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_80011074);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_80011080);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_8001108C);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_80011098);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_800110A4);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_800110B0);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_800110BC);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_800110C8);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_800110D4);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_800110E0);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_800110EC);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_800110F8);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_80011104);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_80011114);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_80011124);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_80011134);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_80011144);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_80011158);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_80011168);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_80011178);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_8001118C);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_8001119C);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_800111B4);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_800111C0);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_800111D4);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_800111EC);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_800111FC);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_80011208);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_8001121C);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_8001122C);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_8001123C);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_80011248);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_80011254);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_80011260);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_80011284);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_800112A8);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_800112CC);

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_8002D424);

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_8002DC44);

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_8002F270);

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_8002F6A0);

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_8002F8FC);

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_8002FA78);

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_8002FDB4);

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_8002FF74);

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_80030278);

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_80030548);

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_80030984);

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_80030B6C);

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_80030DA0);

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_80031064);

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_8003146C);

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_800317D0);

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_80031A48);

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_80031AEC);

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_80031EF4);

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_8003245C);

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_800327BC);

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_80032964);

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_80032C28);

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_80032EE4);

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_800330D4);

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_80033224);

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_80033388);

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_8003356C);

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_800335E8);

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_80033680);

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_800336F8);

/** @brief Clears and sets up a fixed 0x800-byte block near the top of RAM,
 *         then clears and resets the Rec3C table. */
void func_80033790(void) {
    bzero((u8 *)0x801FD000, 0x800);
    func_800337E4((u8 *)0x801FD000);
    bzero((u8 *)D_800A7898, sizeof(Rec3C) * 100);
    func_8003390C(D_800A7898);
}

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_800337E4);

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

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_80033930);

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_8003399C);

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_80033A08);

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_80033AB8);

/** @brief Runs an update, then latches a halfword into the first slot of a
 *         halfword run. */
void func_80033B08(void) {
    func_800330D4();
    D_80095B4C[0] = D_80095748;
}

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_80033B34);

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_80033BF8);

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_80033C90);

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_80033D3C);

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_80033DE8);

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_80033E40);

INCLUDE_ASM("asm/nonmatchings/code_1a098", func_80033E98);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_800114DC);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_800114E8);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_800114F4);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_80011500);

INCLUDE_RODATA("asm/nonmatchings/code_1a098", D_8001151C);
