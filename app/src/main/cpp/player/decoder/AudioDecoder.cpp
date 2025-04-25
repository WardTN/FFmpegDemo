//
// Created by deqi_chen on 2025/4/24.
//

#include "AudioDecoder.h"
#include <LogUtil.h>


/**
 * OnDecoderReady函数用于初始化音频解码器和重采样上下文。
 * 该函数没有参数，也没有返回值。
 * 主要功能包括：
 * - 检查音频渲染模块是否可用
 * - 获取音频编解码器上下文并设置重采样参数
 * - 初始化重采样上下文
 * - 分配输出音频缓冲区
 * - 初始化音频渲染模块
 */

void AudioDecoder::OnDecoderReady() {
    LOGCATE("AudioDecoder::OnDecoderReady");

    if (m_AudioRender){
        // 获取当前音频编解码器的上下文信息
        AVCodecContext *codeCtx = GetCodecContext();
        // 分配一个重采样上下文
        m_SwrContext = swr_alloc();
        
        // 设置输入和输出声道布局
        av_opt_set_int(m_SwrContext, "in_channel_layout", codeCtx->channel_layout, 0);
        av_opt_set_int(m_SwrContext, "out_channel_layout", AUDIO_DST_CHANNEL_LAYOUT, 0);

        // 设置输入和输出采样率
        av_opt_set_int(m_SwrContext, "in_sample_rate", codeCtx->sample_rate, 0);
        av_opt_set_int(m_SwrContext, "out_sample_rate", AUDIO_DST_SAMPLE_RATE, 0);

        // 设置输入和输出样本格式
        av_opt_set_sample_fmt(m_SwrContext, "in_sample_fmt", codeCtx->sample_fmt, 0);
        av_opt_set_sample_fmt(m_SwrContext, "out_sample_fmt", DST_SAMPLT_FORMAT,  0);

        // 初始化重采样上下文
        swr_init(m_SwrContext);

        // 打印音频元数据信息
        LOGCATE("AudioDecoder::OnDecoderReady audio metadata sample rate: %d, channel: %d, format: %d, frame_size: %d, layout: %lld",
                codeCtx->sample_rate, codeCtx->channels, codeCtx->sample_fmt, codeCtx->frame_size,codeCtx->channel_layout);

        // 计算重采样的样本数和目标帧数据大小
        m_nbSamples = (int)av_rescale_rnd(ACC_NB_SAMPLES, AUDIO_DST_SAMPLE_RATE, codeCtx->sample_rate, AV_ROUND_UP);
        m_DstFrameDataSze = av_samples_get_buffer_size(NULL, AUDIO_DST_CHANNEL_COUNTS,m_nbSamples, DST_SAMPLT_FORMAT, 1);

        // 打印计算出的样本数和帧数据大小
        LOGCATE("AudioDecoder::OnDecoderReady [m_nbSamples, m_DstFrameDataSze]=[%d, %d]", m_nbSamples, m_DstFrameDataSze);

        // 分配内存给输出音频缓冲区
        m_AudioOutBuffer = (uint8_t *) malloc(m_DstFrameDataSze);

        // 初始化音频渲染模块
        m_AudioRender->Init();
    }else {
        // 如果音频渲染模块不可用，打印日志信息
        LOGCATE("AudioDecoder::OnDecoderReady m_AudioRender == null");
    }
}

void AudioDecoder::OnFrameAvailable(AVFrame *frame) {
    if(m_AudioRender) {
        int result = swr_convert(m_SwrContext, &m_AudioOutBuffer, m_DstFrameDataSze / 2, (const uint8_t **) frame->data, frame->nb_samples);
        if (result > 0 ) {
            m_AudioRender->RenderAudioFrame(m_AudioOutBuffer, m_DstFrameDataSze);
        }
    }
}

void AudioDecoder::OnDecoderDone() {
    LOGCATE("AudioDecoder::OnDecoderDone");
    if(m_AudioRender)
        m_AudioRender->UnInit();

    if(m_AudioOutBuffer) {
        free(m_AudioOutBuffer);
        m_AudioOutBuffer = nullptr;
    }

    if(m_SwrContext) {
        swr_free(&m_SwrContext);
        m_SwrContext = nullptr;
    }
}

void AudioDecoder::ClearCache() {
    if(m_AudioRender)
        m_AudioRender->ClearAudioCache();
}
