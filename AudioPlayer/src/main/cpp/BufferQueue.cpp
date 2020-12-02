//
// Created by ywl on 2017-12-3.
//

#include "BufferQueue.h"
#include "AndroidLog.h"

BufferQueue::BufferQueue(PlayStatus *playStatus) {
    this->playStatus = playStatus;
    pthread_mutex_init(&mutexBuffer, NULL);
    pthread_cond_init(&condBuffer, NULL);
}

BufferQueue::~BufferQueue() {
    playStatus = NULL;
    pthread_mutex_destroy(&mutexBuffer);
    pthread_cond_destroy(&condBuffer);
    if(LOG_DEBUG)
    {
        LOGE("BufferQueue 释放完了");
    }
}

void BufferQueue::release() {

    if(LOG_DEBUG)
    {
        LOGE("BufferQueue::release");
    }
    noticeThread();
    clearBuffer();

    if(LOG_DEBUG)
    {
        LOGE("BufferQueue::release success");
    }
}

int BufferQueue::putBuffer(SAMPLETYPE *buffer, int size) {
    pthread_mutex_lock(&mutexBuffer);
    PCMBean *pcmBean = new PCMBean(buffer, size);
    queueBuffer.push_back(pcmBean);
    pthread_cond_signal(&condBuffer);
    pthread_mutex_unlock(&mutexBuffer);
    return 0;
}

int BufferQueue::getBuffer(PCMBean **pcmBean) {

    pthread_mutex_lock(&mutexBuffer);

    while(playStatus != NULL && !playStatus->exit)
    {
        if(queueBuffer.size() > 0)
        {
            *pcmBean = queueBuffer.front();
            queueBuffer.pop_front();
            break;
        } else{
            if(!playStatus->exit)
            {
                pthread_cond_wait(&condBuffer, &mutexBuffer);
            }
        }
    }
    pthread_mutex_unlock(&mutexBuffer);
    return 0;
}

int BufferQueue::clearBuffer() {

    pthread_cond_signal(&condBuffer);
    pthread_mutex_lock(&mutexBuffer);
    while (!queueBuffer.empty())
    {
        PCMBean *pcmBean = queueBuffer.front();
        queueBuffer.pop_front();
        delete(pcmBean);
    }
    pthread_mutex_unlock(&mutexBuffer);
    return 0;
}

int BufferQueue::getBufferSize() {
    int size = 0;
    pthread_mutex_lock(&mutexBuffer);
    size = queueBuffer.size();
    pthread_mutex_unlock(&mutexBuffer);
    return size;
}


int BufferQueue::noticeThread() {
    pthread_cond_signal(&condBuffer);
    return 0;
}

