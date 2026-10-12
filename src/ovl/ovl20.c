/* The world 3 intro, an overlay main loads for GM_WORLD_INTRO and runs each
 * frame. It loads the intro's models, animations and sprites, then plays a
 * scripted scene of the player and a partner model under fifteen lines of
 * rising text until the script ends or START skips it, draws one black
 * frame and starts stage 3-1. Unlike the other world intros it plays no XA
 * track and shows no captions. */

#define INTRO_CAPTIONS 0
#define INTRO_SORT_MODES 0
/* The text's sprites: fifteen lines of 16 on texture page 5. */
#define DEFINE_INTRO_SPRITES(i)                                                   \
    for (i = 0; i < 15; i++) {                                                    \
        DefineSprite(SPR_STAGE + i, 0, 0xFF, 0x10, 5, 0, (u8)(i * 16), 0, 0x1FD); \
    }

#include "world_intro.inc"

/* MATCHING: all s32 where model has s16: the view's words are passed
 * and stored whole. */
s32 Lerp(s32 a, s32 b, u16 t, u16 n);

/* Draws the scrolling text: fifteen lines that rise with @p time, fading
 * in near the bottom of the screen and out near the top. */
void DrawRisingText(s32 time) {
    SVECTOR pos;
    SColor color;
    s32 i;

    time /= 2;
    for (i = 0; i < 15; i++) {
        pos.vx = -0x80;
        pos.vy = i * 16 + 0x68 - time;
        color.r = BLEND_ADD;
        if (pos.vy > 0x68) {
            color.g = color.b = color.cd = 0;
        } else if (pos.vy > 0x58) {
            color.g = color.b = color.cd = (0x68 - pos.vy) * 8;
        } else if (pos.vy > 0x38) {
            color.g = color.b = color.cd = 0x80;
        } else if (pos.vy > 0x28) {
            color.g = color.b = color.cd = (pos.vy - 0x28) * 8;
        } else {
            color.g = color.b = color.cd = 0;
        }
        DrawSprite(SPR_STAGE + i, &pos, (CVECTOR *)&color, 0, &gSubOt[gActiveBuff]);
    }
}

/* MATCHING: non-void with no return keeps step 2's delay slot a nop. */
s32 StepWorldIntro(void) {
    SVECTOR pos;
    POLY_F4 *p;

    switch (gWorldIntroStep) {
        case 0:
            gCdLoadLoc[0] = (s32)sIntroPackLoc;
            gCdLoadDest[0] = 0x8019D000;
            gCdLoadCount = 1;
            gCdLoadState = CD_LOAD_SEEK;
            gWorldIntroStep = 1;
            gCdLoadSectors[0] = sIntroPackSectors[0];
            break;
        case 1:
            if (gCdLoadState == CD_LOAD_DONE) {
                LoadIntroPack((s32 *)0x8019D000);
                gVabLoadStep = 0;
                gWorldIntroStep = 2;
            }
            break;
        case 2:
            if (LoadVabBank(4) == 1) {
                gLoading = 0;
                gWorldIntroStep = 3;
            }
            break;
        case 3:
            sPartner = gPlayer;
            sPartner.motion[0] = 0x38;
            sPartner.curMotion[0] = MOT_OFF;
            sPartner.motion[1] = 0x38;
            sPartner.curMotion[1] = MOT_OFF;
            StepIntroMotion(&sPartner, 0, (s32)sPartnerObjs, sPartnerTmd);
            sLights.l[0].r = 0xFF;
            sLights.l[0].g = 0xFF;
            sLights.l[0].b = 0xFF;
            sLights.l[1].r = 0xFF;
            sLights.l[1].g = 0xFF;
            sLights.l[1].b = 0xFF;
            sLights.l[2].r = 0xFF;
            sLights.l[2].g = 0xFF;
            sLights.l[2].b = 0xFF;
            sLights.l[0].vx = 0;
            sLights.l[0].vy = 100;
            sLights.l[0].vz = 100;
            GsSetFlatLight(0, &sLights.l[0]);
            sLights.l[1].vx = 0x56;
            sLights.l[1].vy = 100;
            sLights.l[1].vz = -50;
            GsSetFlatLight(1, &sLights.l[1]);
            sLights.l[2].vx = -0x56;
            sLights.l[2].vy = 100;
            sLights.l[2].vz = -50;
            GsSetFlatLight(2, &sLights.l[2]);
            gPlayer.z = -0x1644;
            gPlayer.yaw = 0x800;
            gPlayer.motion[0] = 0x33;
            gPlayer.x = 0;
            gPlayer.y = 0;
            gPlayer.action = ACT_MODEL_LOOP;
            sPartner.x = -100;
            sPartner.yaw = -0x200;
            sPartner.motion[0] = 0x39;
            sPartner.y = 0;
            sPartner.z = 100;
            sPartner.action = ACT_MODEL_LOOP;
            sPropRot.vx = 0;
            sPropRot.vy = 0;
            sPropRot.vz = 0;
            sPropPos.vx = -250;
            sPropPos.vy = 0;
            sPropPos.vz = 250;
            sView.vpx = 0;
            sView.vpy = -200;
            sView.vpz = -0x17D4;
            sView.vrx = 0;
            sView.vry = 0;
            sView.vrz = -0x1450;
            D_800D39D8.timer = 0;
            sSubStep = 0;
            gWorldIntroStep = 10;
            break;
        case 10:
            DrawFrozenFrame();
            switch (sSubStep) {
                case 0:
                    sView.vpx = 0;
                    sView.vpy = -100;
                    sView.vry = -100;
                    sView.vrx = 0;
                    gPlayer.z += 30;
                    sView.vpz = gPlayer.z + 200 + D_800D39D8.timer * 2;
                    sView.vrz = gPlayer.z;
                    GsSetRefView2(&sView);
                    if (++D_800D39D8.timer >= 50) {
                        D_800D39D8.timer = 0;
                        sSubStep = 3;
                    }
                    break;
                case 3:
                    gPlayer.z += 30;
                    if (D_800D39D8.timer < 90) {
                        sView.vpx = rsin((-(D_800D39D8.timer * 2) + 180) * 4096 / 360) * 300 / 4096;
                        sView.vpy = -100;
                        sView.vpz = gPlayer.z -
                                    rcos((-(D_800D39D8.timer * 2) + 180) * 4096 / 360) * 300 / 4096;
                        sView.vrx = 0;
                        sView.vry = -100;
                        sView.vrz = gPlayer.z;
                    } else {
                        sView.vpx = rsin(0) * 300 / 4096;
                        sView.vpy = -100;
                        sView.vpz = gPlayer.z - rcos(0) * 300 / 4096;
                        sView.vrx = 0;
                        sView.vry = -100;
                        sView.vrz = gPlayer.z;
                    }
                    D_800D39D8.timer++;
                    if (gPlayer.z >= 0) {
                        gView[0] = -250;
                        gView[1] = -200;
                        gView[2] = -150;
                        gView[3] = -50;
                        gView[4] = -100;
                        gView[5] = 50;
                        gPlayer.motion[0] = 0x34;
                        gPlayer.action = ACT_MODEL_ONCE;
                        gPlayer.yaw = 0x600;
                        sPartner.motion[0] = 0x3A;
                        sPartner.action = ACT_MODEL_ONCE;
                        D_800D39D8.timer = 0;
                        sSubStep = 5;
                    }
                    break;
                case 5:
                    sView.vpx = Lerp(sView.vpx, gView[0], D_800D39D8.timer, 15);
                    sView.vpy = Lerp(sView.vpy, gView[1], D_800D39D8.timer, 15);
                    sView.vpz = Lerp(sView.vpz, gView[2], D_800D39D8.timer, 15);
                    sView.vrx = Lerp(sView.vrx, gView[3], D_800D39D8.timer, 15);
                    sView.vry = Lerp(sView.vry, gView[4], D_800D39D8.timer, 15);
                    sView.vrz = Lerp(sView.vrz, gView[5], D_800D39D8.timer, 15);
                    if (++D_800D39D8.timer == 15) {
                        gPlayer.motion[0] = 0x35;
                        gPlayer.action = ACT_MODEL_ONCE;
                        D_800D39D8.timer = 0;
                        sSubStep = 10;
                    }
                    break;
                case 10:
                    if (sPartner.motionResult[0] == 0) {
                        sPartner.motion[0] = rand() % 2 + 0x3A;
                        sPartner.curMotion[0] = MOT_OFF;
                        sPartner.action = 0;
                    }
                    DrawRisingText(D_800D39D8.timer);
                    if (++D_800D39D8.timer == 0x260) {
                        D_800D39D8.timer = 0;
                        sSubStep = 15;
                    }
                    break;
                case 15:
                    if (gPlayer.motionResult[0] == 0) {
                        gPlayer.motion[0] = 0x36;
                        gPlayer.curMotion[0] = MOT_OFF;
                        gPlayer.action = 0;
                        sSubStep = 20;
                    }
                    break;
                case 20:
                    if (gPlayer.motionResult[0] == 0) {
                        gPlayer.motion[0] = 0x37;
                        gPlayer.curMotion[0] = MOT_OFF;
                        gPlayer.action = 0;
                        sPartner.motion[0] = 0x3C;
                        sPartner.curMotion[0] = MOT_OFF;
                        sPartner.action = 0;
                        D_800D39D8.timer = 0;
                        sSubStep = 25;
                    }
                    break;
                case 25:
                    if (gPlayer.motionResult[0] == 0) {
                        gPlayer.yaw = 0x800;
                        gPlayer.motion[0] = 0x33;
                        gPlayer.curMotion[0] = MOT_OFF;
                        gPlayer.action = 1;
                    }
                    if (sPartner.motionResult[0] == 0) {
                        sPartner.motion[0] = 0x3D;
                        sPartner.curMotion[0] = MOT_OFF;
                        sPartner.action = 1;
                    }
                    if (gPlayer.action == 1) {
                        gPlayer.z += 30;
                        if (++D_800D39D8.timer == 90) {
                            gWorldIntroStep = 100;
                        }
                    }
                    break;
            }
            if (gPadTrig & PADstart) {
                gWorldIntroStep = 100;
            }
            DrawIntroPlayer();
            DrawIntroPartner();
            DrawIntroProp();
            break;
        case 100:
            pos.vx = -0x80;
            pos.vy = -50;
            DrawSprite(SPR_STAGE + 30, &pos, NULL, 0, &gSubOt[gActiveBuff]);
            p = (POLY_F4 *)GsOUT_PACKET_P;
            setPolyF4(p);
            p->r0 = p->g0 = p->b0 = 0;
            setXY4(p, -160, -120, 160, -120, -160, 120, 160, 120);
            addPrim(gSubOt[gActiveBuff].org, p);
            gWorldIntroStep++;
            p++;
            GsOUT_PACKET_P = (u8 *)p;
            break;
        case 101:
            gStage = STAGE_3_1;
            BeginLoadScreenModel();
            break;
    }
}
