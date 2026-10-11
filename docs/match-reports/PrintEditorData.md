# PrintEditorData — MATCHED 216/216 (round 9, alpha)

> Renamed from `func_8003245C` on 2026-10-11 (tools/rename.py). Address 0x8003245c.

Unit `src/editor.c`. The data (memory card) mode's screen: prints the
title, then per state `gToolCardStep` (an 18-entry jump table, three states
print nothing) a colour, a message and for some states the save-slot list
(`PrintEditorSlots`), the mode menu (`PrintSaveLoad`) or a yes/no menu
(`PrintYesNo`), and always draws the frame with `SortSemiQuad`.
First build matched.

## Shapes read from retail

- Case bodies in retail's layout order (0, 1, 2, 16, 17, 4, 8, 6, 7,
  10..15), as LEARNINGS says; the three tails ending in
  `FntPrint(...); PrintYesNo();` are cross-jumped into the last one.
- Every message is a rodata or `.sdata` string symbol, declared
  `extern char D_...[]`.

## Declarations

- New in the `.c`: the string externs, forward prototypes of this unit's
  `PrintEditorSlots`, `PrintYesNo` and `PrintSaveLoad`, and
  `SortSemiQuad` (model, not in its header; MATCHING note).

```c
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
```
