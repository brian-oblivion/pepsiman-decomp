void CycleGoalLights(s8 arg) {
    u16 periods[8] = {20, 40, 30, 40, 20, 30, 20, 35};
    s16 i;

    if (gSectionEdges[-1] - gSection < 20) {
        for (i = 0; i < 8; i++) {
            if (gFrameCount % periods[i] == 0) {
                LoadTimRange((u8 *)0x8018D000, i * 4 + arg * 2, 1);
            } else if (gFrameCount % periods[i] == periods[i] >> 1) {
                LoadTimRange((u8 *)0x8018D000, i * 4 + 1 + arg * 2, 1);
            }
        }
    }
}
