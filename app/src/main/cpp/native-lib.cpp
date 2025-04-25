#include <jni.h>
#include <string>
#include <cstdio>
#include "util/LogUtil.h"

//由于 FFmpeg 库是 C 语言实现的，告诉编译器按照 C 的规则进行编译
extern "C" {
#include <libavcodec/version.h>
#include <libavcodec/avcodec.h>
#include <libavformat/version.h>
#include <libavutil/version.h>
#include <libavfilter/version.h>
#include <libswresample/version.h>
#include <libswscale/version.h>
}


#include "VideoDecoder.h"
#include "NativeRender.h"


JavaVM *m_JavaVM = nullptr;
jobject m_JavaObj = nullptr;


#define JAVA_PLAYER_EVENT_CALLBACK_API_NAME "playerEventCallback"

JNIEnv *getJNIENV(bool *isAttach) {
    JNIEnv *env;
    int status;
    if (nullptr == m_JavaVM) {
        LOGCATE("Java_com_wardtn_ffmpegdemo_media_FFMediaPlayer_ANativeWindowInit m_JavaVM is null");
        return nullptr;
    }

    *isAttach = false;
    status = m_JavaVM->GetEnv((void **) &env, JNI_VERSION_1_4);
    if (status != JNI_OK) {
        // attach thread
        status = m_JavaVM->AttachCurrentThread(&env, nullptr);
        if (status != JNI_OK) {
            LOGCATE("FFMediaPlayer::GetJNIEnv failed to attach current thread");
            return nullptr;
        }
        *isAttach = true;
    }
    return env;
}


void postMesage(int msgType, float msgValue) {
    bool isAttach = false;
    JNIEnv *javaEnv = getJNIENV(&isAttach);
    LOGCATE("FFMediaPlayer::PostMessage env=%p", javaEnv);
    if (javaEnv == nullptr) return;

    jobject javaObj = m_JavaObj;
    jmethodID mid = javaEnv->GetMethodID(javaEnv->GetObjectClass(javaObj),
                                         JAVA_PLAYER_EVENT_CALLBACK_API_NAME, "(IF)V");

    javaEnv->CallVoidMethod(javaObj, mid, msgType, msgValue);
    if (isAttach)
        m_JavaVM->DetachCurrentThread();
}


#ifdef __cplusplus
extern "C" {
#endif
JNIEXPORT jstring JNICALL Java_com_wardtn_ffmpegdemo_media_FFMediaPlayer_stringFromJNI
        (JNIEnv *env, jobject) {
    char strBuffer[1024 * 4] = {0};
    strcat(strBuffer, "libavcodec : ");
    strcat(strBuffer, AV_STRINGIFY(LIBAVCODEC_VERSION));
    strcat(strBuffer, "\nlibavformat : ");
    strcat(strBuffer, AV_STRINGIFY(LIBAVFORMAT_VERSION));
    strcat(strBuffer, "\nlibavutil : ");
    strcat(strBuffer, AV_STRINGIFY(LIBAVUTIL_VERSION));
    strcat(strBuffer, "\nlibavfilter : ");
    strcat(strBuffer, AV_STRINGIFY(LIBAVFILTER_VERSION));
    strcat(strBuffer, "\nlibswresample : ");
    strcat(strBuffer, AV_STRINGIFY(LIBSWRESAMPLE_VERSION));
    strcat(strBuffer, "\nlibswscale : ");
    strcat(strBuffer, AV_STRINGIFY(LIBSWSCALE_VERSION));
    strcat(strBuffer, "\navcodec_configure : \n");
    strcat(strBuffer, avcodec_configuration());
    strcat(strBuffer, "\navcodec_license : ");
    strcat(strBuffer, avcodec_license());
    postMesage(0, 0);
    return env->NewStringUTF(strBuffer);
}
#ifdef __cplusplus
}
#endif


VideoDecoder *m_VideoDecoder = nullptr;
VideoRender *m_VideoRender = nullptr;


extern "C"
JNIEXPORT void JNICALL
Java_com_wardtn_ffmpegdemo_media_FFMediaPlayer_ANativeWindowInit(JNIEnv *env, jobject thiz,
                                                                 jstring jurl,
                                                                 jobject surface) {
    // Init
    const char *url = env->GetStringUTFChars(jurl, nullptr);
    m_VideoDecoder = new VideoDecoder(const_cast<char *>(url));
    m_VideoRender = new NativeRender(env, surface);
    m_VideoDecoder->SetVideoRender(m_VideoRender);
    env->ReleaseStringUTFChars(jurl, url);

    env->GetJavaVM(&m_JavaVM);
    m_JavaObj = env->NewGlobalRef(thiz);
}


extern "C"
JNIEXPORT void JNICALL
Java_com_wardtn_ffmpegdemo_media_FFMediaPlayer_ANativeWindowPlay(JNIEnv *env, jobject thiz) {

    if (m_VideoDecoder)
        m_VideoDecoder->Start();
}



