/* One copy of the disc's file table: where each file is (minute, second and
 * sector, in BCD) and its size in sectors, in pairs of arrays. Every source
 * file that loads from the disc held its own copy (the executable's main
 * one, at sBootFileLoc on; each overlay one or more), so the includer names
 * this one: DISC_FILE(x) is the name of its array x, and is undefined again
 * at the end. */

#ifndef CD_LOC
/* A disc position, minute, second and sector given as their BCD digits:
 * CD_LOC(00, 27, 50) is 00:27:50. Every file is on track 1. */
#define CD_LOC(min, sec, sector) {0x##min, 0x##sec, 0x##sector, 1}
#endif

/* The files the executable loads at boot; an unused slot is 00:00:00, 0. */
CdlLOC DISC_FILE(BootLoc)[16] = {
    CD_LOC(00, 05, 69), CD_LOC(00, 06, 07), CD_LOC(00, 06, 24), CD_LOC(00, 07, 26),
    CD_LOC(00, 08, 02), CD_LOC(00, 08, 39), CD_LOC(00, 08, 46), CD_LOC(00, 11, 14),
    CD_LOC(00, 11, 58), CD_LOC(00, 12, 23), CD_LOC(00, 12, 66), CD_LOC(00, 13, 25),
    CD_LOC(00, 13, 69), CD_LOC(00, 14, 40), CD_LOC(00, 15, 08), CD_LOC(00, 00, 00),
};

s32 DISC_FILE(BootSectors)[16] = {13, 17, 77, 51, 37, 7, 193, 44, 40, 43, 34, 44, 46, 43, 43, 0};

/* LoadOverlay's: overlays 00 and 02 first, then 18 to 22 at [10], [12],
 * [14], [16] and [18]. */
CdlLOC DISC_FILE(OvlLoc)[20] = {
    CD_LOC(00, 15, 52), CD_LOC(00, 15, 56), CD_LOC(00, 16, 02), CD_LOC(00, 18, 26),
    CD_LOC(00, 18, 60), CD_LOC(00, 19, 17), CD_LOC(00, 19, 49), CD_LOC(00, 20, 06),
    CD_LOC(00, 20, 38), CD_LOC(00, 20, 70), CD_LOC(00, 21, 27), CD_LOC(00, 21, 33),
    CD_LOC(00, 22, 39), CD_LOC(00, 22, 45), CD_LOC(00, 23, 62), CD_LOC(00, 23, 68),
    CD_LOC(00, 24, 74), CD_LOC(00, 25, 05), CD_LOC(00, 26, 29), CD_LOC(00, 26, 35),
};

s32 DISC_FILE(OvlSectors)[20] = {4, 21, 174, 34, 32, 32, 32, 32, 32, 32,
                                 6, 81, 6,   92, 6,  81, 6,  99, 6,  89};

/* Overlay 03, then the files its loader reads. */
CdlLOC DISC_FILE(Ovl03Loc)[9] = {
    CD_LOC(00, 27, 50), CD_LOC(00, 28, 07), CD_LOC(00, 29, 36),
    CD_LOC(00, 30, 45), CD_LOC(00, 33, 10), CD_LOC(00, 33, 42),
    CD_LOC(00, 34, 00), CD_LOC(00, 34, 34), CD_LOC(00, 00, 00),
};

s32 DISC_FILE(Ovl03Sectors)[9] = {32, 104, 84, 190, 32, 33, 34, 18, 0};

/* Overlay 04, then the files its loader reads. */
CdlLOC DISC_FILE(Ovl04Loc)[9] = {
    CD_LOC(00, 34, 53), CD_LOC(00, 35, 10), CD_LOC(00, 36, 53),
    CD_LOC(00, 37, 60), CD_LOC(00, 40, 19), CD_LOC(00, 40, 54),
    CD_LOC(00, 41, 09), CD_LOC(00, 41, 46), CD_LOC(00, 41, 64),
};

s32 DISC_FILE(Ovl04Sectors)[9] = {32, 118, 82, 184, 35, 30, 37, 18, 18};

/* Overlay 05, then the files its loader reads. */
CdlLOC DISC_FILE(Ovl05Loc)[9] = {
    CD_LOC(00, 42, 08), CD_LOC(00, 42, 25), CD_LOC(00, 43, 72),
    CD_LOC(00, 44, 65), CD_LOC(00, 45, 74), CD_LOC(00, 46, 23),
    CD_LOC(00, 46, 41), CD_LOC(00, 46, 59), CD_LOC(00, 00, 00),
};

s32 DISC_FILE(Ovl05Sectors)[9] = {17, 122, 68, 84, 24, 18, 18, 13, 0};

/* Overlay 06, then the files its loader reads. */
CdlLOC DISC_FILE(Ovl06Loc)[9] = {
    CD_LOC(00, 46, 73), CD_LOC(00, 47, 30), CD_LOC(00, 48, 58),
    CD_LOC(00, 49, 40), CD_LOC(00, 51, 47), CD_LOC(00, 52, 07),
    CD_LOC(00, 52, 41), CD_LOC(00, 53, 07), CD_LOC(00, 00, 00),
};

s32 DISC_FILE(Ovl06Sectors)[9] = {32, 103, 57, 157, 35, 34, 41, 4, 0};

/* Overlay 07, then the files its loader reads. */
CdlLOC DISC_FILE(Ovl07Loc)[9] = {
    CD_LOC(00, 53, 12), CD_LOC(00, 53, 44), CD_LOC(00, 55, 08),
    CD_LOC(00, 55, 43), CD_LOC(00, 58, 04), CD_LOC(00, 58, 25),
    CD_LOC(00, 59, 09), CD_LOC(00, 59, 63), CD_LOC(01, 00, 06),
};

s32 DISC_FILE(Ovl07Sectors)[9] = {32, 114, 35, 186, 21, 59, 54, 18, 18};

/* Overlay 08, then the files its loader reads. */
CdlLOC DISC_FILE(Ovl08Loc)[9] = {
    CD_LOC(01, 00, 25), CD_LOC(01, 00, 42), CD_LOC(01, 02, 20),
    CD_LOC(01, 03, 07), CD_LOC(01, 04, 01), CD_LOC(01, 04, 20),
    CD_LOC(01, 04, 44), CD_LOC(01, 04, 62), CD_LOC(00, 00, 00),
};

s32 DISC_FILE(Ovl08Sectors)[9] = {17, 128, 62, 69, 19, 24, 18, 13, 0};

/* Overlay 09, then the files its loader reads. */
CdlLOC DISC_FILE(Ovl09Loc)[9] = {
    CD_LOC(01, 05, 01), CD_LOC(01, 05, 07), CD_LOC(01, 06, 35),
    CD_LOC(01, 07, 17), CD_LOC(01, 09, 24), CD_LOC(01, 09, 59),
    CD_LOC(01, 10, 18), CD_LOC(01, 10, 59), CD_LOC(00, 00, 00),
};

s32 DISC_FILE(Ovl09Sectors)[9] = {6, 103, 57, 157, 35, 34, 41, 20, 0};

/* Overlay 10, then the files its loader reads. */
CdlLOC DISC_FILE(Ovl10Loc)[9] = {
    CD_LOC(01, 11, 05), CD_LOC(01, 11, 11), CD_LOC(01, 12, 50),
    CD_LOC(01, 13, 10), CD_LOC(01, 15, 46), CD_LOC(01, 15, 67),
    CD_LOC(01, 16, 51), CD_LOC(01, 17, 30), CD_LOC(00, 00, 00),
};

s32 DISC_FILE(Ovl10Sectors)[9] = {6, 114, 35, 186, 21, 59, 54, 18, 0};

/* Overlay 11, then the files its loader reads. */
CdlLOC DISC_FILE(Ovl11Loc)[9] = {
    CD_LOC(01, 17, 49), CD_LOC(01, 17, 65), CD_LOC(01, 19, 37),
    CD_LOC(00, 00, 00), CD_LOC(00, 00, 00), CD_LOC(00, 00, 00),
    CD_LOC(00, 00, 00), CD_LOC(00, 00, 00), CD_LOC(00, 00, 00),
};

s32 DISC_FILE(Ovl11Sectors)[9] = {16, 122, 68, 0, 0, 0, 0, 0, 0};

/* Overlay 12, then the files its loader reads. */
CdlLOC DISC_FILE(Ovl12Loc)[9] = {
    CD_LOC(01, 20, 31), CD_LOC(01, 20, 61), CD_LOC(01, 22, 18),
    CD_LOC(01, 23, 23), CD_LOC(01, 25, 12), CD_LOC(01, 25, 60),
    CD_LOC(01, 25, 66), CD_LOC(01, 26, 28), CD_LOC(00, 00, 00),
};

s32 DISC_FILE(Ovl12Sectors)[9] = {30, 107, 80, 139, 48, 6, 37, 18, 0};

/* Overlay 13, then the files its loader reads. */
CdlLOC DISC_FILE(Ovl13Loc)[9] = {
    CD_LOC(01, 26, 47), CD_LOC(01, 27, 03), CD_LOC(01, 27, 46),
    CD_LOC(01, 27, 69), CD_LOC(01, 30, 31), CD_LOC(01, 30, 59),
    CD_LOC(01, 30, 64), CD_LOC(01, 31, 30), CD_LOC(01, 31, 48),
};

s32 DISC_FILE(Ovl13Sectors)[9] = {31, 43, 23, 187, 28, 5, 41, 18, 13};

/* Overlay 14, then the files its loader reads. */
CdlLOC DISC_FILE(Ovl14Loc)[9] = {
    CD_LOC(01, 31, 62), CD_LOC(01, 31, 74), CD_LOC(01, 32, 37),
    CD_LOC(00, 00, 00), CD_LOC(00, 00, 00), CD_LOC(00, 00, 00),
    CD_LOC(00, 00, 00), CD_LOC(00, 00, 00), CD_LOC(00, 00, 00),
};

s32 DISC_FILE(Ovl14Sectors)[9] = {12, 38, 69, 0, 0, 0, 0, 0, 0};

/* Overlay 15, then the files its loader reads. */
CdlLOC DISC_FILE(Ovl15Loc)[9] = {
    CD_LOC(01, 33, 32), CD_LOC(01, 33, 63), CD_LOC(01, 35, 43),
    CD_LOC(01, 36, 18), CD_LOC(01, 38, 54), CD_LOC(01, 39, 22),
    CD_LOC(01, 39, 30), CD_LOC(01, 39, 64), CD_LOC(00, 00, 00),
};

s32 DISC_FILE(Ovl15Sectors)[9] = {31, 130, 50, 186, 43, 8, 34, 20, 0};

/* Overlay 16, then the files its loader reads. */
CdlLOC DISC_FILE(Ovl16Loc)[9] = {
    CD_LOC(01, 40, 10), CD_LOC(01, 40, 39), CD_LOC(01, 41, 71),
    CD_LOC(01, 42, 25), CD_LOC(01, 44, 67), CD_LOC(01, 45, 15),
    CD_LOC(01, 45, 32), CD_LOC(01, 45, 69), CD_LOC(00, 00, 00),
};

s32 DISC_FILE(Ovl16Sectors)[9] = {29, 107, 29, 192, 23, 17, 37, 16, 0};

/* Overlay 17, then the files its loader reads. */
CdlLOC DISC_FILE(Ovl17Loc)[9] = {
    CD_LOC(01, 46, 11), CD_LOC(01, 46, 21), CD_LOC(01, 46, 73),
    CD_LOC(00, 00, 00), CD_LOC(00, 00, 00), CD_LOC(00, 00, 00),
    CD_LOC(00, 00, 00), CD_LOC(00, 00, 00), CD_LOC(00, 00, 00),
};

s32 DISC_FILE(Ovl17Sectors)[9] = {10, 52, 63, 0, 0, 0, 0, 0, 0};

/* A 9034-sector file at the end, which the engine never reads.
 * MATCHING: unsized, so the executable's -G8 keeps these single entries out
 * of small data. */
CdlLOC DISC_FILE(LastLoc)[] = {
    CD_LOC(01, 47, 62),
};

s32 DISC_FILE(LastSectors)[] = {9034};

#undef DISC_FILE
