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
    FntPrint(D_800110F8);
    FntPrint(D_80095528);
    switch (gToolCardStep) {
        case 0:
            FntPrint(D_80095528);
            PrintTabs(4);
            FntPrint(D_80095620);
            break;
        case 1:
            FntPrint(D_80095528);
            PrintTabs(3);
            FntPrint(D_80011104);
            PrintSaveLoad();
            break;
        case 2:
            FntPrint(D_80095528);
            PrintTabs(3);
            FntPrint(D_80011114);
            PrintEditorSlots();
            break;
        case 16:
            FntPrint(D_80095528);
            PrintTabs(3);
            FntPrint(D_80011124);
            break;
        case 17:
            FntPrint(D_80095528);
            PrintTabs(3);
            FntPrint(D_80011134);
            break;
        case 4:
            FntPrint(D_80095528);
            PrintTabs(3);
            FntPrint(D_80011144, gToolSlot + 1);
            PrintTabs(3);
            FntPrint(D_80011158);
            PrintYesNo();
            break;
        case 8:
            FntPrint(D_80095528);
            PrintTabs(3);
            FntPrint(D_80011168);
            break;
        case 6:
            FntPrint(D_80095528);
            PrintTabs(3);
            FntPrint(D_80011178, gToolSlot + 1);
            PrintTabs(3);
            FntPrint(D_8001118C);
            PrintYesNo();
            break;
        case 7:
            FntPrint(D_80095528);
            PrintTabs(2);
            FntPrint(D_8001119C, gToolSlot + 1);
            break;
        case 10:
            FntPrint(D_80095528);
            PrintTabs(3);
            FntPrint(D_800111B4);
            break;
        case 11:
            FntPrint(D_80095528);
            PrintTabs(2);
            FntPrint(D_800111C0);
            break;
        case 12:
            FntPrint(D_80095528);
            PrintTabs(2);
            FntPrint(D_800111D4);
            break;
        case 13:
            FntPrint(D_80095528);
            PrintTabs(3);
            FntPrint(D_800111EC);
            break;
        case 14:
            FntPrint(D_80095528);
            PrintTabs(4);
            FntPrint(D_800111FC);
            PrintTabs(3);
            FntPrint(D_80011208);
            PrintYesNo();
            break;
        case 15:
            FntPrint(D_80095528);
            PrintTabs(3);
            FntPrint(D_8001121C);
            PrintTabs(3);
            FntPrint(D_8001122C);
            PrintYesNo();
            break;
    }
    SortSemiQuad(0x421, -100, -60, 100, -60, 100, 60, -100, 60, 0);
}
```
