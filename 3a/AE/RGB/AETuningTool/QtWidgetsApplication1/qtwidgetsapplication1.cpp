#include "stdafx.h"
#include "qtwidgetsapplication1.h"
#include <QDebug>


QtWidgetsApplication1::QtWidgetsApplication1(QWidget* parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	//paramVersion
	ui.tableVersion->setColumnCount(2);
	ui.tableVersion->setRowCount(1);
	QStringList m_HeaderVerticalVer;
	m_HeaderVerticalVer << QString("AEParamVer:");
	ui.tableVersion->setVerticalHeaderLabels(m_HeaderVerticalVer);
	ui.tableVersion->verticalHeader()->setVisible(true);
	ui.tableVersion->horizontalHeader()->setVisible(false);

	//1.tableWidgetInt
	ui.tableWidgetInt->setColumnCount(2);
	ui.tableWidgetInt->setRowCount(maxMeteringParamINTCount);
	QStringList m_HeaderHorizon;
	m_HeaderHorizon << QString("write in txt") << QString("value");
	ui.tableWidgetInt->setHorizontalHeaderLabels(m_HeaderHorizon);
	ui.tableWidgetInt->horizontalHeader()->setVisible(true);
	ui.tableWidgetInt->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

	QStringList m_HeaderVertical;
	m_HeaderVertical << QString("ucAntiFlickerMode") << QString("manual_isolevel") << QString("manual_exptime") << QString("manual_adgain") << QString("manual_ae_target")
		<< QString("ae_metering_mode") << QString("converge_speedlv") << QString("ae_target_mode") << QString("ae_expo_mode");
	ui.tableWidgetInt->setVerticalHeaderLabels(m_HeaderVertical);
	ui.tableWidgetInt->verticalHeader()->setVisible(true);
	ui.tableWidgetInt->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);

	//2.tableWidgetHist
	ui.tableWidgetHist->setColumnCount(maxIaeParamSwitchCase);
	ui.tableWidgetHist->setRowCount(maxIaeParamCount + 1);
	QStringList m_HeaderHorizonHist;
	m_HeaderHorizonHist << QString("Case0") << QString("Case1") << QString("Case2") << QString("Case3") << QString("Case4") << QString("Case5") << QString("Case6") << QString("Case7") << QString("Case8") << QString("Case9") << QString("Case10");
	ui.tableWidgetHist->setHorizontalHeaderLabels(m_HeaderHorizonHist);
	ui.tableWidgetHist->horizontalHeader()->setVisible(true);
	ui.tableWidgetHist->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

	QStringList m_HeaderVerticalHist;
	m_HeaderVerticalHist << QString("write in txt") << QString("hist_dark_high") << QString("hist_dark_low") << QString("hist_sat_high") << QString("hist_sat_low")
		<< QString("hist_dark_range_end0(scale1000)") << QString("hist_dark_range_end1(scale1000)") << QString("hist_dark_range_end2(scale1000)")
		<< QString("hist_dark_range_start0(scale1000)") << QString("hist_dark_range_start1(scale1000)") << QString("hist_dark_range_start2(scale1000)")
		<< QString("hist_sat_pushdown_range_end0(scale1000)") << QString("hist_sat_pushdown_range_end1(scale1000)") << QString("hist_sat_pushdown_range_end2(scale1000)")
		<< QString("hist_sat_pushdown_range_start0(scale1000)") << QString("hist_sat_pushdown_range_start1(scale1000)") << QString("hist_sat_pushdown_range_start2(scale1000)")
		<< QString("hist_sat_pushup_range_end0(scale1000)") << QString("hist_sat_pushup_range_end1(scale1000)") << QString("hist_sat_pushup_range_end2(scale1000)")
		<< QString("hist_sat_pushup_range_start0(scale1000)") << QString("hist_sat_pushup_range_start1(scale1000)") << QString("hist_sat_pushup_range_start2(scale1000)")
		<< QString("max_adjust_ratio") << QString("min_adjust_ratio") << QString("max_midtones_gain") << QString("max_shadows_gain") << QString("max_isp_midtones_gain");
	ui.tableWidgetHist->setVerticalHeaderLabels(m_HeaderVerticalHist);
	ui.tableWidgetHist->verticalHeader()->setVisible(true);
	ui.tableWidgetHist->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	for (int i = 1; i < maxIaeParamSwitchCase; i++)
	{
        ui.tableWidgetHist->setColumnHidden(i, true);
	}
	

	//3.tableWidgetBVTh
	ui.tableWidgetBVTh->setColumnCount(1);
	ui.tableWidgetBVTh->setRowCount(maxIaeBVThCount + 2);
	QStringList m_HeaderHorizonBV;
	m_HeaderHorizonBV << QString("value");
	ui.tableWidgetBVTh->setHorizontalHeaderLabels(m_HeaderHorizonBV);
	ui.tableWidgetBVTh->horizontalHeader()->setVisible(true);
	ui.tableWidgetBVTh->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

	QStringList m_HeaderVerticalBV;
	m_HeaderVerticalBV << QString("write in txt") << QString("wdrEnable") << QString("luxIdxth_0") << QString("luxIdxth_insert_01") << QString("luxIdxth_1")
		<< QString("luxIdxth_insert_12") << QString("luxIdxth_2") << QString("luxIdxth_insert_23") << QString("luxIdxth_3") << QString("luxIdxth_insert_34")
		<< QString("luxIdxth_4") << QString("luxIdxth_insert_45") << QString("luxIdxth_5") << QString("luxIdxth_insert_56") << QString("luxIdxth_6")
		<< QString("luxIdxth_insert_67") << QString("luxIdxth_7") << QString("luxIdxth_insert_78") << QString("luxIdxth_8") << QString("luxIdxth_insert_89");
	ui.tableWidgetBVTh->setVerticalHeaderLabels(m_HeaderVerticalBV);
	ui.tableWidgetBVTh->verticalHeader()->setVisible(true);
	ui.tableWidgetBVTh->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);

	//4.tableWidgetHistRatio
	ui.tableWidgetHistRatio->setColumnCount(1);
	ui.tableWidgetHistRatio->setRowCount(maxIaeHistRatioCount);
	QStringList m_HeaderHorizonHistR;
	m_HeaderHorizonHistR << QString("value");
	ui.tableWidgetHistRatio->setHorizontalHeaderLabels(m_HeaderHorizonHistR);
	ui.tableWidgetHistRatio->horizontalHeader()->setVisible(true);
	ui.tableWidgetHistRatio->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

	QStringList m_HeaderVerticalHistR;
	m_HeaderVerticalHistR << QString("brightYHistTh") << QString("darkYHistTh") << QString("brightRatioInTh") << QString("darkRatioInTh") << QString("brightRatioOutTh") << QString("darkRatioOutTh") << QString("WDRMode") << QString("wdrTargetGainInTh") << QString("wdrTargetGainOutTh");
	ui.tableWidgetHistRatio->setVerticalHeaderLabels(m_HeaderVerticalHistR);
	ui.tableWidgetHistRatio->verticalHeader()->setVisible(true);
	ui.tableWidgetHistRatio->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);

	//5.tableTarget
	ui.tableTgMode->setColumnCount(sceneCnt);
	ui.tableTgMode->setRowCount(1);
	QStringList m_HeaderHorizonTgMode;
	m_HeaderHorizonTgMode << QString("NORMAL") << QString("PREVIEW") << QString("NIGHT") << QString("FACE") << QString("SPORT");
	ui.tableTgMode->setHorizontalHeaderLabels(m_HeaderHorizonTgMode);
	ui.tableTgMode->horizontalHeader()->setVisible(true);
	ui.tableTgMode->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	QStringList m_HeaderVerticalTgMode;
	m_HeaderVerticalTgMode << QString("write in txt");


	ui.tableTgNormal->setColumnCount(aeTargetCol);
	ui.tableTgNormal->setRowCount(aeTargetRow);

	ui.tableTgPreview->setColumnCount(aeTargetCol);
	ui.tableTgPreview->setRowCount(aeTargetRow);

	ui.tableTgNight->setColumnCount(aeTargetCol);
	ui.tableTgNight->setRowCount(aeTargetRow);

	ui.tableTgFace->setColumnCount(aeTargetCol);
	ui.tableTgFace->setRowCount(aeTargetRow);

	ui.tableTgSport->setColumnCount(aeTargetCol);
	ui.tableTgSport->setRowCount(aeTargetRow);

	QStringList m_HeaderHorizonTg;
	m_HeaderHorizonTg << QString("luxidx") << QString("value") << QString("default0");
	QStringList m_HeaderVerticalTg;
	m_HeaderVerticalTg << QString("1") << QString("2") << QString("3") << QString("4") << QString("5") << QString("6") << QString("7") << QString("8");



	ui.tableTgNormal->setHorizontalHeaderLabels(m_HeaderHorizonTg);
	ui.tableTgNormal->horizontalHeader()->setVisible(true);
	ui.tableTgNormal->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.tableTgNormal->setVerticalHeaderLabels(m_HeaderVerticalTg);
	ui.tableTgNormal->verticalHeader()->setVisible(true);
	ui.tableTgNormal->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);

	ui.tableTgPreview->setHorizontalHeaderLabels(m_HeaderHorizonTg);
	ui.tableTgPreview->horizontalHeader()->setVisible(true);
	ui.tableTgPreview->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.tableTgPreview->setVerticalHeaderLabels(m_HeaderVerticalTg);
	ui.tableTgPreview->verticalHeader()->setVisible(true);
	ui.tableTgPreview->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);

	ui.tableTgNight->setHorizontalHeaderLabels(m_HeaderHorizonTg);
	ui.tableTgNight->horizontalHeader()->setVisible(true);
	ui.tableTgNight->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.tableTgNight->setVerticalHeaderLabels(m_HeaderVerticalTg);
	ui.tableTgNight->verticalHeader()->setVisible(true);
	ui.tableTgNight->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);

	ui.tableTgFace->setHorizontalHeaderLabels(m_HeaderHorizonTg);
	ui.tableTgFace->horizontalHeader()->setVisible(true);
	ui.tableTgFace->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.tableTgFace->setVerticalHeaderLabels(m_HeaderVerticalTg);
	ui.tableTgFace->verticalHeader()->setVisible(true);
	ui.tableTgFace->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);

	ui.tableTgSport->setHorizontalHeaderLabels(m_HeaderHorizonTg);
	ui.tableTgSport->horizontalHeader()->setVisible(true);
	ui.tableTgSport->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.tableTgSport->setVerticalHeaderLabels(m_HeaderVerticalTg);
	ui.tableTgSport->verticalHeader()->setVisible(true);
	ui.tableTgSport->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);

	//5.tableExpo
	ui.tableExpoMode->setColumnCount(aeTable_Cnt);
	ui.tableExpoMode->setRowCount(1);
	QStringList m_HeaderHorizonExpoMode;
	m_HeaderHorizonExpoMode << QString("AETABLE_PRECIEW_AUTO") << QString("AETABLE_CAPTURE_AUTO") << QString("AETABLE_CAPTURE_NIGHT") << QString("AETABLE_CAPTURE_HDR");
	ui.tableExpoMode->setHorizontalHeaderLabels(m_HeaderHorizonExpoMode);
	ui.tableExpoMode->horizontalHeader()->setVisible(true);
	ui.tableExpoMode->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	QStringList m_HeaderVerticalExpoMode;
	m_HeaderVerticalExpoMode << QString("write in txt");


	ui.tableExpoPreAuto->setColumnCount(aeExpoCol);
	ui.tableExpoPreAuto->setRowCount(aeExpoRow);

	ui.tableExpoCapAuto->setColumnCount(aeExpoCol);
	ui.tableExpoCapAuto->setRowCount(aeExpoRow);

	ui.tableExpoCapNight->setColumnCount(aeExpoCol);
	ui.tableExpoCapNight->setRowCount(aeExpoRow);

	ui.tableExpoCapHdr->setColumnCount(aeExpoCol);
	ui.tableExpoCapHdr->setRowCount(aeExpoRow);

	QStringList m_HeaderHorizonExpo;
	m_HeaderHorizonExpo << QString("expindex") << QString("exptime") << QString("gain") << QString("prio");
	QStringList m_HeaderVerticalExpo;
	m_HeaderVerticalExpo << QString("1") << QString("2") << QString("3") << QString("4") << QString("5") << QString("6") << QString("7") << QString("8");

	ui.tableExpoPreAuto->setHorizontalHeaderLabels(m_HeaderHorizonExpo);
	ui.tableExpoPreAuto->horizontalHeader()->setVisible(true);
	ui.tableExpoPreAuto->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.tableExpoPreAuto->setVerticalHeaderLabels(m_HeaderVerticalExpo);
	ui.tableExpoPreAuto->verticalHeader()->setVisible(true);
	ui.tableExpoPreAuto->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);

	ui.tableExpoCapAuto->setHorizontalHeaderLabels(m_HeaderHorizonExpo);
	ui.tableExpoCapAuto->horizontalHeader()->setVisible(true);
	ui.tableExpoCapAuto->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.tableExpoCapAuto->setVerticalHeaderLabels(m_HeaderVerticalExpo);
	ui.tableExpoCapAuto->verticalHeader()->setVisible(true);
	ui.tableExpoCapAuto->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);

	ui.tableExpoCapNight->setHorizontalHeaderLabels(m_HeaderHorizonExpo);
	ui.tableExpoCapNight->horizontalHeader()->setVisible(true);
	ui.tableExpoCapNight->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.tableExpoCapNight->setVerticalHeaderLabels(m_HeaderVerticalExpo);
	ui.tableExpoCapNight->verticalHeader()->setVisible(true);
	ui.tableExpoCapNight->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);

	ui.tableExpoCapHdr->setHorizontalHeaderLabels(m_HeaderHorizonExpo);
	ui.tableExpoCapHdr->horizontalHeader()->setVisible(true);
	ui.tableExpoCapHdr->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui.tableExpoCapHdr->setVerticalHeaderLabels(m_HeaderVerticalExpo);
	ui.tableExpoCapHdr->verticalHeader()->setVisible(true);
	ui.tableExpoCapHdr->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);


	//6.multicamera
	ui.tableMultiCamCtrl->setColumnCount(1);
	ui.tableMultiCamCtrl->setRowCount(maxMultiCamParamCount);
	QStringList m_HeaderHorizonCam;
	m_HeaderHorizonCam << QString("value");
	ui.tableMultiCamCtrl->setHorizontalHeaderLabels(m_HeaderHorizonCam);
	ui.tableMultiCamCtrl->horizontalHeader()->setVisible(true);
	ui.tableMultiCamCtrl->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

	QStringList m_HeaderVerticalCam;
	m_HeaderVerticalCam << QString("multiCamMode") << QString("BVStart") << QString("BVEnd") << QString("slaveWeightStart") << QString("slaveWeightEnd") << QString("varianceStableTh")
		<< QString("ymeanStableTh") << QString("stableFrame") << QString("cameraMixROI.left.start") << QString("cameraMixROI.left.end") << QString("cameraMixROI.left.top")
		<< QString("cameraMixROI.left.bottom") << QString("cameraMixROI.right.start") << QString("cameraMixROI.right.end") << QString("cameraMixROI.right.top") << QString("cameraMixROI.right.bottom");
	ui.tableMultiCamCtrl->setVerticalHeaderLabels(m_HeaderVerticalCam);
	ui.tableMultiCamCtrl->verticalHeader()->setVisible(true);
	ui.tableMultiCamCtrl->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);

	//7.config info
	ui.tableConfigInfo->setColumnCount(1);
	ui.tableConfigInfo->setRowCount(maxConfigInfoParamCount);
	QStringList m_HeaderHorizonConfig;
	m_HeaderHorizonConfig << QString("value");
	ui.tableConfigInfo->setHorizontalHeaderLabels(m_HeaderHorizonConfig);
	ui.tableConfigInfo->horizontalHeader()->setVisible(true);
	ui.tableConfigInfo->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

	QStringList m_HeaderVerticalConfig;
	m_HeaderVerticalConfig << QString("aeEnable") << QString("pipedelay") << QString("fNumberx1000") << QString("maxfps") << QString("minfps") << QString("maxGain")
		<< QString("minGain") << QString("maxLineCnt") << QString("minLineCnt") << QString("swMaxExptime") << QString("swMinExptime")
		<< QString("exptimePerLineNs") << QString("maxSensorGain");
	ui.tableConfigInfo->setVerticalHeaderLabels(m_HeaderVerticalConfig);
	ui.tableConfigInfo->verticalHeader()->setVisible(true);
	ui.tableConfigInfo->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

//void QtWidgetsApplication1::on_pushButton_updateraw_clicked()
//{
//    QString filename = QFileDialog::getOpenFileName(this, tr("open image"), QDir::homePath(), tr("(*.raw)\n(*.jpg)"));
//    qDebug() << "filename:" << filename;
//    QImage image = QImage(filename);
//    qDebug() << "image:" << image;
//    ui.statusBar->showMessage(tr("open success"), 3000);
//    ui.label->setPixmap(QPixmap::fromImage(image));
//    todo: loadrawParam;
//    return;
//}

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
	delete[] pResult;
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

void QtWidgetsApplication1::on_pushButton_loadParam_clicked()
{
	string dllAddr = GetCurrMoudle();
	string txtAddr;
	printf("dllAddr11 %s\n", dllAddr.c_str());
	txtAddr = dllAddr + "\\AEParam.bin";

	fstream fin;
	fin.open(txtAddr, ios::binary | ios::in);

	if (!fin.is_open())
	{
		QMessageBox::information(this, tr("warning"), tr("open AEParam.bin failed"), QMessageBox::Yes);
		return;
	}
	parseParam Param;
	fin.read((char*)&Param, sizeof(Param));

	//paramVer
	QTableWidgetItem* itemVerminor = new QTableWidgetItem(QString::number(Param.paramVer.minorVer));
	ui.tableVersion->setItem(0, 0, itemVerminor);
	QTableWidgetItem* itemVermajor = new QTableWidgetItem(QString::number(Param.paramVer.majorVer));
	ui.tableVersion->setItem(0, 1, itemVermajor);

	//1. single int Param
	for (int i = 0; i < maxMeteringParamINTCount; i++)
	{
		QTableWidgetItem* item0 = new QTableWidgetItem(QString::number(Param.paramSingleInt[i][0]));
		ui.tableWidgetInt->setItem(i, 0, item0);
		QTableWidgetItem* item1 = new QTableWidgetItem(QString::number(Param.paramSingleInt[i][1]));
		ui.tableWidgetInt->setItem(i, 1, item1);
	}

	//2. struct int Param - IAE tuning Param hist
	//fin >> Param.iaeHistEn;

	QTableWidgetItem* itemHist = new QTableWidgetItem(QString::number(Param.iaeHistEn));
	ui.tableWidgetHist->setItem(0, 0, itemHist);

	for (int i = 0; i < maxIaeParamCount; i++)
	{
		for (int j = 0; j < maxIaeParamSwitchCase; j++)
		{
			//fin >> Param.ParamIAEHist[i][j];
			QTableWidgetItem* itemHist0 = new QTableWidgetItem(QString::number(Param.paramIAEHist[i][j]));
			ui.tableWidgetHist->setItem(i + 1, j, itemHist0);
		}
	}

	//3.struct int Param - IAE tuning Param bv th 
	//fin >> Param.iaeBVThEn;
	QTableWidgetItem* itemBVTh = new QTableWidgetItem(QString::number(Param.iaeBVThEn));
	ui.tableWidgetBVTh->setItem(0, 0, itemBVTh);
	QTableWidgetItem* itemWdrEnable = new QTableWidgetItem(QString::number(Param.wdrEnable));
	ui.tableWidgetBVTh->setItem(1, 0, itemWdrEnable);
	for (int i = 0; i < maxIaeBVThCount; i++)
	{
		//fin >> Param.ParamIAEBVTh[i];
		QTableWidgetItem* itemBVTh0 = new QTableWidgetItem(QString::number(Param.paramIAEBVTh[i]));
		ui.tableWidgetBVTh->setItem(i + 2, 0, itemBVTh0);
	}

	//4.struct int Param - IAE tuning Param hist ratio
	for (int i = 0; i < maxIaeHistRatioCount; i++)
	{
		//fin >> Param.paramIAEHistRatio[i];
		QTableWidgetItem* itemHistRatio = new QTableWidgetItem(QString::number(Param.paramIAEHistRatio[i]));
		ui.tableWidgetHistRatio->setItem(i, 0, itemHistRatio);
	}

	//5.table
	for (int i = 0; i < sceneCnt; i++)
	{
		ui.tableTgMode->setItem(0, i, new QTableWidgetItem(QString::number(Param.targetTable[i].aeTargetEn)));
	}
	for (int i = 0; i < aeTargetRow; i++)
	{
		for (int j = 0; j < aeTargetCol; j++)
		{
			ui.tableTgNormal->setItem(i, j, new QTableWidgetItem(QString::number(Param.targetTable[0].aetargetTable[i][j])));
			ui.tableTgPreview->setItem(i, j, new QTableWidgetItem(QString::number(Param.targetTable[1].aetargetTable[i][j])));
			ui.tableTgNight->setItem(i, j, new QTableWidgetItem(QString::number(Param.targetTable[2].aetargetTable[i][j])));
			ui.tableTgFace->setItem(i, j, new QTableWidgetItem(QString::number(Param.targetTable[3].aetargetTable[i][j])));
			ui.tableTgSport->setItem(i, j, new QTableWidgetItem(QString::number(Param.targetTable[4].aetargetTable[i][j])));
		}
	}

	for (int i = 0; i < aeTable_Cnt; i++)
	{
		ui.tableExpoMode->setItem(0, i, new QTableWidgetItem(QString::number(Param.expousureTable[i].aeExpoEn)));
	}
	for (int i = 0; i < aeExpoRow; i++)
	{
		for (int j = 0; j < aeExpoCol; j++)
		{
			ui.tableExpoPreAuto->setItem(i, j, new QTableWidgetItem(QString::number(Param.expousureTable[0].aeExpoTable[i][j])));
			ui.tableExpoCapAuto->setItem(i, j, new QTableWidgetItem(QString::number(Param.expousureTable[1].aeExpoTable[i][j])));
			ui.tableExpoCapNight->setItem(i, j, new QTableWidgetItem(QString::number(Param.expousureTable[2].aeExpoTable[i][j])));
			ui.tableExpoCapHdr->setItem(i, j, new QTableWidgetItem(QString::number(Param.expousureTable[3].aeExpoTable[i][j])));
		}
	}

	//6.multicamera
    ui.tableMultiCamCtrl->setItem(0, 0, new QTableWidgetItem(QString::number(Param.multiCamParam.multiCamMode)));
	ui.tableMultiCamCtrl->setItem(1, 0, new QTableWidgetItem(QString::number(Param.multiCamParam.BVStart)));
	ui.tableMultiCamCtrl->setItem(2, 0, new QTableWidgetItem(QString::number(Param.multiCamParam.BVEnd)));
	ui.tableMultiCamCtrl->setItem(3, 0, new QTableWidgetItem(QString::number(Param.multiCamParam.slaveWeightStart, 'f', 2)));
	ui.tableMultiCamCtrl->setItem(4, 0, new QTableWidgetItem(QString::number(Param.multiCamParam.slaveWeightEnd, 'f', 2)));
	ui.tableMultiCamCtrl->setItem(5, 0, new QTableWidgetItem(QString::number(Param.multiCamParam.varianceStableTh)));
	ui.tableMultiCamCtrl->setItem(6, 0, new QTableWidgetItem(QString::number(Param.multiCamParam.ymeanStableTh)));
	ui.tableMultiCamCtrl->setItem(7, 0, new QTableWidgetItem(QString::number(Param.multiCamParam.stableFrame)));
	ui.tableMultiCamCtrl->setItem(8, 0, new QTableWidgetItem(QString::number(Param.multiCamParam.cameraMixROI[0].leftstart)));
	ui.tableMultiCamCtrl->setItem(9, 0, new QTableWidgetItem(QString::number(Param.multiCamParam.cameraMixROI[0].rightend)));
	ui.tableMultiCamCtrl->setItem(10, 0, new QTableWidgetItem(QString::number(Param.multiCamParam.cameraMixROI[0].topstart)));
	ui.tableMultiCamCtrl->setItem(11, 0, new QTableWidgetItem(QString::number(Param.multiCamParam.cameraMixROI[0].bottom)));
	ui.tableMultiCamCtrl->setItem(12, 0, new QTableWidgetItem(QString::number(Param.multiCamParam.cameraMixROI[1].leftstart)));
	ui.tableMultiCamCtrl->setItem(13, 0, new QTableWidgetItem(QString::number(Param.multiCamParam.cameraMixROI[1].rightend)));
	ui.tableMultiCamCtrl->setItem(14, 0, new QTableWidgetItem(QString::number(Param.multiCamParam.cameraMixROI[1].topstart)));
	ui.tableMultiCamCtrl->setItem(15, 0, new QTableWidgetItem(QString::number(Param.multiCamParam.cameraMixROI[1].bottom)));

	//7.configinfo
	ui.tableConfigInfo->setItem(0, 0, new QTableWidgetItem(QString::number(Param.configInfo.aeEnable)));
	ui.tableConfigInfo->setItem(1, 0, new QTableWidgetItem(QString::number(Param.configInfo.pipedelay)));
	ui.tableConfigInfo->setItem(2, 0, new QTableWidgetItem(QString::number(Param.configInfo.fNumberx1000)));
	ui.tableConfigInfo->setItem(3, 0, new QTableWidgetItem(QString::number(Param.configInfo.maxfps)));
	ui.tableConfigInfo->setItem(4, 0, new QTableWidgetItem(QString::number(Param.configInfo.minfps)));
	ui.tableConfigInfo->setItem(5, 0, new QTableWidgetItem(QString::number(Param.configInfo.maxGain)));
	ui.tableConfigInfo->setItem(6, 0, new QTableWidgetItem(QString::number(Param.configInfo.minGain)));
	ui.tableConfigInfo->setItem(7, 0, new QTableWidgetItem(QString::number(Param.configInfo.maxLineCnt)));
	ui.tableConfigInfo->setItem(8, 0, new QTableWidgetItem(QString::number(Param.configInfo.minLineCnt)));
	ui.tableConfigInfo->setItem(9, 0, new QTableWidgetItem(QString::number(Param.configInfo.swMaxExptime)));
	ui.tableConfigInfo->setItem(10, 0, new QTableWidgetItem(QString::number(Param.configInfo.swMinExptime)));
	ui.tableConfigInfo->setItem(11, 0, new QTableWidgetItem(QString::number(Param.configInfo.exptimePerLineNs)));
	ui.tableConfigInfo->setItem(12, 0, new QTableWidgetItem(QString::number(Param.configInfo.maxSensorGain)));
	fin.close();

	return;
}

void QtWidgetsApplication1::on_pushButton_saveParam_clicked()
{
	string dllAddr = GetCurrMoudle();
	string txtAddr;
	printf("dllAddr11 %s\n", dllAddr.c_str());
	txtAddr = dllAddr + "\\AEParam.bin";

	fstream fout;
	fout.open(txtAddr, ios::binary | ios::out | ios::in);
	if (!fout.is_open())
	{
		QMessageBox::information(this, tr("warning"), tr("open AEParam.bin failed"), QMessageBox::Yes);
		return;
	}
	parseParam Param;

	//paramVersion
	QString rowStringVer;
	rowStringVer = ui.tableVersion->item(0, 0)->text();
	Param.paramVer.minorVer = rowStringVer.toFloat();
	rowStringVer = ui.tableVersion->item(0, 1)->text();
	Param.paramVer.majorVer = rowStringVer.toInt();
	fout.write((char*)&Param.paramVer, sizeof(Param.paramVer));
	//1. single int Param
	for (int i = 0; i < maxMeteringParamINTCount; i++)
	{
		QString rowString;
		rowString = ui.tableWidgetInt->item(i, 0)->text();
		Param.paramSingleInt[i][0] = rowString.toInt();
		rowString = ui.tableWidgetInt->item(i, 1)->text();
		Param.paramSingleInt[i][1] = rowString.toInt();
		if (Param.paramSingleInt[i][0] == 1)
		{
			//fout << Param.paramSingleInt[i][0] << " " << Param.paramSingleInt[i][1] << endl;
			fout.write((char*)&Param.paramSingleInt[i][0], sizeof(Param.paramSingleInt[i][0]));
			fout.write((char*)&Param.paramSingleInt[i][1], sizeof(Param.paramSingleInt[i][1]));
		}
		else
		{
			//fout.seekp(2 * sizeof(int) + sizeof(" "), ios::cur);//int 4个字节？
			fout.seekp(sizeof(Param.paramSingleInt[i][0]) + sizeof(Param.paramSingleInt[i][1]), ios::cur);
		}

	}
	//2. struct int Param - IAE tuning Param hist
	QString rowString;
	rowString = ui.tableWidgetHist->item(0, 0)->text();
	Param.iaeHistEn = rowString.toInt();
	if (Param.iaeHistEn == 1)
	{
		fout.write((char*)&Param.iaeHistEn, sizeof(Param.iaeHistEn));
		for (int i = 0; i < maxIaeParamCount; i++)
		{
			for (int j = 0; j < maxIaeParamSwitchCase; j++)
			{
				QString rowString;
				rowString = ui.tableWidgetHist->item(i + 1, j)->text();
				Param.paramIAEHist[i][j] = rowString.toInt();
				fout.write((char*)&Param.paramIAEHist[i][j], sizeof(Param.paramIAEHist[i][j]));
			}
		}
	}
	else
	{
		//fout.seekp(maxIaeParamCount * maxIaeParamSwitchCase * 2 * sizeof(int), ios::cur);//int 2个字节？
		//fout << endl;
		fout.seekp(sizeof(Param.paramIAEHist) + sizeof(Param.iaeHistEn), ios::cur);
	}


	//3.struct int Param - IAE tuning Param bv th
	QString rowStringBVTh;
	rowStringBVTh = ui.tableWidgetBVTh->item(0, 0)->text();
	Param.iaeBVThEn = rowStringBVTh.toInt();
	rowStringBVTh = ui.tableWidgetBVTh->item(1, 0)->text();
	Param.wdrEnable = rowStringBVTh.toInt();
	//fout << Param.iaeBVThEn << endl;
	if (Param.iaeBVThEn == 1)
	{
		fout.write((char*)&Param.iaeBVThEn, sizeof(Param.iaeBVThEn));
		fout.write((char*)&Param.wdrEnable, sizeof(Param.wdrEnable));
		for (int i = 0; i < maxIaeBVThCount; i++)
		{
			QString rowStringBVTh0;
			rowStringBVTh0 = ui.tableWidgetBVTh->item(i + 2, 0)->text();
			Param.paramIAEBVTh[i] = rowStringBVTh0.toInt();
			//fout << Param.paramIAEBVTh[i] << endl;

		}
		fout.write((char*)Param.paramIAEBVTh, sizeof(Param.paramIAEBVTh));

		//4.struct int Param - IAE tuning Param hist ratio
		for (int i = 0; i < maxIaeHistRatioCount; i++)
		{
			QString rowStringHistRatio;
			rowStringHistRatio = ui.tableWidgetHistRatio->item(i, 0)->text();
			Param.paramIAEHistRatio[i] = rowStringHistRatio.toInt();
			//fout << Param.paramIAEHistRatio[i] << endl;

		}
		fout.write((char*)Param.paramIAEHistRatio, sizeof(Param.paramIAEHistRatio));
	}
	else
	{
		fout.seekp(sizeof(Param.paramIAEBVTh) + sizeof(Param.iaeBVThEn) + sizeof(Param.wdrEnable) + sizeof(Param.paramIAEHistRatio), ios::cur);
	}

	//5.table
	for (int m = 0; m < sceneCnt; m++)
	{
		QString rowStringTgEn;
		rowStringTgEn = ui.tableTgMode->item(0, m)->text();
		Param.targetTable[m].aeTargetEn = rowStringTgEn.toInt();
		if (Param.targetTable[m].aeTargetEn == 1)
		{
			for (int i = 0; i < aeTargetRow; i++)
			{
				for (int j = 0; j < aeTargetCol; j++)
				{
					QString rowString;
					switch (m)
					{
					case 0:
						rowString = ui.tableTgNormal->item(i, j)->text();
						break;
					case 1:
						rowString = ui.tableTgPreview->item(i, j)->text();
						break;
					case 2:
						rowString = ui.tableTgNight->item(i, j)->text();
						break;
					case 3:
						rowString = ui.tableTgFace->item(i, j)->text();
						break;
					case 4:
						rowString = ui.tableTgSport->item(i, j)->text();
						break;
					default:
						rowString = ui.tableTgNormal->item(i, j)->text();
						break;
					}
					Param.targetTable[m].aetargetTable[i][j] = rowString.toInt();
				}
			}
			fout.write((char*)&Param.targetTable[m], sizeof(Param.targetTable[m]));
			//fout.write((char*)&Param.targetTable[m].aetargetTable, sizeof(Param.targetTable[m].aetargetTable));
		}
		else
		{
			fout.seekp(sizeof(Param.targetTable[m]), ios::cur);
		}
	}
	for (int m = 0; m < aeTable_Cnt; m++)
	{
		QString rowStringExpoEn;
		rowStringExpoEn = ui.tableExpoMode->item(0, m)->text();
		Param.expousureTable[m].aeExpoEn = rowStringExpoEn.toInt();
		if (Param.expousureTable[m].aeExpoEn == 1)
		{
			for (int i = 0; i < aeExpoRow; i++)
			{
				for (int j = 0; j < aeExpoCol; j++)
				{
					QString rowString;
					switch (m)
					{
					case 0:
						rowString = ui.tableExpoPreAuto->item(i, j)->text();
						break;
					case 1:
						rowString = ui.tableExpoCapAuto->item(i, j)->text();
						break;
					case 2:
						rowString = ui.tableExpoCapNight->item(i, j)->text();
						break;
					case 3:
						rowString = ui.tableExpoCapHdr->item(i, j)->text();
						break;
					default:
						rowString = ui.tableExpoPreAuto->item(i, j)->text();
						break;
					}
					Param.expousureTable[m].aeExpoTable[i][j] = rowString.toInt();
				}
			}
			fout.write((char*)&Param.expousureTable[m], sizeof(Param.expousureTable[m]));
			//fout.write((char*)&Param.expousureTable[m].aeExpoTable, sizeof(Param.expousureTable[m].aeExpoTable));
		}
		else
		{
			fout.seekp(sizeof(Param.expousureTable[m]), ios::cur);
		}
	}

	//6.multicamera
	QString rowStringMultiCam;
	rowStringMultiCam = ui.tableMultiCamCtrl->item(0, 0)->text();
	Param.multiCamParam.multiCamMode = rowStringMultiCam.toInt();
	rowStringMultiCam = ui.tableMultiCamCtrl->item(1, 0)->text();
	Param.multiCamParam.BVStart = rowStringMultiCam.toInt();
	rowStringMultiCam = ui.tableMultiCamCtrl->item(2, 0)->text();
	Param.multiCamParam.BVEnd = rowStringMultiCam.toInt();
	rowStringMultiCam = ui.tableMultiCamCtrl->item(3, 0)->text();
	Param.multiCamParam.slaveWeightStart = rowStringMultiCam.toFloat();
	rowStringMultiCam = ui.tableMultiCamCtrl->item(4, 0)->text();
	Param.multiCamParam.slaveWeightEnd = rowStringMultiCam.toFloat();
	rowStringMultiCam = ui.tableMultiCamCtrl->item(5, 0)->text();
	Param.multiCamParam.varianceStableTh = rowStringMultiCam.toInt();
	rowStringMultiCam = ui.tableMultiCamCtrl->item(6, 0)->text();
	Param.multiCamParam.ymeanStableTh = rowStringMultiCam.toInt();
	rowStringMultiCam = ui.tableMultiCamCtrl->item(7, 0)->text();
	Param.multiCamParam.stableFrame = rowStringMultiCam.toInt();
	rowStringMultiCam = ui.tableMultiCamCtrl->item(8, 0)->text();
	Param.multiCamParam.cameraMixROI[0].leftstart = rowStringMultiCam.toInt();
	rowStringMultiCam = ui.tableMultiCamCtrl->item(9, 0)->text();
	Param.multiCamParam.cameraMixROI[0].rightend = rowStringMultiCam.toInt();
	rowStringMultiCam = ui.tableMultiCamCtrl->item(10, 0)->text();
	Param.multiCamParam.cameraMixROI[0].topstart = rowStringMultiCam.toInt();
	rowStringMultiCam = ui.tableMultiCamCtrl->item(11, 0)->text();
	Param.multiCamParam.cameraMixROI[0].bottom = rowStringMultiCam.toInt();
	rowStringMultiCam = ui.tableMultiCamCtrl->item(12, 0)->text();
	Param.multiCamParam.cameraMixROI[1].leftstart = rowStringMultiCam.toInt();
	rowStringMultiCam = ui.tableMultiCamCtrl->item(13, 0)->text();
	Param.multiCamParam.cameraMixROI[1].rightend = rowStringMultiCam.toInt();
	rowStringMultiCam = ui.tableMultiCamCtrl->item(14, 0)->text();
	Param.multiCamParam.cameraMixROI[1].topstart = rowStringMultiCam.toInt();
	rowStringMultiCam = ui.tableMultiCamCtrl->item(15, 0)->text();
	Param.multiCamParam.cameraMixROI[1].bottom = rowStringMultiCam.toInt();
	fout.write((char*)&Param.multiCamParam, sizeof(Param.multiCamParam));

	//7.configInfo
	QString rowStringConfigInfo;
	rowStringConfigInfo = ui.tableConfigInfo->item(0, 0)->text();
	Param.configInfo.aeEnable = rowStringConfigInfo.toInt();
	rowStringConfigInfo = ui.tableConfigInfo->item(1, 0)->text();
	Param.configInfo.pipedelay = rowStringConfigInfo.toInt();
	rowStringConfigInfo = ui.tableConfigInfo->item(2, 0)->text();
	Param.configInfo.fNumberx1000 = rowStringConfigInfo.toInt();
	rowStringConfigInfo = ui.tableConfigInfo->item(3, 0)->text();
	Param.configInfo.maxfps = rowStringConfigInfo.toInt();
	rowStringConfigInfo = ui.tableConfigInfo->item(4, 0)->text();
	Param.configInfo.minfps = rowStringConfigInfo.toInt();
	rowStringConfigInfo = ui.tableConfigInfo->item(5, 0)->text();
	Param.configInfo.maxGain = rowStringConfigInfo.toInt();
	rowStringConfigInfo = ui.tableConfigInfo->item(6, 0)->text();
	Param.configInfo.minGain = rowStringConfigInfo.toInt();
	rowStringConfigInfo = ui.tableConfigInfo->item(7, 0)->text();
	Param.configInfo.maxLineCnt = rowStringConfigInfo.toInt();
	rowStringConfigInfo = ui.tableConfigInfo->item(8, 0)->text();
	Param.configInfo.minLineCnt = rowStringConfigInfo.toInt();
	rowStringConfigInfo = ui.tableConfigInfo->item(9, 0)->text();
	Param.configInfo.swMaxExptime = rowStringConfigInfo.toInt();
	rowStringConfigInfo = ui.tableConfigInfo->item(10, 0)->text();
	Param.configInfo.swMinExptime = rowStringConfigInfo.toInt();
	rowStringConfigInfo = ui.tableConfigInfo->item(11, 0)->text();
	Param.configInfo.exptimePerLineNs = rowStringConfigInfo.toInt();
	rowStringConfigInfo = ui.tableConfigInfo->item(12, 0)->text();
	Param.configInfo.maxSensorGain = rowStringConfigInfo.toInt();
	fout.write((char*)&Param.configInfo, sizeof(Param.configInfo));

	fout.close();
	return;
}

void QtWidgetsApplication1::closeEvent(QCloseEvent* e)
{
	//isClosed = true;
	QMessageBox msg;
	msg.setText("info");
	msg.setInformativeText("accept ok or not");
	msg.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
	msg.setDefaultButton(QMessageBox::Ok);
	int ret = msg.exec();
	if (ret == QMessageBox::Ok)
	{
		e->accept();
	}
	else
	{
		e->ignore();
	}
	return;
}

void QtWidgetsApplication1::on_comboBox_swtichWDRCase()
{
	int idx = ui.comboBox->currentIndex();
	for (int i = 0; i < maxIaeParamSwitchCase; i++)
	{
		ui.tableWidgetHist->setColumnHidden(i, true);
	}
	ui.tableWidgetHist->setColumnHidden(idx, false);

}
QtWidgetsApplication1::~QtWidgetsApplication1()
{}
