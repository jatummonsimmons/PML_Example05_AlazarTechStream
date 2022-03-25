#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "AlazarControlThread.h"
#include "qcustomplot.h"
#include "acquisitionConfig.h"
#include "dataProcessingThread.h"

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
    void setupAvgSigPlot(QCustomPlot *customPlot);
    void setupSigPlot(QCustomPlot *customPlot);
    void setupScImgPlot(QCustomPlot *customPlot);
    void setupPaImgPlot(QCustomPlot *customPlot);

    void setupSigVsMirrorPlot(QCustomPlot *customPlot);


private slots:
    void updateTimeDomain();
    void updateAvgSig();
    void updateSig();

private:
    Ui::MainWindow *ui;
    AlazarControlThread dataThread;
    dataProcessingThread processingThread;

    QVector<double> ch1;
    QVector<double> ch2;
    QVector<double> ch3;
    QVector<double> ch4;
    QVector<double> x;

    QVector< QVector<double> > avgSig;

    QVector<double> sig_m1;
    QVector<double> sig_m2;
    QVector<double> sig_sc;
    QVector<double> sig_pa;
    QVector<double> y;

    QCPColorMap *colorMap_sc;
    QCPColorMap *colorMap_pa;
    int last_y_index;
    double img_count[NUM_PIXELS][NUM_PIXELS];
    double img_sc[NUM_PIXELS][NUM_PIXELS];
    double img_pa[NUM_PIXELS][NUM_PIXELS];

};
#endif // MAINWINDOW_H
