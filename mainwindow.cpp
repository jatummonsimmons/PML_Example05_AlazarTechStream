#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::MainWindow),
    ch1(PRE_TRIGGER_SAMPLES+POST_TRIGGER_SAMPLES),
    ch2(PRE_TRIGGER_SAMPLES+POST_TRIGGER_SAMPLES),
    ch3(PRE_TRIGGER_SAMPLES+POST_TRIGGER_SAMPLES),
    ch4(PRE_TRIGGER_SAMPLES+POST_TRIGGER_SAMPLES),
    x(PRE_TRIGGER_SAMPLES+POST_TRIGGER_SAMPLES),
    avgSig(NUM_AVERAGE_SIGNALS, QVector<double>(PRE_TRIGGER_SAMPLES+POST_TRIGGER_SAMPLES)),
    sig_m1(RECORDS_PER_BUFFER),
    sig_m2(RECORDS_PER_BUFFER),
    sig_sc(RECORDS_PER_BUFFER),
    sig_pa(RECORDS_PER_BUFFER),
    y(RECORDS_PER_BUFFER)
{
    for (int i =0;i<PRE_TRIGGER_SAMPLES;i++){
        x[i] = PRE_TRIGGER_SAMPLES-i;
    }
    for (int i = PRE_TRIGGER_SAMPLES;i<PRE_TRIGGER_SAMPLES+POST_TRIGGER_SAMPLES;i++){
        x[i] = i;
    }
    for (int i =0;i<RECORDS_PER_BUFFER;i++){
        y[i] = i;
    }
    ui->setupUi(this);

    // Setting up image parameters
    colorMap_sc = new QCPColorMap(ui->customPlot_img_sc->xAxis, ui->customPlot_img_sc->yAxis);
    memset(img_count,0,sizeof(img_count));
    memset(img_sc,0,sizeof(img_sc));

    // Setting up image plots
    setupImgPlot(ui->customPlot_img_sc);

    // Setting up time domain plots
    setupTimeDomainPlot(ui->customPlot_1);
    setupTimeDomainPlot(ui->customPlot_2);
    setupTimeDomainPlot(ui->customPlot_3);
    setupTimeDomainPlot(ui->customPlot_4);

    // Setting up average time domain plot
    setupAvgSigPlot(ui->customPlot_avgSig);

    // Setting up signal plots
    setupSigPlot(ui->customPlot_sig_m1);
    setupSigPlot(ui->customPlot_sig_m2);
    setupSigPlot(ui->customPlot_sig_sc);
    setupSigPlot(ui->customPlot_sig_pa);

    // setting up signal vs. mirror plot
    setupSigVsMirrorPlot(ui->customPlot_sc_m1);
    setupSigVsMirrorPlot(ui->customPlot_pa_m1);

    //Initialization connections between control thread and processing thread
    connect(&dataThread,&AlazarControlThread::dataReady,
            &processingThread, &dataProcessingThread::updateTimeDomains);

    connect(&processingThread,&dataProcessingThread::rawSig_ready,
            this,&MainWindow::updateTimeDomain);

    connect(&processingThread,&dataProcessingThread::avgSig_ready,
            this,&MainWindow::updateAvgSig);

    connect(&processingThread,&dataProcessingThread::sig_ready,
            this,&MainWindow::updateSig);

    processingThread.start(QThread::NormalPriority);
    dataThread.start(QThread::HighestPriority);
}

MainWindow::~MainWindow()
{
    dataThread.stopRunning();
    delete ui;
}

void MainWindow::updateTimeDomain()
{
    processingThread.read_rawSig(&ch1,&ch2,&ch3,&ch4);
    ui->customPlot_1->graph(0)->setData(x,ch1);
    ui->customPlot_2->graph(0)->setData(x,ch2);
    ui->customPlot_3->graph(0)->setData(x,ch3);
    ui->customPlot_4->graph(0)->setData(x,ch4);
    ui->customPlot_1->replot();
    ui->customPlot_2->replot();
    ui->customPlot_3->replot();
    ui->customPlot_4->replot();
}

void MainWindow::updateAvgSig()
{
    processingThread.read_avgSig(&avgSig);
    for(int i = 0; i < NUM_AVERAGE_SIGNALS;i++){
        ui->customPlot_avgSig->graph(i)->setData(x,avgSig[i]);
    }
    ui->customPlot_avgSig->replot();
}

void MainWindow::updateSig()
{
    processingThread.read_sig(&sig_pa,&sig_sc,&sig_m1,&sig_m2);

    // Update extracted signal vs. buffer
    ui->customPlot_sig_pa->graph(0)->setData(y,sig_pa);
    ui->customPlot_sig_sc->graph(0)->setData(y,sig_sc);
    ui->customPlot_sig_m1->graph(0)->setData(y,sig_m1);
    ui->customPlot_sig_m2->graph(0)->setData(y,sig_m2);
    ui->customPlot_sig_pa->replot();
    ui->customPlot_sig_sc->replot();
    ui->customPlot_sig_m1->replot();
    ui->customPlot_sig_m2->replot();

    // Update extracted signal vs. fast axis
    ui->customPlot_sc_m1->graph(0)->setData(sig_m1,sig_sc);
    ui->customPlot_pa_m1->graph(0)->setData(sig_m1,sig_pa);
    ui->customPlot_sc_m1->replot();
    ui->customPlot_pa_m1->replot();

    // Update image
    int temp_x_index;
    int temp_y_index;

    for (int i = 0; i < sig_m1.length(); i++)
    {
        colorMap_sc->data()->coordToCell(sig_m1[i], sig_m2[i], &temp_x_index, &temp_y_index);
        //ensure the user has properly set the channel parameters
        if (temp_x_index < 0){
            temp_x_index = 0;
        }
        if (temp_x_index >= 300){
            temp_x_index = 299;
        }
        if (temp_y_index < 0){
            temp_y_index = 0;
        }
        if (temp_y_index >= 300){
            temp_y_index = 299;
        }

        img_count[temp_x_index][temp_y_index] +=1;
        img_sc[temp_x_index][temp_y_index] += sig_sc[i];
        colorMap_sc->data()->setCell(temp_x_index,temp_y_index,
                                  img_sc[temp_x_index][temp_y_index]/img_count[temp_x_index][temp_y_index]);
    }

    if (last_y_index == 0){
        last_y_index = temp_y_index;
    }

    if (std::abs(temp_y_index - last_y_index) > 1){
        int erase_line;
        if (temp_y_index == 299){
            erase_line = temp_y_index - 1;
        }
        else if (temp_y_index == 0){
            erase_line = temp_y_index + 1;
        }
        else if (temp_y_index - last_y_index < 0){
            erase_line = temp_y_index - 1;
        }
        else if (temp_y_index - last_y_index > 0){
            erase_line = temp_y_index + 1;
        }

        for (int i = 0; i < 300; i++){
            colorMap_sc->data()->setCell(i,erase_line,0);
            img_count[i][erase_line] = 0;
            img_sc[i][erase_line] = 0;
        }
        colorMap_sc->rescaleDataRange();
        ui->customPlot_img_sc->replot();
        last_y_index = temp_y_index;

    }

}

void MainWindow::setupTimeDomainPlot(QCustomPlot *customPlot)
{
    // Initialize all necessary customPlot parameters
    customPlot->addGraph();

    // give the axes some labels:
    customPlot->xAxis->setLabel("Time (Samples)");
    customPlot->yAxis->setLabel("Amplitude (V)");

    customPlot->xAxis->setRange(0, PRE_TRIGGER_SAMPLES+POST_TRIGGER_SAMPLES);
    customPlot->yAxis->setRange(-0.05,0.05);

    customPlot->setInteraction(QCP::iRangeZoom,true);
    customPlot->axisRect()->setRangeZoom(customPlot->yAxis->orientation());

    customPlot->setInteraction(QCP::iRangeDrag,true);
    customPlot->axisRect()->setRangeDrag(customPlot->yAxis->orientation());

}

void MainWindow::setupAvgSigPlot(QCustomPlot *customPlot)
{
    for(int i = 0; i < NUM_AVERAGE_SIGNALS; i++){
        customPlot->addGraph();
    }
    customPlot->xAxis->setLabel("Time (Samples)");
    customPlot->yAxis->setLabel("Amplitude (V)");

    customPlot->xAxis->setRange(0, PRE_TRIGGER_SAMPLES+POST_TRIGGER_SAMPLES);
    customPlot->yAxis->setRange(-0.05,0.05);

    customPlot->setInteraction(QCP::iRangeZoom,true);
    customPlot->axisRect()->setRangeZoom(customPlot->yAxis->orientation());

    customPlot->setInteraction(QCP::iRangeDrag,true);
    customPlot->axisRect()->setRangeDrag(customPlot->yAxis->orientation());
}

void MainWindow::setupSigPlot(QCustomPlot *customPlot)
{
    customPlot->addGraph();
    customPlot->xAxis->setLabel("Time (Records)");
    customPlot->yAxis->setLabel("Amplitude (V)");

    customPlot->xAxis->setRange(0, RECORDS_PER_BUFFER);
    customPlot->yAxis->setRange(-0.05,0.05);

    customPlot->setInteraction(QCP::iRangeZoom,true);
    customPlot->axisRect()->setRangeZoom(customPlot->yAxis->orientation());

    customPlot->setInteraction(QCP::iRangeDrag,true);
    customPlot->axisRect()->setRangeDrag(customPlot->yAxis->orientation());
}

void MainWindow::setupImgPlot(QCustomPlot *customPlot)
{
    // configure axis rect:
    customPlot->setInteractions(QCP::iRangeDrag|QCP::iRangeZoom); // this will also allow rescaling the color scale by dragging/zooming
    customPlot->axisRect()->setupFullAxesBox(true);

    // set up the QCPColorMap:
    int nx = 300;
    int ny = 300;
    colorMap_sc->data()->setSize(nx, ny); // we want the color map to have nx * ny data points
    colorMap_sc->data()->setRange(QCPRange(-0.03, 0.03), QCPRange(-0.03, 0.03)); // and span the coordinate range -1400, 1400 in both key (x) and value (y) dimensions

    // add a color scale:
    QCPColorScale *colorScale = new QCPColorScale(customPlot);
    customPlot->plotLayout()->addElement(0, 1, colorScale); // add it to the right of the main axis rect
    colorScale->setType(QCPAxis::atRight); // scale shall be vertical bar with tick/axis labels right (actually atRight is already the default)
    colorMap_sc->setColorScale(colorScale); // associate the color map with the color scale
    colorScale->axis()->setLabel("Scattering Amplitude (Voltage)");

    // set the color gradient of the color map to one of the presets:
    colorMap_sc->setGradient(QCPColorGradient::gpHot);
    // we could have also created a QCPColorGradient instance and added own colors to
    // the gradient, see the documentation of QCPColorGradient for what's possible.

    // make sure the axis rect and color scale synchronize their bottom and top margins (so they line up):
    QCPMarginGroup *marginGroup = new QCPMarginGroup(customPlot);
    customPlot->axisRect()->setMarginGroup(QCP::msBottom|QCP::msTop, marginGroup);
    colorScale->setMarginGroup(QCP::msBottom|QCP::msTop, marginGroup);

    // rescale the key (x) and value (y) axes so the whole color map is visible:
    colorMap_sc->setDataRange(QCPRange(0,100));
    customPlot->rescaleAxes();

}

void MainWindow::setupSigVsMirrorPlot(QCustomPlot *customPlot)
{
    customPlot->addGraph();
    customPlot->xAxis->setLabel("Position (V)");
    customPlot->yAxis->setLabel("Amplitude (V)");

    customPlot->xAxis->setRange(-0.02,0.02);
    customPlot->yAxis->setRange(0,0.5);

    customPlot->setInteraction(QCP::iRangeZoom,true);
    customPlot->axisRect()->setRangeZoom(customPlot->yAxis->orientation());

    customPlot->setInteraction(QCP::iRangeDrag,true);
    customPlot->axisRect()->setRangeDrag(customPlot->yAxis->orientation());
}
