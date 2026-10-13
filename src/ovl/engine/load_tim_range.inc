void LoadTimRange(u8 *dir, u16 first, u16 count) {
    u16 i;
    DirEnt16 *e;
    s32 off;

    e = (DirEnt16 *)dir + first;
    for (i = first; i < first + count;) {
        off = e->offset;
        /* MATCHING: the bump wrapped in a block puts e in $s0 and i in $s1;
         * both bumps ahead of the call are scheduled above the jal. */
        do {
            e++;
        } while (0);
        i++;
        LoadTim(dir + off);
        DrawSync(0);
    }
}
