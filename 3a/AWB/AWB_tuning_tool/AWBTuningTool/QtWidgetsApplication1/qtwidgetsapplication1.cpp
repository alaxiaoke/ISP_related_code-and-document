#include "stdafx.h"
#include "qtwidgetsapplication1.h"

QtWidgetsApplication1::QtWidgetsApplication1(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

	//1.tableWidgetInt
	ui.tableWidgetInt->setColumnCount(4);
	ui.tableWidgetInt->setRowCount(lightSourceNum);
	QStringList m_HeaderHorizon;
	m_HeaderHorizon << QString("R/G") << QString("B/G") << QString("R/G shift") << QString("B/G shift");
	ui.tableWidgetInt->setHorizontalHeaderLabels(m_HeaderHorizon);
	ui.tableWidgetInt->horizontalHeader()->setVisible(true);
	ui.tableWidgetInt->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);

	QStringList m_HeaderVertical;
	m_HeaderVertical << QString("H") << QString("U30") << QString("TL84") << QString("CWF") << QString("D50") << QString("D65") << QString("D75");
	ui.tableWidgetInt->setVerticalHeaderLabels(m_HeaderVertical);
	ui.tableWidgetInt->verticalHeader()->setVisible(true);
	ui.tableWidgetInt->verticalHeader()->setSectionResizeMode(QHeaderView::Interactive);

	//2.tableWidgetHist
	ui.tableWidgetHist->setColumnCount(8);
	ui.tableWidgetHist->setRowCount(colorRegionsCount);
	QStringList m_HeaderHorizonHist;
	m_HeaderHorizonHist << QString("left down r/g") << QString("left down b/g") << QString("right down r/g") << QString("right down b/g") << QString("right up r/g")
		<< QString("right up b/g") << QString("left up r/g") << QString("left up b/g");
	ui.tableWidgetHist->setHorizontalHeaderLabels(m_HeaderHorizonHist);
	ui.tableWidgetHist->horizontalHeader()->setVisible(true);
	ui.tableWidgetInt->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);

	QStringList m_HeaderVerticalHist;
	m_HeaderVerticalHist << QString("red") << QString("blue") << QString("yellow") << QString("green") << QString("color 1")
		<< QString("color 2") << QString("color 3") << QString("color 4")
;
	ui.tableWidgetHist->setVerticalHeaderLabels(m_HeaderVerticalHist);
	ui.tableWidgetHist->verticalHeader()->setVisible(true);
	ui.tableWidgetInt->verticalHeader()->setSectionResizeMode(QHeaderView::Interactive);


	//2.tableWidgetHist_2
	ui.tableWidgetHist_2->setColumnCount(3);
	ui.tableWidgetHist_2->setRowCount(colorRegionsCount);
	QStringList m_HeaderHorizonHist_2;
	m_HeaderHorizonHist_2 << QString("r/g shift") << QString("b/g shift") << QString("weight shift");
	ui.tableWidgetHist_2->setHorizontalHeaderLabels(m_HeaderHorizonHist_2);
	ui.tableWidgetHist_2->horizontalHeader()->setVisible(true);

	QStringList m_HeaderVerticalHist_2;
	m_HeaderVerticalHist_2 << QString("red") << QString("blue") << QString("yellow") << QString("green") << QString("color 1")
		<< QString("color 2") << QString("color 3") << QString("color 4")
		;
	ui.tableWidgetHist_2->setVerticalHeaderLabels(m_HeaderVerticalHist_2);
	ui.tableWidgetHist_2->verticalHeader()->setVisible(true);

	//3.tableWidgetBVTh
	ui.tableWidgetBVTh->setColumnCount(8);
	ui.tableWidgetBVTh->setRowCount(lightSourceNum);
	QStringList m_HeaderHorizonBV;
	m_HeaderHorizonBV << QString("left down r/g") << QString("left down b/g") << QString("right down r/g") << QString("right down b/g") << QString("right up r/g")
		<< QString("right up b/g") << QString("left up r/g") << QString("left up b/g");
	ui.tableWidgetBVTh->setHorizontalHeaderLabels(m_HeaderHorizonBV);
	ui.tableWidgetBVTh->horizontalHeader()->setVisible(true);
	ui.tableWidgetBVTh->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);

	QStringList m_HeaderVerticalBV;
	m_HeaderVerticalBV << QString("grey region 1") << QString("grey region 2") << QString("grey region 3") << QString("grey region 4") << QString("grey region 5")
		<< QString("grey region 6") << QString("grey region 7") << QString("grey region 8");
	ui.tableWidgetBVTh->setVerticalHeaderLabels(m_HeaderVerticalBV);
	ui.tableWidgetBVTh->verticalHeader()->setVisible(true);
	ui.tableWidgetBVTh->verticalHeader()->setSectionResizeMode(QHeaderView::Interactive);

	//4.tableTarget
	ui.tableTgMode->setColumnCount(sceneCnt);
	ui.tableTgMode->setRowCount(1);
	QStringList m_HeaderHorizonTgMode;
	m_HeaderHorizonTgMode << QString("LV1 weight") << QString("LV2 weight") << QString("LV3 weight") << QString("LV4 weight") << QString("LV5 weight");
	ui.tableTgMode->setHorizontalHeaderLabels(m_HeaderHorizonTgMode);
	ui.tableTgMode->horizontalHeader()->setVisible(true);
	ui.tableTgMode->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);

	ui.tableLV1_cct->setColumnCount(1);
	ui.tableLV1_cct->setRowCount(CCTLevel);

	ui.tableLV2_cct->setColumnCount(1);
	ui.tableLV2_cct->setRowCount(CCTLevel);

	ui.tableLV3_cct->setColumnCount(1);
	ui.tableLV3_cct->setRowCount(CCTLevel);

	ui.tableLV4_cct->setColumnCount(1);
	ui.tableLV4_cct->setRowCount(CCTLevel);

	ui.tableLV5_cct->setColumnCount(1);
	ui.tableLV5_cct->setRowCount(CCTLevel);

	QStringList m_HeaderHorizonTg;
	m_HeaderHorizonTg << QString("weight");
	QStringList m_HeaderVerticalTg;
	m_HeaderVerticalTg << QString("H") << QString("U30") << QString("TL84") << QString("CWF") << QString("D50") << QString("D65") << QString("D75");



	ui.tableLV1_cct->setHorizontalHeaderLabels(m_HeaderHorizonTg);
	ui.tableLV1_cct->horizontalHeader()->setVisible(true);
	ui.tableLV1_cct->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
	ui.tableLV1_cct->setVerticalHeaderLabels(m_HeaderVerticalTg);
	ui.tableLV1_cct->verticalHeader()->setVisible(true);
	ui.tableLV1_cct->verticalHeader()->setSectionResizeMode(QHeaderView::Interactive);

	ui.tableLV2_cct->setHorizontalHeaderLabels(m_HeaderHorizonTg);
	ui.tableLV2_cct->horizontalHeader()->setVisible(true);
	ui.tableLV2_cct->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
	ui.tableLV2_cct->setVerticalHeaderLabels(m_HeaderVerticalTg);
	ui.tableLV2_cct->verticalHeader()->setVisible(true);
	ui.tableLV2_cct->verticalHeader()->setSectionResizeMode(QHeaderView::Interactive);

	ui.tableLV3_cct->setHorizontalHeaderLabels(m_HeaderHorizonTg);
	ui.tableLV3_cct->horizontalHeader()->setVisible(true);
	ui.tableLV3_cct->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
	ui.tableLV3_cct->setVerticalHeaderLabels(m_HeaderVerticalTg);
	ui.tableLV3_cct->verticalHeader()->setVisible(true);
	ui.tableLV3_cct->verticalHeader()->setSectionResizeMode(QHeaderView::Interactive);

	ui.tableLV4_cct->setHorizontalHeaderLabels(m_HeaderHorizonTg);
	ui.tableLV4_cct->horizontalHeader()->setVisible(true);
	ui.tableLV4_cct->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
	ui.tableLV4_cct->setVerticalHeaderLabels(m_HeaderVerticalTg);
	ui.tableLV4_cct->verticalHeader()->setVisible(true);
	ui.tableLV4_cct->verticalHeader()->setSectionResizeMode(QHeaderView::Interactive);

	ui.tableLV5_cct->setHorizontalHeaderLabels(m_HeaderHorizonTg);
	ui.tableLV5_cct->horizontalHeader()->setVisible(true);
	ui.tableLV5_cct->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
	ui.tableLV5_cct->setVerticalHeaderLabels(m_HeaderVerticalTg);
	ui.tableLV5_cct->verticalHeader()->setVisible(true);
	ui.tableLV5_cct->verticalHeader()->setSectionResizeMode(QHeaderView::Interactive);

	//5.tableExpo
	//ui.tableExpoMode->setColumnCount(aeTable_Cnt);
	//ui.tableExpoMode->setRowCount(1);
	//QStringList m_HeaderHorizonExpoMode;
	//m_HeaderHorizonExpoMode << QString("AETABLE_PRECIEW_AUTO") << QString("AETABLE_CAPTURE_AUTO") << QString("AETABLE_CAPTURE_NIGHT") << QString("AETABLE_CAPTURE_HDR");
	//ui.tableExpoMode->setHorizontalHeaderLabels(m_HeaderHorizonExpoMode);
	//ui.tableExpoMode->horizontalHeader()->setVisible(true);
	//ui.tableExpoMode->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);

	ui.tableLV5_cct_2->setColumnCount(8);
	ui.tableLV5_cct_2->setRowCount(misColorRegionsCount);


	QStringList m_HeaderHorizonExpo;
	m_HeaderHorizonExpo << QString("left down r/g") << QString("left down b/g") << QString("right down r/g") << QString("right down b/g") << QString("right up r/g")
		<< QString("right up b/g") << QString("left up r/g") << QString("left up b/g");
	QStringList m_HeaderVerticalExpo;
	m_HeaderVerticalExpo << QString("region 1") << QString("region 2") << QString("region 3") << QString("region 4") << QString("region 5") 
		<< QString("region 6") << QString("region 7") << QString("region 8");

	ui.tableLV5_cct_2->setHorizontalHeaderLabels(m_HeaderHorizonExpo);
	ui.tableLV5_cct_2->horizontalHeader()->setVisible(true);
	ui.tableLV5_cct_2->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
	ui.tableLV5_cct_2->setVerticalHeaderLabels(m_HeaderVerticalExpo);
	ui.tableLV5_cct_2->verticalHeader()->setVisible(true);
	ui.tableLV5_cct_2->verticalHeader()->setSectionResizeMode(QHeaderView::Interactive);

	//ccm
	ui.tableLV1_ccm1->setColumnCount(3);
	ui.tableLV1_ccm1->setRowCount(3);
	QStringList m_HeaderHorizonCCM;
	m_HeaderHorizonCCM << QString("R") << QString("G") << QString("B");
	ui.tableLV1_ccm1->setHorizontalHeaderLabels(m_HeaderHorizonCCM);
	ui.tableLV1_ccm1->horizontalHeader()->setVisible(true);
	ui.tableLV1_ccm1->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);

	QStringList m_HeaderVerticalCCM;
	m_HeaderVerticalCCM << QString("R") << QString("G") << QString("B");
	ui.tableLV1_ccm1->setVerticalHeaderLabels(m_HeaderVerticalCCM);
	ui.tableLV1_ccm1->verticalHeader()->setVisible(true);
	ui.tableLV1_ccm1->verticalHeader()->setSectionResizeMode(QHeaderView::Interactive);

	ui.tableLV2_ccm2->setColumnCount(3);
	ui.tableLV2_ccm2->setRowCount(3);

	ui.tableLV2_ccm2->setHorizontalHeaderLabels(m_HeaderHorizonCCM);
	ui.tableLV2_ccm2->horizontalHeader()->setVisible(true);
	ui.tableLV2_ccm2->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);

	ui.tableLV2_ccm2->setVerticalHeaderLabels(m_HeaderVerticalCCM);
	ui.tableLV2_ccm2->verticalHeader()->setVisible(true);
	ui.tableLV2_ccm2->verticalHeader()->setSectionResizeMode(QHeaderView::Interactive);

	ui.tableLV3_ccm3->setColumnCount(3);
	ui.tableLV3_ccm3->setRowCount(3);

	ui.tableLV3_ccm3->setHorizontalHeaderLabels(m_HeaderHorizonCCM);
	ui.tableLV3_ccm3->horizontalHeader()->setVisible(true);
	ui.tableLV3_ccm3->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);

	ui.tableLV3_ccm3->setVerticalHeaderLabels(m_HeaderVerticalCCM);
	ui.tableLV3_ccm3->verticalHeader()->setVisible(true);
	ui.tableLV3_ccm3->verticalHeader()->setSectionResizeMode(QHeaderView::Interactive);

	ui.tableLV4_ccm4->setColumnCount(3);
	ui.tableLV4_ccm4->setRowCount(3);

	ui.tableLV4_ccm4->setHorizontalHeaderLabels(m_HeaderHorizonCCM);
	ui.tableLV4_ccm4->horizontalHeader()->setVisible(true);
	ui.tableLV4_ccm4->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);

	ui.tableLV4_ccm4->setVerticalHeaderLabels(m_HeaderVerticalCCM);
	ui.tableLV4_ccm4->verticalHeader()->setVisible(true);
	ui.tableLV4_ccm4->verticalHeader()->setSectionResizeMode(QHeaderView::Interactive);

	ui.tableLV5_cct_3->setColumnCount(3);
	ui.tableLV5_cct_3->setRowCount(3);

	ui.tableLV5_cct_3->setHorizontalHeaderLabels(m_HeaderHorizonCCM);
	ui.tableLV5_cct_3->horizontalHeader()->setVisible(true);
	ui.tableLV5_cct_3->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);

	ui.tableLV5_cct_3->setVerticalHeaderLabels(m_HeaderVerticalCCM);
	ui.tableLV5_cct_3->verticalHeader()->setVisible(true);
	ui.tableLV5_cct_3->verticalHeader()->setSectionResizeMode(QHeaderView::Interactive);

	//ccm
	ui.tableLV1_shift_rg->setColumnCount(shiftLevel);
	ui.tableLV1_shift_rg->setRowCount(shiftLevel);
	QStringList m_HeaderHorizonShift;
	m_HeaderHorizonShift << QString("0.2") << QString("0.25") << QString("0.3") << QString("0.35") << QString("0.4") << QString("0.45") << QString("0.5")
		<< QString("0.55") << QString("0.6") << QString("0.65") << QString("0.7") << QString("0.75") << QString("0.8");
	ui.tableLV1_shift_rg->setHorizontalHeaderLabels(m_HeaderHorizonShift);
	ui.tableLV1_shift_rg->horizontalHeader()->setVisible(true);
	ui.tableLV1_shift_rg->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);

	QStringList m_HeaderVerticalShift;
	m_HeaderVerticalShift << QString("0.2") << QString("0.25") << QString("0.3") << QString("0.35") << QString("0.4") << QString("0.45") << QString("0.5")
		<< QString("0.55") << QString("0.6") << QString("0.65") << QString("0.7") << QString("0.75") << QString("0.8");
	ui.tableLV1_shift_rg->setVerticalHeaderLabels(m_HeaderVerticalShift);
	ui.tableLV1_shift_rg->verticalHeader()->setVisible(true);
	ui.tableLV1_shift_rg->verticalHeader()->setSectionResizeMode(QHeaderView::Interactive);

	ui.tableLV1_shift_bg->setColumnCount(shiftLevel);
	ui.tableLV1_shift_bg->setRowCount(shiftLevel);

	ui.tableLV1_shift_bg->setHorizontalHeaderLabels(m_HeaderHorizonShift);
	ui.tableLV1_shift_bg->horizontalHeader()->setVisible(true);
	ui.tableLV1_shift_bg->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);

	ui.tableLV1_shift_bg->setVerticalHeaderLabels(m_HeaderVerticalShift);
	ui.tableLV1_shift_bg->verticalHeader()->setVisible(true);
	ui.tableLV1_shift_bg->verticalHeader()->setSectionResizeMode(QHeaderView::Interactive);

	ui.tableLV2_shift_bg->setColumnCount(shiftLevel);
	ui.tableLV2_shift_bg->setRowCount(shiftLevel);

	ui.tableLV2_shift_bg->setHorizontalHeaderLabels(m_HeaderHorizonShift);
	ui.tableLV2_shift_bg->horizontalHeader()->setVisible(true);
	ui.tableLV2_shift_bg->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);

	ui.tableLV2_shift_bg->setVerticalHeaderLabels(m_HeaderVerticalShift);
	ui.tableLV2_shift_bg->verticalHeader()->setVisible(true);
	ui.tableLV2_shift_bg->verticalHeader()->setSectionResizeMode(QHeaderView::Interactive);

	ui.tableLV2_shift_rg->setColumnCount(shiftLevel);
	ui.tableLV2_shift_rg->setRowCount(shiftLevel);

	ui.tableLV2_shift_rg->setHorizontalHeaderLabels(m_HeaderHorizonShift);
	ui.tableLV2_shift_rg->horizontalHeader()->setVisible(true);
	ui.tableLV2_shift_rg->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);

	ui.tableLV2_shift_rg->setVerticalHeaderLabels(m_HeaderVerticalShift);
	ui.tableLV2_shift_rg->verticalHeader()->setVisible(true);
	ui.tableLV2_shift_rg->verticalHeader()->setSectionResizeMode(QHeaderView::Interactive);


	ui.tableLV3_shift_rg->setColumnCount(shiftLevel);
	ui.tableLV3_shift_rg->setRowCount(shiftLevel);

	ui.tableLV3_shift_rg->setHorizontalHeaderLabels(m_HeaderHorizonShift);
	ui.tableLV3_shift_rg->horizontalHeader()->setVisible(true);
	ui.tableLV3_shift_rg->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);

	ui.tableLV3_shift_rg->setVerticalHeaderLabels(m_HeaderVerticalShift);
	ui.tableLV3_shift_rg->verticalHeader()->setVisible(true);
	ui.tableLV3_shift_rg->verticalHeader()->setSectionResizeMode(QHeaderView::Interactive);

	ui.tableLV3_shift_bg->setColumnCount(shiftLevel);
	ui.tableLV3_shift_bg->setRowCount(shiftLevel);

	ui.tableLV3_shift_bg->setHorizontalHeaderLabels(m_HeaderHorizonShift);
	ui.tableLV3_shift_bg->horizontalHeader()->setVisible(true);
	ui.tableLV3_shift_bg->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);

	ui.tableLV3_shift_bg->setVerticalHeaderLabels(m_HeaderVerticalShift);
	ui.tableLV3_shift_bg->verticalHeader()->setVisible(true);
	ui.tableLV3_shift_bg->verticalHeader()->setSectionResizeMode(QHeaderView::Interactive);

	ui.tableLV4_shift_rg->setColumnCount(shiftLevel);
	ui.tableLV4_shift_rg->setRowCount(shiftLevel);

	ui.tableLV4_shift_rg->setHorizontalHeaderLabels(m_HeaderHorizonShift);
	ui.tableLV4_shift_rg->horizontalHeader()->setVisible(true);
	ui.tableLV4_shift_rg->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);

	ui.tableLV4_shift_rg->setVerticalHeaderLabels(m_HeaderVerticalShift);
	ui.tableLV4_shift_rg->verticalHeader()->setVisible(true);
	ui.tableLV4_shift_rg->verticalHeader()->setSectionResizeMode(QHeaderView::Interactive);

	ui.tableLV4_shift_bg->setColumnCount(shiftLevel);
	ui.tableLV4_shift_bg->setRowCount(shiftLevel);

	ui.tableLV4_shift_bg->setHorizontalHeaderLabels(m_HeaderHorizonShift);
	ui.tableLV4_shift_bg->horizontalHeader()->setVisible(true);
	ui.tableLV4_shift_bg->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);

	ui.tableLV4_shift_bg->setVerticalHeaderLabels(m_HeaderVerticalShift);
	ui.tableLV4_shift_bg->verticalHeader()->setVisible(true);
	ui.tableLV4_shift_bg->verticalHeader()->setSectionResizeMode(QHeaderView::Interactive);

	ui.tableLV5_shift_rg->setColumnCount(shiftLevel);
	ui.tableLV5_shift_rg->setRowCount(shiftLevel);

	ui.tableLV5_shift_rg->setHorizontalHeaderLabels(m_HeaderHorizonShift);
	ui.tableLV5_shift_rg->horizontalHeader()->setVisible(true);
	ui.tableLV5_shift_rg->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);

	ui.tableLV5_shift_rg->setVerticalHeaderLabels(m_HeaderVerticalShift);
	ui.tableLV5_shift_rg->verticalHeader()->setVisible(true);
	ui.tableLV5_shift_rg->verticalHeader()->setSectionResizeMode(QHeaderView::Interactive);

	ui.tableLV5_shift_bg->setColumnCount(shiftLevel);
	ui.tableLV5_shift_bg->setRowCount(shiftLevel);

	ui.tableLV5_shift_bg->setHorizontalHeaderLabels(m_HeaderHorizonShift);
	ui.tableLV5_shift_bg->horizontalHeader()->setVisible(true);
	ui.tableLV5_shift_bg->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);

	ui.tableLV5_shift_bg->setVerticalHeaderLabels(m_HeaderVerticalShift);
	ui.tableLV5_shift_bg->verticalHeader()->setVisible(true);
	ui.tableLV5_shift_bg->verticalHeader()->setSectionResizeMode(QHeaderView::Interactive);
}

//void QtWidgetsApplication1::on_pushButton_updateraw_clicked()
//{
//    QString filename = QFileDialog::getOpenFileName(this, tr("open image"), QDir::homePath(), tr("(*.raw)\n(*.jpg)"));
//    qDebug() << "filename:" << filename;
//    QImage image = QImage(filename);
//    qDebug() << "image:" << image;
//    ui.statusBar->showMessage(tr("open success"), 3000);
//    ui.label->setPixmap(QPixmap::fromImage(image));
//    todo: loadrawparam;
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

void QtWidgetsApplication1::on_pushButton_loadParam_clicked()
{
	string dllAddr = GetCurrMoudle();
	string txtAddr;
	printf("dllAddr %s\n", dllAddr.c_str());
	//txtAddr = dllAddr + "\\meteringParam.txt";
	txtAddr = ".\\AWBParam.bin";

	fstream fin;
	fin.open(txtAddr, ios::binary | ios::in);

	if (!fin.is_open())
	{
		QMessageBox::information(this, tr("warning"), tr("open AWBParam.bin failed!!!"), QMessageBox::Yes);
		return;
	}
	parseParam param;
	fin.read((char*)&param, sizeof(param));
	//param.cctWeightTable[0][2] = 888;

	//for (int j = 0; j < lightSourceNum; j++)
	//{
	//	ui.tableWidgetInt->setItem(j, 0, new QTableWidgetItem(QString::number(param.lightSourceTable[j][0])));
	//	
	//}

	for (int i = 0; i < lightSourceNum; i++)
	{
		ui.tableWidgetInt->setItem(i, 0, new QTableWidgetItem(QString::number(param.lightSourceTable[i][0])));
		ui.tableWidgetInt->setItem(i, 1, new QTableWidgetItem(QString::number(param.lightSourceTable[i][1])));
		ui.tableWidgetInt->setItem(i, 2, new QTableWidgetItem(QString::number(param.lightSourceShift[i][0])));
		ui.tableWidgetInt->setItem(i, 3, new QTableWidgetItem(QString::number(param.lightSourceShift[i][1])));

	}

	//for (int i = 0; i < lightSourceNum; i++)
	//{
	//	for (int j = 2; j < 4; j++)
	//	{
	//		ui.tableWidgetInt->setItem(i, j, new QTableWidgetItem(QString::number(param.lightSourceShift[i][j])));
	//	}
	//}

	for (int j = 0; j < CCTLevel; j++)
	{
		ui.tableLV1_cct->setItem(j, 0, new QTableWidgetItem(QString::number(param.cctWeightTable[0][j])));
		ui.tableLV2_cct->setItem(j, 0, new QTableWidgetItem(QString::number(param.cctWeightTable[1][j])));
		ui.tableLV3_cct->setItem(j, 0, new QTableWidgetItem(QString::number(param.cctWeightTable[2][j])));
		ui.tableLV4_cct->setItem(j, 0, new QTableWidgetItem(QString::number(param.cctWeightTable[3][j])));
		ui.tableLV5_cct->setItem(j, 0, new QTableWidgetItem(QString::number(param.cctWeightTable[4][j])));
	}

	for (int j = 0; j < LightLevel; j++)
	{
		ui.tableTgMode->setItem(0, j, new QTableWidgetItem(QString::number(param.lvWeight[j])));
	}
	
	for (int i = 0; i < lightSourceNum; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			ui.tableWidgetBVTh->setItem(i, j, new QTableWidgetItem(QString::number(param.greyRegions[i][j])));
		}
	}

	for (int i = 0; i < colorRegionsCount; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			ui.tableWidgetHist->setItem(i, j, new QTableWidgetItem(QString::number(param.colorRegions[i][j])));
		}
	}

	for (int i = 0; i < misColorRegionsCount; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			ui.tableLV5_cct_2->setItem(i, j, new QTableWidgetItem(QString::number(param.misColorRegions[i][j])));
		}
	}

	for (int i = 0; i < colorRegionsCount; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			ui.tableWidgetHist_2->setItem(i, j, new QTableWidgetItem(QString::number(param.colorRegionsParams[i][j])));
		}
	}

	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			ui.tableLV1_ccm1->setItem(i, j, new QTableWidgetItem(QString::number(param.ccmParams[0][i * 3 + j])));
			ui.tableLV2_ccm2->setItem(i, j, new QTableWidgetItem(QString::number(param.ccmParams[1][i * 3 + j])));
			ui.tableLV3_ccm3->setItem(i, j, new QTableWidgetItem(QString::number(param.ccmParams[2][i * 3 + j])));
			ui.tableLV4_ccm4->setItem(i, j, new QTableWidgetItem(QString::number(param.ccmParams[3][i * 3 + j])));
			ui.tableLV5_cct_3->setItem(i, j, new QTableWidgetItem(QString::number(param.ccmParams[4][i * 3 + j])));

		}
	}


	for (int i = 0; i < shiftLevel; i++)
	{
		for (int j = 0; j < shiftLevel; j++)
		{
			ui.tableLV1_shift_rg->setItem(i, j, new QTableWidgetItem(QString::number(param.shiftRG[0][i][j])));
			ui.tableLV2_shift_rg->setItem(i, j, new QTableWidgetItem(QString::number(param.shiftRG[1][i][j])));
			ui.tableLV3_shift_rg->setItem(i, j, new QTableWidgetItem(QString::number(param.shiftRG[2][i][j])));
			ui.tableLV4_shift_rg->setItem(i, j, new QTableWidgetItem(QString::number(param.shiftRG[3][i][j])));
			ui.tableLV5_shift_rg->setItem(i, j, new QTableWidgetItem(QString::number(param.shiftRG[4][i][j])));

			ui.tableLV1_shift_bg->setItem(i, j, new QTableWidgetItem(QString::number(param.shiftBG[0][i][j])));
			ui.tableLV2_shift_bg->setItem(i, j, new QTableWidgetItem(QString::number(param.shiftBG[1][i][j])));
			ui.tableLV3_shift_bg->setItem(i, j, new QTableWidgetItem(QString::number(param.shiftBG[2][i][j])));
			ui.tableLV4_shift_bg->setItem(i, j, new QTableWidgetItem(QString::number(param.shiftBG[3][i][j])));
			ui.tableLV5_shift_bg->setItem(i, j, new QTableWidgetItem(QString::number(param.shiftBG[4][i][j])));
		}
	}

	fin.close();
    return;
}


void QtWidgetsApplication1::on_pushButton_saveParam_clicked()
{
	string dllAddr = GetCurrMoudle();
	string txtAddr;
	printf("dllAddr11 %s\n", dllAddr.c_str());
	//txtAddr = dllAddr + "\\meteringParam.txt";
	txtAddr = ".\\AWBParam.bin";

	fstream fout;
	fout.open(txtAddr, ios::binary | ios::out | ios::in);
	if (!fout.is_open())
	{
		QMessageBox::information(this, tr("warning"), tr("open AWBParam.bin failed"), QMessageBox::Yes);
		return;
	}
	parseParam param;


	for (int i = 0; i < lightSourceNum; i++)
	{
		for (int j = 0; j < 2; j++)
		{
			QString rowString;
			rowString = ui.tableWidgetInt->item(i, j)->text();
			param.lightSourceTable[i][j] = rowString.toInt();
			//printf("%s", param.cctWeightTable[i][j]);
			fout.write((char*)&param.lightSourceTable[i][j], sizeof(param.lightSourceTable[i][j]));
		}
	}


	for (int i = 0; i < lightSourceNum; i++)
	{
		for (int j = 2; j < 4; j++)
		{
			QString rowString;
			rowString = ui.tableWidgetInt->item(i, j)->text();
			param.lightSourceShift[i][j] = rowString.toInt();
			//printf("%s", param.cctWeightTable[i][j]);
			fout.write((char*)&param.lightSourceShift[i][j], sizeof(param.lightSourceShift[i][j]));
		}
	}

	for (int i = 0; i < LightLevel ; i++)
	{
		for (int j = 0; j < CCTLevel; j++)
		{
			QString rowString;
			switch (i)
			{
				case 0:
					rowString = ui.tableLV1_cct->item(j, 0)->text();
					break;
				case 1:
					rowString = ui.tableLV2_cct->item(j, 0)->text();
					break;
				case 2:
					rowString = ui.tableLV3_cct->item(j, 0)->text();
					break;
				case 3:
					rowString = ui.tableLV4_cct->item(j, 0)->text();
					break;
				case 4:
					rowString = ui.tableLV5_cct->item(j, 0)->text();
					break;
				default:
					rowString = ui.tableLV1_cct->item(j, 0)->text();
					break;
			}
			param.cctWeightTable[i][j] = rowString.toInt();
			//printf("%s", param.cctWeightTable[i][j]);
			fout.write((char*)&param.cctWeightTable[i][j], sizeof(param.cctWeightTable[i][j]));
		}
	}

	for (int j = 0; j < LightLevel; j++)
	{
		QString rowString;
		rowString = ui.tableTgMode->item(0, j)->text();
		param.lvWeight[j] = rowString.toInt();
		fout.write((char*)&param.lvWeight[j], sizeof(param.lvWeight[j]));
	}

	for (int i = 0; i < lightSourceNum; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			QString rowString;
			rowString = ui.tableWidgetBVTh->item(i, j)->text();
			param.greyRegions[i][j] = rowString.toInt();
			fout.write((char*)&param.greyRegions[i][j], sizeof(param.greyRegions[i][j]));
		}
	}

	for (int i = 0; i < colorRegionsCount; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			QString rowString;
			rowString = ui.tableWidgetHist->item(i, j)->text();
			param.colorRegions[i][j] = rowString.toInt();
			fout.write((char*)&param.colorRegions[i][j], sizeof(param.colorRegions[i][j]));
		}
	}

	for (int i = 0; i < misColorRegionsCount; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			QString rowString;
			rowString = ui.tableLV5_cct_2->item(i, j)->text();
			param.misColorRegions[i][j] = rowString.toInt();
			fout.write((char*)&param.misColorRegions[i][j], sizeof(param.misColorRegions[i][j]));
		}
	}

	for (int i = 0; i < colorRegionsCount; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			QString rowString;
			rowString = ui.tableWidgetHist_2->item(i, j)->text();
			param.colorRegionsParams[i][j] = rowString.toInt();
			fout.write((char*)&param.colorRegionsParams[i][j], sizeof(param.colorRegionsParams[i][j]));
		}
	}

	//ccm
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			QString rowString;
			rowString = ui.tableLV1_ccm1->item(i, j)->text();
			param.ccmParams[0][i * 3 + j] = rowString.toInt();
			fout.write((char*)&param.ccmParams[0][i * 3 + j], sizeof(param.ccmParams[0][i * 3 + j]));
		}
	}

	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			QString rowString;
			rowString = ui.tableLV2_ccm2->item(i, j)->text();
			param.ccmParams[1][i * 3 + j] = rowString.toInt();
			fout.write((char*)&param.ccmParams[1][i * 3 + j], sizeof(param.ccmParams[1][i * 3 + j]));
		}
	}
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			QString rowString;
			rowString = ui.tableLV3_ccm3->item(i, j)->text();
			param.ccmParams[2][i * 3 + j] = rowString.toInt();
			fout.write((char*)&param.ccmParams[2][i * 3 + j], sizeof(param.ccmParams[2][i * 3 + j]));
		}
	}
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			QString rowString;
			rowString = ui.tableLV4_ccm4->item(i, j)->text();
			param.ccmParams[3][i * 3 + j] = rowString.toInt();
			fout.write((char*)&param.ccmParams[3][i * 3 + j], sizeof(param.ccmParams[3][i * 3 + j]));
		}
	}
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			QString rowString;
			rowString = ui.tableLV5_cct_3->item(i, j)->text();
			param.ccmParams[4][i * 3 + j] = rowString.toInt();
			fout.write((char*)&param.ccmParams[4][i * 3 + j], sizeof(param.ccmParams[4][i * 3 + j]));
		}
	}


	//倾向性调整RG
	for (int i = 0; i < shiftLevel; i++)
	{
		for (int j = 0; j < shiftLevel; j++)
		{
			QString rowString;
			rowString = ui.tableLV1_shift_rg->item(i, j)->text();
			param.shiftRG[0][i][j] = rowString.toInt();
			fout.write((char*)&param.shiftRG[0][i][j], sizeof(param.shiftRG[0][i][j]));
		}
	}

	for (int i = 0; i < shiftLevel; i++)
	{
		for (int j = 0; j < shiftLevel; j++)
		{
			QString rowString;
			rowString = ui.tableLV2_shift_rg->item(i, j)->text();
			param.shiftRG[1][i][j] = rowString.toInt();
			fout.write((char*)&param.shiftRG[1][i][j], sizeof(param.shiftRG[1][i][j]));
		}
	}

	for (int i = 0; i < shiftLevel; i++)
	{
		for (int j = 0; j < shiftLevel; j++)
		{
			QString rowString;
			rowString = ui.tableLV3_shift_rg->item(i, j)->text();
			param.shiftRG[2][i][j] = rowString.toInt();
			fout.write((char*)&param.shiftRG[2][i][j], sizeof(param.shiftRG[2][i][j]));
		}
	}

	for (int i = 0; i < shiftLevel; i++)
	{
		for (int j = 0; j < shiftLevel; j++)
		{
			QString rowString;
			rowString = ui.tableLV4_shift_rg->item(i, j)->text();
			param.shiftRG[3][i][j] = rowString.toInt();
			fout.write((char*)&param.shiftRG[3][i][j], sizeof(param.shiftRG[3][i][j]));
		}
	}

	for (int i = 0; i < shiftLevel; i++)
	{
		for (int j = 0; j < shiftLevel; j++)
		{
			QString rowString;
			rowString = ui.tableLV5_shift_rg->item(i, j)->text();
			param.shiftRG[4][i][j] = rowString.toInt();
			fout.write((char*)&param.shiftRG[4][i][j], sizeof(param.shiftRG[4][i][j]));
		}
	}

	//倾向性调整BG
	for (int i = 0; i < shiftLevel; i++)
	{
		for (int j = 0; j < shiftLevel; j++)
		{
			QString rowString;
			rowString = ui.tableLV1_shift_bg->item(i, j)->text();
			param.shiftBG[0][i][j] = rowString.toInt();
			fout.write((char*)&param.shiftBG[0][i][j], sizeof(param.shiftBG[0][i][j]));
		}
	}

	for (int i = 0; i < shiftLevel; i++)
	{
		for (int j = 0; j < shiftLevel; j++)
		{
			QString rowString;
			rowString = ui.tableLV2_shift_bg->item(i, j)->text();
			param.shiftBG[1][i][j] = rowString.toInt();
			fout.write((char*)&param.shiftBG[1][i][j], sizeof(param.shiftBG[1][i][j]));
		}
	}

	for (int i = 0; i < shiftLevel; i++)
	{
		for (int j = 0; j < shiftLevel; j++)
		{
			QString rowString;
			rowString = ui.tableLV3_shift_bg->item(i, j)->text();
			param.shiftBG[2][i][j] = rowString.toInt();
			fout.write((char*)&param.shiftBG[2][i][j], sizeof(param.shiftBG[2][i][j]));
		}
	}

	for (int i = 0; i < shiftLevel; i++)
	{
		for (int j = 0; j < shiftLevel; j++)
		{
			QString rowString;
			rowString = ui.tableLV4_shift_bg->item(i, j)->text();
			param.shiftBG[3][i][j] = rowString.toInt();
			fout.write((char*)&param.shiftBG[3][i][j], sizeof(param.shiftBG[3][i][j]));
		}
	}

	for (int i = 0; i < shiftLevel; i++)
	{
		for (int j = 0; j < shiftLevel; j++)
		{
			QString rowString;
			rowString = ui.tableLV5_shift_bg->item(i, j)->text();
			param.shiftBG[4][i][j] = rowString.toInt();
			fout.write((char*)&param.shiftBG[4][i][j], sizeof(param.shiftBG[4][i][j]));
		}
	}

	for (int i = 0; i < 3; i++)
	{
		param.DEBUG_LEVEL[i] = 1;
		fout.write((char*)&param.DEBUG_LEVEL[i], sizeof(param.DEBUG_LEVEL[i]));
	}

	fout.close();
    return;
}

void QtWidgetsApplication1::closeEvent(QCloseEvent *e)
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


QtWidgetsApplication1::~QtWidgetsApplication1()
{}
