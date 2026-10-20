# Contributing

How to change the code without breaking the match. The README covers
getting a first build; this page assumes `./tools/setup.sh` has run.

## Why the SDK disc

The game links Sony's Psy-Q libraries. The build doesn't re-derive that
code as C. It links **Sony's own objects**, listed in
`config/psyq-objects.txt` and placed exactly where the game put them.
Pepsiman links Runtime Library **4.4** (DTL-S2350), measured with
`tools/psyq_sdk.py match`; that one disc gives the build both the library
objects (`lib/`) and Sony's headers (`include/psyq/`). The other discs are
only needed to repeat the measurement. Two pieces stay disassembly: SN's
crt0, which the object converter cannot read, and `libgs/gs_001`, whose
`.bss` Sony's linker scattered.

## Measuring

```sh
python3 tools/progress.py      # matched functions and bytes, per file
python3 tools/readability.py   # what still reads like a decompiler's output
```

## The executable

One plain PS-X EXE loaded at `0x80010000`, entry `0x80042C58`, `$gp`
`0x800954C4`. `config/splat.slps01762.pepsiman.yaml` holds the
segmentation, measured from the bytes:

| file offset | vram | what |
| --- | --- | --- |
| `0x800` | `0x80010000` | read-only data: strings, jump tables |
| `0x396C` | `0x8001316C` | game code (`src/`) |
| `0x33458` | `0x80042C58` | Sony: SN's crt0 (the entry point), then the Psy-Q 4.4 objects |
| `0x62C84` | `0x80072484` | initialized data |
| `0x85CC4` | `0x800954C4` | `.sdata` (gp-relative) |
| `0x85ED0` | `0x800956D0` | `.sbss`; `.bss` runs on to `0x800FAE80`, past the file |

## The overlays

`LoadOverlay` reads code from the disc to `0x800F0000` at run time: 22 files
under `CDDATA/` (`tools/extract_overlays.py` lists them). Overlay 2 is the
menu, 3 to 17 are the fifteen stages (`LoadStageOverlay`), 18 to 22 the five
world intros, and 0 is a mode `main` loads first. Each is its own image: a
splat config in `config/ovl/<ovl>.yaml`, a C unit in `src/ovl/<ovl>.c`, its
own link against the executable's symbol addresses, and its retail SHA1 in
`check-overlays.sha1`. The Makefile's `OVERLAYS` names the ones the build
covers. Overlay code is compiled with `-G0` (the executable's is `-G8`).
`tools/funcdiff.py` scores overlay functions too; `--ovl <ovl>` picks one
when a name is in several overlays.

## Changing the code and keeping it matching

Every change is checked the same way: edit, build, verify.

```sh
./build-and-verify.sh > /tmp/build.log 2>&1; echo "build exit=$?"
grep -nE 'error:|parse error|undefined reference|\*\*\* \[[^]]*\.o\]' /tmp/build.log | head
.venv/bin/python3 tools/funcdiff.py <func>          # how many words match, for one function
.venv/bin/python3 tools/asm-differ/diff.py <func>   # side-by-side diff against retail
```

- **exit 1**: `disk/SLPS_017.62` is missing or is the wrong dump.
- **exit 2 with a grep hit**: the C didn't compile. Psy-Q GCC predates the
  `error:` prefix, so the `*** [...o]` pattern, which matches make failing
  on an object file, is the reliable signal.
- **exit 2 and no hit**: it compiled, but the image differs from retail.
  `cmp -l build/SLPS_017.62 disk/SLPS_017.62 | head` gives 1-based file
  offsets; the address is `(offset - 1) - 0x800 + 0x80010000`, and
  `build/pepsiman.map` names the function at that address.

The rules that break the image if you ignore them:

- **Functions stay in ROM-address order within a file.** A function out of
  order still links, but the image comes out wrong in functions you didn't
  touch.
- **C89, as GCC 2.8.1 reads it.** Declarations go at the top of a block.
  Only `/* */` comments. `char` is unsigned, so a signed byte is `s8`.
- **A rodata string is defined once, by name.** A literal written at a use
  emits a second copy and shifts the image.
- **A struct edit is never local.** A field inserted without shrinking the
  padding beside it moves every later offset, and breaks functions in other
  files. Rebuild the whole image after any header change.
- **An odd spelling that exists to match gets one `MATCHING:` comment**
  beside it in the `.c` file.
- **The toolchain is pinned.** The compiler, its flags and `maspsx` (which
  reproduces Sony's assembler's macro expansions) are part of the retail
  bytes, not knobs to turn.

Rename through the tools, which update every reference:

```sh
python3 tools/rename.py OLD NEW          # a function or global
python3 tools/renametype.py OLD NEW      # a type
python3 tools/unitfile.py rename OLD NEW # a source file
```

Name a sprite by looking at it. `tools/sprites.py <disc image>` renders
every sprite the game defines, from the textures on your disc, into
`build/sprites/index.html`. `tools/sounds.py <disc image>` does the same
for the sound effects: it renders every id PlaySe plays, from the sound
banks its callers have loaded, into `build/sounds/index.html` (players that
load nothing until clicked) and describes each in `sounds.json`.
`tools/models.py <disc image>` renders every stage's trap and object
models (the TMD a stage loads to `RAM_OBJECT_MODELS`) into
`build/models/index.html`, with each placed trap's type, act and the
functions StepStageTrap reaches for it, so a trap routine can be named
from the pictures of what it moves. The pictures, sounds and models are
the game's: they stay in `build/` and are never committed.

Never edit `asm/` (it is regenerated) or `check.sha1` (it is the retail
hash). Run `tools/lint.sh` before sending a change.

## Continuous integration

- **`lint`** runs on every push and pull request: formatting (clang-format
  22), the API documentation check, the readability counters, one
  declaration per name, snake_case file names, and a Doxygen build that
  fails on any warning. The two checks that preprocess the source need
  Sony's headers, so they run only where the headers are available.
- **`build`** rebuilds the executable and the overlays, checks them against
  `check.sha1` and `check-overlays.sha1`, and publishes the objdiff report
  [decomp.dev](https://decomp.dev) reads. The disc and the SDK never enter
  this repository, so the job reads them from a private repository the
  owner keeps (`disk/SLPS_017.62`, `lib/` and `include/psyq/`), named in
  the `DEPS_REPO` variable and read with the `DEPS_TOKEN` secret. Without
  the secret the job skips.

## Method

The tools and the way of working come from
[lsddecomp](https://github.com/brian-oblivion/lsddecomp), a finished
matching decompilation of *LSD: Dream Emulator*. Its matching guide,
parallel-runs protocol and toolchain research are in
[`lsd-reference/`](lsd-reference/); the source shapes proven on GCC 2.8.1
are in [`LEARNINGS.md`](LEARNINGS.md).
