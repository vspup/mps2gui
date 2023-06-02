// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef LICENSEWIZARD_H
#define LICENSEWIZARD_H

#include <QWizard>

QT_BEGIN_NAMESPACE
class QCheckBox;
class QLabel;
class QLineEdit;
class QRadioButton;
class QPropertyAnimation;
//class QComboBox;
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
    QLabel *topLabel;
};

class ConnectPage : public QWizardPage
{
    Q_OBJECT

public:
    ConnectPage(QWidget *parent = nullptr);

    int nextId() const override;

private:
    QLabel *connectLabel;
    QLineEdit *ipLineEdit;
};

class SelectTomoPage : public QWizardPage
{
    Q_OBJECT

public:
    SelectTomoPage(QWidget *parent = nullptr);

    int nextId() const override;

private:
    QLabel *manufLabel;
    QLabel *modelLabel;
    QLineEdit *manufLineEdit;
    QLineEdit *modelLineEdit;
};

class ProcessPage : public QWizardPage
{
    Q_OBJECT

public:
    ProcessPage(QWidget *parent = nullptr);
    int nextId() const override;

private:
    QPushButton *workButton;
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
    void printButtonClicked();

private:
    QLabel *bottomLabel;
    QCheckBox *agreeCheckBox;
};

#endif
