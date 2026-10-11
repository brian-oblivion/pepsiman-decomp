# SetLetterbox — MATCHED 149/149 (round 9, bravo)

> Renamed from `func_80039754` on 2026-10-10 (tools/rename.py). Address 0x80039754.

Unit `src/stage.c`. Called from main with a clip flag. When set, it
takes the current drawing environment, clips it to the 319x175 band 32 lines
down the current buffer, and queues a draw-mode packet and that environment
at the back (entry 0xFFF) of the second ordering table; then clips to the
full 319x239 buffer and queues the second draw-mode packet and environment
at the front of the first ordering table.

```c
extern DR_STP D_800DFA90;
extern DR_ENV D_800DFA10;
extern DR_STP D_800DFAA0;
extern DR_ENV D_800DFA50;

void SetLetterbox(s32 clip) {
    DRAWENV env;
    /* MATCHING: retail's frame has 8 more bytes above the environment. */
    s32 unused[2];

    if (clip) {
        GetDrawEnv(&env);
        env.clip.x = 0;
        env.clip.y = D_800E474C * 240 + 32;
        env.clip.w = 319;
        env.clip.h = 175;
        SetDrawStp(&D_800DFA90, 1);
        addPrim(gOt[gActiveBuff].org + 0xFFF, &D_800DFA90);
        SetDrawEnv(&D_800DFA10, &env);
        addPrim(gOt[gActiveBuff].org + 0xFFF, &D_800DFA10);
        env.clip.x = 0;
        env.clip.y = D_800E474C * 240;
        env.clip.w = 319;
        env.clip.h = 239;
        SetDrawStp(&D_800DFAA0, 1);
        addPrim(gSubOt[gActiveBuff].org, &D_800DFAA0);
        SetDrawEnv(&D_800DFA50, &env);
        addPrim(gSubOt[gActiveBuff].org, &D_800DFA50);
    }
}
```

## Notes

- First build differed only in the frame: retail's is 8 bytes larger
  (registers saved from 0x78, not 0x70), closed by the existing
  `s32 unused[2];` lever. Its `-Wall` warning is baselined in
  `config/typeviews-warnings.txt` (a second stage line).
- The four packets are complete objects of libgpu's types (`DR_STP`,
  `DR_ENV`); retail reaches each through one `lui` reused for the address
  and the tag access.
- The prototype main.c declared locally moved to `include/common.h`
  (declcheck LOCAL once the function became C), with the same signature.
