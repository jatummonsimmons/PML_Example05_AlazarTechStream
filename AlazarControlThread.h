#ifndef ALAZARCONTROLTHREAD_H
#define ALAZARCONTROLTHREAD_H

#include <QMutex>
#include <QSize>
#include <QThread>
#include <QWaitCondition>
#include <QTimer>
#include <QDebug>

#include <iostream>
#include <stdio.h>
#include <cstdint>
#include <stdlib.h>

#include "AlazarError.h"
#include "AlazarApi.h"
#include "AlazarCmd.h"

#ifdef _WIN32
#include <conio.h>
#else // ifndef _WIN32
#include <errno.h>
#include <math.h>
#include <signal.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#define TRUE  1
#define FALSE 0

#define _snprintf snprintf

inline U32 GetTickCount(void);
inline void Sleep(U32 dwTime_ms);
inline int _kbhit (void);
inline int GetLastError();
#endif // ifndef _WIN32

#define BUFFER_COUNT 4

class AlazarControlThread : public QThread
{
    Q_OBJECT

public:
    AlazarControlThread(QObject *parent = nullptr);
    ~AlazarControlThread();
    void readLatestData(QVector<double> *ch1, QVector<double> *ch2, QVector<double> *ch3, QVector<double> *ch4);
    void stopRunning();

signals:
    void dataReady();

protected:
    void run() override;

private:
    bool ConfigureBoard(HANDLE boardHandle);
    bool AcquireData(HANDLE boardHandle);

    U16 *BufferArray[BUFFER_COUNT] = { NULL };
    double samplesPerSec = 0.0;

    U32 preTriggerSamples;
    U32 postTriggerSamples;
    U32 recordsPerBuffer;
    U32 buffersPerAcquisition;
    U32 bytesPerBuffer;
    U32 buffersCompleted;

    U16 * saveBuffer;
    QMutex mutex;
    bool flag;
    QVector<double> ch1_tempBuffer;
    QVector<double> ch2_tempBuffer;
    QVector<double> ch3_tempBuffer;
    QVector<double> ch4_tempBuffer;
    bool running;


};

#endif // ALAZARCONTROLTHREAD_H
