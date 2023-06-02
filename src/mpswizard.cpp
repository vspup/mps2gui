// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QtWidgets>
#include <QDebug>
#include "inc/mpswizard.h"

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
        message = tr("Init page.");
        break;

    case PAGE_CONNECT_E:
        message = tr("Connect here");
        break;
    case PAGE_SEL_TOMO_E:
        message = tr("Select tomograpth here");
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
    //setPixmap(QWizard::WatermarkPixmap, QPixmap(":/images/watermark.png"));

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
    ipLineEdit = new QLineEdit(tr("192.168.0.111"));

    QGridLayout *layout = new QGridLayout;
    layout->addWidget(ipLineEdit, 1, 1);
    layout->addWidget(connectLabel, 1, 0);
    layout->setAlignment(Qt::AlignCenter);
    setLayout(layout);
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
    pictureLabel->setAlignment(Qt::AlignCenter);

    connect(manufComBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SelectTomoPage::updatePicture);
    connect(modelComBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SelectTomoPage::updatePicture);

    QGridLayout *layout = new QGridLayout;
    layout->addWidget(manufLabel, 0, 0);
    layout->addWidget(manufComBox, 0, 1);
    layout->addWidget(modelLabel, 1, 0);
    layout->addWidget(modelComBox, 1, 1);
    layout->addWidget(pictureLabel, 2, 0, 1, 2);
    setLayout(layout);
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

    workButton = new QPushButton(tr("Main"), this);
    workButton->setFixedSize(80, 30);

    QPushButton *btnShim = new QPushButton(tr("SHIM"), this);
    btnShim->setFixedSize(80, 30);

    QGridLayout *layout = new QGridLayout;
    layout->addWidget(workButton, 0, 0);
    layout->addWidget(btnShim, 0, 1);
    setLayout(layout);
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

    registerField("conclusion.agree*", agreeCheckBox);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(bottomLabel);
    layout->addWidget(agreeCheckBox);
    setLayout(layout);
}

int FinalPage::nextId() const
{
    qInfo("Exiting from MPS");
    return -1;
}

void FinalPage::initializePage()
{
    QString text;
    text = tr("Visited: ");

    if (wizard()->hasVisitedPage(MPSWizard::PAGE_CONNECT_E)) {
        text.append("PAGE_CONNECT_E,");
    }

    if (wizard()->hasVisitedPage(MPSWizard::PAGE_PROCESS_E)) {
        text.append("PAGE_PROCESS_E,");
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
