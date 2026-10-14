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
    s16 id;       /**< its sprite (MODEL. plus 30); -1 when free */
    u8 unk2[2];   /**< not yet known */
    s32 pos[3];   /**< a position */
    s32 drift[3]; /**< the position again when placed; object drifts it */
    s16 width;    /**< a width from the template's entry and SIZE. */
    s16 height;   /**< a height from the template's entry and SIZE. */
    s32 radius;   /**< the width again */
    u8 unk24[2];  /**< not yet known */
    u8 flags;     /**< SpriteFlag bits and a SpriteAct */
    u8 walk;      /**< sTotals.spriteWalk when placed */
    u8 se;        /**< sTotals.spriteSe when placed */
    u8 unk29;     /**< not yet known */
    u16 angle;    /**< sTotals.spriteAngle when placed */
    s16 switchNo; /**< sTotals.spriteSwitch when placed */
    u8 unk2E[2];  /**< not yet known */
    s32 section;  /**< gSection when placed */
    s32 block;    /**< the current block when placed */
    u8 unk38[4];  /**< not yet known */
} StageSprite;

/** @brief The editor's modes, gToolMode, named from each screen's title. */
enum EditorMode {
    ED_MOVE = 0,       /**< "MOVE MODE": moves the cursor, places or picks */
    ED_MENU = 1,       /**< "MENU MODE" */
    ED_PEPSI_SEL = 2,  /**< "PEPSI SEL MODE": the can group */
    ED_SWITCH_SEL = 3, /**< "SWITCH SEL MODE" */
    ED_TRAP_SEL = 4,   /**< "TRAP SEL MODE" */
    ED_SPRITE_SEL = 5, /**< "SPR SEL MODE" */
    ED_DATA = 6,       /**< "DATA MODE": the memory card screen */
    ED_NO_GROUND = 7,  /**< warning: no ground hit here, nothing can be placed */
    ED_FULL = 8,       /**< warning: the buffer is full */
    ED_EMPTY = 9,      /**< warning: the buffer is empty, nothing to delete */
    ED_NONE_HERE = 10, /**< warning: nothing in this area to delete */
    ED_NONE_NEAR = 11  /**< warning: no trap or switch near enough to erase */
};

/** @brief What the editor places and picks, EditorState.kind: the first
 *         four lines of its menu. */
enum EditKind {
    EK_PEPSI = 0,  /**< Pepsi can points */
    EK_SWITCH = 1, /**< switch boxes */
    EK_TRAP = 2,   /**< traps */
    EK_SPRITE = 3  /**< stage sprites */
};

/** @brief The editor's state block: the cursor, the counts, and each SEL
 *         screen's values, named from the labels it prints. */
typedef struct {
    u8 magic;   /**< EDITOR_MAGIC, the mark a saved slot is checked for */
    u8 unk1;    /**< 1 after a reset */
    u8 stage;   /**< gStage when the state was reset */
    u8 kind;    /**< what is placed and picked, an EditKind */
    s32 grid;   /**< the move and height step, " GRID"; 2, 10, 50 or 100 */
    s32 height; /**< the cursor's height above the ground; the y readout is red above, blue below */
    s32 camHeight;   /**< the camera's height offset, 0 or below */
    u8 unk10[2];     /**< not yet known */
    u16 switchNo;    /**< "SWITCH SEL" NO.: the switch slot edited */
    u16 switchType;  /**< "SWITCH SEL" TYPE.: the box size, 0 to 2 */
    u16 switchCount; /**< switches in use, "SWITCH NO" */
    u16 trapNo;      /**< "TRAP SEL" NO.: the trap slot the next goes to, or the one picked */
    u16 trapCount;   /**< traps counted live, "TRAP NO" */
    u16 trapModel;   /**< "TRAP SEL" MODEL.: the trap's stage hook */
    u16 trapSwitch;  /**< "TRAP SEL" SWITCH.: the switch that triggers it */
    s16 trapAngle;   /**< the trap's angle in degrees, 0..359 */
    u16 canNo;       /**< "CAN NO.": the selected can of the current block */
    u16 canGroup; /**< the group the next can gets, less one; the edited value saved for ED_PEPSI_SEL */
    u16 canCount; /**< cans in the current block, "PEPSI NO" */
    u16 spriteNo; /**< "SPRITE SEL" NO.: the StageSprite slot the next goes to, or the one picked */
    u16 spriteSwitch; /**< "SPRITE SEL" SWITCH. */
    u16 spriteModel;  /**< "SPRITE SEL" MODEL.: the sprite kind, less 30 */
    u16 spriteSize;   /**< "SPRITE SEL" SIZE., shown less 500 */
    u16 spriteCount;  /**< StageSprites in use */
    u16 spriteHit;    /**< "SPRITE SEL" HIT.: off or on; picks one of two StageSprite handlers */
    u16 spriteFlip;   /**< "SPRITE SEL" FLIP.: off or on; negates the template's width */
    u8 unk36[2];      /**< not yet known */
    s32 savedPos[3];  /**< gPlayer's x, y and z, kept by SaveEditorState */
    s16 savedYaw;     /**< gCourse.rot.vy, kept with them */
    u8 unk46[2];      /**< not yet known */
    s32 savedView[6]; /**< gRefView's first six words, kept with them */
    u8 unk60[8];      /**< not yet known */
    u8 trapType;      /**< "TRAP SEL" TYPENO. */
    u8 trapAct;       /**< "TRAP SEL" ACT NO. */
    s16 spriteAngle;  /**< the sprite's angle in degrees, 0..359 */
    u8 spriteWalk;    /**< "SPRITE SEL" WALK.: 0 off, else a number */
    u8 spriteSe;      /**< "SPRITE SEL" SE.: 0 off, 1 man, 2 woman, 3 child, 4 dog */
    u8 unk6E[2];      /**< not yet known */
    s32 savedSection; /**< gSection, saved on leaving the tool */
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
 *         the slot's data starts at state and its block header area runs
 *         0x200 bytes past this view's end. */
typedef struct {
    u8 unk0[0x200];    /**< not yet known */
    EditorState state; /**< the editor's state block; magic EDITOR_MAGIC when the slot is valid */
    TrapTable traps;   /**< the EditorTrap table */
    SwitchTable switches; /**< the switches */
    SpriteTable sprites;  /**< the StageSprite table */
    u8 unk9A04[0x1D70];   /**< not yet known; the block area starts at 0xB174 */
} SaveSlot;

/* MATCHING: gStageSprites and gEditorState are also declared in object, each
 * unit through its own view (StageSprite, EditorState here; StageSprite, EditorReset there). */
extern StageSprite gStageSprites[]; /**< 100 StageSprite records */
extern u8 gEditorState[];           /**< the editor's state, an EditorState */
#define sTotals (*(EditorState *)gEditorState)
extern char sEdNewlines2[];     /**< "\n\n" */
extern char sEdColorWhite[];    /**< colour code of a highlighted menu line, "~c888" */
extern char sEdColorBlue[];     /**< colour code of a plain menu line, "~c008" */
extern char sEdYesText[];       /**< "  YES\n" */
extern char sEdNoText[];        /**< "  NO" */
extern char sEdSaveText[];      /**< "  SAVE\n" */
extern char sEdLoadText[];      /**< "  LOAD\n" */
extern char sEdArea1Text[];     /**< "1" */
extern char sEdArea2Text[];     /**< "2" */
extern char sEdBossText[];      /**< "BOSS" */
extern char sEdNewline[];       /**< "\n" */
extern char sEdInfoHitLabel[];  /**< "HIT. " */
extern char sEdInfoOffText[];   /**< " OFF / " */
extern char sEdInfoOnText[];    /**< " ON  / " */
extern char sEdInfoActLabel[];  /**< "ACT. " */
extern char sEdInfoWalkLabel[]; /**< "WALK." */
extern char sEdInfoOffLine[];   /**< " OFF\n" */
extern char sEdInfoSeLabel[];   /**< "SE. " */


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
    s32 pos[3];      /**< a position */
    s32 home[3];     /**< where it was placed */
    s16 rotX;        /**< rotation; cleared when the first record is taken */
    s16 rotY;        /**< the heading, 0..4095 */
    s16 rotZ;        /**< cleared when the first record is taken */
    u8 unk1E[2];     /**< not yet known */
    u16 homeRotY;    /**< the placed heading, as an offset from the first record's */
    u8 unk22[2];     /**< not yet known */
    s16 unk24;       /**< cleared when placed */
    s16 unk26;       /**< cleared when placed */
    s32 block;       /**< the current block when placed */
    s32 lightAngle;  /**< an angle from the light direction */
    s32 lightAngle2; /**< a second angle from the light direction */
    s16 switchNo;    /**< sTotals.trapSwitch when placed */
    s16 model;       /**< its ObjectModel entry; -1 when free */
    s16 hook;        /**< sTotals.trapModel for the first placed, else -1 */
    u8 unk3A[2];     /**< not yet known */
    s32 section;     /**< gSection when placed */
    u8 touched;      /**< cleared when placed */
    u8 marks;        /**< 1 when reset */
    u8 type;         /**< sTotals.trapType when placed */
    u8 act;          /**< sTotals.trapAct when placed */
    u8 unk44[4];     /**< not yet known */
} EditorTrap;

/* MATCHING: object declares gEditorInfo as u8 (it only zeroes it); this
 * unit tests it as s8. */
extern EditorTrap gPlaceQueue[]; /**< records waiting to be placed */
extern u16 gPlaceCount;          /**< number of records waiting */
extern s8 gEditorInfo;           /**< 1 when picking shows INFO instead of deleting */

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
extern u8 gEditorPicking; /**< 1 while ED_MOVE picks (DELETE or INFO), 0 while it places */

/* MATCHING: object declares it as its SwitchBox records. */
extern s32 gSwitches[]; /**< the 200 switches */

/** @brief A switch (gSwitches), as this unit
 *         places it. */
typedef struct {
    s8 state;      /**< a SwitchState */
    u8 unk1[0x4F]; /**< not yet known */
    s32 pos[3];    /**< its position */
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
extern char sEdNewlines3[]; /**< "\n\n\n" */
extern char sEdCheckText[]; /**< "CHECK" */

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
void EditPepsiGroup(void);
void RunValueMenu(void);
void ApplyRecordsToEntries(void);
s32 PrintEditorStatus(void);
/* MATCHING: player tests it as s8. */
extern u8 gToolExited; /**< set on leaving the tool */

extern StageSprite sSpriteTemplate; /**< the StageSprite template that gets placed */
extern u8 sSpriteAct; /**< "SPRITE SEL" ACT., a SpriteAct; or-ed into a placed sprite's flags */

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

const char sEdPositionFmt[] = "\nPOSITION X %d "; /**< the editor's position format */

const char sEdSwitchNoFmt[] = "\nSWITCH NO(%2d/ %2d)"; /**< the editor's switch number format */

const char sEdTrapNoFmt[] = " TRAP NO(%3d/ %3d)"; /**< the editor's trap number format */

const char sEdPepsiNoFmt[] = "\nPEPSI NO(%3d/ %3d)"; /**< the editor's pepsi number format */

const char sEdItemText[] = " ITEM : "; /**< the editor's item text */

const char sEdPepsiFmt[] = "PEPSI %2d"; /**< the editor's pepsi format */

const char sEdSwitchFmt[] = "SWITCH %2d"; /**< the editor's switch format */

const char sEdTrapFmt[] = "TRAP %2d"; /**< the editor's trap format */

const char sEdSpriteFmt[] = "SPRITE %2d"; /**< the editor's sprite format */

const char sEdInfoText[] = "INFO    "; /**< the editor's info text */

const char sEdDeleteText[] = "DELETE  "; /**< the editor's delete text */

const char sEdMoveModeText[] = "MOVE MODE"; /**< the editor's move mode text */

const char sEdMenuModeText[] = "MENU MODE"; /**< the editor's menu mode text */

const char sEdPepsiSelModeText[] = "PEPSI SEL MODE"; /**< the editor's pepsi select mode text */

const char sEdNewlines9[] = "\n\n\n\n\n\n\n\n\n"; /**< nine newlines */

const char sEdPepsiTypeSelText[] = "PEPSI TYPE SEL\n\n"; /**< the editor's pepsi type select text */

const char sEdPepsiTypeRangeFmt[] = "   1<-%2d->200"; /**< the editor's pepsi type range format */

const char sEdSwitchSelModeText[] = "SWITCH SEL MODE"; /**< the editor's switch select mode text */

const char sEdNewlines8[] = "\n\n\n\n\n\n\n\n"; /**< eight newlines */

const char sEdSwitchSelText[] = "  SWITCH SEL\n\n"; /**< the editor's switch select text */

const char sEdSwitchNoRangeText[] = "NO.  0<-"; /**< the editor's switch number range text */

const char sEdSwitchTypeRangeFmt[] = "TYPE. 0<-%1d->%d"; /**< the editor's switch type range format */

const char sEdTrapSelModeText[] = "TRAP SEL MODE"; /**< the editor's trap select mode text */

const char sEdTrapSelText[] = "   TRAP  SEL\n\n"; /**< the editor's trap select text */

const char sEdTrapNoRangeText[] = "NO.    0<-"; /**< the editor's trap number range text */

const char sEdTrapModelRangeFmt[] = "MODEL. 0<-%2d->%2d\n"; /**< the editor's trap model range format */

const char sEdUseBufferFmt[] = "   USE BUFFER %d\n"; /**< the editor's use buffer format */

const char sEdTrapSwitchRangeText[] = "SWITCH.0<-"; /**< the editor's trap switch range text */

const char sEdTrapTypeRangeText[] = "TYPENO.0<-"; /**< the editor's trap type range text */

const char sEdTrapActRangeText[] = "ACT NO.0<-"; /**< the editor's trap act range text */

const char sEdAngleRangeFmt[] = "  L1 0<-%3d->360 R1\n"; /**< the editor's angle range format */

const char sEdSpriteSelModeText[] = "SPR SEL MODE"; /**< the editor's sprite select mode text */

const char sEdSpriteSelText[] = "   SPRITE SEL\n\n"; /**< the editor's sprite select text */

const char sEdSpriteNoRangeText[] = "  NO.    0<-"; /**< the editor's sprite number range text */

const char sEdSpriteModelRangeFmt[] = "  MODEL. 0<-%3d->%3d\n"; /**< the editor's sprite model range format */

const char sEdSpriteSwitchRangeText[] = "  SWITCH.0<-"; /**< the editor's sprite switch range text */

const char sEdSpriteSizeRangeFmt[] = "  SIZE. 0<-%3d->%3d\n"; /**< the editor's sprite size range format */

const char sEdHitLabel[] = "  HIT.  "; /**< the editor's hit label */

const char sEdFlipLabel[] = "  FLIP.  "; /**< the editor's flip label */

const char sEdActLabel[] = "  ACT.   "; /**< the editor's act label */

const char sEdNearActText[] = "NEAR ACT\n"; /**< the editor's near act text */

const char sEdHitActText[] = "HIT ACT\n"; /**< the editor's hit act text */

const char sEdWalkLabel[] = "  WALK.  "; /**< the editor's walk label */

const char sEdSeLabel[] = "  SE.    "; /**< the editor's sound effect label */

const char sEdWarningText[] = "!!! WARNING !!!"; /**< the editor's warning text */

const char sEdCansTrapsLine[] = "PEPSICAN TO TRAP TO"; /**< the editor's cans traps line */

const char sEdSwitchesCantPlaceLine[] = "SWITCH GA OKEMASEN."; /**< the editor's switches cant place line */

const char sEdGroundHitLine[] = "JIMEN NO HIT GA"; /**< the editor's ground hit line */

const char sEdUntakeableHereLine[] = "TORENAI BASHO NI"; /**< the editor's untakeable here line */

const char sEdPushCrossText[] = "            PUSH BATU"; /**< the editor's push cross text */

const char sEdBufferMaxLine[] = "BAFFER GA MAX NI NARI"; /**< the editor's buffer max line */

const char sEdReachedNoMoreLine[] = "MASITA. KORE IJYOU HA"; /**< the editor's reached no more line */

const char sEdCantPlaceLine[] = "OKEMASEN."; /**< the editor's cant place line */

const char sEdAskMiyagoeLine[] = "MIYAGOE SAMA NI KIITE"; /**< the editor's ask miyagoe line */

const char sEdPleaseLine[] = "KUDASAI."; /**< the editor's please line */

const char sEdSwitchesDeleteLine[] = "SWITCH GA SAKUJYO"; /**< the editor's switches delete line */

const char sEdCannotLine[] = "DEKIMASEN."; /**< the editor's cannot line */

const char sEdBufferZeroLine[] = "BAFFER GA ZERO NI NARI"; /**< the editor's buffer zero line */

const char sEdCantDeleteLine[] = "SAKUJYO DEKIMASEN."; /**< the editor's cant delete line */

const char sEdAreaDeleteLine[] = "KONO AREA NI SAKUJYO"; /**< the editor's area delete line */

const char sEdDeletableLine[] = "DEKIRU MONO HA"; /**< the editor's deletable line */

const char sEdThereAreNoneLine[] = "ARIMASEN."; /**< the editor's there are none line */

const char sEdTrapsSwitchesLine[] = "TRAP TO SWITCH GA"; /**< the editor's traps switches line */

const char sEdNotNearbyLine[] = "CHIKAKU NI ARIMASEN."; /**< the editor's not nearby line */

const char sEdWhatToEraseLine[] = "KESITAI MONO NO "; /**< the editor's what to erase line */

const char sEdMoveNearLine[] = "CHIKAKU NI IDOU"; /**< the editor's move near line */

const char sEdPleaseDoLine[] = "SITEKUDASAI."; /**< the editor's please do line */

const char sEdCanNoFmt[] = "\n\nCAN NO.  %d\n"; /**< the editor's can number format */

const char sEdCanNumFmt[] = "CAN NUM. %d\n"; /**< the editor's can count format */

const char sEdInfoSwitchNoFmt[] = "\n\nSWITCH NO.%d\n"; /**< the editor's info switch number format */

const char sEdInfoTrapNoFmt[] = "\n\nTRAP NO. %d / "; /**< the editor's info trap number format */

const char sEdInfoTrapModelFmt[] = "MODEL.%2d / "; /**< the editor's info trap model format */

const char sEdOptionsText[] = "OPTIONS  / "; /**< the editor's options text */

const char sEdInfoTrapSwitchFmt[] = "SWITCH NO.%d\n"; /**< the editor's info trap switch format */

const char sEdInfoTypeFmt[] = "TYPE. %d / "; /**< the editor's info type format */

const char sEdInfoActFmt[] = "ACT. %d\n"; /**< the editor's info act format */

const char sEdInfoSpriteFmt[] = "\n\nSPRITE. %d / "; /**< the editor's info sprite format */

const char sEdInfoSpriteModelFmt[] = "MODEL. %d / "; /**< the editor's info sprite model format */

const char sEdInfoSpriteSwitchFmt[] = "SWITCH. %d\n"; /**< the editor's info sprite switch format */

const char sEdNoActText[] = "NO ACT  / "; /**< the editor's no act text */

const char sEdInfoNearActText[] = "NEAR ACT/ "; /**< the editor's info near act text */

const char sEdInfoHitActText[] = "HIT ACT / "; /**< the editor's info hit act text */

const char sEdInfoWalkTypeFmt[] = "TYPE %d\n"; /**< the editor's info walk type format */

const char sEdNoSeText[] = "NO SE / "; /**< the editor's no sound effect text */

const char sEdManText[] = "MAN   / "; /**< the editor's man text */

const char sEdWomanText[] = "WOMAN / "; /**< the editor's woman text */

const char sEdChildText[] = "CHILD / "; /**< the editor's child text */

const char sEdDogText[] = "DOG   / "; /**< the editor's dog text */

const Sizes3 sBoxSizes = {{{2000, 2000}, {1000, 1000}, {4000, 4000}}}; /**< the three box sizes (width, depth) */

const char sEdDataModeText[] = "DATA MODE"; /**< the editor's data mode text */

const char sEdModeSelectText[] = "  MODE SELECT\n\n"; /**< the editor's mode select text */

const char sEdBufferSelectText[] = "BUFFER SELECT\n\n"; /**< the editor's buffer select text */

const char sEdCreateFileText[] = "  CREATE FILE"; /**< the editor's create file text */

const char sEdCardFormatText[] = "  CARD FORMAT"; /**< the editor's card format text */

const char sEdSaveToDataFmt[] = "SAVE TO DATA %d\n\n"; /**< the editor's save to data format */

const char sEdOverwriteOkText[] = "OVER WRITE OK ?"; /**< the editor's overwrite ok text */

const char sEdDataWritingText[] = "DATA WRITING..."; /**< the editor's data writing text */

const char sEdLoadFromDataFmt[] = "LOAD TO DATA %d\n\n"; /**< the editor's load from data format */

const char sEdLoadOkText[] = "    LOAD OK ?"; /**< the editor's load ok text */

const char sEdCantOpenDataFmt[] = "CAN NOT OPEN DATA %d"; /**< the editor's cant open data format */

const char sEdCompleteText[] = "   COMPLATE"; /**< the editor's complete text */

const char sEdErrorText[] = "    !!!! ERROR!!!!"; /**< the editor's error text */

const char sEdNotEnoughMemoryText[] = "   NOT ENOUGH MEMORY"; /**< the editor's not enough memory text */

const char sEdNoCardText[] = "NO CONNECT CARD"; /**< the editor's no card text */

const char sEdNoFileText[] = "NO FILE\n\n"; /**< the editor's no file text */

const char sEdCreateNewFileText[] = "CREATE NEW FILE ?"; /**< the editor's create new file text */

const char sEdUnformattedText[] = "    UN FORMAT\n\n"; /**< the editor's unformatted text */

const char sEdFormatOkText[] = "   FORMAT OK ?"; /**< the editor's format ok text */

const char sEdDataNoFmt[] = "DATA:%d "; /**< the editor's data number format */

const char sEdStageFmt[] = "STAGE %d-"; /**< the editor's stage format */

const char sEdNoDataText[] = "NO DATA\n"; /**< the editor's no data text */

const char sEdTmpFilePath[] = "sim:\\PS\\PEPSI\\DATA\\TOOL0\\TMP.TL0"; /**< path of the tool file on the host */

const char sEdDataFilePath0[] =
    "sim:\\PS\\PEPSI\\DATA\\TOOL0\\DATA0.TLD"; /**< path of slot 0's file on the host */

const char sEdDataFilePath1[] =
    "sim:\\PS\\PEPSI\\DATA\\TOOL0\\DATA1.TLD"; /**< path of slot 1's file on the host */

const char sEdDataFilePath2[] =
    "sim:\\PS\\PEPSI\\DATA\\TOOL0\\DATA2.TLD"; /**< path of slot 2's file on the host */

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
            gToolMode = ED_MOVE;
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
            /* MATCHING: the s16 mode switched as u16 (lhu). */
            switch ((u16)gToolMode) {
                case ED_MOVE:
                    EditorFreeMove();
                    if (gEditorPicking == 0) {
                        switch (sTotals.kind) {
                            case EK_PEPSI:
                                EditorInsertPoint();
                                break;
                            case EK_SWITCH:
                                EditorPlaceBox();
                                TestBoxes();
                                break;
                            case EK_TRAP:
                                EditorPlaceRecords();
                                EditorArrangeRecords();
                                break;
                            case EK_SPRITE:
                                EditorPlaceSprite();
                                break;
                        }
                    } else {
                        switch (sTotals.kind) {
                            case EK_PEPSI:
                                EditorEditPoints();
                                break;
                            case EK_SWITCH:
                                EditorPickBox();
                                TestBoxes();
                                break;
                            case EK_TRAP:
                                EditorPickRecord();
                                if (gEditorInfo == 1 && (gPadTrig & PADRright)) {
                                    gPlayer.x = ((EditorTrap *)gTraps)[sTotals.trapNo].pos[0];
                                    gPlayer.y = ((EditorTrap *)gTraps)[sTotals.trapNo].pos[1];
                                    gPlayer.z = ((EditorTrap *)gTraps)[sTotals.trapNo].pos[2];
                                    sTotals.trapModel = ((EditorTrap *)gTraps)[sTotals.trapNo].hook;
                                    gToolMode = ED_TRAP_SEL;
                                    sTotals.trapSwitch = ((EditorTrap *)gTraps)[sTotals.trapNo].switchNo;
                                    sTotals.trapType = ((EditorTrap *)gTraps)[sTotals.trapNo].type;
                                    sTotals.trapAct = ((EditorTrap *)gTraps)[sTotals.trapNo].act;
                                    sTotals.trapAngle =
                                        ((EditorTrap *)gTraps)[sTotals.trapNo].rotY * 360 / 4096;
                                    gPlaceCount =
                                        CallStageHook(sTotals.trapModel, gPlaceQueue, gPlaceQueue);
                                    for (i = 1; i < gPlaceCount; i++) {
                                        /* MATCHING: base taken in the body */
                                        r = (EditorTrap *)gTraps;
                                        r[sTotals.trapNo + i].model = -1;
                                    }
                                    gEditMax = 200;
                                    gEditorPicking = 0;
                                    gMenuLineCount = 5;
                                    gMenuLine = 0;
                                    gSubStep = 0;
                                    gEditValue = sTotals.trapNo;
                                }
                                break;
                            case EK_SPRITE:
                                EditorPickSprite();
                                if (gEditorInfo == 1 && (gPadTrig & PADRright)) {
                                    gPlayer.x = gStageSprites[sTotals.spriteNo].pos[0];
                                    gPlayer.y = gStageSprites[sTotals.spriteNo].pos[1];
                                    gPlayer.z = gStageSprites[sTotals.spriteNo].pos[2];
                                    sTotals.spriteModel = gStageSprites[sTotals.spriteNo].id - 30;
                                    gToolMode = ED_SPRITE_SEL;
                                    sTotals.spriteSwitch = gStageSprites[sTotals.spriteNo].switchNo;
                                    sTotals.spriteSe = gStageSprites[sTotals.spriteNo].se;
                                    sTotals.spriteWalk = gStageSprites[sTotals.spriteNo].walk;
                                    gEditMax = 100;
                                    gEditorPicking = 0;
                                    gMenuLineCount = 9;
                                    gEditValue = sTotals.spriteNo;
                                    gMenuLine = 0;
                                    gSubStep = 0;
                                    sTotals.spriteAngle = gStageSprites[sTotals.spriteNo].angle;
                                }
                                break;
                        }
                    }
                    if (gPadTrig & PADRup) {
                        gToolMode = ED_MENU;
                        gEditValue = 0;
                        gMenuLine = 0;
                    }
                    break;
                case ED_MENU:
                    UpdateEditorMenu();
                    if (gPadTrig & PADRdown) {
                        if (gMenuLine == 2) {
                            gSubStep = 0;
                        }
                        gToolMode = ED_MOVE;
                    }
                    break;
                case ED_PEPSI_SEL:
                    EditPepsiGroup();
                    if (gPadTrig & PADRdown) {
                        gToolMode = ED_MOVE;
                    }
                    break;
                case ED_SWITCH_SEL:
                    RunValueMenu();
                    if (gPadTrig & PADRdown) {
                        gToolMode = ED_MOVE;
                    }
                    break;
                case ED_TRAP_SEL:
                    EditorEditRecordValues();
                    EditorArrangeRecords();
                    if (gPadTrig & PADRdown) {
                        gToolMode = ED_MOVE;
                    }
                    break;
                case ED_SPRITE_SEL:
                    EditorEditSpriteValues();
                    if (gPadTrig & PADRdown) {
                        gToolMode = ED_MOVE;
                    }
                    break;
                case ED_DATA:
                    UpdateEditorData();
                    break;
                case ED_NO_GROUND:
                case ED_FULL:
                    if (gPadTrig & PADRdown) {
                        gToolMode = ED_MOVE;
                    }
                    break;
                case ED_EMPTY:
                case ED_NONE_HERE:
                case ED_NONE_NEAR:
                    if (gPadTrig & PADRdown) {
                        gToolMode = ED_MOVE;
                        gEditorPicking = 0;
                        gEditorInfo = 0;
                    }
                    break;
            }
            if (gEditorPicking == 0) {
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
        sTotals.savedSection = gSection;
        BackupToolBuffer();
        gRetries = 0xFF;
        gToolExited = 1;
        ResetStageObjects();
        gModeStep = 3;
        gGameMode = GM_STAGE;
    }
    if (gPadTrig & PADselect) {
        switch (sTotals.grid) {
            case 50:
                sTotals.grid = 100;
                break;
            case 100:
                sTotals.grid = 2;
                break;
            case 1000:
                sTotals.grid = 10;
                break;
            default:
                sTotals.grid = 50;
                break;
        }
    }
}

extern char sEdColorCyan[];        /**< "~c088" */
extern char sEdColorRed[];         /**< "~c800" */
extern char sEdPosYFmt[];          /**< "Y %d " */
extern char sEdPosZFmt[];          /**< "Z %d" */
extern char sEdMenuPepsiText[];    /**< "PEPSI\n" */
extern char sEdMenuSwitchText[];   /**< "SWITCH\n" */
extern char sEdMenuTrapText[];     /**< "TRAP\n" */
extern char sEdMenuSpriteText[];   /**< "SPRITE\n" */
extern char sEdMenuDeleteText[];   /**< "DELETE\n" */
extern char sEdMenuInfoText[];     /**< "INFO\n" */
extern char sEdMenuDefPosText[];   /**< "DEFPOS\n" */
extern char sEdMenuDataText[];     /**< "DATA\n" */
extern char sEdColorGrey[];        /**< "~c444" */
extern char sEdNum2Fmt[];          /**< "%2d" */
extern char sEdRangeEndBlankFmt[]; /**< "->%d\n\n" */
extern char sEdNewlines4[];        /**< "\n\n\n\n" */
extern char sEdRangeEndFmt[];      /**< "->%d\n" */
extern char sEdRangeEnd2Fmt[];     /**< "->%2d\n" */
extern char sEdNewlines5[];        /**< "\n\n\n\n\n" */
extern char sEdOffLine[];          /**< "OFF\n" */
extern char sEdOnLine[];           /**< "ON\n" */
extern char sEdNoActLine[];        /**< "NO ACT\n" */
extern char sEdSeNoFmt[];          /**< "NO %3d\n" */
extern char sEdManLine[];          /**< "MAN\n" */
extern char sEdWomanLine[];        /**< "WOMAN\n" */
extern char sEdChildLine[];        /**< "CHILD\n" */
extern char sEdDogLine[];          /**< "DOG\n" */
extern char sEdYouAreLine[];       /**< "IMASU." */
extern VECTOR sStatusPos;          /**< the position printed on the status panel */

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
    FntPrint(gGridFmt, sTotals.grid);
    FntPrint(gNowBlockFmt, gCurBlock, gStageObjCount);
    sStatusPos.vx = gPlayer.x;
    sStatusPos.vy = gPlayer.y;
    sStatusPos.vz = gPlayer.z;
    FntPrint(sEdPositionFmt, sStatusPos.vx);
    if (sTotals.height == 0) {
        FntPrint(sEdColorWhite);
    } else if (sTotals.height < 0) {
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
    FntPrint(sEdSwitchNoFmt, sTotals.switchCount, 200);
    FntPrint(sEdTrapNoFmt, sTotals.trapCount, 200);
    FntPrint(sEdPepsiNoFmt, sTotals.canCount, 200);
    FntPrint(sEdItemText);
    switch (sTotals.kind) {
        case EK_PEPSI:
            FntPrint(sEdPepsiFmt, sTotals.canGroup + 1);
            break;
        case EK_SWITCH:
            FntPrint(sEdSwitchFmt, sTotals.switchNo);
            break;
        case EK_TRAP:
            FntPrint(sEdTrapFmt, sTotals.trapNo);
            break;
        case EK_SPRITE:
            FntPrint(sEdSpriteFmt, sTotals.spriteNo);
            break;
    }
    FntPrint(sEdColorWhite);
    SortSemiQuad(0x421, -160, -120, 160, -120, 160, -70, -160, -70, 0);
    FntPrint(sEdNewline);
    if (gEditorInfo == 1) {
        FntPrint(sEdInfoText);
    } else if (gEditorPicking == 1) {
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
        case ED_MOVE:
            PrintTabs(5);
            FntPrint(sEdMoveModeText);
            PrintEditorCaption();
            break;
        case ED_MENU:
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
        case ED_PEPSI_SEL:
            PrintTabs(4);
            FntPrint(sEdPepsiSelModeText);
            FntPrint(sEdNewlines9);
            PrintTabs(3);
            FntPrint(sEdPepsiTypeSelText);
            PrintTabs(3);
            FntPrint(sEdPepsiTypeRangeFmt, sTotals.canGroup + 1);
            SortSemiQuad(0x421, -60, -30, 60, -30, 60, 30, -60, 30, 0);
            break;
        case ED_SWITCH_SEL:
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
            if (recs[sTotals.switchNo].state == SWITCH_FREE) {
                FntPrint(sEdColorWhite);
            } else {
                FntPrint(sEdColorGrey);
            }
            FntPrint(sEdNum2Fmt, sTotals.switchNo);
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
            FntPrint(sEdSwitchTypeRangeFmt, sTotals.switchType, 2);
            SortSemiQuad(0x421, -60, -30, 60, -30, 60, 30, -60, 30, 0);
            break;
        case ED_TRAP_SEL:
            PrintTabs(4);
            FntPrint(sEdTrapSelModeText);
            if (gEditorInfo == 0) {
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
            if (objs[sTotals.trapNo].model == -1) {
                FntPrint(sEdColorWhite);
            } else {
                FntPrint(sEdColorGrey);
            }
            FntPrint(sEdNum2Fmt, sTotals.trapNo);
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
            FntPrint(sEdTrapModelRangeFmt, sTotals.trapModel, gSectionCount);
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
            if (recs[sTotals.trapSwitch].state != SWITCH_FREE) {
                FntPrint(sEdColorWhite);
            } else {
                FntPrint(sEdColorGrey);
            }
            FntPrint(sEdNum2Fmt, sTotals.trapSwitch);
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
            FntPrint(sEdNum2Fmt, sTotals.trapType);
            FntPrint(sEdRangeEnd2Fmt, 255);
            PrintTabs(3);
            if (gMenuLine == 4) {
                FntPrint(sEdColorWhite);
            } else {
                FntPrint(sEdColorBlue);
            }
            FntPrint(sEdTrapActRangeText);
            FntPrint(sEdNum2Fmt, sTotals.trapAct);
            FntPrint(sEdRangeEnd2Fmt, 255);
            FntPrint(sEdColorWhite);
            PrintTabs(2);
            FntPrint(sEdAngleRangeFmt, sTotals.trapAngle);
            SortSemiQuad(0x421, -100, -30, 100, -30, 100, 56, -100, 56, 0);
            break;
        case ED_SPRITE_SEL:
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
            if (gStageSprites[sTotals.spriteNo].id == -1) {
                FntPrint(sEdColorWhite);
            } else {
                FntPrint(sEdColorGrey);
            }
            FntPrint(sEdNum2Fmt, sTotals.spriteNo);
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
            FntPrint(sEdSpriteModelRangeFmt, sTotals.spriteModel, 512);
            PrintTabs(2);
            if (gMenuLine == 2) {
                FntPrint(sEdColorWhite);
            } else {
                FntPrint(sEdColorBlue);
            }
            FntPrint(sEdSpriteSwitchRangeText);
            recs = sSwitches;
            if (recs[sTotals.spriteSwitch].state != SWITCH_FREE) {
                FntPrint(sEdColorWhite);
            } else {
                FntPrint(sEdColorGrey);
            }
            FntPrint(sEdNum2Fmt, sTotals.spriteSwitch);
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
            FntPrint(sEdSpriteSizeRangeFmt, (s16)sTotals.spriteSize - 500, 999);
            PrintTabs(2);
            if (gMenuLine == 4) {
                FntPrint(sEdColorWhite);
            } else {
                FntPrint(sEdColorBlue);
            }
            FntPrint(sEdHitLabel);
            if (sTotals.spriteHit == 0) {
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
            if (sTotals.spriteFlip == 0) {
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
            switch (sSpriteAct) {
                case SPRITE_ACT_NONE:
                    FntPrint(sEdNoActLine);
                    break;
                case SPRITE_ACT_NEAR:
                    FntPrint(sEdNearActText);
                    break;
                case SPRITE_ACT_HIT:
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
            if (sTotals.spriteWalk == 0) {
                FntPrint(sEdOffLine);
            } else {
                FntPrint(sEdSeNoFmt, sTotals.spriteWalk);
            }
            if (gMenuLine == 8) {
                FntPrint(sEdColorWhite);
            } else {
                FntPrint(sEdColorBlue);
            }
            PrintTabs(2);
            FntPrint(sEdSeLabel);
            switch (sTotals.spriteSe) {
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
            FntPrint(sEdAngleRangeFmt, sTotals.spriteAngle);
            SortSemiQuad(0x421, -100, -54, 100, -54, 100, 60, -100, 60, 0);
            break;
        case ED_DATA:
            PrintEditorData();
            break;
        case ED_NO_GROUND:
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
        case ED_FULL:
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
        case ED_EMPTY:
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
        case ED_NONE_HERE:
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
        case ED_NONE_NEAR:
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

    if (gEditorInfo != 1) {
        return;
    }
    switch (sTotals.kind) {
        case EK_PEPSI:
            e = (Span8 *)gCanSpans;
            e += gCurBlock;
            FntPrint(sEdCanNoFmt, sTotals.canNo);
            FntPrint(sEdCanNumFmt, e->count);
            break;
        case EK_SWITCH:
            FntPrint(sEdInfoSwitchNoFmt, sTotals.switchNo);
            break;
        case EK_TRAP:
            FntPrint(sEdInfoTrapNoFmt, sTotals.trapNo);
            if (((EditorTrap *)gTraps)[sTotals.trapNo].hook != -1) {
                FntPrint(sEdInfoTrapModelFmt, ((EditorTrap *)gTraps)[sTotals.trapNo].hook);
            } else {
                FntPrint(sEdOptionsText);
            }
            /* MATCHING: the table base in a pointer local after the join
             * takes $s1, the totals' base $s0. */
            r = (EditorTrap *)gTraps;
            FntPrint(sEdInfoTrapSwitchFmt, r[sTotals.trapNo].switchNo);
            FntPrint(sEdInfoTypeFmt, r[sTotals.trapNo].type);
            FntPrint(sEdInfoActFmt, r[sTotals.trapNo].act);
            break;
        case EK_SPRITE:
            FntPrint(sEdInfoSpriteFmt, sTotals.spriteNo);
            FntPrint(sEdInfoSpriteModelFmt, gStageSprites[sTotals.spriteNo].id - 30);
            FntPrint(sEdInfoSpriteSwitchFmt, gStageSprites[sTotals.spriteNo].switchNo);
            FntPrint(sEdInfoHitLabel);
            /* MATCHING: the clear-bit arm first, as retail's bnez. */
            if (!(gStageSprites[sTotals.spriteNo].flags & SPRITE_HITTABLE)) {
                FntPrint(sEdInfoOffText);
            } else {
                FntPrint(sEdInfoOnText);
            }
            FntPrint(sEdInfoActLabel);
            switch (gStageSprites[sTotals.spriteNo].flags & SPRITE_ACT_MASK) {
                case SPRITE_ACT_NONE:
                    FntPrint(sEdNoActText);
                    break;
                case SPRITE_ACT_NEAR:
                    FntPrint(sEdInfoNearActText);
                    break;
                case SPRITE_ACT_HIT:
                    FntPrint(sEdInfoHitActText);
                    break;
            }
            FntPrint(sEdInfoWalkLabel);
            if (gStageSprites[sTotals.spriteNo].walk == 0) {
                FntPrint(sEdInfoOffLine);
            } else {
                FntPrint(sEdInfoWalkTypeFmt, gStageSprites[sTotals.spriteNo].walk);
            }
            FntPrint(sEdInfoSeLabel);
            switch (gStageSprites[sTotals.spriteNo].se) {
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
                gEditorPicking = 0;
                gEditorInfo = 0;
                sTotals.kind = gMenuLine;
                gEditValue = sTotals.canGroup;
                gMenuLine = 0;
                gToolMode = ED_PEPSI_SEL;
                break;
            case 1:
                gEditMax = 200;
                gMenuLineCount = 2;
                gEditorPicking = 0;
                gEditorInfo = 0;
                sTotals.kind = gMenuLine;
                gEditValue = sTotals.switchNo;
                gMenuLine = 0;
                gToolMode = ED_SWITCH_SEL;
                break;
            case 2:
                gEditMax = 200;
                gMenuLineCount = 5;
                gEditorPicking = 0;
                gEditorInfo = 0;
                sTotals.kind = gMenuLine;
                gEditValue = sTotals.trapNo;
                gMenuLine = 0;
                gSubStep = 0;
                gToolMode = ED_TRAP_SEL;
                break;
            case 3:
                gEditMax = 100;
                gMenuLineCount = 9;
                gEditorPicking = 0;
                gEditorInfo = 0;
                sTotals.kind = gMenuLine;
                gEditValue = sTotals.spriteNo;
                gMenuLine = 0;
                gSubStep = 0;
                gToolMode = ED_SPRITE_SEL;
                break;
            case 4:
                gEditorPicking = 1;
                gEditorInfo = 0;
                gSubStep = 0;
                break;
            case 5:
                gEditorInfo = 1;
                /* MATCHING: an int-valued 1 (retail rebuilds the constant). */
                gEditorPicking = gEditorInfo != 0;
                gSubStep = 0;
                gToolMode = ED_MOVE;
                break;
            case 6:
                gToolMode = ED_MOVE;
                gPlayer.x = gCourse.x;
                gPlayer.y = gCourse.y;
                gPlayer.z = gCourse.z;
                gCourse.rot.vy = gCourse.rot.vz;
                gFloorIndex = FindFloor(&gPlayer.x);
                gSection = 0;
                gView[1] = (s16)(gCourse.y - 500);
                break;
            case 7:
                gToolMode = ED_DATA;
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
    gRefView[1] = rsin(gViewerAngle * 4096 / 360) * 500 / 4096 + (gPlayer.y + sTotals.camHeight);
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
        MoveGamePos(0, sTotals.grid);
    }
    if (gPadTrig & PADLdown) {
        MoveGamePos(180, sTotals.grid);
    }
    if (gPadTrig & PADLleft) {
        MoveGamePos(-90, sTotals.grid);
    }
    if (gPadTrig & PADLright) {
        MoveGamePos(90, sTotals.grid);
    }
    if (gStickRawY < 0x60) {
        MoveGamePos(0, sTotals.grid);
    }
    if (gStickRawY > 0xA0) {
        MoveGamePos(180, sTotals.grid);
    }
    if (gStickRawX < 0x60) {
        MoveGamePos(-90, sTotals.grid);
    }
    if (gStickRawX > 0xA0) {
        MoveGamePos(90, sTotals.grid);
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
    if (gEditorPicking == 0) {
        if (gPad & PADL2) {
            sTotals.camHeight -= 5;
        }
        if (gPad & PADR2) {
            sTotals.camHeight += 5;
        }
        if (sTotals.camHeight > 0) {
            sTotals.camHeight = 0;
        }
    }
    if (gPadTrig & PADRleft) {
        sTotals.height -= sTotals.grid;
    }
    if (gPadTrig & PADRdown) {
        sTotals.height += sTotals.grid;
    }
    gPlayer.editorY = gPlayer.y;
    cur = FindFloorNear(&gPlayer.x, gFloorY, gFloorIndex);
    gCurBlock = cur;
    if (cur != -1) {
        gFloorIndex = cur;
    }
    gPlayer.y = gFloorY[0] + sTotals.height;
    if (gEditorPicking == 0 && (gPadTrig & PADRright) && cur == -1) {
        gToolMode = ED_NO_GROUND;
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
        if (sTotals.canCount == 200) {
            gToolMode = ED_FULL;
        } else if (gCurBlock == -1) {
            gToolMode = ED_NO_GROUND;
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
            sTotals.canNo = e->start;
            return;
        case 1:
            break;
        default:
            return;
    }
    src = (Pt8 *)gCanPoints;
    src += sTotals.canNo;
    pos.vx = src->x;
    pos.vy = src->y;
    pos.vz = src->z;
    DrawPin(&pos);
    e = (Span8 *)gCanSpans;
    e += gCurBlock;
    if (e->count == 0 || gCurBlock == -1) {
        gToolMode = ED_NONE_HERE;
        return;
    }
    if (gEditorInfo != 1 && (gPadTrig & PADRright)) {
        e = (Span8 *)gCanSpans;
        e += gCurBlock;
        if (e->count == 0) {
            gToolMode = ED_EMPTY;
            return;
        }
        e = (Span8 *)gCanSpans;
        e += gCurBlock;
        dst = (Pt8 *)gCanPoints;
        src = dst;
        src += sTotals.canNo + 1;
        dst += sTotals.canNo;
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
        sTotals.canNo--;
    }
    if (gPadTrig & PADR2) {
        sTotals.canNo++;
    }
    e = (Span8 *)gCanSpans;
    e += gCurBlock;
    if (sTotals.canNo < (u32)e->start) {
        sTotals.canNo = e->start;
    }
    /* MATCHING: the compare as start + (count - 1), the store as
     * count - 1 + start; unsigned, as retail's sltu. */
    if ((u32)e->start + (e->count - 1) < sTotals.canNo) {
        sTotals.canNo = (u32)e->count - 1 + e->start;
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
    sTotals.switchCount = 0;
    for (i = 0; i < 200; i++) {
        if (sSwitches[i].state != SWITCH_FREE) {
            sSwitches[i].state = SWITCH_ARMED;
            sTotals.switchCount++;
        }
    }
    if (sTotals.switchCount == 200) {
        gToolMode = ED_FULL;
        return;
    }
    if (sSwitches[sTotals.switchNo].state != SWITCH_FREE) {
        for (i = 0; i < 200; i++) {
            if (sSwitches[i].state == SWITCH_FREE) {
                break;
            }
        }
        if (i == 200) {
            gToolMode = ED_FULL;
            return;
        }
        sTotals.switchNo = i;
    }
    SetBoxCentred(&sSwitches[sTotals.switchNo], sizes.s[sTotals.switchType].w,
                  sizes.s[sTotals.switchType].d);
    sSwitches[sTotals.switchNo].pos[0] = gPlayer.x;
    sSwitches[sTotals.switchNo].pos[1] = gPlayer.y;
    sSwitches[sTotals.switchNo].pos[2] = gPlayer.z;
    sSwitches[sTotals.switchNo].state = SWITCH_FREE;
    if (gPadTrig & PADRright) {
        if (gCurBlock == -1) {
            gToolMode = ED_NO_GROUND;
            return;
        }
        sSwitches[sTotals.switchNo].state = SWITCH_ARMED;
        for (i = 0; i < 200; i++) {
            if (sSwitches[i].state == SWITCH_FREE) {
                break;
            }
        }
        if (i == 200) {
            gToolMode = ED_FULL;
            return;
        }
        sTotals.switchNo = i;
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
    sTotals.switchCount = 0;
    for (; i < 200; i++) {
        if (sSwitches[i].state != SWITCH_FREE) {
            sSwitches[i].state = SWITCH_ARMED;
            sTotals.switchCount++;
        }
    }
    switch (gSubStep) {
        case 0:
            if (sTotals.switchCount == 0) {
                gToolMode = ED_EMPTY;
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
                if (r->state != SWITCH_FREE) {
                    at.vx = r->pos[0];
                    at.vy = r->pos[1];
                    at.vz = r->pos[2];
                    d = DistanceSquared(&pos, &at);
                    if (d < best) {
                        sTotals.switchNo = i;
                        best = d;
                    }
                }
            }
            if (best == 25000000) {
                gToolMode = ED_NONE_NEAR;
            } else {
                gSubStep++;
            }
            break;
        case 1:
            /* MATCHING: the table base in the record pointer first; indexing
             * sSwitches directly builds the totals' address first. */
            r = sSwitches;
            sel.vx = r[sTotals.switchNo].pos[0];
            sel.vy = r[sTotals.switchNo].pos[1];
            sel.vz = r[sTotals.switchNo].pos[2];
            DrawPin(&sel);
            if (gEditorInfo != 1 && (gPadTrig & PADRright)) {
                sSwitches[sTotals.switchNo].state = SWITCH_FREE;
                gSubStep = 0;
                break;
            }
            if (gPadTrig & PADL2) {
                if (--sTotals.switchNo >= 200) {
                    sTotals.switchNo = 199;
                }
                while (sSwitches[sTotals.switchNo].state == SWITCH_FREE) {
                    if (--sTotals.switchNo >= 200) {
                        sTotals.switchNo = 199;
                    }
                }
            }
            if (gPadTrig & PADR2) {
                /* the first step up clamps to 199, the later ones wrap to 0 */
                if (++sTotals.switchNo >= 200) {
                    sTotals.switchNo = 199;
                }
                while (sSwitches[sTotals.switchNo].state == SWITCH_FREE) {
                    if (++sTotals.switchNo >= 200) {
                        sTotals.switchNo = 0;
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
    n = sTotals.trapNo;
    sTotals.trapCount = 0;
    for (i = 0; i < 200; i++) {
        if (recs[i].model != -1) {
            sTotals.trapCount++;
        }
    }
    if (gPadTrig & PADRright) {
        /* MATCHING: one counter for both loops, unsigned (sltiu) in the
         * first and compared signed (slt) here. */
        for (i = 0; (s32)i < gPlaceCount; i++) {
            w = &gPlaceQueue[i];
            if (n == 200) {
                gToolMode = ED_FULL;
                return;
            }
            w->home[0] = w->pos[0];
            w->home[1] = w->pos[1];
            w->home[2] = w->pos[2];
            w->touched = 0;
            w->unk24 = 0;
            w->unk26 = 0;
            w->block = gCurBlock;
            w->section = gSection;
            w->type = sTotals.trapType;
            w->act = sTotals.trapAct;
            if (i == 0) {
                /* MATCHING: stored through the table base, not w. */
                gPlaceQueue[0].hook = sTotals.trapModel;
            } else {
                w->hook = -1;
            }
            ((EditorTrap *)gTraps)[(s16)n] = *w;
            ((EditorTrap *)gTraps)[(s16)n].switchNo = sTotals.trapSwitch;
            n++;
            sTotals.trapNo = n;
        }
        gSubStep = 0;
        if (gEditorInfo == 1) {
            gEditorInfo = 0;
        }
    }
}

/** @brief Runs the menu editor; on a line change loads the new line's
 *         value and limit, else stores the edited value back; then steps
 *         the angle in sTotals.trapAngle on flag bits 3 and 2.
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
                gEditValue = sTotals.trapNo;
                break;
            case 1:
                gEditMax = gSectionCount;
                gEditValue = sTotals.trapModel;
                break;
            case 2:
                gEditMax = 200;
                gEditValue = sTotals.trapSwitch;
                break;
            case 3:
                gEditMax = 0x100;
                gEditValue = sTotals.trapType;
                break;
            case 4:
                gEditMax = 0x100;
                gEditValue = sTotals.trapAct;
                break;
        }
    } else {
        switch (line) {
            case 0:
                sTotals.trapNo = gEditValue;
                break;
            case 1:
                sTotals.trapModel = gEditValue;
                break;
            case 2:
                sTotals.trapSwitch = gEditValue;
                break;
            case 3:
                sTotals.trapType = gEditValue;
                break;
            case 4:
                sTotals.trapAct = gEditValue;
                break;
        }
    }
    if (gPad & PADR1) {
        if (++sTotals.trapAngle >= 360) {
            sTotals.trapAngle = 0;
        }
    }
    if (gPad & PADL1) {
        if (--sTotals.trapAngle < 0) {
            sTotals.trapAngle = 359;
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
            gPlaceQueue[0].rotX = 0;
            gPlaceQueue[0].rotZ = 0;
            gPlaceQueue[0].pos[0] = gPlayer.x;
            gPlaceQueue[0].pos[1] = gPlayer.y;
            gPlaceQueue[0].pos[2] = gPlayer.z;
            gPlaceQueue[0].rotY = (sTotals.trapAngle << 12) / 360;
            gPlaceCount = CallStageHook(sTotals.trapModel, gPlaceQueue, gPlaceQueue);
            gSubStep++;
            /* fall through */
        case 1:
            w = gPlaceQueue;
            w->pos[0] = gPlayer.x;
            w->pos[1] = gPlayer.y;
            w->pos[2] = gPlayer.z;
            w->rotY = (sTotals.trapAngle << 12) / 360;
            if (gCurBlock == -1) {
                gFloorY[1] = 0;
                gFloorY[2] = -0x1000;
                gFloorY[3] = 0;
            }
            w->lightAngle = ratan2(-gFloorY[3], gFloorY[2]);
            w->lightAngle2 = ratan2(-gFloorY[1], gFloorY[2]);
            UpdateObjectEntry(&gObjectModels[w->model], (Trap *)w);
            for (i = 1; i < gPlaceCount; i++) {
                /* MATCHING: an integer sum puts the index first in the addu. */
                r = (EditorTrap *)(i * sizeof(EditorTrap) + (u32)w);
                gLocalPos.vx = r->home[0];
                gLocalPos.vy = r->home[1];
                gLocalPos.vz = r->home[2];
                TransformByCoord((GsCOORDINATE2 *)gObjectModels[w->model].coord, &gLocalPos, &gWorldPos);
                r->pos[0] = gWorldPos.vx;
                r->pos[1] = gWorldPos.vy;
                r->pos[2] = gWorldPos.vz;
                r->rotY = w->rotY + r->homeRotY;
                r->lightAngle = w->lightAngle;
                r->lightAngle2 = w->lightAngle2;
                UpdateObjectEntry(&gObjectModels[r->model], (Trap *)r);
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
    sTotals.trapCount = 0;
    for (; i < 200; i++) {
        if (((EditorTrap *)gTraps)[i].model != -1) {
            sTotals.trapCount++;
        }
    }
    switch (gSubStep) {
        case 0:
            if (sTotals.trapCount == 0) {
                gToolMode = ED_EMPTY;
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
                if (r->model != -1) {
                    at.vx = r->pos[0];
                    at.vy = r->pos[1];
                    at.vz = r->pos[2];
                    d = DistanceSquared(&pos, &at);
                    if (d < best) {
                        sTotals.trapNo = i;
                        best = d;
                    }
                }
            }
            if (best == 25000000) {
                gToolMode = ED_NONE_NEAR;
            } else {
                gSubStep++;
            }
            break;
        case 1:
            sTotals.trapSwitch = ((EditorTrap *)gTraps)[sTotals.trapNo].switchNo;
            sel.vx = ((EditorTrap *)gTraps)[sTotals.trapNo].pos[0];
            sel.vy = ((EditorTrap *)gTraps)[sTotals.trapNo].pos[1];
            sel.vz = ((EditorTrap *)gTraps)[sTotals.trapNo].pos[2];
            DrawPin(&sel);
            if (gEditorInfo != 1 && (gPadTrig & PADRright)) {
                gSubStep = 0;
                ((EditorTrap *)gTraps)[sTotals.trapNo].model = -1;
            }
            if (gPadTrig & PADL2) {
                if (--sTotals.trapNo >= 200) {
                    sTotals.trapNo = 199;
                }
                while (((EditorTrap *)gTraps)[sTotals.trapNo].model == -1) {
                    if (--sTotals.trapNo >= 200) {
                        sTotals.trapNo = 199;
                    }
                }
            }
            if (gPadTrig & PADR2) {
                /* the first step up clamps to 199, the later ones wrap to 0 */
                if (++sTotals.trapNo >= 200) {
                    sTotals.trapNo = 199;
                }
                while (((EditorTrap *)gTraps)[sTotals.trapNo].model == -1) {
                    if (++sTotals.trapNo >= 200) {
                        sTotals.trapNo = 0;
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
                gEditValue = sTotals.spriteNo;
                break;
            case 1:
                gEditMax = 0x200;
                gEditValue = sTotals.spriteModel;
                break;
            case 2:
                gEditMax = 200;
                gEditValue = sTotals.spriteSwitch;
                break;
            case 3:
                gEditMax = 1000;
                gEditValue = sTotals.spriteSize;
                break;
            case 4:
                gEditMax = 2;
                gEditValue = sTotals.spriteHit;
                break;
            case 5:
                gEditMax = 2;
                gEditValue = sTotals.spriteFlip;
                break;
            case 6:
                gEditMax = 3;
                gEditValue = sSpriteAct;
                break;
            case 7:
                gEditMax = 2;
                gEditValue = sTotals.spriteWalk;
                break;
            case 8:
                gEditMax = 5;
                gEditValue = sTotals.spriteSe;
                break;
        }
    } else {
        switch (line) {
            case 0:
                sTotals.spriteNo = gEditValue;
                break;
            case 1:
                sTotals.spriteModel = gEditValue;
                break;
            case 2:
                sTotals.spriteSwitch = gEditValue;
                break;
            case 3:
                sTotals.spriteSize = gEditValue;
                break;
            case 4:
                sTotals.spriteHit = gEditValue;
                break;
            case 5:
                sTotals.spriteFlip = gEditValue;
                break;
            case 6:
                sSpriteAct = gEditValue;
                break;
            case 7:
                sTotals.spriteWalk = gEditValue;
                break;
            case 8:
                sTotals.spriteSe = gEditValue;
                break;
        }
    }
    if (gPad & PADR1) {
        if (++sTotals.spriteAngle >= 360) {
            sTotals.spriteAngle = 0;
        }
    }
    if (gPad & PADL1) {
        if (--sTotals.spriteAngle < 0) {
            sTotals.spriteAngle = 359;
        }
    }
    if (sTotals.spriteWalk != 0) {
        DrawHeadingLine((VECTOR *)&gPlayer.x, sTotals.spriteAngle);
    }
    sSpriteTemplate.pos[0] = gPlayer.x;
    sSpriteTemplate.pos[1] = gPlayer.y;
    sSpriteTemplate.pos[2] = gPlayer.z;
    GetQuad16(sTotals.spriteModel + 30, &e);
    /* MATCHING: each offset in its own s32 local; inline, the sum is
     * narrowed to a halfword add, and one shared local merges the arms. */
    if (sTotals.spriteFlip == 0) {
        h0 = e.b[6] - 500;
        sSpriteTemplate.width = sTotals.spriteSize + h0;
    } else {
        h1 = e.b[6] - 500;
        sSpriteTemplate.width = -(sTotals.spriteSize + h1);
    }
    h2 = e.b[7] - 500;
    sSpriteTemplate.height = sTotals.spriteSize + h2;
    sSpriteTemplate.radius = sSpriteTemplate.width;
    if (sTotals.spriteHit == 0) {
        DrawObjectSprite(sTotals.spriteModel + 30, &sSpriteTemplate);
    } else {
        StepSprite(sTotals.spriteModel + 30, &sSpriteTemplate);
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
    sSpriteTemplate.pos[0] = gPlayer.x;
    sSpriteTemplate.pos[1] = gPlayer.y;
    sSpriteTemplate.pos[2] = gPlayer.z;
    if (sTotals.spriteHit == 0) {
        DrawObjectSprite(sTotals.spriteModel + 30, &sSpriteTemplate);
    } else {
        StepSprite(sTotals.spriteModel + 30, &sSpriteTemplate);
    }
    /* MATCHING: the counter is cleared before the store, so each arm above
     * ends with it (one in a jump's delay slot). */
    i = 0;
    sTotals.spriteCount = 0;
    for (; i < 100; i++) {
        if (gStageSprites[i].id != -1) {
            sTotals.spriteCount++;
        }
    }
    if (gPadTrig & PADRright) {
        if (sTotals.spriteCount == 100) {
            gToolMode = ED_FULL;
        } else {
            sSpriteTemplate.switchNo = sTotals.spriteSwitch;
            sSpriteTemplate.id = sTotals.spriteModel + 30;
            sSpriteTemplate.drift[0] = sSpriteTemplate.pos[0];
            sSpriteTemplate.drift[1] = sSpriteTemplate.pos[1];
            sSpriteTemplate.drift[2] = sSpriteTemplate.pos[2];
            /* MATCHING: an s32 local assigned after the copy, and the flag
             * byte or-ed in by a second store. */
            f = sTotals.spriteHit << 7; /* SPRITE_HITTABLE */
            sSpriteTemplate.flags = f;
            sSpriteTemplate.flags |= sSpriteAct;
            sSpriteTemplate.walk = sTotals.spriteWalk;
            sSpriteTemplate.se = sTotals.spriteSe;
            sSpriteTemplate.angle = sTotals.spriteAngle;
            sSpriteTemplate.section = gSection;
            sSpriteTemplate.block = gCurBlock;
            gStageSprites[sTotals.spriteNo] = sSpriteTemplate;
            /* MATCHING: the full-table store inside the loop, so its 8 is
             * hoisted and the block is not merged with the one above. */
            i = 0;
            while (1) {
                if (gStageSprites[sTotals.spriteNo].id == -1) {
                    goto found;
                }
                sTotals.spriteNo++;
                if (sTotals.spriteNo == 100) {
                    sTotals.spriteNo = 0;
                }
                if (++i == 100) {
                    gToolMode = ED_FULL;
                    return;
                }
            }
        found:
            if (gEditorInfo == 1) {
                gEditorInfo = 0;
            }
        }
    }
}

/** @brief Runs one of two handlers on every used StageSprite whose unk2C equals
 *         sTotals.spriteSwitch, picked by bit 7 of its unk26. */
void EditorRunSprites(void) {
    u32 i;
    StageSprite *r;

    for (i = 0; i < 100; i++) {
        r = &gStageSprites[i];
        if (r->id != -1 && sTotals.spriteSwitch == r->switchNo) {
            if (r->flags & SPRITE_HITTABLE) {
                StepSprite(r->id, r);
            } else {
                DrawObjectSprite(r->id, r);
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
    sTotals.spriteCount = 0;
    for (; i < 100; i++) {
        if (gStageSprites[i].id != -1) {
            sTotals.spriteCount++;
        }
    }
    switch (gSubStep) {
        case 0:
            if (sTotals.spriteCount == 0) {
                gToolMode = ED_EMPTY;
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
                if (r->id != -1) {
                    at.vx = r->pos[0];
                    at.vy = r->pos[1];
                    at.vz = r->pos[2];
                    d = DistanceSquared(&pos, &at);
                    if (d < best) {
                        sTotals.spriteNo = i;
                        best = d;
                    }
                }
            }
            if (best == 25000000) {
                gToolMode = ED_NONE_NEAR;
            } else {
                gSubStep++;
            }
            break;
        case 1:
            sTotals.spriteSwitch = gStageSprites[sTotals.spriteNo].switchNo;
            sel.vx = gStageSprites[sTotals.spriteNo].pos[0];
            sel.vy = gStageSprites[sTotals.spriteNo].pos[1];
            sel.vz = gStageSprites[sTotals.spriteNo].pos[2];
            DrawPin(&sel);
            if (gEditorInfo != 1 && (gPadTrig & PADRright)) {
                gSubStep = 0;
                gStageSprites[sTotals.spriteNo].id = -1;
            }
            if (gPadTrig & PADL2) {
                if (--sTotals.spriteNo >= 100) {
                    sTotals.spriteNo = 99;
                }
                while (gStageSprites[sTotals.spriteNo].id == -1) {
                    if (--sTotals.spriteNo >= 100) {
                        sTotals.spriteNo = 99;
                    }
                }
            }
            if (gPadTrig & PADR2) {
                /* the first step up clamps to 99, the later ones wrap to 0 */
                if (++sTotals.spriteNo >= 100) {
                    sTotals.spriteNo = 99;
                }
                while (gStageSprites[sTotals.spriteNo].id == -1) {
                    if (++sTotals.spriteNo >= 100) {
                        sTotals.spriteNo = 0;
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
    err = CARD_OK;
    switch (gSubStep) {
        case 0:
            OpenCardEvents();
            gToolCardStep = CS_CHECK;
            gMenuLine = 0;
            gEditValue = 0;
            gSubStep++;
            break;
        case 1:
            switch (gToolCardStep) {
                case CS_CHECK:
                    gCardNew = 0;
                    err = RunCardOp1(CARD_OP_READ);
                    if (err == CARD_OK) {
                        gToolCardStep = CS_MODE_SELECT;
                    }
                    break;
                case CS_MODE_SELECT:
                    if (CheckCard() == -1 || (gPadTrig & PADRdown)) {
                        gToolCardStep = CS_EXIT;
                        break;
                    }
                    gMenuChoice = RunMenu(2);
                    switch (gMenuChoice) {
                        case 2:
                            if (WriteEditorFile() == -1) {
                                gToolCardStep = CS_ERROR;
                            } else if (WriteEditorSlotFiles() == -1) {
                                gToolCardStep = CS_ERROR;
                            } else {
                                gToolCardStep = CS_COMPLETE;
                            }
                            break;
                        case 3:
                            if (ReadEditorFile() == -1) {
                                gToolCardStep = CS_ERROR;
                            } else {
                                gMenuChoice = 1;
                                gToolCardStep = CS_SLOT_SELECT;
                            }
                            break;
                        case -1:
                            break;
                        default:
                            gToolCardStep = CS_SLOT_SELECT;
                            break;
                    }
                    break;
                case CS_SLOT_SELECT:
                    if (gPadTrig & PADRdown) {
                        gToolCardStep = CS_MODE_SELECT;
                        break;
                    }
                    gToolSlot = RunMenu(3);
                    if (gToolSlot == -1) {
                        break;
                    }
                    switch (gMenuChoice) {
                        case 0:
                            gToolCardStep = CS_SAVE_CHECK;
                            break;
                        case 1:
                            gToolCardStep = CS_LOAD_CHECK;
                            break;
                    }
                    break;
                case CS_COMPLETE:
                    if (gPadTrig & (PADRdown | PADRright)) {
                        gPlayer.x = gCourse.x;
                        gPlayer.y = gCourse.y;
                        gPlayer.z = gCourse.z;
                        gCourse.rot.vy = gCourse.rot.vz;
                        gFloorIndex = FindFloor(&gPlayer.x);
                        gSection = 0;
                        gView[1] = (s16)(gCourse.y - 500);
                        SetCurrentBlock((BlockHeader *)0x801FD000);
                        gToolCardStep = CS_MODE_SELECT;
                    }
                    break;
                case CS_SAVE_CHECK:
                    if ((s16)CheckEditorSlot(gToolSlot) == -1) {
                        gToolCardStep = CS_WRITE;
                    } else {
                        gToolCardStep = CS_SAVE_ASK;
                        gMenuLine = 0;
                    }
                    break;
                case CS_SAVE_ASK:
                    r = RunMenu(2);
                    switch (r) {
                        case 0:
                            gToolCardStep = CS_WRITE;
                            break;
                        case 1:
                            gToolCardStep = CS_MODE_SELECT;
                            break;
                    }
                    break;
                case CS_WRITE:
                    SaveEditorState(gToolSlot);
                    err = RunCardOp1(CARD_OP_REWRITE);
                    if (err == CARD_OK) {
                        gToolCardStep = CS_COMPLETE;
                    }
                    break;
                case CS_LOAD_CHECK:
                    if ((s16)CheckEditorSlot(gToolSlot) == -1) {
                        gToolCardStep = CS_NO_DATA;
                    } else {
                        gToolCardStep = CS_LOAD_ASK;
                        gMenuLine = 0;
                    }
                    break;
                case CS_LOAD_ASK:
                    r = RunMenu(2);
                    switch (r) {
                        case 0:
                            gToolCardStep = CS_LOAD;
                            break;
                        case 1:
                            gToolCardStep = CS_MODE_SELECT;
                            break;
                    }
                    break;
                case CS_NO_DATA:
                    if (gPadTrig & PADRright) {
                        gToolCardStep = CS_LOAD_CHECK;
                    }
                    break;
                case CS_LOAD:
                    RestoreEditorState(gToolSlot);
                    gToolCardStep = CS_COMPLETE;
                    break;
                case CS_ERROR:
                case CS_NO_CARD:
                    if (gPadTrig & PADRdown) {
                        gToolCardStep = CS_EXIT;
                    }
                    break;
                case CS_NO_FILE:
                    r = RunMenu(2);
                    switch (r) {
                        case 0:
                            gToolCardStep = CS_CREATE;
                            break;
                        case 1:
                            gToolCardStep = CS_EXIT;
                            break;
                        case -1:
                            break;
                    }
                    break;
                case CS_UNFORMATTED:
                    r = RunMenu(2);
                    switch (r) {
                        case 0:
                            gToolCardStep = CS_FORMAT;
                            break;
                        case 1:
                            gToolCardStep = CS_EXIT;
                            break;
                        case -1:
                            break;
                    }
                    break;
                case CS_CREATE:
                    err = RunCardOp1(CARD_OP_CREATE);
                    if (err == CARD_OK) {
                        gToolCardStep = CS_CHECK;
                    }
                    break;
                case CS_FORMAT:
                    err = RunCardOp1(CARD_OP_FORMAT);
                    if (err == CARD_OK) {
                        gToolCardStep = CS_CHECK;
                    }
                    break;
                case CS_EXIT:
                    gSubStep++;
                    break;
            }
            switch (err) {
                case CARD_PROBE_FAILED:
                case CARD_FAILED:
                    gToolCardStep = CS_ERROR;
                    break;
                case CARD_FULL:
                    gToolCardStep = CS_NO_MEMORY;
                    break;
                case CARD_ABSENT:
                    gToolCardStep = CS_NO_CARD;
                    break;
                case CARD_NO_FILE:
                    gToolCardStep = CS_NO_FILE;
                    break;
                case CARD_UNFORMATTED:
                    gToolCardStep = CS_UNFORMATTED;
                    break;
            }
            break;
        case 2:
            CloseCardEvents();
            gToolMode = ED_MOVE;
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
        case CS_CHECK:
            FntPrint(sEdNewlines3);
            PrintTabs(4);
            FntPrint(sEdCheckText);
            break;
        case CS_MODE_SELECT:
            FntPrint(sEdNewlines3);
            PrintTabs(3);
            FntPrint(sEdModeSelectText);
            PrintSaveLoad();
            break;
        case CS_SLOT_SELECT:
            FntPrint(sEdNewlines3);
            PrintTabs(3);
            FntPrint(sEdBufferSelectText);
            PrintEditorSlots();
            break;
        case CS_CREATE:
            FntPrint(sEdNewlines3);
            PrintTabs(3);
            FntPrint(sEdCreateFileText);
            break;
        case CS_FORMAT:
            FntPrint(sEdNewlines3);
            PrintTabs(3);
            FntPrint(sEdCardFormatText);
            break;
        case CS_SAVE_ASK:
            FntPrint(sEdNewlines3);
            PrintTabs(3);
            FntPrint(sEdSaveToDataFmt, gToolSlot + 1);
            PrintTabs(3);
            FntPrint(sEdOverwriteOkText);
            PrintYesNo();
            break;
        case CS_WRITE:
            FntPrint(sEdNewlines3);
            PrintTabs(3);
            FntPrint(sEdDataWritingText);
            break;
        case CS_LOAD_ASK:
            FntPrint(sEdNewlines3);
            PrintTabs(3);
            FntPrint(sEdLoadFromDataFmt, gToolSlot + 1);
            PrintTabs(3);
            FntPrint(sEdLoadOkText);
            PrintYesNo();
            break;
        case CS_NO_DATA:
            FntPrint(sEdNewlines3);
            PrintTabs(2);
            FntPrint(sEdCantOpenDataFmt, gToolSlot + 1);
            break;
        case CS_COMPLETE:
            FntPrint(sEdNewlines3);
            PrintTabs(3);
            FntPrint(sEdCompleteText);
            break;
        case CS_ERROR:
            FntPrint(sEdNewlines3);
            PrintTabs(2);
            FntPrint(sEdErrorText);
            break;
        case CS_NO_MEMORY:
            FntPrint(sEdNewlines3);
            PrintTabs(2);
            FntPrint(sEdNotEnoughMemoryText);
            break;
        case CS_NO_CARD:
            FntPrint(sEdNewlines3);
            PrintTabs(3);
            FntPrint(sEdNoCardText);
            break;
        case CS_NO_FILE:
            FntPrint(sEdNewlines3);
            PrintTabs(4);
            FntPrint(sEdNoFileText);
            PrintTabs(3);
            FntPrint(sEdCreateNewFileText);
            PrintYesNo();
            break;
        case CS_UNFORMATTED:
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
        if (slot->state.magic == EDITOR_MAGIC) {
            FntPrint(sEdDataNoFmt, i + 1);
            v = slot->state.stage;
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
    ((SaveSlot *)buf)[a].switches = *(SwitchTable *)gSwitches;
    ((SaveSlot *)buf)[a].traps = *(TrapTable *)gTraps;
    /* MATCHING: byte offsets past 0x7FFF build the offset whole (ori). */
    *(SpriteTable *)((u8 *)&((SaveSlot *)buf)[a] + 0x8294) = *(SpriteTable *)gStageSprites;
    *(BlockAreaCopy *)((u8 *)&((SaveSlot *)buf)[a] + 0xB174) = *(BlockAreaCopy *)0x801FD000;
    *(EditorStateCopy *)&((SaveSlot *)buf)[a].state = *(EditorStateCopy *)gEditorState;
}

/** @brief Restores the record tables, the block header area and the tool
 *         state block from save slot `a` of `buf`. */
void LoadEditorSlot(s16 a, u8 *buf) {
    *(SwitchTable *)gSwitches = ((SaveSlot *)buf)[a].switches;
    *(TrapTable *)gTraps = ((SaveSlot *)buf)[a].traps;
    /* MATCHING: byte offsets past 0x7FFF build the offset whole (ori). */
    *(SpriteTable *)gStageSprites = *(SpriteTable *)((u8 *)&((SaveSlot *)buf)[a] + 0x8294);
    *(BlockAreaCopy *)0x801FD000 = *(BlockAreaCopy *)((u8 *)&((SaveSlot *)buf)[a] + 0xB174);
    *(EditorStateCopy *)gEditorState = *(EditorStateCopy *)&((SaveSlot *)buf)[a].state;
}

/** @brief Writes the four data blocks of each save slot of the tool buffer
 *         to its file on the host.
 *  @return 0, or -1 when a write fails. */
s32 WriteEditorSlotFiles(void) {
    s32 fd;
    s32 n;
    SaveSlot *s;

    s = (SaveSlot *)0x8016D000;
    fd = open(sEdDataFilePath0, O_CREAT | O_WRONLY);
    if (write(fd, &s[0].traps, sizeof(TrapTable)) == -1) {
        close(fd);
        return -1;
    }
    if (write(fd, &s[0].switches, sizeof(SwitchTable)) == -1) {
        close(fd);
        return -1;
    }
    if (write(fd, &s[0].sprites, sizeof(SpriteTable)) == -1) {
        close(fd);
        return -1;
    }
    if (write(fd, (u8 *)&s[0] + 0xB174, sizeof(BlockAreaCopy)) == -1) {
        close(fd);
        return -1;
    }
    close(fd);
    fd = open(sEdDataFilePath1, O_CREAT | O_WRONLY);
    if (write(fd, &s[1].traps, sizeof(TrapTable)) == -1) {
        close(fd);
        return -1;
    }
    if (write(fd, &s[1].switches, sizeof(SwitchTable)) == -1) {
        close(fd);
        return -1;
    }
    if (write(fd, &s[1].sprites, sizeof(SpriteTable)) == -1) {
        close(fd);
        return -1;
    }
    if (write(fd, (u8 *)&s[1] + 0xB174, sizeof(BlockAreaCopy)) == -1) {
        /* MATCHING: the copy ranks n above fd in global alloc (permuter). */
        n = fd;
        close(n);
        return -1;
    }
    close(fd);
    fd = open(sEdDataFilePath2, O_CREAT | O_WRONLY);
    if (write(fd, &s[2].traps, sizeof(TrapTable)) == -1) {
        close(fd);
        return -1;
    }
    if (write(fd, &s[2].switches, sizeof(SwitchTable)) == -1) {
        close(fd);
        return -1;
    }
    if (write(fd, &s[2].sprites, sizeof(SpriteTable)) == -1) {
        close(fd);
        return -1;
    }
    n = write(fd, (u8 *)&s[2] + 0xB174, sizeof(BlockAreaCopy));
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
    sTotals.savedPos[0] = gPlayer.x;
    sTotals.savedPos[1] = gPlayer.y;
    sTotals.savedPos[2] = gPlayer.z;
    sTotals.savedYaw = gCourse.rot.vy;
    sTotals.savedView[0] = gRefView[0];
    sTotals.savedView[1] = gRefView[1];
    sTotals.savedView[2] = gRefView[2];
    sTotals.savedView[3] = gRefView[3];
    sTotals.savedView[4] = gRefView[4];
    sTotals.savedView[5] = gRefView[5];
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
    if (slot->state.magic != EDITOR_MAGIC) {
        return -1;
    }
    if (slot->state.stage != gStage) {
        return -1;
    }
    return 0;
}

/** @brief Runs a step on the tool buffer, then restores the game state
 *         the save step put in the tool state block. */
void RestoreEditorState(s16 a) {
    LoadEditorSlot(a, (u8 *)0x8016D000);
    gPlayer.x = sTotals.savedPos[0];
    gPlayer.y = sTotals.savedPos[1];
    gPlayer.z = sTotals.savedPos[2];
    gCourse.rot.vy = sTotals.savedYaw;
    gRefView[0] = sTotals.savedView[0];
    gRefView[1] = sTotals.savedView[1];
    gRefView[2] = sTotals.savedView[2];
    gRefView[3] = sTotals.savedView[3];
    gRefView[4] = sTotals.savedView[4];
    gRefView[5] = sTotals.savedView[5];
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
        recs->state = SWITCH_FREE;
        recs++;
    }
}

/** @brief Frees switches 100 to 199 (byte 0 to -1). */
void ClearSwitchBoxUpper(SwitchBox *recs) {
    u32 i;

    recs += 100;
    for (i = 100; i < 200; i++) {
        recs->state = SWITCH_FREE;
        recs++;
    }
}

/** @brief Resets all 200 records of a Trap table. */
void ResetTraps(Trap *recs) {
    u32 i;

    for (i = 0; i < 200; i++) {
        recs->model = -1;
        recs->hook = -1;
        recs->switchNo = -1;
        recs->marks = 1;
        recs++;
    }
}

/** @brief Resets traps 150 to 199. */
void ResetTrapUpper(Trap *recs) {
    u32 i;
    Trap *r;

    r = &recs[150];
    for (i = 150; i < 200; i++) {
        r->model = -1;
        r->switchNo = -1;
        r->marks = 1;
        r++;
    }
}

/** @brief Sets the halfword at 0 of all 100 records of a StageSprite table to -1. */
void ClearStageSprites(StageSprite *recs) {
    u32 i;

    for (i = 0; i < 100; i++) {
        recs->id = -1;
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

/** @brief A can span as TotalBlockEntries sums it: its count's low half. */
typedef struct {
    s32 start;   /**< index of the span's first can */
    u16 count;   /**< the count's low half */
    u16 countHi; /**< its high half */
} SpanCount;

/** @brief Totals the cans of the current block's spans into sTotals.canCount. */
void TotalBlockEntries(void) {
    s32 i;
    SpanCount *e;

    e = (SpanCount *)gCanSpans;
    sTotals.canCount = 0;
    for (i = 0; i < gCanSpanCount; i++) {
        sTotals.canCount += e->count;
        e++;
    }
}

/** @brief ED_PEPSI_SEL's step: moves the cursor and keeps the edited value
 *         as the group the next can gets. */
void EditPepsiGroup(void) {
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
                gEditValue = sTotals.switchNo;
                break;
            case 1:
                gEditMax = 3;
                gEditValue = sTotals.switchType;
                break;
        }
    } else {
        switch (old) {
            case 0:
                sTotals.switchNo = gEditValue;
                break;
            case 1:
                sTotals.switchType = gEditValue;
                break;
        }
    }
}

/** @brief Applies every used Trap whose unk34 equals sTotals.trapSwitch to the
 *         ObjectModel its unk36 names. */
void ApplyRecordsToEntries(void) {
    u32 i;
    Trap *r;

    for (i = 0; i < 200; i++) {
        r = &((Trap *)gTraps)[i];
        if (r->model != -1 && sTotals.trapSwitch == r->switchNo) {
            UpdateObjectEntry(&gObjectModels[r->model], r);
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
