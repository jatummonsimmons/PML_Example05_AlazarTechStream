#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::MainWindow),
    ch1(256),
    ch2(256),
    ch3(256),
    ch4(256),
    x(256)
{
    for (int i =0;i<256;i++){
        x[i] = i;
    }
    ui->setupUi(this);
    setupTimeDomainPlot(ui->customPlot_1);
    ui->customPlot_1->plotLayout()->insertRow(0);
    ui->customPlot_1->plotLayout()->addElement(0, 0, new QCPTextElement(ui->customPlot_1, "Channel 1", QFont("sans", 12, QFont::Bold)));
    setupTimeDomainPlot(ui->customPlot_2);
    ui->customPlot_2->plotLayout()->insertRow(0);
    ui->customPlot_2->plotLayout()->addElement(0, 0, new QCPTextElement(ui->customPlot_2, "Channel 2", QFont("sans", 12, QFont::Bold)));
    setupTimeDomainPlot(ui->customPlot_3);
    ui->customPlot_3->plotLayout()->insertRow(0);
    ui->customPlot_3->plotLayout()->addElement(0, 0, new QCPTextElement(ui->customPlot_3, "Channel 3", QFont("sans", 12, QFont::Bold)));
    setupTimeDomainPlot(ui->customPlot_4);
    ui->customPlot_4->plotLayout()->insertRow(0);
    ui->customPlot_4->plotLayout()->addElement(0, 0, new QCPTextElement(ui->customPlot_4, "Channel 4", QFont("sans", 12, QFont::Bold)));


    connect(&dataThread,&AlazarControlThread::dataReady,this, &MainWindow::updateTimeDomain);
    dataThread.start(QThread::NormalPriority);
}

MainWindow::~MainWindow()
{
    dataThread.stopRunning();
    delete ui;
}

void MainWindow::updateTimeDomain()
{
    dataThread.readLatestData(&ch1,&ch2,&ch3,&ch4);
    ui->customPlot_1->graph(0)->setData(x,ch1);
    ui->customPlot_2->graph(0)->setData(x,ch2);
    ui->customPlot_3->graph(0)->setData(x,ch3);
    ui->customPlot_4->graph(0)->setData(x,ch4);
    ui->customPlot_1->replot();
    ui->customPlot_2->replot();
    ui->customPlot_3->replot();
    ui->customPlot_4->replot();
}

void MainWindow::setupTimeDomainPlot(QCustomPlot *customPlot)
{
    // Initialize all necessary customPlot parameters
    customPlot->addGraph();

    // give the axes some labels:
    customPlot->xAxis->setLabel("Time (Samples)");
    customPlot->yAxis->setLabel("Amplitude (ADC Units)");

    customPlot->xAxis->setRange(0, 256);
    customPlot->yAxis->setRange(32500,33000);
}
