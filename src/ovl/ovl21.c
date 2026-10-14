/* The world 4 intro, an overlay main loads for GM_WORLD_INTRO and runs each
 * frame. It loads the intro's models, animations and sprites, starts its XA
 * track, then plays a scripted scene of the player and a partner model with
 * captions until the script ends or START skips it, and starts stage 4-1. */

#define INTRO_CAPTIONS 8
#define INTRO_CAPTION_TIMES 0xED, 0x10B, 0x17A, 0x1CB, 0x222, 0x282, 0x2FA, 0x348, 0x3EA
#define INTRO_SORT_MODES 1
/* The captions' sprites: rows of 32 lines on texture page 5. */
#define DEFINE_INTRO_SPRITES(i)                                                   \
    for (i = 0; i < 8; i++) {                                                     \
        DefineSprite(SPR_STAGE + i, 0, 0xFF, 0x20, 5, 0, (u8)(i * 32), 0, 0x1FE); \
    }

#define INTRO_WORLD 4
#define INTRO_XA 1

#include "world_intro.inc"

void StepWorldIntro(void) {
    /* MATCHING: never used; its slot puts the inline functions' frames at
     * sp+0x20. */
    SVECTOR unused;

    switch (gWorldIntroStep) {
        case 0:
            gCdLoadLoc[0] = (s32)&sFiles1OvlLoc[INTRO_PACK];
            gCdLoadDest[0] = RAM_STAGE_LOAD;
            gCdLoadCount = 1;
            gCdLoadState = CD_LOAD_SEEK;
            gWorldIntroStep = 1;
            gCdLoadSectors[0] = sFiles1OvlSectors[INTRO_PACK];
            break;
        case 1:
            if (gCdLoadState == CD_LOAD_DONE) {
                LoadIntroPack((s32 *)RAM_STAGE_LOAD);
                gVabLoadStep = 0;
                gWorldIntroStep = 2;
            }
            break;
        case 2:
            if (LoadVabBank(5) == 1) {
                gLoading = 0;
                CdControlF(CdlSeekL, (u_char *)&sFiles1LastLoc[0]);
                gWorldIntroStep = 3;
            }
            break;
        case 3:
            CdSync(0, 0);
            PlayXa(3);
            sIntroFrame = 0;
            gWorldIntroStep = 4;
            break;
        case 4:
            sPartner = gPlayer;
            sPartner.motion[0] = 0x38;
            sPartner.curMotion[0] = MOT_OFF;
            sPartner.motion[1] = 0x38;
            sPartner.curMotion[1] = MOT_OFF;
            StepIntroMotion(&sPartner, 0, (s32)sPartnerObjTable, sPartnerTmd);
            sLights.l[0].r = 0x80;
            sLights.l[0].g = 0x80;
            sLights.l[0].b = 0x80;
            sLights.l[1].r = 0x80;
            sLights.l[1].g = 0x80;
            sLights.l[1].b = 0x80;
            sLights.l[2].r = 0x80;
            sLights.l[2].g = 0x80;
            sLights.l[2].b = 0x80;
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
            gPlayer.curMotion[0] = MOT_OFF;
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
            sPropPos.vx = -500;
            sPropPos.vy = 0;
            sPropPos.vz = 1000;
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
            DrawCaptions(sIntroFrame);
            if (sScriptPaused == 0) {
                switch (sSubStep) {
                    case 0:
                        gPlayer.z += 30;
                        sView.vpx = rsin(0x800) * (D_800D39D8.timer * 2 + 200) / 4096 + gPlayer.x;
                        sView.vpy = -100 - (D_800D39D8.timer >> 1);
                        sView.vpz = rcos(0x800) * (D_800D39D8.timer * 2 + 200) / 4096 + gPlayer.z;
                        sView.vrx = gPlayer.x;
                        /* MATCHING: `/ 2` where vpy has `>> 1`; the timer is then reloaded
                         * for the increment. */
                        sView.vry = -100 - (D_800D39D8.timer / 2);
                        sView.vrz = gPlayer.z;
                        if (++D_800D39D8.timer >= 100) {
                            D_800D39D8.timer = 0;
                            sSubStep = 3;
                        }
                        break;
                    case 3:
                        gPlayer.z += 30;
                        sView.vpx = rsin(0x800) * 400 / 4096 + gPlayer.x;
                        sView.vpy = -150;
                        sView.vpz = rcos(0x800) * 400 / 4096 + gPlayer.z;
                        sView.vrx = gPlayer.x;
                        sView.vry = -150;
                        sView.vrz = gPlayer.z;
                        D_800D39D8.timer++;
                        if (gPlayer.z >= -0xE1) {
                            gPlayer.motion[0] = 0x34;
                            gPlayer.action = ACT_MODEL_ONCE;
                            sPartner.motion[0] = 0x3A;
                            sPartner.action = ACT_MODEL_ONCE;
                            D_800D39D8.timer = 30;
                            sSubStep = 5;
                        }
                        break;
                    case 5:
                        gPlayer.z += D_800D39D8.timer;
                        gPlayer.yaw -= 0x22;
                        sView.vpx = rsin(0x800) * 400 / 4096 + gPlayer.x;
                        sView.vpy = -150;
                        sView.vpz = rcos(0x800) * 400 / 4096 + gPlayer.z;
                        sView.vrx = gPlayer.x;
                        sView.vry = -150;
                        sView.vrz = gPlayer.z;
                        if ((D_800D39D8.timer -= 2) == 0) {
                            gPlayer.yaw = 0x600;
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
                        if (sIntroFrame == 378) {
                            sView.vpx = 270;
                            sView.vpy = -200;
                            sView.vpz = 180;
                            sView.vrx = -60;
                            sView.vry = -80;
                            sView.vrz = 40;
                        }
                        if (sIntroFrame == 1002) {
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
                                StopXa();
                                gStage = STAGE_4_1;
                                BeginLoadScreenModel();
                            }
                        }
                        sView.vpx = rsin(0x800) * 400 / 4096 + gPlayer.x;
                        sView.vpy = -150;
                        sView.vpz = rcos(0x800) * 400 / 4096 + gPlayer.z;
                        sView.vrx = gPlayer.x;
                        sView.vry = -150;
                        sView.vrz = gPlayer.z;
                        break;
                }
                GsSetRefView2(&sView);
            }
            if (gPadTrig & PADstart) {
                StopXa();
                gStage = STAGE_4_1;
                BeginLoadScreenModel();
            }
            DrawIntroPlayer();
            DrawIntroPartner();
            DrawIntroProp();
            break;
    }
    sIntroFrame++;
}
