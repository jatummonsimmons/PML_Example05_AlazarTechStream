#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "AlazarControlThread.h"
#include "qcustomplot.h"
#include "acquisitionConfig.h"


// TODO: Add #defines for common constants (in particular data size constants) - add this and pass it to the constructor for the main window

#define CONSTANT_QWJEQJIWE (2000)

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void setupTimeDomainPlot(QCustomPlot *customPlot);

private slots:
    void updateTimeDomain();

private:
    Ui::MainWindow *ui;
    AlazarControlThread dataThread;
    QVector<double> ch1;
    QVector<double> ch2;
    QVector<double> ch3;
    QVector<double> ch4;
    QVector<double> x;
};
#endif // MAINWINDOW_H
