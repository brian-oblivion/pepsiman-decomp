void RunTrapType04(StageTrap *t) {
    Speeds sp = sChargeRanges;
    s16 n;

    n = sp.v[t->act / 4];
    StepChargingTrap(t, t->act * 10 + 40, n);
}
