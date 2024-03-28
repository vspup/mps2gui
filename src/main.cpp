// gui mps2
// dsi
// git v 0.3s

#include "inc/mainwindow.h"

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

#include <QFile>
#include <QTextStream>


#include "inc/chart.h"
#include "inc/chartview.h"

#include <QApplication>
#include <QtCharts/QLineSeries>
#include <QMainWindow>
#include <QRandomGenerator>
#include <QtMath>
#include <QtCharts/QValueAxis>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();


// ------ TEST CHART -------
//    QApplication a(argc, argv);
//
//    auto series = new QLineSeries;
//    for (int i = 0; i < 500; i++) {
//        QPointF p((qreal) i, qSin(M_PI / 50 * i) * 100);
//        p.ry() += QRandomGenerator::global()->bounded(20);
//        *series << p;
//    }
//
//    auto chart = new Chart;
//    chart->addSeries(series);
//    chart->setTitle("Zoom in/out example");
//    chart->setAnimationOptions(QChart::SeriesAnimations);
//    chart->legend()->hide();
//    chart->createDefaultAxes();
//
//    auto chartView = new ChartView(chart);
//    chartView->setRenderHint(QPainter::Antialiasing);
//
//    QMainWindow window;
//    window.setCentralWidget(chartView);
//    window.resize(400, 300);
//    window.grabGesture(Qt::PanGesture);
//    window.grabGesture(Qt::PinchGesture);
//    window.show();
//
//    return a.exec();
}

