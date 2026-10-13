void DrawTrapWhileLinked(StageTrap *t) {
    Query30 q;
    StageTrap *k;

    k = &((StageTrap *)gTraps)[195];
    if (k->state >= 0 && (k->state < 4 || (k->state == 4 && k->timer < 4))) {
        if (gSection - UpdatePathSegment(t) < 2) {
            DrawObjectModel(&gObjectModels[t->model], t);
        }
    }
}
