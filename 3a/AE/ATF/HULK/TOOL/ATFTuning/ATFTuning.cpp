#include "ATFTuning.h"

ATFTuning::ATFTuning(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);

    //atf tuning param struct

    //1. default param
    ui.tableWidget_init->setColumnCount(2);
    ui.tableWidget_init->setRowCount(DEFAULT_PARAM_NUM);

    QStringList m_HeaderHorizon;
    m_HeaderHorizon << QString("setParamEn") << QString("value");
    ui.tableWidget_init->setHorizontalHeaderLabels(m_HeaderHorizon);
    ui.tableWidget_init->horizontalHeader()->setVisible(true);
    ui.tableWidget_init->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);

    QStringList m_HeaderVertica;
    m_HeaderVertica << QString("line_time_ns") << QString("sensor_height") << QString("sensor_width")
        << QString("_50Hz_flicker_pos") << QString("_60Hz_flicker_pos") << QString("default_hz") << QString("level_50") << QString("level_60") << QString("level_stable_th")
        << QString("base") << QString("ratio_shift");
    ui.tableWidget_init->setVerticalHeaderLabels(m_HeaderVertica);
    ui.tableWidget_init->verticalHeader()->setVisible(true);
    ui.tableWidget_init->verticalHeader()->setSectionResizeMode(QHeaderView::Interactive);

    //2.effect param
    ui.tableWidget_set->setColumnCount(2);
    ui.tableWidget_set->setRowCount(EFFECT_PARAM_NUM);

    QStringList m_HeaderHorizon1;
    m_HeaderHorizon1 << QString("setParamEn") << QString("value");
    ui.tableWidget_set->setHorizontalHeaderLabels(m_HeaderHorizon1);
    ui.tableWidget_set->horizontalHeader()->setVisible(true);
    ui.tableWidget_set->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);

    QStringList m_HeaderVertical;
    m_HeaderVertical << QString("jump_sample_num") << QString("jump_sample_do_avg_count") << QString("enable_2nd_region") << QString("ratio_max_range") << QString("points_1st_region") << QString("points_2nd_region")
        << QString("th_amp") << QString("th_amp_min") << QString("ratio_peak_dc") << QString("ratio_peak_2nd") << QString("index_range_50_0") << QString("index_range_50_1") << QString("index_range_50_2")
        << QString("index_range_60_0") << QString("index_range_60_1") << QString("index_range_60_2") << QString("th_symm_min") << QString("size_symmetric") << QString("ratio_symmetric_0")
        << QString("ratio_symmetric_1") << QString("ratio_symmetric_2") << QString("ratio_symmetric_3") << QString("th_symm_50hz") << QString("th_symm_60hz") << QString("peak_decay_count") << QString("ratio_low_amp");
    ui.tableWidget_set->setVerticalHeaderLabels(m_HeaderVertical);
    ui.tableWidget_set->verticalHeader()->setVisible(true);
    ui.tableWidget_set->verticalHeader()->setSectionResizeMode(QHeaderView::Interactive);

    //3.enable param
    ui.tableWidget_enable->setColumnCount(2);
    ui.tableWidget_enable->setRowCount(ENABLE_PARAM_NUM);

    QStringList m_HeaderHorizon_en;
    m_HeaderHorizon_en << QString("setParamEn") << QString("value");
    ui.tableWidget_enable->setHorizontalHeaderLabels(m_HeaderHorizon_en);
    ui.tableWidget_enable->horizontalHeader()->setVisible(true);
    ui.tableWidget_enable->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);

    QStringList m_HeaderVertica_en;
    m_HeaderVertica_en << QString("enable") << QString("close_after_detecte") << QString("default_mode");
    ui.tableWidget_enable->setVerticalHeaderLabels(m_HeaderVertica_en);
    ui.tableWidget_enable->verticalHeader()->setVisible(true);
    ui.tableWidget_enable->verticalHeader()->setSectionResizeMode(QHeaderView::Interactive);

    //4. fs calc
    ui.flicker_calc->setColumnCount(2);
    ui.flicker_calc->setRowCount(FS_CALC_NUM);

    QStringList m_HeaderHorizon_fs;
    m_HeaderHorizon_fs << QString("setParamEn") << QString("value");
    ui.flicker_calc->setHorizontalHeaderLabels(m_HeaderHorizon_fs);
    ui.flicker_calc->horizontalHeader()->setVisible(true);
    ui.flicker_calc->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);

    QStringList m_HeaderVertica_fs;
    m_HeaderVertica_fs << QString("fs_th_dc_amp") << QString("amp_2nd_peak_ratio") << QString("freq_box_0")
        << QString("freq_box_1") << QString("freq_box_2") << QString("freq_box_3") << QString("freq_box_4") 
        << QString("freq_box_5") << QString("freq_box_6") << QString("vote_delta_th") << QString("fs_th_stable")
        << QString("fs_enable_close_after_change_mode");
    ui.flicker_calc->setVerticalHeaderLabels(m_HeaderVertica_fs);
    ui.flicker_calc->verticalHeader()->setVisible(true);
    ui.flicker_calc->verticalHeader()->setSectionResizeMode(QHeaderView::Interactive);
}

ATFTuning::~ATFTuning()
{}

string UnicodeToAnsi_str(const wchar_t* szStr)
{
    int nLen = WideCharToMultiByte(CP_ACP, 0, szStr, -1, NULL, 0, NULL, NULL);
    if (nLen == 0)
    {
        return NULL;
    }
    char* pResult = new char[nLen];
    WideCharToMultiByte(CP_ACP, 0, szStr, -1, pResult, nLen, NULL, NULL);
    string  str(pResult);
    delete(pResult);
    return str;
}

string GetCurrMoudle()
{
    HMODULE hModule = NULL;
    TCHAR dllDir[4096] = { 0 };
    GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCWSTR)GetCurrMoudle, &hModule);
    if (NULL != hModule)
    {
        GetModuleFileName(hModule, (LPWSTR)dllDir, 4096);
    }
    string dllAddr;
    dllAddr = UnicodeToAnsi_str(dllDir);
    printf("dlladdr %s\n", dllAddr.c_str());

    int  dwfind = dllAddr.find_last_of("\\");
    string strpath = dllAddr.substr(0, dwfind);
    string strname = dllAddr.substr(dwfind + 1);
    printf("strpath %s\n", strpath.c_str());

    return strpath;
}

void ATFTuning::on_pushButton_loadParam_clicked()
{
    string dllAddr = GetCurrMoudle();
    string txtAddr;
    printf("dllAddr11 %s\n", dllAddr.c_str());
    txtAddr = dllAddr + "\\ATFTuningParam.bin";

    fstream fin;
    fin.open(txtAddr, ios::binary | ios::in);

    if (!fin.is_open())
    {
        QMessageBox::information(this, tr("warning"), tr("open ATFTuningParam.txt failed"), QMessageBox::Yes);
        return;
    }
    parseParam param;
    fin.read((char*)&param, sizeof(param));

    //1. default param
    for (int i = 0; i < DEFAULT_PARAM_NUM; i++)
    {
        QTableWidgetItem* item0 = new QTableWidgetItem(QString::number(param.defaultParam[i][0]));
        ui.tableWidget_init->setItem(i, 0, item0);
        QTableWidgetItem* item1 = new QTableWidgetItem(QString::number(param.defaultParam[i][1]));
        ui.tableWidget_init->setItem(i, 1, item1);
    }

    //2. EFFECT param
    for (int i = 0; i < EFFECT_PARAM_NUM; i++)
    {
        QTableWidgetItem* item0 = new QTableWidgetItem(QString::number(param.setEffectParam[i][0]));
        ui.tableWidget_set->setItem(i, 0, item0);
        QTableWidgetItem* item1 = new QTableWidgetItem(QString::number(param.setEffectParam[i][1]));
        ui.tableWidget_set->setItem(i, 1, item1);
    }

    //3. ENABLE param
    for (int i = 0; i < ENABLE_PARAM_NUM; i++)
    {
        QTableWidgetItem* item0 = new QTableWidgetItem(QString::number(param.enableFlag[i][0]));
        ui.tableWidget_enable->setItem(i, 0, item0);
        QTableWidgetItem* item1 = new QTableWidgetItem(QString::number(param.enableFlag[i][1]));
        ui.tableWidget_enable->setItem(i, 1, item1);
    }

    // 4. fs comfig

    //5. fs calc param
    for (int i = 0; i < FS_CALC_NUM; i++)
    {
        QTableWidgetItem* item0 = new QTableWidgetItem(QString::number(param.setFScalcParam[i][0]));
        ui.flicker_calc->setItem(i, 0, item0);
        QTableWidgetItem* item1 = new QTableWidgetItem(QString::number(param.setFScalcParam[i][1]));
        ui.flicker_calc->setItem(i, 1, item1);
    }

    fin.close();

    QMessageBox msg;
    msg.setText("Info");
    msg.setInformativeText("Parameters load sucessed");
    QTimer::singleShot(1500, &msg, SLOT(close())); //也可将accept改为close
    msg.exec();
    return;
}

void ATFTuning::on_pushButton_saveParam_clicked()
{
    string dllAddr = GetCurrMoudle();
    string txtAddr;
    printf("dllAddr11 %s\n", dllAddr.c_str());
    txtAddr = dllAddr + "\\ATFTuningParam.bin";

    fstream fout;
    fout.open(txtAddr, ios::binary | ios::out | ios::in);

    if (!fout.is_open())
    {
        QMessageBox::information(this, tr("warning"), tr("open ATFTuningParam.txt failed"), QMessageBox::Yes);
        return;
    }
    parseParam param;

    //1. default param
    for (int i = 0; i < DEFAULT_PARAM_NUM; i++)
    {
        QString rowString;
        rowString = ui.tableWidget_init->item(i, 0)->text();
        param.defaultParam[i][0] = rowString.toShort();
        rowString = ui.tableWidget_init->item(i, 1)->text();
        param.defaultParam[i][1] = rowString.toShort();
        if (param.defaultParam[i][0] == 1)
        {
            fout.write((char*)&param.defaultParam[i][0], sizeof(param.defaultParam[i][0]));
            fout.write((char*)&param.defaultParam[i][1], sizeof(param.defaultParam[i][1]));
        }
        else
        {
            fout.seekp(sizeof(param.defaultParam[i][0]) + sizeof(param.defaultParam[i][1]), ios::cur);
        }
    }

    //2. effect param
    for (int i = 0; i < EFFECT_PARAM_NUM; i++)
    {
        QString rowString;
        rowString = ui.tableWidget_set->item(i, 0)->text();
        param.setEffectParam[i][0] = rowString.toShort();
        rowString = ui.tableWidget_set->item(i, 1)->text();
        param.setEffectParam[i][1] = rowString.toShort();
        if (param.setEffectParam[i][0] == 1)
        {
            fout.write((char*)&param.setEffectParam[i][0], sizeof(param.setEffectParam[i][0]));
            fout.write((char*)&param.setEffectParam[i][1], sizeof(param.setEffectParam[i][1]));
        }
        else
        {
            fout.seekp(sizeof(param.setEffectParam[i][0]) + sizeof(param.setEffectParam[i][1]), ios::cur);
        }
    }

    //3. enable param
    for (int i = 0; i < ENABLE_PARAM_NUM; i++)
    {
        QString rowString;
        rowString = ui.tableWidget_enable->item(i, 0)->text();
        param.enableFlag[i][0] = rowString.toShort();
        rowString = ui.tableWidget_enable->item(i, 1)->text();
        param.enableFlag[i][1] = rowString.toShort();
        if (param.enableFlag[i][0] == 1)
        {
            fout.write((char*)&param.enableFlag[i][0], sizeof(param.enableFlag[i][0]));
            fout.write((char*)&param.enableFlag[i][1], sizeof(param.enableFlag[i][1]));
        }
        else
        {
            fout.seekp(sizeof(param.enableFlag[i][0]) + sizeof(param.enableFlag[i][1]), ios::cur);
        }
    }

    // 4. fs comfig

    //5. fs calc param
    for (int i = 0; i < FS_CALC_NUM; i++)
    {
        QString rowString;
        rowString = ui.flicker_calc->item(i, 0)->text();
        param.setFScalcParam[i][0] = rowString.toShort();
        rowString = ui.flicker_calc->item(i, 1)->text();
        param.setFScalcParam[i][1] = rowString.toShort();
        if (param.setFScalcParam[i][0] == 1)
        {
            fout.write((char*)&param.setFScalcParam[i][0], sizeof(param.setFScalcParam[i][0]));
            fout.write((char*)&param.setFScalcParam[i][1], sizeof(param.setFScalcParam[i][1]));
        }
        else
        {
            fout.seekp(sizeof(param.setFScalcParam[i][0]) + sizeof(param.setFScalcParam[i][1]), ios::cur);
        }
    }


    fout.close();

    QMessageBox msg;
    msg.setText("Info");
    msg.setInformativeText("Changes save sucessed");

    QTimer::singleShot(1500, &msg, SLOT(accept())); //也可将accept改为close
    msg.exec();
    return;
}

void ATFTuning::closeEvent(QCloseEvent* e)
{
    QMessageBox msg;
    msg.setText("Info");
    msg.setInformativeText("Changes save or not");
    msg.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msg.setDefaultButton(QMessageBox::Yes);
    int ret = msg.exec();
    if (ret == QMessageBox::Yes)
    {
        on_pushButton_saveParam_clicked();
        e->accept();
    }
    else
    {
        e->accept();
    }
    return;
}
