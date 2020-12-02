//
// Created by Dwayne on 20/11/24.
//

#include "CallJava.h"

CallJava::CallJava(JavaVM *vm, JNIEnv *env, jobject *obj) {
    this->javaVM = vm;
    this->jniEnv = env;
    this->jobj = env->NewGlobalRef(*obj);

    jclass jclz = jniEnv->GetObjectClass(jobj);
    if (!jclz) {
        if (LOG_DEBUG) {
            LOGE("get jclass wrong");
        }
        return;
    }

    jmid_prepared = env->GetMethodID(jclz, "onCallPrepare", "()V");
    jmid_load = env->GetMethodID(jclz, "onCallLoad", "(Z)V");
    jmid_timeinfo = env->GetMethodID(jclz, "onTimeInfo", "(II)V");
    jmid_error = env->GetMethodID(jclz, "onCallError", "(ILjava/lang/String;)V");
    jmid_complete = env->GetMethodID(jclz, "onCallComplete", "()V");
    jmid_volumedb = env->GetMethodID(jclz, "onCallVolumeDB", "(I)V");
    jmid_pcmtoaac = env->GetMethodID(jclz, "encodecPCMToAAC", "(I[B)V");
    jmid_pcminfo = env->GetMethodID(jclz, "onCallPCMInfo", "([BI)V");
    jmid_pcmrate = env->GetMethodID(jclz, "onCallPCMRate", "(III)V");
}

CallJava::~CallJava() {
}

void CallJava::onCallPrepared(int threadType) {

    if (threadType == MAIN_THREAD) {
        jniEnv->CallVoidMethod(jobj, jmid_prepared);
    } else if (threadType == CHILD_THREAD) {
        JNIEnv *jniEnv;
        if (javaVM->AttachCurrentThread(&jniEnv, 0) != JNI_OK) {
            if (LOG_DEBUG) {
                LOGE("get child thread jnienv wrong");
            }
            return;
        }
        jniEnv->CallVoidMethod(jobj, jmid_prepared);
        javaVM->DetachCurrentThread();
    }
}

void CallJava::onCallLoad(int threadType, bool load) {
    if (threadType == MAIN_THREAD) {
        jniEnv->CallVoidMethod(jobj, jmid_load, load);
    } else if (threadType == CHILD_THREAD) {
        JNIEnv *jniEnv;
        if (javaVM->AttachCurrentThread(&jniEnv, 0) != JNI_OK) {
            if (LOG_DEBUG) {
                LOGE("get child thread jnienv wrong");
            }
            return;
        }
        jniEnv->CallVoidMethod(jobj, jmid_load, load);
        javaVM->DetachCurrentThread();
    }
}

void CallJava::onCallTimeInfo(int threadType, int curr, int total) {
    if (threadType == MAIN_THREAD) {
        jniEnv->CallVoidMethod(jobj, jmid_timeinfo, curr, total);
    } else if (threadType == CHILD_THREAD) {
        JNIEnv *jniEnv;
        if (javaVM->AttachCurrentThread(&jniEnv, 0) != JNI_OK) {
            if (LOG_DEBUG) {
                LOGE("get child thread jnienv wrong");
            }
            return;
        }
        jniEnv->CallVoidMethod(jobj, jmid_timeinfo, curr, total);
        javaVM->DetachCurrentThread();
    }
}

void CallJava::onCallError(int threadType, int code, const char *msg) {
    if (threadType == MAIN_THREAD) {
        jstring jmsg = jniEnv->NewStringUTF(msg);
        jniEnv->CallVoidMethod(jobj, jmid_error, code, jmsg);
        jniEnv->DeleteLocalRef(jmsg);
    } else if (threadType == CHILD_THREAD) {
        JNIEnv *jniEnv;
        if (javaVM->AttachCurrentThread(&jniEnv, 0) != JNI_OK) {
            if (LOG_DEBUG) {
                LOGE("get child thread jnienv wrong");
            }
            return;
        }
        jstring jmsg = jniEnv->NewStringUTF(msg);
        jniEnv->CallVoidMethod(jobj, jmid_error, code, jmsg);
        jniEnv->DeleteLocalRef(jmsg);
        javaVM->DetachCurrentThread();
    }
}

void CallJava::onCallComplete(int threadType) {
    if (threadType == MAIN_THREAD) {
        jniEnv->CallVoidMethod(jobj, jmid_complete);
    } else if (threadType == CHILD_THREAD) {
        JNIEnv *jniEnv;
        if (javaVM->AttachCurrentThread(&jniEnv, 0) != JNI_OK) {
            if (LOG_DEBUG) {
                LOGE("get child thread jnienv wrong");
            }
            return;
        }
        jniEnv->CallVoidMethod(jobj, jmid_complete);
        javaVM->DetachCurrentThread();
    }
}

void CallJava::onCallVolumeDB(int threadType, int db) {
    if (threadType == MAIN_THREAD) {
        jniEnv->CallVoidMethod(jobj, jmid_volumedb, db);
    } else if (threadType == CHILD_THREAD) {
        JNIEnv *jniEnv;
        if (javaVM->AttachCurrentThread(&jniEnv, 0) != JNI_OK) {
            if (LOG_DEBUG) {
                LOGE("get child thread jnienv wrong");
            }
            return;
        }
        jniEnv->CallVoidMethod(jobj, jmid_volumedb, db);
        javaVM->DetachCurrentThread();
    }
}

void CallJava::onCallPCMToAAC(int threadType, int size, void *buffer) {
    if (threadType == MAIN_THREAD) {
        jbyteArray  jbuffer = jniEnv->NewByteArray(size);
        jniEnv->SetByteArrayRegion(jbuffer, 0, size, (const jbyte *)(buffer));
        jniEnv->CallVoidMethod(jobj, jmid_pcmtoaac, size, jbuffer);
        jniEnv->DeleteLocalRef(jbuffer);
    } else if (threadType == CHILD_THREAD) {
        JNIEnv *jniEnv;
        if (javaVM->AttachCurrentThread(&jniEnv, 0) != JNI_OK) {
            if (LOG_DEBUG) {
                LOGE("get child thread jnienv wrong");
            }
            return;
        }
        jbyteArray  jbuffer = jniEnv->NewByteArray(size);
        jniEnv->SetByteArrayRegion(jbuffer, 0, size, (const jbyte *)(buffer));
        jniEnv->CallVoidMethod(jobj, jmid_pcmtoaac, size, jbuffer);
        jniEnv->DeleteLocalRef(jbuffer);
        javaVM->DetachCurrentThread();
    }
}

void CallJava::onCallPCMInfo(int threadType, void *buffer, int bufferSize) {
    if (threadType == MAIN_THREAD) {
        jbyteArray  jbuffer = jniEnv->NewByteArray(bufferSize);
        jniEnv->SetByteArrayRegion(jbuffer, 0, bufferSize, (const jbyte *)(buffer));
        jniEnv->CallVoidMethod(jobj, jmid_pcminfo, jbuffer, bufferSize);
        jniEnv->DeleteLocalRef(jbuffer);
    } else if (threadType == CHILD_THREAD) {
        JNIEnv *jniEnv;
        if (javaVM->AttachCurrentThread(&jniEnv, 0) != JNI_OK) {
            if (LOG_DEBUG) {
                LOGE("get child thread jnienv wrong");
            }
            return;
        }
        jbyteArray  jbuffer = jniEnv->NewByteArray(bufferSize);
        jniEnv->SetByteArrayRegion(jbuffer, 0, bufferSize, (const jbyte *)(buffer));
        jniEnv->CallVoidMethod(jobj, jmid_pcminfo, jbuffer, bufferSize);
        jniEnv->DeleteLocalRef(jbuffer);
        javaVM->DetachCurrentThread();
    }
}

void CallJava::onCallPCMRate(int threadType, int sampleRate, int bit, int channels) {
    if (threadType == MAIN_THREAD) {
        jniEnv->CallVoidMethod(jobj, jmid_pcmrate, sampleRate, bit, channels);
    } else if (threadType == CHILD_THREAD) {
        JNIEnv *jniEnv;
        if (javaVM->AttachCurrentThread(&jniEnv, 0) != JNI_OK) {
            if (LOG_DEBUG) {
                LOGE("get child thread jnienv wrong");
            }
            return;
        }
        jniEnv->CallVoidMethod(jobj, jmid_pcmrate, sampleRate, bit, channels);
        javaVM->DetachCurrentThread();
    }
}


