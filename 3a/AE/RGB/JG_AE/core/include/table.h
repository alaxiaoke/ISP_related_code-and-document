#ifndef _AE_TARGET_H_
#define _AE_TARGET_H_

#include "./AE_Lib_core.h"
const static int EXPO_TRIGGER_NUM  =  8;
const static int EXPO_PARAM_NUM  =  4;

typedef enum eAETableID
{
    AETABLE_PRECIEW_AUTO,
    AETABLE_CAPTURE_AUTO,
    AETABLE_CAPTURE_NIGHT,
    AETABLE_CAPTURE_HDR,
    AETABLE_CNT
};

//{  // CAPTURE_AUTO //index, time, maxgain
//    {0, 7000, 64, 0},  // 1000   bv +1500  low light 30lux
//    { 1800, 7000, 32, 0 },   // 2500 indoor   100lux
//    { 2500, 7000, 16, 0 },   //  4000 indoor   //  4000  indoor -> outdoor 300lux
//    { 3500, 4166, 16, 0 },   //  4000  outdoor
//    { 4000, 4166, 16, 0 },  // 5500 outdoor   1500lux
//    { 5500, 4166, 12, 0 },  // 7000 
//    { 7000, 4166, 8, 0 },   // 7000+ 
//    { 8500, 2000, 4, 0 }
//}


static int G_EVTABLE[AETABLE_CNT][EXPO_TRIGGER_NUM][EXPO_PARAM_NUM] =
{
    {  // PRECIEW_AUTO
        {0, 7000, 64, 0}, //index, time, maxgain,priflag
        {1800, 7000, 32, 0},
        {2500, 7000, 16, 0},
        {3500, 4166, 16, 0},
        {4000, 4166, 16, 0},
        {5500, 4166, 12, 0},
        { 7000, 4166, 8, 0 },
        { 8500, 2000, 4, 0 }
    },
    {  // CAPTURE_AUTO //index, time, maxgain
        {0, 6, 1, 0},  // 
        {24343, 2000, 4, 0},   //
        {29170, 4166, 8, 0},   //  7000+ 
        {31515, 4166, 16, 0},   //outdoor   1500lux
        {33271, 7000, 16, 0},  //indoor -> outdoor 300lux
        {35616, 7000, 32, 0},  //indoor   100lux
        {37961, 7000, 64, 0},   //low light 30lux
        {39524, 11111, 64, 0}

    },
    {  // CAPTURE_NIGHT
        {1000, 7000, 64, 0}, //index, time, maxgain
        {2500, 7000, 32, 0},
        {4000, 7000, 16, 0},
        {5500, 4000, 16, 0},
        {7000, 2000, 16, 0},
        {8500, 1000, 16, 0}
    },
    {  // CAPTURE_HDR
        {1000, 7000, 64, 0}, //index, time, maxgain
        {2500, 7000, 32, 0},
        {4000, 7000, 16, 0},
        {5500, 4000, 16, 0},
        {7000, 2000, 16, 0},
        {8500, 1000, 16, 0}
    },
};

typedef struct strAETable
{
    enum eAETableID	eAETableID;    // AE MODE FOR DIFF TABLE    G_EVTABLE[eAETableID]
    int			totalIndex;
    int			trigerBV;
    int			maxBV;
    int			minBV;
    int			effectMaxBV;
    int			effectMinBV;
};

static struct strAETable g_AE_CaptureAutoTable =
{
    AETABLE_CAPTURE_AUTO,
    EXPO_TRIGGER_NUM,      				//totalIndex;
    //trigerBV;
    9000,						//maxBV;
    0,						//minBV;
    8500,						//effectMaxBV;
    0,						//effectMinBV;
};


/* Luma Targets */
enum SceneMode
{
    NORMAL,
    PREVIEW,
    NIGHT,
    FACE,
    SPORT,
    SCENECNT
};

const static int TAGET_TRIGGER_NUM = 8;
const static int TAGET_PARAM_NUM = 3;


//{   //PREVIEW
//    { 0, 1000 }, //index(bv), target  (ultra light) 30 lux  bv + 1500 
//    { 1000, 1800 }, //index(bv), target  (ultra light) 60 lux  bv + 1500 
//    { 1800, 2500 }, //index(bv), target  (ultra light) 100 lux  bv + 1500 
//    { 2500, 3000 },  // (lowlight)   150 lux
//    { 3500, 4000 }, // (indoor)    300 lux
//    { 4600, 4500 }, // (outdoor)    700 lux
//    { 5700, 4800 }, //(outdoor)     1500 lux
//    { 6700, 4800 } //(outdoor)     3000 lux
//},

//{   //scene mode  NORMAL
//    { 0, 4800 }, //(outdoor)     3000 lux
//    { 24343, 4800 }, //(outdoor)     1500 lux
//    { 29170, 4500 }, //(outdoor)    700 lux
//    { 31515, 4000 },  // (indoor)    300 lux
//    { 33271, 3000 }, // (lowlight)   150 lux
//    { 35616, 2500 }, // (ultra light) 100 lux
//    { 37961, 1800 }, //(ultra light) 60 lux
//    { 39524, 1000 } //(ultra light) 30 lux
//},

static int G_AE_TARGET[SCENECNT][TAGET_TRIGGER_NUM][TAGET_PARAM_NUM] =
{
    {   //scene mode  NORMAL
        {0, 4800}, //(outdoor)     3000 lux
        {24343, 4800}, //(outdoor)     1500 lux
        {29170, 4500}, //(outdoor)    700 lux
        {31515, 4000},  // (indoor)    300 lux
        {33271, 3000}, // (lowlight)   150 lux
        {35616, 2500}, // (ultra light) 100 lux
        {37961, 1800}, //(ultra light) 60 lux
        {39524, 1000} //(ultra light) 30 lux

    },
    {   //PREVIEW wdrtarget
        {0, 4800}, //(outdoor)     3000 lux
        {24343, 4800}, //(outdoor)     1500 lux
        {29170, 4500}, //(outdoor)    700 lux
        {31515, 4000},  // (indoor)    300 lux
        {33271, 3000}, // (lowlight)   150 lux
        {35616, 2500}, // (ultra light) 100 lux
        {37961, 1800}, //(ultra light) 60 lux
        {39524, 1000} //(ultra light) 30 lux
    },
    {   //NIGHT
        {1000, 30}, //index, target
        {3000, 50},
        {5000, 55},
        {7000, 65},
        {8500, 70}//TODO
    },
    {   //FACE
        {1000, 30}, //index, target
        {3000, 50},
        {5000, 55},
        {7000, 65},
        {8500, 70}//TODO
    },
    {   //SPORT
        {1000, 30}, //index, target
        {3000, 50},
        {5000, 55},
        {7000, 65},
        {8500, 70}//TODO
    },
};

typedef struct strAETarget
{
    int			totalIndex;         ///< Num Zones Used 
    int         minTarget;
    int         maxTarget;
    enum SceneMode   eSceneMode;         ///< scenemode to select target table  G_AE_TARGET[eSceneMode]
};

static struct strAETarget g_AE_NormalTarget =
{
    TAGET_TRIGGER_NUM,
    30,
    70,
    NORMAL,       ///< scene Used  to select Triggers index & target table 
};


UINT32 updateTarget(void* ae_runtime_dat, int aemode);
UINT32 matchExpoTable(void* ptAEBuffer, void* ae_output, INT16 wBv1000, int aemode);
void   checkExpoTable(float expIndex, int evIndex, int totalIndex, int (*table)[4], UINT32 &newExposureTime, UINT32 &newExposureGain, void* ae_runtime_dat);
void   isDynamicScene(void* ae_runtime_dat);
#endif 