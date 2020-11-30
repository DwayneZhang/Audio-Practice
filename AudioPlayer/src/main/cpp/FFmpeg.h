//
// Created by Dwayne on 20/11/24.
//

#ifndef AUDIO_PRACTICE_FFMPEG_H
#define AUDIO_PRACTICE_FFMPEG_H

#include "CallJava.h"
#include "pthread.h"
#include "Audio.h"

extern "C" {
#include "libavformat/avformat.h"
#include "libavutil/time.h"
}

class FFmpeg {

public:
    CallJava *callJava = NULL;
    const char *url = NULL;
    pthread_t decodeThread;
    AVFormatContext  *pFormatCtx = NULL;
    Audio *audio = NULL;
    PlayStatus *playStatus = NULL;
    pthread_mutex_t  init_mutex;
    bool exit = false;
    int duration = 0;
    pthread_mutex_t seek_mutex;

public:
    FFmpeg(PlayStatus *playStatus, CallJava *callJava, const char *url);
    ~FFmpeg();

    void perpare();

    void decodeFFmpegThread();

    void start();

    void pause();

    void resume();

    void release();

    void seek(int64_t secds);

    void setVolume(int percent);

    void setMute(int mute);
};


#endif //AUDIO_PRACTICE_FFMPEG_H
