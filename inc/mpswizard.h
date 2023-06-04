// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef LICENSEWIZARD_H
#define LICENSEWIZARD_H

#include "qcombobox.h"
#include "qlabel.h"
#include <QWizard>

QT_BEGIN_NAMESPACE
class QCheckBox;
class QLabel;
class QLineEdit;
class QRadioButton;
class QPropertyAnimation;
class QComboBox;
QT_END_NAMESPACE

class MPSWizard : public QWizard
{
    Q_OBJECT

public:
    enum { PAGE_INIT_E,
           PAGE_CONNECT_E,
           PAGE_SEL_TOMO_E,
           PAGE_PROCESS_E,
           PAGE_FINAL_E
    };
    
    MPSWizard(QWidget *parent = nullptr);

private slots:
    void showHelp();
};

class InitPage : public QWizardPage
{
    Q_OBJECT

public:
    InitPage(QWidget *parent = nullptr);
    int nextId() const override;

private:

};

class ConnectPage : public QWizardPage
{
    Q_OBJECT

public:
    ConnectPage(QWidget *parent = nullptr);
    void handleButtonClicked();
    int nextId() const override;

private:
    QComboBox *ipComBox;
};

class SelectTomoPage : public QWizardPage
{
    Q_OBJECT

public:
    enum { MANUF_1,
           MANUF_2,
           MANUF_3,
           MANUF_MAX,
    };

    enum { MODEL_1,
           MODEL_2,
           MODEL_3,
           MODEL_4,
           MODEL_5,
           MODEL_6,
           MODEL_MAX,
    };

    SelectTomoPage(QWidget *parent = nullptr);
    void handlePrintButtonClicked();
    void updatePicture();
    int nextId() const override;

private:
    QComboBox *manufComBox;
    QComboBox *modelComBox;
    QLabel *pictureLabel;
    QPushButton* printButton;
};

class ProcessPage : public QWizardPage
{
    Q_OBJECT

public:
    ProcessPage(QWidget *parent = nullptr);
    void handleRunOldUIButtonClicked();
    int nextId() const override;

private:

};

class FinalPage : public QWizardPage
{
    Q_OBJECT

public:
    FinalPage(QWidget *parent = nullptr);

    void initializePage() override;
    int nextId() const override;
    void setVisible(bool visible) override;

private slots:
    void printCustomButtonClicked();

private:
    QLabel *bottomLabel;
    QCheckBox *agreeCheckBox;
};

#endif
