# PrintEditorSlots — MATCHED 106/106 (round 8, bravo)

> Renamed from `func_800327BC` on 2026-10-11 (tools/rename.py). Address 0x800327bc.

Unit `src/editor.c`. Prints the three save slots of the tool buffer
(`0x8016D000`, 0xB774-byte `SaveSlot`s) as a menu: the highlight colour
for the selected line, then "DATA:%d " and "STAGE %d-" with "1", "2" or
"BOSS" for a valid slot (`unk200 == 0x38`), else "NO DATA". Third build
matched.

## Shapes read from retail

- **The buffer base in a saved register (`lui`/`ori` into `$s3` before the
  first call): a pointer local set before the loop**, `slots = (SaveSlot
  *)0x8016D000; slot = &slots[i];`. Indexing the literal cast rebuilds the
  constant every pass.
- **A quotient masked (`andi 0xFFFF`) for one use and used unmasked in the
  remainder: `u16 stage = v / 3;` and `part = v % 3;`**; cc1 computes the
  remainder from the shared quotient. `part = v - stage * 3` multiplies the
  masked value.
- The slot owner byte `unk202` is the stage and part, `stage * 3 + part`.
- The strings are `.rodata` (`sEdDataNoFmt`, `sEdStageFmt`, `sEdNoDataText`)
  and `.sdata` (`sEdArea1Text`, `sEdArea2Text`, `sEdBossText`, `sEdNewline`)
  symbols, declared `extern char []`.

```c
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
```
