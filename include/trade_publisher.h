#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>
#include "trade.h"

class TradePublisher
{
public:
    void push(const Trade &t)
    {
        {
            std::lock_guard<std::mutex> lock(mtx);
            queue.push(t);
        }
        cv.notify_one();
    }

    bool pop(Trade &t)
    {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [&]
                { return !queue.empty(); });
        t = queue.front();
        queue.pop();
        return true;
    }

private:
    std::queue<Trade> queue;
    std::mutex mtx;
    std::condition_variable cv;
};
