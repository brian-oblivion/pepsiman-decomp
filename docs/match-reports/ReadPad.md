# ReadPad — MATCHED 369/369 (round 11, delta)

> Renamed from `func_800142EC` on 2026-10-10 (tools/rename.py). Address 0x800142ec.

Unit `src/main.c`. The per-frame pad update: sets the speed byte
`gRepeatSpeed` to 4, polls the pads (PollPads), centres both analogue
axes when the pad type is 0x41, copies pad `arg`'s held and pressed words
(`gPadHeld[arg]` to `gPad`, `gPadPressed[arg]` to `gPadTrig`),
then runs an auto-repeat on the four d-pad bits and, when
`gRepeatButtonsOn == 1`, on 0x80, 0x40 and 0x20. Called from EndFrame,
AdjustScreenPosition and sound.

The signature is common.h's, `void ReadPad(s32 arg)`; the body
indexes with `(u16)arg`, which gives retail's `andi 0xFFFF` at the use.

The body is in `src/main.c`: seven expansions of one `REPEAT(bit, cnt)`
macro (held for 30/speed frames, then the bit repeats every 4/speed
frames on the frame counter `gFrameCount`; release clears the counter).

## Levers

Four builds: 362 -> 369 -> 363 (do/while macro) -> 369.

- **The two pad-word copies are written pressed-word first.** Retail
  builds `gPadPressed`'s address before `gPadHeld`'s; source order sets
  that. The stores still come out `gPad` first (362 -> 369; the
  only residue was the two loads' registers).
- **The repeat macro is a bare `{ }` block.** Wrapping it in
  `do { } while (0)` changes the branch layout (369 -> 363).
- `gStickRawX = gStickRawY = 0x80;` stores C8 first, as retail
  (LEARNINGS, chained assignment stores right to left).
- The first expansion reuses the pressed word still in `$a1` for
  `|= 0x1000`; plain C gives it.

## Declarations left for the head

declcheck flags three globals this unit now shares with another unit's
local declaration: `gPadType` (`u8`, player.c), `gStickRawY` and
`gStickRawX` (`u32`, editor.c). They belong in common.h.

### Proposed learning

- **Two indexed loads whose addresses are built in the other order:
  swap the two statements**; the stores can keep retail's order.
  (ReadPad)
