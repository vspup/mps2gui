#ifndef WIZARDMPS_H
#define WIZARDMPS_H

#include <QWizard>

QT_BEGIN_NAMESPACE
//class QCheckBox;
class QLabel;
//class QLineEdit;
//class QRadioButton;
QT_END_NAMESPACE

class WizardMPS : public QWizard
{
    Q_OBJECT

public:
    enum { Page_Start,
           //Page_Connect,
           //Page_Select_Tomograph,
           //Page_Process,
           //Page_Final
    };

    WizardMPS(QWidget *parent = nullptr);

private slots:
    void showHelp();
};

class StartPage : public QWizardPage
{
    Q_OBJECT

public:
    StartPage(QWidget *parent = nullptr);

    int nextId() const override;

private:
    QLabel *topLabel;
};

#endif // WIZARDMPS_H
