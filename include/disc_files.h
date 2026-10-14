#ifndef DISC_FILES_H
#define DISC_FILES_H

/**
 * @file disc_files.h
 * @brief The disc's file table: where each file the game reads by position
 *        starts (a CdlLOC) and how many sectors it has.
 *
 * Every unit that has the table holds all of it as initialised data, most
 * of them twice: the executable's main and every overlay begin their data
 * with two copies, and ovl02 has a third. disc_file_tables.inc defines one
 * copy where it is included (see there); a unit reads the entries it loads
 * from its own copy.
 *
 * Each set of files is two arrays, the files' positions and then their
 * sizes, and the code reaches the two through separate symbols: a
 * position and a size of one entry never share a base register.
 */

#include "common.h"
#include "libcd.h"

/** @brief A file's position on the disc, minute, second and sector, each
 *         written as its two BCD digits; the file is on track 1. An unused
 *         slot is 00:00:00 with 0 sectors. */
#define DISC_LOC(m, s, f) {0x##m, 0x##s, 0x##f, 1}

/** @brief The name of one of a copy's arrays: the copy's prefix, then
 *         @p name (`DISC_FILE(SharedLoc)` is `sDiscSharedLoc` in the copy
 *         whose prefix is `sDisc`). */
#define DISC_FILE(name) DISC_FILE_JOIN(DISC_FILE_PREFIX, name)
/** @brief Joins @p a and @p b after expanding them (for DISC_FILE). */
#define DISC_FILE_JOIN(a, b) DISC_FILE_JOIN2(a, b)
/** @brief Joins @p a and @p b as written (for DISC_FILE_JOIN). */
#define DISC_FILE_JOIN2(a, b) a##b

/** @brief The shared set's entry holding world @p n's intro pack (1 to 5). */
#define DISC_INTRO_PACK(n) (9 + (n) * 2)

#endif
