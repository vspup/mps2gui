#include <QDebug>
#include <QWizard>

#include "inc/selectwindow.h"
#include "ui_selectwindow.h"
#include "inc/menu.h"


SelectWindow::SelectWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::SelectWindow)
{
    ui->setupUi(this);

    mps_menu_state_t start_menu = MPS_MENU_CONNECT_PSU;

    ui->stackedWidget->setCurrentIndex(start_menu);
}

SelectWindow::~SelectWindow()
{
    delete ui;
}

void SelectWindow::on_btConnect_clicked()
{
    qInfo() << "on_btConnect_clicked";
}

