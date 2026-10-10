# UpdateEffects — MATCHED (116/116), round 8, runner charlie

> Renamed from `func_8003F664` on 2026-10-10 (tools/rename.py). Address 0x8003f664.

Unit `src/code_29f54.c`. Runs every active slot once a frame: walks the
active list from `gEffectActive` through each record's `next` byte (a negative
link ends it), calls the record's step function from the table `gEffectSteps`
(StepEffect0, StepEffect1, ... indexed by `unk12`) with the drawing
position, and picks the ordering table by distance (`2*d*d > 0x77A0F`, `d`
the x offset: far records go to `gOt`). A step that returns nonzero
is unlinked and pushed onto the free list `gEffectFree`, the reverse of
SpawnEffect. The first parameter is never read.

```c
/** @brief One step of a slot's animation; returns nonzero when it ends. */
typedef s32 (*SlotStep)(Slot *p, GsOT *ot, s16 x, s16 y, s16 z);

extern SlotStep gEffectSteps[];

void UpdateEffects(s32 unused, s16 x, s16 y, s16 z) {
    u8 i;
    u8 prev;
    u8 next;
    s32 d;
    s32 done;
    Slot *slot;

    prev = 0xFF;
    if ((s8)gEffectActive < 0) {
        return;
    }
    i = gEffectActive;
    do {
        slot = &sSlots[i];
        d = x + slot->unkC;
        next = slot->next;
        if (d * (d * 2) > 0x77A0F) {
            done = gEffectSteps[slot->unk12](slot, &gOt[gActiveBuff], x, y, z);
        } else {
            done = gEffectSteps[slot->unk12](slot, &gSubOt[gActiveBuff], x, y, z);
        }
        if (done) {
            if (prev == 0xFF) {
                gEffectActive = sSlots[i].next;
            } else {
                sSlots[prev].next = sSlots[i].next;
            }
            sSlots[i].next = gEffectFree;
            gEffectFree = i;
        } else {
            prev = i;
        }
        if ((s8)next < 0) {
            break;
        }
        i = next;
    } while (1);
}```

## Levers

- **The list head read twice, `lb` for the test and `lbu` for the walk:
  the guard first, then the assignment** (`if ((s8)gEffectActive < 0)
  return; i = gEffectActive;`). With `i` assigned first (or a `for` from
  `gEffectActive`) cc1 reused the `lb` value.
- **The loop test on `next` with `i = next` in the back-branch delay slot:
  `if ((s8)next < 0) break; i = next;`** at the bottom of an endless loop.
  `i = next; } while ((s8)next >= 0);` and `while ((s8)(i = next) >= 0)`
  both test the copy and cost a word.
- **One call per arm of the distance test**; cross-jumping shares the tail
  from the stack argument on, as retail does. One call with an `ot` local
  was far off.
- **`mult (d*2), d`: `d * (d * 2)`.** `d * 2 * d` swaps the operands.
- **`y` is an `s16` parameter even though retail re-extends it before each
  call** (`sll`/`sra` on `$s5` in both arms) while `x` and `z` are extended
  once: as `s32` with the conversion at the call, the build differed only in
  which saved registers held `y`, `prev` and the table base (106/116). As
  `s16` it is byte-exact. Reordering locals, moving `prev = 0xFF`, and a
  `steps` table local did not help.

### Proposed learning

- **A byte global read `lb` (test) then `lbu` (value): test it before
  assigning it.** (UpdateEffects)
- **A loop whose back branch tests the next link and copies it in the delay
  slot: `if ((s8)next < 0) break; i = next;` inside `do { } while (1)`.**
  (UpdateEffects)
