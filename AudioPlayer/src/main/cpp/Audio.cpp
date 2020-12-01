//
// Created by Dwayne on 20/11/24.
//

#include "Audio.h"

Audio::Audio(PlayStatus *playStatus, int sample_rate, CallJava *callJava) {
    this->playStatus = playStatus;
    this->sample_rate = sample_rate;
    this->callJava = callJava;
    queue = new Queue(playStatus);
    buffer = (uint8_t *) av_malloc(sample_rate * 2 * 2);

    sampleBuffer = (SAMPLETYPE *) malloc(sample_rate * 2 * 2);
    soundTouch = new SoundTouch();
    soundTouch->setSampleRate(sample_rate);
    soundTouch->setChannels(2);
    soundTouch->setPitch(pitch); //音调
    soundTouch->setTempo(speed); //音调
}

Audio::~Audio() {

}

void *decodPlay(void *data) {
    Audio *audio = (Audio *) data;
    audio->initOpenSLES();
    pthread_exit(&audio->thread_play);
}

void Audio::play() {
    pthread_create(&thread_play, NULL, decodPlay, this);
}

int Audio::resampleAudio(void **pcmbuf) {

    while (playStatus != NULL && !playStatus->exit) {
        if (queue->getQueueSize() == 0) {
            if (!playStatus->load) {
                playStatus->load = true;
                callJava->onCallLoad(CHILD_THREAD, true);
            }
            continue;
        } else {
            if (playStatus->load) {
                playStatus->load = false;
                callJava->onCallLoad(CHILD_THREAD, false);
            }
        }

        avPacket = av_packet_alloc();
        if (queue->getAvPacket(avPacket) != 0) {
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            continue;
        }
        ret = avcodec_send_packet(avCodecContext, avPacket);
        if (ret != 0) {
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            continue;
        }
        avFrame = av_frame_alloc();
        ret = avcodec_receive_frame(avCodecContext, avFrame);
        if (ret == 0) {
            if (avFrame->channels > 0 && avFrame->channel_layout == 0) {
                avFrame->channel_layout = av_get_default_channel_layout(
                        avFrame->channels);
            } else if (avFrame->channels == 0 && avFrame->channel_layout > 0) {
                avFrame->channels = av_get_channel_layout_nb_channels(
                        avFrame->channel_layout);
            }
            SwrContext *swr_ctx;
            swr_ctx = swr_alloc_set_opts(
                    NULL,
                    AV_CH_LAYOUT_STEREO,
                    AV_SAMPLE_FMT_S16,
                    avFrame->sample_rate,
                    avFrame->channel_layout,
                    (AVSampleFormat) avFrame->format,
                    avFrame->sample_rate,
                    0, 0);
            ret = swr_init(swr_ctx);
            if (!swr_ctx || ret < 0) {
                av_packet_free(&avPacket);
                av_free(avPacket);
                avPacket = NULL;

                av_frame_free(&avFrame);
                av_free(avFrame);
                avFrame = NULL;
                if (swr_ctx != NULL) {
                    swr_free(&swr_ctx);
                    swr_ctx = NULL;
                }
                continue;
            }

            nb = swr_convert(
                    swr_ctx,
                    &buffer,
                    avFrame->nb_samples,
                    (const uint8_t **) avFrame->data,
                    avFrame->nb_samples);

            int out_channels = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);
            data_size = nb * out_channels * av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);
//            if (LOG_DEBUG) {
//                LOGE("data size is %d", data_size);
//            }

            now_time = avFrame->pts * av_q2d(time_base);
            if (now_time < clock) {
                now_time = clock;
            }
            clock = now_time;
            *pcmbuf = buffer;
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;

            av_frame_free(&avFrame);
            av_free(avFrame);
            avFrame = NULL;
            swr_free(&swr_ctx);
            swr_ctx = NULL;
            break;
        } else {
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;

            av_frame_free(&avFrame);
            av_free(avFrame);
            avFrame = NULL;
            continue;
        }
    }

    return data_size;
}

int Audio::getSoundTouchData() {

    while (playStatus != NULL && !playStatus->exit) {
        out_buffer = NULL;
        if (finished) {
            finished = false;
            data_size = resampleAudio((void **) &out_buffer);
            if (data_size > 0) {
                for (int i = 0; i < data_size / 2 + 1;
                     i++) {
                    sampleBuffer[i] = (out_buffer[i * 2] |
                                       ((out_buffer[i * 2 + 1])) << 8);
                }
                soundTouch->putSamples(sampleBuffer, nb);
                num = soundTouch->receiveSamples(sampleBuffer, data_size / 4);
            } else {
                soundTouch->flush();
            }
        }
        if (num == 0) {
            finished = true;
            continue;
        } else {
            if (out_buffer = NULL) {
                num = soundTouch->receiveSamples(sampleBuffer, data_size / 4);
                if (num == 0) {
                    finished = true;
                    continue;
                }
            }
            return num;
        }
    }

    return 0;
}

void pcmBufferCallBack(SLAndroidSimpleBufferQueueItf bf, void *context) {
    Audio *audio = (Audio *) context;
    if (audio != NULL) {
        int buffersize = audio->getSoundTouchData();
        if (buffersize > 0) {
            audio->clock += buffersize / (double) (audio->sample_rate * 2 * 2);
            if (audio->clock - audio->last_time >= 0.1) {
                audio->last_time = audio->clock;
                audio->callJava->onCallTimeInfo(CHILD_THREAD, audio->clock,
                                                audio->duration);
            }

//            audio->callJava->onCallPCMToAAC(CHILD_THREAD, buffersize * 2 * 2,
//                                            audio->sampleBuffer);

            audio->callJava->onCallVolumeDB(
                    CHILD_THREAD,
                    audio->getPCMDB((char *) (audio->sampleBuffer), buffersize *4));
            (*audio->pcmBufferQueue)->Enqueue(audio->pcmBufferQueue,
                                              (char *) audio->sampleBuffer,
                                              buffersize * 2 * 2);
        }
    }
}

void Audio::initOpenSLES() {
    //第一步，创建OpenSL引擎
    SLresult result;
    result = slCreateEngine(&engineObject, 0, 0, 0, 0, 0);
    result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine);

    //第二步，创建混音器
    const SLInterfaceID mids[1] = {SL_IID_ENVIRONMENTALREVERB};
    const SLboolean mreq[1] = {SL_BOOLEAN_FALSE};
    result = (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 1, mids,
                                              mreq);
    (void) result;
    result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    (void) result;
    result = (*outputMixObject)->GetInterface(outputMixObject, SL_IID_ENVIRONMENTALREVERB,
                                              &outputMixEnvironmentalReverb);
    if (SL_RESULT_SUCCESS == result) {
        result = (*outputMixEnvironmentalReverb)->SetEnvironmentalReverbProperties(
                outputMixEnvironmentalReverb, &reverbSettings);
        (void) result;
    }
    SLDataLocator_OutputMix outputMix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    SLDataSink audioSnk = {&outputMix, 0};


    // 第三步，配置PCM格式信息
    SLDataLocator_AndroidSimpleBufferQueue android_queue = {
            SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};

    SLDataFormat_PCM pcm = {
            SL_DATAFORMAT_PCM,//播放pcm格式的数据
            2,//2个声道（立体声）
            (SLuint32) getCurrentSampleRateForOpensles(sample_rate),//44100hz的频率
            SL_PCMSAMPLEFORMAT_FIXED_16,//位数 16位
            SL_PCMSAMPLEFORMAT_FIXED_16,//和位数一致就行
            SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,//立体声（前左前右）
            SL_BYTEORDER_LITTLEENDIAN//结束标志
    };
    SLDataSource slDataSource = {&android_queue, &pcm};


    const SLInterfaceID ids[3] = {SL_IID_BUFFERQUEUE, SL_IID_VOLUME, SL_IID_MUTESOLO};
    const SLboolean req[3] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};

    (*engineEngine)->CreateAudioPlayer(engineEngine, &pcmPlayerObject, &slDataSource,
                                       &audioSnk, 3, ids, req);
    //初始化播放器
    (*pcmPlayerObject)->Realize(pcmPlayerObject, SL_BOOLEAN_FALSE);

    //得到接口后调用  获取Player接口
    (*pcmPlayerObject)->GetInterface(pcmPlayerObject, SL_IID_PLAY, &pcmPlayerPlay);

    //获取Volume接口
    (*pcmPlayerObject)->GetInterface(pcmPlayerObject, SL_IID_VOLUME, &pcmVolumePlay);
    setVolume(volumePercent);
    setMute(mute);

    //获取声道接口
    (*pcmPlayerObject)->GetInterface(pcmPlayerObject, SL_IID_MUTESOLO, &pcmMutePlay);

    //注册回调缓冲区 获取缓冲队列接口
    (*pcmPlayerObject)->GetInterface(pcmPlayerObject, SL_IID_BUFFERQUEUE,
                                     &pcmBufferQueue);
    //缓冲接口回调
    (*pcmBufferQueue)->RegisterCallback(pcmBufferQueue, pcmBufferCallBack, this);
    //获取播放状态接口
    (*pcmPlayerPlay)->SetPlayState(pcmPlayerPlay, SL_PLAYSTATE_PLAYING);
    pcmBufferCallBack(pcmBufferQueue, this);
}

int Audio::getCurrentSampleRateForOpensles(int sample_rate) {
    int rate = 0;
    switch (sample_rate) {
        case 8000:
            rate = SL_SAMPLINGRATE_8;
            break;
        case 11025:
            rate = SL_SAMPLINGRATE_11_025;
            break;
        case 12000:
            rate = SL_SAMPLINGRATE_12;
            break;
        case 16000:
            rate = SL_SAMPLINGRATE_16;
            break;
        case 22050:
            rate = SL_SAMPLINGRATE_22_05;
            break;
        case 24000:
            rate = SL_SAMPLINGRATE_24;
            break;
        case 32000:
            rate = SL_SAMPLINGRATE_32;
            break;
        case 44100:
            rate = SL_SAMPLINGRATE_44_1;
            break;
        case 48000:
            rate = SL_SAMPLINGRATE_48;
            break;
        case 64000:
            rate = SL_SAMPLINGRATE_64;
            break;
        case 88200:
            rate = SL_SAMPLINGRATE_88_2;
            break;
        case 96000:
            rate = SL_SAMPLINGRATE_96;
            break;
        case 192000:
            rate = SL_SAMPLINGRATE_192;
            break;
        default:
            rate = SL_SAMPLINGRATE_44_1;
    }
    return rate;
}

void Audio::pause() {
    if (pcmPlayerObject != NULL) {
        (*pcmPlayerPlay)->SetPlayState(pcmPlayerPlay, SL_PLAYSTATE_PAUSED);
    }
}

void Audio::resume() {
    if (pcmPlayerObject != NULL) {
        (*pcmPlayerPlay)->SetPlayState(pcmPlayerPlay, SL_PLAYSTATE_PLAYING);
    }
}

void Audio::stop() {
    if (pcmPlayerObject != NULL) {
        (*pcmPlayerPlay)->SetPlayState(pcmPlayerPlay, SL_PLAYSTATE_STOPPED);
    }
}

void Audio::release() {
    stop();
    if (queue != NULL) {
        delete queue;
        queue = NULL;
    }

    if (pcmPlayerObject != NULL) {
        (*pcmPlayerObject)->Destroy(pcmPlayerObject);
        pcmPlayerObject = NULL;
        pcmPlayerPlay = NULL;
        pcmBufferQueue = NULL;
    }

    if (outputMixObject != NULL) {
        (*outputMixObject)->Destroy(outputMixObject);
        outputMixObject = NULL;
        outputMixEnvironmentalReverb = NULL;
    }

    if (engineObject != NULL) {
        (*engineObject)->Destroy(engineObject);
        engineObject = NULL;
        engineEngine = NULL;
    }

    if (buffer != NULL) {
        free(buffer);
        buffer = NULL;
    }

    if (avCodecContext != NULL) {
        avcodec_close(avCodecContext);
        avcodec_free_context(&avCodecContext);
        avCodecContext = NULL;
    }

    if (playStatus != NULL) {
        playStatus = NULL;
    }

    if (callJava != NULL) {
        callJava = NULL;
    }
}

void Audio::setVolume(int percent) {
    volumePercent = percent;
    if (pcmVolumePlay != NULL) {
        if (percent > 30) {
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -20);
        } else if (percent > 25) {
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -22);
        } else if (percent > 20) {
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -25);
        } else if (percent > 15) {
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -28);
        } else if (percent > 10) {
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -30);
        } else if (percent > 5) {
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -34);
        } else if (percent > 3) {
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -37);
        } else if (percent > 0) {
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -40);
        } else {
            (*pcmVolumePlay)->SetVolumeLevel(pcmVolumePlay, (100 - percent) * -100);
        }
    }
}

void Audio::setMute(int mute) {
    this->mute = mute;
    if (pcmMutePlay != NULL) {
        if (mute == 0) { //left
            (*pcmMutePlay)->SetChannelMute(pcmMutePlay, 0, false);//0:左声道, 1:右声道
            (*pcmMutePlay)->SetChannelMute(pcmMutePlay, 1, true);
        } else if (mute == 1) { //right
            (*pcmMutePlay)->SetChannelMute(pcmMutePlay, 0, true);
            (*pcmMutePlay)->SetChannelMute(pcmMutePlay, 1, false);
        } else if (mute == 2) { //center
            (*pcmMutePlay)->SetChannelMute(pcmMutePlay, 0, false);
            (*pcmMutePlay)->SetChannelMute(pcmMutePlay, 1, false);
        }
    }
}

void Audio::setPitch(float pitch) {
    this->pitch = pitch;
    if (soundTouch != NULL) {
        soundTouch->setPitch(pitch);
    }
}

void Audio::setSpeed(float speed) {
    this->speed = speed;
    if (soundTouch != NULL) {
        soundTouch->setTempo(speed);
    }
}

int Audio::getPCMDB(char *pcmadata, size_t pcmsize) {
    int db = 0;
    short int pervalue = 0;
    double sum = 0;
    for (int i = 0; i < pcmsize; i += 2) {
        memcpy(&pervalue, pcmadata + i, 2);
        sum += abs(pervalue);
    }
    sum = sum / (pcmsize / 2);
    if (sum > 0) {
        db = (int) 20 * log10(sum);
    }
    return db;
}
