// gui mps2
// dsi
// git v 0.3s

#include "inc/mainwindow.h"
#include "inc/selectwindow.h"

#include <QApplication>

#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <ctype.h>
#include <QTextStream>

#include <QFile>
#include <QTextStream>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    //MainWindow w;
    SelectWindow w;
    w.show();

    return a.exec();
}

