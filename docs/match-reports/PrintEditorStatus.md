# PrintEditorStatus — MATCHED 1419/1419 (round 12, delta)

> Renamed from `func_8002DC44` on 2026-10-11 (tools/rename.py). Address 0x8002dc44.

Unit `src/editor.c`. The tool's status panel, printed every frame from
UpdateEditor: the sync count (two colour codes past 400 and 450), the grid
step, the block, the position (copied into `D_800ACED0`), the record counts
and the current item, a frame around it, the INFO / DELETE line, then one
panel per `gToolMode` (a jump table over 0..11): move mode, the menu, the
pepsi, switch, trap and sprite selectors with their highlighted line
(`gMenuLine`), PrintEditorData's data panel, and five warnings. Every
"highlighted or plain" pair is `if (c) FntPrint(A); else FntPrint(B);`;
cross-jumping merges the calls, and the warnings and frame calls share
their tails the same way.

```c
extern char gSyncFmt[]; /**< "SYNC %3d" */
extern char gGridFmt[]; /**< " GRID %4d" */
extern char gNowBlockFmt[]; /**< " NOW BLOCK (%2d/%2d)" */
extern char sEdPositionFmt[]; /**< "\nPOSITION X %d " */
extern char sEdSwitchNoFmt[]; /**< "\nSWITCH NO(%2d/ %2d)" */
extern char sEdTrapNoFmt[]; /**< " TRAP NO(%3d/ %3d)" */
extern char sEdPepsiNoFmt[]; /**< "\nPEPSI NO(%3d/ %3d)" */
extern char sEdItemText[]; /**< " ITEM : " */
extern char sEdPepsiFmt[]; /**< "PEPSI %2d" */
extern char sEdSwitchFmt[]; /**< "SWITCH %2d" */
extern char sEdTrapFmt[]; /**< "TRAP %2d" */
extern char sEdSpriteFmt[]; /**< "SPRITE %2d" */
extern char sEdInfoText[]; /**< "INFO    " */
extern char sEdDeleteText[]; /**< "DELETE  " */
extern char sEdMoveModeText[]; /**< "MOVE MODE" */
extern char sEdMenuModeText[]; /**< "MENU MODE" */
extern char sEdPepsiSelModeText[]; /**< "PEPSI SEL MODE" */
extern char sEdNewlines9[]; /**< nine newlines */
extern char sEdPepsiTypeSelText[]; /**< "PEPSI TYPE SEL\n\n" */
extern char sEdPepsiTypeRangeFmt[]; /**< "   1<-%2d->200" */
extern char sEdSwitchSelModeText[]; /**< "SWITCH SEL MODE" */
extern char sEdNewlines8[]; /**< eight newlines */
extern char sEdSwitchSelText[]; /**< "  SWITCH SEL\n\n" */
extern char sEdSwitchNoRangeText[]; /**< "NO.  0<-" */
extern char sEdSwitchTypeRangeFmt[]; /**< "TYPE. 0<-%1d->%d" */
extern char sEdTrapSelModeText[]; /**< "TRAP SEL MODE" */
extern char sEdTrapSelText[]; /**< "   TRAP  SEL\n\n" */
extern char sEdTrapNoRangeText[]; /**< "NO.    0<-" */
extern char sEdTrapModelRangeFmt[]; /**< "MODEL. 0<-%2d->%2d\n" */
extern char sEdUseBufferFmt[]; /**< "   USE BUFFER %d\n" */
extern char sEdTrapSwitchRangeText[]; /**< "SWITCH.0<-" */
extern char sEdTrapTypeRangeText[]; /**< "TYPENO.0<-" */
extern char sEdTrapActRangeText[]; /**< "ACT NO.0<-" */
extern char sEdAngleRangeFmt[]; /**< "  L1 0<-%3d->360 R1\n" */
extern char sEdSpriteSelModeText[]; /**< "SPR SEL MODE" */
extern char sEdSpriteSelText[]; /**< "   SPRITE SEL\n\n" */
extern char sEdSpriteNoRangeText[]; /**< "  NO.    0<-" */
extern char sEdSpriteModelRangeFmt[]; /**< "  MODEL. 0<-%3d->%3d\n" */
extern char sEdSpriteSwitchRangeText[]; /**< "  SWITCH.0<-" */
extern char sEdSpriteSizeRangeFmt[]; /**< "  SIZE. 0<-%3d->%3d\n" */
extern char sEdHitLabel[]; /**< "  HIT.  " */
extern char sEdFlipLabel[]; /**< "  FLIP.  " */
extern char sEdActLabel[]; /**< "  ACT.   " */
extern char sEdNearActText[]; /**< "NEAR ACT\n" */
extern char sEdHitActText[]; /**< "HIT ACT\n" */
extern char sEdWalkLabel[]; /**< "  WALK.  " */
extern char sEdSeLabel[]; /**< "  SE.    " */
extern char sEdWarningText[]; /**< "!!! WARNING !!!" */
extern char sEdCansTrapsLine[]; /**< "PEPSICAN TO TRAP TO" */
extern char sEdSwitchesCantPlaceLine[]; /**< "SWITCH GA OKEMASEN." */
extern char sEdGroundHitLine[]; /**< "JIMEN NO HIT GA" */
extern char sEdUntakeableHereLine[]; /**< "TORENAI BASHO NI" */
extern char sEdPushCrossText[]; /**< "            PUSH BATU" */
extern char sEdBufferMaxLine[]; /**< "BAFFER GA MAX NI NARI" */
extern char sEdReachedNoMoreLine[]; /**< "MASITA. KORE IJYOU HA" */
extern char sEdCantPlaceLine[]; /**< "OKEMASEN." */
extern char sEdAskMiyagoeLine[]; /**< "MIYAGOE SAMA NI KIITE" */
extern char sEdPleaseLine[]; /**< "KUDASAI." */
extern char sEdSwitchesDeleteLine[]; /**< "SWITCH GA SAKUJYO" */
extern char sEdCannotLine[]; /**< "DEKIMASEN." */
extern char sEdBufferZeroLine[]; /**< "BAFFER GA ZERO NI NARI" */
extern char sEdCantDeleteLine[]; /**< "SAKUJYO DEKIMASEN." */
extern char sEdAreaDeleteLine[]; /**< "KONO AREA NI SAKUJYO" */
extern char sEdDeletableLine[]; /**< "DEKIRU MONO HA" */
extern char sEdThereAreNoneLine[]; /**< "ARIMASEN." */
extern char sEdTrapsSwitchesLine[]; /**< "TRAP TO SWITCH GA" */
extern char sEdNotNearbyLine[]; /**< "CHIKAKU NI ARIMASEN." */
extern char sEdWhatToEraseLine[]; /**< "KESITAI MONO NO " */
extern char sEdMoveNearLine[]; /**< "CHIKAKU NI IDOU" */
extern char sEdPleaseDoLine[]; /**< "SITEKUDASAI." */
extern char sEdColorCyan[]; /**< "~c088" */
extern char sEdColorRed[]; /**< "~c800" */
extern char sEdPosYFmt[]; /**< "Y %d " */
extern char sEdPosZFmt[]; /**< "Z %d" */
extern char sEdMenuPepsiText[]; /**< "PEPSI\n" */
extern char sEdMenuSwitchText[]; /**< "SWITCH\n" */
extern char sEdMenuTrapText[]; /**< "TRAP\n" */
extern char sEdMenuSpriteText[]; /**< "SPRITE\n" */
extern char sEdMenuDeleteText[]; /**< "DELETE\n" */
extern char sEdMenuInfoText[]; /**< "INFO\n" */
extern char sEdMenuDefPosText[]; /**< "DEFPOS\n" */
extern char sEdMenuDataText[]; /**< "DATA\n" */
extern char sEdColorGrey[]; /**< "~c444" */
extern char sEdNum2Fmt[]; /**< "%2d" */
extern char sEdRangeEndBlankFmt[]; /**< "->%d\n\n" */
extern char sEdNewlines4[]; /**< "\n\n\n\n" */
extern char sEdRangeEndFmt[]; /**< "->%d\n" */
extern char sEdRangeEnd2Fmt[]; /**< "->%2d\n" */
extern char sEdNewlines5[]; /**< "\n\n\n\n\n" */
extern char sEdOffLine[]; /**< "OFF\n" */
extern char sEdOnLine[]; /**< "ON\n" */
extern char sEdNoActLine[]; /**< "NO ACT\n" */
extern char sEdSeNoFmt[]; /**< "NO %3d\n" */
extern char sEdManLine[]; /**< "MAN\n" */
extern char sEdWomanLine[]; /**< "WOMAN\n" */
extern char sEdChildLine[]; /**< "CHILD\n" */
extern char sEdDogLine[]; /**< "DOG\n" */
extern char sEdYouAreLine[]; /**< "IMASU." */
extern VECTOR D_800ACED0; /**< the position printed on the status panel */

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
    Rec5Cv *recs;
    Obj48 *objs;

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
    FntPrint(gNowBlockFmt, D_80095824, gStageObjCount);
    D_800ACED0.vx = sGameSave.unk348[0];
    D_800ACED0.vy = sGameSave.unk348[1];
    D_800ACED0.vz = sGameSave.unk348[2];
    FntPrint(sEdPositionFmt, D_800ACED0.vx);
    if (sTotals.unk8 == 0) {
        FntPrint(sEdColorWhite);
    } else if (sTotals.unk8 < 0) {
        FntPrint(sEdColorBlue);
    } else {
        FntPrint(sEdColorRed);
    }
    FntPrint(sEdPosYFmt, D_800ACED0.vy);
    FntPrint(sEdColorWhite);
    FntPrint(sEdPosZFmt, D_800ACED0.vz);
    if (D_80095824 != -1) {
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
            FntPrint(sEdPepsiFmt, sTotals.unk24 + 1);
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
            FntPrint(sEdPepsiTypeRangeFmt, D_80095B4C[0] + 1);
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
            recs = sRecs5C;
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
            objs = (Obj48 *)D_800A9008;
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
            FntPrint(sEdUseBufferFmt, D_8009596E);
            PrintTabs(3);
            if (gMenuLine == 2) {
                FntPrint(sEdColorWhite);
            } else {
                FntPrint(sEdColorBlue);
            }
            FntPrint(sEdTrapSwitchRangeText);
            recs = sRecs5C;
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
            if (D_800A7898[sTotals.unk28].unk0 == -1) {
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
            recs = sRecs5C;
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
            switch (D_800959E2) {
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
```

`sTotals`, `sGameSave`, `sRecs5C`, `Rec5Cv` and `Obj48` are the unit's
existing views (declared above UpdateEditor). `D_800ACED0` (a VECTOR two
GsOTs past `gSubOt`) and the strings are new externs in the unit; the
prototype `void PrintEditorStatus(void);` above UpdateEditor became `s32`.

## How it closed (5 builds, no search)

| step | words |
| --- | --- |
| m2c draft, ternaries `FntPrint(c ? A : B)` | 104/1419, 36 words short, frame 0x38 |
| every ternary as `if (c) FntPrint(A); else FntPrint(B);`, `s32 unused[2]` | 1408/1419, length exact |
| `s32` with no return (jump table's bound-check slot holds the `sll`) | 1411/1419 |
| `(*(T (*)[200])D)[i]` for the tables | 1411 (no change) |
| table bases in pointer locals, `recs = sRecs5C; recs[i]` | 1419/1419 |

- **Ternary vs. two calls**: a `char *` choice shares one `%hi` (both
  strings are in `.sdata`) and hoists it into the branch's delay slot;
  retail builds each string's `lui` in its own arm, which is one call per
  arm (LEARNINGS, PrintToolSlots). The nested `unk8` test is an
  if/else-if chain of three calls.
- **Table-first `lui`**: `((T *)D)[i]` and the pointer-to-array cast both
  load the index's `lui` first (fold puts the address constant second);
  a real array, or a pointer local assigned just before the test, loads
  the table first. `D_800A7898` is a real `Rec3C` array and needed
  nothing.
- **Address CSE** across calls (`sEdColorWhite` in `$s1`/`$s0`,
  `sEdNewlines3`/`sEdNewline`/`sEdNewlines2` in the warning panels) came out
  on its own from straight-line source.

### Proposed learning

- **An indexed table read whose table `lui` precedes the index's `lui`:
  the base went into a pointer local** (`recs = (T *)D; recs[i].f`), or
  the symbol is a real array of `T`. The cast form `((T *)D)[i]` loads
  the index first. (PrintEditorStatus) Note LEARNINGS' "A table index shifted
  before the table's `lui`: the table address went into a pointer local
  first" (PlaySe) describes the `sll` placement, not the `lui`
  order; both hold.
- **Menu lines `FntPrint(sel == k ? HI : LO)`: write two calls**; the
  ternary shares the `%hi` and changes the length by a word per line.
  (PrintEditorStatus)
