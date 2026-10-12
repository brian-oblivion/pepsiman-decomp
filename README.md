# pepsiman-decomp

A matching decompilation of **Pepsiman** (PlayStation, 1999, Japan,
SLPS-01762).

"Matching" means the C in `src/` compiles, with the Psy-Q GCC 2.8.1
toolchain the game was built with, into an executable that is
**byte-for-byte identical** to the one on the retail disc.

## Status

Every game function is C and the executable rebuilds byte for byte. Sony's
library code is linked from Sony's own Psy-Q objects. The work now is
readability: names, types and constants. To measure:

```sh
python3 tools/progress.py      # matching
python3 tools/readability.py   # what still reads like a decompiler's output
```

## The source

| file | what it holds |
| --- | --- |
| `src/main.c` | `main` and its game-mode loop, frame end, CD loader, pads and rumble, load screens |
| `src/stage_flow.c` | playing a stage: pause, reset, restart, the effect pool, the stage overlay dispatch |
| `src/stage_overlay.c` | per-stage overlay loading and entry points, stage tables |
| `src/stage.c` | stage models and sections, the HUD, the camera, results screens, effect steps |
| `src/player.c` | the player: steering, speed, jump, slide, hit reactions, the miss sequence |
| `src/object.c` | stage objects, floor and wall queries, paths, pickups, two debug viewers |
| `src/collision.c` | the player's collision points, spheres and quads |
| `src/model.c` | TIM and TOD (animation) players, CD audio tracks, lights, sky, the motion viewer |
| `src/draw.c`, `src/draw_tmd.c` | sprites and billboards, the TMD polygon handlers; the overlay loader |
| `src/sound.c` | SEQ/VAB sound, fades, XA, and the streamed-movie player |
| `src/card.c` | memory card files |
| `src/editor.c`, `src/hit_tool.c` | the developers' stage-object editor and hit-data tool, still in the retail build |

## What is and is not in this repository

**This repository contains no game data and no Sony files.** You bring your
own copies, and the build reads them from directories git ignores:

| you provide | where | what the build does with it |
| --- | --- | --- |
| the game executable `SLPS_017.62` (or the disc image it comes from) | `disk/` | disassembles it (`asm/`, generated), and checks the rebuilt image against it |
| the code overlays, from the same disc image | `disk/ovl/` | the same, one image per overlay |
| Sony's Psy-Q "Runtime Library" SDK discs | `sdk/` | converts Sony's library objects into `lib/` and copies Sony's headers into `include/psyq/`, both generated and ignored |

## Building it

You need Linux, `python3`, `git`, `curl`, `make`, `sha1sum` and a host C
compiler, plus:

- **The game.** The Japanese retail release, SLPS-01762. Extract the
  executable from the redump's data track:

  ```sh
  python3 tools/extract_exe.py "Pepsiman (Japan) (Track 1).bin"   # writes disk/SLPS_017.62
  ```

  The expected file is `SLPS_017.62`, 548864 bytes, SHA1
  `116d895763ebef141b4e2c3c90c1b227d257b804`. No other revision is
  supported. Then the code overlays, from the same image:

  ```sh
  python3 tools/extract_overlays.py "Pepsiman (Japan) (Track 1).bin"  # writes disk/ovl/, checked against check-overlays.sha1
  ```

- **The Psy-Q SDK.** "Programmer Tool - Runtime Library" discs, as redump
  zips (don't unpack them), in `sdk/`. They are on archive.org
  (<https://archive.org/download/ps1_sdks>). The version is read from the
  file name, so keep the original names. Pepsiman links Runtime Library
  **4.4** (DTL-S2350), measured with `tools/psyq_sdk.py match`; that one
  disc gives the build both the library objects and Sony's headers. The
  other discs are only needed to repeat the measurement.

Then:

```sh
./tools/setup.sh          # toolchain, venv, SDK conversion, split, and a verified first build
./build-and-verify.sh     # build, then compare the whole image with retail
```

`setup.sh` fetches the prebuilt GCC 2.8.1, builds `mipsel-linux-gnu`
binutils, installs splat into a venv, and clones maspsx (patched from
`tools/patches/`), m2c, asm-differ and decomp-permuter. After that,
`./build-and-verify.sh` is the only build command. It ends `OK: build
matches retail SLPS_017.62 and its overlays` when every image is exact.

## The executable

One plain PS-X EXE loaded at `0x80010000`, entry `0x80042C58`, `$gp`
`0x800954C4`. `config/splat.slps01762.pepsiman.yaml` holds the
segmentation, measured from the bytes:

| file offset | vram | what |
| --- | --- | --- |
| `0x800` | `0x80010000` | read-only data: strings, jump tables |
| `0x396C` | `0x8001316C` | game code (`src/`) |
| `0x33458` | `0x80042C58` | Sony: SN's crt0 (the entry point), then the Psy-Q 4.4 objects, to `0x80072484` |
| `0x62C84` | `0x80072484` | initialized data |
| `0x85CC4` | `0x800954C4` | `.sdata` (gp-relative) |
| `0x85ED0` | `0x800956D0` | `.sbss`; `.bss` runs on to `0x800FAE80`, past the file |

Sony's objects are listed in `config/psyq-objects.txt` and linked from
`lib/`; two pieces stay disassembly (SN's crt0, which the object converter
cannot read, and `libgs/gs_001`, whose `.bss` Sony's linker scattered).

## The overlays

`LoadOverlay` reads code from the disc to `0x800F0000` at run time: 22 files
under `CDDATA/` (`tools/extract_overlays.py` lists them). Overlay 2 is the
menu, 3 to 17 are the fifteen stages (`LoadStageOverlay`), 18 to 22 the five
world intros, and 0 is a mode `main` loads first. Each is its own image: a
splat config in `config/ovl/<ovl>.yaml`, a C unit in `src/ovl/<ovl>.c`, its
own link against the executable's symbol addresses, and its retail SHA1 in
`check-overlays.sha1`. The Makefile's `OVERLAYS` names the ones the build
covers. Overlay code is compiled with `-G0` (the executable's is `-G8`).

## Changing the code and keeping it matching

```sh
./build-and-verify.sh > /tmp/build.log 2>&1; echo "build exit=$?"
grep -nE 'error:|parse error|undefined reference|\*\*\* \[[^]]*\.o\]' /tmp/build.log | head
.venv/bin/python3 tools/funcdiff.py <func>          # does it match yet, in words
.venv/bin/python3 tools/asm-differ/diff.py <func>   # side-by-side diff against retail
```

- **exit 1**: `disk/SLPS_017.62` is missing or is the wrong dump.
- **exit 2 with a grep hit**: the C didn't compile. Psy-Q GCC predates the
  `error:` prefix, so the `*** [...o]` pattern is the reliable signal.
- **exit 2 and no hit**: it compiled, but the image differs from retail.
  `cmp -l build/SLPS_017.62 disk/SLPS_017.62 | head` gives 1-based file
  offsets; the address is `(offset - 1) - 0x800 + 0x80010000`, and
  `build/pepsiman.map` names the function at that address.

The rules that break the image if you ignore them:

- **Functions stay in ROM-address order within a file.**
- **C89, as GCC 2.8.1 reads it.** Declarations at the top of a block, only
  `/* */` comments, `char` is unsigned (a signed byte is `s8`).
- **A rodata string is defined once, by name.** A literal written at a use
  emits a second copy and shifts the image.
- **A struct edit is never local.** Rebuild the whole image after any
  header change.
- **The toolchain is pinned.** The compiler, its flags and maspsx are part
  of the retail bytes, not knobs to turn.

Rename through the tools, which update every reference:

```sh
python3 tools/rename.py OLD NEW          # a function or global
python3 tools/renametype.py OLD NEW      # a type
python3 tools/unitfile.py rename OLD NEW # a source file
```

Never edit `asm/` (it is regenerated) or `check.sha1` (it is the retail
hash). Run `tools/lint.sh` before sending a change.

## Method

The tools and the way of working come from
[lsddecomp](https://github.com/brian-oblivion/lsddecomp), a finished
matching decompilation of *LSD: Dream Emulator* done largely by AI agents
working in parallel. Its matching guide, parallel-runs protocol and
toolchain research are in [`docs/lsd-reference/`](docs/lsd-reference/).

## Licence

[CC0 1.0](LICENSE). That covers only this project's own contribution. See
`LICENSE` for what it doesn't cover: the game itself, Sony's SDK, and the
maspsx patches in `tools/patches/`.

## Credits

- **[lsddecomp](https://github.com/brian-oblivion/lsddecomp)** (CC0), the
  tools and method.
- **[parasite-eve-2-decomp](https://github.com/GabeRealB/parasite-eve-2-decomp)** (CC0), for linking Sony's
  SDK objects instead of decompiling them, and for `include/gte_macros.inc`.
- **The decomp toolchain:** [splat](https://github.com/ethteck/splat),
  [maspsx](https://github.com/mkst/maspsx),
  [m2c](https://github.com/matt-kempster/m2c),
  [asm-differ](https://github.com/simonlindholm/asm-differ),
  [decomp-permuter](https://github.com/simonlindholm/decomp-permuter),
  [objdiff](https://github.com/encounter/objdiff) and pcsx-redux's
  psyq-obj-parser, and the prebuilt GCC 2.8.1 from
  [decompals/old-gcc](https://github.com/decompals/old-gcc).
