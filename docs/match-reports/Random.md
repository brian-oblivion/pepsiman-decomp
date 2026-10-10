# Random — MATCHED (8/8), round 3, runner charlie

> Renamed from `func_80017B18` on 2026-10-10 (tools/rename.py). Address 0x80017b18.

Unit `src/code_7d74.c`. A wrapper that returns Sony's `rand()`.

```c
s32 Random(void) {
    return rand();
}
```

## Attempts (1)

1. As above, `rand` prototyped by Psy-Q's `rand.h`: 8/8 first build. A
   `void` body with a bare `rand();` would give the same bytes; the `s32`
   return is the reading that keeps the value.
