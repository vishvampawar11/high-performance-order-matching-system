#include <cstddef>

template <typename T, size_t N>
class SPSCQueue
{
private:
    static constexpr size_t mask = N - 1;
    static_assert((N & mask) == 0, "N must be a power of 2");

    T buf_[N];
    alignas(64) std::atomic<size_t> head_{0};
    alignas(64) std::atomic<size_t> tail_{0};

public:
    bool push(const T &item)
    {
        const size_t h = head_.load(std::memory_order_relaxed);
        const size_t next = (h + 1) & mask;
        if (next == tail_.load(std::memory_order_acquire))
            return false;
        buf_[h] = item;
        head_.store(next, std::memory_order_release);
        return true;
    }

    bool pop(T &out)
    {
        const size_t t = tail_.load(std::memory_order_relaxed);
        if (t == head_.load(std::memory_order_acquire))
            return false;
        out = buf_[t];
        tail_.store((t + 1) & mask, std::memory_order_release);
        return true;
    }
};
