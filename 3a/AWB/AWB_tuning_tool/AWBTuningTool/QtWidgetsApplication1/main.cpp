#include "stdafx.h"
#include <QCloseEvent>
#include "qtwidgetsapplication1.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QtWidgetsApplication1 w;
    w.show();
    return a.exec();
}


///
///