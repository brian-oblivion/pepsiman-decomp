/* The data the act-3 chases built on chase_head.inc (1-3, 2-3, 4-3 and
 * 5-3) share in shape, which follows the stage's object list in its main
 * unit. That unit includes this at its end, after defining sPlacements,
 * with CHASE_PIECE_STEPS (the step of each object kind, in kind order) and
 * CHASE_SKY (the sky's four colours) defined.
 *
 * The zeroed state at the end has no initializer: it is the unit's .bss,
 * which the overlay's file holds after both units' .data. */

/** @brief How many objects the frame step walks: the whole object list. */
s32 sObjCount = sizeof(sPlacements) / sizeof(sPlacements[0]);

/** @brief The jump's vertical speed; the player starts on the ground. */
s32 sJumpSpeed = 5;

/** @brief The player's depth scale (0x1000 is 1:1); 0 once caught. */
s32 sChaseLive = 0x1000;

/** @brief Each object kind's step, which moves and draws one object. */
Act3Step sPieceSteps[] = {CHASE_PIECE_STEPS};

/** @brief The sky's colours: the upper band's top and bottom, then the
 * lower band's (model.c's SkyColors). */
CVECTOR sChaseSky[4] = {CHASE_SKY};

/* The zeroed state: tentative definitions, the unit's .bss in source order. */
s32 sHurtTimer;
s32 sChaseUnused[3]; /**< never read or written */
MATRIX sChaseMatrix; /**< the stage models' local-screen and light matrix */
s32 sCaughtTimer;    /**< frames since the can caught the player, 0 before */
s32 sEscapeTimer;    /**< frames since the run's end; the scenes' scratch */
s32 sSceneTimer;     /**< frames into the current scene's step */
POLY_F4 *sFadePoly;  /**< the screen fade's polygon, in the packet area */
DR_MODE *sFadeMode;  /**< the fade's draw mode, after its polygon */
