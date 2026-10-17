void UpdateStageEventHook(void) {
    StageTrap *m;
    SwitchBox *s;

    m = &gEventTraps[1];
    s = gSwitches;
    if (s[192].state == SWITCH_ON) {
        m->state = 0;
        gPlayer.motion[0] = MOT_JUMP_LOW;
        gPlayer.jumpPower = 40;
        gPlayer.action = ACT_RUN;
        s[192].state = SWITCH_OFF;
        gPlayer.dropRide = 1;
        m->timer = 0;
        m->state = 1;
        gMissState = 0;
        gPlayer.hittable = 0;
        gPlayer.invulnTimer = 30;
        gPlayer.rideMode &= 0xF0;
        PlaySe(0x38);
    }
}
