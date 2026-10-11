# MoveGamePos — MATCHED 44/44 (round 6, bravo)

> Renamed from `func_80033A08` on 2026-10-11 (tools/rename.py). Address 0x80033a08.

Unit `src/editor.c`. Converts `deg` (degrees) to a 4096-unit angle
(`(deg << 12) / 360`, the `0xB60B60B7` magic divide), adds the yaw
`gCourse[0].vy`, and moves the position words at 0x348 and 0x350 of
`gPlayer` back by `rsin(...) * dist >> 12` and `rcos(...) * dist >> 12`.

```c
void MoveGamePos(s32 deg, s32 dist) {
    s32 ang;
    SVECTOR *rot;

    rot = gCourse;
    ang = (deg << 12) / 360;
    sGameSave.unk348[0] -= rsin(rot->vy + ang) * dist >> 12;
    sGameSave.unk348[2] -= rcos(rot->vy + ang) * dist >> 12;
}
```

## Levers

- `deg` is `s32`: retail shifts it with no `sll 16`/`sra 16` first.
- **The angle in the register the position base reuses.** Retail keeps
  the angle in `s0` and the `gCourse` base in `s1`, and reuses `s0` for
  the `gPlayer` base once the second `addu` is done. With
  `gCourse[0].vy` read directly and an `ang` local (12/44, one word
  short): the base took `s0`, the angle `s1`, and the second `addu` sank
  below the `gPlayer` base load. Two spellings give 44/44: the base in
  a local pointer (`rot = gCourse;`, kept) or the angle written back
  into the parameter (`deg = (deg << 12) / 360;`).

## Declarations

`extern SVECTOR gCourse[];` moved from player.c (not a round unit)
into `include/player.h`, which editor.c now includes; declcheck
needs one site once two units use it. editor also uses it in
SaveEditorState/RestoreEditorState (`gCourse[0].vy`, which replaced the
`D_800A7682` line those commits had added to `include/common.h`).
