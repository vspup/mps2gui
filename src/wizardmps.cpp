#include "inc/wizardmps.h"

#include <QDebug>
#include <QtWidgets>

WizardMPS::WizardMPS(QWidget *parent)
    : QWizard(parent)
{
    setPage(Page_Start, new StartPage);
    //setPage(Page_Connect, new EvaluatePage);
    //setPage(Page_Select_Tomograph, new RegisterPage);
    //setPage(Page_Process, new DetailsPage);
    //setPage(Page_Final, new ConclusionPage);

    setStartId(Page_Start);

    setOption(HaveHelpButton, true);
    setPixmap(QWizard::LogoPixmap, QPixmap(":/images/logo.png"));

    connect(this, &QWizard::helpRequested, this, &WizardMPS::showHelp);

    setWindowTitle("MPS");
}

StartPage::StartPage(QWidget *parent)
    : QWizardPage(parent)
{
    setTitle("Start Page");
    //setPixmap(QWizard::WatermarkPixmap, QPixmap(":/images/watermark.png"));

    topLabel = new QLabel("This wizard will help you to setup MPS");
    topLabel->setWordWrap(true);
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(topLabel);
    setLayout(layout);
}

int StartPage::nextId() const
{
//    if (evaluateRadioButton->isChecked()) {
//        return WizardMPS::Page_Evaluate;
//    } else {
//        return WizardMPS::Page_Register;
//    }
    return WizardMPS::Page_Start;
}
