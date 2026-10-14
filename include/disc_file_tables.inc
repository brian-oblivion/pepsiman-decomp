/* One copy of the disc's file table (disc_files.h), the same in
 * every copy: arrays named with the copy's prefix. The including
 * unit names the copy first,
 *   #define DISC_FILE_PREFIX sDisc
 *   #include "disc_file_tables.inc"
 * and includes it again, under another prefix, for each further copy. */

/* The files main loads at boot (its own copy is sBootFileLoc). */
CdlLOC DISC_FILE(BootLoc)[16] = {
    DISC_LOC(00, 05, 69), DISC_LOC(00, 06, 07), DISC_LOC(00, 06, 24), DISC_LOC(00, 07, 26),
    DISC_LOC(00, 08, 02), DISC_LOC(00, 08, 39), DISC_LOC(00, 08, 46), DISC_LOC(00, 11, 14),
    DISC_LOC(00, 11, 58), DISC_LOC(00, 12, 23), DISC_LOC(00, 12, 66), DISC_LOC(00, 13, 25),
    DISC_LOC(00, 13, 69), DISC_LOC(00, 14, 40), DISC_LOC(00, 15, 08), DISC_LOC(00, 00, 00)};
s32 DISC_FILE(BootSectors)[16] = {13, 17, 77, 51, 37, 7, 193, 44, 40, 43, 34, 44, 46, 43, 43, 0};

/* The menus' and the world intros' files; world N's intro pack is
 * entry DISC_INTRO_PACK(N). */
CdlLOC DISC_FILE(SharedLoc)[20] = {
    DISC_LOC(00, 15, 52), DISC_LOC(00, 15, 56), DISC_LOC(00, 16, 02), DISC_LOC(00, 18, 26),
    DISC_LOC(00, 18, 60), DISC_LOC(00, 19, 17), DISC_LOC(00, 19, 49), DISC_LOC(00, 20, 06),
    DISC_LOC(00, 20, 38), DISC_LOC(00, 20, 70), DISC_LOC(00, 21, 27), DISC_LOC(00, 21, 33),
    DISC_LOC(00, 22, 39), DISC_LOC(00, 22, 45), DISC_LOC(00, 23, 62), DISC_LOC(00, 23, 68),
    DISC_LOC(00, 24, 74), DISC_LOC(00, 25, 05), DISC_LOC(00, 26, 29), DISC_LOC(00, 26, 35)};
s32 DISC_FILE(SharedSectors)[20] = {4, 21, 174, 34, 32, 32, 32, 32, 32, 32,
                                    6, 81, 6,   92, 6,  81, 6,  99, 6,  89};

/* Fifteen sets of nine, the stages' files. */
CdlLOC DISC_FILE(Stage00Loc)[9] = {
    DISC_LOC(00, 27, 50), DISC_LOC(00, 28, 07), DISC_LOC(00, 29, 36),
    DISC_LOC(00, 30, 45), DISC_LOC(00, 33, 10), DISC_LOC(00, 33, 42),
    DISC_LOC(00, 34, 00), DISC_LOC(00, 34, 34), DISC_LOC(00, 00, 00)};
s32 DISC_FILE(Stage00Sectors)[9] = {32, 104, 84, 190, 32, 33, 34, 18, 0};
CdlLOC DISC_FILE(Stage01Loc)[9] = {
    DISC_LOC(00, 34, 53), DISC_LOC(00, 35, 10), DISC_LOC(00, 36, 53),
    DISC_LOC(00, 37, 60), DISC_LOC(00, 40, 19), DISC_LOC(00, 40, 54),
    DISC_LOC(00, 41, 09), DISC_LOC(00, 41, 46), DISC_LOC(00, 41, 64)};
s32 DISC_FILE(Stage01Sectors)[9] = {32, 118, 82, 184, 35, 30, 37, 18, 18};
CdlLOC DISC_FILE(Stage02Loc)[9] = {
    DISC_LOC(00, 42, 08), DISC_LOC(00, 42, 25), DISC_LOC(00, 43, 72),
    DISC_LOC(00, 44, 65), DISC_LOC(00, 45, 74), DISC_LOC(00, 46, 23),
    DISC_LOC(00, 46, 41), DISC_LOC(00, 46, 59), DISC_LOC(00, 00, 00)};
s32 DISC_FILE(Stage02Sectors)[9] = {17, 122, 68, 84, 24, 18, 18, 13, 0};
CdlLOC DISC_FILE(Stage03Loc)[9] = {
    DISC_LOC(00, 46, 73), DISC_LOC(00, 47, 30), DISC_LOC(00, 48, 58),
    DISC_LOC(00, 49, 40), DISC_LOC(00, 51, 47), DISC_LOC(00, 52, 07),
    DISC_LOC(00, 52, 41), DISC_LOC(00, 53, 07), DISC_LOC(00, 00, 00)};
s32 DISC_FILE(Stage03Sectors)[9] = {32, 103, 57, 157, 35, 34, 41, 4, 0};
CdlLOC DISC_FILE(Stage04Loc)[9] = {
    DISC_LOC(00, 53, 12), DISC_LOC(00, 53, 44), DISC_LOC(00, 55, 08),
    DISC_LOC(00, 55, 43), DISC_LOC(00, 58, 04), DISC_LOC(00, 58, 25),
    DISC_LOC(00, 59, 09), DISC_LOC(00, 59, 63), DISC_LOC(01, 00, 06)};
s32 DISC_FILE(Stage04Sectors)[9] = {32, 114, 35, 186, 21, 59, 54, 18, 18};
CdlLOC DISC_FILE(Stage05Loc)[9] = {
    DISC_LOC(01, 00, 25), DISC_LOC(01, 00, 42), DISC_LOC(01, 02, 20),
    DISC_LOC(01, 03, 07), DISC_LOC(01, 04, 01), DISC_LOC(01, 04, 20),
    DISC_LOC(01, 04, 44), DISC_LOC(01, 04, 62), DISC_LOC(00, 00, 00)};
s32 DISC_FILE(Stage05Sectors)[9] = {17, 128, 62, 69, 19, 24, 18, 13, 0};
CdlLOC DISC_FILE(Stage06Loc)[9] = {
    DISC_LOC(01, 05, 01), DISC_LOC(01, 05, 07), DISC_LOC(01, 06, 35),
    DISC_LOC(01, 07, 17), DISC_LOC(01, 09, 24), DISC_LOC(01, 09, 59),
    DISC_LOC(01, 10, 18), DISC_LOC(01, 10, 59), DISC_LOC(00, 00, 00)};
s32 DISC_FILE(Stage06Sectors)[9] = {6, 103, 57, 157, 35, 34, 41, 20, 0};
CdlLOC DISC_FILE(Stage07Loc)[9] = {
    DISC_LOC(01, 11, 05), DISC_LOC(01, 11, 11), DISC_LOC(01, 12, 50),
    DISC_LOC(01, 13, 10), DISC_LOC(01, 15, 46), DISC_LOC(01, 15, 67),
    DISC_LOC(01, 16, 51), DISC_LOC(01, 17, 30), DISC_LOC(00, 00, 00)};
s32 DISC_FILE(Stage07Sectors)[9] = {6, 114, 35, 186, 21, 59, 54, 18, 0};
CdlLOC DISC_FILE(Stage08Loc)[9] = {
    DISC_LOC(01, 17, 49), DISC_LOC(01, 17, 65), DISC_LOC(01, 19, 37),
    DISC_LOC(00, 00, 00), DISC_LOC(00, 00, 00), DISC_LOC(00, 00, 00),
    DISC_LOC(00, 00, 00), DISC_LOC(00, 00, 00), DISC_LOC(00, 00, 00)};
s32 DISC_FILE(Stage08Sectors)[9] = {16, 122, 68, 0, 0, 0, 0, 0, 0};
CdlLOC DISC_FILE(Stage09Loc)[9] = {
    DISC_LOC(01, 20, 31), DISC_LOC(01, 20, 61), DISC_LOC(01, 22, 18),
    DISC_LOC(01, 23, 23), DISC_LOC(01, 25, 12), DISC_LOC(01, 25, 60),
    DISC_LOC(01, 25, 66), DISC_LOC(01, 26, 28), DISC_LOC(00, 00, 00)};
s32 DISC_FILE(Stage09Sectors)[9] = {30, 107, 80, 139, 48, 6, 37, 18, 0};
CdlLOC DISC_FILE(Stage10Loc)[9] = {
    DISC_LOC(01, 26, 47), DISC_LOC(01, 27, 03), DISC_LOC(01, 27, 46),
    DISC_LOC(01, 27, 69), DISC_LOC(01, 30, 31), DISC_LOC(01, 30, 59),
    DISC_LOC(01, 30, 64), DISC_LOC(01, 31, 30), DISC_LOC(01, 31, 48)};
s32 DISC_FILE(Stage10Sectors)[9] = {31, 43, 23, 187, 28, 5, 41, 18, 13};
CdlLOC DISC_FILE(Stage11Loc)[9] = {
    DISC_LOC(01, 31, 62), DISC_LOC(01, 31, 74), DISC_LOC(01, 32, 37),
    DISC_LOC(00, 00, 00), DISC_LOC(00, 00, 00), DISC_LOC(00, 00, 00),
    DISC_LOC(00, 00, 00), DISC_LOC(00, 00, 00), DISC_LOC(00, 00, 00)};
s32 DISC_FILE(Stage11Sectors)[9] = {12, 38, 69, 0, 0, 0, 0, 0, 0};
CdlLOC DISC_FILE(Stage12Loc)[9] = {
    DISC_LOC(01, 33, 32), DISC_LOC(01, 33, 63), DISC_LOC(01, 35, 43),
    DISC_LOC(01, 36, 18), DISC_LOC(01, 38, 54), DISC_LOC(01, 39, 22),
    DISC_LOC(01, 39, 30), DISC_LOC(01, 39, 64), DISC_LOC(00, 00, 00)};
s32 DISC_FILE(Stage12Sectors)[9] = {31, 130, 50, 186, 43, 8, 34, 20, 0};
CdlLOC DISC_FILE(Stage13Loc)[9] = {
    DISC_LOC(01, 40, 10), DISC_LOC(01, 40, 39), DISC_LOC(01, 41, 71),
    DISC_LOC(01, 42, 25), DISC_LOC(01, 44, 67), DISC_LOC(01, 45, 15),
    DISC_LOC(01, 45, 32), DISC_LOC(01, 45, 69), DISC_LOC(00, 00, 00)};
s32 DISC_FILE(Stage13Sectors)[9] = {29, 107, 29, 192, 23, 17, 37, 16, 0};
CdlLOC DISC_FILE(Stage14Loc)[9] = {
    DISC_LOC(01, 46, 11), DISC_LOC(01, 46, 21), DISC_LOC(01, 46, 73),
    DISC_LOC(00, 00, 00), DISC_LOC(00, 00, 00), DISC_LOC(00, 00, 00),
    DISC_LOC(00, 00, 00), DISC_LOC(00, 00, 00), DISC_LOC(00, 00, 00)};
s32 DISC_FILE(Stage14Sectors)[9] = {10, 52, 63, 0, 0, 0, 0, 0, 0};

/* The XA audio file the world intros seek to. */
CdlLOC DISC_FILE(XaLoc)[1] = {DISC_LOC(01, 47, 62)};
s32 DISC_FILE(XaSectors)[1] = {9034};

#undef DISC_FILE_PREFIX
