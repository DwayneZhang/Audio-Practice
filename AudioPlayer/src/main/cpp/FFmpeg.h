//
// Created by Dwayne on 20/11/24.
//

#ifndef AUDIO_PRACTICE_FFMPEG_H
#define AUDIO_PRACTICE_FFMPEG_H

#include "CallJava.h"
#include "pthread.h"
#include "Audio.h"

extern "C" {
#include <libavformat/avformat.h>
}

class FFmpeg {

public:
    CallJava *callJava = NULL;
    const char *url = NULL;
    pthread_t decodeThread;
    AVFormatContext  *pFormatCtx = NULL;
    Audio *audio = NULL;
    PlayStatus *playStatus = NULL;

public:
    FFmpeg(PlayStatus *playStatus, CallJava *callJava, const char *url);
    ~FFmpeg();

    void perpare();

    void decodeFFmpegThread();

    void start();

};


#endif //AUDIO_PRACTICE_FFMPEG_H
