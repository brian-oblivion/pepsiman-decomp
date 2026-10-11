# CloseCardEvents — MATCHED 34/34 (round 5, echo)

> Renamed from `func_800386A8` on 2026-10-10 (tools/rename.py). Address 0x800386a8.

Unit `src/card.c`. Inside a critical section, closes the eight
memory-card events whose handles `EnableCardEvents` enables (`gCardSwIoe` ..
`gCardHwNew`, all `%gp_rel`). Called from `UpdateToolCard` (hit_tool) and
`UpdateEditorData` (object).

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
