void RunTrapType0B(StageTrap *t, s8 dir) {
    s16 v[2] = {60, 90};
    s16 n;

    n = v[t->act / 5];
    RunTrapThenTurnTimed(t, t->act * 10 + 60, n, dir);
}
