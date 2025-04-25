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
    return env->NewStringUTF(strBuffer);
}
#ifdef __cplusplus
}
#endif


VideoDecoder *m_VideoDecoder = nullptr;
VideoRender *m_VideoRender = nullptr;

extern "C"
JNIEXPORT void JNICALL
Java_com_wardtn_ffmpegdemo_media_FFMediaPlayer_ANativeWindowInit(JNIEnv *env, jobject thiz, jstring jurl,
                                                          jobject surface) {
    // Init
    const char *url = env->GetStringUTFChars(jurl, nullptr);
    m_VideoDecoder = new VideoDecoder(const_cast<char *>(url));
    m_VideoRender = new NativeRender(env, surface);
    m_VideoDecoder->SetVideoRender(m_VideoRender);
    env->ReleaseStringUTFChars(jurl, url);
}


extern "C"
JNIEXPORT void JNICALL
Java_com_wardtn_ffmpegdemo_media_FFMediaPlayer_ANativeWindowPlay(JNIEnv *env, jobject thiz) {

    if (m_VideoDecoder)
        m_VideoDecoder->Start();

}
