//
// Created by Dwayne on 20/11/24.
//

#ifndef AUDIO_PRACTICE_CALLJAVA_H
#define AUDIO_PRACTICE_CALLJAVA_H

#include "jni.h"
#include <linux/stddef.h>
#include "androidlog.h"

#define MAIN_THREAD 0
#define CHILD_THREAD 1


class CallJava {

public:
    _JavaVM *javaVM = NULL;
    JNIEnv *jniEnv = NULL;
    jobject jobj;

    jmethodID  jmid_prepared;
    jmethodID  jmid_load;
    jmethodID  jmid_timeinfo;
    jmethodID  jmid_error;
    jmethodID  jmid_complete;
    jmethodID  jmid_volumedb;
    jmethodID  jmid_pcmtoaac;

public:
    CallJava(JavaVM *vm, JNIEnv *env, jobject *obj);
    ~CallJava();

    void onCallPrepared(int threadType);

    void onCallLoad(int threadType, bool load);

    void onCallTimeInfo(int threadType, int curr, int total);

    void onCallError(int threadType, int code, const char *msg);

    void onCallComplete(int threadType);

    void onCallVolumeDB(int threadType, int db);

    void onCallPCMToAAC(int threadType, int size, void *buffer);

};


#endif //AUDIO_PRACTICE_CALLJAVA_H
