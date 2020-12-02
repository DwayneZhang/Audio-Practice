//
// Created by ywl on 2017-12-3.
//

#ifndef AUDIO_PRACTICE_BUFFERQUEUE_H
#define AUDIO_PRACTICE_BUFFERQUEUE_H

#include "deque"
#include "PlayStatus.h"
#include "PCMBean.h"

extern "C"
{
#include <libavcodec/avcodec.h>
#include "pthread.h"
};

class BufferQueue {

public:
    std::deque<PCMBean *> queueBuffer;
    pthread_mutex_t mutexBuffer;
    pthread_cond_t condBuffer;
    PlayStatus *playStatus = NULL;

public:
    BufferQueue(PlayStatus *playStatus);
    ~BufferQueue();
    int putBuffer(SAMPLETYPE *buffer, int size);
    int getBuffer(PCMBean **pcmBean);
    int clearBuffer();

    void release();
    int getBufferSize();

    int noticeThread();
};


#endif //AUDIO_PRACTICE_BUFFERQUEUE_H
