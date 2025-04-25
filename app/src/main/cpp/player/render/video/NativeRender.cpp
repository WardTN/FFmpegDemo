//
// Created by deqi_chen on 2025/4/24.
//

#include "NativeRender.h"

NativeRender::NativeRender(JNIEnv *env, jobject surface) : VideoRender(VIDEO_RENDER_ANWINDOW) {
    m_NativeWindow = ANativeWindow_fromSurface(env, surface);
}

NativeRender::~NativeRender() {
    if (m_NativeWindow)
        ANativeWindow_release(m_NativeWindow);
}

void NativeRender::Init(int videoWidth, int videoHeight, int *dstSize) {
    LOGCATE("NativeRender::Init m_NativeWindow=%p, video[w,h]=[%d, %d]", m_NativeWindow, videoWidth,
            videoHeight);
    if (m_NativeWindow == nullptr) return;
    // 窗口实际宽高
    int windowWidth = ANativeWindow_getWidth(m_NativeWindow);
    int windowHeight = ANativeWindow_getHeight(m_NativeWindow);

    // 根据视频尺寸计算窗口尺寸
    if (windowWidth < windowHeight * videoWidth / videoHeight) {
        m_DstWidth = windowWidth;
        m_DstHeight = windowWidth * videoHeight / videoWidth;
    } else {
        m_DstWidth = windowHeight * videoWidth / videoHeight;
        m_DstHeight = windowHeight;
    }

    LOGCATE("NativeRender::Init window[w,h]=[%d, %d],DstSize[w, h]=[%d, %d]", windowWidth, windowHeight, m_DstWidth, m_DstHeight);

    // 设置窗口尺寸
    ANativeWindow_setBuffersGeometry(m_NativeWindow, m_DstWidth,
                                     m_DstHeight, WINDOW_FORMAT_RGBA_8888);
    // 将目标尺寸保存到dstSize
    dstSize[0] = m_DstWidth;
    dstSize[1] = m_DstHeight;

}

/**
 * 渲染视频帧
 * @param pImage
 */
void NativeRender::RenderVideoFrame(NativeImage *pImage) {
    if(m_NativeWindow == nullptr || pImage == nullptr) return;
    ANativeWindow_lock(m_NativeWindow, &m_NativeWindowBuffer, nullptr);

    uint8_t *dstBuffer = static_cast<uint8_t *>(m_NativeWindowBuffer.bits);

    int srcLineSize = pImage->width * 4;//RGBA
    int dstLineSize = m_NativeWindowBuffer.stride * 4;

    for (int i = 0; i < m_DstHeight; ++i) {
        memcpy(dstBuffer + i * dstLineSize, pImage->ppPlane[0] + i * srcLineSize, srcLineSize);
    }

    ANativeWindow_unlockAndPost(m_NativeWindow);
}


void NativeRender::UnInit()
{

}


