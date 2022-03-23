#include "dataProcessingThread.h"

dataProcessingThread::dataProcessingThread(QObject *parent) :
    QThread(parent),
    rawSig_ch1(RECORDS_PER_BUFFER, QVector<double>(PRE_TRIGGER_SAMPLES+POST_TRIGGER_SAMPLES)),
    rawSig_ch2(RECORDS_PER_BUFFER, QVector<double>(PRE_TRIGGER_SAMPLES+POST_TRIGGER_SAMPLES)),
    rawSig_ch3(RECORDS_PER_BUFFER, QVector<double>(PRE_TRIGGER_SAMPLES+POST_TRIGGER_SAMPLES)),
    rawSig_ch4(RECORDS_PER_BUFFER, QVector<double>(PRE_TRIGGER_SAMPLES+POST_TRIGGER_SAMPLES)),
    rawSig_flag(false),
    avgSig(NUM_AVERAGE_SIGNALS, QVector<double>(PRE_TRIGGER_SAMPLES+POST_TRIGGER_SAMPLES)),
    avgSig_flag(false),
    sig_m1(RECORDS_PER_BUFFER),
    sig_m2(RECORDS_PER_BUFFER),
    sig_flag(false)
{

}


dataProcessingThread::~dataProcessingThread()
{
    wait();
}

void dataProcessingThread::run()
{
    exec();
}


void dataProcessingThread::read_rawSig(QVector<double> *ch1,
                                       QVector<double> *ch2,
                                       QVector<double> *ch3,
                                       QVector<double> *ch4)
{
    QMutexLocker locker(&rawSig_mutex);

    *ch1 = rawSig_ch1[0];
    *ch2 = rawSig_ch2[0];
    *ch3 = rawSig_ch3[0];
    *ch4 = rawSig_ch4[0];

    rawSig_flag = false;
}

void dataProcessingThread::read_avgSig(QVector< QVector<double> > *avgSig_ch1)
{
    QMutexLocker locker(&avgSig_mutex);

    *avgSig_ch1 = this->avgSig;

    avgSig_flag = false;
}

void dataProcessingThread::read_sig(QVector<double> *sig_m1, QVector<double> *sig_m2)
{
    QMutexLocker locker(&sig_mutex);

    *sig_m1 = this->sig_m1;
    *sig_m2 = this->sig_m2;

    sig_flag = false;
}

void dataProcessingThread::updateTimeDomains(AlazarControlThread *dataThread)
{
    // Raw signal emission of one data point
    QVector< QVector<double> > temp_rawSig_ch1(RECORDS_PER_BUFFER, QVector<double>(PRE_TRIGGER_SAMPLES+POST_TRIGGER_SAMPLES));
    QVector< QVector<double> > temp_rawSig_ch2(RECORDS_PER_BUFFER, QVector<double>(PRE_TRIGGER_SAMPLES+POST_TRIGGER_SAMPLES));
    QVector< QVector<double> > temp_rawSig_ch3(RECORDS_PER_BUFFER, QVector<double>(PRE_TRIGGER_SAMPLES+POST_TRIGGER_SAMPLES));
    QVector< QVector<double> > temp_rawSig_ch4(RECORDS_PER_BUFFER, QVector<double>(PRE_TRIGGER_SAMPLES+POST_TRIGGER_SAMPLES));
    bool rawSig_sendEmit;

    dataThread->readLatestData(&temp_rawSig_ch1,
                               &temp_rawSig_ch2,
                               &temp_rawSig_ch3,
                               &temp_rawSig_ch4);

    {
        QMutexLocker locker(&rawSig_mutex);

        rawSig_ch1 = temp_rawSig_ch1;
        rawSig_ch2 = temp_rawSig_ch2;
        rawSig_ch3 = temp_rawSig_ch3;
        rawSig_ch4 = temp_rawSig_ch4;

        rawSig_sendEmit = !rawSig_flag;
        rawSig_flag = true;
    }

    if (rawSig_sendEmit)
    {
        emit rawSig_ready();
    }

    // Average signal emission
    QVector< QVector<double> > temp_avgSig(NUM_AVERAGE_SIGNALS, QVector<double>(PRE_TRIGGER_SAMPLES+POST_TRIGGER_SAMPLES));
    bool avgSig_sendEmit;

    for(int i = 0; i < NUM_AVERAGE_SIGNALS;i++){
        for(int j = 0; j < PRE_TRIGGER_SAMPLES+POST_TRIGGER_SAMPLES; j++){
            for (int k = 0; k < RECORDS_PER_BUFFER/NUM_AVERAGE_SIGNALS; k++){
                temp_avgSig[i][j] += temp_rawSig_ch3[k+i*RECORDS_PER_BUFFER/NUM_AVERAGE_SIGNALS][j];
            }
            temp_avgSig[i][j] /= RECORDS_PER_BUFFER/NUM_AVERAGE_SIGNALS;
        }
    }

    {
        QMutexLocker locker(&avgSig_mutex);
        avgSig = temp_avgSig;
        avgSig_sendEmit = !avgSig_flag;
        avgSig_flag = true;
    }

    if (avgSig_sendEmit)
    {
        emit avgSig_ready();
    }

    // Extracted signal emission for all data points
    QVector< double > temp_sig_m1(RECORDS_PER_BUFFER);
    QVector< double > temp_sig_m2(RECORDS_PER_BUFFER);
    bool sig_sendEmit;

    for (int i = 0; i < RECORDS_PER_BUFFER; i++){
        for (int j = 0; j < PRE_TRIGGER_SAMPLES+POST_TRIGGER_SAMPLES; j++){
            temp_sig_m1[i] += temp_rawSig_ch3[i][j];
        }
        temp_sig_m1[i] /= (PRE_TRIGGER_SAMPLES+POST_TRIGGER_SAMPLES);
    }

    for (int i = 0; i < RECORDS_PER_BUFFER; i++){
        for (int j = 0; j < PRE_TRIGGER_SAMPLES+POST_TRIGGER_SAMPLES; j++){
            temp_sig_m2[i] += temp_rawSig_ch4[i][j];
        }
        temp_sig_m2[i] /= (PRE_TRIGGER_SAMPLES+POST_TRIGGER_SAMPLES);
    }

    {
        QMutexLocker locker(&sig_mutex);
        sig_m1 = temp_sig_m1;
        sig_m2 = temp_sig_m2;
        sig_sendEmit = !sig_flag;
        sig_flag = true;
    }

    if (sig_sendEmit){
        emit sig_ready();
    }
}
