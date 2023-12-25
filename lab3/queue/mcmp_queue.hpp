#pragma once

#include <pthread.h>
#include <queue>
#include <memory>
#include "queue.hpp"

template<typename T>
class MCMP{
public:
    MCMP(){
        pthread_mutex_init(&_mut, nullptr);
    };

    ~MCMP(){
        pthread_mutex_destroy(&_mut);
    };

    void enqueue(T val){
        auto data(std::make_shared<T>(std::move(val)));
        pthread_mutex_lock(&_mut);
        _dataQueue.push(std::move(data));
        pthread_mutex_unlock(&_mut);
    }

    std::shared_ptr<T> dequeue(){
        pthread_mutex_lock(&_mut);
        if(_dataQueue.empty()){
            pthread_mutex_unlock(&_mut);
            return std::shared_ptr<T>();
        }
        auto res = _dataQueue.front();
        _dataQueue.pop();
        pthread_mutex_unlock(&_mut);
        return res;
    }

    bool empty(){
        pthread_mutex_lock(&_mut);
        bool res = _dataQueue.empty();
        pthread_mutex_unlock(&_mut);
        return res;
    }
private:
    pthread_mutex_t _mut;
    Queue<std::shared_ptr<T>> _dataQueue;
};