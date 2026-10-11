# PrintToolCardMenu — MATCHED 216/216 (round 9, echo)

> Renamed from `func_80035E24` on 2026-10-11 (tools/rename.py). Address 0x80035e24.

Unit `src/hit_tool.c`. Prints the tool's help panel: a title, then by
`gToolCardStep` (jump table `jtbl_800118AC`, 18 entries; 3, 5, 9 and anything
past 17 print nothing) one or two prompt lines, some with the 1-based
`gToolSlot`, some followed by a sub-menu (`PrintSaveLoad`,
`PrintToolSlots`, `PrintYesNo`); then draws the panel's frame with
`SortSemiQuad`.

```c
extern char sHitNewlines3[]; /**< a line prefix */
extern char sHitCheckText[]; /**< not yet known */
extern char sHitDataModeText[];
extern char sHitModeSelectText[];
extern char sHitBufferSelectText[];
extern char sHitCreateFileText[];
extern char sHitCardFormatText[];
extern char sHitSaveToDataFmt[];
extern char sHitOverwriteOkText[];
extern char sHitDataWritingText[];
extern char sHitLoadFromDataFmt[];
extern char sHitLoadOkText[];
extern char sHitCantOpenDataFmt[];
extern char sHitCompleteText[];
extern char sHitErrorText[];
extern char sHitNotEnoughMemoryText[];
extern char sHitNoCardText[];
extern char sHitNoFileText[];
extern char sHitCreateNewFileText[];
extern char sHitUnformattedText[];
extern char sHitFormatOkText[];

/* Defined by editor, which has no header. */
void PrintYesNo(void);
void PrintSaveLoad(void);

void PrintToolCardMenu(void) {
    PrintTabs(5);
    FntPrint(sHitDataModeText);
    FntPrint(sHitNewlines3);
    switch (gToolCardStep) {
        case 0:
            FntPrint(sHitNewlines3);
            PrintTabs(4);
            FntPrint(sHitCheckText);
            break;
        case 1:
            FntPrint(sHitNewlines3);
            PrintTabs(3);
            FntPrint(sHitModeSelectText);
            PrintSaveLoad();
            break;
        case 2:
            FntPrint(sHitNewlines3);
            PrintTabs(3);
            FntPrint(sHitBufferSelectText);
            PrintToolSlots();
            break;
        case 16:
            FntPrint(sHitNewlines3);
            PrintTabs(3);
            FntPrint(sHitCreateFileText);
            break;
        case 17:
            FntPrint(sHitNewlines3);
            PrintTabs(3);
            FntPrint(sHitCardFormatText);
            break;
        case 4:
            FntPrint(sHitNewlines3);
            PrintTabs(3);
            FntPrint(sHitSaveToDataFmt, gToolSlot + 1);
            PrintTabs(3);
            FntPrint(sHitOverwriteOkText);
            PrintYesNo();
            break;
        case 8:
            FntPrint(sHitNewlines3);
            PrintTabs(3);
            FntPrint(sHitDataWritingText);
            break;
        case 6:
            FntPrint(sHitNewlines3);
            PrintTabs(3);
            FntPrint(sHitLoadFromDataFmt, gToolSlot + 1);
            PrintTabs(3);
            FntPrint(sHitLoadOkText);
            PrintYesNo();
            break;
        case 7:
            FntPrint(sHitNewlines3);
            PrintTabs(2);
            FntPrint(sHitCantOpenDataFmt, gToolSlot + 1);
            break;
        case 10:
            FntPrint(sHitNewlines3);
            PrintTabs(3);
            FntPrint(sHitCompleteText);
            break;
        case 11:
            FntPrint(sHitNewlines3);
            PrintTabs(2);
            FntPrint(sHitErrorText);
            break;
        case 12:
            FntPrint(sHitNewlines3);
            PrintTabs(2);
            FntPrint(sHitNotEnoughMemoryText);
            break;
        case 13:
            FntPrint(sHitNewlines3);
            PrintTabs(3);
            FntPrint(sHitNoCardText);
            break;
        case 14:
            FntPrint(sHitNewlines3);
            PrintTabs(4);
            FntPrint(sHitNoFileText);
            PrintTabs(3);
            FntPrint(sHitCreateNewFileText);
            PrintYesNo();
            break;
        case 15:
            FntPrint(sHitNewlines3);
            PrintTabs(3);
            FntPrint(sHitUnformattedText);
            PrintTabs(3);
            FntPrint(sHitFormatOkText);
            PrintYesNo();
            break;
    }
    SortSemiQuad(0x421, -100, -60, 100, -60, 100, 60, -100, 60, 0);
}
```

## Levers

First build matched. The cases are written in retail's block order (0, 1,
2, 16, 17, 4, 8, 6, 7, 10..15), each with its own calls; cross-jumping
merges the shared `FntPrint(...); PrintYesNo();` tails and the
`li a0, 0x421` exits by itself.

## Declarations

`PrintYesNo` and `PrintSaveLoad` (editor, which has no header) and
the rodata format strings are declared in this `.c`; declcheck lists the
two functions LOCAL.
