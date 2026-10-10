# CloseCardEvents — MATCHED 34/34 (round 5, echo)

> Renamed from `func_800386A8` on 2026-10-10 (tools/rename.py). Address 0x800386a8.

Unit `src/code_27bc8.c`. Inside a critical section, closes the eight
memory-card events whose handles `EnableCardEvents` enables (`gCardSwIoe` ..
`gCardHwNew`, all `%gp_rel`). Called from `func_80035970` (code_24748) and
`func_80031EF4` (code_1a098).

```c
void CloseCardEvents(void) {
    EnterCriticalSection();
    CloseEvent(gCardSwIoe);
    CloseEvent(gCardSwError);
    CloseEvent(gCardSwTimeout);
    CloseEvent(gCardSwNew);
    CloseEvent(gCardHwIoe);
    CloseEvent(gCardHwError);
    CloseEvent(gCardHwTimeout);
    CloseEvent(gCardHwNew);
    ExitCriticalSection();
}
```

First build matched. Prototypes from `libapi.h`.
