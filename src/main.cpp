//#include "inc/mainwindow.h"
#include "inc/mpswizard.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    //MainWindow w;
    MPSWizard w;
    w.resize(400, 300);
    w.show();

    return a.exec();
}

