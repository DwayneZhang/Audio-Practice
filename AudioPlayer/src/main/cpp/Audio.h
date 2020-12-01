//
// Created by Dwayne on 20/11/24.
//

#ifndef AUDIO_PRACTICE_AUDIO_H
#define AUDIO_PRACTICE_AUDIO_H


#include "Queue.h"
#include "PlayStatus.h"
#include "CallJava.h"
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include "SoundTouch.h"

using namespace soundtouch;

extern "C" {
#include "libavcodec/avcodec.h"
#include "libswresample/swresample.h"
#include "libavutil/time.h"
}

class Audio {

public:
    int streamIndex = -1;
    AVCodecParameters *codecpar = NULL;
    AVCodecContext *avCodecContext = NULL;
    Queue *queue = NULL;
    PlayStatus *playStatus = NULL;
    CallJava *callJava = NULL;

    pthread_t thread_play;
    AVPacket *avPacket = NULL;
    AVFrame *avFrame = NULL;
    int ret = 0;
    uint8_t *buffer = NULL;
    int data_size = 0;
    int sample_rate = 0;

    int duration = 0;
    AVRational time_base;
    double now_time  = 0;//当然帧时间
    double clock = 0;//总播放时长
    double last_time = 0;
    int volumePercent = 100;
    int mute = 2;
    float pitch = 1.0f;
    float speed = 1.0f;

    bool isRecordPCM = false;
    bool readFrameFinished = true;

    // 引擎接口
    SLObjectItf engineObject = NULL;
    SLEngineItf engineEngine = NULL;

    //混音器
    SLObjectItf outputMixObject = NULL;
    SLEnvironmentalReverbItf outputMixEnvironmentalReverb = NULL;
    SLEnvironmentalReverbSettings reverbSettings = SL_I3DL2_ENVIRONMENT_PRESET_STONECORRIDOR;

    //pcm
    SLObjectItf pcmPlayerObject = NULL;
    SLPlayItf pcmPlayerPlay = NULL;
    SLVolumeItf pcmVolumePlay = NULL;
    SLMuteSoloItf pcmMutePlay = NULL;

    //缓冲器队列接口
    SLAndroidSimpleBufferQueueItf pcmBufferQueue = NULL;

    SoundTouch *soundTouch = NULL;
    SAMPLETYPE *sampleBuffer = NULL;
    bool  finished = true;
    uint8_t *out_buffer = NULL;
    int nb = 0;
    int num = 0;

public:
    Audio(PlayStatus *playStatus, int sample_rate, CallJava *callJava);
    ~Audio();

    void play();

    int resampleAudio(void **pcmbuf);

    void initOpenSLES();

    int getCurrentSampleRateForOpensles(int sample_rate);

    void pause();

    void resume();

    void stop();

    void release();

    void setVolume(int percent);

    void setMute(int mute);

    int getSoundTouchData();

    void setPitch(float pitch);

    void setSpeed(float speed);

    int getPCMDB(char *pcmadata, size_t pcmsize);

    void recordPCM(bool record);
};


#endif //AUDIO_PRACTICE_AUDIO_H
