void EmitTrapParticles(StageTrap *t) {
    s32 r;

    if ((gFrameCount & 3) == 0) {
        r = rand() % 10 + 15;
        SpawnEffectWithParams(12, t->x, t->y, t->z, 0, t->rotY + 0x800, r >= 0 ? r : -r, 20, 0);
    }
}
