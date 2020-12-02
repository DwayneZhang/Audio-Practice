#include <jni.h>
#include <string>
#include "androidlog.h"
#include "CallJava.h"
#include "FFmpeg.h"

extern "C" {
#include "libavformat/avformat.h"
}

JavaVM *javaVM = NULL;
CallJava *callJava = NULL;
FFmpeg *ffmpeg = NULL;
PlayStatus *playStatus = NULL;
pthread_t thread_start;

bool nexit = true;

extern "C"
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    jint result = -1;
    javaVM = vm;
    JNIEnv *env;
    if (vm->GetEnv((void **) (&env), JNI_VERSION_1_4) != JNI_OK) {
        return result;
    }
    return JNI_VERSION_1_4;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_dwayne_com_audioplayer_player_AudioPlayer_n_1prepare(JNIEnv *env, jobject thiz,
                                                              jstring source) {

    const char *url = env->GetStringUTFChars(source, 0);

    if (ffmpeg == NULL) {
        if (callJava == NULL) {
            callJava = new CallJava(javaVM, env, &thiz);
        }
        callJava->onCallLoad(MAIN_THREAD, true);
        playStatus = new PlayStatus;
        ffmpeg = new FFmpeg(playStatus, callJava, url);
        ffmpeg->perpare();
    }

//    env->ReleaseStringUTFChars(source, url);
}

void *startCallback(void *data) {
    FFmpeg *ffmpeg = (FFmpeg *) data;
    ffmpeg->start();
    pthread_exit(&thread_start);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_dwayne_com_audioplayer_player_AudioPlayer_n_1start(JNIEnv *env, jobject thiz) {
    if (ffmpeg != NULL) {
        pthread_create(&thread_start, NULL, startCallback, ffmpeg);
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_dwayne_com_audioplayer_player_AudioPlayer_n_1pause(JNIEnv *env, jobject thiz) {
    if (ffmpeg != NULL) {
        ffmpeg->pause();
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_dwayne_com_audioplayer_player_AudioPlayer_n_1resume(JNIEnv *env, jobject thiz) {
    if (ffmpeg != NULL) {
        ffmpeg->resume();
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_dwayne_com_audioplayer_player_AudioPlayer_n_1stop(JNIEnv *env, jobject thiz) {
    if (!nexit) {
        return;
    }

    jclass jlz = env->GetObjectClass(thiz);
    jmethodID jmid_next = env->GetMethodID(jlz, "onCallNext", "()V");

    nexit = false;
    if (ffmpeg != NULL) {
        ffmpeg->release();
        delete (ffmpeg);
        ffmpeg = NULL;
        if (callJava != NULL) {
            delete (callJava);
            callJava = NULL;
        }
        if (playStatus != NULL) {
            delete (playStatus);
            playStatus = NULL;
        }
    }
    nexit = true;
    env->CallVoidMethod(thiz, jmid_next);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_dwayne_com_audioplayer_player_AudioPlayer_n_1seek(JNIEnv *env, jobject thiz,
                                                           jint secds) {
    if (ffmpeg != NULL) {
        ffmpeg->seek(secds);
    }
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_dwayne_com_audioplayer_player_AudioPlayer_n_1duration(JNIEnv *env,
                                                               jobject thiz) {
    if (ffmpeg != NULL) {
        return ffmpeg->duration;
    }
    return 0;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_dwayne_com_audioplayer_player_AudioPlayer_n_1volume(JNIEnv *env, jobject thiz,
                                                             jint percent) {
    if (ffmpeg != NULL) {
        ffmpeg->setVolume(percent);
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_dwayne_com_audioplayer_player_AudioPlayer_n_1mute(JNIEnv *env, jobject thiz,
                                                           jint mute) {
    if (ffmpeg != NULL) {
        ffmpeg->setMute(mute);
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_dwayne_com_audioplayer_player_AudioPlayer_n_1pitch(JNIEnv *env, jobject thiz,
                                                            jfloat pitch) {
    if (ffmpeg != NULL) {
        ffmpeg->setPitch(pitch);
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_dwayne_com_audioplayer_player_AudioPlayer_n_1speed(JNIEnv *env, jobject thiz,
                                                            jfloat speed) {
    if (ffmpeg != NULL) {
        ffmpeg->setSpeed(speed);
    }
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_dwayne_com_audioplayer_player_AudioPlayer_n_1samplerate(JNIEnv *env,
                                                                 jobject thiz) {
    if (ffmpeg != NULL) {
        return ffmpeg->getSamplerate();
    }
    return 0;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_dwayne_com_audioplayer_player_AudioPlayer_n_1record(JNIEnv *env,
        jobject thiz, jboolean record) {
    if (ffmpeg != NULL) {
        ffmpeg->record(record);
    }
}extern "C"
JNIEXPORT jboolean JNICALL
Java_com_dwayne_com_audioplayer_player_AudioPlayer_n_1cutaudio(JNIEnv *env, jobject thiz,
                                                               jint start_time,
                                                               jint end_time,
                                                               jboolean show_pcm) {
    if (ffmpeg != NULL) {

    }
    return false;
}