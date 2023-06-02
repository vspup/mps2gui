// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QtWidgets>
#include <QDebug>
#include "inc/mpswizard.h"
#include "inc/mainwindow.h"

QString emailRegExp = QStringLiteral(".+@.+");

MPSWizard::MPSWizard(QWidget *parent)
    : QWizard(parent)
{
    setPage(PAGE_INIT_E, new InitPage);
    setPage(PAGE_CONNECT_E, new ConnectPage);
    setPage(PAGE_SEL_TOMO_E, new SelectTomoPage);
    setPage(PAGE_PROCESS_E, new ProcessPage);
    setPage(PAGE_FINAL_E, new FinalPage);

    setStartId(PAGE_INIT_E);

    setWizardStyle(ModernStyle);

    setOption(HaveHelpButton, true);
    setPixmap(QWizard::LogoPixmap, QPixmap(":/images/logo.png"));

    connect(this, &QWizard::helpRequested, this, &MPSWizard::showHelp);

    setWindowTitle(tr("MPS"));
}

void MPSWizard::showHelp()
{
    static QString lastHelpMessage;

    QString message;

    switch (currentId()) {
    case PAGE_INIT_E:
        message = tr("Init page, go further");
        break;

    case PAGE_CONNECT_E:
        message = tr("Connect here by IP");
        break;
    case PAGE_SEL_TOMO_E:
        message = tr("Select tomograph here");
        break;
    case PAGE_PROCESS_E:
        message = tr("Main process of soft is here");
        break;
    case PAGE_FINAL_E:
        message = tr("Last page in sequence");
        break;

    default:
        message = tr("Default");
    }

    if (lastHelpMessage == message)
        message = tr("Help pressed twice");

    QMessageBox::information(this, tr("MPS Help"), message);

    lastHelpMessage = message;
}

InitPage::InitPage(QWidget *parent)
    : QWizardPage(parent)
{
    setTitle(tr("Init Page Title"));
    setPixmap(QWizard::WatermarkPixmap, QPixmap(":/images/watermark.png"));

    topLabel = new QLabel(tr("This software will help you configure the product"));
    topLabel->setWordWrap(true);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(topLabel);
    setLayout(layout);
}

int InitPage::nextId() const
{
    return MPSWizard::PAGE_CONNECT_E;
}

ConnectPage::ConnectPage(QWidget *parent)
    : QWizardPage(parent)
{
    setTitle(tr("Connection"));
    setSubTitle(tr("Select the IP you want to connect to."));

    connectLabel = new QLabel(tr("Connect to PSU"));
    ipComBox = new QComboBox;

    ipComBox->addItem("192.168.0.101");
    ipComBox->addItem("192.168.0.102");
    ipComBox->addItem("192.168.0.103");

    connectButton = new QPushButton(tr("Select IP"), this);
    connect(connectButton, &QPushButton::clicked, this, &ConnectPage::handleButtonClicked);

    QGridLayout *layout = new QGridLayout;
    layout->addWidget(ipComBox, 1, 1);
    layout->addWidget(connectLabel, 1, 0);
    layout->addWidget(connectButton, 2, 1);
    layout->setAlignment(Qt::AlignCenter);
    setLayout(layout);
}

void ConnectPage::handleButtonClicked()
{
    QString ipAddr = ipComBox->currentText();
    qInfo() << "Selected IP: " << ipAddr;
}

int ConnectPage::nextId() const
{
    return MPSWizard::PAGE_SEL_TOMO_E;
}

SelectTomoPage::SelectTomoPage(QWidget *parent)
    : QWizardPage(parent)
{
    setTitle(tr("Device selection"));
    setSubTitle(tr("Here you can select manufacturer and model of tomograph"));

    manufLabel = new QLabel(tr("Manufacturer:"));
    manufComBox = new QComboBox;
    manufComBox->addItem("Manufacturer 1");
    manufComBox->addItem("Manufacturer 2");
    manufComBox->addItem("Manufacturer 3");

    modelLabel = new QLabel(tr("Model:"));
    modelComBox = new QComboBox;
    modelComBox->addItem("Model 1");
    modelComBox->addItem("Model 2");

    pictureLabel = new QLabel;
    updatePicture();

    connect(manufComBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SelectTomoPage::updatePicture);
    connect(modelComBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SelectTomoPage::updatePicture);

    printButton = new QPushButton(tr("Select"), this);
    connect(printButton, &QPushButton::clicked, this, &SelectTomoPage::handlePrintButtonClicked);

    QGridLayout *layout = new QGridLayout;
    layout->addWidget(manufLabel, 0, 0);
    layout->addWidget(manufComBox, 0, 1);
    layout->addWidget(modelLabel, 1, 0);
    layout->addWidget(modelComBox, 1, 1);
    layout->addWidget(pictureLabel, 0, 0, 3, 1); // 0, 2, 3, 1 - at right
    layout->addWidget(printButton, 4, 0, 1, 3);
    setLayout(layout);
}

void SelectTomoPage::updatePicture() {
    QString picturePath;
    QSize size(200, 200);

    switch (manufComBox->currentIndex()) {
    case MANUF_1:
        switch (modelComBox->currentIndex()) {
        case MODEL_1:
            picturePath = ":/images/tomograph/01_cardiographe.jpg";
            break;
        case MODEL_2:
            picturePath = ":/images/tomograph/02_trgen_3.jpg";
            break;
        default:
            break;
        }
        break;
    case MANUF_2:
        switch (modelComBox->currentIndex()) {
        case MODEL_1:
            picturePath = ":/images/tomograph/03_revolution_frontier.jpg";
            break;
        case MODEL_2:
            picturePath = ":/images/tomograph/04_apex";
            break;
        default:
            break;
        }
        break;
    case MANUF_3:
        switch (modelComBox->currentIndex()) {
        case MODEL_1:
            picturePath = ":/images/tomograph/05_ascend";
            break;
        case MODEL_2:
            picturePath = ":/images/tomograph/06_maxima";
            break;
        default:
            break;
        }
        break;
    }

    if (!picturePath.isEmpty()) {
        QPixmap pixmap(picturePath);
        QPixmap scaledPixmap = pixmap.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        pictureLabel->setPixmap(scaledPixmap);
        pictureLabel->setFixedSize(scaledPixmap.size());
    } else {
        pictureLabel->clear();
    }
}

void SelectTomoPage::handlePrintButtonClicked()
{
    QString manufacturer = manufComBox->currentText();
    QString model = modelComBox->currentText();

    qInfo() << "Manufacturer: " << manufacturer;
    qInfo() << "Selected Model: " << model;
}

int SelectTomoPage::nextId() const
{
    return MPSWizard::PAGE_PROCESS_E;
}

ProcessPage::ProcessPage(QWidget *parent)
    : QWizardPage(parent)
{
    setTitle(tr("Process of MPS"));
    setSubTitle(tr("Most of work should be done here"));

    workButton = new QPushButton(tr("Dummy"), this);
    workButton->setFixedSize(80, 30);

    QPushButton *runOld = new QPushButton(tr("Run old UI"), this);
    runOld->setFixedSize(80, 30);

    connect(runOld, &QPushButton::clicked, this, &ProcessPage::handleRunOldUIButtonClicked);

    QGridLayout *layout = new QGridLayout;
    layout->addWidget(workButton, 0, 0);
    layout->addWidget(runOld, 0, 1);
    layout->setAlignment(Qt::AlignCenter);
    setLayout(layout);
}

void ProcessPage::handleRunOldUIButtonClicked()
{
    MainWindow *w = new MainWindow;
    w->show();
    close();
}

int ProcessPage::nextId() const
{
    return MPSWizard::PAGE_FINAL_E;
}

FinalPage::FinalPage(QWidget *parent)
    : QWizardPage(parent)
{
    setTitle(tr("Final Page"));
    setPixmap(QWizard::WatermarkPixmap, QPixmap(":/images/watermark.png"));

    bottomLabel = new QLabel;
    bottomLabel->setWordWrap(true);

    agreeCheckBox = new QCheckBox(tr("Select this to exit from MPS"));

    registerField("checked*", agreeCheckBox);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(bottomLabel);
    layout->addWidget(agreeCheckBox);
    setLayout(layout);
}

int FinalPage::nextId() const
{
    return -1;
}

void FinalPage::initializePage()
{
    QString text;
    text = tr("Visited: ");

    if (wizard()->hasVisitedPage(MPSWizard::PAGE_CONNECT_E)) {
        text.append("PAGE_CONNECT_E, ");
    }

    if (wizard()->hasVisitedPage(MPSWizard::PAGE_SEL_TOMO_E)) {
        text.append("PAGE_SEL_TOMO_E, ");
    }

    if (wizard()->hasVisitedPage(MPSWizard::PAGE_PROCESS_E)) {
        text.append("PAGE_PROCESS_E, ");
    }

    bottomLabel->setText(text);
}

void FinalPage::setVisible(bool visible)
{
    QWizardPage::setVisible(visible);

    if (visible) {
        wizard()->setButtonText(QWizard::CustomButton1, tr("&Print"));
        wizard()->setOption(QWizard::HaveCustomButton1, true);
        connect(wizard(), &QWizard::customButtonClicked,
                this, &FinalPage::printButtonClicked);
    } else {
        wizard()->setOption(QWizard::HaveCustomButton1, false);
        disconnect(wizard(), &QWizard::customButtonClicked,
                   this, &FinalPage::printButtonClicked);
    }
}

void FinalPage::printButtonClicked()
{
    qInfo ("print clicked");
}
