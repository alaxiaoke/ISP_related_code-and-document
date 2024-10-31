#pragma once

#include <QtWidgets/QMainWindow>
#include "stdafx.h"
#include "ui_qtwidgetsapplication1.h"

#include "QTableWidget.h"
#include "QComboBox.h"
#include "QDebug.h"

#include <fstream>
#include <iostream>
#include <Windows.h>
using namespace std;

const int maxMeteringParamINTCount = 9;
const int maxIaeParamSwitchCase = 10;
const int maxIaeParamCount = 27;
const int maxIaeBVThCount = 18;
const int maxIaeHistRatioCount = 9;
const int sceneCnt = 5;
const int aeTargetRow = 8;
const int aeTargetCol = 3;
const int aeTable_Cnt = 4;
const int aeExpoRow = 8;
const int aeExpoCol = 4;
const int maxMultiCamParamCount = 16;
const int maxConfigInfoParamCount = 13;

#pragma pack(push) /* push current alignment setting to stack */
#pragma pack(4)    /* new alignment setting */
struct targetTableStruct {
    int aeTargetEn;
    int aetargetTable[aeTargetRow][aeTargetCol];
};
#pragma pack(pop)  /* restore old alignment setting from stack */


#pragma pack(push) /* push current alignment setting to stack */
#pragma pack(4)    /* new alignment setting */
struct expousureTableStruct {
    int aeExpoEn;
    int aeExpoTable[aeExpoRow][aeExpoCol];
};
#pragma pack(pop)  /* restore old alignment setting from stack */

enum multiCamModeType {
    AE_MULTICAM_MODE_BYSELF,
    AE_MULTICAM_MODE_ONLYMASTER,
    AE_MULTICAM_MODE_WEIGHTEXP_BYMASTER,
    AE_MULTICAM_MODE_WEIGHTSTATS_SLAVEBYSELF,
    AE_MULTICAM_MODE_WEIGHTSTATS_MASTERTWOINONE,
    AE_MULTICAM_MODE_MAX
};

#pragma pack(push) /* push current alignment setting to stack */
#pragma pack(4)    /* new alignment setting */
struct cameraMixROIParam {
    int leftstart;
    int rightend;
    int topstart;
    int bottom;
};
#pragma pack(pop)  /* restore old alignment setting from stack */

#pragma pack(push) /* push current alignment setting to stack */
#pragma pack(4)    /* new alignment setting */
struct multiCamStruct {
    int multiCamMode;
    int BVStart;
    int BVEnd;
    float slaveWeightStart;
    float slaveWeightEnd;
    int varianceStableTh;
    int ymeanStableTh;
    int stableFrame;
    cameraMixROIParam cameraMixROI[2];
};
#pragma pack(pop)  /* restore old alignment setting from stack */

#pragma pack(push) /* push current alignment setting to stack */
#pragma pack(4)    /* new alignment setting */
struct configInitInfo {
    int aeEnable;
    int pipedelay;
    int fNumberx1000;
    int maxfps;
    int minfps;
    int maxGain;
    int minGain;
    int maxLineCnt;
    int minLineCnt;
    int swMaxExptime;
    int swMinExptime;
    int exptimePerLineNs;
    int maxSensorGain;
};
#pragma pack(pop)  /* restore old alignment setting from stack */

#pragma pack(push) /* push current alignment setting to stack */
#pragma pack(4)    /* new alignment setting */
struct paramVersion {
    float minorVer;
    int   majorVer;
};
#pragma pack(pop)  /* restore old alignment setting from stack */

#pragma pack(push) /* push current alignment setting to stack */
#pragma pack(4)    /* new alignment setting */
struct parseParam
{
    struct paramVersion paramVer;
    int paramSingleInt[maxMeteringParamINTCount][2];
    int iaeHistEn;
    int paramIAEHist[maxIaeParamCount][maxIaeParamSwitchCase];
    int iaeBVThEn;
    int wdrEnable;
    int paramIAEBVTh[maxIaeBVThCount];
    int paramIAEHistRatio[maxIaeHistRatioCount];
    struct targetTableStruct targetTable[sceneCnt];
    struct expousureTableStruct expousureTable[aeTable_Cnt];
    struct multiCamStruct multiCamParam;
    struct configInitInfo configInfo;
};
#pragma pack(pop)  /* restore old alignment setting from stack */

class QtWidgetsApplication1 : public QMainWindow
{
    Q_OBJECT

public:
    QtWidgetsApplication1(QWidget* parent = nullptr);
    ~QtWidgetsApplication1();

private:
    Ui::QtWidgetsApplication1Class ui;

protected:
public slots:
    void on_pushButton_loadParam_clicked();
    void on_pushButton_saveParam_clicked();
    void on_comboBox_swtichWDRCase();
    void closeEvent(QCloseEvent* e);
};
