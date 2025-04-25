//
// Created by deqi_chen on 2025/4/23.
//

#include "DecoderBase.h"
#include "../../util/LogUtil.h"

void DecoderBase::StartDecodingThread() {
    //创建并启动一个新的线程来执行AV编码任务
    //参数this 表示当前实例对象,作为线程函数的参数传递
    m_Thread = new thread(DoAVDecoding, this);
}

void DecoderBase::Start() {
    if (m_Thread == nullptr) {
        StartDecodingThread();
    } else {
        // 对 互斥锁 m_Mutex 加锁，确保线程安全
        // 调用 m_Cond.notify_all() 唤醒所有等待 m_Cond 条件变量的线程
        std::unique_lock<std::mutex> lock(m_Mutex);
        m_DecoderState = STATE_DECODING;
        m_Cond.notify_all();
    }
}

void DecoderBase::Pause() {
    std::unique_lock<std::mutex> lock(m_Mutex);
    m_DecoderState = STATE_PAUSE;
}

void DecoderBase::Stop() {
    LOGCATE("DecoderBase::Stop");
    std::unique_lock<std::mutex> lock(m_Mutex);
    m_DecoderState = STATE_STOP;
    m_Cond.notify_all();
}

void DecoderBase::SeekToPosition(float position) {
    LOGCATE("DecoderBase::SeekToPosition position=%f", position);
    std::unique_lock<std::mutex> lock(m_Mutex);
    m_SeekPosition = position;
    m_DecoderState = STATE_DECODING;
    m_Cond.notify_all();
}

float DecoderBase::GetCurrentPosition() {
    //std::unique_lock<std::mutex> lock(m_Mutex);//读写保护
    //单位 ms
    return m_CurTimeStamp;
}

int DecoderBase::Init(const char *url, AVMediaType mediaType) {
    LOGCATE("DecoderBase::Init url=%s, mediaType=%d", url, mediaType);
    // 将url拷贝到m_Url中
    strcpy(m_Url, url);
    // 设置解码器类型
    m_MediaType = mediaType;
    return 0;
}

void DecoderBase::UnInit() {
    LOGCATE("DecoderBase::UnInit m_MediaType=%d", m_MediaType);
    if (m_Thread) {
        Stop();
        m_Thread->join();
        delete m_Thread;
        m_Thread = nullptr;
    }
    LOGCATE("DecoderBase::UnInit end, m_MediaType=%d", m_MediaType);
}

int DecoderBase::InitFFDecoder() {
    int result = -1;
    do {
        //1.创建封装格式上下文
        m_AVFormatContext = avformat_alloc_context();

        //2.打开文件
        if (avformat_open_input(&m_AVFormatContext, m_Url, NULL, NULL) != 0) {
            LOGCATE("DecoderBase::InitFFDecoder avformat_open_input fail.");
            break;
        }

        //3.获取音视频流信息
        if (avformat_find_stream_info(m_AVFormatContext, NULL) < 0) {
            LOGCATE("DecoderBase::InitFFDecoder avformat_find_stream_info fail.");
            break;
        }

        //4.获取音视频流索引
        for (int i = 0; i < m_AVFormatContext->nb_streams; i++) {
            if (m_AVFormatContext->streams[i]->codecpar->codec_type == m_MediaType) {
                m_StreamIndex = i;
                break;
            }
        }

        if (m_StreamIndex == -1) {
            LOGCATE("DecoderBase::InitFFDecoder Fail to find stream index.");
            break;
        }

        //5.获取解码器参数
        AVCodecParameters *codecParameters = m_AVFormatContext->streams[m_StreamIndex]->codecpar;

        //6.获取解码器
        m_AVCodec = avcodec_find_decoder(codecParameters->codec_id);
        if (m_AVCodec == nullptr) {
            LOGCATE("DecoderBase::InitFFDecoder avcodec_find_decoder fail.");
            break;
        }

        //7.创建解码器上下文
        m_AVCodecContext = avcodec_alloc_context3(m_AVCodec);

        if (avcodec_parameters_to_context(m_AVCodecContext, codecParameters) != 0) {
            LOGCATE("DecoderBase::InitFFDecoder avcodec_parameters_to_context fail.");
            break;
        }

        // 初始化一个空的AVDictionary对象用于存储选项
        AVDictionary *pAVDictionary = nullptr;

        // 设置缓冲区大小为1024000字节，以优化数据流处理
        av_dict_set(&pAVDictionary, "buffer_size", "1024000", 0);
        // 设置接收数据的超时时间为20000000微秒（20秒），以防止长时间运行读取超时
        av_dict_set(&pAVDictionary, "stimeout", "20000000", 0);
        // 设置最大延迟为30000000微秒（30秒），以控制数据流处理的最大延迟时间
        av_dict_set(&pAVDictionary, "max_delay", "30000000", 0);
        // 设置RTSP传输协议为TCP，以利用TCP的可靠传输特性
        av_dict_set(&pAVDictionary, "rtsp_transport", "tcp", 0);

        //8.打开解码器
        result = avcodec_open2(m_AVCodecContext, m_AVCodec, &pAVDictionary);

        if (result < 0) {
            LOGCATE("DecoderBase::InitFFDecoder avcodec_open2 fail. result=%d", result);
            break;
        }
        result = 0;

        m_Duration = m_AVFormatContext->duration / AV_TIME_BASE * 1000;//us to ms
        //创建 AVPacket 存放编码数据
        m_Packet = av_packet_alloc(); // 存储音频或视
        //创建 AVFrame 存放解码后的数据
        m_Frame = av_frame_alloc(); // 存储音频和视频解码数据(原始数据)
    } while (false);

    if (result != 0 && m_MsgContext && m_MsgCallback)
        m_MsgCallback(m_MsgContext, MSG_DECODER_INIT_ERROR, 0);

    return result;
}

void DecoderBase::UnInitDecoder() {
    LOGCATE("DecoderBase::UnInitDecoder");
    if (m_Frame != nullptr) {
        av_frame_free(&m_Frame);
        m_Frame = nullptr;
    }

    if (m_Packet != nullptr) {
        av_packet_free(&m_Packet);
        m_Packet = nullptr;
    }

    if (m_AVCodecContext != nullptr) {
        avcodec_close(m_AVCodecContext);
        avcodec_free_context(&m_AVCodecContext);
        m_AVCodecContext = nullptr;
        m_AVCodec = nullptr;
    }

    if (m_AVFormatContext != nullptr) {
        avformat_close_input(&m_AVFormatContext);
        avformat_free_context(m_AVFormatContext);
        m_AVFormatContext = nullptr;
    }
}


void DecoderBase::DecodingLoop() {
    // 记录解码器开始解码过程的日志,包括媒体类型信息
    LOGCATE("DecoderBase::DecodingLoop start, m_MediaType=%d", m_MediaType);
    {
        // 加锁以保护共享资源
        std::unique_lock<std::mutex> lock(m_Mutex);
        // 设置解码器状态为正在解码
        m_DecoderState = STATE_DECODING;
        // 解锁以释放共享资源锁
        lock.unlock();
    }

    // 主解码循环，持续解码直到停止状态
    for (;;) {
        // 当解码器处于暂停状态时,等待解码恢复或停止
        while (m_DecoderState == STATE_PAUSE) {
            // 锁定互斥锁以安全地访问共享资源
            std::unique_lock<std::mutex> lock(m_Mutex);
            // 日志输出：解码器处于等待状态，包括媒体类型信息
            LOGCATE("DecoderBase::DecodingLoop waiting, m_MediaType=%d", m_MediaType);
            // 等待条件变量触发或超时继续执行
            m_Cond.wait_for(lock, std::chrono::milliseconds(10));
            // 更新开始时间戳，用于计算解码时间
            m_StartTimeStamp = GetSysCurrentTime() - m_CurTimeStamp;
        }

        // 检查解码器是否处于停止状态，如果是，则退出循环
        if (m_DecoderState == STATE_STOP) {
            break;
        }

        // 如果开始时间戳未设置，则设置为当前系统时间
        if (m_StartTimeStamp == -1)
            m_StartTimeStamp = GetSysCurrentTime();

        // 尝试解码一个数据包，如果失败，则暂停解码器
        if (DecodeOnePacket() != 0) {
            // 解码结束，暂停解码器
            std::unique_lock<std::mutex> lock(m_Mutex);
            m_DecoderState = STATE_PAUSE;
        }
    }
    LOGCATE("DecoderBase::DecodingLoop end");
}

/**
 * 更新时间戳
 *
 * 此函数用于更新当前解码帧的时间戳在某些情况下，例如当帧的DTS或PTS有效时，
 * 它们将被用来更新当前时间戳如果两者都无效，则将当前时间戳设置为0
 * 此外，如果执行了成功的位置查找，则根据当前时间戳调整开始时间戳
 */
void DecoderBase::UpdateTimeStamp() {

    // 记录函数调用日志
    LOGCATE("DecoderBase::UpdateTimeStamp");

    // 加锁以保护共享资源
    std::unique_lock<std::mutex> lock(m_Mutex);

    // 如果帧的DTS有效，使用DTS更新当前时间戳
    if (m_Frame->pkt_dts != AV_NOPTS_VALUE) {
        m_CurTimeStamp = m_Frame->pkt_dts;
    }
        // 否则，如果帧的PTS有效，使用PTS更新当前时间戳
    else if (m_Frame->pts != AV_NOPTS_VALUE) {
        m_CurTimeStamp = m_Frame->pts;
    }
        // 如果DTS和PTS都无效，将当前时间戳设置为0
    else {
        m_CurTimeStamp = 0;
    }

    // 将当前时间戳转换为毫秒单位
    m_CurTimeStamp = (int64_t) (
            (m_CurTimeStamp * av_q2d(m_AVFormatContext->streams[m_StreamIndex]->time_base)) * 1000);

    // 如果执行了成功的位置查找，调整开始时间戳
    if (m_SeekPosition > 0 && m_SeekSuccess) {
        m_StartTimeStamp = GetSysCurrentTime() - m_CurTimeStamp;
        // 重置查找位置和查找成功标志
        m_SeekPosition = 0;
        m_SeekSuccess = false;
    }

}

long DecoderBase::AVSync() {
    LOGCATE("DecoderBase::AVSync");
    long curSysTime = GetSysCurrentTime();
    //基于系统时钟计算从开始播放流逝的时间
    long elapsedTime = curSysTime - m_StartTimeStamp;

    if (m_MsgContext && m_MsgCallback && m_MediaType == AVMEDIA_TYPE_AUDIO)
        m_MsgCallback(m_MsgContext, MSG_DECODING_TIME, m_CurTimeStamp * 1.0f / 1000);

    long delay = 0;

    //向系统时钟同步
    if (m_CurTimeStamp > elapsedTime) {
        //休眠时间
        auto sleepTime = static_cast<unsigned int>(m_CurTimeStamp - elapsedTime);//ms
        //限制休眠时间不能过长
        sleepTime = sleepTime > DELAY_THRESHOLD ? DELAY_THRESHOLD : sleepTime;
        av_usleep(sleepTime * 1000);
    }
    delay = elapsedTime - m_CurTimeStamp;

    return delay;
}

int DecoderBase::DecodeOnePacket() {
    LOGCATE("DecoderBase::DecodeOnePacket m_MediaType=%d", m_MediaType);
    if (m_SeekPosition > 0) {
        //seek to frame
        // 将寻求位置转换为微秒单位
        int64_t seek_target = static_cast<int64_t>(m_SeekPosition * 1000000);//微秒
        // 初始化寻求位置的最小值
        int64_t seek_min = INT64_MIN;
        // 初始化寻求位置的最大值
        int64_t seek_max = INT64_MAX;
        //跳转到Seek位置
        int seek_ret = avformat_seek_file(m_AVFormatContext, -1, seek_min, seek_target, seek_max,0);
        if (seek_ret < 0) {
            // 如果寻求失败，设置寻求成功标志为false，并记录错误日志
            m_SeekSuccess = false;
            LOGCATE("BaseDecoder::DecodeOneFrame error while seeking m_MediaType=%d", m_MediaType);
        } else {
            // 如果寻求成功，刷新解码器缓冲区（如果流索引已设置）
            if (-1 != m_StreamIndex) {
                avcodec_flush_buffers(m_AVCodecContext);
            }
            ClearCache();
            m_SeekSuccess = true;
            LOGCATE("BaseDecoder::DecodeOneFrame seekFrame pos=%f, m_MediaType=%d", m_SeekPosition,
                    m_MediaType);
        }
    }

    // 从媒体文件中读取一帧数据到packet
    int result = av_read_frame(m_AVFormatContext, m_Packet);
    // 循环读取帧，直到无法读取更多帧（即达到文件末尾或发生错误）
    while (result == 0) {
        // 检查当前帧是否属于目标流
        if (m_Packet->stream_index == m_StreamIndex) {
            // 更新时间戳
//            UpdateTimeStamp(m_Packet);
            // 执行同步检查，如果延迟超过阈值则跳过当前帧
//            if(AVSync() > DELAY_THRESHOLD && m_CurTimeStamp > DELAY_THRESHOLD)
//            {
//                result = 0;
//                goto __EXIT;
//            }
            // 视频解码
            // 向解码器发送数据包进行解码
            if (avcodec_send_packet(m_AVCodecContext, m_Packet) == AVERROR_EOF) {
                // 解码结束
                result = -1;
                goto __EXIT;
            }

            // 一个 packet 包含多少 frame?
            int frameCount = 0;
            // 接收解码后的帧
            while (avcodec_receive_frame(m_AVCodecContext, m_Frame) == 0) {
                // 获取到 m_Frame 解码数据，在这里进行格式转换，然后进行渲染，下一节介绍 ANativeWindow 渲染过程
                // 更新时间戳
                UpdateTimeStamp();
                // 同步
                AVSync();
                // 渲染
                LOGCATE("DecoderBase::DecodeOnePacket 000 m_MediaType=%d", m_MediaType);
                OnFrameAvailable(m_Frame);
                LOGCATE("DecoderBase::DecodeOnePacket 0001 m_MediaType=%d", m_MediaType);
                frameCount++;
            }
            // 记录当前包解码出的帧数
            LOGCATE("BaseDecoder::DecodeOneFrame frameCount=%d", frameCount);
            // 判断一个 packet 是否解码完成
            if (frameCount > 0) {
                result = 0;
                goto __EXIT;
            }
        }
        // 释放当前包的引用，准备读取下一个包
        av_packet_unref(m_Packet);
        // 尝试读取下一帧
        result = av_read_frame(m_AVFormatContext, m_Packet);
    }

    __EXIT:
    av_packet_unref(m_Packet);
    return result;
}


void DecoderBase::DoAVDecoding(DecoderBase *decoder) {
    LOGCATE("DecoderBase::DoAVDecoding");
    do {
        // 初始化解码器
        if (decoder->InitFFDecoder() != 0) {
            break;
        }
        decoder->OnDecoderReady();
        decoder->DecodingLoop();
    } while (false);

    decoder->UnInitDecoder();
    decoder->OnDecoderDone();
}









