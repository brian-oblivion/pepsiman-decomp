void ScatterTrapDust(StageTrap *t, u8 side) {
    VECTOR v;
    u16 i;

    switch (side) {
        case 2:
            v.vx = t->x + 280;
            v.vz = t->z - 200;
            v.vy = gFloorY[0];
            ScatterDust(&v);
            break;
        case 1:
            v.vx = t->x - 280;
            v.vz = t->z + 200;
            v.vy = gFloorY[0];
            ScatterDust(&v);
            break;
        case 3:
            for (i = 0; i < 10; i++) {
                v.vx = t->x + 300;
                v.vx -= i * 50;
                v.vz = t->z - 200;
                v.vz += i * 50;
                v.vy = gFloorY[0];
                ScatterDust(&v);
            }
            break;
    }
}
