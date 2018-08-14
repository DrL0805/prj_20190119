#include "platform_common.h"
#include "algorithm_lis3dh.h"


typedef struct
{
    unsigned char     up:1;
    unsigned char     down:1;
    unsigned char     ninestep:1;
    unsigned char     faststep:1;
    unsigned char     finishstep:1;
} __Lis3DStepFlag;
static __Lis3DStepFlag  Lis3DStepFlag = {0};



static unsigned int X_axis[4];
static unsigned int Y_axis[4];
static unsigned int Z_axis[4];

static unsigned char StepInternalTimer[12];
static unsigned char DifferenceTime[12];

volatile static unsigned int StepRange[12];


static unsigned int x2y2z2[5];

static unsigned char ReadDataCount =0;
volatile static unsigned char CompareCount =0;
static unsigned char ReplaceCount =0;
static unsigned char NineStepCount =0;
static unsigned char Count_46_7ms_Lis3dh =1;
static unsigned char BiggerCount =0;
//static unsigned char Lis3dh_15msCount =0;

static unsigned int CompareData =0;
static unsigned int MaxAcceleration =0, MinAcceleration =0;
static unsigned char InvalidTime =0;

#define StepUp1                     100//125
#define StepUp2                     55//55
#define StepDown1                   95//115
#define StepDown2                   45//45






// *******************************************************************************
// *Funtion name:CalculateStep
// *Description :calculate step, calorio, distance and judge if in runing ,
//               if big fire calorio.
// *
// *Input: none
// *Output:none
// /*****************************************************************************
int Algorithm_Calculate_Step(unsigned char xyzDataTemp[3])
{
    unsigned char   i;
    unsigned char   tempflag;
	unsigned char	xyzData[3];
    unsigned int    Ave_Xaxis, Ave_Yaxis, Ave_Zaxis;
    unsigned long   TempData_16;
    int             stepdata = 0;
    static unsigned char timecount = 0;

    xyzData[0] = xyzDataTemp[0];
	xyzData[1] = xyzDataTemp[1];
	xyzData[2] = xyzDataTemp[2];
    
    //Lis3dh_Read_xyz(xyzData);
    if(xyzData[0] & 0x80){xyzData[0] = ~xyzData[0] + 1; }
    if(xyzData[1] & 0x80){xyzData[1] = ~xyzData[1] + 1; }
    if(xyzData[2] & 0x80){xyzData[2] = ~xyzData[2] + 1; }
    X_axis[ReadDataCount] = xyzData[0] / 2;
    Y_axis[ReadDataCount] = xyzData[1] / 2;
    Z_axis[ReadDataCount] = xyzData[2] / 2;
    if(Lis3DStepFlag.faststep) {
       if(ReadDataCount == 0) {
           Ave_Xaxis = (X_axis[0] + X_axis[3]) >> 1;
           Ave_Yaxis = (Y_axis[0] + Y_axis[3]) >> 1;
           Ave_Zaxis = (Z_axis[0] + Z_axis[3]) >> 1;
       } else {
           Ave_Xaxis = (X_axis[ReadDataCount] + X_axis[ReadDataCount - 1]) >> 1;
           Ave_Yaxis = (Y_axis[ReadDataCount] + Y_axis[ReadDataCount - 1]) >> 1;
           Ave_Zaxis = (Z_axis[ReadDataCount] + Z_axis[ReadDataCount - 1]) >> 1;
       }
    } else {
        Ave_Xaxis = (X_axis[0] + X_axis[1] + X_axis[2] + X_axis[3]) >> 2;
        Ave_Yaxis = (Y_axis[0] + Y_axis[1] + Y_axis[2] + Y_axis[3]) >> 2;
        Ave_Zaxis = (Z_axis[0] + Z_axis[1] + Z_axis[2] + Z_axis[3]) >> 2;
    }
    Y_axis[ReadDataCount] = (unsigned char)(Ave_Yaxis);
    Z_axis[ReadDataCount] = (unsigned char)(Ave_Zaxis);

    x2y2z2[ReadDataCount] = (Ave_Xaxis * Ave_Xaxis + Ave_Yaxis * Ave_Yaxis + Ave_Zaxis * Ave_Zaxis);
    x2y2z2[4] = (x2y2z2[0] / 4 + x2y2z2[1] / 4 + x2y2z2[2] / 4 + x2y2z2[3] / 4);
    tempflag = 0;
    if(Count_46_7ms_Lis3dh) {Count_46_7ms_Lis3dh++; }
    if(Count_46_7ms_Lis3dh > 38) {
        Count_46_7ms_Lis3dh = 0;
        Lis3DStepFlag.ninestep = 0;
        Lis3DStepFlag.faststep = 0;
        NineStepCount = 0;
        ReplaceCount = 0;
    }
    if(Lis3DStepFlag.finishstep) {
        InvalidTime++;
        if(InvalidTime > 4) {
            Lis3DStepFlag.finishstep = 0;
        }
    }
    if(x2y2z2[4] > CompareData) {
        if(MaxAcceleration < x2y2z2[4]) {
            MaxAcceleration = x2y2z2[4];
        }
        if(Lis3DStepFlag.up){        //if has find up
            if(x2y2z2[4] > CompareData + StepUp2) {
                CompareData = x2y2z2[4];
            }
        } else {
            if(x2y2z2[4] > CompareData + StepUp1) {
                CompareData = x2y2z2[4];
                MaxAcceleration = x2y2z2[4];
                Lis3DStepFlag.up = 1;
                Lis3DStepFlag.down = 0;
                timecount = 0;          //special operate
            }
        }
    } else {
        if(x2y2z2[4] < MinAcceleration) {
            MinAcceleration = x2y2z2[4];
        }
        if(Lis3DStepFlag.down) {
            if(CompareData > x2y2z2[4] + StepDown2) {
                CompareData = x2y2z2[4];
                tempflag = 1;
            }
        } else {
            if(CompareData > x2y2z2[4] + StepDown1) {
                CompareData = x2y2z2[4];
                MinAcceleration = x2y2z2[4];
                Lis3DStepFlag.down = 1;
                InvalidTime = 0;
                tempflag = 1;
                timecount = 0;          //special operate
            }
        }
    }
    if(tempflag) {
        if(Lis3DStepFlag.up && (Lis3DStepFlag.finishstep == 0)){    //finish one step
            tempflag = 0;       //finish one step flag
            Lis3DStepFlag.up = 0;
            Lis3DStepFlag.down = 0;
            Lis3DStepFlag.finishstep = 1;
            InvalidTime = 0;
        }
    } else {
        tempflag = 1;
    }
        if(tempflag == 0) {

			if(Lis3DStepFlag.ninestep) {
                ReplaceCount++;
                if(ReplaceCount > 11) {
                    ReplaceCount = 1;
                }
                StepInternalTimer[ReplaceCount] = Count_46_7ms_Lis3dh;
                StepInternalTimer[0] = 0;
                for(i = 1; i < 12; i++) {
                    StepInternalTimer[0] += StepInternalTimer[i];
                }
                // if(ReplaceCount == 1)
                // {
                //     CaloriePerStep = BaseCalorie / StepInternalTimer[0];
                // }
                StepInternalTimer[0] = StepInternalTimer[0] / 11;

                if(Count_46_7ms_Lis3dh > 8 && Count_46_7ms_Lis3dh < 12) {
                    Lis3DStepFlag.faststep = 1;
                } else {
                    Lis3DStepFlag.faststep = 0;
                }

/**************************every step*************************************************/
                stepdata = 1;

/**************************every step*************************************************/
            } else {
                if(ReplaceCount == 0) {
                    StepInternalTimer[NineStepCount] = Count_46_7ms_Lis3dh;
					StepRange[NineStepCount]= MaxAcceleration - MinAcceleration;
                } else {
                    StepInternalTimer[ReplaceCount] = Count_46_7ms_Lis3dh;
					StepRange[ReplaceCount]= MaxAcceleration - MinAcceleration;
                }
                NineStepCount++;
                if(NineStepCount == 12) {
					BiggerCount = 0;
                    TempData_16 = 0;
                    for(i = 1; i < 12; i++) {
                        TempData_16 += StepInternalTimer[i];
                    }
                    StepInternalTimer[0] = (unsigned char)(TempData_16 / 11);
                    // CaloriePerStep = BaseCalorie / TempData_16;
                    for(i = 1; i < 12; i++) {
                        if(StepInternalTimer[i] > StepInternalTimer[0]) {
                            DifferenceTime[i] = (StepInternalTimer[i] - StepInternalTimer[0]) * 4;
                        } else {
                            DifferenceTime[i] = (StepInternalTimer[0] - StepInternalTimer[i]) * 4;
                        }
                        if(DifferenceTime[i] > StepInternalTimer[0]) {
                            BiggerCount++;
                        }
                        if(StepInternalTimer[i] > StepInternalTimer[0]) {
                            DifferenceTime[i] = (StepInternalTimer[i] - StepInternalTimer[0]) * 2;
                        } else {
                            DifferenceTime[i] = (StepInternalTimer[0] - StepInternalTimer[i]) * 2;
                        }
                        if(DifferenceTime[i] > StepInternalTimer[0]) {
                            BiggerCount++;
                        }
                    }
                    if(StepInternalTimer[0] > 8 && StepInternalTimer[0] < 12) {
                        Lis3DStepFlag.faststep = 1;
                    } else {
                        Lis3DStepFlag.faststep = 0;
                    }
                    if(BiggerCount < 5) {
#ifdef      SWINGJUDGE
                        BiggerCount = 0;
                        TempData_16 = 0;
                        for(i = 0; i < 9; i++) {
                            TempData_16 += StepRange[i];
                        }
                        TempData_16 = TempData_16 / 9;
                        for(i = 0; i < 9; i++) {
                            if(StepRange[i] > TempData_16) {
                                DifStepRange[i] = (StepRange[i] - TempData_16) * 3;
                            } else {
                                DifStepRange[i] = (TempData_16 - StepRange[i]) * 3;
                            }
                            if(DifStepRange[i] > TempData_16) {
                                BiggerCount++;
                            }
                            if(StepRange[i] > TempData_16) {
                                DifStepRange[i] = (StepRange[i] - TempData_16) * 2;
                            } else {
                                DifStepRange[i] = (TempData_16 - StepRange[i]) * 2;
                            }
                            if(DifStepRange[i] > TempData_16) {
                                BiggerCount++;
                            }
                        }

                        if((BiggerCount < 4) || (StepInternalTimer[0] < 12)) {
                            RunTimeCount = 0;
                            RunCount = 0;
                            Lis3DStepFlag.ninestep = 1;
                            NineStepCount = 0;
/**************************finish targe step enable every step calculate********************/
                            stepdata = 12;

/**************************finish targe step enable every step calculate********************/
                        } else {
                            NineStepCount--;
                            ReplaceCount++;
                            if(ReplaceCount > 8) {
                                ReplaceCount = 1;
                            }
                        }
#else
                        Lis3DStepFlag.ninestep = 1;
                        NineStepCount = 0;
/**************************finish targe step enable every step calculate********************/
                        stepdata = 12;
/**************************finish targe step enable every step calculate********************/
#endif
                    } else {
                        NineStepCount--;
                        ReplaceCount++;
                        if(ReplaceCount > 11) {
                            ReplaceCount = 1;
                        }
                    }
                }
            }
            Count_46_7ms_Lis3dh = 1;
        }
        ReadDataCount++;
        if(ReadDataCount > 3) {
            ReadDataCount = 0;
        }
        if(stepdata == 0) {
            timecount++;
        } else {
            timecount = 0;
        }
        if(timecount >= 250) {
            stepdata = -2;
            timecount = 0;
        }
        return stepdata;
}


