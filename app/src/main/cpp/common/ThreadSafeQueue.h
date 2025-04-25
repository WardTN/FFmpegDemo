//
// Created by deqi_chen on 2025/4/24.
//

#ifndef FFMPEGDEMO_THREADSAFEQUEUE_H
#define FFMPEGDEMO_THREADSAFEQUEUE_H

#include <queue>
#include <memory>
#include <mutex>
#include <condition_variable>

template<typename T>

class ThreadSafeQueue {
public:
    ThreadSafeQueue() {}

    ThreadSafeQueue(ThreadSafeQueue const &other) {
        std::lock_guard<std::mutex> lk(other.m_mutex);
        m_dataQueue = other.m_dataQueue;
    }

    void Push(T new_value) {
        std::lock_guard<std::mutex> lk(m_mutex);
        m_dataQueue.push(new_value);
        m_condVar.notify_one();
    }

    T Pop(){
        std::unique_lock<std::mutex> lk(m_mutex);
        if(Empty()) return nullptr;
        T res = m_dataQueue.front();
        m_dataQueue.pop();
        return res;
    }

    bool Empty() const {
        return m_dataQueue.empty();
    }

    int Size() {
        std::unique_lock<std::mutex> lk(m_mutex);
        return m_dataQueue.size();
    }

private:
    mutable std::mutex m_mutex;
    // 存储类型为 T的数据队列
    std::queue<T> m_dataQueue;
    // 条件变量
    std::condition_variable m_condVar;
};


#endif //FFMPEGDEMO_THREADSAFEQUEUE_H
