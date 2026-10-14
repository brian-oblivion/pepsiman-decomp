/**
 * @file cdfile.h
 * @brief The disc's file table: every file of CDDATA/, its position and its
 * size in sectors, one pair of tables per directory.
 *
 * Every unit of the game includes this, so each holds its own copy: the
 * first 0x560 bytes of every unit's .data, in the executable and the
 * overlays alike. A unit reads the entries it loads (LoadChaseStage reads its
 * stage's files 1 and 2, the textures and the act-3 data); the rest are
 * unused copies. Directories 2 to G are the stages', file 0 the stage's
 * overlay. A table pads to its directory's slot count with an empty
 * position (track 1, 00:00:00) of 0 sectors.
 */
#ifndef CDFILE_H
#define CDFILE_H

#include "common.h"
#include "libcd.h"

/** @brief CDDATA/0/: each file's disc position. */
static CdlLOC sCdData0Loc[16] = {
    {0x00, 0x05, 0x69, 0x01}, /* 0000 */
    {0x00, 0x06, 0x07, 0x01}, /* 0001 */
    {0x00, 0x06, 0x24, 0x01}, /* 0002 */
    {0x00, 0x07, 0x26, 0x01}, /* 0003 */
    {0x00, 0x08, 0x02, 0x01}, /* 0004 */
    {0x00, 0x08, 0x39, 0x01}, /* 0005 */
    {0x00, 0x08, 0x46, 0x01}, /* 0100 */
    {0x00, 0x11, 0x14, 0x01}, /* 0101 */
    {0x00, 0x11, 0x58, 0x01}, /* 0102 */
    {0x00, 0x12, 0x23, 0x01}, /* 0103 */
    {0x00, 0x12, 0x66, 0x01}, /* 0104 */
    {0x00, 0x13, 0x25, 0x01}, /* 0105 */
    {0x00, 0x13, 0x69, 0x01}, /* 0106 */
    {0x00, 0x14, 0x40, 0x01}, /* 0107 */
    {0x00, 0x15, 0x08, 0x01}, /* 0108 */
    {0x00, 0x00, 0x00, 0x01}, /* none */
};
/** @brief CDDATA/0/: each file's size in sectors. */
static s32 sCdData0Sectors[16] = {
    13,  /* 0000 */
    17,  /* 0001 */
    77,  /* 0002 */
    51,  /* 0003 */
    37,  /* 0004 */
    7,   /* 0005 */
    193, /* 0100 */
    44,  /* 0101 */
    40,  /* 0102 */
    43,  /* 0103 */
    34,  /* 0104 */
    44,  /* 0105 */
    46,  /* 0106 */
    43,  /* 0107 */
    43,  /* 0108 */
    0,   /* none */
};

/** @brief CDDATA/1/: each file's disc position. */
static CdlLOC sCdData1Loc[20] = {
    {0x00, 0x15, 0x52, 0x01}, /* 1000 */
    {0x00, 0x15, 0x56, 0x01}, /* 1001 */
    {0x00, 0x16, 0x02, 0x01}, /* 1002 */
    {0x00, 0x18, 0x26, 0x01}, /* 1003 */
    {0x00, 0x18, 0x60, 0x01}, /* 1004 */
    {0x00, 0x19, 0x17, 0x01}, /* 1005 */
    {0x00, 0x19, 0x49, 0x01}, /* 1006 */
    {0x00, 0x20, 0x06, 0x01}, /* 1007 */
    {0x00, 0x20, 0x38, 0x01}, /* 1008 */
    {0x00, 0x20, 0x70, 0x01}, /* 1009 */
    {0x00, 0x21, 0x27, 0x01}, /* 1100 */
    {0x00, 0x21, 0x33, 0x01}, /* 1101 */
    {0x00, 0x22, 0x39, 0x01}, /* 1200 */
    {0x00, 0x22, 0x45, 0x01}, /* 1201 */
    {0x00, 0x23, 0x62, 0x01}, /* 1300 */
    {0x00, 0x23, 0x68, 0x01}, /* 1301 */
    {0x00, 0x24, 0x74, 0x01}, /* 1400 */
    {0x00, 0x25, 0x05, 0x01}, /* 1401 */
    {0x00, 0x26, 0x29, 0x01}, /* 1500 */
    {0x00, 0x26, 0x35, 0x01}, /* 1501 */
};
/** @brief CDDATA/1/: each file's size in sectors. */
static s32 sCdData1Sectors[20] = {
    4,   /* 1000 */
    21,  /* 1001 */
    174, /* 1002 */
    34,  /* 1003 */
    32,  /* 1004 */
    32,  /* 1005 */
    32,  /* 1006 */
    32,  /* 1007 */
    32,  /* 1008 */
    32,  /* 1009 */
    6,   /* 1100 */
    81,  /* 1101 */
    6,   /* 1200 */
    92,  /* 1201 */
    6,   /* 1300 */
    81,  /* 1301 */
    6,   /* 1400 */
    99,  /* 1401 */
    6,   /* 1500 */
    89,  /* 1501 */
};

/** @brief CDDATA/2/: each file's disc position. */
static CdlLOC sCdData2Loc[9] = {
    {0x00, 0x27, 0x50, 0x01}, /* 2000 */
    {0x00, 0x28, 0x07, 0x01}, /* 2001 */
    {0x00, 0x29, 0x36, 0x01}, /* 2002 */
    {0x00, 0x30, 0x45, 0x01}, /* 2003 */
    {0x00, 0x33, 0x10, 0x01}, /* 2004 */
    {0x00, 0x33, 0x42, 0x01}, /* 2005 */
    {0x00, 0x34, 0x00, 0x01}, /* 2006 */
    {0x00, 0x34, 0x34, 0x01}, /* 2007 */
    {0x00, 0x00, 0x00, 0x01}, /* none */
};
/** @brief CDDATA/2/: each file's size in sectors. */
static s32 sCdData2Sectors[9] = {
    32,  /* 2000 */
    104, /* 2001 */
    84,  /* 2002 */
    190, /* 2003 */
    32,  /* 2004 */
    33,  /* 2005 */
    34,  /* 2006 */
    18,  /* 2007 */
    0,   /* none */
};

/** @brief CDDATA/3/: each file's disc position. */
static CdlLOC sCdData3Loc[9] = {
    {0x00, 0x34, 0x53, 0x01}, /* 3000 */
    {0x00, 0x35, 0x10, 0x01}, /* 3001 */
    {0x00, 0x36, 0x53, 0x01}, /* 3002 */
    {0x00, 0x37, 0x60, 0x01}, /* 3003 */
    {0x00, 0x40, 0x19, 0x01}, /* 3004 */
    {0x00, 0x40, 0x54, 0x01}, /* 3005 */
    {0x00, 0x41, 0x09, 0x01}, /* 3006 */
    {0x00, 0x41, 0x46, 0x01}, /* 3007 */
    {0x00, 0x41, 0x64, 0x01}, /* 3008 */
};
/** @brief CDDATA/3/: each file's size in sectors. */
static s32 sCdData3Sectors[9] = {
    32,  /* 3000 */
    118, /* 3001 */
    82,  /* 3002 */
    184, /* 3003 */
    35,  /* 3004 */
    30,  /* 3005 */
    37,  /* 3006 */
    18,  /* 3007 */
    18,  /* 3008 */
};

/** @brief CDDATA/4/: each file's disc position. */
static CdlLOC sCdData4Loc[9] = {
    {0x00, 0x42, 0x08, 0x01}, /* 4000 */
    {0x00, 0x42, 0x25, 0x01}, /* 4001 */
    {0x00, 0x43, 0x72, 0x01}, /* 4002 */
    {0x00, 0x44, 0x65, 0x01}, /* 4003 */
    {0x00, 0x45, 0x74, 0x01}, /* 4004 */
    {0x00, 0x46, 0x23, 0x01}, /* 4005 */
    {0x00, 0x46, 0x41, 0x01}, /* 4006 */
    {0x00, 0x46, 0x59, 0x01}, /* 4007 */
    {0x00, 0x00, 0x00, 0x01}, /* none */
};
/** @brief CDDATA/4/: each file's size in sectors. */
static s32 sCdData4Sectors[9] = {
    17,  /* 4000 */
    122, /* 4001 */
    68,  /* 4002 */
    84,  /* 4003 */
    24,  /* 4004 */
    18,  /* 4005 */
    18,  /* 4006 */
    13,  /* 4007 */
    0,   /* none */
};

/** @brief CDDATA/5/: each file's disc position. */
static CdlLOC sCdData5Loc[9] = {
    {0x00, 0x46, 0x73, 0x01}, /* 5000 */
    {0x00, 0x47, 0x30, 0x01}, /* 5001 */
    {0x00, 0x48, 0x58, 0x01}, /* 5002 */
    {0x00, 0x49, 0x40, 0x01}, /* 5003 */
    {0x00, 0x51, 0x47, 0x01}, /* 5004 */
    {0x00, 0x52, 0x07, 0x01}, /* 5005 */
    {0x00, 0x52, 0x41, 0x01}, /* 5006 */
    {0x00, 0x53, 0x07, 0x01}, /* 5007 */
    {0x00, 0x00, 0x00, 0x01}, /* none */
};
/** @brief CDDATA/5/: each file's size in sectors. */
static s32 sCdData5Sectors[9] = {
    32,  /* 5000 */
    103, /* 5001 */
    57,  /* 5002 */
    157, /* 5003 */
    35,  /* 5004 */
    34,  /* 5005 */
    41,  /* 5006 */
    4,   /* 5007 */
    0,   /* none */
};

/** @brief CDDATA/6/: each file's disc position. */
static CdlLOC sCdData6Loc[9] = {
    {0x00, 0x53, 0x12, 0x01}, /* 6000 */
    {0x00, 0x53, 0x44, 0x01}, /* 6001 */
    {0x00, 0x55, 0x08, 0x01}, /* 6002 */
    {0x00, 0x55, 0x43, 0x01}, /* 6003 */
    {0x00, 0x58, 0x04, 0x01}, /* 6004 */
    {0x00, 0x58, 0x25, 0x01}, /* 6005 */
    {0x00, 0x59, 0x09, 0x01}, /* 6006 */
    {0x00, 0x59, 0x63, 0x01}, /* 6007 */
    {0x01, 0x00, 0x06, 0x01}, /* 6008 */
};
/** @brief CDDATA/6/: each file's size in sectors. */
static s32 sCdData6Sectors[9] = {
    32,  /* 6000 */
    114, /* 6001 */
    35,  /* 6002 */
    186, /* 6003 */
    21,  /* 6004 */
    59,  /* 6005 */
    54,  /* 6006 */
    18,  /* 6007 */
    18,  /* 6008 */
};

/** @brief CDDATA/7/: each file's disc position. */
static CdlLOC sCdData7Loc[9] = {
    {0x01, 0x00, 0x25, 0x01}, /* 7000 */
    {0x01, 0x00, 0x42, 0x01}, /* 7001 */
    {0x01, 0x02, 0x20, 0x01}, /* 7002 */
    {0x01, 0x03, 0x07, 0x01}, /* 7003 */
    {0x01, 0x04, 0x01, 0x01}, /* 7004 */
    {0x01, 0x04, 0x20, 0x01}, /* 7005 */
    {0x01, 0x04, 0x44, 0x01}, /* 7006 */
    {0x01, 0x04, 0x62, 0x01}, /* 7007 */
    {0x00, 0x00, 0x00, 0x01}, /* none */
};
/** @brief CDDATA/7/: each file's size in sectors. */
static s32 sCdData7Sectors[9] = {
    17,  /* 7000 */
    128, /* 7001 */
    62,  /* 7002 */
    69,  /* 7003 */
    19,  /* 7004 */
    24,  /* 7005 */
    18,  /* 7006 */
    13,  /* 7007 */
    0,   /* none */
};

/** @brief CDDATA/8/: each file's disc position. */
static CdlLOC sCdData8Loc[9] = {
    {0x01, 0x05, 0x01, 0x01}, /* 8000 */
    {0x01, 0x05, 0x07, 0x01}, /* 8001 */
    {0x01, 0x06, 0x35, 0x01}, /* 8002 */
    {0x01, 0x07, 0x17, 0x01}, /* 8003 */
    {0x01, 0x09, 0x24, 0x01}, /* 8004 */
    {0x01, 0x09, 0x59, 0x01}, /* 8005 */
    {0x01, 0x10, 0x18, 0x01}, /* 8006 */
    {0x01, 0x10, 0x59, 0x01}, /* 8007 */
    {0x00, 0x00, 0x00, 0x01}, /* none */
};
/** @brief CDDATA/8/: each file's size in sectors. */
static s32 sCdData8Sectors[9] = {
    6,   /* 8000 */
    103, /* 8001 */
    57,  /* 8002 */
    157, /* 8003 */
    35,  /* 8004 */
    34,  /* 8005 */
    41,  /* 8006 */
    20,  /* 8007 */
    0,   /* none */
};

/** @brief CDDATA/9/: each file's disc position. */
static CdlLOC sCdData9Loc[9] = {
    {0x01, 0x11, 0x05, 0x01}, /* 9000 */
    {0x01, 0x11, 0x11, 0x01}, /* 9001 */
    {0x01, 0x12, 0x50, 0x01}, /* 9002 */
    {0x01, 0x13, 0x10, 0x01}, /* 9003 */
    {0x01, 0x15, 0x46, 0x01}, /* 9004 */
    {0x01, 0x15, 0x67, 0x01}, /* 9005 */
    {0x01, 0x16, 0x51, 0x01}, /* 9006 */
    {0x01, 0x17, 0x30, 0x01}, /* 9007 */
    {0x00, 0x00, 0x00, 0x01}, /* none */
};
/** @brief CDDATA/9/: each file's size in sectors. */
static s32 sCdData9Sectors[9] = {
    6,   /* 9000 */
    114, /* 9001 */
    35,  /* 9002 */
    186, /* 9003 */
    21,  /* 9004 */
    59,  /* 9005 */
    54,  /* 9006 */
    18,  /* 9007 */
    0,   /* none */
};

/** @brief CDDATA/A/: each file's disc position. */
static CdlLOC sCdDataALoc[9] = {
    {0x01, 0x17, 0x49, 0x01}, /* A000 */
    {0x01, 0x17, 0x65, 0x01}, /* A001 */
    {0x01, 0x19, 0x37, 0x01}, /* A002 */
    {0x00, 0x00, 0x00, 0x01}, /* none */
    {0x00, 0x00, 0x00, 0x01}, /* none */
    {0x00, 0x00, 0x00, 0x01}, /* none */
    {0x00, 0x00, 0x00, 0x01}, /* none */
    {0x00, 0x00, 0x00, 0x01}, /* none */
    {0x00, 0x00, 0x00, 0x01}, /* none */
};
/** @brief CDDATA/A/: each file's size in sectors. */
static s32 sCdDataASectors[9] = {
    16,  /* A000 */
    122, /* A001 */
    68,  /* A002 */
    0,   /* none */
    0,   /* none */
    0,   /* none */
    0,   /* none */
    0,   /* none */
    0,   /* none */
};

/** @brief CDDATA/B/: each file's disc position. */
static CdlLOC sCdDataBLoc[9] = {
    {0x01, 0x20, 0x31, 0x01}, /* B000 */
    {0x01, 0x20, 0x61, 0x01}, /* B001 */
    {0x01, 0x22, 0x18, 0x01}, /* B002 */
    {0x01, 0x23, 0x23, 0x01}, /* B003 */
    {0x01, 0x25, 0x12, 0x01}, /* B004 */
    {0x01, 0x25, 0x60, 0x01}, /* B005 */
    {0x01, 0x25, 0x66, 0x01}, /* B006 */
    {0x01, 0x26, 0x28, 0x01}, /* B007 */
    {0x00, 0x00, 0x00, 0x01}, /* none */
};
/** @brief CDDATA/B/: each file's size in sectors. */
static s32 sCdDataBSectors[9] = {
    30,  /* B000 */
    107, /* B001 */
    80,  /* B002 */
    139, /* B003 */
    48,  /* B004 */
    6,   /* B005 */
    37,  /* B006 */
    18,  /* B007 */
    0,   /* none */
};

/** @brief CDDATA/C/: each file's disc position. */
static CdlLOC sCdDataCLoc[9] = {
    {0x01, 0x26, 0x47, 0x01}, /* C000 */
    {0x01, 0x27, 0x03, 0x01}, /* C001 */
    {0x01, 0x27, 0x46, 0x01}, /* C002 */
    {0x01, 0x27, 0x69, 0x01}, /* C003 */
    {0x01, 0x30, 0x31, 0x01}, /* C004 */
    {0x01, 0x30, 0x59, 0x01}, /* C005 */
    {0x01, 0x30, 0x64, 0x01}, /* C006 */
    {0x01, 0x31, 0x30, 0x01}, /* C007 */
    {0x01, 0x31, 0x48, 0x01}, /* C008 */
};
/** @brief CDDATA/C/: each file's size in sectors. */
static s32 sCdDataCSectors[9] = {
    31,  /* C000 */
    43,  /* C001 */
    23,  /* C002 */
    187, /* C003 */
    28,  /* C004 */
    5,   /* C005 */
    41,  /* C006 */
    18,  /* C007 */
    13,  /* C008 */
};

/** @brief CDDATA/D/: each file's disc position. */
static CdlLOC sCdDataDLoc[9] = {
    {0x01, 0x31, 0x62, 0x01}, /* D000 */
    {0x01, 0x31, 0x74, 0x01}, /* D001 */
    {0x01, 0x32, 0x37, 0x01}, /* D002 */
    {0x00, 0x00, 0x00, 0x01}, /* none */
    {0x00, 0x00, 0x00, 0x01}, /* none */
    {0x00, 0x00, 0x00, 0x01}, /* none */
    {0x00, 0x00, 0x00, 0x01}, /* none */
    {0x00, 0x00, 0x00, 0x01}, /* none */
    {0x00, 0x00, 0x00, 0x01}, /* none */
};
/** @brief CDDATA/D/: each file's size in sectors. */
static s32 sCdDataDSectors[9] = {
    12, /* D000 */
    38, /* D001 */
    69, /* D002 */
    0,  /* none */
    0,  /* none */
    0,  /* none */
    0,  /* none */
    0,  /* none */
    0,  /* none */
};

/** @brief CDDATA/E/: each file's disc position. */
static CdlLOC sCdDataELoc[9] = {
    {0x01, 0x33, 0x32, 0x01}, /* E000 */
    {0x01, 0x33, 0x63, 0x01}, /* E001 */
    {0x01, 0x35, 0x43, 0x01}, /* E002 */
    {0x01, 0x36, 0x18, 0x01}, /* E003 */
    {0x01, 0x38, 0x54, 0x01}, /* E004 */
    {0x01, 0x39, 0x22, 0x01}, /* E005 */
    {0x01, 0x39, 0x30, 0x01}, /* E006 */
    {0x01, 0x39, 0x64, 0x01}, /* E007 */
    {0x00, 0x00, 0x00, 0x01}, /* none */
};
/** @brief CDDATA/E/: each file's size in sectors. */
static s32 sCdDataESectors[9] = {
    31,  /* E000 */
    130, /* E001 */
    50,  /* E002 */
    186, /* E003 */
    43,  /* E004 */
    8,   /* E005 */
    34,  /* E006 */
    20,  /* E007 */
    0,   /* none */
};

/** @brief CDDATA/F/: each file's disc position. */
static CdlLOC sCdDataFLoc[9] = {
    {0x01, 0x40, 0x10, 0x01}, /* F000 */
    {0x01, 0x40, 0x39, 0x01}, /* F001 */
    {0x01, 0x41, 0x71, 0x01}, /* F002 */
    {0x01, 0x42, 0x25, 0x01}, /* F003 */
    {0x01, 0x44, 0x67, 0x01}, /* F004 */
    {0x01, 0x45, 0x15, 0x01}, /* F005 */
    {0x01, 0x45, 0x32, 0x01}, /* F006 */
    {0x01, 0x45, 0x69, 0x01}, /* F007 */
    {0x00, 0x00, 0x00, 0x01}, /* none */
};
/** @brief CDDATA/F/: each file's size in sectors. */
static s32 sCdDataFSectors[9] = {
    29,  /* F000 */
    107, /* F001 */
    29,  /* F002 */
    192, /* F003 */
    23,  /* F004 */
    17,  /* F005 */
    37,  /* F006 */
    16,  /* F007 */
    0,   /* none */
};

/** @brief CDDATA/G/: each file's disc position. */
static CdlLOC sCdDataGLoc[9] = {
    {0x01, 0x46, 0x11, 0x01}, /* G000 */
    {0x01, 0x46, 0x21, 0x01}, /* G001 */
    {0x01, 0x46, 0x73, 0x01}, /* G002 */
    {0x00, 0x00, 0x00, 0x01}, /* none */
    {0x00, 0x00, 0x00, 0x01}, /* none */
    {0x00, 0x00, 0x00, 0x01}, /* none */
    {0x00, 0x00, 0x00, 0x01}, /* none */
    {0x00, 0x00, 0x00, 0x01}, /* none */
    {0x00, 0x00, 0x00, 0x01}, /* none */
};
/** @brief CDDATA/G/: each file's size in sectors. */
static s32 sCdDataGSectors[9] = {
    10, /* G000 */
    52, /* G001 */
    63, /* G002 */
    0,  /* none */
    0,  /* none */
    0,  /* none */
    0,  /* none */
    0,  /* none */
    0,  /* none */
};

/** @brief CDDATA/H/: each file's disc position. */
static CdlLOC sCdDataHLoc[1] = {
    {0x01, 0x47, 0x62, 0x01}, /* H000 */
};
/** @brief CDDATA/H/: each file's size in sectors. */
static s32 sCdDataHSectors[1] = {
    9034, /* H000; its directory entry gives 7920 */
};

#endif /* CDFILE_H */
