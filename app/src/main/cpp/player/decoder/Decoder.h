//
// Created by deqi_chen on 2025/4/23.
//

#ifndef FFMPEGDEMO_DECODER_H
#define FFMPEGDEMO_DECODER_H

/**
 * 定义消息回调函数类型
 * @param context 任意类型的指针,用于传递用户数据或上下文信息
 * @param int 消息类型或标识,表示不同的消息或事件
 * @param float 消息的附加信息或参数 如时间戳 、值等
 */
typedef void (*MessageCallback)(void*,int,float);

typedef long (*AVSyncCallback)(void*);

class Decoder {
public:
    virtual void Start() = 0;
    virtual void Pause() = 0;
    virtual void Stop() = 0;
    virtual float GetDuration() = 0;
    virtual void SeekToPosition(float position) = 0;
    virtual float GetCurrentPosition() = 0;
    virtual void SetMessageCallback(void* context, MessageCallback callback) = 0;

};


#endif //FFMPEGDEMO_DECODER_H
