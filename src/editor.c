#include "common.h"
#include "libetc.h"
#include "memory.h"
#include "libapi.h"
#include "libgte.h"
#include "libgpu.h"
#include "sys/file.h"
#include "object.h"
#include "player.h"
#include "draw.h"
#include "model.h"
#include "collision.h"

/** @brief A stage sprite (gStageSprites); only the halfword at 0 is
 *         known. */
typedef struct {
    s16 unk0;     /**< -1 when the record is free (a guess) */
    u8 unk2[2];   /**< not yet known */
    s32 unk4[3];  /**< a position */
    s32 unk10[3]; /**< a copy of unk4 */
    s16 unk1C;    /**< a height from the template's entry */
    s16 unk1E;    /**< a second height from the template's entry */
    s32 unk20;    /**< a copy of unk1C */
    u8 unk24[2];  /**< not yet known */
    u8 unk26;     /**< bit 7 picks one of two handlers */
    u8 unk27;     /**< sTotals.unk6C when placed */
    u8 unk28;     /**< sTotals.unk6D when placed */
    u8 unk29;     /**< not yet known */
    u16 unk2A;    /**< sTotals.unk6A when placed */
    s16 unk2C;    /**< matched against sTotals.unk2A */
    u8 unk2E[2];  /**< not yet known */
    s32 unk30;    /**< a global stamp when placed */
    s32 unk34;    /**< the current entry when placed */
    u8 unk38[4];  /**< not yet known */
} StageSprite;

/** @brief The tool state block: counts, totals and saved menu values. */
typedef struct {
    u8 unk0[3];  /**< not yet known */
    u8 unk3;     /**< the menu line the tool mode was entered from */
    s32 unk4;    /**< a step: the move distance and the height step */
    s32 unk8;    /**< a height offset added to the ground height */
    s32 unkC;    /**< a height offset added to the camera's y */
    u8 unk10[2]; /**< not yet known */
    u16 unk12;   /**< the edited value saved for menu line 0 */
    u16 unk14;   /**< the edited value saved for menu line 1 */
    u16 unk16;   /**< number of switches in use */
    u16 unk18;   /**< number of EditorTrap records in use */
    u16 unk1A;   /**< number of EditorTrap records counted live */
    u16 unk1C;   /**< unk38 of the first record placed */
    u16 unk1E;   /**< matched against a Trap's unk34 */
    s16 unk20;   /**< an angle in degrees, wrapped to 0..359 */
    u16 unk22;   /**< the selected point of the current entry */
    u16 canGroup; /**< the group the next can gets, less one; the edited value saved for tool mode 2 */
    u16 unk26;    /**< a sum over the current block's entries */
    u16 unk28;    /**< the StageSprite slot the next record goes to */
    u16 unk2A;    /**< matched against a StageSprite's unk2C */
    u16 unk2C;    /**< a StageSprite kind, less 30 */
    u16 unk2E;    /**< a height base for the StageSprite template */
    u16 unk30;    /**< number of StageSprite records in use */
    u16 unk32;    /**< picks one of two StageSprite handlers */
    u16 unk34;    /**< negates the StageSprite template's height when set */
    u8 unk36[0x32]; /**< not yet known */
    u8 unk68;       /**< copied into a placed record's unk42 */
    u8 unk69;       /**< copied into a placed record's unk43 */
    s16 unk6A;      /**< an angle in degrees; copied into a placed StageSprite's unk2A */
    u8 unk6C;       /**< copied into a placed StageSprite's unk27 */
    u8 unk6D;       /**< copied into a placed StageSprite's unk28 */
    u8 unk6E[2];    /**< not yet known */
    s32 unk70;      /**< the global stamp, saved on leaving the tool */
} EditorState;

/** @brief 64 KiB of the tool buffer, copied whole. */
typedef struct {
    u8 b[0x10000]; /**< not yet known */
} Page64K;

/** @brief The tool state block, copied whole. */
typedef struct {
    s32 w[0x74 / 4]; /**< the block */
} EditorStateCopy;

/** @brief The 200 traps, copied whole. */
typedef struct {
    s32 w[0x3840 / 4]; /**< the records */
} TrapTable;

/** @brief The 200 switches, copied whole. */
typedef struct {
    s32 w[0x47E0 / 4]; /**< the records */
} SwitchTable;

/** @brief The 100 stage sprites, copied whole. */
typedef struct {
    s32 w[0x1770 / 4]; /**< the records */
} SpriteTable;

/** @brief The block header area, copied whole; bytes, so a copy of it
 *         tests the alignment at run time. */
typedef struct {
    u8 b[0x800]; /**< the area */
} BlockAreaCopy;

/** @brief A 0xB774-byte slot of the tool buffer, seen 0x200 bytes early:
 *         the slot's data starts at unk200 and its block header area runs
 *         0x200 bytes past this view's end. */
typedef struct {
    u8 unk0[0x200];      /**< not yet known */
    u8 unk200;           /**< 0x38 when the slot is valid */
    u8 unk201;           /**< not yet known */
    u8 unk202;           /**< the owner; compared with a global */
    u8 unk203[0x71];     /**< the rest of the tool state block */
    TrapTable unk274;    /**< the EditorTrap table */
    SwitchTable unk3AB4; /**< the switches */
    SpriteTable unk8294; /**< the StageSprite table */
    u8 unk9A04[0x1D70];  /**< not yet known; the block area starts at 0xB174 */
} SaveSlot;

/* MATCHING: gStageSprites and gEditorState are also declared in object, each
 * unit through its own view (StageSprite, EditorState here; StageSprite, Progress6E there). */
extern StageSprite gStageSprites[]; /**< 100 StageSprite records */
extern u8 gEditorState[];           /**< the editor's state, an EditorState */
extern char sEdTmpFilePath[]; /**< path of the tool file, "sim:\\PS\\PEPSI\\DATA\\TOOL0\\TMP.TL0" */
#define sTotals (*(EditorState *)gEditorState)
extern char sEdNewlines2[];           /**< "\n\n" */
extern char sEdColorWhite[];          /**< colour code of a highlighted menu line, "~c888" */
extern char sEdColorBlue[];           /**< colour code of a plain menu line, "~c008" */
extern char sEdYesText[];             /**< "  YES\n" */
extern char sEdNoText[];              /**< "  NO" */
extern char sEdSaveText[];            /**< "  SAVE\n" */
extern char sEdLoadText[];            /**< "  LOAD\n" */
extern char sEdDataNoFmt[];           /**< "DATA:%d " */
extern char sEdStageFmt[];            /**< "STAGE %d-" */
extern char sEdNoDataText[];          /**< "NO DATA\n" */
extern char sEdArea1Text[];           /**< "1" */
extern char sEdArea2Text[];           /**< "2" */
extern char sEdBossText[];            /**< "BOSS" */
extern char sEdNewline[];             /**< "\n" */
extern char sEdCanNoFmt[];            /**< "\n\nCAN NO.  %d\n" */
extern char sEdCanNumFmt[];           /**< "CAN NUM. %d\n" */
extern char sEdInfoSwitchNoFmt[];     /**< "\n\nSWITCH NO.%d\n" */
extern char sEdInfoTrapNoFmt[];       /**< "\n\nTRAP NO. %d / " */
extern char sEdInfoTrapModelFmt[];    /**< "MODEL.%2d / " */
extern char sEdOptionsText[];         /**< "OPTIONS  / " */
extern char sEdInfoTrapSwitchFmt[];   /**< "SWITCH NO.%d\n" */
extern char sEdInfoTypeFmt[];         /**< "TYPE. %d / " */
extern char sEdInfoActFmt[];          /**< "ACT. %d\n" */
extern char sEdInfoSpriteFmt[];       /**< "\n\nSPRITE. %d / " */
extern char sEdInfoSpriteModelFmt[];  /**< "MODEL. %d / " */
extern char sEdInfoSpriteSwitchFmt[]; /**< "SWITCH. %d\n" */
extern char sEdNoActText[];           /**< "NO ACT  / " */
extern char sEdInfoNearActText[];     /**< "NEAR ACT/ " */
extern char sEdInfoHitActText[];      /**< "HIT ACT / " */
extern char sEdInfoWalkTypeFmt[];     /**< "TYPE %d\n" */
extern char sEdNoSeText[];            /**< "NO SE / " */
extern char sEdManText[];             /**< "MAN   / " */
extern char sEdWomanText[];           /**< "WOMAN / " */
extern char sEdChildText[];           /**< "CHILD / " */
extern char sEdDogText[];             /**< "DOG   / " */
extern char sEdInfoHitLabel[];        /**< "HIT. " */
extern char sEdInfoOffText[];         /**< " OFF / " */
extern char sEdInfoOnText[];          /**< " ON  / " */
extern char sEdInfoActLabel[];        /**< "ACT. " */
extern char sEdInfoWalkLabel[];       /**< "WALK." */
extern char sEdInfoOffLine[];         /**< " OFF\n" */
extern char sEdInfoSeLabel[];         /**< "SE. " */

/** @brief The tool state block, seen as the save area past its totals. */
typedef struct {
    u8 unk0[0x38]; /**< not yet known */
    s32 unk38[3];  /**< a copy of &gPlayer.x */
    s16 unk44;     /**< a saved halfword of game state */
    u8 unk46[2];   /**< not yet known */
    s32 unk48[6];  /**< six saved words of game state */
} ToolSave;

#define sToolSave (*(ToolSave *)gEditorState)


/* MATCHING: also declared in object, which only zeroes it. */
extern s32 gCurBlock;   /**< the stage block the player is on, -1 for none */
extern s32 gStickTrigX; /**< a pad word; bits step the edited value */

s32 WrapMenuValues(void);

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
#define sCur (*(BlockCur *)&gCanSpans)

/** @brief A trap (gTraps), as
 *         this unit places it (object.h's Trap is the same record). */
typedef struct {
    s32 unk0[3]; /**< a position */
    s32 unkC[3]; /**< a copy of unk0 */
    s16 unk18;   /**< cleared when the first record is taken */
    s16 unk1A;   /**< a heading, 0..4095 */
    s16 unk1C;   /**< cleared when the first record is taken */
    u8 unk1E[2]; /**< not yet known */
    u16 unk20;   /**< a heading offset from the first record's */
    u8 unk22[2]; /**< not yet known */
    s16 unk24;   /**< cleared when placed */
    s16 unk26;   /**< cleared when placed */
    s32 unk28;   /**< the current entry when placed */
    s32 unk2C;   /**< an angle from the light direction */
    s32 unk30;   /**< a second angle from the light direction */
    s16 unk34;   /**< sTotals.unk1E when placed */
    s16 unk36;   /**< -1 when free */
    s16 unk38;   /**< sTotals.unk1C for the first placed, else -1 */
    u8 unk3A[2]; /**< not yet known */
    s32 unk3C;   /**< a global stamp when placed */
    u8 unk40;    /**< cleared when placed */
    u8 unk41;    /**< not yet known */
    u8 unk42;    /**< sTotals.unk68 when placed */
    u8 unk43;    /**< sTotals.unk69 when placed */
    u8 unk44[4]; /**< not yet known */
} EditorTrap;

/* MATCHING: object declares D_800959D8 as u8 (it only zeroes it); this
 * unit tests it as s8. */
extern EditorTrap gPlaceQueue[]; /**< records waiting to be placed */
extern u16 gPlaceCount;          /**< number of records waiting */
extern s8 D_800959D8;            /**< a flag; cleared after placing when 1 */

/* MATCHING: per-unit views while StageSprite is unit-local; the two handlers
 * take an s32 id here (passed unextended), u16 and s16 in object. */
void SaveEditorSlot(s16 a, u8 *buf);
void LoadEditorSlot(s16 a, u8 *buf);
void BuildEmptyBlock(u8 *buf);
void ClearStageSprites(StageSprite *recs);
void StepSprite(s32 id, StageSprite *r);
void DrawObjectSprite(s32 id, StageSprite *r);


/* MATCHING: declared per unit (object and stage_flow give their own
 * views); here the start position is passed as words. */
s32 FindFloor(s32 *pos);
extern u8 D_800958D8; /**< a flag set by tool modes 4 and 5 */

/* MATCHING: object declares it as its SwitchBox records. */
extern s32 gSwitches[]; /**< the 200 switches */

/** @brief A switch (gSwitches), as this unit
 *         places it. */
typedef struct {
    s8 unk0;       /**< -1 when the record is free */
    u8 unk1[0x4F]; /**< not yet known */
    s32 unk50[3];  /**< a position */
} EditorSwitch;

#define sSwitches ((EditorSwitch *)gSwitches)

/** @brief A width and a depth. */
typedef struct {
    s16 w; /**< the width */
    s16 d; /**< the depth */
} Size2;

/** @brief Three sizes, copied whole. */
typedef struct {
    Size2 s[3]; /**< the sizes */
} Sizes3;

extern Sizes3 sBoxSizes; /**< the three box sizes, 2000, 1000 and 4000 */

/* MATCHING: object defines it on its Box4. */
void SetBoxCentred(EditorSwitch *b, s16 w, s16 d);

/* MATCHING: stage_flow defines it with an s16 first parameter; this unit
 * passes it unextended and hands it two record pointers. */
u16 CallStageHook(s32 id, EditorTrap *a, EditorTrap *b);

/* MATCHING: defined in player and hit_tool, which have no header. */
void TransformByCoord(GsCOORDINATE2 *coord, SVECTOR *pos, VECTOR *out);
void DrawPin(VECTOR *pos);

void MoveGamePos(s32 deg, s32 dist);

void PrintEditorSlots(void);
extern char sEdNewlines3[];           /**< "\n\n\n" */
extern char sEdCheckText[];           /**< "CHECK" */
extern char sEdDataModeText[];        /**< "DATA MODE" */
extern char sEdModeSelectText[];      /**< "  MODE SELECT\n\n" */
extern char sEdBufferSelectText[];    /**< "BUFFER SELECT\n\n" */
extern char sEdCreateFileText[];      /**< "  CREATE FILE" */
extern char sEdCardFormatText[];      /**< "  CARD FORMAT" */
extern char sEdSaveToDataFmt[];       /**< "SAVE TO DATA %d\n\n" */
extern char sEdOverwriteOkText[];     /**< "OVER WRITE OK ?" */
extern char sEdDataWritingText[];     /**< "DATA WRITING..." */
extern char sEdLoadFromDataFmt[];     /**< "LOAD TO DATA %d\n\n" */
extern char sEdLoadOkText[];          /**< "    LOAD OK ?" */
extern char sEdCantOpenDataFmt[];     /**< "CAN NOT OPEN DATA %d" */
extern char sEdCompleteText[];        /**< "   COMPLATE" */
extern char sEdErrorText[];           /**< "    !!!! ERROR!!!!" */
extern char sEdNotEnoughMemoryText[]; /**< "   NOT ENOUGH MEMORY" */
extern char sEdNoCardText[];          /**< "NO CONNECT CARD" */
extern char sEdNoFileText[];          /**< "NO FILE\n\n" */
extern char sEdCreateNewFileText[];   /**< "CREATE NEW FILE ?" */
extern char sEdUnformattedText[];     /**< "    UN FORMAT\n\n" */
extern char sEdFormatOkText[];        /**< "   FORMAT OK ?" */

/* Defined in other units, without a header. */
/* MATCHING: object defines it with no parameter; called with an OT. */
void UpdatePickups(GsOT *ot);
void ResetStageObjects(void);
void DrawSky(void);
void DrawStage(s16 x, s16 y, s16 z);
void UpdateCamera(s16 x, s16 y, s16 z, s16 w);

s32 UpdateEditorMenu(void);
void SetEditorCamera(void);
s32 EditorFreeMove(void);
void EditorInsertPoint(void);
s32 EditorEditPoints(void);
s32 EditorPlaceBox(void);
s32 EditorPickBox(void);
void EditorPlaceRecords(void);
s32 EditorEditRecordValues(void);
s32 EditorArrangeRecords(void);
s32 EditorPickRecord(void);
void EditorEditSpriteValues(void);
s32 EditorPlaceSprite(void);
void EditorRunSprites(void);
s32 EditorPickSprite(void);
s32 UpdateEditorData(void);
void DrawPole(VECTOR *pos);
void BackupToolBuffer(void);
void TotalBlockEntries(void);
void func_80033B08(void);
void RunValueMenu(void);
void ApplyRecordsToEntries(void);
s32 PrintEditorStatus(void);
/* MATCHING: player tests it as s8. */
extern u8 gToolExited; /**< set on leaving the tool */

extern StageSprite sSpriteTemplate; /**< the StageSprite template that gets placed */
extern u8 sSpriteFlags;             /**< or-ed into a placed StageSprite's unk26 */

/** @brief Eight bytes, copied whole. */
typedef struct {
    u8 b[8]; /**< the bytes */
} Bytes8;

/* MATCHING: object defines it as (u16, Quad16 *); this unit passes the
 * index unextended. */
void GetQuad16(s32 i, Bytes8 *out);
void DrawHeadingLine(VECTOR *pos, s16 deg);

/* MATCHING: defined in collision, which has no header. */

/* MATCHING: defined in card, which has no header. */
s32 RunCardOp1(s16 mode);
/* MATCHING: s16 in card; its result is compared here unextended. */
s32 CheckCard(void);
s32 WriteEditorSlotFiles(void);
s32 CheckEditorSlot(s16 i);
void SaveEditorState(s16 a);
void RestoreEditorState(s16 a);
s32 WriteEditorFile(void);
s32 ReadEditorFile(void);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdPositionFmt);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdSwitchNoFmt);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdTrapNoFmt);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdPepsiNoFmt);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdItemText);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdPepsiFmt);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdSwitchFmt);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdTrapFmt);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdSpriteFmt);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdInfoText);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdDeleteText);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdMoveModeText);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdMenuModeText);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdPepsiSelModeText);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdNewlines9);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdPepsiTypeSelText);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdPepsiTypeRangeFmt);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdSwitchSelModeText);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdNewlines8);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdSwitchSelText);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdSwitchNoRangeText);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdSwitchTypeRangeFmt);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdTrapSelModeText);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdTrapSelText);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdTrapNoRangeText);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdTrapModelRangeFmt);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdUseBufferFmt);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdTrapSwitchRangeText);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdTrapTypeRangeText);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdTrapActRangeText);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdAngleRangeFmt);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdSpriteSelModeText);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdSpriteSelText);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdSpriteNoRangeText);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdSpriteModelRangeFmt);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdSpriteSwitchRangeText);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdSpriteSizeRangeFmt);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdHitLabel);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdFlipLabel);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdActLabel);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdNearActText);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdHitActText);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdWalkLabel);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdSeLabel);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdWarningText);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdCansTrapsLine);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdSwitchesCantPlaceLine);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdGroundHitLine);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdUntakeableHereLine);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdPushCrossText);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdBufferMaxLine);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdReachedNoMoreLine);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdCantPlaceLine);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdAskMiyagoeLine);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdPleaseLine);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdSwitchesDeleteLine);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdCannotLine);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdBufferZeroLine);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdCantDeleteLine);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdAreaDeleteLine);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdDeletableLine);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdThereAreNoneLine);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdTrapsSwitchesLine);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdNotNearbyLine);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdWhatToEraseLine);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdMoveNearLine);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdPleaseDoLine);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdCanNoFmt);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdCanNumFmt);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdInfoSwitchNoFmt);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdInfoTrapNoFmt);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdInfoTrapModelFmt);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdOptionsText);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdInfoTrapSwitchFmt);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdInfoTypeFmt);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdInfoActFmt);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdInfoSpriteFmt);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdInfoSpriteModelFmt);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdInfoSpriteSwitchFmt);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdNoActText);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdInfoNearActText);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdInfoHitActText);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdInfoWalkTypeFmt);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdNoSeText);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdManText);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdWomanText);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdChildText);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdDogText);

INCLUDE_RODATA("asm/nonmatchings/editor", sBoxSizes);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdDataModeText);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdModeSelectText);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdBufferSelectText);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdCreateFileText);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdCardFormatText);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdSaveToDataFmt);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdOverwriteOkText);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdDataWritingText);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdLoadFromDataFmt);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdLoadOkText);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdCantOpenDataFmt);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdCompleteText);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdErrorText);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdNotEnoughMemoryText);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdNoCardText);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdNoFileText);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdCreateNewFileText);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdUnformattedText);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdFormatOkText);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdDataNoFmt);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdStageFmt);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdNoDataText);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdTmpFilePath);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdDataFilePath0);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdDataFilePath1);

INCLUDE_RODATA("asm/nonmatchings/editor", sEdDataFilePath2);

/** @brief The tool's main step: sets the tool up on the first pass;
 *         then runs the current tool mode's editor, the camera and the
 *         view, and handles the flag bits that leave the tool and step
 *         the move distance.
 *  @return nothing; the value is undefined. */
s32 UpdateEditor(void) {
    /* MATCHING: an unused 16-byte local gives retail's 0x30 frame. */
    s32 unused[4];
    EditorTrap *r;
    s32 i;

    /* MATCHING: non-void with no return keeps the move-distance test's
     * delay slot for the constant 100. */
    switch (gModeStep) {
        case 0:
            ResetGameState((Trap *)0x8016D000);
            gToolMode = 0;
            ResetStageOverlay();
            gRepeatButtonsOn = 1;
            gModeStep++;
            break;
        case 1:
            TotalBlockEntries();
            for (i = 1; i < 128; i++) {
                gCanGroupOn[i] = 1;
            }
            ShowRevealedCans();
            /* MATCHING: the s16 error code switched as u16 (lhu). */
            switch ((u16)gToolMode) {
                case 0:
                    EditorFreeMove();
                    if (D_800958D8 == 0) {
                        switch (sTotals.unk3) {
                            case 0:
                                EditorInsertPoint();
                                break;
                            case 1:
                                EditorPlaceBox();
                                TestBoxes();
                                break;
                            case 2:
                                EditorPlaceRecords();
                                EditorArrangeRecords();
                                break;
                            case 3:
                                EditorPlaceSprite();
                                break;
                        }
                    } else {
                        switch (sTotals.unk3) {
                            case 0:
                                EditorEditPoints();
                                break;
                            case 1:
                                EditorPickBox();
                                TestBoxes();
                                break;
                            case 2:
                                EditorPickRecord();
                                if (D_800959D8 == 1 && (gPadTrig & PADRright)) {
                                    gPlayer.x = ((EditorTrap *)gTraps)[sTotals.unk18].unk0[0];
                                    gPlayer.y = ((EditorTrap *)gTraps)[sTotals.unk18].unk0[1];
                                    gPlayer.z = ((EditorTrap *)gTraps)[sTotals.unk18].unk0[2];
                                    sTotals.unk1C = ((EditorTrap *)gTraps)[sTotals.unk18].unk38;
                                    gToolMode = 4;
                                    sTotals.unk1E = ((EditorTrap *)gTraps)[sTotals.unk18].unk34;
                                    sTotals.unk68 = ((EditorTrap *)gTraps)[sTotals.unk18].unk42;
                                    sTotals.unk69 = ((EditorTrap *)gTraps)[sTotals.unk18].unk43;
                                    sTotals.unk20 =
                                        ((EditorTrap *)gTraps)[sTotals.unk18].unk1A * 360 / 4096;
                                    gPlaceCount = CallStageHook(sTotals.unk1C, gPlaceQueue, gPlaceQueue);
                                    for (i = 1; i < gPlaceCount; i++) {
                                        /* MATCHING: base taken in the body */
                                        r = (EditorTrap *)gTraps;
                                        r[sTotals.unk18 + i].unk36 = -1;
                                    }
                                    gEditMax = 200;
                                    D_800958D8 = 0;
                                    gMenuLineCount = 5;
                                    gMenuLine = 0;
                                    gSubStep = 0;
                                    gEditValue = sTotals.unk18;
                                }
                                break;
                            case 3:
                                EditorPickSprite();
                                if (D_800959D8 == 1 && (gPadTrig & PADRright)) {
                                    gPlayer.x = gStageSprites[sTotals.unk28].unk4[0];
                                    gPlayer.y = gStageSprites[sTotals.unk28].unk4[1];
                                    gPlayer.z = gStageSprites[sTotals.unk28].unk4[2];
                                    sTotals.unk2C = gStageSprites[sTotals.unk28].unk0 - 30;
                                    gToolMode = 5;
                                    sTotals.unk2A = gStageSprites[sTotals.unk28].unk2C;
                                    sTotals.unk6D = gStageSprites[sTotals.unk28].unk28;
                                    sTotals.unk6C = gStageSprites[sTotals.unk28].unk27;
                                    gEditMax = 100;
                                    D_800958D8 = 0;
                                    gMenuLineCount = 9;
                                    gEditValue = sTotals.unk28;
                                    gMenuLine = 0;
                                    gSubStep = 0;
                                    sTotals.unk6A = gStageSprites[sTotals.unk28].unk2A;
                                }
                                break;
                        }
                    }
                    if (gPadTrig & PADRup) {
                        gToolMode = 1;
                        gEditValue = 0;
                        gMenuLine = 0;
                    }
                    break;
                case 1:
                    UpdateEditorMenu();
                    if (gPadTrig & PADRdown) {
                        if (gMenuLine == 2) {
                            gSubStep = 0;
                        }
                        gToolMode = 0;
                    }
                    break;
                case 2:
                    func_80033B08();
                    if (gPadTrig & PADRdown) {
                        gToolMode = 0;
                    }
                    break;
                case 3:
                    RunValueMenu();
                    if (gPadTrig & PADRdown) {
                        gToolMode = 0;
                    }
                    break;
                case 4:
                    EditorEditRecordValues();
                    EditorArrangeRecords();
                    if (gPadTrig & PADRdown) {
                        gToolMode = 0;
                    }
                    break;
                case 5:
                    EditorEditSpriteValues();
                    if (gPadTrig & PADRdown) {
                        gToolMode = 0;
                    }
                    break;
                case 6:
                    UpdateEditorData();
                    break;
                case 7:
                case 8:
                    if (gPadTrig & PADRdown) {
                        gToolMode = 0;
                    }
                    break;
                case 9:
                case 10:
                case 11:
                    if (gPadTrig & PADRdown) {
                        gToolMode = 0;
                        D_800958D8 = 0;
                        D_800959D8 = 0;
                    }
                    break;
            }
            if (D_800958D8 == 0) {
                DrawPin((VECTOR *)&gPlayer.x);
                DrawPole((VECTOR *)&gPlayer.x);
            }
            UpdatePickups(&gSubOt[gActiveBuff]);
            ApplyRecordsToEntries();
            EditorRunSprites();
            DrawStage(-gPlayer.x, 0, -gPlayer.z);
            DrawSky();
            UpdateCamera(gPlayer.x, gPlayer.y, gPlayer.z, gPlayer.groundY);
            SetEditorCamera();
            PrintEditorStatus();
            break;
    }
    if (gPadTrig & PADstart) {
        sTotals.unk70 = gSection;
        BackupToolBuffer();
        gRetries = 0xFF;
        gToolExited = 1;
        ResetStageObjects();
        gModeStep = 3;
        gGameMode = GM_STAGE;
    }
    if (gPadTrig & PADselect) {
        switch (sTotals.unk4) {
            case 50:
                sTotals.unk4 = 100;
                break;
            case 100:
                sTotals.unk4 = 2;
                break;
            case 1000:
                sTotals.unk4 = 10;
                break;
            default:
                sTotals.unk4 = 50;
                break;
        }
    }
}

extern char gSyncFmt[];                 /**< "SYNC %3d" */
extern char gGridFmt[];                 /**< " GRID %4d" */
extern char gNowBlockFmt[];             /**< " NOW BLOCK (%2d/%2d)" */
extern char sEdPositionFmt[];           /**< "\nPOSITION X %d " */
extern char sEdSwitchNoFmt[];           /**< "\nSWITCH NO(%2d/ %2d)" */
extern char sEdTrapNoFmt[];             /**< " TRAP NO(%3d/ %3d)" */
extern char sEdPepsiNoFmt[];            /**< "\nPEPSI NO(%3d/ %3d)" */
extern char sEdItemText[];              /**< " ITEM : " */
extern char sEdPepsiFmt[];              /**< "PEPSI %2d" */
extern char sEdSwitchFmt[];             /**< "SWITCH %2d" */
extern char sEdTrapFmt[];               /**< "TRAP %2d" */
extern char sEdSpriteFmt[];             /**< "SPRITE %2d" */
extern char sEdInfoText[];              /**< "INFO    " */
extern char sEdDeleteText[];            /**< "DELETE  " */
extern char sEdMoveModeText[];          /**< "MOVE MODE" */
extern char sEdMenuModeText[];          /**< "MENU MODE" */
extern char sEdPepsiSelModeText[];      /**< "PEPSI SEL MODE" */
extern char sEdNewlines9[];             /**< nine newlines */
extern char sEdPepsiTypeSelText[];      /**< "PEPSI TYPE SEL\n\n" */
extern char sEdPepsiTypeRangeFmt[];     /**< "   1<-%2d->200" */
extern char sEdSwitchSelModeText[];     /**< "SWITCH SEL MODE" */
extern char sEdNewlines8[];             /**< eight newlines */
extern char sEdSwitchSelText[];         /**< "  SWITCH SEL\n\n" */
extern char sEdSwitchNoRangeText[];     /**< "NO.  0<-" */
extern char sEdSwitchTypeRangeFmt[];    /**< "TYPE. 0<-%1d->%d" */
extern char sEdTrapSelModeText[];       /**< "TRAP SEL MODE" */
extern char sEdTrapSelText[];           /**< "   TRAP  SEL\n\n" */
extern char sEdTrapNoRangeText[];       /**< "NO.    0<-" */
extern char sEdTrapModelRangeFmt[];     /**< "MODEL. 0<-%2d->%2d\n" */
extern char sEdUseBufferFmt[];          /**< "   USE BUFFER %d\n" */
extern char sEdTrapSwitchRangeText[];   /**< "SWITCH.0<-" */
extern char sEdTrapTypeRangeText[];     /**< "TYPENO.0<-" */
extern char sEdTrapActRangeText[];      /**< "ACT NO.0<-" */
extern char sEdAngleRangeFmt[];         /**< "  L1 0<-%3d->360 R1\n" */
extern char sEdSpriteSelModeText[];     /**< "SPR SEL MODE" */
extern char sEdSpriteSelText[];         /**< "   SPRITE SEL\n\n" */
extern char sEdSpriteNoRangeText[];     /**< "  NO.    0<-" */
extern char sEdSpriteModelRangeFmt[];   /**< "  MODEL. 0<-%3d->%3d\n" */
extern char sEdSpriteSwitchRangeText[]; /**< "  SWITCH.0<-" */
extern char sEdSpriteSizeRangeFmt[];    /**< "  SIZE. 0<-%3d->%3d\n" */
extern char sEdHitLabel[];              /**< "  HIT.  " */
extern char sEdFlipLabel[];             /**< "  FLIP.  " */
extern char sEdActLabel[];              /**< "  ACT.   " */
extern char sEdNearActText[];           /**< "NEAR ACT\n" */
extern char sEdHitActText[];            /**< "HIT ACT\n" */
extern char sEdWalkLabel[];             /**< "  WALK.  " */
extern char sEdSeLabel[];               /**< "  SE.    " */
extern char sEdWarningText[];           /**< "!!! WARNING !!!" */
extern char sEdCansTrapsLine[];         /**< "PEPSICAN TO TRAP TO" */
extern char sEdSwitchesCantPlaceLine[]; /**< "SWITCH GA OKEMASEN." */
extern char sEdGroundHitLine[];         /**< "JIMEN NO HIT GA" */
extern char sEdUntakeableHereLine[];    /**< "TORENAI BASHO NI" */
extern char sEdPushCrossText[];         /**< "            PUSH BATU" */
extern char sEdBufferMaxLine[];         /**< "BAFFER GA MAX NI NARI" */
extern char sEdReachedNoMoreLine[];     /**< "MASITA. KORE IJYOU HA" */
extern char sEdCantPlaceLine[];         /**< "OKEMASEN." */
extern char sEdAskMiyagoeLine[];        /**< "MIYAGOE SAMA NI KIITE" */
extern char sEdPleaseLine[];            /**< "KUDASAI." */
extern char sEdSwitchesDeleteLine[];    /**< "SWITCH GA SAKUJYO" */
extern char sEdCannotLine[];            /**< "DEKIMASEN." */
extern char sEdBufferZeroLine[];        /**< "BAFFER GA ZERO NI NARI" */
extern char sEdCantDeleteLine[];        /**< "SAKUJYO DEKIMASEN." */
extern char sEdAreaDeleteLine[];        /**< "KONO AREA NI SAKUJYO" */
extern char sEdDeletableLine[];         /**< "DEKIRU MONO HA" */
extern char sEdThereAreNoneLine[];      /**< "ARIMASEN." */
extern char sEdTrapsSwitchesLine[];     /**< "TRAP TO SWITCH GA" */
extern char sEdNotNearbyLine[];         /**< "CHIKAKU NI ARIMASEN." */
extern char sEdWhatToEraseLine[];       /**< "KESITAI MONO NO " */
extern char sEdMoveNearLine[];          /**< "CHIKAKU NI IDOU" */
extern char sEdPleaseDoLine[];          /**< "SITEKUDASAI." */
extern char sEdColorCyan[];             /**< "~c088" */
extern char sEdColorRed[];              /**< "~c800" */
extern char sEdPosYFmt[];               /**< "Y %d " */
extern char sEdPosZFmt[];               /**< "Z %d" */
extern char sEdMenuPepsiText[];         /**< "PEPSI\n" */
extern char sEdMenuSwitchText[];        /**< "SWITCH\n" */
extern char sEdMenuTrapText[];          /**< "TRAP\n" */
extern char sEdMenuSpriteText[];        /**< "SPRITE\n" */
extern char sEdMenuDeleteText[];        /**< "DELETE\n" */
extern char sEdMenuInfoText[];          /**< "INFO\n" */
extern char sEdMenuDefPosText[];        /**< "DEFPOS\n" */
extern char sEdMenuDataText[];          /**< "DATA\n" */
extern char sEdColorGrey[];             /**< "~c444" */
extern char sEdNum2Fmt[];               /**< "%2d" */
extern char sEdRangeEndBlankFmt[];      /**< "->%d\n\n" */
extern char sEdNewlines4[];             /**< "\n\n\n\n" */
extern char sEdRangeEndFmt[];           /**< "->%d\n" */
extern char sEdRangeEnd2Fmt[];          /**< "->%2d\n" */
extern char sEdNewlines5[];             /**< "\n\n\n\n\n" */
extern char sEdOffLine[];               /**< "OFF\n" */
extern char sEdOnLine[];                /**< "ON\n" */
extern char sEdNoActLine[];             /**< "NO ACT\n" */
extern char sEdSeNoFmt[];               /**< "NO %3d\n" */
extern char sEdManLine[];               /**< "MAN\n" */
extern char sEdWomanLine[];             /**< "WOMAN\n" */
extern char sEdChildLine[];             /**< "CHILD\n" */
extern char sEdDogLine[];               /**< "DOG\n" */
extern char sEdYouAreLine[];            /**< "IMASU." */
extern VECTOR sStatusPos;               /**< the position printed on the status panel */

void PrintEditorCaption(void);
void PrintEditorData(void);

/** @brief Prints the tool's status panel: the sync count, the grid step,
 *         the block, the position, the record counts and the current item,
 *         then the panel of the current tool mode or error.
 *  @return nothing; the value is undefined. */
s32 PrintEditorStatus(void) {
    /* MATCHING: s32 with no return; the unused local gives the 0x40 frame. */
    s32 unused[2];
    /* MATCHING: table bases through locals load before the index. */
    EditorSwitch *recs;
    EditorTrap *objs;

    FntPrint(sEdColorWhite);
    if (gVSyncTime > 400) {
        FntPrint(sEdColorCyan);
    }
    if (gVSyncTime > 450) {
        FntPrint(sEdColorRed);
    }
    FntPrint(gSyncFmt, gVSyncTime);
    FntPrint(sEdColorWhite);
    FntPrint(gGridFmt, sTotals.unk4);
    FntPrint(gNowBlockFmt, gCurBlock, gStageObjCount);
    sStatusPos.vx = gPlayer.x;
    sStatusPos.vy = gPlayer.y;
    sStatusPos.vz = gPlayer.z;
    FntPrint(sEdPositionFmt, sStatusPos.vx);
    if (sTotals.unk8 == 0) {
        FntPrint(sEdColorWhite);
    } else if (sTotals.unk8 < 0) {
        FntPrint(sEdColorBlue);
    } else {
        FntPrint(sEdColorRed);
    }
    FntPrint(sEdPosYFmt, sStatusPos.vy);
    FntPrint(sEdColorWhite);
    FntPrint(sEdPosZFmt, sStatusPos.vz);
    if (gCurBlock != -1) {
        FntPrint(sEdColorWhite);
    } else {
        FntPrint(sEdColorRed);
    }
    FntPrint(sEdSwitchNoFmt, sTotals.unk16, 200);
    FntPrint(sEdTrapNoFmt, sTotals.unk1A, 200);
    FntPrint(sEdPepsiNoFmt, sTotals.unk26, 200);
    FntPrint(sEdItemText);
    switch (sTotals.unk3) {
        case 0:
            FntPrint(sEdPepsiFmt, sTotals.canGroup + 1);
            break;
        case 1:
            FntPrint(sEdSwitchFmt, sTotals.unk12);
            break;
        case 2:
            FntPrint(sEdTrapFmt, sTotals.unk18);
            break;
        case 3:
            FntPrint(sEdSpriteFmt, sTotals.unk28);
            break;
    }
    FntPrint(sEdColorWhite);
    SortSemiQuad(0x421, -160, -120, 160, -120, 160, -70, -160, -70, 0);
    FntPrint(sEdNewline);
    if (D_800959D8 == 1) {
        FntPrint(sEdInfoText);
    } else if (D_800958D8 == 1) {
        if ((gFrameCount & 0xF) < 8) {
            FntPrint(sEdColorWhite);
        } else {
            FntPrint(sEdColorRed);
        }
        FntPrint(sEdDeleteText);
    } else {
        PrintTabs(2);
    }
    FntPrint(sEdColorWhite);
    switch ((u16)gToolMode) {
        case 0:
            PrintTabs(5);
            FntPrint(sEdMoveModeText);
            PrintEditorCaption();
            break;
        case 1:
            gEditMax = 1;
            gMenuLineCount = 8;
            PrintTabs(5);
            FntPrint(sEdMenuModeText);
            FntPrint(sEdNewlines3);
            if (gMenuLine == 0) {
                FntPrint(sEdColorWhite);
            } else {
                FntPrint(sEdColorBlue);
            }
            PrintTabs(7);
            FntPrint(sEdMenuPepsiText);
            if (gMenuLine == 1) {
                FntPrint(sEdColorWhite);
            } else {
                FntPrint(sEdColorBlue);
            }
            PrintTabs(7);
            FntPrint(sEdMenuSwitchText);
            if (gMenuLine == 2) {
                FntPrint(sEdColorWhite);
            } else {
                FntPrint(sEdColorBlue);
            }
            PrintTabs(7);
            FntPrint(sEdMenuTrapText);
            if (gMenuLine == 3) {
                FntPrint(sEdColorWhite);
            } else {
                FntPrint(sEdColorBlue);
            }
            PrintTabs(7);
            FntPrint(sEdMenuSpriteText);
            if (gMenuLine == 4) {
                FntPrint(sEdColorWhite);
            } else {
                FntPrint(sEdColorBlue);
            }
            PrintTabs(7);
            FntPrint(sEdMenuDeleteText);
            if (gMenuLine == 5) {
                FntPrint(sEdColorWhite);
            } else {
                FntPrint(sEdColorBlue);
            }
            PrintTabs(7);
            FntPrint(sEdMenuInfoText);
            if (gMenuLine == 6) {
                FntPrint(sEdColorWhite);
            } else {
                FntPrint(sEdColorBlue);
            }
            PrintTabs(7);
            FntPrint(sEdMenuDefPosText);
            if (gMenuLine == 7) {
                FntPrint(sEdColorWhite);
            } else {
                FntPrint(sEdColorBlue);
            }
            PrintTabs(7);
            FntPrint(sEdMenuDataText);
            SortSemiQuad(0x421, 60, -68, 130, -68, 130, 12, 60, 12, 0);
            break;
        case 2:
            PrintTabs(4);
            FntPrint(sEdPepsiSelModeText);
            FntPrint(sEdNewlines9);
            PrintTabs(3);
            FntPrint(sEdPepsiTypeSelText);
            PrintTabs(3);
            FntPrint(sEdPepsiTypeRangeFmt, sTotals.canGroup + 1);
            SortSemiQuad(0x421, -60, -30, 60, -30, 60, 30, -60, 30, 0);
            break;
        case 3:
            PrintTabs(4);
            FntPrint(sEdSwitchSelModeText);
            FntPrint(sEdNewlines8);
            PrintTabs(3);
            FntPrint(sEdSwitchSelText);
            PrintTabs(3);
            if (gMenuLine == 0) {
                FntPrint(sEdColorWhite);
            } else {
                FntPrint(sEdColorBlue);
            }
            FntPrint(sEdSwitchNoRangeText);
            recs = sSwitches;
            if (recs[sTotals.unk12].unk0 == -1) {
                FntPrint(sEdColorWhite);
            } else {
                FntPrint(sEdColorGrey);
            }
            FntPrint(sEdNum2Fmt, sTotals.unk12);
            if (gMenuLine == 0) {
                FntPrint(sEdColorWhite);
            } else {
                FntPrint(sEdColorBlue);
            }
            FntPrint(sEdRangeEndBlankFmt, 199);
            PrintTabs(3);
            if (gMenuLine == 1) {
                FntPrint(sEdColorWhite);
            } else {
                FntPrint(sEdColorBlue);
            }
            FntPrint(sEdSwitchTypeRangeFmt, sTotals.unk14, 2);
            SortSemiQuad(0x421, -60, -30, 60, -30, 60, 30, -60, 30, 0);
            break;
        case 4:
            PrintTabs(4);
            FntPrint(sEdTrapSelModeText);
            if (D_800959D8 == 0) {
                FntPrint(sEdNewlines4);
            }
            PrintEditorCaption();
            FntPrint(sEdNewlines4);
            PrintTabs(3);
            FntPrint(sEdTrapSelText);
            PrintTabs(3);
            if (gMenuLine == 0) {
                FntPrint(sEdColorWhite);
            } else {
                FntPrint(sEdColorBlue);
            }
            FntPrint(sEdTrapNoRangeText);
            objs = (EditorTrap *)gTraps;
            if (objs[sTotals.unk18].unk36 == -1) {
                FntPrint(sEdColorWhite);
            } else {
                FntPrint(sEdColorGrey);
            }
            FntPrint(sEdNum2Fmt, sTotals.unk18);
            if (gMenuLine == 0) {
                FntPrint(sEdColorWhite);
            } else {
                FntPrint(sEdColorBlue);
            }
            FntPrint(sEdRangeEndFmt, 199);
            PrintTabs(3);
            if (gMenuLine == 1) {
                FntPrint(sEdColorWhite);
            } else {
                FntPrint(sEdColorBlue);
            }
            FntPrint(sEdTrapModelRangeFmt, sTotals.unk1C, gSectionCount);
            PrintTabs(3);
            FntPrint(sEdUseBufferFmt, gPlaceCount);
            PrintTabs(3);
            if (gMenuLine == 2) {
                FntPrint(sEdColorWhite);
            } else {
                FntPrint(sEdColorBlue);
            }
            FntPrint(sEdTrapSwitchRangeText);
            recs = sSwitches;
            if (recs[sTotals.unk1E].unk0 != -1) {
                FntPrint(sEdColorWhite);
            } else {
                FntPrint(sEdColorGrey);
            }
            FntPrint(sEdNum2Fmt, sTotals.unk1E);
            if (gMenuLine == 2) {
                FntPrint(sEdColorWhite);
            } else {
                FntPrint(sEdColorBlue);
            }
            FntPrint(sEdRangeEnd2Fmt, 199);
            PrintTabs(3);
            if (gMenuLine == 3) {
                FntPrint(sEdColorWhite);
            } else {
                FntPrint(sEdColorBlue);
            }
            FntPrint(sEdTrapTypeRangeText);
            FntPrint(sEdNum2Fmt, sTotals.unk68);
            FntPrint(sEdRangeEnd2Fmt, 255);
            PrintTabs(3);
            if (gMenuLine == 4) {
                FntPrint(sEdColorWhite);
            } else {
                FntPrint(sEdColorBlue);
            }
            FntPrint(sEdTrapActRangeText);
            FntPrint(sEdNum2Fmt, sTotals.unk69);
            FntPrint(sEdRangeEnd2Fmt, 255);
            FntPrint(sEdColorWhite);
            PrintTabs(2);
            FntPrint(sEdAngleRangeFmt, sTotals.unk20);
            SortSemiQuad(0x421, -100, -30, 100, -30, 100, 56, -100, 56, 0);
            break;
        case 5:
            PrintTabs(4);
            FntPrint(sEdSpriteSelModeText);
            FntPrint(sEdNewlines5);
            PrintTabs(3);
            FntPrint(sEdSpriteSelText);
            PrintTabs(2);
            if (gMenuLine == 0) {
                FntPrint(sEdColorWhite);
            } else {
                FntPrint(sEdColorBlue);
            }
            FntPrint(sEdSpriteNoRangeText);
            if (gStageSprites[sTotals.unk28].unk0 == -1) {
                FntPrint(sEdColorWhite);
            } else {
                FntPrint(sEdColorGrey);
            }
            FntPrint(sEdNum2Fmt, sTotals.unk28);
            if (gMenuLine == 0) {
                FntPrint(sEdColorWhite);
            } else {
                FntPrint(sEdColorBlue);
            }
            FntPrint(sEdRangeEndFmt, 99);
            PrintTabs(2);
            if (gMenuLine == 1) {
                FntPrint(sEdColorWhite);
            } else {
                FntPrint(sEdColorBlue);
            }
            FntPrint(sEdSpriteModelRangeFmt, sTotals.unk2C, 512);
            PrintTabs(2);
            if (gMenuLine == 2) {
                FntPrint(sEdColorWhite);
            } else {
                FntPrint(sEdColorBlue);
            }
            FntPrint(sEdSpriteSwitchRangeText);
            recs = sSwitches;
            if (recs[sTotals.unk2A].unk0 != -1) {
                FntPrint(sEdColorWhite);
            } else {
                FntPrint(sEdColorGrey);
            }
            FntPrint(sEdNum2Fmt, sTotals.unk2A);
            if (gMenuLine == 2) {
                FntPrint(sEdColorWhite);
            } else {
                FntPrint(sEdColorBlue);
            }
            FntPrint(sEdRangeEnd2Fmt, 199);
            PrintTabs(2);
            if (gMenuLine == 3) {
                FntPrint(sEdColorWhite);
            } else {
                FntPrint(sEdColorBlue);
            }
            FntPrint(sEdSpriteSizeRangeFmt, (s16)sTotals.unk2E - 500, 999);
            PrintTabs(2);
            if (gMenuLine == 4) {
                FntPrint(sEdColorWhite);
            } else {
                FntPrint(sEdColorBlue);
            }
            FntPrint(sEdHitLabel);
            if (sTotals.unk32 == 0) {
                FntPrint(sEdOffLine);
            } else {
                FntPrint(sEdOnLine);
            }
            if (gMenuLine == 5) {
                FntPrint(sEdColorWhite);
            } else {
                FntPrint(sEdColorBlue);
            }
            PrintTabs(2);
            FntPrint(sEdFlipLabel);
            if (sTotals.unk34 == 0) {
                FntPrint(sEdOffLine);
            } else {
                FntPrint(sEdOnLine);
            }
            if (gMenuLine == 6) {
                FntPrint(sEdColorWhite);
            } else {
                FntPrint(sEdColorBlue);
            }
            PrintTabs(2);
            FntPrint(sEdActLabel);
            switch (sSpriteFlags) {
                case 0:
                    FntPrint(sEdNoActLine);
                    break;
                case 1:
                    FntPrint(sEdNearActText);
                    break;
                case 2:
                    FntPrint(sEdHitActText);
                    break;
            }
            if (gMenuLine == 7) {
                FntPrint(sEdColorWhite);
            } else {
                FntPrint(sEdColorBlue);
            }
            PrintTabs(2);
            FntPrint(sEdWalkLabel);
            if (sTotals.unk6C == 0) {
                FntPrint(sEdOffLine);
            } else {
                FntPrint(sEdSeNoFmt, sTotals.unk6C);
            }
            if (gMenuLine == 8) {
                FntPrint(sEdColorWhite);
            } else {
                FntPrint(sEdColorBlue);
            }
            PrintTabs(2);
            FntPrint(sEdSeLabel);
            switch (sTotals.unk6D) {
                case 0:
                    FntPrint(sEdOffLine);
                    break;
                case 1:
                    FntPrint(sEdManLine);
                    break;
                case 2:
                    FntPrint(sEdWomanLine);
                    break;
                case 3:
                    FntPrint(sEdChildLine);
                    break;
                case 4:
                    FntPrint(sEdDogLine);
                    break;
            }
            FntPrint(sEdColorWhite);
            PrintTabs(2);
            FntPrint(sEdAngleRangeFmt, sTotals.unk6A);
            SortSemiQuad(0x421, -100, -54, 100, -54, 100, 60, -100, 60, 0);
            break;
        case 6:
            PrintEditorData();
            break;
        case 7:
            FntPrint(sEdNewlines3);
            PrintTabs(3);
            FntPrint(sEdWarningText);
            FntPrint(sEdNewlines3);
            PrintTabs(2);
            FntPrint(sEdCansTrapsLine);
            FntPrint(sEdNewline);
            PrintTabs(2);
            FntPrint(sEdSwitchesCantPlaceLine);
            FntPrint(sEdNewlines2);
            PrintTabs(2);
            FntPrint(sEdGroundHitLine);
            FntPrint(sEdNewline);
            PrintTabs(2);
            FntPrint(sEdUntakeableHereLine);
            FntPrint(sEdNewline);
            PrintTabs(2);
            FntPrint(sEdYouAreLine);
            FntPrint(sEdNewline);
            PrintTabs(2);
            FntPrint(sEdPushCrossText);
            SortSemiQuad(0x42F, -100, -62, 100, -62, 100, 80, -100, 80, 0);
            break;
        case 8:
            FntPrint(sEdNewlines3);
            PrintTabs(3);
            FntPrint(sEdWarningText);
            FntPrint(sEdNewlines3);
            PrintTabs(2);
            FntPrint(sEdCansTrapsLine);
            FntPrint(sEdNewline);
            PrintTabs(2);
            FntPrint(sEdSwitchesCantPlaceLine);
            FntPrint(sEdNewlines2);
            PrintTabs(2);
            FntPrint(sEdBufferMaxLine);
            FntPrint(sEdNewline);
            PrintTabs(2);
            FntPrint(sEdReachedNoMoreLine);
            FntPrint(sEdNewline);
            PrintTabs(2);
            FntPrint(sEdCantPlaceLine);
            FntPrint(sEdNewline);
            PrintTabs(2);
            FntPrint(sEdAskMiyagoeLine);
            FntPrint(sEdNewline);
            PrintTabs(2);
            FntPrint(sEdPleaseLine);
            FntPrint(sEdNewlines2);
            PrintTabs(2);
            FntPrint(sEdPushCrossText);
            SortSemiQuad(0x42F, -100, -62, 100, -62, 100, 80, -100, 80, 0);
            break;
        case 9:
            FntPrint(sEdNewlines3);
            PrintTabs(3);
            FntPrint(sEdWarningText);
            FntPrint(sEdNewlines3);
            PrintTabs(2);
            FntPrint(sEdCansTrapsLine);
            FntPrint(sEdNewline);
            PrintTabs(2);
            FntPrint(sEdSwitchesDeleteLine);
            FntPrint(sEdNewline);
            PrintTabs(2);
            FntPrint(sEdCannotLine);
            FntPrint(sEdNewlines2);
            PrintTabs(2);
            FntPrint(sEdBufferZeroLine);
            FntPrint(sEdNewline);
            PrintTabs(2);
            FntPrint(sEdReachedNoMoreLine);
            FntPrint(sEdNewline);
            PrintTabs(2);
            FntPrint(sEdCantDeleteLine);
            FntPrint(sEdNewlines2);
            PrintTabs(2);
            FntPrint(sEdPushCrossText);
            SortSemiQuad(0x42F, -100, -62, 100, -62, 100, 80, -100, 80, 0);
            break;
        case 10:
            FntPrint(sEdNewlines3);
            PrintTabs(3);
            FntPrint(sEdWarningText);
            FntPrint(sEdNewlines3);
            PrintTabs(2);
            FntPrint(sEdCansTrapsLine);
            FntPrint(sEdNewline);
            PrintTabs(2);
            FntPrint(sEdSwitchesDeleteLine);
            FntPrint(sEdNewline);
            PrintTabs(2);
            FntPrint(sEdCannotLine);
            FntPrint(sEdNewlines2);
            PrintTabs(2);
            FntPrint(sEdAreaDeleteLine);
            FntPrint(sEdNewline);
            PrintTabs(2);
            FntPrint(sEdDeletableLine);
            FntPrint(sEdNewline);
            PrintTabs(2);
            FntPrint(sEdThereAreNoneLine);
            FntPrint(sEdNewlines2);
            PrintTabs(2);
            FntPrint(sEdPushCrossText);
            SortSemiQuad(0x42F, -100, -62, 100, -62, 100, 80, -100, 80, 0);
            break;
        case 11:
            FntPrint(sEdNewlines3);
            PrintTabs(3);
            FntPrint(sEdWarningText);
            FntPrint(sEdNewlines3);
            PrintTabs(2);
            FntPrint(sEdTrapsSwitchesLine);
            FntPrint(sEdNewline);
            PrintTabs(2);
            FntPrint(sEdNotNearbyLine);
            FntPrint(sEdNewlines2);
            PrintTabs(2);
            FntPrint(sEdWhatToEraseLine);
            FntPrint(sEdNewline);
            PrintTabs(2);
            FntPrint(sEdMoveNearLine);
            FntPrint(sEdNewline);
            PrintTabs(2);
            FntPrint(sEdPleaseDoLine);
            FntPrint(sEdNewlines2);
            PrintTabs(2);
            FntPrint(sEdPushCrossText);
            SortSemiQuad(0x42F, -100, -62, 100, -62, 100, 80, -100, 80, 0);
            break;
    }
}

/** @brief Prints the tool mode's caption for the current mode: the
 *         selected point, switch, record or sprite with their settings,
 *         then draws the frame around it. Runs only while the tool flag
 *         is 1. */
void PrintEditorCaption(void) {
    /* MATCHING: an unused eight-byte local gives retail's 0x40 frame. */
    s32 unused[2];
    Span8 *e;
    EditorTrap *r;

    if (D_800959D8 != 1) {
        return;
    }
    switch (sTotals.unk3) {
        case 0:
            e = (Span8 *)gCanSpans;
            e += gCurBlock;
            FntPrint(sEdCanNoFmt, sTotals.unk22);
            FntPrint(sEdCanNumFmt, e->count);
            break;
        case 1:
            FntPrint(sEdInfoSwitchNoFmt, sTotals.unk12);
            break;
        case 2:
            FntPrint(sEdInfoTrapNoFmt, sTotals.unk18);
            if (((EditorTrap *)gTraps)[sTotals.unk18].unk38 != -1) {
                FntPrint(sEdInfoTrapModelFmt, ((EditorTrap *)gTraps)[sTotals.unk18].unk38);
            } else {
                FntPrint(sEdOptionsText);
            }
            /* MATCHING: the table base in a pointer local after the join
             * takes $s1, the totals' base $s0. */
            r = (EditorTrap *)gTraps;
            FntPrint(sEdInfoTrapSwitchFmt, r[sTotals.unk18].unk34);
            FntPrint(sEdInfoTypeFmt, r[sTotals.unk18].unk42);
            FntPrint(sEdInfoActFmt, r[sTotals.unk18].unk43);
            break;
        case 3:
            FntPrint(sEdInfoSpriteFmt, sTotals.unk28);
            FntPrint(sEdInfoSpriteModelFmt, gStageSprites[sTotals.unk28].unk0 - 30);
            FntPrint(sEdInfoSpriteSwitchFmt, gStageSprites[sTotals.unk28].unk2C);
            FntPrint(sEdInfoHitLabel);
            /* MATCHING: the clear-bit arm first, as retail's bnez. */
            if (!(gStageSprites[sTotals.unk28].unk26 & 0x80)) {
                FntPrint(sEdInfoOffText);
            } else {
                FntPrint(sEdInfoOnText);
            }
            FntPrint(sEdInfoActLabel);
            switch (gStageSprites[sTotals.unk28].unk26 & 0x3F) {
                case 0:
                    FntPrint(sEdNoActText);
                    break;
                case 1:
                    FntPrint(sEdInfoNearActText);
                    break;
                case 2:
                    FntPrint(sEdInfoHitActText);
                    break;
            }
            FntPrint(sEdInfoWalkLabel);
            if (gStageSprites[sTotals.unk28].unk27 == 0) {
                FntPrint(sEdInfoOffLine);
            } else {
                FntPrint(sEdInfoWalkTypeFmt, gStageSprites[sTotals.unk28].unk27);
            }
            FntPrint(sEdInfoSeLabel);
            switch (gStageSprites[sTotals.unk28].unk28) {
                case 0:
                    FntPrint(sEdNoSeText);
                    break;
                case 1:
                    FntPrint(sEdManText);
                    break;
                case 2:
                    FntPrint(sEdWomanText);
                    break;
                case 3:
                    FntPrint(sEdChildText);
                    break;
                case 4:
                    FntPrint(sEdDogText);
                    break;
            }
            FntPrint(sEdNewline);
            break;
    }
    SortSemiQuad(0x7C21, -160, -70, 160, -70, 160, -44, -160, -44, 0);
}

/** @brief Runs the menu editor, then on flag bit 5 enters the tool mode
 *         of the highlighted line.
 *  @return nothing; the value is undefined. */
s32 UpdateEditorMenu(void) {
    /* MATCHING: non-void with no return puts the index sll in the bound
     * check's delay slot. */
    StepMenuCursor();
    if (gPadTrig & PADRright) {
        switch (gMenuLine) {
            case 0:
                gEditMax = 200;
                gMenuLineCount = 1;
                D_800958D8 = 0;
                D_800959D8 = 0;
                sTotals.unk3 = gMenuLine;
                gEditValue = sTotals.canGroup;
                gMenuLine = 0;
                gToolMode = 2;
                break;
            case 1:
                gEditMax = 200;
                gMenuLineCount = 2;
                D_800958D8 = 0;
                D_800959D8 = 0;
                sTotals.unk3 = gMenuLine;
                gEditValue = sTotals.unk12;
                gMenuLine = 0;
                gToolMode = 3;
                break;
            case 2:
                gEditMax = 200;
                gMenuLineCount = 5;
                D_800958D8 = 0;
                D_800959D8 = 0;
                sTotals.unk3 = gMenuLine;
                gEditValue = sTotals.unk18;
                gMenuLine = 0;
                gSubStep = 0;
                gToolMode = 4;
                break;
            case 3:
                gEditMax = 100;
                gMenuLineCount = 9;
                D_800958D8 = 0;
                D_800959D8 = 0;
                sTotals.unk3 = gMenuLine;
                gEditValue = sTotals.unk28;
                gMenuLine = 0;
                gSubStep = 0;
                gToolMode = 5;
                break;
            case 4:
                D_800958D8 = 1;
                D_800959D8 = 0;
                gSubStep = 0;
                break;
            case 5:
                D_800959D8 = 1;
                /* MATCHING: an int-valued 1 (retail rebuilds the constant). */
                D_800958D8 = D_800959D8 != 0;
                gSubStep = 0;
                gToolMode = 0;
                break;
            case 6:
                gToolMode = 0;
                gPlayer.x = gCourse.x;
                gPlayer.y = gCourse.y;
                gPlayer.z = gCourse.z;
                gCourse.rot.vy = gCourse.rot.vz;
                gFloorIndex = FindFloor(&gPlayer.x);
                gSection = 0;
                gView[1] = (s16)(gCourse.y - 500);
                break;
            case 7:
                gToolMode = 6;
                gSubStep = 0;
                break;
        }
    }
}

/** @brief Fills the six camera words from the yaw in the first rotation
 *         and the view's orbit angle. */
void SetEditorCamera(void) {
    SVECTOR *rot;

    rot = &gCourse.rot;
    gRefView[0] = rsin(rot->vy) * 500 / 4096;
    gRefView[1] = rsin(gViewerAngle * 4096 / 360) * 500 / 4096 + (gPlayer.y + sTotals.unkC);
    gRefView[2] = rcos(rot->vy) * 500 / 4096;
    gRefView[3] = rsin(rot->vy - 0x800) * 900 / 4096;
    gRefView[4] = gPlayer.y;
    gRefView[5] = rcos(rot->vy - 0x800) * 900 / 4096;
}

/** @brief Free movement in the tool: moves the game position by the pad
 *         buttons and the analogue stick, turns, raises or lowers the
 *         camera and the height offset, then finds the entry under the
 *         position and puts the position on its ground height.
 *  @return nothing; the value is undefined. */
s32 EditorFreeMove(void) {
    s32 cur;

    /* MATCHING: non-void with no return keeps the flag test's delay slot
     * a nop. */
    if (gPadTrig & PADLup) {
        MoveGamePos(0, sTotals.unk4);
    }
    if (gPadTrig & PADLdown) {
        MoveGamePos(180, sTotals.unk4);
    }
    if (gPadTrig & PADLleft) {
        MoveGamePos(-90, sTotals.unk4);
    }
    if (gPadTrig & PADLright) {
        MoveGamePos(90, sTotals.unk4);
    }
    if (gStickRawY < 0x60) {
        MoveGamePos(0, sTotals.unk4);
    }
    if (gStickRawY > 0xA0) {
        MoveGamePos(180, sTotals.unk4);
    }
    if (gStickRawX < 0x60) {
        MoveGamePos(-90, sTotals.unk4);
    }
    if (gStickRawX > 0xA0) {
        MoveGamePos(90, sTotals.unk4);
    }
    gWorldShift[1] = 0;
    gWorldShift[0] = -gPlayer.x;
    gWorldShift[2] = -gPlayer.z;
    if (gPad & PADL1) {
        gCourse.rot.vy -= 11;
    }
    if (gPad & PADR1) {
        gCourse.rot.vy += 11;
    }
    if (D_800958D8 == 0) {
        if (gPad & PADL2) {
            sTotals.unkC -= 5;
        }
        if (gPad & PADR2) {
            sTotals.unkC += 5;
        }
        if (sTotals.unkC > 0) {
            sTotals.unkC = 0;
        }
    }
    if (gPadTrig & PADRleft) {
        sTotals.unk8 -= sTotals.unk4;
    }
    if (gPadTrig & PADRdown) {
        sTotals.unk8 += sTotals.unk4;
    }
    gPlayer.unk3C4 = gPlayer.y;
    cur = FindFloorNear(&gPlayer.x, gFloorY, gFloorIndex);
    gCurBlock = cur;
    if (cur != -1) {
        gFloorIndex = cur;
    }
    gPlayer.y = gFloorY[0] + sTotals.unk8;
    if (D_800958D8 == 0 && (gPadTrig & PADRright) && cur == -1) {
        gToolMode = 7;
    }
}

/** @brief On flag bit 5, inserts the game position as a new point at the
 *         end of the current entry, shifting the later points up, then
 *         rebuilds the block header. */
/* MATCHING: `e = base; e += k;` and one points pointer for the shift source
 * and the new point. */
void EditorInsertPoint(void) {
    s16 pos[3];
    Pt8 *dst;
    Pt8 *p;
    Span8 *e;
    s32 i;
    u32 j;

    if (gPadTrig & PADRright) {
        if (sTotals.unk26 == 200) {
            gToolMode = 8;
        } else if (gCurBlock == -1) {
            gToolMode = 7;
        } else {
            e = (Span8 *)sCur.ents;
            e += gCurBlock;
            p = (Pt8 *)sCur.pts;
            dst = p;
            pos[0] = gPlayer.x;
            pos[1] = gPlayer.y;
            pos[2] = gPlayer.z;
            p += 198;
            dst += 199;
            for (i = e->start + e->count; i < 200; i++) {
                *dst = *p;
                dst--;
                p--;
            }
            e = (Span8 *)sCur.ents;
            e += gCurBlock;
            p = (Pt8 *)sCur.pts;
            j = e->count;
            e->count = j + 1;
            p += e->start + j;
            e = (Span8 *)sCur.ents;
            e += gCurBlock + 1;
            p->x = pos[0];
            p->y = pos[1];
            p->z = pos[2];
            p->tag = sTotals.canGroup + 1;
            for (j = gCurBlock + 1; j < gStageObjCount; j++) {
                e->start++;
                e++;
            }
            SetCurrentBlock((BlockHeader *)0x801FD000);
        }
    }
}

/** @brief Point editor: on the first pass selects the current entry's
 *         first point; then moves the game position to the selected point,
 *         deletes it on flag bit 5 (shifting the later points down), steps
 *         the selection on flag bits 0 and 1 and clamps it to the entry.
 *  @return nothing; the value is undefined. */
s32 EditorEditPoints(void) {
    VECTOR pos;
    Pt8 *dst;
    Pt8 *src;
    Span8 *e;
    s32 i;
    u32 j;

    /* MATCHING: non-void with no return value; each entry access is
     * `e = base; e += k;`, which also keeps the second count test. */
    switch (gSubStep) {
        case 0:
            e = (Span8 *)gCanSpans;
            e += gCurBlock;
            gSubStep = 1;
            sTotals.unk22 = e->start;
            return;
        case 1:
            break;
        default:
            return;
    }
    src = (Pt8 *)gCanPoints;
    src += sTotals.unk22;
    pos.vx = src->x;
    pos.vy = src->y;
    pos.vz = src->z;
    DrawPin(&pos);
    e = (Span8 *)gCanSpans;
    e += gCurBlock;
    if (e->count == 0 || gCurBlock == -1) {
        gToolMode = 10;
        return;
    }
    if (D_800959D8 != 1 && (gPadTrig & PADRright)) {
        e = (Span8 *)gCanSpans;
        e += gCurBlock;
        if (e->count == 0) {
            gToolMode = 9;
            return;
        }
        e = (Span8 *)gCanSpans;
        e += gCurBlock;
        dst = (Pt8 *)gCanPoints;
        src = dst;
        src += sTotals.unk22 + 1;
        dst += sTotals.unk22;
        for (i = e->start + e->count; i < 199; i++) {
            *dst = *src;
            dst++;
            src++;
        }
        e = (Span8 *)gCanSpans;
        e += gCurBlock;
        e->count--;
        e = (Span8 *)gCanSpans;
        e += gCurBlock + 1;
        for (j = gCurBlock + 1; j < gStageObjCount; j++) {
            e->start--;
            e++;
        }
    }
    if (gPadTrig & PADL2) {
        sTotals.unk22--;
    }
    if (gPadTrig & PADR2) {
        sTotals.unk22++;
    }
    e = (Span8 *)gCanSpans;
    e += gCurBlock;
    if (sTotals.unk22 < (u32)e->start) {
        sTotals.unk22 = e->start;
    }
    /* MATCHING: the compare as start + (count - 1), the store as
     * count - 1 + start; unsigned, as retail's sltu. */
    if ((u32)e->start + (e->count - 1) < sTotals.unk22) {
        sTotals.unk22 = (u32)e->count - 1 + e->start;
    }
}

/** @brief Counts the used switches (clearing them to 0), picks a
 *         free slot if the current one is taken, sizes a box there at the
 *         game position, and on flag bit 5 places it and moves the slot on
 *         to the next free one (error 7 with no entry, 8 when full).
 *  @return nothing; the value is undefined. */
s32 EditorPlaceBox(void) {
    Sizes3 sizes;
    u32 i;

    /* MATCHING: non-void with no return value keeps the flag test's delay
     * slot a nop. */
    sizes = sBoxSizes;
    sTotals.unk16 = 0;
    for (i = 0; i < 200; i++) {
        if (sSwitches[i].unk0 != -1) {
            sSwitches[i].unk0 = 0;
            sTotals.unk16++;
        }
    }
    if (sTotals.unk16 == 200) {
        gToolMode = 8;
        return;
    }
    if (sSwitches[sTotals.unk12].unk0 != -1) {
        for (i = 0; i < 200; i++) {
            if (sSwitches[i].unk0 == -1) {
                break;
            }
        }
        if (i == 200) {
            gToolMode = 8;
            return;
        }
        sTotals.unk12 = i;
    }
    SetBoxCentred(&sSwitches[sTotals.unk12], sizes.s[sTotals.unk14].w, sizes.s[sTotals.unk14].d);
    sSwitches[sTotals.unk12].unk50[0] = gPlayer.x;
    sSwitches[sTotals.unk12].unk50[1] = gPlayer.y;
    sSwitches[sTotals.unk12].unk50[2] = gPlayer.z;
    sSwitches[sTotals.unk12].unk0 = -1;
    if (gPadTrig & PADRright) {
        if (gCurBlock == -1) {
            gToolMode = 7;
            return;
        }
        sSwitches[sTotals.unk12].unk0 = 0;
        for (i = 0; i < 200; i++) {
            if (sSwitches[i].unk0 == -1) {
                break;
            }
        }
        if (i == 200) {
            gToolMode = 8;
            return;
        }
        sTotals.unk12 = i;
    }
}

/** @brief Box picker: counts the used switches (clearing them to
 *         0); on the first pass selects the one nearest the game position
 *         (error 9 with none, 11 when none is in range); then moves the
 *         game position to the selection, frees it on flag bit 5, or else
 *         steps the selection over the free records on flag bits 0 and 1.
 *  @return nothing; the value is undefined. */
s32 EditorPickBox(void) {
    VECTOR pos;
    VECTOR at;
    VECTOR sel;
    EditorSwitch *r;
    u32 i;
    s32 best;
    s32 d;

    /* MATCHING: non-void with no return keeps the last test's delay slot a
     * nop; the counter cleared before the count, as retail's prologue. */
    i = 0;
    sTotals.unk16 = 0;
    for (; i < 200; i++) {
        if (sSwitches[i].unk0 != -1) {
            sSwitches[i].unk0 = 0;
            sTotals.unk16++;
        }
    }
    switch (gSubStep) {
        case 0:
            if (sTotals.unk16 == 0) {
                gToolMode = 9;
                break;
            }
            best = 25000000;
            pos.vx = gPlayer.x;
            pos.vy = gPlayer.y;
            pos.vz = gPlayer.z;
            /* MATCHING: the record pointer taken inside the body keeps -1
             * rebuilt every pass. */
            for (i = 0; i < 200; i++) {
                r = &sSwitches[i];
                if (r->unk0 != -1) {
                    at.vx = r->unk50[0];
                    at.vy = r->unk50[1];
                    at.vz = r->unk50[2];
                    d = DistanceSquared(&pos, &at);
                    if (d < best) {
                        sTotals.unk12 = i;
                        best = d;
                    }
                }
            }
            if (best == 25000000) {
                gToolMode = 11;
            } else {
                gSubStep++;
            }
            break;
        case 1:
            /* MATCHING: the table base in the record pointer first; indexing
             * sSwitches directly builds the totals' address first. */
            r = sSwitches;
            sel.vx = r[sTotals.unk12].unk50[0];
            sel.vy = r[sTotals.unk12].unk50[1];
            sel.vz = r[sTotals.unk12].unk50[2];
            DrawPin(&sel);
            if (D_800959D8 != 1 && (gPadTrig & PADRright)) {
                sSwitches[sTotals.unk12].unk0 = -1;
                gSubStep = 0;
                break;
            }
            if (gPadTrig & PADL2) {
                if (--sTotals.unk12 >= 200) {
                    sTotals.unk12 = 199;
                }
                while (sSwitches[sTotals.unk12].unk0 == -1) {
                    if (--sTotals.unk12 >= 200) {
                        sTotals.unk12 = 199;
                    }
                }
            }
            if (gPadTrig & PADR2) {
                /* the first step up clamps to 199, the later ones wrap to 0 */
                if (++sTotals.unk12 >= 200) {
                    sTotals.unk12 = 199;
                }
                while (sSwitches[sTotals.unk12].unk0 == -1) {
                    if (++sTotals.unk12 >= 200) {
                        sTotals.unk12 = 0;
                    }
                }
            }
            break;
    }
}

/** @brief Counts the live EditorTrap records; on flag bit 5, places every
 *         waiting record into the table (error 8 when it is full). */
void EditorPlaceRecords(void) {
    u32 i;
    u16 n;
    EditorTrap *recs;
    EditorTrap *w;

    recs = (EditorTrap *)gTraps;
    n = sTotals.unk18;
    sTotals.unk1A = 0;
    for (i = 0; i < 200; i++) {
        if (recs[i].unk36 != -1) {
            sTotals.unk1A++;
        }
    }
    if (gPadTrig & PADRright) {
        /* MATCHING: one counter for both loops, unsigned (sltiu) in the
         * first and compared signed (slt) here. */
        for (i = 0; (s32)i < gPlaceCount; i++) {
            w = &gPlaceQueue[i];
            if (n == 200) {
                gToolMode = 8;
                return;
            }
            w->unkC[0] = w->unk0[0];
            w->unkC[1] = w->unk0[1];
            w->unkC[2] = w->unk0[2];
            w->unk40 = 0;
            w->unk24 = 0;
            w->unk26 = 0;
            w->unk28 = gCurBlock;
            w->unk3C = gSection;
            w->unk42 = sTotals.unk68;
            w->unk43 = sTotals.unk69;
            if (i == 0) {
                /* MATCHING: stored through the table base, not w. */
                gPlaceQueue[0].unk38 = sTotals.unk1C;
            } else {
                w->unk38 = -1;
            }
            ((EditorTrap *)gTraps)[(s16)n] = *w;
            ((EditorTrap *)gTraps)[(s16)n].unk34 = sTotals.unk1E;
            n++;
            sTotals.unk18 = n;
        }
        gSubStep = 0;
        if (D_800959D8 == 1) {
            D_800959D8 = 0;
        }
    }
}

/** @brief Runs the menu editor; on a line change loads the new line's
 *         value and limit, else stores the edited value back; then steps
 *         the angle in sTotals.unk20 on flag bits 3 and 2.
 *  @return nothing; the value is undefined. */
s32 EditorEditRecordValues(void) {
    s16 line;
    s16 val;

    /* MATCHING: non-void with no return keeps the last two branches' delay
     * slots nops. */
    line = gMenuLine;
    val = gEditValue;
    StepMenuCursor();
    if (gMenuLine == 1 && val != (s16)gEditValue) {
        gSubStep = 0;
    }
    if (line != gMenuLine) {
        switch (gMenuLine) {
            case 0:
                gEditMax = 200;
                gEditValue = sTotals.unk18;
                break;
            case 1:
                gEditMax = gSectionCount;
                gEditValue = sTotals.unk1C;
                break;
            case 2:
                gEditMax = 200;
                gEditValue = sTotals.unk1E;
                break;
            case 3:
                gEditMax = 0x100;
                gEditValue = sTotals.unk68;
                break;
            case 4:
                gEditMax = 0x100;
                gEditValue = sTotals.unk69;
                break;
        }
    } else {
        switch (line) {
            case 0:
                sTotals.unk18 = gEditValue;
                break;
            case 1:
                sTotals.unk1C = gEditValue;
                break;
            case 2:
                sTotals.unk1E = gEditValue;
                break;
            case 3:
                sTotals.unk68 = gEditValue;
                break;
            case 4:
                sTotals.unk69 = gEditValue;
                break;
        }
    }
    if (gPad & PADR1) {
        if (++sTotals.unk20 >= 360) {
            sTotals.unk20 = 0;
        }
    }
    if (gPad & PADL1) {
        if (--sTotals.unk20 < 0) {
            sTotals.unk20 = 359;
        }
    }
}

/** @brief Places the waiting records: on the first pass fetches them for
 *         the game position, then sets the first one's position, heading
 *         and light angles and every later one relative to it.
 *  @return nothing; the value is undefined. */
s32 EditorArrangeRecords(void) {
    /* MATCHING: an unused record-sized local gives retail's 0x78 frame. */
    EditorTrap unused;
    EditorTrap *w;
    EditorTrap *r;
    u16 i;

    /* MATCHING: non-void with no return keeps the loop's delay slot a nop. */
    switch (gSubStep) {
        case 0:
            gPlaceQueue[0].unk18 = 0;
            gPlaceQueue[0].unk1C = 0;
            gPlaceQueue[0].unk0[0] = gPlayer.x;
            gPlaceQueue[0].unk0[1] = gPlayer.y;
            gPlaceQueue[0].unk0[2] = gPlayer.z;
            gPlaceQueue[0].unk1A = (sTotals.unk20 << 12) / 360;
            gPlaceCount = CallStageHook(sTotals.unk1C, gPlaceQueue, gPlaceQueue);
            gSubStep++;
            /* fall through */
        case 1:
            w = gPlaceQueue;
            w->unk0[0] = gPlayer.x;
            w->unk0[1] = gPlayer.y;
            w->unk0[2] = gPlayer.z;
            w->unk1A = (sTotals.unk20 << 12) / 360;
            if (gCurBlock == -1) {
                gFloorY[1] = 0;
                gFloorY[2] = -0x1000;
                gFloorY[3] = 0;
            }
            w->unk2C = ratan2(-gFloorY[3], gFloorY[2]);
            w->unk30 = ratan2(-gFloorY[1], gFloorY[2]);
            UpdateObjectEntry(&gObjectModels[w->unk36], (Trap *)w);
            for (i = 1; i < gPlaceCount; i++) {
                /* MATCHING: an integer sum puts the index first in the addu. */
                r = (EditorTrap *)(i * sizeof(EditorTrap) + (u32)w);
                gLocalPos.vx = r->unkC[0];
                gLocalPos.vy = r->unkC[1];
                gLocalPos.vz = r->unkC[2];
                TransformByCoord((GsCOORDINATE2 *)gObjectModels[w->unk36].unk10, &gLocalPos, &gWorldPos);
                r->unk0[0] = gWorldPos.vx;
                r->unk0[1] = gWorldPos.vy;
                r->unk0[2] = gWorldPos.vz;
                r->unk1A = w->unk1A + r->unk20;
                r->unk2C = w->unk2C;
                r->unk30 = w->unk30;
                UpdateObjectEntry(&gObjectModels[r->unk36], (Trap *)r);
            }
            break;
    }
}

/** @brief Record picker: counts the live EditorTrap records; on the first pass
 *         selects the one nearest the game position (error 9 with none,
 *         11 when none is in range); then moves the game position to the
 *         selection, frees it on flag bit 5 and steps the selection over
 *         the free records on flag bits 0 and 1.
 *  @return nothing; the value is undefined. */
s32 EditorPickRecord(void) {
    VECTOR pos;
    VECTOR at;
    VECTOR sel;
    EditorTrap *r;
    u32 i;
    s32 best;
    s32 d;

    /* MATCHING: non-void with no return keeps the last test's delay slot a
     * nop; the counter cleared before the count, as retail's prologue. */
    i = 0;
    sTotals.unk1A = 0;
    for (; i < 200; i++) {
        if (((EditorTrap *)gTraps)[i].unk36 != -1) {
            sTotals.unk1A++;
        }
    }
    switch (gSubStep) {
        case 0:
            if (sTotals.unk1A == 0) {
                gToolMode = 9;
                break;
            }
            best = 25000000;
            pos.vx = gPlayer.x;
            pos.vy = gPlayer.y;
            pos.vz = gPlayer.z;
            /* MATCHING: the record pointer taken inside the body keeps -1
             * rebuilt every pass. */
            for (i = 0; i < 200; i++) {
                r = &((EditorTrap *)gTraps)[i];
                if (r->unk36 != -1) {
                    at.vx = r->unk0[0];
                    at.vy = r->unk0[1];
                    at.vz = r->unk0[2];
                    d = DistanceSquared(&pos, &at);
                    if (d < best) {
                        sTotals.unk18 = i;
                        best = d;
                    }
                }
            }
            if (best == 25000000) {
                gToolMode = 11;
            } else {
                gSubStep++;
            }
            break;
        case 1:
            sTotals.unk1E = ((EditorTrap *)gTraps)[sTotals.unk18].unk34;
            sel.vx = ((EditorTrap *)gTraps)[sTotals.unk18].unk0[0];
            sel.vy = ((EditorTrap *)gTraps)[sTotals.unk18].unk0[1];
            sel.vz = ((EditorTrap *)gTraps)[sTotals.unk18].unk0[2];
            DrawPin(&sel);
            if (D_800959D8 != 1 && (gPadTrig & PADRright)) {
                gSubStep = 0;
                ((EditorTrap *)gTraps)[sTotals.unk18].unk36 = -1;
            }
            if (gPadTrig & PADL2) {
                if (--sTotals.unk18 >= 200) {
                    sTotals.unk18 = 199;
                }
                while (((EditorTrap *)gTraps)[sTotals.unk18].unk36 == -1) {
                    if (--sTotals.unk18 >= 200) {
                        sTotals.unk18 = 199;
                    }
                }
            }
            if (gPadTrig & PADR2) {
                /* the first step up clamps to 199, the later ones wrap to 0 */
                if (++sTotals.unk18 >= 200) {
                    sTotals.unk18 = 199;
                }
                while (((EditorTrap *)gTraps)[sTotals.unk18].unk36 == -1) {
                    if (++sTotals.unk18 >= 200) {
                        sTotals.unk18 = 0;
                    }
                }
            }
            break;
    }
}

/** @brief Runs the StageSprite template's menu editor: on a line change loads
 *         the new line's value and limit, else stores the edited value
 *         back; steps the template's angle on flag bits 3 and 2, then
 *         rebuilds the template at the game position and runs its handler. */
void EditorEditSpriteValues(void) {
    s16 line;
    Bytes8 e;
    s32 h0;
    s32 h1;
    s32 h2;

    line = gMenuLine;
    StepMenuCursor();
    if (line != gMenuLine) {
        switch (gMenuLine) {
            case 0:
                gEditMax = 100;
                gEditValue = sTotals.unk28;
                break;
            case 1:
                gEditMax = 0x200;
                gEditValue = sTotals.unk2C;
                break;
            case 2:
                gEditMax = 200;
                gEditValue = sTotals.unk2A;
                break;
            case 3:
                gEditMax = 1000;
                gEditValue = sTotals.unk2E;
                break;
            case 4:
                gEditMax = 2;
                gEditValue = sTotals.unk32;
                break;
            case 5:
                gEditMax = 2;
                gEditValue = sTotals.unk34;
                break;
            case 6:
                gEditMax = 3;
                gEditValue = sSpriteFlags;
                break;
            case 7:
                gEditMax = 2;
                gEditValue = sTotals.unk6C;
                break;
            case 8:
                gEditMax = 5;
                gEditValue = sTotals.unk6D;
                break;
        }
    } else {
        switch (line) {
            case 0:
                sTotals.unk28 = gEditValue;
                break;
            case 1:
                sTotals.unk2C = gEditValue;
                break;
            case 2:
                sTotals.unk2A = gEditValue;
                break;
            case 3:
                sTotals.unk2E = gEditValue;
                break;
            case 4:
                sTotals.unk32 = gEditValue;
                break;
            case 5:
                sTotals.unk34 = gEditValue;
                break;
            case 6:
                sSpriteFlags = gEditValue;
                break;
            case 7:
                sTotals.unk6C = gEditValue;
                break;
            case 8:
                sTotals.unk6D = gEditValue;
                break;
        }
    }
    if (gPad & PADR1) {
        if (++sTotals.unk6A >= 360) {
            sTotals.unk6A = 0;
        }
    }
    if (gPad & PADL1) {
        if (--sTotals.unk6A < 0) {
            sTotals.unk6A = 359;
        }
    }
    if (sTotals.unk6C != 0) {
        DrawHeadingLine((VECTOR *)&gPlayer.x, sTotals.unk6A);
    }
    sSpriteTemplate.unk4[0] = gPlayer.x;
    sSpriteTemplate.unk4[1] = gPlayer.y;
    sSpriteTemplate.unk4[2] = gPlayer.z;
    GetQuad16(sTotals.unk2C + 30, &e);
    /* MATCHING: each offset in its own s32 local; inline, the sum is
     * narrowed to a halfword add, and one shared local merges the arms. */
    if (sTotals.unk34 == 0) {
        h0 = e.b[6] - 500;
        sSpriteTemplate.unk1C = sTotals.unk2E + h0;
    } else {
        h1 = e.b[6] - 500;
        sSpriteTemplate.unk1C = -(sTotals.unk2E + h1);
    }
    h2 = e.b[7] - 500;
    sSpriteTemplate.unk1E = sTotals.unk2E + h2;
    sSpriteTemplate.unk20 = sSpriteTemplate.unk1C;
    if (sTotals.unk32 == 0) {
        DrawObjectSprite(sTotals.unk2C + 30, &sSpriteTemplate);
    } else {
        StepSprite(sTotals.unk2C + 30, &sSpriteTemplate);
    }
}

/** @brief Fills the StageSprite template at the game position and runs its
 *         handler, counts the used StageSprite records, and on flag bit 5 places
 *         the template at the next slot and moves the slot on to a free
 *         one (error 8 when the table is full).
 *  @return nothing; the value is undefined. */
s32 EditorPlaceSprite(void) {
    u32 i;
    s32 f;

    /* MATCHING: non-void with no return value; void moves the block order
     * and the delay slots. */
    sSpriteTemplate.unk4[0] = gPlayer.x;
    sSpriteTemplate.unk4[1] = gPlayer.y;
    sSpriteTemplate.unk4[2] = gPlayer.z;
    if (sTotals.unk32 == 0) {
        DrawObjectSprite(sTotals.unk2C + 30, &sSpriteTemplate);
    } else {
        StepSprite(sTotals.unk2C + 30, &sSpriteTemplate);
    }
    /* MATCHING: the counter is cleared before the store, so each arm above
     * ends with it (one in a jump's delay slot). */
    i = 0;
    sTotals.unk30 = 0;
    for (; i < 100; i++) {
        if (gStageSprites[i].unk0 != -1) {
            sTotals.unk30++;
        }
    }
    if (gPadTrig & PADRright) {
        if (sTotals.unk30 == 100) {
            gToolMode = 8;
        } else {
            sSpriteTemplate.unk2C = sTotals.unk2A;
            sSpriteTemplate.unk0 = sTotals.unk2C + 30;
            sSpriteTemplate.unk10[0] = sSpriteTemplate.unk4[0];
            sSpriteTemplate.unk10[1] = sSpriteTemplate.unk4[1];
            sSpriteTemplate.unk10[2] = sSpriteTemplate.unk4[2];
            /* MATCHING: an s32 local assigned after the copy, and the flag
             * byte or-ed in by a second store. */
            f = sTotals.unk32 << 7;
            sSpriteTemplate.unk26 = f;
            sSpriteTemplate.unk26 |= sSpriteFlags;
            sSpriteTemplate.unk27 = sTotals.unk6C;
            sSpriteTemplate.unk28 = sTotals.unk6D;
            sSpriteTemplate.unk2A = sTotals.unk6A;
            sSpriteTemplate.unk30 = gSection;
            sSpriteTemplate.unk34 = gCurBlock;
            gStageSprites[sTotals.unk28] = sSpriteTemplate;
            /* MATCHING: the full-table store inside the loop, so its 8 is
             * hoisted and the block is not merged with the one above. */
            i = 0;
            while (1) {
                if (gStageSprites[sTotals.unk28].unk0 == -1) {
                    goto found;
                }
                sTotals.unk28++;
                if (sTotals.unk28 == 100) {
                    sTotals.unk28 = 0;
                }
                if (++i == 100) {
                    gToolMode = 8;
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

/** @brief Runs one of two handlers on every used StageSprite whose unk2C equals
 *         sTotals.unk2A, picked by bit 7 of its unk26. */
void EditorRunSprites(void) {
    u32 i;
    StageSprite *r;

    for (i = 0; i < 100; i++) {
        r = &gStageSprites[i];
        if (r->unk0 != -1 && sTotals.unk2A == r->unk2C) {
            if (r->unk26 & 0x80) {
                StepSprite(r->unk0, r);
            } else {
                DrawObjectSprite(r->unk0, r);
            }
        }
    }
}

/** @brief StageSprite picker, the EditorTrap picker's twin: counts the used StageSprite
 *         records; on the first pass selects the one nearest the game
 *         position (error 9 with none, 11 when none is in range); then
 *         moves the game position to the selection, frees it on flag
 *         bit 5 and steps the selection over the free records on flag
 *         bits 0 and 1.
 *  @return nothing; the value is undefined. */
s32 EditorPickSprite(void) {
    VECTOR pos;
    VECTOR at;
    VECTOR sel;
    StageSprite *r;
    u32 i;
    s32 best;
    s32 d;

    /* MATCHING: non-void with no return keeps the last test's delay slot a
     * nop; the counter cleared before the count, as retail's prologue. */
    i = 0;
    sTotals.unk30 = 0;
    for (; i < 100; i++) {
        if (gStageSprites[i].unk0 != -1) {
            sTotals.unk30++;
        }
    }
    switch (gSubStep) {
        case 0:
            if (sTotals.unk30 == 0) {
                gToolMode = 9;
                break;
            }
            best = 25000000;
            pos.vx = gPlayer.x;
            pos.vy = gPlayer.y;
            pos.vz = gPlayer.z;
            /* MATCHING: the record pointer taken inside the body keeps -1
             * rebuilt every pass. */
            for (i = 0; i < 100; i++) {
                r = &gStageSprites[i];
                if (r->unk0 != -1) {
                    at.vx = r->unk4[0];
                    at.vy = r->unk4[1];
                    at.vz = r->unk4[2];
                    d = DistanceSquared(&pos, &at);
                    if (d < best) {
                        sTotals.unk28 = i;
                        best = d;
                    }
                }
            }
            if (best == 25000000) {
                gToolMode = 11;
            } else {
                gSubStep++;
            }
            break;
        case 1:
            sTotals.unk2A = gStageSprites[sTotals.unk28].unk2C;
            sel.vx = gStageSprites[sTotals.unk28].unk4[0];
            sel.vy = gStageSprites[sTotals.unk28].unk4[1];
            sel.vz = gStageSprites[sTotals.unk28].unk4[2];
            DrawPin(&sel);
            if (D_800959D8 != 1 && (gPadTrig & PADRright)) {
                gSubStep = 0;
                gStageSprites[sTotals.unk28].unk0 = -1;
            }
            if (gPadTrig & PADL2) {
                if (--sTotals.unk28 >= 100) {
                    sTotals.unk28 = 99;
                }
                while (gStageSprites[sTotals.unk28].unk0 == -1) {
                    if (--sTotals.unk28 >= 100) {
                        sTotals.unk28 = 99;
                    }
                }
            }
            if (gPadTrig & PADR2) {
                /* the first step up clamps to 99, the later ones wrap to 0 */
                if (++sTotals.unk28 >= 100) {
                    sTotals.unk28 = 99;
                }
                while (gStageSprites[sTotals.unk28].unk0 == -1) {
                    if (++sTotals.unk28 >= 100) {
                        sTotals.unk28 = 0;
                    }
                }
            }
            break;
    }
}

/** @brief The tool's data-mode state machine: opens the memory card,
 *         steps the data-mode screen (yes/no menus, card and host-file
 *         reads and writes, the save slots of the tool buffer), maps a
 *         card error to a message screen, and closes the card.
 *  @return nothing; the value is undefined. */
s32 UpdateEditorData(void) {
    s32 err;
    /* MATCHING: an s16 answer, both menus a switch on it: case 4's tail
     * then cross-jumps into case 6's. */
    s16 r;

    /* MATCHING: non-void with no return keeps the error table's index
     * shift in its bound check's delay slot. */
    err = 0;
    switch (gSubStep) {
        case 0:
            OpenCardEvents();
            gToolCardStep = 0;
            gMenuLine = 0;
            gEditValue = 0;
            gSubStep++;
            break;
        case 1:
            switch (gToolCardStep) {
                case 0:
                    D_80095A14 = 0;
                    err = RunCardOp1(0);
                    if (err == 0) {
                        gToolCardStep = 1;
                    }
                    break;
                case 1:
                    if (CheckCard() == -1 || (gPadTrig & PADRdown)) {
                        gToolCardStep = 18;
                        break;
                    }
                    gMenuChoice = RunMenu(2);
                    switch (gMenuChoice) {
                        case 2:
                            if (WriteEditorFile() == -1) {
                                gToolCardStep = 11;
                            } else if (WriteEditorSlotFiles() == -1) {
                                gToolCardStep = 11;
                            } else {
                                gToolCardStep = 10;
                            }
                            break;
                        case 3:
                            if (ReadEditorFile() == -1) {
                                gToolCardStep = 11;
                            } else {
                                gMenuChoice = 1;
                                gToolCardStep = 2;
                            }
                            break;
                        case -1:
                            break;
                        default:
                            gToolCardStep = 2;
                            break;
                    }
                    break;
                case 2:
                    if (gPadTrig & PADRdown) {
                        gToolCardStep = 1;
                        break;
                    }
                    gToolSlot = RunMenu(3);
                    if (gToolSlot == -1) {
                        break;
                    }
                    switch (gMenuChoice) {
                        case 0:
                            gToolCardStep = 3;
                            break;
                        case 1:
                            gToolCardStep = 5;
                            break;
                    }
                    break;
                case 10:
                    if (gPadTrig & (PADRdown | PADRright)) {
                        gPlayer.x = gCourse.x;
                        gPlayer.y = gCourse.y;
                        gPlayer.z = gCourse.z;
                        gCourse.rot.vy = gCourse.rot.vz;
                        gFloorIndex = FindFloor(&gPlayer.x);
                        gSection = 0;
                        gView[1] = (s16)(gCourse.y - 500);
                        SetCurrentBlock((BlockHeader *)0x801FD000);
                        gToolCardStep = 1;
                    }
                    break;
                case 3:
                    if ((s16)CheckEditorSlot(gToolSlot) == -1) {
                        gToolCardStep = 8;
                    } else {
                        gToolCardStep = 4;
                        gMenuLine = 0;
                    }
                    break;
                case 4:
                    r = RunMenu(2);
                    switch (r) {
                        case 0:
                            gToolCardStep = 8;
                            break;
                        case 1:
                            gToolCardStep = 1;
                            break;
                    }
                    break;
                case 8:
                    SaveEditorState(gToolSlot);
                    err = RunCardOp1(1);
                    if (err == 0) {
                        gToolCardStep = 10;
                    }
                    break;
                case 5:
                    if ((s16)CheckEditorSlot(gToolSlot) == -1) {
                        gToolCardStep = 7;
                    } else {
                        gToolCardStep = 6;
                        gMenuLine = 0;
                    }
                    break;
                case 6:
                    r = RunMenu(2);
                    switch (r) {
                        case 0:
                            gToolCardStep = 9;
                            break;
                        case 1:
                            gToolCardStep = 1;
                            break;
                    }
                    break;
                case 7:
                    if (gPadTrig & PADRright) {
                        gToolCardStep = 5;
                    }
                    break;
                case 9:
                    RestoreEditorState(gToolSlot);
                    gToolCardStep = 10;
                    break;
                case 11:
                case 13:
                    if (gPadTrig & PADRdown) {
                        gToolCardStep = 18;
                    }
                    break;
                case 14:
                    r = RunMenu(2);
                    switch (r) {
                        case 0:
                            gToolCardStep = 16;
                            break;
                        case 1:
                            gToolCardStep = 18;
                            break;
                        case -1:
                            break;
                    }
                    break;
                case 15:
                    r = RunMenu(2);
                    switch (r) {
                        case 0:
                            gToolCardStep = 17;
                            break;
                        case 1:
                            gToolCardStep = 18;
                            break;
                        case -1:
                            break;
                    }
                    break;
                case 16:
                    err = RunCardOp1(2);
                    if (err == 0) {
                        gToolCardStep = 0;
                    }
                    break;
                case 17:
                    err = RunCardOp1(3);
                    if (err == 0) {
                        gToolCardStep = 0;
                    }
                    break;
                case 18:
                    gSubStep++;
                    break;
            }
            switch (err) {
                case -2:
                case -1:
                    gToolCardStep = 11;
                    break;
                case 1:
                    gToolCardStep = 12;
                    break;
                case 2:
                    gToolCardStep = 13;
                    break;
                case 3:
                    gToolCardStep = 14;
                    break;
                case 4:
                    gToolCardStep = 15;
                    break;
            }
            break;
        case 2:
            CloseCardEvents();
            gToolMode = 0;
            break;
    }
}

/** @brief Prints the data mode's screen for the current memory-card
 *         state, with the save-slot list or a yes/no menu where the state
 *         asks for one, then draws the frame around it. */
void PrintEditorData(void) {
    PrintTabs(5);
    FntPrint(sEdDataModeText);
    FntPrint(sEdNewlines3);
    switch (gToolCardStep) {
        case 0:
            FntPrint(sEdNewlines3);
            PrintTabs(4);
            FntPrint(sEdCheckText);
            break;
        case 1:
            FntPrint(sEdNewlines3);
            PrintTabs(3);
            FntPrint(sEdModeSelectText);
            PrintSaveLoad();
            break;
        case 2:
            FntPrint(sEdNewlines3);
            PrintTabs(3);
            FntPrint(sEdBufferSelectText);
            PrintEditorSlots();
            break;
        case 16:
            FntPrint(sEdNewlines3);
            PrintTabs(3);
            FntPrint(sEdCreateFileText);
            break;
        case 17:
            FntPrint(sEdNewlines3);
            PrintTabs(3);
            FntPrint(sEdCardFormatText);
            break;
        case 4:
            FntPrint(sEdNewlines3);
            PrintTabs(3);
            FntPrint(sEdSaveToDataFmt, gToolSlot + 1);
            PrintTabs(3);
            FntPrint(sEdOverwriteOkText);
            PrintYesNo();
            break;
        case 8:
            FntPrint(sEdNewlines3);
            PrintTabs(3);
            FntPrint(sEdDataWritingText);
            break;
        case 6:
            FntPrint(sEdNewlines3);
            PrintTabs(3);
            FntPrint(sEdLoadFromDataFmt, gToolSlot + 1);
            PrintTabs(3);
            FntPrint(sEdLoadOkText);
            PrintYesNo();
            break;
        case 7:
            FntPrint(sEdNewlines3);
            PrintTabs(2);
            FntPrint(sEdCantOpenDataFmt, gToolSlot + 1);
            break;
        case 10:
            FntPrint(sEdNewlines3);
            PrintTabs(3);
            FntPrint(sEdCompleteText);
            break;
        case 11:
            FntPrint(sEdNewlines3);
            PrintTabs(2);
            FntPrint(sEdErrorText);
            break;
        case 12:
            FntPrint(sEdNewlines3);
            PrintTabs(2);
            FntPrint(sEdNotEnoughMemoryText);
            break;
        case 13:
            FntPrint(sEdNewlines3);
            PrintTabs(3);
            FntPrint(sEdNoCardText);
            break;
        case 14:
            FntPrint(sEdNewlines3);
            PrintTabs(4);
            FntPrint(sEdNoFileText);
            PrintTabs(3);
            FntPrint(sEdCreateNewFileText);
            PrintYesNo();
            break;
        case 15:
            FntPrint(sEdNewlines3);
            PrintTabs(3);
            FntPrint(sEdUnformattedText);
            PrintTabs(3);
            FntPrint(sEdFormatOkText);
            PrintYesNo();
            break;
    }
    SortSemiQuad(0x421, -100, -60, 100, -60, 100, 60, -100, 60, 0);
}

/** @brief Prints the three save slots of the tool buffer as a menu, each
 *         with its stage and part, or "NO DATA". */
void PrintEditorSlots(void) {
    s16 i;
    SaveSlot *slots;
    SaveSlot *slot;
    u16 stage;
    u16 part;
    u16 v;

    slots = (SaveSlot *)0x8016D000;
    FntPrint(sEdNewlines2);
    for (i = 0; i < 3; i++) {
        PrintTabs(3);
        if (gMenuLine == i) {
            FntPrint(sEdColorWhite);
        } else {
            FntPrint(sEdColorBlue);
        }
        slot = &slots[i];
        if (slot->unk200 == 0x38) {
            FntPrint(sEdDataNoFmt, i + 1);
            v = slot->unk202;
            stage = v / 3;
            FntPrint(sEdStageFmt, stage + 1);
            part = v % 3;
            switch (part) {
                case 0:
                    FntPrint(sEdArea1Text);
                    break;
                case 1:
                    FntPrint(sEdArea2Text);
                    break;
                case 2:
                    FntPrint(sEdBossText);
                    break;
            }
            FntPrint(sEdNewline);
        } else {
            FntPrint(sEdNoDataText);
        }
    }
}

/** @brief Saves the record tables, the block header area and the tool
 *         state block into save slot `a` of `buf`. */
void SaveEditorSlot(s16 a, u8 *buf) {
    ((SaveSlot *)buf)[a].unk3AB4 = *(SwitchTable *)gSwitches;
    ((SaveSlot *)buf)[a].unk274 = *(TrapTable *)gTraps;
    /* MATCHING: byte offsets past 0x7FFF build the offset whole (ori). */
    *(SpriteTable *)((u8 *)&((SaveSlot *)buf)[a] + 0x8294) = *(SpriteTable *)gStageSprites;
    *(BlockAreaCopy *)((u8 *)&((SaveSlot *)buf)[a] + 0xB174) = *(BlockAreaCopy *)0x801FD000;
    *(EditorStateCopy *)&((SaveSlot *)buf)[a].unk200 = *(EditorStateCopy *)gEditorState;
}

/** @brief Restores the record tables, the block header area and the tool
 *         state block from save slot `a` of `buf`. */
void LoadEditorSlot(s16 a, u8 *buf) {
    *(SwitchTable *)gSwitches = ((SaveSlot *)buf)[a].unk3AB4;
    *(TrapTable *)gTraps = ((SaveSlot *)buf)[a].unk274;
    /* MATCHING: byte offsets past 0x7FFF build the offset whole (ori). */
    *(SpriteTable *)gStageSprites = *(SpriteTable *)((u8 *)&((SaveSlot *)buf)[a] + 0x8294);
    *(BlockAreaCopy *)0x801FD000 = *(BlockAreaCopy *)((u8 *)&((SaveSlot *)buf)[a] + 0xB174);
    *(EditorStateCopy *)gEditorState = *(EditorStateCopy *)&((SaveSlot *)buf)[a].unk200;
}

extern char sEdDataFilePath0[]; /**< path of slot 0's file on the host */
extern char sEdDataFilePath1[]; /**< path of slot 1's file on the host */
extern char sEdDataFilePath2[]; /**< path of slot 2's file on the host */

/** @brief Writes the four data blocks of each save slot of the tool buffer
 *         to its file on the host.
 *  @return 0, or -1 when a write fails. */
s32 WriteEditorSlotFiles(void) {
    s32 fd;
    s32 n;
    SaveSlot *s;

    s = (SaveSlot *)0x8016D000;
    fd = open(sEdDataFilePath0, O_CREAT | O_WRONLY);
    if (write(fd, &s[0].unk274, 0x3840) == -1) {
        close(fd);
        return -1;
    }
    if (write(fd, &s[0].unk3AB4, 0x47E0) == -1) {
        close(fd);
        return -1;
    }
    if (write(fd, &s[0].unk8294, 0x1770) == -1) {
        close(fd);
        return -1;
    }
    if (write(fd, (u8 *)&s[0] + 0xB174, 0x800) == -1) {
        close(fd);
        return -1;
    }
    close(fd);
    fd = open(sEdDataFilePath1, O_CREAT | O_WRONLY);
    if (write(fd, &s[1].unk274, 0x3840) == -1) {
        close(fd);
        return -1;
    }
    if (write(fd, &s[1].unk3AB4, 0x47E0) == -1) {
        close(fd);
        return -1;
    }
    if (write(fd, &s[1].unk8294, 0x1770) == -1) {
        close(fd);
        return -1;
    }
    if (write(fd, (u8 *)&s[1] + 0xB174, 0x800) == -1) {
        /* MATCHING: the copy ranks n above fd in global alloc (permuter). */
        n = fd;
        close(n);
        return -1;
    }
    close(fd);
    fd = open(sEdDataFilePath2, O_CREAT | O_WRONLY);
    if (write(fd, &s[2].unk274, 0x3840) == -1) {
        close(fd);
        return -1;
    }
    if (write(fd, &s[2].unk3AB4, 0x47E0) == -1) {
        close(fd);
        return -1;
    }
    if (write(fd, &s[2].unk8294, 0x1770) == -1) {
        close(fd);
        return -1;
    }
    n = write(fd, (u8 *)&s[2] + 0xB174, 0x800);
    if (n == -1) {
        close(fd);
        return n;
    }
    close(fd);
    return 0;
}

/** @brief Steps the edited value (bits 0x2000 up, 0x8000 down) or the
 *         highlighted line (0x4000 up, 0x1000 down) from the first pad
 *         word, else from the second and third, then wraps both.
 *  @return 0. */
s32 StepMenuCursor(void) {
    if (gPadTrig & PADLright) {
        gEditValue++;
        WrapMenuValues();
    } else if (gPadTrig & PADLleft) {
        gEditValue--;
        WrapMenuValues();
    } else if (gPadTrig & PADLdown) {
        gMenuLine++;
        WrapMenuValues();
    } else if (gPadTrig & PADLup) {
        gMenuLine--;
        WrapMenuValues();
    } else if (gStickTrigX & PADLright) {
        gEditValue++;
        WrapMenuValues();
    } else if (gStickTrigX & PADLleft) {
        gEditValue--;
        WrapMenuValues();
    } else if (gStickTrigY & PADLdown) {
        gMenuLine++;
        WrapMenuValues();
    } else if (gStickTrigY & PADLup) {
        gMenuLine--;
        WrapMenuValues();
    } else {
        return 0;
    }
    return 0;
}

/** @brief Sorts a pulsing red line from `pos` to 200 units above it into
 *         the current ordering table. */
/* MATCHING: the twin of hit_tool's DrawToolPole (an s16 colour local, an
 * unused 8 bytes for the frame), without its mirrored x and z. */
void DrawPole(VECTOR *pos) {
    s32 unused[2];
    VECTOR world;
    SVECTOR screen;
    GsLINE line;
    s16 g;

    g = ((rsin(gFrameCount * 10 % 360 * 4096 / 360) * 50) >> 12) + 160;
    line.attribute = 0;
    line.r = 0xFF;
    line.g = g;
    line.b = g;
    world.vx = pos->vx + gWorldShift[0];
    world.vy = pos->vy;
    world.vz = pos->vz + gWorldShift[2];
    ProjectToScreen(&world, &screen);
    line.x0 = screen.vx;
    line.y0 = screen.vy;
    world.vy = pos->vy - 200;
    ProjectToScreen(&world, &screen);
    line.x1 = screen.vx;
    line.y1 = screen.vy;
    GsSortLine(&line, &gSubOt[gActiveBuff], 50);
}

/** @brief Sorts a pulsing red line from `pos` to 100 units away from it
 *         along heading `deg` (degrees) into the current ordering table. */
/* MATCHING: the unused 8 and 16 bytes place world at 0x18 and screen at
 * 0x38 in the 0x68-byte frame. */
void DrawHeadingLine(VECTOR *pos, s16 deg) {
    s32 unused[2];
    VECTOR world;
    s32 unused2[4];
    SVECTOR screen;
    GsLINE line;
    s16 g;
    s32 ang;

    g = ((rsin(gFrameCount * 10 % 360 * 4096 / 360) * 50) >> 12) + 160;
    line.attribute = 0;
    line.r = 0xFF;
    line.g = g;
    line.b = g;
    world.vx = pos->vx + gWorldShift[0];
    world.vy = pos->vy;
    world.vz = pos->vz + gWorldShift[2];
    ProjectToScreen(&world, &screen);
    line.x0 = screen.vx;
    line.y0 = screen.vy;
    ang = deg * 4096 / 360;
    world.vx += rsin(ang) * 100 >> 12;
    world.vz += rcos(ang) * 100 >> 12;
    ProjectToScreen(&world, &screen);
    line.x1 = screen.vx;
    line.y1 = screen.vy;
    GsSortLine(&line, &gSubOt[gActiveBuff], 50);
}

/** @brief Wraps the highlighted line at `n` lines, resets the edited value
 *         and runs an update.
 *  @return the highlighted line when flag bit 5 is set, else -1. */
s32 RunMenu(s16 n) {
    gEditValue = 0;
    gMenuLineCount = n;
    gEditMax = 1;
    gMenuLine = gMenuLine % n;
    StepMenuCursor();
    if (gPadTrig & PADRright) {
        return gMenuLine;
    }
    return -1;
}

/** @brief Saves three pieces of game state into the tool state block,
 *         then runs a step on the tool buffer. */
void SaveEditorState(s16 a) {
    sToolSave.unk38[0] = gPlayer.x;
    sToolSave.unk38[1] = gPlayer.y;
    sToolSave.unk38[2] = gPlayer.z;
    sToolSave.unk44 = gCourse.rot.vy;
    sToolSave.unk48[0] = gRefView[0];
    sToolSave.unk48[1] = gRefView[1];
    sToolSave.unk48[2] = gRefView[2];
    sToolSave.unk48[3] = gRefView[3];
    sToolSave.unk48[4] = gRefView[4];
    sToolSave.unk48[5] = gRefView[5];
    SaveEditorSlot(a, (u8 *)0x8016D000);
}

/** @brief Checks that save slot `i` of the tool buffer is valid and
 *         belongs to the current owner.
 *  @return 0 when it does, -1 when not. */
s32 CheckEditorSlot(s16 i) {
    SaveSlot *slot;

    slot = &((SaveSlot *)0x8016D000)[i];
    /* MATCHING: two guards, each returning -1; an && test lays the success
     * path out as the branch target. */
    if (slot->unk200 != 0x38) {
        return -1;
    }
    if (slot->unk202 != gStage) {
        return -1;
    }
    return 0;
}

/** @brief Runs a step on the tool buffer, then restores the game state
 *         the save step put in the tool state block. */
void RestoreEditorState(s16 a) {
    LoadEditorSlot(a, (u8 *)0x8016D000);
    gPlayer.x = sToolSave.unk38[0];
    gPlayer.y = sToolSave.unk38[1];
    gPlayer.z = sToolSave.unk38[2];
    gCourse.rot.vy = sToolSave.unk44;
    gRefView[0] = sToolSave.unk48[0];
    gRefView[1] = sToolSave.unk48[1];
    gRefView[2] = sToolSave.unk48[2];
    gRefView[3] = sToolSave.unk48[3];
    gRefView[4] = sToolSave.unk48[4];
    gRefView[5] = sToolSave.unk48[5];
}

/** @brief Clears and sets up a fixed 0x800-byte block near the top of RAM,
 *         then clears and resets the StageSprite table. */
void ResetEditorBlock(void) {
    bzero((u8 *)0x801FD000, 0x800);
    BuildEmptyBlock((u8 *)0x801FD000);
    bzero((u8 *)gStageSprites, sizeof(StageSprite) * 100);
    ClearStageSprites(gStageSprites);
}

/** @brief Builds an empty block header in `buf` for the current entry
 *         count and points the current-block globals at it. */
void BuildEmptyBlock(u8 *buf) {
    s32 n;

    SetCurrentBlock((BlockHeader *)buf);
    n = gStageObjCount;
    gCanSpans = buf + 8;
    gCanSpanCount = n;
    gCanPoints = buf + (n * 8 + 8);
    /* MATCHING: byte-pointer stores, not BlockHeader members: a member
     * store does not alias the globals, so the reload below would go. */
    *(s32 *)buf = n;
    *(s32 *)(buf + 4) = gCanSpanCount * 8 + 8;
}

/** @brief Sets byte 0 of all 200 records of a SwitchBox table to -1. */
void ClearSwitches(SwitchBox *recs) {
    u32 i;

    for (i = 0; i < 200; i++) {
        recs->unk0 = -1;
        recs++;
    }
}

/** @brief Frees switches 100 to 199 (byte 0 to -1). */
void ClearSwitchBoxUpper(SwitchBox *recs) {
    u32 i;

    recs += 100;
    for (i = 100; i < 200; i++) {
        recs->unk0 = -1;
        recs++;
    }
}

/** @brief Resets all 200 records of a Trap table. */
void ResetTraps(Trap *recs) {
    u32 i;

    for (i = 0; i < 200; i++) {
        recs->unk36 = -1;
        recs->unk38 = -1;
        recs->unk34 = -1;
        recs->unk41 = 1;
        recs++;
    }
}

/** @brief Resets traps 150 to 199. */
void ResetTrapUpper(Trap *recs) {
    u32 i;
    Trap *r;

    r = &recs[150];
    for (i = 150; i < 200; i++) {
        r->unk36 = -1;
        r->unk34 = -1;
        r->unk41 = 1;
        r++;
    }
}

/** @brief Sets the halfword at 0 of all 100 records of a StageSprite table to -1. */
void ClearStageSprites(StageSprite *recs) {
    u32 i;

    for (i = 0; i < 100; i++) {
        recs->unk0 = -1;
        recs++;
    }
}

/** @brief Runs a step on the tool buffer, then copies its first 64 KiB to
 *         the next 64 KiB. */
void BackupToolBuffer(void) {
    SaveEditorSlot(0, (u8 *)0x8016D000);
    *(Page64K *)0x8017D000 = *(Page64K *)0x8016D000;
}

/** @brief Copies the second 64 KiB of the tool buffer back over the first,
 *         then runs a step on it. */
void RestoreToolBuffer(void) {
    *(Page64K *)0x8016D000 = *(Page64K *)0x8017D000;
    LoadEditorSlot(0, (u8 *)0x8016D000);
}

/** @brief Moves the game position `dist` units along heading `deg`
 *         (degrees) relative to the yaw in the first rotation. */
void MoveGamePos(s32 deg, s32 dist) {
    s32 ang;
    SVECTOR *rot;

    rot = &gCourse.rot;
    ang = (deg << 12) / 360;
    gPlayer.x -= rsin(rot->vy + ang) * dist >> 12;
    gPlayer.z -= rcos(rot->vy + ang) * dist >> 12;
}

/** @brief Totals unk4 of the current block's entries into sTotals.unk26. */
void TotalBlockEntries(void) {
    s32 i;
    Ent8 *e;

    e = (Ent8 *)gCanSpans;
    sTotals.unk26 = 0;
    for (i = 0; i < gCanSpanCount; i++) {
        sTotals.unk26 += e->unk4;
        e++;
    }
}

/** @brief Runs an update, then latches a halfword into the first slot of a
 *         halfword run. */
void func_80033B08(void) {
    StepMenuCursor();
    sTotals.canGroup = gEditValue;
}

/** @brief Runs an update; when the highlighted line moved, loads the
 *         edited value and its limit for the new line, else saves the
 *         edited value for the current one. */
void RunValueMenu(void) {
    s16 old;

    old = gMenuLine;
    StepMenuCursor();
    if (old != gMenuLine) {
        switch (gMenuLine) {
            case 0:
                gEditMax = 200;
                gEditValue = sTotals.unk12;
                break;
            case 1:
                gEditMax = 3;
                gEditValue = sTotals.unk14;
                break;
        }
    } else {
        switch (old) {
            case 0:
                sTotals.unk12 = gEditValue;
                break;
            case 1:
                sTotals.unk14 = gEditValue;
                break;
        }
    }
}

/** @brief Applies every used Trap whose unk34 equals sTotals.unk1E to the
 *         ObjectModel its unk36 names. */
void ApplyRecordsToEntries(void) {
    u32 i;
    Trap *r;

    for (i = 0; i < 200; i++) {
        r = &((Trap *)gTraps)[i];
        if (r->unk36 != -1 && sTotals.unk1E == r->unk34) {
            UpdateObjectEntry(&gObjectModels[r->unk36], r);
        }
    }
}

/** @brief Prints a two-line YES/NO menu, highlighting the line
 *         the menu cursor selects. */
void PrintYesNo(void) {
    FntPrint(sEdNewlines2);
    PrintTabs(4);
    if (gMenuLine == 0) {
        FntPrint(sEdColorWhite);
    } else {
        FntPrint(sEdColorBlue);
    }
    FntPrint(sEdYesText);
    PrintTabs(4);
    if (gMenuLine == 1) {
        FntPrint(sEdColorWhite);
    } else {
        FntPrint(sEdColorBlue);
    }
    FntPrint(sEdNoText);
}

/** @brief Prints a two-line SAVE/LOAD menu, highlighting the line
 *         the menu cursor selects. */
void PrintSaveLoad(void) {
    FntPrint(sEdNewlines2);
    PrintTabs(4);
    if (gMenuLine == 0) {
        FntPrint(sEdColorWhite);
    } else {
        FntPrint(sEdColorBlue);
    }
    FntPrint(sEdSaveText);
    PrintTabs(4);
    if (gMenuLine == 1) {
        FntPrint(sEdColorWhite);
    } else {
        FntPrint(sEdColorBlue);
    }
    FntPrint(sEdLoadText);
}

/** @brief Writes the tool buffer to the tool file on the host.
 *  @return the count written. */
s32 WriteEditorFile(void) {
    s32 fd;
    s32 n;

    fd = open(sEdTmpFilePath, O_CREAT | O_WRONLY);
    n = write(fd, (void *)0x8016D000, 0x2285C);
    close(fd);
    return n;
}

/** @brief Reads the tool file on the host into the tool buffer.
 *  @return the count read. */
s32 ReadEditorFile(void) {
    s32 fd;
    s32 n;

    fd = open(sEdTmpFilePath, O_RDONLY);
    n = read(fd, (void *)0x8016D000, 0x2285C);
    close(fd);
    return n;
}

/** @brief Wraps the edited value and the highlighted line into their
 *         ranges: below 0 to the top, at or past the limit to 0.
 *  @return 0. */
s32 WrapMenuValues(void) {
    /* MATCHING: retail reads the edited value signed (lh); common.h's
     * declaration is u16. */
    if ((s16)gEditValue < 0) {
        gEditValue = gEditMax - 1;
    }
    if ((s16)gEditValue >= gEditMax) {
        gEditValue = 0;
    }
    if (gMenuLine < 0) {
        gMenuLine = gMenuLineCount - 1;
    }
    if (gMenuLine >= gMenuLineCount) {
        gMenuLine = 0;
    }
    return 0;
}
