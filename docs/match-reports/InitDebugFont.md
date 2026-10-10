# InitDebugFont — MATCHED (17/17), round 1, runner charlie

> Renamed from `func_80014D6C` on 2026-10-10 (tools/rename.py). Address 0x80014d6c.

Loads the debug font into VRAM at (960, 0) and opens a 320x256 print stream
of up to 1024 characters, with no background.

```c
void InitDebugFont(void) {
    FntLoad(0x3C0, 0);
    FntOpen(-0x9A, -0x74, 0x140, 0x100, 0, 0x400);
}
```

First attempt, with Sony's prototypes from `libgpu.h`. The return value of
`FntOpen` is discarded.
