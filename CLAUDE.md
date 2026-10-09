# Pepsiman (PSX) — matching decompilation

C source meant to compile byte-for-byte to the retail `SLPS_017.62`
(SLPS-01762). README.md covers the layout and the build; read it first.
The tooling and the working method come from lsddecomp, a finished matching
decomp of another PS1 game; `docs/lsd-reference/` holds its matching guide,
its parallel-runs protocol and its toolchain research.

**Keep this file short.** It holds rules and the loop, never project state,
counts or history. A number written here is right for one session and wrong
for every one after. Measure instead (`python3 tools/progress.py`). Lessons
go in commit messages, or in `docs/` when they are method.

A tool comment that cites a document "on the `archive/process` branch"
means lsddecomp's branch, not this repository's
(`git -C <lsddecomp checkout> show archive/process:docs/<file>`). The
general guides from it are already in `docs/lsd-reference/`.

## Hard rules

1. **Never edit `check.sha1` or `build.sha1`.** They hold the retail SHA1.
   Changing one to make a build pass deletes the oracle. A hook blocks it.
2. **Every build goes through `./build-and-verify.sh`.** A bare `make` says
   nothing about whether the bytes are right. A hook blocks in-repo `make`
   except `extract`, `progress`, `format`, `clean` and `nonmatching`.
3. **Never commit the executable, a disc image, or anything of Sony's.**
   `disk/`, `sdk/`, `lib/` and `include/psyq/` are gitignored and generated
   from the user's own files (`tools/psyq_sdk.py install`).
4. **Never commit anything that identifies the operator**: no real name, OS
   username, home-directory path, email or hostname, in code, docs, tool
   output, commit messages or PR text. Use repo-relative paths. Grep the
   staged diff for `/home/` before committing.
5. **Never edit `asm/`.** splat regenerates it. Rename symbols in
   `config/symbols.slps01762.pepsiman.txt` (through `tools/rename.py`) and
   change segmentation in `config/splat.slps01762.pepsiman.yaml`. A hook
   blocks it.
6. **The toolchain is pinned** (GCC 2.8.1, binutils, flags, maspsx). A
   suspected toolchain problem is reported to the operator with a minimal
   reproducer, never experimented on. `tools/cctest.py` scores a function
   across compilers and flags; use it to build the evidence, not to change
   the pin.
7. **No register pinning.** `register T v asm("$N")` and extended-asm operand
   constraints are banned as a way to fix which register holds a value. A
   bare `__asm__("")` barrier (order only) is allowed. The one exception is
   an instruction with no C spelling and no `gte_*` macro in `include/gte.h`
   (COP2 `lwc2`/`swc2`), and those constraints live inside `include/gte.h`.

## The loop

1. Edit `src/` or `include/`. Keep functions within a file in ROM-address
   order. C89 only (`/* */` comments, declarations at block top). `char` is
   unsigned, so a signed byte is `s8`.
2. Verify, chained so a failed build can't hand you a score:

   ```sh
   ./build-and-verify.sh > /tmp/b.log 2>&1; echo "build exit=$?"; \
   grep -nE 'error:|parse error|undefined reference|\*\*\* \[[^]]*\.o\]' /tmp/b.log | head -8; \
   .venv/bin/python3 tools/funcdiff.py <func>
   ```

   Exit 2 is both "did not compile" and "compiled, does not match". Only the
   grep tells them apart: any hit means the C didn't build, and any score is
   from the previous build. Psy-Q GCC prints semantic errors without an
   `error:` prefix, which is why the `*** [….o]` pattern is there.
3. `tools/lint.sh` before committing. Renames go through `tools/rename.py`,
   `tools/renametype.py` and `tools/unitfile.py`, never by hand.
4. Code that doesn't match never stays live. A readable near-miss goes in
   `#ifdef NON_MATCHING` with the `INCLUDE_ASM` in its `#else`
   (`tools/check-nonmatching.sh`). An odd spelling kept for matching gets one
   `MATCHING:` comment in the `.c`.

Parallel agent rounds (worktrees, runners, the head's merge loop) follow
`docs/lsd-reference/PARALLEL-RUNS.md`, set up with
`tools/setup-worktree.sh` and torn down with `tools/teardown-worktree.sh`.

## How a score lies

- **Stale build:** a failed compile or link leaves the old image, and its
  score is plausible. `funcdiff.py` warns on mtimes.
- **Still `INCLUDE_ASM`:** compares retail with retail and reports a full
  match. Check the function is really C.
- **Address drift:** C of a different length shifts everything after it.
  `funcdiff.py` reports out-of-range differing bytes. Also check that no
  sibling function in the unit lost its wrapper.
- **Half-finished merge:** a unit with conflict markers may not rebuild, so
  the build can read green. Check `git rev-parse -q --verify MERGE_HEAD`
  first.
- **Skipped unit:** after a *header* edit breaks a unit once, the pipeline
  leaves a stale `.o` that make then skips. A red build with no compile error
  means `rm -f build/src/<dir>/<unit>.c.o` and rebuild.

To localise a red build with no compile error, run
`cmp -l build/SLPS_017.62 disk/SLPS_017.62 | head`. The offsets are 1-based,
`vram = (N - 1) - 0x800 + 0x80010000`, and `grep` that address in
`build/pepsiman.map`. The usual causes are a struct edit that moved an
offset (any struct edit is non-local) or a rodata string written as a
literal (declare the existing `extern const char D_…[]` instead).

## Facts worth knowing

- Plain PS-X EXE, loaded at `0x80010000`; `file offset = vram - 0x80010000 +
  0x800`. Entry `0x80042C58`. `$gp` is `0x800954C4`; `.sdata` starts there,
  `.sbss` at `0x800956D0`, and `.bss` runs to `0x800FAE80`. Little-endian
  R3000, no FPU.
- Flags: `-mips1 -mcpu=3000 -O2 -G8 -funsigned-char -fno-builtin
  -mno-abicalls`, then maspsx with the flags in the Makefile's
  `MASPSX_FLAGS`. Anything that compiles in isolation reads the flags from
  the Makefile and never retypes them.
- The compiler is GCC 2.8.1, not lsddecomp's 2.6.3. `docs/LEARNINGS.md`
  holds the source shapes proven on it; lessons in `docs/lsd-reference/`
  about 2.6.3's codegen (most of `DECOMPILATION_LEARNINGS.md`) are
  hypotheses here, not rules.
- **A global's declaration decides how it is reached; read retail's access
  to choose it.** A small complete object (scalar, pointer, struct of at
  most 8 bytes) stays a symbolic load that maspsx makes `%gp_rel` if the
  name is in the unit's `config/gp/<unit>.txt` (what retail reaches through
  `$gp` from that unit; `tools/gpsyms.py`) and `lui $at` if not. An array of
  unknown size or a larger object is split by cc1 into `lui <reg>` and
  `%lo(sym)(<reg>)`. `include/common.h` declares the globals every unit
  reaches the same way; one that units reach differently is declared in
  each unit with a `MATCHING:` note (`tools/declcheck.py` accepts that).
- Sony's SDK is linked from Sony's objects (`config/psyq-objects.txt`). Never
  write C for a function a Sony object owns; `tools/sdkstalls.py` and
  `tools/psyq_sdk.py coverage` say which those are. Pepsiman's libraries
  come from a 4.x Runtime Library: the 3.x discs place none of libgpu,
  libetc, libc2 or libcd.
- A prototype for a function another unit defines belongs in that unit's
  header or in your `.c`, not in a shared header of your own. Put in a shared
  header only what a sibling would use unchanged.
- Some tools were written for lsddecomp's class framework
  (`tools/classtable.py`, parts of `tools/readability.py` and
  `tools/typeviews.py`) and only apply if Pepsiman turns out to use one.
