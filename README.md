# Pepsiman decompilation

A complete, matching decompilation of **Pepsiman** (PlayStation, 1999,
KID / Sony, SLPS-01762).

The C in `src/` compiles into an executable, and 22 code overlays, that are
byte-for-byte identical to the ones on the retail disc. Every one of the
game's functions is C; the work left is names, types and constants.

## Building it

This repository contains no game data and no Sony files. You need your own:

- **The game**: the Japanese retail disc, SLPS-01762. Extract the
  executable and the overlays from the redump's data track:

  ```sh
  python3 tools/extract_exe.py "Pepsiman (Japan) (Track 1).bin"
  python3 tools/extract_overlays.py "Pepsiman (Japan) (Track 1).bin"
  ```

  The executable must be `SLPS_017.62` with SHA1
  `116d895763ebef141b4e2c3c90c1b227d257b804`. No other version works.

- **Sony's Psy-Q SDK**: this redump zip from
  <https://archive.org/download/ps1_sdks>, in `sdk/`, still zipped and
  with its original name:

  ```
  Programmer Tool - Runtime Library Version 4.4 (Japan)_DTL-S2350_redump.zip
  ```

On Linux, with `python3`, `git`, `curl`, `make` and a C compiler:

```sh
./tools/setup.sh          # one time: toolchain, SDK, first verified build
./build-and-verify.sh     # every build after that
```

A good build ends with `OK: build matches retail SLPS_017.62 and its
overlays`.

## Reading the code

| file | what's in it |
| --- | --- |
| `src/main.c` | `main` and its game-mode loop, the CD loader, pads and rumble, load screens |
| `src/stage_flow.c` | playing a stage: pause, reset, restart, the effect pool |
| `src/stage_overlay.c` | loading each stage's overlay, the stage tables |
| `src/stage.c` | stage models and sections, the HUD, the camera, results screens |
| `src/player.c` | the player: steering, speed, jump, slide, getting hit, the miss sequence |
| `src/object.c` | stage objects, floor and wall queries, paths, pickups |
| `src/collision.c` | the player's collision points, spheres and quads |
| `src/model.c` | TIM and TOD (animation) players, CD audio, lights, the sky |
| `src/draw.c`, `src/draw_tmd.c` | sprites and billboards, the TMD polygon handlers |
| `src/sound.c` | sound effects and music, XA, the movie player |
| `src/card.c` | memory card saves |
| `src/editor.c`, `src/hit_tool.c` | the developers' stage editor and hit-data tool, left in the retail build |
| `src/ovl/` | the overlays: the menu, the fifteen stages, the five world intros |

To change the code and keep it matching, read
[`docs/CONTRIBUTING.md`](docs/CONTRIBUTING.md).

## A note on AI

The decompilation was heavily assisted by AI (Claude Code), and checked at
every step against the retail executable's SHA1. The tools and method come
from [lsddecomp](https://github.com/brian-oblivion/lsddecomp).

## Licence

This project's own work (the C, headers, configuration and tools) is
[CC0 1.0](LICENSE). The game, Sony's SDK and the maspsx patches in
`tools/patches/` are not; see `LICENSE`.

## Credits

- [lsddecomp](https://github.com/brian-oblivion/lsddecomp) (CC0), for the
  tools and the method.
- [parasite-eve-2-decomp](https://github.com/GabeRealB/parasite-eve-2-decomp)
  (CC0), for linking Sony's SDK objects and `include/gte_macros.inc`.
- The decomp toolchain: [splat](https://github.com/ethteck/splat),
  [maspsx](https://github.com/mkst/maspsx),
  [m2c](https://github.com/matt-kempster/m2c),
  [asm-differ](https://github.com/simonlindholm/asm-differ),
  [decomp-permuter](https://github.com/simonlindholm/decomp-permuter),
  [objdiff](https://github.com/encounter/objdiff), pcsx-redux's
  psyq-obj-parser, and GCC 2.8.1 from
  [decompals/old-gcc](https://github.com/decompals/old-gcc).
