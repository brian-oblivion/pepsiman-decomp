#ifndef RAM_H
#define RAM_H

/**
 * @file ram.h
 * @brief The game's fixed buffers: main RAM from just past `.bss` up to the
 *        stack, which the game divides by address, not by allocation.
 *
 * Most regions are shared over time: what a region holds depends on the
 * mode, and each comment names who uses it. The values are bare integers so
 * that each use keeps its own cast.
 */

/** The first buffer, past the end of `.bss`. */
#define RAM_BUFFERS 0x80101000
/** The buffers' total size, which InitSystem clears: up to the stack. */
#define RAM_BUFFERS_SIZE 0xFC800

/** The sound pack, the SEQs and the first VAB that LoadSoundData loads:
 *  the SEQs are opened in place and stay there. */
#define RAM_SOUND_PACK RAM_BUFFERS
/** The common animations, a directory of TOD motions loaded at boot. */
#define RAM_COMMON_ANIMS 0x80123000
/** The common models, the TMD pack loaded at boot: gAltModel and the
 *  mode objects. */
#define RAM_COMMON_MODELS 0x8014D000
/** The stage's object models, the TMD file InitEntryModels sets up. */
#define RAM_OBJECT_MODELS 0x80155000

/** A 64 KB work buffer, the most shared: at boot the TIM list; VAB bodies
 *  on their way to the sound processor; the stage's sprite pack; the
 *  stage's hit and object file, which runs on into RAM_WORK_B; the card's
 *  save buffer; the editor's slots and the hit tool's point bank. */
#define RAM_WORK_A 0x8016D000
/** A 64 KB work buffer: the hit tool's quad bank and the editor's copy of
 *  RAM_WORK_A. */
#define RAM_WORK_B 0x8017D000
/** A 64 KB work buffer: the boot TIM pack, a stage's late file, the card's
 *  save data and the hit tool's three slots. */
#define RAM_WORK_C 0x8018D000
/** The stage load area: each stage's own data, TIM lists and TIM pack. */
#define RAM_STAGE_LOAD 0x8019D000
/** The size of one work buffer. */
#define RAM_WORK_SIZE 0x10000

/** The current block's 2 KB header (SetCurrentBlock): RestoreStageObjects
 *  copies the stage's saved one here, and the editor builds an empty one. */
#define RAM_CUR_BLOCK 0x801FD000

/* The movie decoder's buffers take over the work buffers while a movie
 * plays (InitMovieDecoder, InitMovieStream). */
/** The movie decoder's first VLC buffer. */
#define RAM_MOVIE_VLC0 0x8015D000
/** The movie decoder's second VLC buffer. */
#define RAM_MOVIE_VLC1 0x80171000
/** The ring buffer of the movie's CD stream. */
#define RAM_MOVIE_RING 0x80185000
/** The decoded image's buffer. */
#define RAM_MOVIE_IMAGE 0x80195000

#endif
