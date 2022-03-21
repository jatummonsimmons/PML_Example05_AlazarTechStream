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

    void readLatestData();

signals:
    void dataReady();

protected:
    void run() override;

private:
    bool ConfigureBoard(HANDLE boardHandle);
    bool AcquireData(HANDLE boardHandle);

    U16 *BufferArray[BUFFER_COUNT] = { NULL };
    double samplesPerSec = 0.0;

};

#endif // ALAZARCONTROLTHREAD_H
