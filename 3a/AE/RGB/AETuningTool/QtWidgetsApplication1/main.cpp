#include "stdafx.h"
#include <QCloseEvent>
#include "qtwidgetsapplication1.h"
#include <QtWidgets/QApplication>
#include "AETuningParamDefalut.h"

void generateTuningBin()
{
	//string dllAddr = GetCurrMoudle();
	string txtAddr;
	//printf("dllAddr11 %s\n", dllAddr.c_str());
	txtAddr = "D:\\3aalgo\\3a-tmp\\3a\\AE\\RGB\\AETuningTool\\QtWidgetsApplication1\\x64\\Release\\AEParam.bin";

    printf("txtAddr %s\n", txtAddr.c_str());
	fstream fout;
	fout.open(txtAddr, ios::binary | ios::out | ios::in);

	fout.write((char*)&defaultParam, sizeof(defaultParam));
	fout.close();
	return;
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QtWidgetsApplication1 w;
    
    //generateTuningBin();

    w.show();

    
    return a.exec();
}


