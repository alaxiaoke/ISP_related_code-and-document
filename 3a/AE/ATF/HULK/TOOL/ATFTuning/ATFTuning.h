#pragma once

#include <QtWidgets/QWidget>
#include "ui_ATFTuning.h"

#include<Windows.h>
#include "QTableWidget.h"
#include "QComboBox.h"
#include "QDebug.h"
#include <QtWidgets>

#include <fstream>
#include <iostream>

using namespace std;

#define DEFAULT_PARAM_NUM (11)
#define EFFECT_PARAM_NUM  (26)
#define ENABLE_PARAM_NUM  (3)
#define FS_CONFIG_NUM     (3)
#define FS_CALC_NUM       (12)


struct parseParam
{
    uint16_t defaultParam[DEFAULT_PARAM_NUM][2];
    uint16_t setEffectParam[EFFECT_PARAM_NUM][2];
    uint16_t  enableFlag[ENABLE_PARAM_NUM][2];
    uint16_t  setFScalcParam[FS_CALC_NUM][2];
};


class ATFTuning : public QWidget
{
    Q_OBJECT

public:
    ATFTuning(QWidget *parent = nullptr);
    ~ATFTuning();

private:
    Ui::ATFTuningClass ui;

protected:
    public slots:
    void on_pushButton_loadParam_clicked();
    void on_pushButton_saveParam_clicked();
    void closeEvent(QCloseEvent* e);
};
