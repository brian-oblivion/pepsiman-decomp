#ifndef CODE_31CEC_H
#define CODE_31CEC_H

/**
 * @file code_31cec.h
 * @brief What other units call in code_31cec, the movie player, that needs
 *        libcd's types.
 */

#include "common.h"
#include "libcd.h"

/** @brief Hands an index to stage 0's overlay; other stages ignore it.
 *  @param index passed through unchanged */
void func_800414EC(s16 index);

/** @brief Plays a streamed movie to its end, or until START skips it.
 *  @param name      the movie file's name; empty to play from @p loc
 *  @param loc       where the file is on the disc
 *  @param lastFrame the frame number the movie ends at
 *  @param height    the picture's height in lines, centred on 240
 *  @param skippable 1 when START may end the movie early */
void PlayMovie(char *name, CdlLOC *loc, s32 lastFrame, s16 height, s16 skippable);

/** @brief Defined in code_31cec. */
void UpdateSoundFade(void);

#endif
