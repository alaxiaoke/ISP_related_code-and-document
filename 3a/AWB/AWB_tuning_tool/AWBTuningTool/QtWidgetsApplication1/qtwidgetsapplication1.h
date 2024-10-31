#pragma once

#include <QtWidgets/QMainWindow>
#include "stdafx.h"
#include "ui_qtwidgetsapplication1.h"
//#include "QtWidgetsClassMetering.h"
//#include "QtWidgetsClassArbitration.h"

#include "QTableWidget.h"
#include "QComboBox.h"
#include "QDebug.h"

#include <fstream>
#include <iostream>
using namespace std;

const int maxMeteringParamINTCount = 9;
const int colorRegionsCount = 8;
const int misColorRegionsCount = 8;
const int maxIaeParamCount = 26;
const int arbParamIdxStart = 98;
const int sceneCnt = 5;
const int aeTargetRow = 7;
const int aeTargetCol = 3;
const int aeTable_Cnt = 4;
const int aeExpoRow = 8;
const int aeExpoCol = 3;

const int CCTLevel = 7;
const int LightLevel = 5;
const int lightSourceNum = 7;
const int ccmNum = 5;
const int shiftLevel = 13;

struct parseParam
{
    int lightSourceTable[lightSourceNum][2];
    int lightSourceShift[lightSourceNum][2];
    int cctWeightTable[LightLevel][CCTLevel];
    int lvWeight[5];
    int greyRegions[lightSourceNum][8];
    int colorRegions[colorRegionsCount][8];
    int misColorRegions[misColorRegionsCount][8];
    int colorRegionsParams[colorRegionsCount][3];
    int ccmParams[ccmNum][9];
    int shiftRG[LightLevel][shiftLevel][shiftLevel];
    int shiftBG[LightLevel][shiftLevel][shiftLevel];
    int DEBUG_LEVEL[3];
};

class QtWidgetsApplication1 : public QMainWindow
{
    Q_OBJECT

public:
    QtWidgetsApplication1(QWidget *parent = nullptr);
    ~QtWidgetsApplication1();

private:
    Ui::QtWidgetsApplication1Class ui;

protected:
    public slots:
    void on_pushButton_loadParam_clicked();
    void on_pushButton_saveParam_clicked();
    void closeEvent(QCloseEvent *e);
};
