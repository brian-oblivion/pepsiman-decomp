# lsddecomp's method, for reuse

These documents come from [lsddecomp](https://github.com/brian-oblivion/lsddecomp),
a finished matching decompilation of *LSD: Dream Emulator* built with the same
toolchain and tools this repository uses. They were written for that game, so
their examples, function names, round numbers and addresses are LSD's. The
method carries over unchanged.

| file | what it is |
| --- | --- |
| `MATCHING-GUIDE.md` | how to take one function from `INCLUDE_ASM` to byte-exact C: gates, levers, when to stop |
| `PARALLEL-RUNS.md` | the protocol for running many agent sessions at once: head and runners, worktrees, collision rules, the merge loop |
| `SDK-OBJECTS-GUIDE.md` | how Sony's library code was identified in the image and linked from Sony's own objects |
| `DECOMPILATION_LEARNINGS.md` | GCC 2.6.3 / maspsx code-generation lessons, condensed |
| `research/` | toolchain findings behind the maspsx patches and flags (`addiu-at`, `gp-relative`, load-delay nops), GTE macros, the class-framework write-up |

Where a guide names an lsddecomp tool that is not in `tools/` here (`plan.py`
drove LSD's cleanup tracks), the step it describes is LSD-specific.
