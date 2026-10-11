# PrintToolCardMenu — MATCHED 216/216 (round 9, echo)

> Renamed from `func_80035E24` on 2026-10-11 (tools/rename.py). Address 0x80035e24.

Unit `src/code_24748.c`. Prints the tool's help panel: a title, then by
`gToolCardStep` (jump table `jtbl_800118AC`, 18 entries; 3, 5, 9 and anything
past 17 print nothing) one or two prompt lines, some with the 1-based
`gToolSlot`, some followed by a sub-menu (`PrintSaveLoad`,
`PrintToolSlots`, `PrintYesNo`); then draws the panel's frame with
`SortSemiQuad`.

```c
extern char D_80095698[]; /**< a line prefix */
extern char D_8009569C[]; /**< not yet known */
extern char D_80011618[];
extern char D_80011624[];
extern char D_80011634[];
extern char D_80011644[];
extern char D_80011654[];
extern char D_80011664[];
extern char D_80011678[];
extern char D_80011688[];
extern char D_80011698[];
extern char D_800116AC[];
extern char D_800116BC[];
extern char D_800116D4[];
extern char D_800116E0[];
extern char D_800116F4[];
extern char D_8001170C[];
extern char D_8001171C[];
extern char D_80011728[];
extern char D_8001173C[];
extern char D_8001174C[];

/* Defined by code_1dc24, which has no header. */
void PrintYesNo(void);
void PrintSaveLoad(void);

void PrintToolCardMenu(void) {
    PrintTabs(5);
    FntPrint(D_80011618);
    FntPrint(D_80095698);
    switch (gToolCardStep) {
        case 0:
            FntPrint(D_80095698);
            PrintTabs(4);
            FntPrint(D_8009569C);
            break;
        case 1:
            FntPrint(D_80095698);
            PrintTabs(3);
            FntPrint(D_80011624);
            PrintSaveLoad();
            break;
        case 2:
            FntPrint(D_80095698);
            PrintTabs(3);
            FntPrint(D_80011634);
            PrintToolSlots();
            break;
        case 16:
            FntPrint(D_80095698);
            PrintTabs(3);
            FntPrint(D_80011644);
            break;
        case 17:
            FntPrint(D_80095698);
            PrintTabs(3);
            FntPrint(D_80011654);
            break;
        case 4:
            FntPrint(D_80095698);
            PrintTabs(3);
            FntPrint(D_80011664, gToolSlot + 1);
            PrintTabs(3);
            FntPrint(D_80011678);
            PrintYesNo();
            break;
        case 8:
            FntPrint(D_80095698);
            PrintTabs(3);
            FntPrint(D_80011688);
            break;
        case 6:
            FntPrint(D_80095698);
            PrintTabs(3);
            FntPrint(D_80011698, gToolSlot + 1);
            PrintTabs(3);
            FntPrint(D_800116AC);
            PrintYesNo();
            break;
        case 7:
            FntPrint(D_80095698);
            PrintTabs(2);
            FntPrint(D_800116BC, gToolSlot + 1);
            break;
        case 10:
            FntPrint(D_80095698);
            PrintTabs(3);
            FntPrint(D_800116D4);
            break;
        case 11:
            FntPrint(D_80095698);
            PrintTabs(2);
            FntPrint(D_800116E0);
            break;
        case 12:
            FntPrint(D_80095698);
            PrintTabs(2);
            FntPrint(D_800116F4);
            break;
        case 13:
            FntPrint(D_80095698);
            PrintTabs(3);
            FntPrint(D_8001170C);
            break;
        case 14:
            FntPrint(D_80095698);
            PrintTabs(4);
            FntPrint(D_8001171C);
            PrintTabs(3);
            FntPrint(D_80011728);
            PrintYesNo();
            break;
        case 15:
            FntPrint(D_80095698);
            PrintTabs(3);
            FntPrint(D_8001173C);
            PrintTabs(3);
            FntPrint(D_8001174C);
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

`PrintYesNo` and `PrintSaveLoad` (code_1dc24, which has no header) and
the rodata format strings are declared in this `.c`; declcheck lists the
two functions LOCAL.
