#ifndef SELECTWINDOW_H
#define SELECTWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>

namespace Ui {
class SelectWindow;
}

class SelectWindow : public QMainWindow
{
    Q_OBJECT

public:
    void writeLog(QString logstr);
    explicit SelectWindow(QWidget *parent = nullptr);
    ~SelectWindow();

private slots:

    void on_btConnect_clicked();

private:
    Ui::SelectWindow *ui;
};

#endif // SELECTWINDOW_H
